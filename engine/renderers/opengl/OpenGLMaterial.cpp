/*
Copyright (C) 2016-2021 Rodrigo Jose Hernandez Cordoba

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
#include "aeongames/Material.h"
#include "aeongames/Texture.h"
#include "OpenGLFunctions.h"
#include "OpenGLMaterial.h"
#include "OpenGLRenderer.h"

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

    void OpenGLMaterial::Bind() const
    {
        for ( GLenum i = 0; i < mMaterial->GetSamplers().size(); ++i )
        {
            glActiveTexture ( GL_TEXTURE0 + i );
            OPENGL_CHECK_ERROR_NO_THROW;
            glBindTexture ( GL_TEXTURE_2D,
                            mOpenGLRenderer.GetTextureId ( *std::get<1> ( mMaterial->GetSamplers() [i] ).Cast<Texture>() ) );
            OPENGL_CHECK_ERROR_NO_THROW;
        }
        if ( mMaterial->GetUniformBuffer().size() )
        {
            glBindBuffer ( GL_UNIFORM_BUFFER, mUniformBuffer.GetBufferId() );
            OPENGL_CHECK_ERROR_THROW;
            glBindBufferBase ( GL_UNIFORM_BUFFER, MATERIAL, mUniformBuffer.GetBufferId() );
            OPENGL_CHECK_ERROR_THROW;
        }
    }
}
