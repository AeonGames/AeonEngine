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
#ifndef AEONGAMES_OPENGLMODEL_H
#define AEONGAMES_OPENGLMODEL_H
#include "OpenGLFunctions.h"

namespace AeonGames
{
    class Model;
    class OpenGLProgram;
    class OpenGLMesh;
    class OpenGLModel
    {
    public:
        OpenGLModel ( const std::shared_ptr<Model> aModel );
        ~OpenGLModel();
        void Render ( GLuint aMatricesBuffer ) const;
    private:
        void Initialize();
        void Finalize();
        const std::shared_ptr<Model> mModel;
        std::shared_ptr<OpenGLProgram> mProgram;
        std::shared_ptr<OpenGLMesh> mMesh;
    };
}
#endif
