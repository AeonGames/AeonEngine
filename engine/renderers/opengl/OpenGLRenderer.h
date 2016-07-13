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
#ifndef AEONGAMES_OPENGLRENDERER_H
#define AEONGAMES_OPENGLRENDERER_H

#include "aeongames/Renderer.h"
#include <exception>
#include <vector>
#include <memory>
#include "OpenGLFunctions.h"

namespace AeonGames
{
    class Mesh;
    class Program;
    class OpenGLProgram;
    class OpenGLRenderer : public Renderer
    {
    public:
        OpenGLRenderer();
        ~OpenGLRenderer();
        void BeginRender() const override final;
        void EndRender() const override final;
        void Render ( const std::shared_ptr<Mesh>& aMesh, const std::shared_ptr<Program>& aProgram ) const override final;
        std::shared_ptr<Mesh> GetMesh ( const std::string& aFilename ) const override final;
        std::shared_ptr<Program> GetProgram ( const std::string& aFilename ) const override final;
        bool RegisterRenderingWindow ( uintptr_t aWindowId ) override final;
        void UnregisterRenderingWindow ( uintptr_t aWindowId ) override final;
        void Resize ( uintptr_t aWindowId, uint32_t aWidth, uint32_t aHeight ) const override final;
        void SetViewMatrix ( const float aMatrix[16] ) override final;
        void SetProjectionMatrix ( const float aMatrix[16] ) override final;
        void SetModelMatrix ( const float aMatrix[16] ) override final;
    private:
        void Initialize();
        void Finalize();
        void UpdateMatrices();
        GLuint mMatricesBuffer;
        float mMatrices[ ( 16 * 6 ) + ( 12 * 1 )] =
        {
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1,
            // mProjectionMatrix
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1,
            // mModelMatrix
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1,
            // mViewProjectionMatrix
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1,
            // mModelViewMatrix
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1,
            // mModelViewProjectionMatrix
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1,
            /*  mNormalMatrix With Padding,
                this should really be a 3x3 matrix,
                but std140 packing requires 16 byte alignment
                and a mat3 is escentially a vec3[3]*/
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
        };

        float* mViewMatrix = mMatrices + ( 16 * 0 );
        float* mProjectionMatrix = mMatrices + ( 16 * 1 );
        float* mModelMatrix = mMatrices + ( 16 * 2 );
        // Cache Matrices
        float* mViewProjectionMatrix = mMatrices + ( 16 * 3 );
        float* mModelViewMatrix = mMatrices + ( 16 * 4 );
        float* mModelViewProjectionMatrix = mMatrices + ( 16 * 5 );
        float* mNormalMatrix = mMatrices + ( 16 * 6 );

        uintptr_t mWindowId = 0;
#ifdef _WIN32
        HDC mDeviceContext = nullptr;
        HGLRC mOpenGLContext = nullptr;
#else
        Window mWindow = 0;
        Display* mDisplay = nullptr;
        GLXContext mGLXContext = nullptr;
#endif
    };
}
extern "C"
{
    DLL AeonGames::Renderer* CreateRenderer();
    DLL void DestroyRenderer ( AeonGames::Renderer* aRenderer );
}
#endif
