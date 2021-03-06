/*
Copyright (C) 2017-2021 Rodrigo Jose Hernandez Cordoba

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
#include "aeongames/CommonWindow.h"
#include "OpenGLFunctions.h"
#include "OpenGLBuffer.h"
#include "OpenGLFrameBuffer.h"
#include "OpenGLMemoryPoolBuffer.h"

namespace AeonGames
{
    class Buffer;
    class OpenGLRenderer;
    class OpenGLWindow : public CommonWindow
    {
    public:
        OpenGLWindow ( OpenGLRenderer& aOpenGLRenderer, int32_t aX, int32_t aY, uint32_t aWidth, uint32_t aHeight, bool aFullScreen );
        OpenGLWindow ( OpenGLRenderer& aOpenGLRenderer );
        ~OpenGLWindow();
        void* GetWindowId() const;
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
        void WriteOverlayPixels ( int32_t aXOffset, int32_t aYOffset, uint32_t aWidth, uint32_t aHeight, Texture::Format aFormat, Texture::Type aType, const uint8_t* aPixels ) final;
        void SetProjectionMatrix ( const Matrix4x4& aMatrix ) final;
        void SetViewMatrix ( const Matrix4x4& aMatrix ) final;
        const Matrix4x4 & GetProjectionMatrix() const final;
        const Matrix4x4 & GetViewMatrix() const final;
        void ResizeViewport ( int32_t aX, int32_t aY, uint32_t aWidth, uint32_t aHeight ) final;
    protected:
        OpenGLRenderer& mOpenGLRenderer;
        Matrix4x4 mProjectionMatrix{};
        Matrix4x4 mViewMatrix{};
        //OpenGLTexture mOverlay{Texture::Format::RGBA, Texture::Type::UNSIGNED_INT_8_8_8_8_REV};
        void Initialize();
        void Finalize();
    private:
        virtual bool MakeCurrent() = 0;
        virtual void SwapBuffers() = 0;
        OpenGLFrameBuffer mFrameBuffer {};
        OpenGLMemoryPoolBuffer mMemoryPoolBuffer;
    };
}
#endif
