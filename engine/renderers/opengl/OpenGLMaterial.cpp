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
#include "aeongames/Material.hpp"
#include "aeongames/Texture.hpp"
#include "aeongames/Mesh.hpp"
#include "aeongames/CRC.hpp"
#include "aeongames/MaterialSamplers.hpp"

#include "OpenGLFunctions.hpp"
#include "OpenGLMaterial.hpp"
#include "OpenGLRenderer.hpp"
#include "OpenGLPipeline.hpp"

namespace AeonGames
{
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
    }

    OpenGLMaterial::~OpenGLMaterial ( )
    {
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
        mUniformBuffer{std::move ( aOpenGLMaterial.mUniformBuffer ) }
    {
    }

    void OpenGLMaterial::Bind ( const OpenGLPipeline& aPipeline ) const
    {
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
