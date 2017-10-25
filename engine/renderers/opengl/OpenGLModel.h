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
#include "aeongames/RenderModel.h"

namespace AeonGames
{
    class Model;
    class ModelInstance;
    class OpenGLRenderer;
    class OpenGLPipeline;
    class OpenGLMaterial;
    class OpenGLMesh;
    class OpenGLSkeleton;
    class OpenGLModel : public RenderModel
    {
    public:
        OpenGLModel ( const std::shared_ptr<const Model> aModel, const std::shared_ptr<const OpenGLRenderer> aOpenGLRenderer );
        virtual ~OpenGLModel();
        void Render ( const std::shared_ptr<const ModelInstance>& aInstance ) const final;
    private:
        void Initialize();
        void Finalize();
        std::shared_ptr<const Model> mModel;
        std::shared_ptr<const OpenGLRenderer> mOpenGLRenderer;
        std::shared_ptr<OpenGLSkeleton> mSkeleton;
        std::vector<std::tuple<
        std::shared_ptr<OpenGLPipeline>,
            std::shared_ptr<OpenGLMaterial>,
            std::shared_ptr<OpenGLMesh>>> mAssemblies;
    };
}
#endif
