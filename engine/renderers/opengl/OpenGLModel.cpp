/*
Copyright (C) 2016,2017 Rodrigo Jose Hernandez Cordoba

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
#include <cassert>
#include <vector>
#include "aeongames/Model.h"
#include "aeongames/ResourceCache.h"
#include "OpenGLModel.h"
#include "OpenGLProgram.h"
#include "OpenGLMesh.h"

namespace AeonGames
{
    OpenGLModel::OpenGLModel ( const std::shared_ptr<Model> aModel ) :
        mModel ( aModel )
    {
        try
        {
            Initialize();
        }
        catch ( ... )
        {
            Finalize();
            throw;
        }
    }

    OpenGLModel::~OpenGLModel()
    {
        Finalize();
    }

    void OpenGLModel::Render ( GLuint aMatricesBuffer ) const
    {
        /**@todo Revisit, may be a better idea
        to keep a back reference to the renderer
        instead of taking a parameter. */
        mProgram->Use();
        glBindBufferBase ( GL_UNIFORM_BUFFER, 0, aMatricesBuffer );
        OPENGL_CHECK_ERROR_NO_THROW;
        mMesh->Render();
    }

    void OpenGLModel::Initialize()
    {
        mProgram = Get<OpenGLProgram> ( mModel->GetProgram() );
        mMesh = Get<OpenGLMesh> ( mModel->GetMesh() );
    }

    void OpenGLModel::Finalize()
    {
    }
}
