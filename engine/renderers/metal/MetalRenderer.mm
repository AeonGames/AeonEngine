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
#import <Metal/Metal.h>

#include <array>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <vector>
#include "aeongames/AABB.hpp"
#include "aeongames/GpuClusterParams.hpp"
#include "aeongames/GuiOverlay.hpp"
#include "aeongames/LogLevel.hpp"
#include "aeongames/Material.hpp"
#include "aeongames/Matrix4x4.hpp"
#include "aeongames/MemoryPoolBuffer.hpp"
#include "aeongames/Mesh.hpp"
#include "aeongames/Pipeline.hpp"
#include "aeongames/ResourceId.hpp"
#include "aeongames/Scene.hpp"
#include "aeongames/Texture.hpp"
#include "MetalBindlessResources.h"
#include "MetalMaterial.h"
#include "MetalMesh.h"
#include "MetalPipeline.h"
#include "MetalRenderer.h"
#include "MetalTexture.h"
#include "MetalWindow.h"

namespace AeonGames
{
    namespace
    {
        struct MetalGpuCullInstance
        {
            Matrix4x4 mModel;
            float mCenter[4];
            float mRadii[4];
            uint32_t mDraw[4];
        };
        static_assert ( sizeof ( MetalGpuCullInstance ) == 112 );

        /** Every shading draw binds the same eleven per-frame blocks plus, at
         *  most, one material uniform block, so the binding list lives on the
         *  stack instead of allocating a vector per draw call. */
        constexpr size_t kMaxDrawBindings = 12;

        constexpr char kOverlayShaderSource[] = R"(
#include <metal_stdlib>
using namespace metal;

struct OverlayVertexOut
{
    float4 position [[position]];
    float2 texCoords;
};

vertex OverlayVertexOut OverlayVertex ( uint vertexId [[vertex_id]] )
{
    constexpr float2 positions[] =
    {
        float2 ( -1.0,  1.0 ),
        float2 ( -1.0, -1.0 ),
        float2 (  1.0,  1.0 ),
        float2 (  1.0, -1.0 )
    };
    // The window renders through a Y-flipped viewport so that Metal matches the
    // Vulkan +Y-down clip space every generated shader is authored against.
    // This quad is emitted directly in clip space, so it pairs the first overlay
    // row with NDC y = -1, which the flipped viewport puts at the top row of the
    // render target.
    constexpr float2 texCoords[] =
    {
        float2 ( 0.0, 1.0 ),
        float2 ( 0.0, 0.0 ),
        float2 ( 1.0, 1.0 ),
        float2 ( 1.0, 0.0 )
    };
    OverlayVertexOut result;
    result.position = float4 ( positions[vertexId], 0.0, 1.0 );
    result.texCoords = texCoords[vertexId];
    return result;
}

fragment float4 OverlayFragment ( OverlayVertexOut input [[stage_in]],
                                  texture2d<float> overlay [[texture(0)]],
                                  sampler overlaySampler [[sampler(0)]] )
{
    return overlay.sample ( overlaySampler, input.texCoords );
}
)";

    }

    class MetalRenderer::Impl
    {
    public:
        explicit Impl ( const RendererSettings& aSettings ) : mSettings{aSettings}
        {
            if ( !@available ( macOS 13.0, * ) )
            {
                throw std::runtime_error ( "MetalRenderer requires macOS 13 or newer" );
            }
            mDevice = MTLCreateSystemDefaultDevice();
            if ( mDevice == nil )
            {
                throw std::runtime_error ( "MetalRenderer could not find a Metal device" );
            }
            if ( ![mDevice supportsFamily:MTLGPUFamilyApple7] )
            {
                throw std::runtime_error ( "MetalRenderer requires an Apple Silicon GPU" );
            }
            if ( mDevice.argumentBuffersSupport != MTLArgumentBuffersTier2 )
            {
                throw std::runtime_error ( "MetalRenderer requires argument-buffer tier 2" );
            }
            mCommandQueue = [mDevice newCommandQueue];
            if ( mCommandQueue == nil )
            {
                throw std::runtime_error ( "MetalRenderer could not create a command queue" );
            }
            std::cout << LogLevel::Info << "MetalRenderer device: " << mDevice.name.UTF8String << std::endl;
            mBindless = std::make_unique<MetalBindlessResources> ( ( __bridge void* ) mDevice, mSettings );
            InitializeOverlay();
        }

        void InitializeOverlay()
        {
            MTLCompileOptions* options = [MTLCompileOptions new];
            options.languageVersion = MTLLanguageVersion3_0;
            NSError* error = nil;
            id<MTLLibrary> library = [mDevice
                                      newLibraryWithSource:[NSString stringWithUTF8String:kOverlayShaderSource]
                                      options:options error:&error];
            if ( library == nil )
            {
                throw std::runtime_error ( "MetalRenderer GUI overlay shader compilation failed: " +
                                           std::string {error.localizedDescription.UTF8String} );
            }
            id<MTLFunction> vertex = [library newFunctionWithName:@"OverlayVertex"];
            id<MTLFunction> fragment = [library newFunctionWithName:@"OverlayFragment"];
            MTLRenderPipelineDescriptor* descriptor = [MTLRenderPipelineDescriptor new];
            descriptor.label = @"AeonEngine GUI overlay";
            descriptor.vertexFunction = vertex;
            descriptor.fragmentFunction = fragment;
            descriptor.colorAttachments[0].pixelFormat = MTLPixelFormatRGBA16Float;
            descriptor.colorAttachments[0].blendingEnabled = YES;
            descriptor.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorOne;
            descriptor.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
            descriptor.colorAttachments[0].rgbBlendOperation = MTLBlendOperationAdd;
            descriptor.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactorOne;
            descriptor.colorAttachments[0].destinationAlphaBlendFactor = MTLBlendFactorZero;
            descriptor.colorAttachments[0].alphaBlendOperation = MTLBlendOperationAdd;
            descriptor.colorAttachments[1].pixelFormat = MTLPixelFormatRGBA16Float;
            descriptor.colorAttachments[1].writeMask = MTLColorWriteMaskNone;
            descriptor.colorAttachments[2].pixelFormat = MTLPixelFormatRGBA16Float;
            descriptor.colorAttachments[2].writeMask = MTLColorWriteMaskNone;
            descriptor.depthAttachmentPixelFormat = MTLPixelFormatDepth32Float;
            mOverlayPipeline = [mDevice newRenderPipelineStateWithDescriptor:descriptor error:&error];
            if ( mOverlayPipeline == nil )
            {
                throw std::runtime_error ( "MetalRenderer GUI overlay pipeline creation failed: " +
                                           std::string {error.localizedDescription.UTF8String} );
            }
            MTLSamplerDescriptor* sampler = [MTLSamplerDescriptor new];
            sampler.minFilter = MTLSamplerMinMagFilterNearest;
            sampler.magFilter = MTLSamplerMinMagFilterNearest;
            sampler.sAddressMode = MTLSamplerAddressModeClampToEdge;
            sampler.tAddressMode = MTLSamplerAddressModeClampToEdge;
            mOverlaySampler = [mDevice newSamplerStateWithDescriptor:sampler];
            if ( mOverlaySampler == nil )
            {
                throw std::runtime_error ( "MetalRenderer failed to create the GUI overlay sampler" );
            }
            // The overlay shares the scene's render encoder, so it must not
            // inherit the depth state of whatever drew last: it is a composite
            // over the finished frame and always passes.
            MTLDepthStencilDescriptor* depth = [MTLDepthStencilDescriptor new];
            depth.depthCompareFunction = MTLCompareFunctionAlways;
            depth.depthWriteEnabled = NO;
            mOverlayDepthState = [mDevice newDepthStencilStateWithDescriptor:depth];
            if ( mOverlayDepthState == nil )
            {
                throw std::runtime_error ( "MetalRenderer failed to create the GUI overlay depth state" );
            }
        }

        MetalWindow* FindWindow ( void* aWindowId ) const
        {
            auto it = mWindows.find ( aWindowId );
            return it != mWindows.end() ? it->second.get() : nullptr;
        }

        std::array<const MetalTexture*, kMaterialSamplerSlots.size() > ResolveMaterialTextures (
                    MetalRenderer& aRenderer, const Material& aMaterial )
        {
            std::array<const MetalTexture*, kMaterialSamplerSlots.size() > textures{};
            for ( uint32_t slot = 0; slot < kMaterialSamplerSlots.size(); ++slot )
            {
                const uint32_t slot_hash = crc32i ( kMaterialSamplerSlots[slot].name,
                                                    std::strlen ( kMaterialSamplerSlots[slot].name ) );
                ResourceId texture_id { "Texture"_crc32, kMaterialSamplerSlots[slot].fallback_path };
                for ( const Material::SamplerKeyValue& sampler : aMaterial.GetSamplers() )
                {
                    if ( std::get<0> ( sampler ) == slot_hash )
                    {
                        texture_id = std::get<1> ( sampler );
                        break;
                    }
                }
                Texture* texture = texture_id.Get<Texture>();
                if ( texture == nullptr )
                {
                    throw std::runtime_error ( "Metal material texture could not be loaded: " + texture_id.GetPathString() );
                }
                aRenderer.LoadTexture ( *texture );
                textures[slot] = mTextures.at ( texture->GetConsecutiveId() ).get();
            }
            return textures;
        }

        void InitializeDefaultMaterial ( MetalRenderer& aRenderer )
        {
            Material material;
            mDefaultMaterial = std::make_unique<MetalMaterial> (
                                   *mBindless, material, ResolveMaterialTextures ( aRenderer, material ) );
        }

        void EnsureClusterMarkPipeline()
        {
            if ( mClusterMark != nullptr )
            {
                return;
            }
            mClusterMarkAsset.LoadFromFile ( "shaders/cluster_mark" );
            mClusterMark = std::make_unique<MetalPipeline> ( ( __bridge void* ) mDevice, mClusterMarkAsset );
        }

        void EnsureShadowPipelines()
        {
            if ( mShadowDepth == nullptr )
            {
                mShadowDepthAsset.LoadFromFile ( "shaders/shadow_depth" );
                mShadowDepth = std::make_unique<MetalPipeline> ( ( __bridge void* ) mDevice, mShadowDepthAsset );
            }
            if ( mPointShadowDepth == nullptr )
            {
                mPointShadowDepthAsset.LoadFromFile ( "shaders/point_shadow_depth" );
                mPointShadowDepth = std::make_unique<MetalPipeline> ( ( __bridge void* ) mDevice, mPointShadowDepthAsset );
            }
        }

        void EnsurePresentationPipelines()
        {
            if ( mSkybox == nullptr )
            {
                mSkyboxAsset.LoadFromFile ( "shaders/skybox" );
                mSkybox = std::make_unique<MetalPipeline> ( ( __bridge void* ) mDevice, mSkyboxAsset );
            }
            if ( mTonemap == nullptr )
            {
                mTonemapAsset.LoadFromFile ( "shaders/tonemap" );
                mTonemap = std::make_unique<MetalPipeline> ( ( __bridge void* ) mDevice, mTonemapAsset );
            }
        }

        void EnsureHiZPipeline()
        {
            if ( mHiZ != nullptr )
            {
                return;
            }
            mHiZAsset.LoadFromFile ( "shaders/hiz_build" );
            mHiZ = std::make_unique<MetalPipeline> ( ( __bridge void* ) mDevice, mHiZAsset );
        }

        void EnsureCullPipeline()
        {
            if ( mCull != nullptr )
            {
                return;
            }
            mCullAsset.LoadFromFile ( "shaders/cull" );
            mCull = std::make_unique<MetalPipeline> ( ( __bridge void* ) mDevice, mCullAsset );
        }

        RendererSettings mSettings{};
        id<MTLDevice> mDevice{nil};
        id<MTLCommandQueue> mCommandQueue{nil};
        id<MTLRenderPipelineState> mOverlayPipeline{nil};
        id<MTLSamplerState> mOverlaySampler{nil};
        id<MTLDepthStencilState> mOverlayDepthState{nil};
        std::unique_ptr<MetalBindlessResources> mBindless{};
        std::unordered_map<size_t, std::unique_ptr<MetalTexture>> mTextures{};
        std::unordered_map<size_t, std::unique_ptr<MetalMaterial>> mMaterials{};
        std::unique_ptr<MetalMaterial> mDefaultMaterial{};
        std::unordered_map<size_t, std::unique_ptr<MetalMesh>> mMeshes{};
        std::unordered_map<size_t, std::unique_ptr<MetalPipeline>> mPipelines{};
        Pipeline mClusterMarkAsset{};
        std::unique_ptr<MetalPipeline> mClusterMark{};
        Pipeline mShadowDepthAsset{};
        std::unique_ptr<MetalPipeline> mShadowDepth{};
        Pipeline mPointShadowDepthAsset{};
        std::unique_ptr<MetalPipeline> mPointShadowDepth{};
        Pipeline mSkyboxAsset{};
        std::unique_ptr<MetalPipeline> mSkybox{};
        Pipeline mTonemapAsset{};
        std::unique_ptr<MetalPipeline> mTonemap{};
        Pipeline mHiZAsset{};
        std::unique_ptr<MetalPipeline> mHiZ{};
        Pipeline mCullAsset{};
        std::unique_ptr<MetalPipeline> mCull{};
        std::unordered_map<void*, std::unique_ptr<MetalWindow>> mWindows{};
    };

    MetalRenderer::MetalRenderer ( void* aWindow, const RendererSettings& aSettings ) :
        mImpl{std::make_unique<Impl> ( aSettings ) }
    {
        AttachWindow ( aWindow );
        mImpl->InitializeDefaultMaterial ( *this );
    }

    MetalRenderer::~MetalRenderer() = default;

    std::string_view MetalRenderer::GetName() const
    {
        return "Metal";
    }

    const RendererSettings& MetalRenderer::GetSettings() const
    {
        return mImpl->mSettings;
    }

    void MetalRenderer::LoadMesh ( const Mesh& aMesh )
    {
        const size_t id = aMesh.GetConsecutiveId();
        if ( mImpl->mMeshes.find ( id ) == mImpl->mMeshes.end() )
        {
            mImpl->mMeshes.emplace ( id, std::make_unique<MetalMesh> (
                                         ( __bridge void* ) mImpl->mDevice, aMesh ) );
        }
    }

    void MetalRenderer::UnloadMesh ( const Mesh& aMesh )
    {
        mImpl->mMeshes.erase ( aMesh.GetConsecutiveId() );
    }
    void MetalRenderer::LoadPipeline ( const Pipeline& aPipeline )
    {
        const size_t id = aPipeline.GetConsecutiveId();
        if ( mImpl->mPipelines.find ( id ) == mImpl->mPipelines.end() )
        {
            mImpl->mPipelines.emplace ( id, std::make_unique<MetalPipeline> (
                                            ( __bridge void* ) mImpl->mDevice, aPipeline ) );
        }
    }

    void MetalRenderer::UnloadPipeline ( const Pipeline& aPipeline )
    {
        mImpl->mPipelines.erase ( aPipeline.GetConsecutiveId() );
    }
    void MetalRenderer::LoadMaterial ( const Material& aMaterial )
    {
        const size_t id = aMaterial.GetConsecutiveId();
        if ( mImpl->mMaterials.find ( id ) == mImpl->mMaterials.end() )
        {
            mImpl->mMaterials.emplace ( id, std::make_unique<MetalMaterial> (
                                            *mImpl->mBindless, aMaterial,
                                            mImpl->ResolveMaterialTextures ( *this, aMaterial ) ) );
        }
    }

    void MetalRenderer::UnloadMaterial ( const Material& aMaterial )
    {
        mImpl->mMaterials.erase ( aMaterial.GetConsecutiveId() );
    }

    void MetalRenderer::LoadTexture ( const Texture& aTexture )
    {
        const size_t id = aTexture.GetConsecutiveId();
        if ( mImpl->mTextures.find ( id ) == mImpl->mTextures.end() )
        {
            mImpl->mTextures.emplace ( id, std::make_unique<MetalTexture> (
                                           ( __bridge void* ) mImpl->mDevice,
                                           ( __bridge void* ) mImpl->mCommandQueue, aTexture ) );
        }
    }

    void MetalRenderer::UnloadTexture ( const Texture& aTexture )
    {
        mImpl->mTextures.erase ( aTexture.GetConsecutiveId() );
    }

    void MetalRenderer::AttachWindow ( void* aWindowId )
    {
        if ( aWindowId == nullptr || mImpl->FindWindow ( aWindowId ) != nullptr )
        {
            return;
        }
        mImpl->mWindows.emplace ( aWindowId, std::make_unique<MetalWindow> (
                                      ( __bridge void* ) mImpl->mDevice,
                                      ( __bridge void* ) mImpl->mCommandQueue,
                                      mImpl->mSettings, aWindowId ) );
    }

    void MetalRenderer::DetachWindow ( void* aWindowId )
    {
        mImpl->mWindows.erase ( aWindowId );
    }

    void MetalRenderer::SetProjectionMatrix ( void* aWindowId, const Matrix4x4& aMatrix )
    {
        if ( MetalWindow * window = mImpl->FindWindow ( aWindowId ) ) window->SetProjectionMatrix ( aMatrix );
    }

    void MetalRenderer::SetViewMatrix ( void* aWindowId, const Matrix4x4& aMatrix )
    {
        if ( MetalWindow * window = mImpl->FindWindow ( aWindowId ) ) window->SetViewMatrix ( aMatrix );
    }

    void MetalRenderer::SetLights ( void* aWindowId, std::span<const GpuLight> aLights )
    {
        if ( MetalWindow * window = mImpl->FindWindow ( aWindowId ) )
        {
            window->SetLights ( FilterLightsByType ( aLights ) );
        }
    }

    void MetalRenderer::SetGlobals ( void* aWindowId, const GpuGlobals& aGlobals )
    {
        if ( MetalWindow * window = mImpl->FindWindow ( aWindowId ) ) window->SetGlobals ( aGlobals );
    }
    void MetalRenderer::SetEnvironmentMap ( void* aWindowId, const Texture* aEnvironmentMap )
    {
        if ( MetalWindow * window = mImpl->FindWindow ( aWindowId ) ) window->SetEnvironmentMap ( aEnvironmentMap );
    }

    void MetalRenderer::SetClearColor ( void* aWindowId, float aRed, float aGreen, float aBlue, float aAlpha )
    {
        if ( MetalWindow * window = mImpl->FindWindow ( aWindowId ) ) window->SetClearColor ( aRed, aGreen, aBlue, aAlpha );
    }

    void MetalRenderer::ResizeViewport ( void* aWindowId, int32_t aX, int32_t aY, uint32_t aWidth, uint32_t aHeight )
    {
        if ( MetalWindow * window = mImpl->FindWindow ( aWindowId ) ) window->ResizeViewport ( aX, aY, aWidth, aHeight );
    }

    void MetalRenderer::BeginRender ( void* aWindowId, const Pipeline* aComputePipeline )
    {
        MetalWindow* window = mImpl->FindWindow ( aWindowId );
        if ( window == nullptr )
        {
            return;
        }
        window->BeginFrame();
        if ( aComputePipeline != nullptr )
        {
            LoadPipeline ( *aComputePipeline );
            mImpl->EnsureClusterMarkPipeline();
            window->PrepareClusters();
            const StorageBufferBinding bindings[]
            {
                { Mesh::BindingLocations::CLUSTER_AABBS, &window->GetClusterAABBs() },
                { Mesh::BindingLocations::LIGHT_GRID, &window->GetLightGrid() },
                { Mesh::BindingLocations::LIGHT_INDEX_LIST, &window->GetLightIndexList() },
                { Mesh::BindingLocations::LIGHT_INDEX_COUNTER, &window->GetLightIndexCounter() },
                { Mesh::BindingLocations::CLUSTER_ACTIVE, &window->GetClusterActive() },
            };
            constexpr uint32_t group_count = ( CLUSTER_COUNT + 63u ) / 64u;
            mImpl->mPipelines.at ( aComputePipeline->GetConsecutiveId() )->Dispatch (
                                 window->GetCommandBuffer(), window->GetArgumentBufferPool(), group_count, 1, 1, bindings, 0,
                                 &window->GetClusterParams(), &window->GetMatrices(), &window->GetLights() );
        }
        // The main pass is not opened here: it opens on the first draw that
        // needs it, so the shadow passes that follow do not have to tear down an
        // empty clear-and-store of the HDR attachments first.
    }

    void MetalRenderer::BeginFrame ( void* aWindowId )
    {
        if ( MetalWindow * window = mImpl->FindWindow ( aWindowId ) ) window->BeginFrame();
    }

    void MetalRenderer::BeginRenderPass ( void* aWindowId )
    {
        if ( MetalWindow * window = mImpl->FindWindow ( aWindowId ) ) window->BeginRenderPass();
    }

    void MetalRenderer::BeginShadowPass ( void* aWindowId, const Matrix4x4& aLightViewProjection )
    {
        mImpl->EnsureShadowPipelines();
        if ( MetalWindow * window = mImpl->FindWindow ( aWindowId ) ) window->BeginShadowPass ( aLightViewProjection );
    }
    void MetalRenderer::EndShadowPass ( void* aWindowId )
    {
        if ( MetalWindow * window = mImpl->FindWindow ( aWindowId ) ) window->EndShadowPass();
    }
    void MetalRenderer::SetSpotShadowParams ( void* aWindowId, const GpuSpotShadowParams& aSpotShadowParams )
    {
        if ( MetalWindow * window = mImpl->FindWindow ( aWindowId ) ) window->SetSpotShadowParams ( aSpotShadowParams );
    }
    void MetalRenderer::BeginSpotShadowPass ( void* aWindowId, uint32_t aSlot,
            const Matrix4x4& aLightViewProjection )
    {
        mImpl->EnsureShadowPipelines();
        if ( MetalWindow * window = mImpl->FindWindow ( aWindowId ) )
        {
            window->BeginSpotShadowPass ( aSlot, aLightViewProjection );
        }
    }
    void MetalRenderer::EndSpotShadowPass ( void* aWindowId )
    {
        if ( MetalWindow * window = mImpl->FindWindow ( aWindowId ) ) window->EndSpotShadowPass();
    }
    void MetalRenderer::SetPointShadowParams ( void* aWindowId, const GpuPointShadowParams& aPointShadowParams )
    {
        if ( MetalWindow * window = mImpl->FindWindow ( aWindowId ) ) window->SetPointShadowParams ( aPointShadowParams );
    }
    void MetalRenderer::BeginPointShadowPass ( void* aWindowId, uint32_t aCaster )
    {
        mImpl->EnsureShadowPipelines();
        if ( MetalWindow * window = mImpl->FindWindow ( aWindowId ) ) window->BeginPointShadowPass ( aCaster );
    }
    void MetalRenderer::EndPointShadowPass ( void* aWindowId )
    {
        if ( MetalWindow * window = mImpl->FindWindow ( aWindowId ) ) window->EndPointShadowPass();
    }
    void MetalRenderer::EndDepthPrePass ( void* aWindowId, const Pipeline* aComputePipeline )
    {
        MetalWindow* window = mImpl->FindWindow ( aWindowId );
        if ( window == nullptr )
        {
            return;
        }
        // The pre-pass encoder must close even without a light-culling pipeline,
        // otherwise the depth pre-pass keeps the render encoder open and the
        // shading pass reuses it with the pre-pass state still bound.
        window->EndDepthPrePass();
        if ( aComputePipeline == nullptr )
        {
            return;
        }
        mImpl->EnsureHiZPipeline();
        window->BuildHiZ ( *mImpl->mHiZ );
        const StorageBufferBinding bindings[]
        {
            { Mesh::BindingLocations::CLUSTER_AABBS, &window->GetClusterAABBs() },
            { Mesh::BindingLocations::LIGHT_GRID, &window->GetLightGrid() },
            { Mesh::BindingLocations::LIGHT_INDEX_LIST, &window->GetLightIndexList() },
            { Mesh::BindingLocations::LIGHT_INDEX_COUNTER, &window->GetLightIndexCounter() },
            { Mesh::BindingLocations::CLUSTER_ACTIVE, &window->GetClusterActive() },
        };
        constexpr uint32_t group_count = ( CLUSTER_COUNT + 63u ) / 64u;
        const uint32_t stage_count = aComputePipeline->GetComputeStageCount ( GetName() );
        for ( uint32_t stage = 1; stage < stage_count; ++stage )
        {
            mImpl->mPipelines.at ( aComputePipeline->GetConsecutiveId() )->Dispatch (
                                 window->GetCommandBuffer(), window->GetArgumentBufferPool(), group_count, 1, 1, bindings, stage,
                                 &window->GetClusterParams(), &window->GetMatrices(), &window->GetLights() );
        }
    }

    void MetalRenderer::EndRender ( void* aWindowId )
    {
        if ( MetalWindow * window = mImpl->FindWindow ( aWindowId ) )
        {
            mImpl->EnsurePresentationPipelines();
            window->ResolveFrame ( *mImpl->mSkybox, *mImpl->mTonemap );
            window->EndRender();
        }
    }

    void MetalRenderer::Finish ( void* aWindowId )
    {
        if ( MetalWindow * window = mImpl->FindWindow ( aWindowId ) ) window->Finish();
    }

    void MetalRenderer::RequestCapture ( void* aWindowId )
    {
        if ( MetalWindow * window = mImpl->FindWindow ( aWindowId ) ) window->RequestCapture();
    }

    bool MetalRenderer::ReadPixels ( void* aWindowId, Texture& aTexture ) const
    {
        MetalWindow* window = mImpl->FindWindow ( aWindowId );
        return window != nullptr && window->ReadPixels ( aTexture );
    }

    void MetalRenderer::Render ( void* aWindowId, const Matrix4x4& aModelMatrix,
                                 const Mesh& aMesh, const Pipeline& aPipeline, const Material* aMaterial, Topology aTopology,
                                 uint32_t aVertexStart, uint32_t aVertexCount,
                                 uint32_t aInstanceCount, uint32_t aFirstInstance,
                                 const BufferAccessor* aSkinnedVertices, RenderPass aRenderPass ) const
    {
        MetalWindow* window = mImpl->FindWindow ( aWindowId );
        if ( window == nullptr )
        {
            return;
        }
        MetalRenderer& renderer = const_cast<MetalRenderer&> ( *this );
        renderer.LoadMesh ( aMesh );
        renderer.LoadPipeline ( aPipeline );
        if ( aMaterial != nullptr )
        {
            renderer.LoadMaterial ( *aMaterial );
        }
        const auto mesh = mImpl->mMeshes.find ( aMesh.GetConsecutiveId() );
        const auto pipeline = mImpl->mPipelines.find ( aPipeline.GetConsecutiveId() );
        if ( mesh == mImpl->mMeshes.end() || pipeline == mImpl->mPipelines.end() )
        {
            throw std::runtime_error ( "MetalRenderer draw requires a known window, mesh and pipeline" );
        }
        BufferAccessor instance_matrices = window->AllocateStorage (
                                               static_cast<size_t> ( aInstanceCount ) * sizeof ( Matrix4x4 ) );
        std::uninitialized_fill_n ( static_cast<Matrix4x4*> ( instance_matrices.Map() ),
                                    aInstanceCount, aModelMatrix );
        instance_matrices.Unmap();
        if ( aRenderPass == RenderPass::ShadowPass )
        {
            mImpl->EnsureShadowPipelines();
            const BufferAccessor& shadow_params = window->IsPointShadowPass()
                                                  ? window->GetPointShadowDepthParams()
                                                  : window->GetShadowDepthParams();
            const MetalBufferBinding shadow_buffers[]
            {
                { "ShadowParams"_crc32, &shadow_params },
                { "InstanceMatrices"_crc32, &instance_matrices },
            };
            MetalPipeline& shadow_pipeline = window->IsPointShadowPass()
                                             ? *mImpl->mPointShadowDepth : *mImpl->mShadowDepth;
            shadow_pipeline.Draw ( window->GetRenderEncoder(), window->GetArgumentBufferPool(), *mesh->second, shadow_buffers, {},
                                   *mImpl->mBindless, nullptr, aTopology, aVertexStart, aVertexCount,
                                   aInstanceCount, aFirstInstance );
            return;
        }
        const uint32_t material_index = aMaterial != nullptr
                                        ? mImpl->mMaterials.at ( aMaterial->GetConsecutiveId() )->GetBindlessIndex()
                                        : mImpl->mDefaultMaterial->GetBindlessIndex();
        BufferAccessor instance_materials = window->AllocateStorage (
                                                static_cast<size_t> ( aInstanceCount ) * sizeof ( uint32_t ) );
        std::uninitialized_fill_n ( static_cast<uint32_t*> ( instance_materials.Map() ),
                                    aInstanceCount, material_index );
        instance_materials.Unmap();
        BufferAccessor material_uniform;
        std::array<MetalBufferBinding, kMaxDrawBindings> buffers
        {
            {
                { "Matrices"_crc32, &window->GetMatrices() },
                { "InstanceMatrices"_crc32, &instance_matrices },
                { "InstanceMaterials"_crc32, &instance_materials },
                { "ClusterParams"_crc32, &window->GetClusterParams() },
                { "Lights"_crc32, &window->GetLights() },
                { "Globals"_crc32, &window->GetGlobals() },
                { "ShadowParams"_crc32, &window->GetShadowParams() },
                { "SpotShadowParams"_crc32, &window->GetSpotShadowParams() },
                { "PointShadowParams"_crc32, &window->GetPointShadowParams() },
                { "LightGrid"_crc32, &window->GetLightGrid() },
                { "LightIndexList"_crc32, &window->GetLightIndexList() },
            }
        };
        size_t buffer_count = kMaxDrawBindings - 1;
        if ( aMaterial != nullptr && !aMaterial->GetUniformBuffer().empty() )
        {
            material_uniform = window->AllocateUniform ( aMaterial->GetUniformBuffer().size() );
            material_uniform.WriteMemory ( 0, aMaterial->GetUniformBuffer().size(),
                                           aMaterial->GetUniformBuffer().data() );
            buffers[buffer_count++] = { "Material"_crc32, &material_uniform };
        }
        const MetalTextureBinding textures[]
        {
            { "ShadowMap"_crc32, window->GetShadowMap(), window->GetShadowSampler() },
            { "SpotShadowMap"_crc32, window->GetSpotShadowMap(), window->GetShadowSampler() },
            { "PointShadowMap"_crc32, window->GetPointShadowMap(), window->GetShadowSampler() },
        };
        if ( aRenderPass == RenderPass::DepthPrePass )
        {
            mImpl->EnsureClusterMarkPipeline();
        }
        MetalPipeline& draw_pipeline = aRenderPass == RenderPass::DepthPrePass
                                       ? *mImpl->mClusterMark : *pipeline->second;
        draw_pipeline.Draw ( window->GetRenderEncoder(), window->GetArgumentBufferPool(), *mesh->second,
                             std::span<const MetalBufferBinding> {buffers.data(), buffer_count}, textures,
                             *mImpl->mBindless, aSkinnedVertices,
                             aTopology, aVertexStart, aVertexCount, aInstanceCount, aFirstInstance );
    }

    void MetalRenderer::RenderInstanced ( void* aWindowId, std::span<const Matrix4x4> aModelMatrices,
                                          const Mesh& aMesh, const Pipeline& aPipeline, const Material* aMaterial,
                                          Topology aTopology, uint32_t aVertexStart, uint32_t aVertexCount,
                                          RenderPass aRenderPass )
    {
        MetalWindow* window = mImpl->FindWindow ( aWindowId );
        if ( window == nullptr || aModelMatrices.empty() )
        {
            return;
        }
        LoadMesh ( aMesh );
        LoadPipeline ( aPipeline );
        if ( aMaterial != nullptr )
        {
            LoadMaterial ( *aMaterial );
        }
        const auto mesh = mImpl->mMeshes.find ( aMesh.GetConsecutiveId() );
        const auto pipeline = mImpl->mPipelines.find ( aPipeline.GetConsecutiveId() );
        const size_t byte_size = aModelMatrices.size_bytes();
        BufferAccessor instance_matrices = window->AllocateStorage ( byte_size );
        instance_matrices.WriteMemory ( 0, byte_size, aModelMatrices.data() );
        if ( aRenderPass == RenderPass::ShadowPass )
        {
            mImpl->EnsureShadowPipelines();
            const BufferAccessor& shadow_params = window->IsPointShadowPass()
                                                  ? window->GetPointShadowDepthParams()
                                                  : window->GetShadowDepthParams();
            const MetalBufferBinding shadow_buffers[]
            {
                { "ShadowParams"_crc32, &shadow_params },
                { "InstanceMatrices"_crc32, &instance_matrices },
            };
            MetalPipeline& shadow_pipeline = window->IsPointShadowPass()
                                             ? *mImpl->mPointShadowDepth : *mImpl->mShadowDepth;
            shadow_pipeline.Draw ( window->GetRenderEncoder(), window->GetArgumentBufferPool(), *mesh->second, shadow_buffers, {},
                                   *mImpl->mBindless, nullptr, aTopology, aVertexStart, aVertexCount,
                                   static_cast<uint32_t> ( aModelMatrices.size() ), 0 );
            return;
        }
        const uint32_t material_index = aMaterial != nullptr
                                        ? mImpl->mMaterials.at ( aMaterial->GetConsecutiveId() )->GetBindlessIndex()
                                        : mImpl->mDefaultMaterial->GetBindlessIndex();
        BufferAccessor instance_materials = window->AllocateStorage ( aModelMatrices.size() * sizeof ( uint32_t ) );
        std::uninitialized_fill_n ( static_cast<uint32_t*> ( instance_materials.Map() ),
                                    aModelMatrices.size(), material_index );
        instance_materials.Unmap();
        BufferAccessor material_uniform;
        std::array<MetalBufferBinding, kMaxDrawBindings> buffers
        {
            {
                { "Matrices"_crc32, &window->GetMatrices() },
                { "InstanceMatrices"_crc32, &instance_matrices },
                { "InstanceMaterials"_crc32, &instance_materials },
                { "ClusterParams"_crc32, &window->GetClusterParams() },
                { "Lights"_crc32, &window->GetLights() },
                { "Globals"_crc32, &window->GetGlobals() },
                { "ShadowParams"_crc32, &window->GetShadowParams() },
                { "SpotShadowParams"_crc32, &window->GetSpotShadowParams() },
                { "PointShadowParams"_crc32, &window->GetPointShadowParams() },
                { "LightGrid"_crc32, &window->GetLightGrid() },
                { "LightIndexList"_crc32, &window->GetLightIndexList() },
            }
        };
        size_t buffer_count = kMaxDrawBindings - 1;
        if ( aMaterial != nullptr && !aMaterial->GetUniformBuffer().empty() )
        {
            material_uniform = window->AllocateUniform ( aMaterial->GetUniformBuffer().size() );
            material_uniform.WriteMemory ( 0, aMaterial->GetUniformBuffer().size(),
                                           aMaterial->GetUniformBuffer().data() );
            buffers[buffer_count++] = { "Material"_crc32, &material_uniform };
        }
        const MetalTextureBinding textures[]
        {
            { "ShadowMap"_crc32, window->GetShadowMap(), window->GetShadowSampler() },
            { "SpotShadowMap"_crc32, window->GetSpotShadowMap(), window->GetShadowSampler() },
            { "PointShadowMap"_crc32, window->GetPointShadowMap(), window->GetShadowSampler() },
        };
        if ( aRenderPass == RenderPass::DepthPrePass )
        {
            mImpl->EnsureClusterMarkPipeline();
        }
        MetalPipeline& draw_pipeline = aRenderPass == RenderPass::DepthPrePass
                                       ? *mImpl->mClusterMark : *pipeline->second;
        draw_pipeline.Draw ( window->GetRenderEncoder(), window->GetArgumentBufferPool(), *mesh->second,
                             std::span<const MetalBufferBinding> {buffers.data(), buffer_count}, textures,
                             *mImpl->mBindless, nullptr,
                             aTopology, aVertexStart, aVertexCount,
                             static_cast<uint32_t> ( aModelMatrices.size() ), 0 );
    }

    void MetalRenderer::Dispatch ( void* aWindowId, const Pipeline& aPipeline,
                                   uint32_t aGroupCountX, uint32_t aGroupCountY, uint32_t aGroupCountZ,
                                   std::span<const StorageBufferBinding> aStorageBuffers,
                                   uint32_t aComputeStageIndex ) const
    {
        MetalWindow* window = mImpl->FindWindow ( aWindowId );
        const auto pipeline = mImpl->mPipelines.find ( aPipeline.GetConsecutiveId() );
        if ( window == nullptr || pipeline == mImpl->mPipelines.end() )
        {
            throw std::runtime_error ( "MetalRenderer dispatch requires a known window and loaded pipeline" );
        }
        void* command_buffer = window->GetCommandBuffer();
        if ( command_buffer == nullptr )
        {
            return;
        }
        pipeline->second->Dispatch ( command_buffer, window->GetArgumentBufferPool(), aGroupCountX, aGroupCountY, aGroupCountZ,
                                     aStorageBuffers, aComputeStageIndex,
                                     &window->GetClusterParams(), &window->GetMatrices(), &window->GetLights() );
    }

    void MetalRenderer::Skin ( void* aWindowId, const Pipeline& aPipeline, const Mesh& aMesh,
                               const BufferAccessor& aSkinningMatrices,
                               const BufferAccessor& aSkinnedVertices ) const
    {
        MetalWindow* window = mImpl->FindWindow ( aWindowId );
        if ( window == nullptr )
        {
            return;
        }
        MetalRenderer& renderer = const_cast<MetalRenderer&> ( *this );
        renderer.LoadMesh ( aMesh );
        renderer.LoadPipeline ( aPipeline );
        const auto mesh = mImpl->mMeshes.find ( aMesh.GetConsecutiveId() );
        const auto pipeline = mImpl->mPipelines.find ( aPipeline.GetConsecutiveId() );
        if ( mesh == mImpl->mMeshes.end() || pipeline == mImpl->mPipelines.end() )
        {
            throw std::runtime_error ( "MetalRenderer Skin requires a known window, mesh and pipeline" );
        }
        const StorageBufferBinding bindings[]
        {
            { Mesh::BindingLocations::SKINNING_MATRICES, &aSkinningMatrices },
            { Mesh::BindingLocations::SKINNED_VERTICES, &aSkinnedVertices },
        };
        const uint32_t groups = ( aMesh.GetVertexCount() + 63u ) / 64u;
        pipeline->second->Dispatch ( window->GetCommandBuffer(), window->GetArgumentBufferPool(), groups, 1, 1, bindings, 0,
                                     nullptr, nullptr, nullptr, mesh->second.get() );
    }

    void MetalRenderer::Barrier ( void* ) const {}

    const Frustum& MetalRenderer::GetFrustum ( void* aWindowId ) const
    {
        if ( MetalWindow * window = mImpl->FindWindow ( aWindowId ) ) return window->GetFrustum();
        throw std::runtime_error ( "MetalRenderer: unknown window in GetFrustum" );
    }

    const Matrix4x4& MetalRenderer::GetProjectionMatrix ( void* aWindowId ) const
    {
        if ( MetalWindow * window = mImpl->FindWindow ( aWindowId ) ) return window->GetProjectionMatrix();
        throw std::runtime_error ( "MetalRenderer: unknown window in GetProjectionMatrix" );
    }

    const BufferAccessor* MetalRenderer::GetFrameLightGrid ( void* aWindowId ) const
    {
        MetalWindow* window = mImpl->FindWindow ( aWindowId );
        return window != nullptr ? window->GetFrameLightGrid() : nullptr;
    }
    const BufferAccessor* MetalRenderer::GetFrameClusterActive ( void* aWindowId ) const
    {
        MetalWindow* window = mImpl->FindWindow ( aWindowId );
        return window != nullptr ? window->GetFrameClusterActive() : nullptr;
    }
    BufferAccessor MetalRenderer::AllocateSingleFrameUniformMemory ( void* aWindowId, size_t aSize )
    {
        MetalWindow* window = mImpl->FindWindow ( aWindowId );
        return window != nullptr ? window->AllocateUniform ( aSize ) : BufferAccessor{};
    }

    BufferAccessor MetalRenderer::AllocateSingleFrameStorageMemory ( void* aWindowId, size_t aSize )
    {
        MetalWindow* window = mImpl->FindWindow ( aWindowId );
        return window != nullptr ? window->AllocateStorage ( aSize ) : BufferAccessor{};
    }
    void MetalRenderer::RenderOverlay ( void* aWindowId, const GuiOverlay& aGuiOverlay )
    {
        const uint8_t* pixels = aGuiOverlay.GetPixels();
        const uint32_t width = aGuiOverlay.GetWidth();
        const uint32_t height = aGuiOverlay.GetHeight();
        MetalWindow* window = mImpl->FindWindow ( aWindowId );
        if ( window == nullptr || pixels == nullptr || width == 0 || height == 0 )
        {
            return;
        }
        window->RenderOverlay ( ( __bridge void* ) mImpl->mOverlayPipeline,
                                ( __bridge void* ) mImpl->mOverlaySampler,
                                ( __bridge void* ) mImpl->mOverlayDepthState,
                                pixels, width, height );
    }
    void MetalRenderer::SubmitRenderQueue ( void* aWindowId, const Scene& aScene, RenderPass aRenderPass )
    {
        MetalWindow* window = mImpl->FindWindow ( aWindowId );
        if ( window == nullptr || aRenderPass != RenderPass::Shading || !window->HasHiZ() )
        {
            aScene.SubmitRenderQueue ( *this, aWindowId, aRenderPass );
            return;
        }

        struct CulledBatch
        {
            const Mesh* mMesh;
            MetalMesh* mMetalMesh;
            MetalPipeline* mPipeline;
            const Material* mMaterial;
            BufferAccessor mCommands;
            BufferAccessor mModels;
            BufferAccessor mMaterials;
            uint32_t mMaxDraws;
        };
        std::vector<CulledBatch> culled_batches;
        mImpl->EnsureCullPipeline();

        auto eligible = [] ( std::span<const RenderItem> aBatch )
        {
            return !aBatch.empty() && aBatch.front().mSkinnedVertices == nullptr &&
                   aBatch.front().mMesh->GetIndexCount() != 0;
        };
        aScene.ForEachRenderBatch ( [&] ( std::span<const RenderItem> aBatch )
        {
            if ( !eligible ( aBatch ) )
            {
                return;
            }
            const RenderItem& head = aBatch.front();
            LoadMesh ( *head.mMesh );
            LoadPipeline ( *head.mPipeline );
            if ( head.mMaterial != nullptr )
            {
                LoadMaterial ( *head.mMaterial );
            }
            const uint32_t material_index = head.mMaterial != nullptr
                                            ? mImpl->mMaterials.at ( head.mMaterial->GetConsecutiveId() )->GetBindlessIndex()
                                            : mImpl->mDefaultMaterial->GetBindlessIndex();
            const AABB& bounds = head.mMesh->GetAABB();
            const Vector3& center = bounds.GetCenter();
            const Vector3& radii = bounds.GetRadii();
            const uint32_t count = static_cast<uint32_t> ( aBatch.size() );
            BufferAccessor input = window->AllocateStorage (
                                       static_cast<size_t> ( count ) * sizeof ( MetalGpuCullInstance ) );
            // The cull input is built straight in the frame pool: it is written
            // once, read once by the compute pass, and never touched again.
            auto* instances = static_cast<MetalGpuCullInstance*> ( input.Map() );
            for ( uint32_t index = 0; index < count; ++index )
            {
                MetalGpuCullInstance& instance = instances[index];
                instance = MetalGpuCullInstance{};
                instance.mModel = aBatch[index].mTransform;
                instance.mCenter[0] = center.GetX();
                instance.mCenter[1] = center.GetY();
                instance.mCenter[2] = center.GetZ();
                instance.mRadii[0] = radii.GetX();
                instance.mRadii[1] = radii.GetY();
                instance.mRadii[2] = radii.GetZ();
                instance.mDraw[0] = head.mMesh->GetIndexCount();
                instance.mDraw[3] = material_index;
            }
            input.Unmap();
            BufferAccessor commands = window->AllocateStorage ( static_cast<size_t> ( count ) * 5 * sizeof ( uint32_t ) );
            commands.WriteMemory ( 0, commands.GetSize() );
            BufferAccessor draw_count = window->AllocateStorage ( sizeof ( uint32_t ) );
            draw_count.WriteMemory ( 0, sizeof ( uint32_t ) );
            BufferAccessor models = window->AllocateStorage ( static_cast<size_t> ( count ) * sizeof ( Matrix4x4 ) );
            BufferAccessor materials = window->AllocateStorage ( static_cast<size_t> ( count ) * sizeof ( uint32_t ) );
            const StorageBufferBinding bindings[]
            {
                { Mesh::BindingLocations::CULL_INSTANCES, &input },
                { Mesh::BindingLocations::DRAW_COMMANDS, &commands },
                { Mesh::BindingLocations::DRAW_COUNT, &draw_count },
                { Mesh::BindingLocations::INSTANCE_MATRICES, &models },
                { Mesh::BindingLocations::INSTANCE_MATERIALS, &materials },
            };
            const MetalTextureBinding textures[]
            {
                { "HiZ"_crc32, window->GetHiZTexture(), window->GetHiZSampler() },
            };
            mImpl->mCull->Dispatch ( window->GetCommandBuffer(), window->GetArgumentBufferPool(), ( count + 63u ) / 64u, 1, 1,
                                     bindings, 0, &window->GetClusterParams(), &window->GetMatrices(),
                                     &window->GetLights(), nullptr, textures );
            culled_batches.push_back ( CulledBatch
            {
                head.mMesh,
                mImpl->mMeshes.at ( head.mMesh->GetConsecutiveId() ).get(),
                mImpl->mPipelines.at ( head.mPipeline->GetConsecutiveId() ).get(),
                head.mMaterial,
                commands,
                models,
                materials,
                count,
            } );
        } );

        for ( CulledBatch& batch : culled_batches )
        {
            BufferAccessor material_uniform;
            std::array<MetalBufferBinding, kMaxDrawBindings> buffers
            {
                {
                    { "Matrices"_crc32, &window->GetMatrices() },
                    { "InstanceMatrices"_crc32, &batch.mModels },
                    { "InstanceMaterials"_crc32, &batch.mMaterials },
                    { "ClusterParams"_crc32, &window->GetClusterParams() },
                    { "Lights"_crc32, &window->GetLights() },
                    { "Globals"_crc32, &window->GetGlobals() },
                    { "ShadowParams"_crc32, &window->GetShadowParams() },
                    { "SpotShadowParams"_crc32, &window->GetSpotShadowParams() },
                    { "PointShadowParams"_crc32, &window->GetPointShadowParams() },
                    { "LightGrid"_crc32, &window->GetLightGrid() },
                    { "LightIndexList"_crc32, &window->GetLightIndexList() },
                }
            };
            size_t buffer_count = kMaxDrawBindings - 1;
            if ( batch.mMaterial != nullptr && !batch.mMaterial->GetUniformBuffer().empty() )
            {
                material_uniform = window->AllocateUniform ( batch.mMaterial->GetUniformBuffer().size() );
                material_uniform.WriteMemory ( 0, batch.mMaterial->GetUniformBuffer().size(),
                                               batch.mMaterial->GetUniformBuffer().data() );
                buffers[buffer_count++] = { "Material"_crc32, &material_uniform };
            }
            const MetalTextureBinding textures[]
            {
                { "ShadowMap"_crc32, window->GetShadowMap(), window->GetShadowSampler() },
                { "SpotShadowMap"_crc32, window->GetSpotShadowMap(), window->GetShadowSampler() },
                { "PointShadowMap"_crc32, window->GetPointShadowMap(), window->GetShadowSampler() },
            };
            batch.mPipeline->Draw ( window->GetRenderEncoder(), window->GetArgumentBufferPool(), *batch.mMetalMesh,
                                    std::span<const MetalBufferBinding> {buffers.data(), buffer_count}, textures,
                                    *mImpl->mBindless, nullptr, Topology::TRIANGLE_LIST,
                                    0, 0xffffffff, 1, 0, &batch.mCommands, batch.mMaxDraws );
        }

        aScene.ForEachRenderBatch ( [&] ( std::span<const RenderItem> aBatch )
        {
            if ( aBatch.empty() || eligible ( aBatch ) )
            {
                return;
            }
            const RenderItem& head = aBatch.front();
            if ( aBatch.size() == 1 )
            {
                Render ( aWindowId, head.mTransform, *head.mMesh, *head.mPipeline, head.mMaterial,
                         Topology::TRIANGLE_LIST, 0, 0xffffffff, 1, 0,
                         head.mSkinnedVertices, aRenderPass );
                return;
            }
            std::vector<Matrix4x4> transforms;
            transforms.reserve ( aBatch.size() );
            for ( const RenderItem& item : aBatch )
            {
                transforms.push_back ( item.mTransform );
            }
            RenderInstanced ( aWindowId, transforms, *head.mMesh, *head.mPipeline,
                              head.mMaterial, Topology::TRIANGLE_LIST, 0, 0xffffffff, aRenderPass );
        } );
    }

    bool MetalRenderer::IsValidWindow ( void* aWindowId ) const
    {
        return mImpl->FindWindow ( aWindowId ) != nullptr;
    }
}