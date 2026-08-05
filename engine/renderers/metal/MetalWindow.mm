/*
Copyright (C) 2026 Rodrigo Jose Hernandez Cordoba

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
#import <AppKit/AppKit.h>
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#import <QuartzCore/CATransaction.h>

#include <algorithm>
#include <array>
#include <iostream>
#include <stdexcept>
#include <vector>
#include "aeongames/Frustum.hpp"
#include "aeongames/GpuClusterParams.hpp"
#include "aeongames/GpuGlobals.hpp"
#include "aeongames/GpuLight.hpp"
#include "aeongames/GpuShadowParams.hpp"
#include "aeongames/LogLevel.hpp"
#include "aeongames/Matrix4x4.hpp"
#include "aeongames/Texture.hpp"
#include "MetalBuffer.h"
#include "MetalMemoryPoolBuffer.h"
#include "MetalPipeline.h"
#include "MetalWindow.h"

namespace AeonGames
{
    namespace
    {
        size_t AlignUp ( size_t aValue, size_t aAlignment )
        {
            return ( aValue + aAlignment - 1 ) & ~ ( aAlignment - 1 );
        }
        /** Frame uniform pool alignment. 256 bytes satisfies both Metal's
         *  constant-buffer offset rule and every argument buffer alignment the
         *  Apple GPUs report. */
        constexpr size_t kUniformPoolAlignment = 256;

        /** Metal's normalized device coordinates are +Y up, while Vulkan -- the
         *  convention the engine's projection matrices and every shared shader
         *  are authored for -- is +Y down. Both APIs put the framebuffer origin
         *  at the top-left, so rendering Vulkan clip space through a normal
         *  Metal viewport mirrors the image vertically, which also reverses
         *  framebuffer winding and turns back-face culling into front-face
         *  culling. A negative-height viewport makes Metal's NDC-to-framebuffer
         *  mapping identical to Vulkan's and fixes both at once (the same
         *  approach MoltenVK takes). */
        MTLViewport FlippedViewport ( double aWidth, double aHeight )
        {
            return MTLViewport { 0.0, aHeight, aWidth, -aHeight, 0.0, 1.0 };
        }
    }

    class MetalWindow::Impl final : public MetalArgumentBufferPool
    {
    public:
        Impl ( void* aDevice, void* aCommandQueue, const RendererSettings& aSettings, void* aWindowId ) :
            mSettings{aSettings},
            mDevice{ ( __bridge id<MTLDevice> ) aDevice},
            mCommandQueue{ ( __bridge id<MTLCommandQueue> ) aCommandQueue}
        {
            if ( aWindowId == nullptr )
            {
                throw std::runtime_error ( "MetalRenderer requires a valid NSView" );
            }
            if ( ![NSThread isMainThread] )
            {
                throw std::runtime_error ( "MetalRenderer window attachment must run on the AppKit main thread" );
            }
            mView = ( __bridge NSView* ) aWindowId;
            [mView setWantsLayer:YES];
            if ( [mView.layer isKindOfClass:[CAMetalLayer class]] )
            {
                mLayer = static_cast<CAMetalLayer*> ( mView.layer );
            }
            else
            {
                mLayer = [CAMetalLayer layer];
                [mView setLayer:mLayer];
            }
            mLayer.device = mDevice;
            mLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;
            mLayer.framebufferOnly = NO;
            mLayer.maximumDrawableCount = kFramesInFlight;
            UpdateLayerSize();
            InitializeShadowFallbacks();
            mShadowParams.params[0] = 1.0f / static_cast<float> ( mSettings.mDirectionalShadowMapResolution );
            mSpotShadowParams.params[0] = 1.0f / static_cast<float> ( mSettings.mSpotShadowMapResolution );
            mPointShadowParams.params[0] = 1.0f / static_cast<float> ( mSettings.mPointShadowMapResolution );
            for ( FrameContext& frame : mFrames )
            {
                frame.available = dispatch_semaphore_create ( 1 );
                frame.uniformPool = std::make_unique<MetalMemoryPoolBuffer> (
                                        ( __bridge void* ) mDevice, mSettings.mUniformPoolInitialCapacity,
                                        kUniformPoolAlignment );
                frame.storagePool = std::make_unique<MetalMemoryPoolBuffer> (
                                        ( __bridge void* ) mDevice, mSettings.mStoragePoolInitialCapacity, 16 );
            }
        }

        ~Impl()
        {
            if ( mFrameBegun )
            {
                FrameContext& frame = mFrames[mFrameIndex];
                if ( mRenderEncoder != nil )
                {
                    [mRenderEncoder endEncoding];
                    mRenderEncoder = nil;
                }
                // A backend-independent readback sequence begins the next
                // frame before Finish(). If the window is then destroyed,
                // submit that otherwise-empty command buffer so Metal never
                // releases an acquired drawable with uncommitted work.
                [frame.commandBuffer commit];
                [frame.commandBuffer waitUntilCompleted];
                frame.commandBuffer = nil;
                frame.drawable = nil;
                mFrameBegun = false;
                dispatch_semaphore_signal ( frame.available );
            }
            Finish();
            if ( [NSThread isMainThread] && mView.layer == mLayer )
            {
                [mView setLayer:nil];
            }
        }

        void SetProjectionMatrix ( const Matrix4x4& aMatrix )
        {
            mProjectionMatrix = aMatrix;
            mFrustum = mProjectionMatrix * mViewMatrix;
            InvalidateFrameBlocks();
        }

        void SetViewMatrix ( const Matrix4x4& aMatrix )
        {
            mViewMatrix = aMatrix;
            mFrustum = mProjectionMatrix * mViewMatrix;
            InvalidateFrameBlocks();
        }

        void SetLights ( std::span<const GpuLight> aLights )
        {
            const size_t count = std::min ( aLights.size(), static_cast<size_t> ( MAX_LIGHTS_PER_FRAME ) );
            mLights.assign ( aLights.begin(), aLights.begin() + count );
            if ( mFrameBegun )
            {
                mFrames[mFrameIndex].lights = {};
            }
        }

        void SetGlobals ( const GpuGlobals& aGlobals )
        {
            mGlobals = aGlobals;
            if ( mFrameBegun )
            {
                mFrames[mFrameIndex].globals = {};
            }
        }

        void SetSpotShadowParams ( const GpuSpotShadowParams& aSpotShadowParams )
        {
            mSpotShadowParams = aSpotShadowParams;
            if ( mFrameBegun )
            {
                mFrames[mFrameIndex].spotShadowParams = {};
            }
        }

        void SetPointShadowParams ( const GpuPointShadowParams& aPointShadowParams )
        {
            mPointShadowParams = aPointShadowParams;
            if ( mFrameBegun )
            {
                mFrames[mFrameIndex].pointShadowParams = {};
            }
        }

        void SetEnvironmentMap ( const Texture* aEnvironmentMap )
        {
            if ( aEnvironmentMap == mEnvironmentSource )
            {
                return;
            }
            mEnvironmentSource = aEnvironmentMap;
            mEnvironmentCube = nil;
            mPrefilteredEnvironment = nil;
            if ( aEnvironmentMap == nullptr )
            {
                return;
            }
            std::vector<std::vector<float>> skybox_mips;
            if ( !PrefilterEnvironmentCube ( *aEnvironmentMap, mSettings.mSkyboxEnvironmentFaceSize,
                                             1, skybox_mips ) )
            {
                throw std::runtime_error ( "MetalRenderer could not build the environment cube" );
            }
            mEnvironmentCube = CreateEnvironmentCube ( skybox_mips, mSettings.mSkyboxEnvironmentFaceSize );

            const uint32_t max_mips = 1u + static_cast<uint32_t> (
                                          std::floor ( std::log2 ( mSettings.mPrefilteredEnvironmentFaceSize ) ) );
            const uint32_t mip_count = std::min ( mSettings.mPrefilteredEnvironmentMipCount, max_mips );
            std::vector<std::vector<float>> prefiltered_mips;
            if ( !PrefilterEnvironmentCube ( *aEnvironmentMap, mSettings.mPrefilteredEnvironmentFaceSize,
                                             mip_count, prefiltered_mips ) )
            {
                throw std::runtime_error ( "MetalRenderer could not prefilter the environment cube" );
            }
            mPrefilteredEnvironment = CreateEnvironmentCube ( prefiltered_mips,
                mSettings.mPrefilteredEnvironmentFaceSize );
        }

        void SetClearColor ( float aRed, float aGreen, float aBlue, float aAlpha )
        {
            mClearColor = MTLClearColorMake ( aRed, aGreen, aBlue, aAlpha );
        }

        void ResizeViewport ( int32_t aX, int32_t aY, uint32_t aWidth, uint32_t aHeight )
        {
            mViewportOriginX = aX;
            mViewportOriginY = aY;
            mViewportWidth = aWidth;
            mViewportHeight = aHeight;
            UpdateLayerSize();
            if ( mFrameBegun )
            {
                mFrames[mFrameIndex].clusterParams = {};
            }
        }

        void BeginFrame()
        {
            if ( mFrameBegun )
            {
                return;
            }
            FrameContext& frame = mFrames[mFrameIndex];
            dispatch_semaphore_wait ( frame.available, DISPATCH_TIME_FOREVER );
            frame.uniformPool->Reset();
            frame.storagePool->Reset();
            frame.clusterParams = {};
            frame.matrices = {};
            frame.lights = {};
            frame.globals = {};
            frame.shadowParams = {};
            frame.spotShadowParams = {};
            frame.pointShadowParams = {};
            frame.shadowDepthParams = {};
            frame.pointShadowDepthParams = {};
            frame.clusterAABBs = {};
            frame.lightGrid = {};
            frame.lightIndexList = {};
            frame.lightIndexCounter = {};
            frame.clusterActive = {};
            frame.hiZReady = false;
            mMainPassOpened = false;
            mShadowParams.params[3] = 0.0f;
            frame.drawable = [mLayer nextDrawable];
            if ( frame.drawable == nil )
            {
                dispatch_semaphore_signal ( frame.available );
                return;
            }
            EnsureAttachments ( frame );
            frame.commandBuffer = [mCommandQueue commandBuffer];
            if ( frame.commandBuffer == nil )
            {
                frame.drawable = nil;
                dispatch_semaphore_signal ( frame.available );
                throw std::runtime_error ( "MetalRenderer failed to create a command buffer" );
            }
            frame.commandBuffer.label = @"AeonEngine frame";
            mFrameBegun = true;
        }

        void PrepareClusters()
        {
            BeginFrame();
            if ( !mFrameBegun )
            {
                return;
            }
            FrameContext& frame = mFrames[mFrameIndex];
            frame.clusterAABBs = frame.storagePool->Allocate ( CLUSTER_COUNT * sizeof ( GpuClusterAABB ) );
            frame.lightGrid = frame.storagePool->Allocate ( CLUSTER_COUNT * sizeof ( GpuLightGridCell ) );
            frame.lightIndexList = frame.storagePool->Allocate ( LIGHT_INDEX_LIST_CAPACITY * sizeof ( uint32_t ) );
            frame.lightIndexCounter = frame.storagePool->Allocate ( sizeof ( uint32_t ) );
            frame.clusterActive = frame.storagePool->Allocate ( CLUSTER_COUNT * sizeof ( uint32_t ) );
            mLastFrameLightGrid = frame.lightGrid;
            mLastFrameClusterActive = frame.clusterActive;
        }

        void BeginRenderPass()
        {
            BeginFrame();
            if ( !mFrameBegun || mRenderEncoder != nil )
            {
                return;
            }
            FrameContext& frame = mFrames[mFrameIndex];
            // The HDR attachments are cleared the first time the pass opens in a
            // frame and loaded on every re-open. Shadow passes and the depth
            // pre-pass interrupt the main pass repeatedly, and re-clearing would
            // both discard the depth pre-pass result and burn full-target
            // bandwidth on every interruption.
            const MTLLoadAction load_action = mMainPassOpened ? MTLLoadActionLoad : MTLLoadActionClear;
            MTLRenderPassDescriptor* descriptor = [MTLRenderPassDescriptor renderPassDescriptor];
            descriptor.colorAttachments[0].texture = frame.hdrColor;
            descriptor.colorAttachments[0].loadAction = load_action;
            descriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
            descriptor.colorAttachments[0].clearColor = mClearColor;
            descriptor.colorAttachments[1].texture = frame.normalRoughness;
            descriptor.colorAttachments[1].loadAction = load_action;
            descriptor.colorAttachments[1].storeAction = MTLStoreActionStore;
            descriptor.colorAttachments[1].clearColor = MTLClearColorMake ( 0.0, 0.0, 0.0, 0.0 );
            descriptor.colorAttachments[2].texture = frame.specularWeight;
            descriptor.colorAttachments[2].loadAction = load_action;
            descriptor.colorAttachments[2].storeAction = MTLStoreActionStore;
            descriptor.colorAttachments[2].clearColor = MTLClearColorMake ( 0.0, 0.0, 0.0, 0.0 );
            descriptor.depthAttachment.texture = frame.depth;
            descriptor.depthAttachment.loadAction = load_action;
            descriptor.depthAttachment.storeAction = MTLStoreActionStore;
            descriptor.depthAttachment.clearDepth = 1.0;
            mRenderEncoder = [frame.commandBuffer renderCommandEncoderWithDescriptor:descriptor];
            if ( mRenderEncoder == nil )
            {
                throw std::runtime_error ( "MetalRenderer failed to begin the drawable render pass" );
            }
            mMainPassOpened = true;
            // The layer's drawable is already sized to the viewport, so the
            // render target origin is the viewport origin.
            const double width = static_cast<double> ( frame.drawable.texture.width );
            const double height = static_cast<double> ( frame.drawable.texture.height );
            [mRenderEncoder setViewport:FlippedViewport ( width, height )];
        }

        void BeginShadowPass ( const Matrix4x4& aLightViewProjection )
        {
            EnsureDirectionalShadowTargets();
            mShadowParams.light_view_projection = aLightViewProjection;
            mShadowParams.params[3] = 1.0f;
            mShadowDepthParams = mShadowParams;
            FrameContext& frame = mFrames[mFrameIndex];
            frame.shadowParams = {};
            frame.shadowDepthParams = {};
            BeginShadowEncoder ( mShadowColor, mShadowMap,
                                 mSettings.mDirectionalShadowMapResolution, 0, 1 );
        }

        void EndShadowPass()
        {
            EndShadowEncoder();
        }

        void BeginSpotShadowPass ( uint32_t aSlot, const Matrix4x4& aLightViewProjection )
        {
            if ( aSlot >= MAX_SPOT_SHADOW_CASTERS )
            {
                return;
            }
            EnsureSpotShadowTargets();
            mShadowDepthParams = {};
            mShadowDepthParams.light_view_projection = aLightViewProjection;
            mShadowDepthParams.params[3] = 1.0f;
            mFrames[mFrameIndex].shadowDepthParams = {};
            BeginShadowEncoder ( mSpotShadowColor, mSpotShadowMap,
                                 mSettings.mSpotShadowMapResolution, aSlot, 1 );
        }

        void EndSpotShadowPass()
        {
            EndShadowEncoder();
        }

        void BeginPointShadowPass ( uint32_t aCaster )
        {
            if ( aCaster >= MAX_POINT_SHADOW_CASTERS )
            {
                return;
            }
            EnsurePointShadowTargets();
            mPointShadowDepthParams = {};
            for ( uint32_t face = 0; face < POINT_SHADOW_FACES; ++face )
            {
                mPointShadowDepthParams.face_view_projection[face] =
                    mPointShadowParams.point_light_view_projection[aCaster * POINT_SHADOW_FACES + face];
            }
            mPointShadowDepthParams.light_position_radius = mPointShadowParams.caster_position_radius[aCaster];
            mFrames[mFrameIndex].pointShadowDepthParams = {};
            mInPointShadowPass = true;
            BeginShadowEncoder ( mPointShadowColor, mPointShadowMap,
                                 mSettings.mPointShadowMapResolution,
                                 aCaster * POINT_SHADOW_FACES, POINT_SHADOW_FACES );
        }

        void EndPointShadowPass()
        {
            mInPointShadowPass = false;
            EndShadowEncoder();
        }

        void EndDepthPrePass()
        {
            if ( mRenderEncoder != nil )
            {
                [mRenderEncoder endEncoding];
                mRenderEncoder = nil;
            }
        }

        void BuildHiZ ( MetalPipeline& aPipeline )
        {
            FrameContext& frame = mFrames[mFrameIndex];
            if ( frame.hiZ == nil )
            {
                return;
            }
            uint32_t width = static_cast<uint32_t> ( frame.hiZ.width );
            uint32_t height = static_cast<uint32_t> ( frame.hiZ.height );
            for ( uint32_t level = 0; level < frame.hiZ.mipmapLevelCount; ++level )
            {
                id<MTLTexture> source = level == 0 ? frame.depth :
                                        [frame.hiZ newTextureViewWithPixelFormat:MTLPixelFormatR32Float
                                         textureType:MTLTextureType2D
                                         levels:NSMakeRange ( level - 1, 1 )
                                         slices:NSMakeRange ( 0, 1 )];
                id<MTLTexture> destination = [frame.hiZ newTextureViewWithPixelFormat:MTLPixelFormatR32Float
                                              textureType:MTLTextureType2D
                                              levels:NSMakeRange ( level, 1 )
                                              slices:NSMakeRange ( 0, 1 )];
                const MetalTextureBinding textures[]
                {
                    { "uSource"_crc32, ( __bridge void* ) source, ( __bridge void* ) mLinearSampler },
                    { "uDest"_crc32, ( __bridge void* ) destination, nullptr },
                };
                aPipeline.Dispatch ( GetCommandBuffer(), *this, ( width + 7u ) / 8u, ( height + 7u ) / 8u, 1,
                                     {}, 0, nullptr, nullptr, nullptr, nullptr, textures );
                width = std::max ( 1u, width / 2 );
                height = std::max ( 1u, height / 2 );
            }
            frame.hiZReady = true;
        }

        void ResolveFrame ( MetalPipeline& aSkyboxPipeline, MetalPipeline& aTonemapPipeline )
        {
            BeginRenderPass();
            if ( !mFrameBegun )
            {
                return;
            }
            FrameContext& frame = mFrames[mFrameIndex];
            if ( mEnvironmentCube != nil )
            {
                const MetalBufferBinding buffers[]
                {
                    { "Matrices"_crc32, &GetMatrices() },
                };
                const MetalTextureBinding textures[]
                {
                    {
                        "EnvironmentMap"_crc32, ( __bridge void* ) mEnvironmentCube,
                        ( __bridge void* ) mLinearSampler
                    },
                };
                aSkyboxPipeline.DrawFullscreen ( ( __bridge void* ) mRenderEncoder, *this, buffers, textures );
            }
            if ( mRenderEncoder != nil )
            {
                [mRenderEncoder endEncoding];
                mRenderEncoder = nil;
            }

            MTLRenderPassDescriptor* descriptor = [MTLRenderPassDescriptor renderPassDescriptor];
            descriptor.colorAttachments[0].texture = frame.drawable.texture;
            descriptor.colorAttachments[0].loadAction = MTLLoadActionDontCare;
            descriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
            mRenderEncoder = [frame.commandBuffer renderCommandEncoderWithDescriptor:descriptor];
            const double width = static_cast<double> ( frame.drawable.texture.width );
            const double height = static_cast<double> ( frame.drawable.texture.height );
            [mRenderEncoder setViewport:FlippedViewport ( width, height )];
            const MetalBufferBinding buffers[]
            {
                { "Matrices"_crc32, &GetMatrices() },
                { "Globals"_crc32, &GetGlobals() },
            };
            const MetalTextureBinding textures[]
            {
                { "HdrColor"_crc32, ( __bridge void* ) frame.hdrColor, ( __bridge void* ) mLinearSampler },
                { "GNormalRough"_crc32, ( __bridge void* ) frame.normalRoughness, ( __bridge void* ) mLinearSampler },
                { "GSpecWeight"_crc32, ( __bridge void* ) frame.specularWeight, ( __bridge void* ) mLinearSampler },
                { "SceneDepth"_crc32, ( __bridge void* ) frame.depth, ( __bridge void* ) mLinearSampler },
                {
                    "PrefilteredEnvironment"_crc32,
                    ( __bridge void* ) ( mPrefilteredEnvironment != nil ? mPrefilteredEnvironment : mFallbackEnvironment ),
                    ( __bridge void* ) mLinearSampler
                },
            };
            aTonemapPipeline.DrawFullscreen ( ( __bridge void* ) mRenderEncoder, *this, buffers, textures );
        }

        void RenderOverlay ( void* aPipeline, void* aSampler, void* aDepthState, const uint8_t* aPixels,
                             uint32_t aWidth, uint32_t aHeight )
        {
            BeginFrame();
            if ( !mFrameBegun )
            {
                return;
            }
            FrameContext& frame = mFrames[mFrameIndex];
            if ( frame.overlay == nil || frame.overlay.width != aWidth || frame.overlay.height != aHeight )
            {
                MTLTextureDescriptor* descriptor = [MTLTextureDescriptor
                                                    texture2DDescriptorWithPixelFormat:MTLPixelFormatBGRA8Unorm
                                                    width:aWidth height:aHeight mipmapped:NO];
                descriptor.storageMode = MTLStorageModeShared;
                descriptor.usage = MTLTextureUsageShaderRead;
                frame.overlay = [mDevice newTextureWithDescriptor:descriptor];
                if ( frame.overlay == nil )
                {
                    throw std::runtime_error ( "MetalRenderer failed to allocate the GUI overlay texture" );
                }
            }
            [frame.overlay replaceRegion:MTLRegionMake2D ( 0, 0, aWidth, aHeight )
             mipmapLevel:0 withBytes:aPixels bytesPerRow:static_cast<NSUInteger> ( aWidth ) * 4];

            BeginRenderPass();
            [mRenderEncoder setRenderPipelineState: ( __bridge id<MTLRenderPipelineState> ) aPipeline];
            [mRenderEncoder setDepthStencilState: ( __bridge id<MTLDepthStencilState> ) aDepthState];
            [mRenderEncoder setCullMode:MTLCullModeNone];
            [mRenderEncoder setTriangleFillMode:MTLTriangleFillModeFill];
            [mRenderEncoder setFragmentTexture:frame.overlay atIndex:0];
            [mRenderEncoder setFragmentSamplerState: ( __bridge id<MTLSamplerState> ) aSampler atIndex:0];
            [mRenderEncoder drawPrimitives:MTLPrimitiveTypeTriangleStrip vertexStart:0 vertexCount:4];
        }

        void EndRender()
        {
            if ( !mFrameBegun )
            {
                return;
            }
            FrameContext& frame = mFrames[mFrameIndex];
            if ( mRenderEncoder != nil )
            {
                [mRenderEncoder endEncoding];
                mRenderEncoder = nil;
            }
            if ( mCaptureRequested )
            {
                EncodeCapture ( frame );
                mCaptureRequested = false;
            }
            [frame.commandBuffer presentDrawable:frame.drawable];
            dispatch_semaphore_t available = frame.available;
            [frame.commandBuffer addCompletedHandler: ^ ( id<MTLCommandBuffer> aCommandBuffer )
            {
                if ( aCommandBuffer.status == MTLCommandBufferStatusError )
                {
                    std::cerr << LogLevel::Error << "Metal command buffer failed: "
                              << aCommandBuffer.error.description.UTF8String << std::endl;
                }
                dispatch_semaphore_signal ( available );
            }];
            mLastSubmitted = frame.commandBuffer;
            [frame.commandBuffer commit];
            frame.commandBuffer = nil;
            frame.drawable = nil;
            mFrameBegun = false;
            mFrameIndex = ( mFrameIndex + 1 ) % kFramesInFlight;
        }

        void Finish() const
        {
            [mLastSubmitted waitUntilCompleted];
        }

        void RequestCapture()
        {
            mCaptureRequested = true;
        }

        bool ReadPixels ( Texture& aTexture ) const
        {
            [mLastSubmitted waitUntilCompleted];
            if ( mCaptureBuffer == nil || mCaptureWidth == 0 || mCaptureHeight == 0 )
            {
                return false;
            }
            const auto* source = static_cast<const uint8_t*> ( mCaptureBuffer.contents );
            std::vector<uint8_t> rgba ( static_cast<size_t> ( mCaptureWidth ) * mCaptureHeight * 4 );
            for ( uint32_t y = 0; y < mCaptureHeight; ++y )
            {
                const uint8_t* source_row = source + static_cast<size_t> ( y ) * mCaptureBytesPerRow;
                uint8_t* destination_row = rgba.data() + static_cast<size_t> ( y ) * mCaptureWidth * 4;
                for ( uint32_t x = 0; x < mCaptureWidth; ++x )
                {
                    destination_row[x * 4 + 0] = source_row[x * 4 + 2];
                    destination_row[x * 4 + 1] = source_row[x * 4 + 1];
                    destination_row[x * 4 + 2] = source_row[x * 4 + 0];
                    destination_row[x * 4 + 3] = source_row[x * 4 + 3];
                }
            }
            aTexture.Resize ( mCaptureWidth, mCaptureHeight, rgba.data(), Texture::Format::RGBA, Texture::Type::UNSIGNED_BYTE );
            return true;
        }

        const Frustum& GetFrustum() const
        {
            return mFrustum;
        }

        const Matrix4x4& GetProjectionMatrix() const
        {
            return mProjectionMatrix;
        }

        void* GetCommandBuffer()
        {
            BeginFrame();
            return mFrameBegun ? ( __bridge void* ) mFrames[mFrameIndex].commandBuffer : nullptr;
        }

        void* GetRenderEncoder()
        {
            BeginRenderPass();
            return ( __bridge void* ) mRenderEncoder;
        }

        BufferAccessor AllocateUniform ( size_t aSize )
        {
            BeginFrame();
            return mFrames[mFrameIndex].uniformPool->Allocate ( aSize );
        }

        Allocation AllocateArgumentBuffer ( size_t aSize, size_t aAlignment ) final
        {
            BeginFrame();
            if ( aAlignment > kUniformPoolAlignment )
        {
            throw std::runtime_error (
                "Metal argument buffer alignment exceeds the frame uniform pool alignment" );
            }
            MetalMemoryPoolBuffer& pool = *mFrames[mFrameIndex].uniformPool;
            const BufferAccessor accessor = pool.Allocate ( std::max<size_t> ( aSize, 1 ) );
            const auto& buffer = static_cast<const MetalBuffer&> ( pool.GetBuffer() );
            return Allocation{ buffer.GetNativeBuffer(), accessor.GetOffset() };
        }

        BufferAccessor AllocateStorage ( size_t aSize )
        {
            BeginFrame();
            return mFrames[mFrameIndex].storagePool->Allocate ( aSize );
        }

        const BufferAccessor& GetClusterParams()
        {
            BeginFrame();
            FrameContext& frame = mFrames[mFrameIndex];
            if ( frame.clusterParams.GetMemoryPoolBuffer() == nullptr )
            {
                GpuClusterParams params{};
                params.inverse_projection = mProjectionMatrix.GetInvertedMatrix4x4();
                params.screen[0] = static_cast<float> ( mViewportWidth );
                params.screen[1] = static_cast<float> ( mViewportHeight );
                params.occlusion[0] = 1.0f;
                frame.clusterParams = frame.uniformPool->Allocate ( sizeof ( params ) );
                frame.clusterParams.WriteMemory ( 0, sizeof ( params ), &params );
            }
            return frame.clusterParams;
        }

        const BufferAccessor& GetMatrices()
        {
            BeginFrame();
            FrameContext& frame = mFrames[mFrameIndex];
            if ( frame.matrices.GetMemoryPoolBuffer() == nullptr )
            {
                const Matrix4x4 matrices[] { mProjectionMatrix, mViewMatrix };
                frame.matrices = frame.uniformPool->Allocate ( sizeof ( matrices ) );
                frame.matrices.WriteMemory ( 0, sizeof ( matrices ), matrices );
            }
            return frame.matrices;
        }

        const BufferAccessor& GetLights()
        {
            BeginFrame();
            FrameContext& frame = mFrames[mFrameIndex];
            if ( frame.lights.GetMemoryPoolBuffer() == nullptr )
            {
                const size_t size = sizeof ( GpuLightsHeader ) + mLights.size() * sizeof ( GpuLight );
                frame.lights = frame.storagePool->Allocate ( size );
                const GpuLightsHeader header{static_cast<uint32_t> ( mLights.size() ), {0, 0, 0}};
                frame.lights.WriteMemory ( 0, sizeof ( header ), &header );
                if ( !mLights.empty() )
                {
                    frame.lights.WriteMemory ( sizeof ( header ), mLights.size() * sizeof ( GpuLight ), mLights.data() );
                }
            }
            return frame.lights;
        }

        const BufferAccessor& GetGlobals()
        {
            BeginFrame();
            FrameContext& frame = mFrames[mFrameIndex];
            if ( frame.globals.GetMemoryPoolBuffer() == nullptr )
            {
                frame.globals = frame.uniformPool->Allocate ( sizeof ( mGlobals ) );
                frame.globals.WriteMemory ( 0, sizeof ( mGlobals ), &mGlobals );
            }
            return frame.globals;
        }

        const BufferAccessor& GetShadowParams()
        {
            BeginFrame();
            FrameContext& frame = mFrames[mFrameIndex];
            if ( frame.shadowParams.GetMemoryPoolBuffer() == nullptr )
            {
                frame.shadowParams = frame.uniformPool->Allocate ( sizeof ( mShadowParams ) );
                frame.shadowParams.WriteMemory ( 0, sizeof ( mShadowParams ), &mShadowParams );
            }
            return frame.shadowParams;
        }

        const BufferAccessor& GetSpotShadowParams()
        {
            BeginFrame();
            FrameContext& frame = mFrames[mFrameIndex];
            if ( frame.spotShadowParams.GetMemoryPoolBuffer() == nullptr )
            {
                frame.spotShadowParams = frame.uniformPool->Allocate ( sizeof ( mSpotShadowParams ) );
                frame.spotShadowParams.WriteMemory ( 0, sizeof ( mSpotShadowParams ), &mSpotShadowParams );
            }
            return frame.spotShadowParams;
        }

        const BufferAccessor& GetPointShadowParams()
        {
            BeginFrame();
            FrameContext& frame = mFrames[mFrameIndex];
            if ( frame.pointShadowParams.GetMemoryPoolBuffer() == nullptr )
            {
                frame.pointShadowParams = frame.uniformPool->Allocate ( sizeof ( mPointShadowParams ) );
                frame.pointShadowParams.WriteMemory ( 0, sizeof ( mPointShadowParams ), &mPointShadowParams );
            }
            return frame.pointShadowParams;
        }

        const BufferAccessor& GetShadowDepthParams()
        {
            BeginFrame();
            FrameContext& frame = mFrames[mFrameIndex];
            if ( frame.shadowDepthParams.GetMemoryPoolBuffer() == nullptr )
            {
                frame.shadowDepthParams = frame.uniformPool->Allocate ( sizeof ( mShadowDepthParams ) );
                frame.shadowDepthParams.WriteMemory ( 0, sizeof ( mShadowDepthParams ), &mShadowDepthParams );
            }
            return frame.shadowDepthParams;
        }

        const BufferAccessor& GetPointShadowDepthParams()
        {
            BeginFrame();
            FrameContext& frame = mFrames[mFrameIndex];
            if ( frame.pointShadowDepthParams.GetMemoryPoolBuffer() == nullptr )
            {
                frame.pointShadowDepthParams = frame.uniformPool->Allocate ( sizeof ( mPointShadowDepthParams ) );
                frame.pointShadowDepthParams.WriteMemory ( 0, sizeof ( mPointShadowDepthParams ), &mPointShadowDepthParams );
            }
            return frame.pointShadowDepthParams;
        }

        const BufferAccessor& GetClusterAABBs() const
        {
            return mFrames[mFrameIndex].clusterAABBs;
        }
        const BufferAccessor& GetLightGrid() const
        {
            return mFrames[mFrameIndex].lightGrid;
        }
        const BufferAccessor& GetLightIndexList() const
        {
            return mFrames[mFrameIndex].lightIndexList;
        }
        const BufferAccessor& GetLightIndexCounter() const
        {
            return mFrames[mFrameIndex].lightIndexCounter;
        }
        const BufferAccessor& GetClusterActive() const
        {
            return mFrames[mFrameIndex].clusterActive;
        }
        const BufferAccessor* GetFrameLightGrid() const
        {
            return mLastFrameLightGrid.GetMemoryPoolBuffer() != nullptr ? &mLastFrameLightGrid : nullptr;
        }
        const BufferAccessor* GetFrameClusterActive() const
        {
            return mLastFrameClusterActive.GetMemoryPoolBuffer() != nullptr ? &mLastFrameClusterActive : nullptr;
        }

        void* GetShadowMap() const
        {
            return ( __bridge void* ) ( mShadowMap != nil ? mShadowMap : mFallbackShadowMap );
        }
        void* GetSpotShadowMap() const
        {
            return ( __bridge void* ) ( mSpotShadowMap != nil ? mSpotShadowMap : mFallbackSpotShadowMap );
        }
        void* GetPointShadowMap() const
        {
            return ( __bridge void* ) ( mPointShadowMap != nil ? mPointShadowMap : mFallbackPointShadowMap );
        }
        void* GetShadowSampler() const
        {
            return ( __bridge void* ) mShadowSampler;
        }
        void* GetHiZTexture() const
        {
            return ( __bridge void* ) mFrames[mFrameIndex].hiZ;
        }
        void* GetLinearSampler() const
        {
            return ( __bridge void* ) mLinearSampler;
        }
        void* GetHiZSampler() const
        {
            return ( __bridge void* ) mHiZSampler;
        }
        bool HasHiZ() const
        {
            return mFrames[mFrameIndex].hiZReady;
        }
        bool IsPointShadowPass() const
        {
            return mInPointShadowPass;
        }

    private:
        struct FrameContext
        {
            dispatch_semaphore_t available{nullptr};
            id<CAMetalDrawable> drawable{nil};
            id<MTLCommandBuffer> commandBuffer{nil};
            std::unique_ptr<MetalMemoryPoolBuffer> uniformPool{};
            std::unique_ptr<MetalMemoryPoolBuffer> storagePool{};
            id<MTLTexture> normalRoughness{nil};
            id<MTLTexture> specularWeight{nil};
            id<MTLTexture> hdrColor{nil};
            id<MTLTexture> depth{nil};
            id<MTLTexture> hiZ{nil};
            id<MTLTexture> overlay{nil};
            BufferAccessor clusterParams{};
            BufferAccessor matrices{};
            BufferAccessor lights{};
            BufferAccessor globals{};
            BufferAccessor shadowParams{};
            BufferAccessor spotShadowParams{};
            BufferAccessor pointShadowParams{};
            BufferAccessor shadowDepthParams{};
            BufferAccessor pointShadowDepthParams{};
            BufferAccessor clusterAABBs{};
            BufferAccessor lightGrid{};
            BufferAccessor lightIndexList{};
            BufferAccessor lightIndexCounter{};
            BufferAccessor clusterActive{};
            bool hiZReady{false};
        };

        void InvalidateFrameBlocks()
        {
            if ( !mFrameBegun )
            {
                return;
            }
            FrameContext& frame = mFrames[mFrameIndex];
            frame.clusterParams = {};
            frame.matrices = {};
        }

        void InitializeShadowFallbacks()
        {
            MTLTextureDescriptor* directional = [MTLTextureDescriptor
                                                 texture2DDescriptorWithPixelFormat:MTLPixelFormatDepth32Float
                                                 width:1 height:1 mipmapped:NO];
            directional.storageMode = MTLStorageModeShared;
            directional.usage = MTLTextureUsageShaderRead;
            mFallbackShadowMap = [mDevice newTextureWithDescriptor:directional];

            MTLTextureDescriptor* spot = [directional copy];
            spot.textureType = MTLTextureType2DArray;
            spot.arrayLength = MAX_SPOT_SHADOW_CASTERS;
            mFallbackSpotShadowMap = [mDevice newTextureWithDescriptor:spot];

            MTLTextureDescriptor* point = [directional copy];
            point.textureType = MTLTextureTypeCubeArray;
            point.arrayLength = MAX_POINT_SHADOW_CASTERS;
            mFallbackPointShadowMap = [mDevice newTextureWithDescriptor:point];

            // Sampled whenever a shadow type is inactive. A freshly allocated
            // texture has undefined contents, so seed the far plane: with the
            // LessEqual comparison sampler that reads back as fully lit.
            auto clear_shadow_fallback = [] ( id<MTLTexture> aTexture, uint32_t aSlices )
            {
                const float far_depth = 1.0f;
                for ( uint32_t slice = 0; slice < aSlices; ++slice )
                {
                    [aTexture replaceRegion:MTLRegionMake2D ( 0, 0, 1, 1 ) mipmapLevel:0 slice:slice
                     withBytes:&far_depth bytesPerRow:sizeof ( far_depth ) bytesPerImage:sizeof ( far_depth )];
                }
            };
            clear_shadow_fallback ( mFallbackShadowMap, 1 );
            clear_shadow_fallback ( mFallbackSpotShadowMap, MAX_SPOT_SHADOW_CASTERS );
            clear_shadow_fallback ( mFallbackPointShadowMap, MAX_POINT_SHADOW_CASTERS * 6 );

            MTLSamplerDescriptor* sampler = [MTLSamplerDescriptor new];
            sampler.minFilter = MTLSamplerMinMagFilterLinear;
            sampler.magFilter = MTLSamplerMinMagFilterLinear;
            sampler.sAddressMode = MTLSamplerAddressModeClampToEdge;
            sampler.tAddressMode = MTLSamplerAddressModeClampToEdge;
            sampler.rAddressMode = MTLSamplerAddressModeClampToEdge;
            sampler.compareFunction = MTLCompareFunctionLessEqual;
            sampler.supportArgumentBuffers = YES;
            mShadowSampler = [mDevice newSamplerStateWithDescriptor:sampler];
            if ( mFallbackShadowMap == nil || mFallbackSpotShadowMap == nil ||
                 mFallbackPointShadowMap == nil || mShadowSampler == nil )
            {
                throw std::runtime_error ( "MetalRenderer failed to create fallback shadow resources" );
            }

            MTLTextureDescriptor* environment = [MTLTextureDescriptor
                                                 textureCubeDescriptorWithPixelFormat:MTLPixelFormatRGBA32Float
                                                 size:1 mipmapped:NO];
            environment.storageMode = MTLStorageModeShared;
            environment.usage = MTLTextureUsageShaderRead;
            mFallbackEnvironment = [mDevice newTextureWithDescriptor:environment];
            const float black[] {0.0f, 0.0f, 0.0f, 1.0f};
            for ( uint32_t face = 0; face < 6; ++face )
            {
                [mFallbackEnvironment replaceRegion:MTLRegionMake2D ( 0, 0, 1, 1 )
                 mipmapLevel:0 slice:face withBytes:black bytesPerRow:sizeof ( black ) bytesPerImage:sizeof ( black )];
            }
            MTLSamplerDescriptor* linear = [MTLSamplerDescriptor new];
            linear.minFilter = MTLSamplerMinMagFilterLinear;
            linear.magFilter = MTLSamplerMinMagFilterLinear;
            linear.mipFilter = MTLSamplerMipFilterLinear;
            linear.sAddressMode = MTLSamplerAddressModeClampToEdge;
            linear.tAddressMode = MTLSamplerAddressModeClampToEdge;
            linear.rAddressMode = MTLSamplerAddressModeClampToEdge;
            linear.supportArgumentBuffers = YES;
            mLinearSampler = [mDevice newSamplerStateWithDescriptor:linear];
            MTLSamplerDescriptor* hi_z_sampler = [linear copy];
            hi_z_sampler.minFilter = MTLSamplerMinMagFilterNearest;
            hi_z_sampler.magFilter = MTLSamplerMinMagFilterNearest;
            hi_z_sampler.mipFilter = MTLSamplerMipFilterNearest;
            mHiZSampler = [mDevice newSamplerStateWithDescriptor:hi_z_sampler];
        }

        id<MTLTexture> CreateEnvironmentCube ( const std::vector<std::vector<float>>& aMips,
                                               uint32_t aFaceSize ) const
        {
            MTLTextureDescriptor* descriptor = [MTLTextureDescriptor
                                                textureCubeDescriptorWithPixelFormat:MTLPixelFormatRGBA32Float
                                                size:aFaceSize mipmapped:aMips.size() > 1];
            descriptor.mipmapLevelCount = aMips.size();
            descriptor.storageMode = MTLStorageModeShared;
            descriptor.usage = MTLTextureUsageShaderRead;
id<MTLTexture> texture = [mDevice newTextureWithDescriptor:descriptor];
            if ( texture == nil )
        {
            throw std::runtime_error ( "MetalRenderer failed to allocate an environment cube" );
            }
            uint32_t size = aFaceSize;
            for ( uint32_t mip = 0; mip < aMips.size(); ++mip )
            {
                const size_t face_values = static_cast<size_t> ( size ) * size * 4;
                for ( uint32_t face = 0; face < 6; ++face )
                {
                    [texture replaceRegion:MTLRegionMake2D ( 0, 0, size, size )
                     mipmapLevel:mip slice:face withBytes:aMips[mip].data() + face * face_values
                     bytesPerRow:static_cast<NSUInteger> ( size ) * sizeof ( float ) * 4
                     bytesPerImage:face_values * sizeof ( float )];
                }
                size = std::max ( 1u, size / 2 );
            }
            return texture;
        }

        void EnsureDirectionalShadowTargets()
        {
            if ( mShadowMap != nil )
            {
                return;
            }
            mShadowMap = CreateShadowTexture ( MTLTextureType2D,
                                               mSettings.mDirectionalShadowMapResolution, 1 );
            mShadowColor = CreateShadowColorTexture ( MTLTextureType2D,
                mSettings.mDirectionalShadowMapResolution, 1 );
        }

        void EnsureSpotShadowTargets()
        {
            if ( mSpotShadowMap != nil )
            {
                return;
            }
            mSpotShadowMap = CreateShadowTexture ( MTLTextureType2DArray,
                                                   mSettings.mSpotShadowMapResolution,
                                                   MAX_SPOT_SHADOW_CASTERS );
            mSpotShadowColor = CreateShadowColorTexture ( MTLTextureType2DArray,
                mSettings.mSpotShadowMapResolution,
                MAX_SPOT_SHADOW_CASTERS );
        }

        void EnsurePointShadowTargets()
        {
            if ( mPointShadowMap != nil )
            {
                return;
            }
            mPointShadowMap = CreateShadowTexture ( MTLTextureTypeCubeArray,
                                                    mSettings.mPointShadowMapResolution,
                                                    MAX_POINT_SHADOW_CASTERS );
            mPointShadowColor = CreateShadowColorTexture ( MTLTextureType2DArray,
                mSettings.mPointShadowMapResolution,
                POINT_SHADOW_FACES * MAX_POINT_SHADOW_CASTERS );
        }

        id<MTLTexture> CreateShadowTexture ( MTLTextureType aType, uint32_t aResolution,
                                             uint32_t aArrayLength ) const
        {
            MTLTextureDescriptor* descriptor = [MTLTextureDescriptor
                                                texture2DDescriptorWithPixelFormat:MTLPixelFormatDepth32Float
                                                width:aResolution height:aResolution mipmapped:NO];
            descriptor.textureType = aType;
            descriptor.arrayLength = aArrayLength;
            descriptor.storageMode = MTLStorageModePrivate;
            descriptor.usage = MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead;
            id<MTLTexture> texture = [mDevice newTextureWithDescriptor:descriptor];
            if ( texture == nil )
            {
                throw std::runtime_error ( "MetalRenderer failed to allocate a shadow depth texture" );
            }
            return texture;
        }

        id<MTLTexture> CreateShadowColorTexture ( MTLTextureType aType, uint32_t aResolution,
                uint32_t aArrayLength ) const
        {
            MTLTextureDescriptor* descriptor = [MTLTextureDescriptor
                                                texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA16Float
                                                width:aResolution height:aResolution mipmapped:NO];
            descriptor.textureType = aType;
            descriptor.arrayLength = aArrayLength;
            descriptor.storageMode = MTLStorageModePrivate;
            descriptor.usage = MTLTextureUsageRenderTarget;
            id<MTLTexture> texture = [mDevice newTextureWithDescriptor:descriptor];
            if ( texture == nil )
            {
                throw std::runtime_error ( "MetalRenderer failed to allocate a shadow color texture" );
            }
            return texture;
        }

        void BeginShadowEncoder ( id<MTLTexture> aColor, id<MTLTexture> aDepth,
                                  uint32_t aResolution, uint32_t aSlice, uint32_t aLayerCount )
        {
            BeginFrame();
            if ( mRenderEncoder != nil )
            {
                [mRenderEncoder endEncoding];
                mRenderEncoder = nil;
            }
            MTLRenderPassDescriptor* descriptor = [MTLRenderPassDescriptor renderPassDescriptor];
            descriptor.colorAttachments[0].texture = aColor;
            descriptor.colorAttachments[0].slice = aSlice;
            descriptor.colorAttachments[0].loadAction = MTLLoadActionDontCare;
            descriptor.colorAttachments[0].storeAction = MTLStoreActionDontCare;
            descriptor.depthAttachment.texture = aDepth;
            descriptor.depthAttachment.slice = aSlice;
            descriptor.depthAttachment.loadAction = MTLLoadActionClear;
            descriptor.depthAttachment.storeAction = MTLStoreActionStore;
            descriptor.depthAttachment.clearDepth = 1.0;
            descriptor.renderTargetArrayLength = aLayerCount;
            mRenderEncoder = [mFrames[mFrameIndex].commandBuffer renderCommandEncoderWithDescriptor:descriptor];
            if ( mRenderEncoder == nil )
            {
                throw std::runtime_error ( "MetalRenderer failed to begin a shadow render pass" );
            }
            // Shadow maps are sampled with UVs derived from the same Vulkan-
            // convention light matrices, so they must be rendered with the same
            // flipped mapping as the camera passes.
            [mRenderEncoder setViewport:FlippedViewport ( static_cast<double> ( aResolution ),
                    static_cast<double> ( aResolution ) )];
            [mRenderEncoder setDepthBias:4.0f slopeScale:2.5f clamp:0.0f];
        }

        void EndShadowEncoder()
        {
            if ( mRenderEncoder != nil )
            {
                [mRenderEncoder endEncoding];
                mRenderEncoder = nil;
            }
            // The main pass is not re-opened here: it opens lazily on the next
            // draw, so a run of shadow passes costs one pass each instead of an
            // extra empty clear-and-store of the HDR attachments in between.
        }

        void EnsureAttachments ( FrameContext& aFrame )
        {
            const NSUInteger width = aFrame.drawable.texture.width;
            const NSUInteger height = aFrame.drawable.texture.height;
            if ( aFrame.depth != nil && aFrame.depth.width == width && aFrame.depth.height == height )
            {
                return;
            }
            MTLTextureDescriptor* color = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA16Float
                                           width:width height:height mipmapped:NO];
            color.storageMode = MTLStorageModePrivate;
            color.usage = MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead;
            aFrame.hdrColor = [mDevice newTextureWithDescriptor:color];
            aFrame.normalRoughness = [mDevice newTextureWithDescriptor:color];
            aFrame.specularWeight = [mDevice newTextureWithDescriptor:color];
            MTLTextureDescriptor* depth = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatDepth32Float
                                           width:width height:height mipmapped:NO];
            depth.storageMode = MTLStorageModePrivate;
            depth.usage = MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead;
            aFrame.depth = [mDevice newTextureWithDescriptor:depth];
            const uint32_t hi_z_width = std::max ( 1u, static_cast<uint32_t> ( width ) / 2 );
            const uint32_t hi_z_height = std::max ( 1u, static_cast<uint32_t> ( height ) / 2 );
            const uint32_t hi_z_mips = 1u + static_cast<uint32_t> (
                                           std::floor ( std::log2 ( std::max ( hi_z_width, hi_z_height ) ) ) );
            MTLTextureDescriptor* hi_z = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatR32Float
                                          width:hi_z_width height:hi_z_height mipmapped:hi_z_mips > 1];
            hi_z.mipmapLevelCount = hi_z_mips;
            hi_z.storageMode = MTLStorageModePrivate;
            hi_z.usage = MTLTextureUsageShaderRead | MTLTextureUsageShaderWrite | MTLTextureUsagePixelFormatView;
            aFrame.hiZ = [mDevice newTextureWithDescriptor:hi_z];
            if ( aFrame.hdrColor == nil || aFrame.normalRoughness == nil ||
                 aFrame.specularWeight == nil || aFrame.depth == nil || aFrame.hiZ == nil )
            {
                throw std::runtime_error ( "MetalRenderer failed to allocate frame attachments" );
            }
        }

        void UpdateLayerSize()
        {
            const CGFloat scale = mView.window != nil ? mView.window.backingScaleFactor : NSScreen.mainScreen.backingScaleFactor;
            [CATransaction begin];
            [CATransaction setDisableActions:YES];
            mLayer.contentsScale = scale;
            mLayer.frame = mView.bounds;
            const CGFloat width = mViewportWidth != 0 ? mViewportWidth : mView.bounds.size.width;
            const CGFloat height = mViewportHeight != 0 ? mViewportHeight : mView.bounds.size.height;
            mLayer.drawableSize = CGSizeMake ( std::max<CGFloat> ( 1.0, width * scale ),
                                               std::max<CGFloat> ( 1.0, height * scale ) );
            [CATransaction commit];
        }

        void EncodeCapture ( FrameContext& aFrame )
        {
            mCaptureWidth = static_cast<uint32_t> ( aFrame.drawable.texture.width );
            mCaptureHeight = static_cast<uint32_t> ( aFrame.drawable.texture.height );
            mCaptureBytesPerRow = AlignUp ( static_cast<size_t> ( mCaptureWidth ) * 4, 256 );
            const size_t capture_size = mCaptureBytesPerRow * mCaptureHeight;
            if ( mCaptureBuffer == nil || mCaptureBuffer.length < capture_size )
            {
                mCaptureBuffer = [mDevice newBufferWithLength:capture_size options:MTLResourceStorageModeShared];
            }
            id<MTLBlitCommandEncoder> blit = [aFrame.commandBuffer blitCommandEncoder];
            [blit copyFromTexture:aFrame.drawable.texture
             sourceSlice:0
             sourceLevel:0
             sourceOrigin:MTLOriginMake ( 0, 0, 0 )
             sourceSize:MTLSizeMake ( mCaptureWidth, mCaptureHeight, 1 )
             toBuffer:mCaptureBuffer
             destinationOffset:0
             destinationBytesPerRow:mCaptureBytesPerRow
             destinationBytesPerImage:capture_size];
            [blit endEncoding];
        }

        const RendererSettings& mSettings;
        id<MTLDevice> mDevice{nil};
        id<MTLCommandQueue> mCommandQueue{nil};
        __weak NSView* mView{nil};
        CAMetalLayer* mLayer{nil};
        std::array<FrameContext, kFramesInFlight> mFrames{};
        id<MTLRenderCommandEncoder> mRenderEncoder{nil};
        id<MTLCommandBuffer> mLastSubmitted{nil};
        id<MTLBuffer> mCaptureBuffer{nil};
        // Scene colour clear, matched to the value VulkanWindow::BeginRenderPass
        // bakes into its HDR pass and to OpenGLWindow's initial glClearColor.
        // Nothing in the game calls SetClearColor, so this default is what the
        // empty background actually shows; a backend-specific value here reads
        // as a different background colour on Metal than on the other two.
        // Alpha is irrelevant: tonemap.frag samples HdrColor.rgb and writes 1.0.
        MTLClearColor mClearColor{MTLClearColorMake ( 0.5, 0.5, 0.5, 0.0 ) };
        Matrix4x4 mProjectionMatrix{};
        Matrix4x4 mViewMatrix{};
        Frustum mFrustum{};
        std::vector<GpuLight> mLights{};
        GpuGlobals mGlobals{};
        GpuShadowParams mShadowParams{};
        GpuSpotShadowParams mSpotShadowParams{};
        GpuPointShadowParams mPointShadowParams{};
        GpuShadowParams mShadowDepthParams{};
        GpuPointDepthParams mPointShadowDepthParams{};
        BufferAccessor mLastFrameLightGrid{};
        BufferAccessor mLastFrameClusterActive{};
        id<MTLTexture> mFallbackShadowMap{nil};
        id<MTLTexture> mFallbackSpotShadowMap{nil};
        id<MTLTexture> mFallbackPointShadowMap{nil};
        id<MTLSamplerState> mShadowSampler{nil};
        id<MTLSamplerState> mLinearSampler{nil};
        id<MTLSamplerState> mHiZSampler{nil};
        const Texture* mEnvironmentSource{nullptr};
        id<MTLTexture> mEnvironmentCube{nil};
        id<MTLTexture> mPrefilteredEnvironment{nil};
        id<MTLTexture> mFallbackEnvironment{nil};
        id<MTLTexture> mShadowMap{nil};
        id<MTLTexture> mShadowColor{nil};
        id<MTLTexture> mSpotShadowMap{nil};
        id<MTLTexture> mSpotShadowColor{nil};
        id<MTLTexture> mPointShadowMap{nil};
        id<MTLTexture> mPointShadowColor{nil};
        uint32_t mFrameIndex{0};
        int32_t mViewportOriginX{0};
        int32_t mViewportOriginY{0};
        uint32_t mViewportWidth{0};
        uint32_t mViewportHeight{0};
        uint32_t mCaptureWidth{0};
        uint32_t mCaptureHeight{0};
        size_t mCaptureBytesPerRow{0};
        bool mFrameBegun{false};
        bool mCaptureRequested{false};
        bool mMainPassOpened{false};
        bool mInPointShadowPass{false};
    };

    MetalWindow::MetalWindow ( void* aDevice, void* aCommandQueue,
                               const RendererSettings& aSettings, void* aWindowId ) :
        mImpl{std::make_unique<Impl> ( aDevice, aCommandQueue, aSettings, aWindowId ) }
    {
    }

    MetalWindow::~MetalWindow() = default;

    void MetalWindow::SetProjectionMatrix ( const Matrix4x4& aMatrix )
    {
        mImpl->SetProjectionMatrix ( aMatrix );
    }

    void MetalWindow::SetViewMatrix ( const Matrix4x4& aMatrix )
    {
        mImpl->SetViewMatrix ( aMatrix );
    }

    void MetalWindow::SetLights ( std::span<const GpuLight> aLights )
    {
        mImpl->SetLights ( aLights );
    }

    void MetalWindow::SetGlobals ( const GpuGlobals& aGlobals )
    {
        mImpl->SetGlobals ( aGlobals );
    }

    void MetalWindow::SetSpotShadowParams ( const GpuSpotShadowParams& aSpotShadowParams )
    {
        mImpl->SetSpotShadowParams ( aSpotShadowParams );
    }

    void MetalWindow::SetPointShadowParams ( const GpuPointShadowParams& aPointShadowParams )
    {
        mImpl->SetPointShadowParams ( aPointShadowParams );
    }

    void MetalWindow::SetEnvironmentMap ( const Texture* aEnvironmentMap )
    {
        mImpl->SetEnvironmentMap ( aEnvironmentMap );
    }

    void MetalWindow::SetClearColor ( float aRed, float aGreen, float aBlue, float aAlpha )
    {
        mImpl->SetClearColor ( aRed, aGreen, aBlue, aAlpha );
    }

    void MetalWindow::ResizeViewport ( int32_t aX, int32_t aY, uint32_t aWidth, uint32_t aHeight )
    {
        mImpl->ResizeViewport ( aX, aY, aWidth, aHeight );
    }

    void MetalWindow::BeginFrame()
    {
        mImpl->BeginFrame();
    }

    void MetalWindow::PrepareClusters()
    {
        mImpl->PrepareClusters();
    }

    void MetalWindow::BeginRenderPass()
    {
        mImpl->BeginRenderPass();
    }

    void MetalWindow::BeginShadowPass ( const Matrix4x4& aLightViewProjection )
    {
        mImpl->BeginShadowPass ( aLightViewProjection );
    }

    void MetalWindow::EndShadowPass()
    {
        mImpl->EndShadowPass();
    }

    void MetalWindow::BeginSpotShadowPass ( uint32_t aSlot, const Matrix4x4& aLightViewProjection )
    {
        mImpl->BeginSpotShadowPass ( aSlot, aLightViewProjection );
    }

    void MetalWindow::EndSpotShadowPass()
    {
        mImpl->EndSpotShadowPass();
    }

    void MetalWindow::BeginPointShadowPass ( uint32_t aCaster )
    {
        mImpl->BeginPointShadowPass ( aCaster );
    }

    void MetalWindow::EndPointShadowPass()
    {
        mImpl->EndPointShadowPass();
    }

    void MetalWindow::ResolveFrame ( MetalPipeline& aSkyboxPipeline, MetalPipeline& aTonemapPipeline )
    {
        mImpl->ResolveFrame ( aSkyboxPipeline, aTonemapPipeline );
    }

    void MetalWindow::EndDepthPrePass()
    {
        mImpl->EndDepthPrePass();
    }

    void MetalWindow::BuildHiZ ( MetalPipeline& aPipeline )
    {
        mImpl->BuildHiZ ( aPipeline );
    }

    void MetalWindow::RenderOverlay ( void* aPipeline, void* aSampler, void* aDepthState, const uint8_t* aPixels,
                                      uint32_t aWidth, uint32_t aHeight )
    {
        mImpl->RenderOverlay ( aPipeline, aSampler, aDepthState, aPixels, aWidth, aHeight );
    }

    void MetalWindow::EndRender()
    {
        mImpl->EndRender();
    }

    void MetalWindow::Finish() const
    {
        mImpl->Finish();
    }

    void MetalWindow::RequestCapture()
    {
        mImpl->RequestCapture();
    }

    bool MetalWindow::ReadPixels ( Texture& aTexture ) const
    {
        return mImpl->ReadPixels ( aTexture );
    }

    const Frustum& MetalWindow::GetFrustum() const
    {
        return mImpl->GetFrustum();
    }

    const Matrix4x4& MetalWindow::GetProjectionMatrix() const
    {
        return mImpl->GetProjectionMatrix();
    }

    void* MetalWindow::GetCommandBuffer()
    {
        return mImpl->GetCommandBuffer();
    }

    void* MetalWindow::GetRenderEncoder()
    {
        return mImpl->GetRenderEncoder();
    }

    MetalArgumentBufferPool& MetalWindow::GetArgumentBufferPool()
    {
        return *mImpl;
    }

    BufferAccessor MetalWindow::AllocateUniform ( size_t aSize )
    {
        return mImpl->AllocateUniform ( aSize );
    }

    BufferAccessor MetalWindow::AllocateStorage ( size_t aSize )
    {
        return mImpl->AllocateStorage ( aSize );
    }

    const BufferAccessor& MetalWindow::GetClusterParams()
    {
        return mImpl->GetClusterParams();
    }

    const BufferAccessor& MetalWindow::GetMatrices()
    {
        return mImpl->GetMatrices();
    }

    const BufferAccessor& MetalWindow::GetLights()
    {
        return mImpl->GetLights();
    }

    const BufferAccessor& MetalWindow::GetGlobals()
    {
        return mImpl->GetGlobals();
    }

    const BufferAccessor& MetalWindow::GetShadowParams()
    {
        return mImpl->GetShadowParams();
    }
    const BufferAccessor& MetalWindow::GetSpotShadowParams()
    {
        return mImpl->GetSpotShadowParams();
    }
    const BufferAccessor& MetalWindow::GetPointShadowParams()
    {
        return mImpl->GetPointShadowParams();
    }
    const BufferAccessor& MetalWindow::GetShadowDepthParams()
    {
        return mImpl->GetShadowDepthParams();
    }
    const BufferAccessor& MetalWindow::GetPointShadowDepthParams()
    {
        return mImpl->GetPointShadowDepthParams();
    }

    const BufferAccessor& MetalWindow::GetClusterAABBs() const
    {
        return mImpl->GetClusterAABBs();
    }
    const BufferAccessor& MetalWindow::GetLightGrid() const
    {
        return mImpl->GetLightGrid();
    }
    const BufferAccessor& MetalWindow::GetLightIndexList() const
    {
        return mImpl->GetLightIndexList();
    }
    const BufferAccessor& MetalWindow::GetLightIndexCounter() const
    {
        return mImpl->GetLightIndexCounter();
    }
    const BufferAccessor& MetalWindow::GetClusterActive() const
    {
        return mImpl->GetClusterActive();
    }
    const BufferAccessor* MetalWindow::GetFrameLightGrid() const
    {
        return mImpl->GetFrameLightGrid();
    }
    const BufferAccessor* MetalWindow::GetFrameClusterActive() const
    {
        return mImpl->GetFrameClusterActive();
    }
    void* MetalWindow::GetShadowMap() const
    {
        return mImpl->GetShadowMap();
    }
    void* MetalWindow::GetSpotShadowMap() const
    {
        return mImpl->GetSpotShadowMap();
    }
    void* MetalWindow::GetPointShadowMap() const
    {
        return mImpl->GetPointShadowMap();
    }
    void* MetalWindow::GetShadowSampler() const
    {
        return mImpl->GetShadowSampler();
    }
    void* MetalWindow::GetHiZTexture() const
    {
        return mImpl->GetHiZTexture();
    }
    void* MetalWindow::GetLinearSampler() const
    {
        return mImpl->GetLinearSampler();
    }
    void* MetalWindow::GetHiZSampler() const
    {
        return mImpl->GetHiZSampler();
    }
    bool MetalWindow::HasHiZ() const
    {
        return mImpl->HasHiZ();
    }
    bool MetalWindow::IsPointShadowPass() const
    {
        return mImpl->IsPointShadowPass();
    }
}