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

#include <algorithm>
#include <array>
#include <cstring>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>
#include "aeongames/BufferAccessor.hpp"
#include "aeongames/MemoryPoolBuffer.hpp"
#include "aeongames/Mesh.hpp"
#include "aeongames/Pipeline.hpp"
#include "MetalBindlessResources.h"
#include "MetalBuffer.h"
#include "MetalMesh.h"
#include "MetalPipeline.h"

namespace AeonGames
{
    namespace
    {
        /** Suballocate one argument buffer region from the frame pool. Metal
         *  argument buffers are written once and read for the rest of the frame,
         *  so a fresh MTLBuffer per draw would allocate thousands of objects a
         *  frame for no benefit. */
        id<MTLBuffer> AllocateArgumentBuffer ( MetalArgumentBufferPool& aPool, id<MTLArgumentEncoder> aEncoder,
                                               size_t& aOffset )
        {
            const MetalArgumentBufferPool::Allocation allocation =
                aPool.AllocateArgumentBuffer ( aEncoder.encodedLength, aEncoder.alignment );
            id<MTLBuffer> buffer = ( __bridge id<MTLBuffer> ) allocation.mBuffer;
            if ( buffer == nil )
            {
                throw std::runtime_error ( "Metal argument buffer allocation failed" );
            }
            aOffset = allocation.mOffset;
            return buffer;
        }
    }

    class MetalPipeline::Impl
    {
    public:
        Impl ( id<MTLDevice> aDevice, const Pipeline& aPipeline ) :
            mDevice{aDevice},
            mPipeline{&aPipeline}
        {
            const std::string_view vertex_source = aPipeline.GetShaderCode ( VERT, "Metal" );
            const std::string_view fragment_source = aPipeline.GetShaderCode ( FRAG, "Metal" );
            if ( !vertex_source.empty() || !fragment_source.empty() )
            {
                if ( vertex_source.empty() || fragment_source.empty() )
                {
                    throw std::runtime_error ( "Metal graphics pipeline requires vertex and fragment stages" );
                }
                if ( !aPipeline.GetShaderCode ( GEOM, "Metal" ).empty() ||
                     !aPipeline.GetShaderCode ( TESC, "Metal" ).empty() ||
                     !aPipeline.GetShaderCode ( TESE, "Metal" ).empty() )
                {
                    throw std::runtime_error ( "Metal geometry/tessellation stages are not implemented yet" );
                }
                mVertexInterface = aPipeline.GetShaderInterface ( VERT, "Metal" );
                mFragmentInterface = aPipeline.GetShaderInterface ( FRAG, "Metal" );
                if ( mVertexInterface == nullptr || mFragmentInterface == nullptr )
                {
                    throw std::runtime_error ( "Metal graphics pipeline is missing interface metadata" );
                }
                // The interface metadata is immutable, so the per-set masks and
                // the bindless usage flags are resolved once here instead of
                // rescanning every resource on every draw.
                for ( const ShaderResource& resource : mVertexInterface->resources )
                {
                    if ( resource.set < mVertexSets.size() )
                    {
                        mVertexSets[resource.set] = true;
                    }
                }
                for ( const ShaderResource& resource : mFragmentInterface->resources )
                {
                    if ( resource.set < mFragmentSets.size() )
                    {
                        mFragmentSets[resource.set] = true;
                    }
                    mUsesBindlessTextures = mUsesBindlessTextures ||
                                            ( resource.set == 2 && resource.type == ShaderResourceType::CombinedImageSampler );
                    mUsesBindlessMaterials = mUsesBindlessMaterials ||
                                             ( resource.set == 7 && resource.name == "Bindless" );
                }
                mVertexLibrary = CompileLibrary ( vertex_source );
                mFragmentLibrary = CompileLibrary ( fragment_source );
                mVertexFunction = [mVertexLibrary newFunctionWithName:[NSString stringWithUTF8String:mVertexInterface->entry_point.c_str()]];
                mFragmentFunction = [mFragmentLibrary newFunctionWithName:[NSString stringWithUTF8String:mFragmentInterface->entry_point.c_str()]];
                if ( mVertexFunction == nil || mFragmentFunction == nil )
                {
                    throw std::runtime_error ( "Metal graphics entry point was not found" );
                }
                if ( vertex_source.find ( "spvViewMask" ) != std::string_view::npos )
                {
                    const uint32_t view_mask[] {0, 6};
                    mMultiviewMask = [mDevice newBufferWithBytes:view_mask
                                      length:sizeof ( view_mask ) options:MTLResourceStorageModeShared];
                    mViewCount = 6;
                }
                MTLDepthStencilDescriptor* depth = [[MTLDepthStencilDescriptor alloc] init];
                const PipelineDepthStencilState& depth_state = aPipeline.GetDepthStencilState();
                const bool depth_test = depth_state.depth_test == PipelineToggle::ENABLED;
                depth.depthCompareFunction = depth_test ? ToCompareFunction ( depth_state.depth_compare ) : MTLCompareFunctionAlways;
                depth.depthWriteEnabled = depth_test && depth_state.depth_write == PipelineToggle::ENABLED;
                mDepthState = [mDevice newDepthStencilStateWithDescriptor:depth];
                MTLDepthStencilDescriptor* disabled_depth = [[MTLDepthStencilDescriptor alloc] init];
                disabled_depth.depthCompareFunction = MTLCompareFunctionAlways;
                disabled_depth.depthWriteEnabled = NO;
                mDisabledDepthState = [mDevice newDepthStencilStateWithDescriptor:disabled_depth];
            }
            const uint32_t stage_count = aPipeline.GetComputeStageCount ( "Metal" );
            mComputeStages.reserve ( stage_count );
            for ( uint32_t stage = 0; stage < stage_count; ++stage )
            {
                const std::string_view source = aPipeline.GetComputeShaderCode ( stage, "Metal" );
                const ShaderInterface* shader_interface = aPipeline.GetShaderInterface ( COMP, "Metal", stage );
                if ( source.empty() || shader_interface == nullptr )
                {
                    throw std::runtime_error ( "Metal compute pipeline is missing source or interface metadata" );
                }
                MTLCompileOptions* options = [[MTLCompileOptions alloc] init];
                options.languageVersion = MTLLanguageVersion3_0;
                NSError* error = nil;
                NSString* source_string = [[NSString alloc] initWithBytes:source.data()
                                           length:source.size()
                                           encoding:NSUTF8StringEncoding];
                id<MTLLibrary> library = [mDevice newLibraryWithSource:source_string options:options error:&error];
                if ( library == nil )
                {
                    throw std::runtime_error ( std::string {"Metal shader compilation failed: "} +
                                               ( error.localizedDescription.UTF8String ? : "unknown error" ) );
                }
                NSString* entry_name = [NSString stringWithUTF8String:shader_interface->entry_point.c_str()];
                id<MTLFunction> function = [library newFunctionWithName:entry_name];
                if ( function == nil )
                {
                    throw std::runtime_error ( "Metal compute entry point not found: " + shader_interface->entry_point );
                }
                MTLComputePipelineReflection* reflection = nil;
                id<MTLComputePipelineState> state = [mDevice newComputePipelineStateWithFunction:function
                                                     options:MTLPipelineOptionBindingInfo | MTLPipelineOptionBufferTypeInfo
                                                     reflection:&reflection
                                                     error:&error];
                if ( state == nil )
                {
                    throw std::runtime_error ( std::string {"Metal compute pipeline creation failed: "} +
                                               ( error.localizedDescription.UTF8String ? : "unknown error" ) );
                }
                ComputeStage compute_stage{library, function, state, *shader_interface, {}};
                compute_stage.buffer_size_argument.fill ( -1 );
                for ( id<MTLBinding> binding in reflection.bindings )
                {
                    if ( binding.type != MTLBindingTypeBuffer || binding.index >= compute_stage.buffer_size_argument.size() )
                    {
                        continue;
                    }
                    id<MTLBufferBinding> buffer_binding = ( id<MTLBufferBinding> ) binding;
                    MTLStructMember* size_member = [buffer_binding.bufferStructType memberByName:@"spvBufferSizeConstants"];
                    if ( size_member != nil )
                    {
                        compute_stage.buffer_size_argument[binding.index] = static_cast<int32_t> ( size_member.argumentIndex );
                    }
                }
                mComputeStages.push_back ( std::move ( compute_stage ) );
            }
        }

        struct ComputeStage
        {
            id<MTLLibrary> library{nil};
            id<MTLFunction> function{nil};
            id<MTLComputePipelineState> state{nil};
            ShaderInterface shader_interface{};
            std::array<int32_t, 8> buffer_size_argument{};
            std::array<id<MTLArgumentEncoder>, 8> argument_encoders{};
        };

        /** Argument encoders are immutable per (function, set) and are the most
         *  expensive object Metal builds per binding, so they are created once
         *  and retargeted with setArgumentBuffer: on each use. */
        static id<MTLArgumentEncoder> GetArgumentEncoder ( std::array<id<MTLArgumentEncoder>, 8>& aCache,
                id<MTLFunction> aFunction, uint32_t aSet )
        {
            if ( aSet >= aCache.size() )
            {
                return nil;
            }
            if ( aCache[aSet] == nil )
            {
                aCache[aSet] = [aFunction newArgumentEncoderWithBufferIndex:aSet];
            }
            return aCache[aSet];
        }

        id<MTLLibrary> CompileLibrary ( std::string_view aSource ) const
        {
            MTLCompileOptions* options = [[MTLCompileOptions alloc] init];
            options.languageVersion = MTLLanguageVersion3_0;
            NSError* error = nil;
            NSString* source = [[NSString alloc] initWithBytes:aSource.data()
                                length:aSource.size()
                                encoding:NSUTF8StringEncoding];
            id<MTLLibrary> library = [mDevice newLibraryWithSource:source options:options error:&error];
            if ( library == nil )
            {
                throw std::runtime_error ( std::string {"Metal shader compilation failed: "} +
                                           ( error.localizedDescription.UTF8String ? : "unknown error" ) );
            }
            return library;
        }

        static MTLCompareFunction ToCompareFunction ( PipelineCompareOp aOperation )
        {
            switch ( aOperation )
            {
            case PipelineCompareOp::NEVER:
                return MTLCompareFunctionNever;
            case PipelineCompareOp::LESS:
                return MTLCompareFunctionLess;
            case PipelineCompareOp::EQUAL:
                return MTLCompareFunctionEqual;
            case PipelineCompareOp::GREATER:
                return MTLCompareFunctionGreater;
            case PipelineCompareOp::NOT_EQUAL:
                return MTLCompareFunctionNotEqual;
            case PipelineCompareOp::GREATER_OR_EQUAL:
                return MTLCompareFunctionGreaterEqual;
            case PipelineCompareOp::ALWAYS:
                return MTLCompareFunctionAlways;
            case PipelineCompareOp::LESS_OR_EQUAL:
            default:
                return MTLCompareFunctionLessEqual;
            }
        }

        static MTLBlendFactor ToBlendFactor ( PipelineBlendFactor aFactor )
        {
            switch ( aFactor )
            {
            case PipelineBlendFactor::ZERO:
                return MTLBlendFactorZero;
            case PipelineBlendFactor::SOURCE_ALPHA:
                return MTLBlendFactorSourceAlpha;
            case PipelineBlendFactor::ONE_MINUS_SOURCE_ALPHA:
                return MTLBlendFactorOneMinusSourceAlpha;
            case PipelineBlendFactor::DESTINATION_ALPHA:
                return MTLBlendFactorDestinationAlpha;
            case PipelineBlendFactor::ONE_MINUS_DESTINATION_ALPHA:
                return MTLBlendFactorOneMinusDestinationAlpha;
            case PipelineBlendFactor::DESTINATION_COLOR:
                return MTLBlendFactorDestinationColor;
            case PipelineBlendFactor::ONE_MINUS_DESTINATION_COLOR:
                return MTLBlendFactorOneMinusDestinationColor;
            case PipelineBlendFactor::ONE:
            default:
                return MTLBlendFactorOne;
            }
        }

        static MTLBlendOperation ToBlendOperation ( PipelineBlendOp aOperation )
        {
            switch ( aOperation )
            {
            case PipelineBlendOp::SUBTRACT:
                return MTLBlendOperationSubtract;
            case PipelineBlendOp::REVERSE_SUBTRACT:
                return MTLBlendOperationReverseSubtract;
            case PipelineBlendOp::MIN:
                return MTLBlendOperationMin;
            case PipelineBlendOp::MAX:
                return MTLBlendOperationMax;
            case PipelineBlendOp::ADD:
            default:
                return MTLBlendOperationAdd;
            }
        }

        void ConfigureBlend ( MTLRenderPipelineColorAttachmentDescriptor* aAttachment ) const
        {
            const PipelineBlendState& blend = mPipeline->GetBlendState();
            aAttachment.writeMask = static_cast<MTLColorWriteMask> ( blend.color_write_mask & 0xf );
            aAttachment.blendingEnabled = blend.enabled == PipelineToggle::ENABLED;
            aAttachment.sourceRGBBlendFactor = ToBlendFactor ( blend.source_color );
            aAttachment.destinationRGBBlendFactor = ToBlendFactor ( blend.destination_color );
            aAttachment.rgbBlendOperation = ToBlendOperation ( blend.color_operation );
            aAttachment.sourceAlphaBlendFactor = ToBlendFactor ( blend.source_alpha );
            aAttachment.destinationAlphaBlendFactor = ToBlendFactor ( blend.destination_alpha );
            aAttachment.alphaBlendOperation = ToBlendOperation ( blend.alpha_operation );
        }

        static MTLVertexFormat ToVertexFormat ( const Mesh::AttributeTuple& aAttribute )
        {
            const uint32_t size = std::get<1> ( aAttribute );
            const Mesh::AttributeType type = std::get<2> ( aAttribute );
            const bool normalized = ( std::get<3> ( aAttribute ) & Mesh::NORMALIZED ) != 0;
            if ( type == Mesh::FLOAT )
            {
                return size == 1 ? MTLVertexFormatFloat : size == 2 ? MTLVertexFormatFloat2 :
                       size == 3 ? MTLVertexFormatFloat3 : MTLVertexFormatFloat4;
            }
            if ( type == Mesh::UNSIGNED_BYTE && size == 4 )
            {
                return normalized ? MTLVertexFormatUChar4Normalized : MTLVertexFormatUChar4;
            }
            if ( type == Mesh::UNSIGNED_SHORT )
            {
                if ( size == 1 ) return normalized ? MTLVertexFormatUShortNormalized : MTLVertexFormatUShort;
                if ( size == 2 ) return normalized ? MTLVertexFormatUShort2Normalized : MTLVertexFormatUShort2;
                if ( size == 3 ) return normalized ? MTLVertexFormatUShort3Normalized : MTLVertexFormatUShort3;
                return normalized ? MTLVertexFormatUShort4Normalized : MTLVertexFormatUShort4;
            }
            throw std::runtime_error ( "Metal vertex attribute format is unsupported" );
        }

        id<MTLRenderPipelineState> GetRenderPipeline ( const MetalMesh& aMesh, size_t aStride )
        {
            // The vertex descriptor is a function of the mesh attribute layout
            // and the stride; the shader inputs are fixed for this pipeline. Two
            // meshes that merely share a stride can still declare different
            // attribute formats and offsets, so the whole layout is keyed.
            size_t key = aStride;
            auto mix = [&key] ( size_t aValue )
            {
                key ^= aValue + 0x9e3779b97f4a7c15ull + ( key << 6 ) + ( key >> 2 );
            };
            for ( const Mesh::AttributeTuple& attribute : aMesh.GetMesh().GetAttributes() )
            {
                mix ( static_cast<size_t> ( std::get<0> ( attribute ) ) );
                mix ( static_cast<size_t> ( std::get<1> ( attribute ) ) );
                mix ( static_cast<size_t> ( std::get<2> ( attribute ) ) );
                mix ( static_cast<size_t> ( std::get<3> ( attribute ) ) );
                mix ( static_cast<size_t> ( std::get<4> ( attribute ) ) );
            }
            auto found = mRenderPipelines.find ( key );
            if ( found != mRenderPipelines.end() )
            {
                return found->second;
            }

            MTLVertexDescriptor* vertex_descriptor = [[MTLVertexDescriptor alloc] init];
            for ( const ShaderInput& input : mVertexInterface->inputs )
            {
                const Mesh::AttributeTuple* attribute = nullptr;
                for ( const Mesh::AttributeTuple& candidate : aMesh.GetMesh().GetAttributes() )
                {
                    if ( static_cast<uint32_t> ( std::get<0> ( candidate ) ) == input.name_hash )
                    {
                        attribute = &candidate;
                        break;
                    }
                }
                if ( attribute == nullptr )
                {
                    throw std::runtime_error ( "Metal mesh is missing vertex input " + input.name );
                }
                vertex_descriptor.attributes[input.location].format = ToVertexFormat ( *attribute );
                vertex_descriptor.attributes[input.location].offset = aMesh.GetMesh().GetAttributeOffset ( *attribute );
                vertex_descriptor.attributes[input.location].bufferIndex = 8;
            }
            vertex_descriptor.layouts[8].stride = aStride;
            vertex_descriptor.layouts[8].stepFunction = MTLVertexStepFunctionPerVertex;

            MTLRenderPipelineDescriptor* descriptor = [[MTLRenderPipelineDescriptor alloc] init];
            descriptor.vertexFunction = mVertexFunction;
            descriptor.fragmentFunction = mFragmentFunction;
            descriptor.vertexDescriptor = vertex_descriptor;
            const uint32_t topology_class = mPipeline->GetTopologyClass();
            descriptor.inputPrimitiveTopology = ( topology_class & Pipeline::TOPOLOGY_CLASS_TRIANGLE ) != 0
                                                ? MTLPrimitiveTopologyClassTriangle
                                                : ( topology_class & Pipeline::TOPOLOGY_CLASS_LINE ) != 0
                                                ? MTLPrimitiveTopologyClassLine
                                                : MTLPrimitiveTopologyClassPoint;
            descriptor.colorAttachments[0].pixelFormat = MTLPixelFormatRGBA16Float;
            ConfigureBlend ( descriptor.colorAttachments[0] );
            for ( uint32_t color = 1; color < mFragmentInterface->fragment_output_count; ++color )
            {
                descriptor.colorAttachments[color].pixelFormat = MTLPixelFormatRGBA16Float;
            }
            descriptor.depthAttachmentPixelFormat = MTLPixelFormatDepth32Float;
            descriptor.rasterSampleCount = 1;
            NSError* error = nil;
            MTLRenderPipelineReflection* reflection = nil;
            id<MTLRenderPipelineState> state = [mDevice newRenderPipelineStateWithDescriptor:descriptor
                                                options:MTLPipelineOptionBindingInfo | MTLPipelineOptionBufferTypeInfo
                                                reflection:&reflection error:&error];
            if ( state == nil )
            {
                throw std::runtime_error ( std::string {"Metal render pipeline creation failed: "} +
                                           ( error.localizedDescription.UTF8String ? : "unknown error" ) );
            }
            for ( id<MTLBinding> binding in reflection.fragmentBindings )
            {
                if ( binding.type == MTLBindingTypeBuffer && binding.index < mFragmentArgumentSets.size() )
                {
                    mFragmentArgumentSets[binding.index] = true;
                }
            }
            mRenderPipelines.emplace ( key, state );
            return state;
        }

        id<MTLRenderPipelineState> GetFullscreenPipeline ( bool aTonemap )
        {
            id<MTLRenderPipelineState> state = aTonemap ? mTonemapPipeline : mFullscreenPipeline;
            if ( state != nil )
            {
                return state;
            }
            MTLRenderPipelineDescriptor* descriptor = [[MTLRenderPipelineDescriptor alloc] init];
            descriptor.vertexFunction = mVertexFunction;
            descriptor.fragmentFunction = mFragmentFunction;
            descriptor.inputPrimitiveTopology = MTLPrimitiveTopologyClassTriangle;
            descriptor.colorAttachments[0].pixelFormat = aTonemap ? MTLPixelFormatBGRA8Unorm : MTLPixelFormatRGBA16Float;
            ConfigureBlend ( descriptor.colorAttachments[0] );
            if ( !aTonemap )
            {
                descriptor.colorAttachments[1].pixelFormat = MTLPixelFormatRGBA16Float;
                descriptor.colorAttachments[2].pixelFormat = MTLPixelFormatRGBA16Float;
                descriptor.depthAttachmentPixelFormat = MTLPixelFormatDepth32Float;
            }
            NSError* error = nil;
            MTLRenderPipelineReflection* reflection = nil;
            state = [mDevice newRenderPipelineStateWithDescriptor:descriptor
                     options:MTLPipelineOptionBindingInfo | MTLPipelineOptionBufferTypeInfo
                     reflection:&reflection error:&error];
            if ( state == nil )
            {
                throw std::runtime_error ( std::string {"Metal fullscreen pipeline creation failed: "} +
                                           ( error.localizedDescription.UTF8String ? : "unknown error" ) );
            }
            for ( id<MTLBinding> binding in reflection.fragmentBindings )
            {
                if ( binding.type == MTLBindingTypeBuffer && binding.index < mFragmentArgumentSets.size() )
                {
                    mFragmentArgumentSets[binding.index] = true;
                }
            }
            if ( aTonemap )
            {
                mTonemapPipeline = state;
            }
            else
            {
                mFullscreenPipeline = state;
            }
            return state;
        }

        const BufferAccessor* FindBuffer ( std::span<const MetalBufferBinding> aBuffers,
                                           uint32_t aName ) const
        {
for ( const MetalBufferBinding& binding : aBuffers )
        {
            if ( binding.mName == aName )
                {
                    return binding.mBuffer;
                }
            }
            return nullptr;
        }

        const MetalTextureBinding* FindTexture ( std::span<const MetalTextureBinding> aTextures,
                uint32_t aName ) const
        {
for ( const MetalTextureBinding& binding : aTextures )
        {
            if ( binding.mName == aName )
                {
                    return &binding;
                }
            }
            return nullptr;
        }

        void BindBindlessResources ( id<MTLRenderCommandEncoder> aEncoder,
                                     const MetalBindlessResources& aBindless )
        {
            const bool uses_textures = mUsesBindlessTextures;
            const bool uses_materials = mUsesBindlessMaterials;
            if ( !uses_textures && !uses_materials )
            {
                return;
            }
            const uint64_t generation = aBindless.GetGeneration();
            if ( uses_textures && ( mBindlessTextures == nil || generation != mBindlessGeneration ) )
            {
                mBindlessTextures = ( __bridge_transfer id<MTLBuffer> )
                                    aBindless.CreateTextureArgumentBuffer ( ( __bridge void* ) mFragmentFunction );
            }
            if ( uses_materials && mBindlessMaterials == nil )
            {
                mBindlessMaterials = ( __bridge_transfer id<MTLBuffer> )
                                     aBindless.CreateMaterialArgumentBuffer ( ( __bridge void* ) mFragmentFunction );
            }
            if ( uses_textures )
            {
                [aEncoder setFragmentBuffer:mBindlessTextures offset:0 atIndex:2];
            }
            if ( uses_materials )
            {
                [aEncoder setFragmentBuffer:mBindlessMaterials offset:0 atIndex:7];
            }
            aBindless.UseResources ( ( __bridge void* ) aEncoder );
            mBindlessGeneration = generation;
        }

        id<MTLDevice> mDevice{nil};
        const Pipeline* mPipeline{nullptr};
        const ShaderInterface* mVertexInterface{nullptr};
        const ShaderInterface* mFragmentInterface{nullptr};
        id<MTLLibrary> mVertexLibrary{nil};
        id<MTLLibrary> mFragmentLibrary{nil};
        id<MTLFunction> mVertexFunction{nil};
        id<MTLFunction> mFragmentFunction{nil};
        id<MTLDepthStencilState> mDepthState{nil};
        id<MTLDepthStencilState> mDisabledDepthState{nil};
        id<MTLRenderPipelineState> mFullscreenPipeline{nil};
        id<MTLRenderPipelineState> mTonemapPipeline{nil};
        id<MTLBuffer> mMultiviewMask{nil};
        uint32_t mViewCount{1};
        id<MTLBuffer> mBindlessTextures{nil};
        id<MTLBuffer> mBindlessMaterials{nil};
        uint64_t mBindlessGeneration{0};
        std::array<bool, 8> mFragmentArgumentSets{};
        std::array<bool, 8> mVertexSets{};
        std::array<bool, 8> mFragmentSets{};
        std::array<id<MTLArgumentEncoder>, 8> mVertexArgumentEncoders{};
        std::array<id<MTLArgumentEncoder>, 8> mFragmentArgumentEncoders{};
        bool mUsesBindlessTextures{false};
        bool mUsesBindlessMaterials{false};
        std::unordered_map<size_t, id<MTLRenderPipelineState>> mRenderPipelines{};
        std::vector<ComputeStage> mComputeStages{};
    };

    MetalPipeline::MetalPipeline ( void* aDevice, const Pipeline& aPipeline ) :
        mImpl{std::make_unique<Impl> ( ( __bridge id<MTLDevice> ) aDevice, aPipeline ) }
    {
    }

    MetalPipeline::~MetalPipeline() = default;
    MetalPipeline::MetalPipeline ( MetalPipeline&& ) noexcept = default;
    MetalPipeline& MetalPipeline::operator= ( MetalPipeline&& ) noexcept = default;

    void MetalPipeline::Dispatch ( void* aCommandBuffer, MetalArgumentBufferPool& aArgumentPool,
                                   uint32_t aGroupCountX,
                                   uint32_t aGroupCountY, uint32_t aGroupCountZ,
                                   std::span<const StorageBufferBinding> aStorageBuffers,
                                   uint32_t aComputeStageIndex,
                                   const BufferAccessor* aClusterParams,
                                   const BufferAccessor* aMatrices,
                                   const BufferAccessor* aLights,
                                   const MetalMesh* aSourceMesh,
                                   std::span<const MetalTextureBinding> aTextures ) const
    {
        if ( aComputeStageIndex >= mImpl->mComputeStages.size() )
        {
            throw std::out_of_range ( "Metal compute stage index is out of range" );
        }
        Impl::ComputeStage& stage = mImpl->mComputeStages[aComputeStageIndex];
        id<MTLCommandBuffer> command_buffer = ( __bridge id<MTLCommandBuffer> ) aCommandBuffer;
        id<MTLComputeCommandEncoder> encoder = [command_buffer computeCommandEncoder];
        if ( encoder == nil )
        {
            throw std::runtime_error ( "Metal compute encoder creation failed" );
        }
        [encoder setComputePipelineState:stage.state];

        std::array<bool, 8> encoded_sets{};
        for ( const ShaderResource& first_resource : stage.shader_interface.resources )
        {
            if ( first_resource.set >= encoded_sets.size() || encoded_sets[first_resource.set] )
            {
                continue;
            }
            id<MTLArgumentEncoder> argument_encoder = Impl::GetArgumentEncoder ( stage.argument_encoders,
                stage.function, first_resource.set );
            if ( argument_encoder == nil )
            {
                throw std::runtime_error ( "Metal argument encoder creation failed for descriptor set " +
                                           std::to_string ( first_resource.set ) );
            }
            size_t argument_offset = 0;
            id<MTLBuffer> argument_buffer = AllocateArgumentBuffer ( aArgumentPool, argument_encoder, argument_offset );
            [argument_encoder setArgumentBuffer:argument_buffer offset:argument_offset];
            uint32_t first_buffer_size = 0;
            for ( const ShaderResource& resource : stage.shader_interface.resources )
            {
                if ( resource.set != first_resource.set )
                {
                    continue;
                }
                if ( resource.type == ShaderResourceType::CombinedImageSampler ||
                     resource.type == ShaderResourceType::SampledImage ||
                     resource.type == ShaderResourceType::StorageImage ||
                     resource.type == ShaderResourceType::Sampler )
                {
                    const MetalTextureBinding* binding = mImpl->FindTexture ( aTextures, resource.name_hash );
                    if ( binding == nullptr ||
                         ( resource.type != ShaderResourceType::Sampler && binding->mTexture == nullptr ) ||
                         ( ( resource.type == ShaderResourceType::CombinedImageSampler ||
                             resource.type == ShaderResourceType::Sampler ) && binding->mSampler == nullptr ) )
                    {
                        throw std::runtime_error ( "Metal dispatch is missing texture resource " + resource.name );
                    }
                    if ( resource.type != ShaderResourceType::Sampler )
                    {
                        id<MTLTexture> texture = ( __bridge id<MTLTexture> ) binding->mTexture;
                        [argument_encoder setTexture:texture atIndex:resource.binding];
                        const MTLResourceUsage usage = resource.type == ShaderResourceType::StorageImage
                                                       ? MTLResourceUsageWrite : MTLResourceUsageRead;
                        [encoder useResource:texture usage:usage];
                    }
                    if ( resource.type == ShaderResourceType::CombinedImageSampler ||
                         resource.type == ShaderResourceType::Sampler )
                    {
                        const uint32_t sampler_index = resource.type == ShaderResourceType::CombinedImageSampler
                                                       ? resource.binding + std::max ( 1u, resource.count )
                                                       : resource.binding;
                        [argument_encoder setSamplerState: ( __bridge id<MTLSamplerState> ) binding->mSampler
                         atIndex:sampler_index];
                    }
                    continue;
                }
                if ( resource.type != ShaderResourceType::StorageBuffer &&
                     resource.type != ShaderResourceType::UniformBuffer )
                {
                    continue;
                }
                const BufferAccessor* accessor = nullptr;
                for ( const StorageBufferBinding& candidate : aStorageBuffers )
                {
                    if ( candidate.mBinding == resource.name_hash )
                    {
                        accessor = candidate.mBuffer;
                        break;
                    }
                }
                if ( accessor == nullptr && resource.name == "ClusterParams" )
                {
                    accessor = aClusterParams;
                }
                else if ( accessor == nullptr && resource.name == "Matrices" )
                {
                    accessor = aMatrices;
                }
                else if ( accessor == nullptr && resource.name == "Lights" )
                {
                    accessor = aLights;
                }
                id<MTLBuffer> native_buffer = nil;
                size_t native_offset = 0;
                uint32_t native_size = 0;
                if ( accessor != nullptr )
                {
                    const Buffer& buffer = accessor->GetMemoryPoolBuffer()->GetBuffer();
                    const auto* metal_buffer = dynamic_cast<const MetalBuffer*> ( &buffer );
                    if ( metal_buffer == nullptr )
                    {
                        throw std::runtime_error ( "Metal dispatch received a non-Metal buffer" );
                    }
                    native_buffer = ( __bridge id<MTLBuffer> ) metal_buffer->GetNativeBuffer();
                    native_offset = accessor->GetOffset();
                    native_size = static_cast<uint32_t> ( accessor->GetSize() );
                }
                else if ( resource.name == "SourceVertices" && aSourceMesh != nullptr )
                {
                    native_buffer = ( __bridge id<MTLBuffer> ) aSourceMesh->GetVertexBuffer();
                    native_size = static_cast<uint32_t> ( aSourceMesh->GetVertexBufferSize() );
                }
                else
                {
                    throw std::runtime_error ( "Metal dispatch is missing buffer " + resource.name );
                }
                if ( first_buffer_size == 0 )
                {
                    first_buffer_size = native_size;
                }
                [argument_encoder setBuffer:native_buffer offset:native_offset atIndex:resource.binding];
                [encoder useResource:native_buffer usage:MTLResourceUsageRead | MTLResourceUsageWrite];
            }
            if ( stage.buffer_size_argument[first_resource.set] >= 0 )
            {
                if ( first_buffer_size == 0 )
                {
                    throw std::runtime_error ( "Metal runtime-array descriptor set has no buffer" );
                }
                const MetalArgumentBufferPool::Allocation size_allocation =
                    aArgumentPool.AllocateArgumentBuffer ( sizeof ( first_buffer_size ), alignof ( uint32_t ) );
                id<MTLBuffer> size_buffer = ( __bridge id<MTLBuffer> ) size_allocation.mBuffer;
                if ( size_buffer == nil )
                {
                    throw std::runtime_error ( "Metal buffer-size constant allocation failed" );
                }
                const size_t size_offset = size_allocation.mOffset;
                std::memcpy ( static_cast<uint8_t*> ( size_buffer.contents ) + size_offset,
                              &first_buffer_size, sizeof ( first_buffer_size ) );
                [argument_encoder setBuffer:size_buffer offset:size_offset
                 atIndex:static_cast<uint32_t> ( stage.buffer_size_argument[first_resource.set] )];
                [encoder useResource:size_buffer usage:MTLResourceUsageRead];
            }
            [encoder setBuffer:argument_buffer offset:argument_offset atIndex:first_resource.set];
            encoded_sets[first_resource.set] = true;
        }
        const std::array<uint32_t, 3>& local = stage.shader_interface.local_size;
        [encoder dispatchThreadgroups:MTLSizeMake ( aGroupCountX, aGroupCountY, aGroupCountZ )
         threadsPerThreadgroup:MTLSizeMake ( std::max ( 1u, local[0] ),
                                             std::max ( 1u, local[1] ),
                                             std::max ( 1u, local[2] ) )];
        [encoder endEncoding];
    }

    void MetalPipeline::Draw ( void* aRenderEncoder, MetalArgumentBufferPool& aArgumentPool, const MetalMesh& aMesh,
                               std::span<const MetalBufferBinding> aBuffers,
                               std::span<const MetalTextureBinding> aTextures,
                               const MetalBindlessResources& aBindless,
                               const BufferAccessor* aSkinnedVertices,
                               Topology aTopology,
                               uint32_t aVertexStart,
                               uint32_t aVertexCount,
                               uint32_t aInstanceCount,
                               uint32_t aFirstInstance,
                               const BufferAccessor* aIndirectCommands,
                               uint32_t aIndirectDrawCount ) const
    {
        id<MTLRenderCommandEncoder> encoder = ( __bridge id<MTLRenderCommandEncoder> ) aRenderEncoder;
        if ( encoder == nil || mImpl->mVertexFunction == nil )
        {
            throw std::runtime_error ( "Metal draw requires a graphics pipeline and active render encoder" );
        }
        const size_t stride = aSkinnedVertices != nullptr ? Mesh::kSkinnedVertexStride : aMesh.GetMesh().GetStride();
        [encoder setRenderPipelineState:mImpl->GetRenderPipeline ( aMesh, stride )];
        [encoder setDepthStencilState:mImpl->mDepthState];
        switch ( mImpl->mPipeline->GetRasterState().cull_mode )
        {
        case PipelineCullMode::FRONT:
            [encoder setCullMode:MTLCullModeFront];
            break;
        case PipelineCullMode::NONE:
            [encoder setCullMode:MTLCullModeNone];
            break;
        case PipelineCullMode::BACK:
        default:
            [encoder setCullMode:MTLCullModeBack];
            break;
        }
        [encoder setFrontFacingWinding:mImpl->mPipeline->GetRasterState().front_face == PipelineFrontFace::CLOCKWISE
         ? MTLWindingClockwise : MTLWindingCounterClockwise];
        [encoder setTriangleFillMode:mImpl->mPipeline->GetRasterState().polygon_mode == PipelinePolygonMode::LINE
         ? MTLTriangleFillModeLines : MTLTriangleFillModeFill];
        mImpl->BindBindlessResources ( encoder, aBindless );

        auto native = [] ( const BufferAccessor & aAccessor ) -> id<MTLBuffer>
        {
            const auto* buffer = dynamic_cast<const MetalBuffer*> ( &aAccessor.GetMemoryPoolBuffer()->GetBuffer() );
            if ( buffer == nullptr )
            {
                throw std::runtime_error ( "Metal graphics binding received a non-Metal buffer" );
            }
            return ( __bridge id<MTLBuffer> ) buffer->GetNativeBuffer();
        };
        for ( uint32_t set = 0; set < mImpl->mVertexSets.size(); ++set )
        {
            if ( !mImpl->mVertexSets[set] )
            {
                continue;
            }
            id<MTLArgumentEncoder> argument_encoder = Impl::GetArgumentEncoder ( mImpl->mVertexArgumentEncoders,
                mImpl->mVertexFunction, set );
            size_t argument_offset = 0;
            id<MTLBuffer> argument_buffer = AllocateArgumentBuffer ( aArgumentPool, argument_encoder, argument_offset );
            [argument_encoder setArgumentBuffer:argument_buffer offset:argument_offset];
            for ( const ShaderResource& resource : mImpl->mVertexInterface->resources )
            {
                if ( resource.set != set )
                {
                    continue;
                }
                const BufferAccessor* accessor = mImpl->FindBuffer ( aBuffers, resource.name_hash );
                if ( accessor == nullptr )
                {
                    throw std::runtime_error ( "Metal draw is missing vertex resource " + resource.name );
                }
                id<MTLBuffer> buffer = native ( *accessor );
                [argument_encoder setBuffer:buffer offset:accessor->GetOffset() atIndex:resource.binding];
                [encoder useResource:buffer usage:MTLResourceUsageRead stages:MTLRenderStageVertex];
            }
            [encoder setVertexBuffer:argument_buffer offset:argument_offset atIndex:set];
        }

        for ( uint32_t set = 0; set < mImpl->mFragmentArgumentSets.size(); ++set )
        {
            if ( !mImpl->mFragmentArgumentSets[set] )
            {
                continue;
            }
            bool bindless_set = false;
            bool has_resource = false;
            for ( const ShaderResource& resource : mImpl->mFragmentInterface->resources )
            {
                if ( resource.set != set )
                {
                    continue;
                }
                bindless_set = bindless_set ||
                               ( set == 2 && resource.type == ShaderResourceType::CombinedImageSampler ) ||
                               ( set == 7 && resource.name == "Bindless" );
                has_resource = has_resource ||
                               resource.type == ShaderResourceType::UniformBuffer ||
                               resource.type == ShaderResourceType::StorageBuffer ||
                               resource.type == ShaderResourceType::CombinedImageSampler;
            }
            if ( bindless_set || !has_resource )
            {
                continue;
            }
            id<MTLArgumentEncoder> argument_encoder = Impl::GetArgumentEncoder ( mImpl->mFragmentArgumentEncoders,
                mImpl->mFragmentFunction, set );
            if ( argument_encoder == nil )
            {
                continue;
            }
            size_t argument_offset = 0;
            id<MTLBuffer> argument_buffer = AllocateArgumentBuffer ( aArgumentPool, argument_encoder, argument_offset );
            [argument_encoder setArgumentBuffer:argument_buffer offset:argument_offset];
            for ( const ShaderResource& resource : mImpl->mFragmentInterface->resources )
            {
                if ( resource.set != set )
                {
                    continue;
                }
                if ( resource.type == ShaderResourceType::CombinedImageSampler )
                {
                    const MetalTextureBinding* binding = mImpl->FindTexture ( aTextures, resource.name_hash );
                    if ( binding == nullptr || binding->mTexture == nullptr || binding->mSampler == nullptr )
                    {
                        throw std::runtime_error ( "Metal draw is missing fragment texture " + resource.name );
                    }
                    id<MTLTexture> texture = ( __bridge id<MTLTexture> ) binding->mTexture;
                    const uint32_t texture_index = resource.binding;
                    [argument_encoder setTexture:texture atIndex:texture_index];
                    [argument_encoder setSamplerState: ( __bridge id<MTLSamplerState> ) binding->mSampler
                     atIndex:texture_index + std::max ( 1u, resource.count )];
                    [encoder useResource:texture usage:MTLResourceUsageRead stages:MTLRenderStageFragment];
                    continue;
                }
                if ( resource.type == ShaderResourceType::UniformBuffer ||
                     resource.type == ShaderResourceType::StorageBuffer )
                {
                    const BufferAccessor* accessor = mImpl->FindBuffer ( aBuffers, resource.name_hash );
                    if ( accessor == nullptr )
                    {
                        throw std::runtime_error ( "Metal draw is missing fragment resource " + resource.name );
                    }
                    id<MTLBuffer> buffer = native ( *accessor );
                    [argument_encoder setBuffer:buffer offset:accessor->GetOffset() atIndex:resource.binding];
                    [encoder useResource:buffer usage:MTLResourceUsageRead stages:MTLRenderStageFragment];
                }
            }
            [encoder setFragmentBuffer:argument_buffer offset:argument_offset atIndex:set];
        }

        id<MTLBuffer> vertex_buffer = nil;
        size_t vertex_offset = 0;
        if ( aSkinnedVertices != nullptr )
        {
            vertex_buffer = native ( *aSkinnedVertices );
            vertex_offset = aSkinnedVertices->GetOffset();
        }
        else
        {
            vertex_buffer = ( __bridge id<MTLBuffer> ) aMesh.GetVertexBuffer();
        }
        [encoder setVertexBuffer:vertex_buffer offset:vertex_offset atIndex:8];
        if ( mImpl->mMultiviewMask != nil )
        {
            [encoder setVertexBuffer:mImpl->mMultiviewMask offset:0 atIndex:24];
        }

        MTLPrimitiveType primitive = MTLPrimitiveTypeTriangle;
        if ( aTopology == Topology::LINE_LIST || aTopology == Topology::LINE_STRIP )
        {
            primitive = aTopology == Topology::LINE_LIST ? MTLPrimitiveTypeLine : MTLPrimitiveTypeLineStrip;
        }
        else if ( aTopology == Topology::POINT_LIST )
        {
            primitive = MTLPrimitiveTypePoint;
        }
        else if ( aTopology == Topology::TRIANGLE_STRIP )
        {
            primitive = MTLPrimitiveTypeTriangleStrip;
        }
        else if ( aTopology != Topology::TRIANGLE_LIST )
        {
            throw std::runtime_error ( "Metal primitive topology is not implemented" );
        }
        const uint32_t available_count = aMesh.GetIndexCount() != 0 ? aMesh.GetIndexCount() : aMesh.GetMesh().GetVertexCount();
        const uint32_t draw_count = aVertexCount == 0xffffffff ? available_count - aVertexStart : aVertexCount;
        if ( aIndirectCommands != nullptr )
        {
            if ( aMesh.GetIndexCount() == 0 )
            {
                throw std::runtime_error ( "Metal indirect drawing requires an indexed mesh" );
            }
            id<MTLBuffer> indirect_buffer = native ( *aIndirectCommands );
            id<MTLBuffer> index_buffer = ( __bridge id<MTLBuffer> ) aMesh.GetIndexBuffer();
            const MTLIndexType index_type = aMesh.Has32BitIndices() ? MTLIndexTypeUInt32 : MTLIndexTypeUInt16;
            constexpr size_t command_size = 5 * sizeof ( uint32_t );
            for ( uint32_t command = 0; command < aIndirectDrawCount; ++command )
            {
                [encoder drawIndexedPrimitives:primitive indexType:index_type
                 indexBuffer:index_buffer indexBufferOffset:0
                 indirectBuffer:indirect_buffer
                 indirectBufferOffset:aIndirectCommands->GetOffset() + command * command_size];
            }
            return;
        }
        if ( aMesh.GetIndexCount() != 0 )
        {
            id<MTLBuffer> index_buffer = ( __bridge id<MTLBuffer> ) aMesh.GetIndexBuffer();
            const MTLIndexType index_type = aMesh.Has32BitIndices() ? MTLIndexTypeUInt32 : MTLIndexTypeUInt16;
            const size_t index_size = aMesh.Has32BitIndices() ? 4 : 2;
            [encoder drawIndexedPrimitives:primitive indexCount:draw_count indexType:index_type
             indexBuffer:index_buffer indexBufferOffset:static_cast<size_t> ( aVertexStart ) * index_size
             instanceCount:aInstanceCount * mImpl->mViewCount baseVertex:0 baseInstance:aFirstInstance];
        }
        else
        {
            [encoder drawPrimitives:primitive vertexStart:aVertexStart vertexCount:draw_count
             instanceCount:aInstanceCount * mImpl->mViewCount baseInstance:aFirstInstance];
        }
    }

    void MetalPipeline::DrawFullscreen ( void* aRenderEncoder, MetalArgumentBufferPool& aArgumentPool,
                                         std::span<const MetalBufferBinding> aBuffers,
                                         std::span<const MetalTextureBinding> aTextures ) const
    {
        id<MTLRenderCommandEncoder> encoder = ( __bridge id<MTLRenderCommandEncoder> ) aRenderEncoder;
        if ( encoder == nil || mImpl->mVertexFunction == nil )
        {
            throw std::runtime_error ( "Metal fullscreen draw requires an active render encoder" );
        }
        const bool tonemap = mImpl->mFragmentInterface->fragment_output_count == 1;
        [encoder setRenderPipelineState:mImpl->GetFullscreenPipeline ( tonemap )];
        [encoder setDepthStencilState:tonemap ? mImpl->mDisabledDepthState : mImpl->mDepthState];

        auto native = [] ( const BufferAccessor & aAccessor ) -> id<MTLBuffer>
        {
            const auto* buffer = dynamic_cast<const MetalBuffer*> ( &aAccessor.GetMemoryPoolBuffer()->GetBuffer() );
            if ( buffer == nullptr )
            {
                throw std::runtime_error ( "Metal fullscreen binding received a non-Metal buffer" );
            }
            return ( __bridge id<MTLBuffer> ) buffer->GetNativeBuffer();
        };
        for ( uint32_t set = 0; set < mImpl->mVertexSets.size(); ++set )
        {
            if ( mImpl->mVertexSets[set] )
            {
                id<MTLArgumentEncoder> argument_encoder = Impl::GetArgumentEncoder ( mImpl->mVertexArgumentEncoders,
                    mImpl->mVertexFunction, set );
                size_t argument_offset = 0;
                id<MTLBuffer> argument_buffer = AllocateArgumentBuffer ( aArgumentPool, argument_encoder, argument_offset );
                [argument_encoder setArgumentBuffer:argument_buffer offset:argument_offset];
                for ( const ShaderResource& resource : mImpl->mVertexInterface->resources )
                {
                    if ( resource.set != set )
                    {
                        continue;
                    }
                    const BufferAccessor* accessor = mImpl->FindBuffer ( aBuffers, resource.name_hash );
                    if ( accessor == nullptr )
                    {
                        throw std::runtime_error ( "Metal fullscreen draw is missing vertex resource " + resource.name );
                    }
                    id<MTLBuffer> buffer = native ( *accessor );
                    [argument_encoder setBuffer:buffer offset:accessor->GetOffset() atIndex:resource.binding];
                    [encoder useResource:buffer usage:MTLResourceUsageRead stages:MTLRenderStageVertex];
                }
                [encoder setVertexBuffer:argument_buffer offset:argument_offset atIndex:set];
            }

            bool has_fragment_set = mImpl->mFragmentSets[set];
            if ( !has_fragment_set )
            {
                continue;
            }
            id<MTLArgumentEncoder> argument_encoder = Impl::GetArgumentEncoder ( mImpl->mFragmentArgumentEncoders,
                mImpl->mFragmentFunction, set );
            size_t argument_offset = 0;
            id<MTLBuffer> argument_buffer = AllocateArgumentBuffer ( aArgumentPool, argument_encoder, argument_offset );
            [argument_encoder setArgumentBuffer:argument_buffer offset:argument_offset];
            for ( const ShaderResource& resource : mImpl->mFragmentInterface->resources )
            {
                if ( resource.set != set )
                {
                    continue;
                }
                if ( resource.type == ShaderResourceType::CombinedImageSampler )
                {
                    const MetalTextureBinding* binding = mImpl->FindTexture ( aTextures, resource.name_hash );
                    if ( binding == nullptr )
                    {
                        throw std::runtime_error ( "Metal fullscreen draw is missing texture " + resource.name );
                    }
                    id<MTLTexture> texture = ( __bridge id<MTLTexture> ) binding->mTexture;
                    [argument_encoder setTexture:texture atIndex:resource.binding];
                    [argument_encoder setSamplerState: ( __bridge id<MTLSamplerState> ) binding->mSampler
                     atIndex:resource.binding + std::max ( 1u, resource.count )];
                    [encoder useResource:texture usage:MTLResourceUsageRead stages:MTLRenderStageFragment];
                }
                else if ( resource.type == ShaderResourceType::UniformBuffer ||
                          resource.type == ShaderResourceType::StorageBuffer )
                {
                    const BufferAccessor* accessor = mImpl->FindBuffer ( aBuffers, resource.name_hash );
                    if ( accessor == nullptr )
                    {
                        throw std::runtime_error ( "Metal fullscreen draw is missing fragment resource " + resource.name );
                    }
                    id<MTLBuffer> buffer = native ( *accessor );
                    [argument_encoder setBuffer:buffer offset:accessor->GetOffset() atIndex:resource.binding];
                    [encoder useResource:buffer usage:MTLResourceUsageRead stages:MTLRenderStageFragment];
                }
            }
            [encoder setFragmentBuffer:argument_buffer offset:argument_offset atIndex:set];
        }
        [encoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:3];
    }
}