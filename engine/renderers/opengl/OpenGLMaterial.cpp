/*
Copyright (C) 2016-2021,2025,2026 Rodrigo Jose Hernandez Cordoba

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
#include <cstring>
#include <algorithm>
#include "aeongames/Material.hpp"
#include "aeongames/Texture.hpp"
#include "aeongames/Mesh.hpp"
#include "aeongames/CRC.hpp"
#include "aeongames/MaterialSamplers.hpp"
#include "aeongames/GpuMaterial.hpp"

#include "OpenGLFunctions.hpp"
#include "OpenGLMaterial.hpp"
#include "OpenGLRenderer.hpp"
#include "OpenGLPipeline.hpp"

namespace AeonGames
{
    // Matches "layout(location = 0) uniform uint MaterialIndex;" in the OpenGL
    // bindless branch of clustered_phong.frag.
    static constexpr GLint kMaterialIndexUniformLocation = 0;

    OpenGLMaterial::OpenGLMaterial ( OpenGLRenderer& aOpenGLRenderer, const Material& aMaterial ) :
        mOpenGLRenderer{aOpenGLRenderer},
        mMaterial{&aMaterial},
        mUniformBuffer{}
    {
        if ( aMaterial.GetUniformBuffer().size() )
        {
            mUniformBuffer.Initialize ( static_cast<GLsizei> ( aMaterial.GetUniformBuffer().size() ), GL_STATIC_DRAW, aMaterial.GetUniformBuffer().data() );
        }

        // Preload linked textures
        for ( auto& i : aMaterial.GetSamplers() )
        {
            const Texture* texture = std::get<1> ( i ).Get<Texture>();
            mOpenGLRenderer.LoadTexture ( *texture );
        }

        // Bindless: build this material's record in the renderer's global material
        // storage buffer. Each of the six canonical sampler slots resolves to the
        // material's own texture or the slot's fallback, stored as that texture's
        // resident ARB_bindless handle (uvec2). The metallic-roughness factors are
        // copied verbatim from the material uniform block, whose std140 layout is
        // identical to the record's factor region.
        if ( mOpenGLRenderer.HasBindlessTexture() )
        {
            GpuMaterial record{};
            const auto& samplers = aMaterial.GetSamplers();
            for ( size_t slot = 0; slot < kMaterialSamplerSlots.size(); ++slot )
            {
                const char* slot_name = kMaterialSamplerSlots[slot].name;
                const uint32_t slot_crc = crc32i ( slot_name, std::strlen ( slot_name ) );
                const Texture* texture = nullptr;
                for ( const auto& sampler : samplers )
                {
                    if ( std::get<0> ( sampler ) == slot_crc )
                    {
                        texture = std::get<1> ( sampler ).Cast<Texture>();
                        break;
                    }
                }
                ResourceId fallback{ "Texture"_crc32, kMaterialSamplerSlots[slot].fallback_path };
                if ( texture == nullptr )
                {
                    texture = fallback.Get<Texture>();
                }
                const GLuint64 handle = mOpenGLRenderer.GetTextureHandle ( *texture );
                record.texture_refs[slot][0] = static_cast<uint32_t> ( handle & 0xFFFFFFFFu );
                record.texture_refs[slot][1] = static_cast<uint32_t> ( handle >> 32 );
            }
            const size_t factor_size = sizeof ( record ) - sizeof ( record.texture_refs );
            const size_t copy_size = std::min ( aMaterial.GetUniformBuffer().size(), factor_size );
            if ( copy_size != 0 )
            {
                std::memcpy ( reinterpret_cast<uint8_t*> ( &record ) + sizeof ( record.texture_refs ),
                              aMaterial.GetUniformBuffer().data(), copy_size );
            }
            mBindlessMaterialIndex = mOpenGLRenderer.RegisterBindlessMaterial ( record );
        }
    }

    OpenGLMaterial::~OpenGLMaterial ( )
    {
        mOpenGLRenderer.UnregisterBindlessMaterial ( mBindlessMaterialIndex );
        // A moved-from material owns nothing: its resources were transferred to
        // the moved-to object. Releasing them here would unload textures and
        // finalize a buffer the live material still uses (this is what corrupted
        // the baked bindless handles when emplacing into the material store).
        if ( mMaterial == nullptr )
        {
            return;
        }
        // Unload linked textures
        for ( auto& i : mMaterial->GetSamplers() )
        {
            const Texture* texture = std::get<1> ( i ).Get<Texture>();
            mOpenGLRenderer.UnloadTexture ( *texture );
        }
        if ( mMaterial->GetUniformBuffer().size() )
        {
            mUniformBuffer.Finalize();
        }
    }

    OpenGLMaterial::OpenGLMaterial ( OpenGLMaterial&& aOpenGLMaterial ) :
        mOpenGLRenderer{aOpenGLMaterial.mOpenGLRenderer},
        mMaterial{aOpenGLMaterial.mMaterial},
        mUniformBuffer{std::move ( aOpenGLMaterial.mUniformBuffer ) },
        mBindlessMaterialIndex{aOpenGLMaterial.mBindlessMaterialIndex}
    {
        // Transfer ownership: the moved-from material must not release the
        // textures / bindless record now owned by this instance.
        aOpenGLMaterial.mMaterial = nullptr;
        aOpenGLMaterial.mBindlessMaterialIndex = UINT32_MAX;
    }

    uint32_t OpenGLMaterial::GetBindlessMaterialIndex() const
    {
        return mBindlessMaterialIndex;
    }

    void OpenGLMaterial::Bind ( const OpenGLPipeline& aPipeline ) const
    {
        // Bindless pipelines (those declaring the global material storage block)
        // read this material's factors and textures from one SSBO record: bind
        // that renderer-owned buffer and set the per-draw record index. The legacy
        // per-sampler / Material-UBO binding below is a no-op for such pipelines
        // (they reflect neither the material samplers nor the Material block) and
        // still drives the non-bindless pipelines (solid colour, debug grid).
        if ( const OpenGLUniformBlock * material_block = aPipeline.GetStorageBlock ( Mesh::BINDLESS );
             material_block != nullptr && mBindlessMaterialIndex != UINT32_MAX )
        {
            glBindBufferBase ( GL_SHADER_STORAGE_BUFFER, material_block->binding, mOpenGLRenderer.GetMaterialStorageBufferId() );
            OPENGL_CHECK_ERROR_THROW;
            glProgramUniform1ui ( static_cast<GLuint> ( aPipeline.GetProgramId() ), kMaterialIndexUniformLocation, mBindlessMaterialIndex );
            OPENGL_CHECK_ERROR_THROW;
        }

        const auto& samplers = mMaterial->GetSamplers();
        // Bind every canonical material sampler slot the pipeline declares to
        // the material's matching texture (by name crc), or the slot's fallback
        // texture when the material omits it (white diffuse / flat normal). This
        // mirrors the Vulkan material's padded sampler set so both backends
        // sample a valid texture for NormalMap even on materials that have none.
        for ( size_t slot = 0; slot < kMaterialSamplerSlots.size(); ++slot )
        {
            const char* slot_name = kMaterialSamplerSlots[slot].name;
            const uint32_t slot_crc = crc32i ( slot_name, std::strlen ( slot_name ) );
            if ( !aPipeline.HasSampler ( slot_crc ) )
            {
                continue;
            }
            GLuint texture_id = 0;
            bool found = false;
            for ( const auto& sampler : samplers )
            {
                if ( std::get<0> ( sampler ) == slot_crc )
                {
                    texture_id = mOpenGLRenderer.GetTextureId ( *std::get<1> ( sampler ).Cast<Texture>() );
                    found = true;
                    break;
                }
            }
            if ( !found )
            {
                ResourceId fallback{ "Texture"_crc32, kMaterialSamplerSlots[slot].fallback_path };
                texture_id = mOpenGLRenderer.GetTextureId ( *fallback.Get<Texture>() );
            }
            glBindTextureUnit ( aPipeline.GetSamplerLocation ( slot_crc ), texture_id );
            OPENGL_CHECK_ERROR_THROW;
        }

        ///@todo Material format does not currently support non block uniforms.

        if ( mMaterial->GetUniformBuffer().size() )
        {
            const OpenGLUniformBlock* uniform_block = aPipeline.GetUniformBlock ( Mesh::MATERIAL );
            if ( uniform_block != nullptr )
            {
                glBindBufferBase ( GL_UNIFORM_BUFFER, uniform_block->binding, mUniformBuffer.GetBufferId() );
                OPENGL_CHECK_ERROR_THROW;
            }
        }
    }
}
