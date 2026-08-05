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
#include <limits>
#include <stdexcept>
#include <vector>
#include "aeongames/GpuMaterial.hpp"
#include "MetalBuffer.h"
#include "MetalBindlessResources.h"
#include "MetalTexture.h"

namespace AeonGames
{
    namespace
    {
        MTLSamplerAddressMode ToAddressMode ( Material::SamplerAddressMode aMode )
        {
            switch ( aMode )
            {
            case Material::SamplerAddressMode::MIRRORED_REPEAT:
                return MTLSamplerAddressModeMirrorRepeat;
            case Material::SamplerAddressMode::CLAMP_TO_EDGE:
                return MTLSamplerAddressModeClampToEdge;
            case Material::SamplerAddressMode::CLAMP_TO_BORDER:
                return MTLSamplerAddressModeClampToBorderColor;
            case Material::SamplerAddressMode::REPEAT:
            default:
                return MTLSamplerAddressModeRepeat;
            }
        }

        MTLCompareFunction ToCompareFunction ( Material::SamplerCompareOp aOperation )
        {
            switch ( aOperation )
            {
            case Material::SamplerCompareOp::LESS:
                return MTLCompareFunctionLess;
            case Material::SamplerCompareOp::EQUAL:
                return MTLCompareFunctionEqual;
            case Material::SamplerCompareOp::LESS_OR_EQUAL:
                return MTLCompareFunctionLessEqual;
            case Material::SamplerCompareOp::GREATER:
                return MTLCompareFunctionGreater;
            case Material::SamplerCompareOp::NOT_EQUAL:
                return MTLCompareFunctionNotEqual;
            case Material::SamplerCompareOp::GREATER_OR_EQUAL:
                return MTLCompareFunctionGreaterEqual;
            case Material::SamplerCompareOp::ALWAYS:
                return MTLCompareFunctionAlways;
            case Material::SamplerCompareOp::NEVER:
            default:
                return MTLCompareFunctionNever;
            }
        }

        id<MTLSamplerState> CreateSampler ( id<MTLDevice> aDevice, const Material::SamplerState& aState )
        {
            MTLSamplerDescriptor* descriptor = [[MTLSamplerDescriptor alloc] init];
            descriptor.minFilter = aState.min_filter == Material::SamplerFilter::NEAREST
                                   ? MTLSamplerMinMagFilterNearest : MTLSamplerMinMagFilterLinear;
            descriptor.magFilter = aState.mag_filter == Material::SamplerFilter::NEAREST
                                   ? MTLSamplerMinMagFilterNearest : MTLSamplerMinMagFilterLinear;
            descriptor.mipFilter = !aState.mipmap_enable ? MTLSamplerMipFilterNotMipmapped :
                                   aState.mipmap_mode == Material::SamplerMipmapMode::NEAREST
                                   ? MTLSamplerMipFilterNearest : MTLSamplerMipFilterLinear;
            descriptor.sAddressMode = ToAddressMode ( aState.address_mode_u );
            descriptor.tAddressMode = ToAddressMode ( aState.address_mode_v );
            descriptor.rAddressMode = ToAddressMode ( aState.address_mode_w );
            descriptor.maxAnisotropy = aState.anisotropy_enable
                                       ? static_cast<NSUInteger> ( std::max ( 1.0f, aState.max_anisotropy ) ) : 1;
            descriptor.lodMinClamp = aState.min_lod;
            descriptor.lodMaxClamp = aState.mipmap_enable ? aState.max_lod : 0.0f;
            descriptor.compareFunction = aState.compare_enable ? ToCompareFunction ( aState.compare_op ) : MTLCompareFunctionNever;
            descriptor.borderColor = aState.border_color == Material::SamplerBorderColor::OPAQUE_WHITE
                                     ? MTLSamplerBorderColorOpaqueWhite :
                                     aState.border_color == Material::SamplerBorderColor::OPAQUE_BLACK
                                     ? MTLSamplerBorderColorOpaqueBlack : MTLSamplerBorderColorTransparentBlack;
            descriptor.supportArgumentBuffers = YES;
            id<MTLSamplerState> sampler = [aDevice newSamplerStateWithDescriptor:descriptor];
            if ( sampler == nil )
            {
                throw std::runtime_error ( "Metal sampler creation failed" );
            }
            return sampler;
        }
    }

    class MetalBindlessResources::Impl
    {
    public:
        struct Pair
        {
            const MetalTexture* texture{nullptr};
            Material::SamplerState state{};
            id<MTLSamplerState> sampler{nil};
            uint32_t references{0};
        };

        Impl ( id<MTLDevice> aDevice, const RendererSettings& aSettings ) :
            device{aDevice},
            texture_capacity{std::min ( aSettings.mBindlessTextureCapacity, kShaderTextureCapacity ) },
            material_capacity{std::min ( aSettings.mBindlessMaterialCapacity, 4096u ) },
            material_buffer{ ( __bridge void* ) aDevice,
                             static_cast<size_t> ( material_capacity ) * sizeof ( GpuMaterial ) },
            pairs ( texture_capacity ),
            material_used ( material_capacity, false )
        {
            if ( texture_capacity == 0 || material_capacity == 0 )
            {
                throw std::runtime_error ( "Metal bindless capacities must be greater than zero" );
            }
        }

        id<MTLDevice> device{nil};
        uint32_t texture_capacity{0};
        uint32_t material_capacity{0};
        MetalBuffer material_buffer;
        std::vector<Pair> pairs{};
        std::vector<bool> material_used{};
        uint64_t generation{1};
        // Residency list rebuilt only when the pair table changes: making the
        // bindless set resident is a per-draw cost, so it must not scale with
        // the (16k) capacity, only with the slots actually in use.
        std::vector<__unsafe_unretained id<MTLResource>> resident{};
        uint64_t resident_generation{0};
    };

    MetalBindlessResources::MetalBindlessResources ( void* aDevice, const RendererSettings& aSettings ) :
        mImpl{std::make_unique<Impl> ( ( __bridge id<MTLDevice> ) aDevice, aSettings ) }
    {
    }

    MetalBindlessResources::~MetalBindlessResources() = default;

    uint32_t MetalBindlessResources::AcquirePair ( const MetalTexture& aTexture, const Material::SamplerState& aState )
    {
        for ( uint32_t slot = 0; slot < mImpl->pairs.size(); ++slot )
        {
            Impl::Pair& pair = mImpl->pairs[slot];
            if ( pair.references != 0 && pair.texture == &aTexture && pair.state == aState )
            {
                ++pair.references;
                return slot;
            }
        }
        for ( uint32_t slot = 0; slot < mImpl->pairs.size(); ++slot )
        {
            Impl::Pair& pair = mImpl->pairs[slot];
            if ( pair.references == 0 )
            {
                pair.texture = &aTexture;
                pair.state = aState;
                pair.sampler = CreateSampler ( mImpl->device, aState );
                pair.references = 1;
                ++mImpl->generation;
                return slot;
            }
        }
        throw std::runtime_error ( "Metal bindless texture/sampler capacity exhausted" );
    }

    void MetalBindlessResources::ReleasePair ( uint32_t aSlot )
    {
        if ( aSlot >= mImpl->pairs.size() || mImpl->pairs[aSlot].references == 0 )
        {
            return;
        }
        Impl::Pair& pair = mImpl->pairs[aSlot];
        if ( --pair.references == 0 )
        {
            pair.texture = nullptr;
            pair.sampler = nil;
            ++mImpl->generation;
        }
    }

    uint32_t MetalBindlessResources::RegisterMaterial ( const GpuMaterial& aMaterial )
    {
        for ( uint32_t index = 0; index < mImpl->material_used.size(); ++index )
        {
            if ( !mImpl->material_used[index] )
            {
                mImpl->material_used[index] = true;
                mImpl->material_buffer.WriteMemory ( static_cast<size_t> ( index ) * sizeof ( GpuMaterial ),
                                                     sizeof ( GpuMaterial ), &aMaterial );
                return index;
            }
        }
        throw std::runtime_error ( "Metal bindless material capacity exhausted" );
    }

    void MetalBindlessResources::UnregisterMaterial ( uint32_t aIndex )
    {
        if ( aIndex < mImpl->material_used.size() )
        {
            mImpl->material_used[aIndex] = false;
            mImpl->material_buffer.WriteMemory ( static_cast<size_t> ( aIndex ) * sizeof ( GpuMaterial ),
                                                 sizeof ( GpuMaterial ) );
        }
    }

    uint64_t MetalBindlessResources::GetGeneration() const
    {
        return mImpl->generation;
    }

    void* MetalBindlessResources::CreateTextureArgumentBuffer ( void* aFragmentFunction ) const
    {
        id<MTLFunction> function = ( __bridge id<MTLFunction> ) aFragmentFunction;
        id<MTLArgumentEncoder> encoder = [function newArgumentEncoderWithBufferIndex:2];
        if ( encoder == nil )
        {
            throw std::runtime_error ( "Metal pipeline has no bindless set 2 argument buffer" );
        }
        id<MTLBuffer> buffer = [mImpl->device newBufferWithLength:encoder.encodedLength
                                options:MTLResourceStorageModeShared];
        [encoder setArgumentBuffer:buffer offset:0];
        for ( uint32_t slot = 0; slot < mImpl->pairs.size(); ++slot )
        {
            const Impl::Pair& pair = mImpl->pairs[slot];
            if ( pair.references == 0 )
            {
                continue;
            }
            [encoder setTexture: ( __bridge id<MTLTexture> ) pair.texture->GetNativeTexture() atIndex:slot];
            [encoder setSamplerState:pair.sampler atIndex:kShaderTextureCapacity + slot];
        }
        return ( __bridge_retained void* ) buffer;
    }

    void* MetalBindlessResources::CreateMaterialArgumentBuffer ( void* aFragmentFunction ) const
    {
        id<MTLFunction> function = ( __bridge id<MTLFunction> ) aFragmentFunction;
        id<MTLArgumentEncoder> encoder = [function newArgumentEncoderWithBufferIndex:7];
        if ( encoder == nil )
        {
            throw std::runtime_error ( "Metal pipeline has no material set 7 argument buffer" );
        }
        id<MTLBuffer> buffer = [mImpl->device newBufferWithLength:encoder.encodedLength
                                options:MTLResourceStorageModeShared];
        [encoder setArgumentBuffer:buffer offset:0];
        [encoder setBuffer: ( __bridge id<MTLBuffer> ) mImpl->material_buffer.GetNativeBuffer()
         offset:0 atIndex:0];
        return ( __bridge_retained void* ) buffer;
    }

    void MetalBindlessResources::UseResources ( void* aRenderEncoder ) const
    {
        id<MTLRenderCommandEncoder> encoder = ( __bridge id<MTLRenderCommandEncoder> ) aRenderEncoder;
        if ( mImpl->resident.empty() || mImpl->resident_generation != mImpl->generation )
        {
            mImpl->resident.clear();
            mImpl->resident.push_back ( ( __bridge id<MTLBuffer> ) mImpl->material_buffer.GetNativeBuffer() );
            for ( const Impl::Pair& pair : mImpl->pairs )
            {
                if ( pair.references != 0 )
                {
                    mImpl->resident.push_back ( ( __bridge id<MTLTexture> ) pair.texture->GetNativeTexture() );
                }
            }
            mImpl->resident_generation = mImpl->generation;
        }
        [encoder useResources:mImpl->resident.data()
         count:mImpl->resident.size()
         usage:MTLResourceUsageRead
         stages:MTLRenderStageFragment];
    }
}