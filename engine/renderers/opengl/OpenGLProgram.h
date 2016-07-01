/*
Copyright 2016 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_OPENGLPROGRAM_H
#define AEONGAMES_OPENGLPROGRAM_H
#include <cstdint>
#include <string>
#include "aeongames/Program.h"
#include "OpenGLMaterial.h"
#include "Uniform.h"

namespace AeonGames
{
    class OpenGLProgram : public Program
    {
    public:
        OpenGLProgram ( const std::string& aFilename );
        ~OpenGLProgram();
#if 0
        void SetViewMatrix ( const float aMatrix[16] );
        void SetProjectionMatrix ( const float aMatrix[16] );
        void SetModelMatrix ( const float aMatrix[16] );
        void SetViewProjectionMatrix ( const float aMatrix[16] );
        void SetModelViewMatrix ( const float aMatrix[16] );
        void SetModelViewProjectionMatrix ( const float aMatrix[16] );
        void SetNormalMatrix ( const float aMatrix[9] );
#endif
        void Use() const;
        uint32_t GetMatricesBlockIndex() const;
    private:
        void Initialize();
        void Finalize();
        std::string mFilename;
        uint32_t mProgram;
#if 0
        int mViewMatrixLocation = -1;
        int mProjectionMatrixLocation = -1;
        int mModelMatrixLocation = -1;
        int mViewProjectionMatrixLocation = -1;
        int mModelViewMatrixLocation = -1;
        int mModelViewProjectionMatrixLocation = -1;
        int mNormalMatrixLocation = -1;
#else
        uint32_t mMatricesBlockIndex = 0;
        uint32_t mPropertiesBlockIndex = 0;
#endif
        std::vector<Uniform> mDefaultValues;
    };
}
#endif
