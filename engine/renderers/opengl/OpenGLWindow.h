/*
Copyright (C) 2017-2019 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_OPENGLWINDOW_H
#define AEONGAMES_OPENGLWINDOW_H

#include <cstdint>
#include <vector>
#include <mutex>
#include "aeongames/Window.h"
#include "OpenGLFunctions.h"
#include "OpenGLBuffer.h"
#include "OpenGLMaterial.h"
#include "OpenGLMemoryPoolBuffer.h"

namespace AeonGames
{
    class Buffer;
    class OpenGLRenderer;
    class OpenGLWindow : public Window
    {
    public:
        OpenGLWindow ( const OpenGLRenderer& aOpenGLRenderer, int32_t aX, int32_t aY, uint32_t aWidth, uint32_t aHeight, bool aFullScreen );
        OpenGLWindow ( const OpenGLRenderer& aOpenGLRenderer, void* aWindowId );
        ~OpenGLWindow() final;
        void* GetWindowId() const;
        void OnResizeViewport ( int32_t aX, int32_t aY, uint32_t aWidth, uint32_t aHeight ) final;
        void BeginRender() final;
        void EndRender() final;
        void Render (   const Matrix4x4& aModelMatrix,
                        const Mesh& aMesh,
                        const Pipeline& aPipeline,
                        const Material* aMaterial = nullptr,
                        const BufferAccessor* aSkeleton = nullptr,
                        uint32_t aVertexStart = 0,
                        uint32_t aVertexCount = 0xffffffff,
                        uint32_t aInstanceCount = 1,
                        uint32_t aFirstInstance = 0 ) const final;
        BufferAccessor AllocateSingleFrameUniformMemory ( size_t aSize ) final;
        const GLuint GetMatricesBuffer() const;
    private:
        void OnSetProjectionMatrix() final;
        void OnSetViewMatrix() final;
        void InitializePlatform();
        void InitializeCommon();
        void FinalizeCommon();
        void FinalizePlatform();
        const OpenGLRenderer& mOpenGLRenderer;
        mutable void* mDeviceContext{};
        GLuint mVAO {};
        GLuint mFBO {};
        GLuint mColorBuffer {};
        GLuint mRBO {};
        GLuint mProgram{};
        OpenGLBuffer mScreenQuad{};
        mutable OpenGLMaterial mMatrices{};
        OpenGLMemoryPoolBuffer mMemoryPoolBuffer;
        bool mOwnsWindowId{ false };
        bool mFullScreen{ false };
    };
}
#endif
