/*
Copyright (C) 2017-2022,2025,2026 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_OPENGLWINDOW_HPP
#define AEONGAMES_OPENGLWINDOW_HPP

#include <cstdint>
#include <vector>
#include <mutex>
#include "aeongames/Platform.hpp"
#include "aeongames/Matrix4x4.hpp"
#include "aeongames/Frustum.hpp"
#include "aeongames/Texture.hpp"
#include "OpenGLFunctions.hpp"
#include "OpenGLBuffer.hpp"
#include "OpenGLFrameBuffer.hpp"
#include "OpenGLMemoryPoolBuffer.hpp"

namespace AeonGames
{
    class Buffer;
    class OpenGLRenderer;
    class OpenGLWindow
    {
    public:
#if defined(_WIN32)
        OpenGLWindow ( OpenGLRenderer& aOpenGLRenderer, HWND aWindowId );
#elif defined(__unix__)
        OpenGLWindow ( OpenGLRenderer& aOpenGLRenderer, Display* aDisplay, ::Window aWindowId );
#endif
        OpenGLWindow ( OpenGLWindow&& aOpenGLWindow );
        OpenGLWindow ( const OpenGLWindow& aOpenGLWindow ) = delete;
        OpenGLWindow& operator= ( const OpenGLWindow& aOpenGLWindow ) = delete;
        OpenGLWindow& operator= ( OpenGLWindow&& aOpenGLWindow ) = delete;
        ~OpenGLWindow();
        void* GetWindowId() const;
        void BeginRender();
        void EndRender();
        void Render (   const Matrix4x4& aModelMatrix,
                        const Mesh& aMesh,
                        const Pipeline& aPipeline,
                        const Material* aMaterial = nullptr,
                        const BufferAccessor* aSkeleton = nullptr,
                        Topology aTopology = Topology::TRIANGLE_LIST,
                        uint32_t aVertexStart = 0,
                        uint32_t aVertexCount = 0xffffffff,
                        uint32_t aInstanceCount = 1,
                        uint32_t aFirstInstance = 0 ) const;
        BufferAccessor AllocateSingleFrameUniformMemory ( size_t aSize );
        void WriteOverlayPixels ( int32_t aXOffset, int32_t aYOffset, uint32_t aWidth, uint32_t aHeight, Texture::Format aFormat, Texture::Type aType, const uint8_t* aPixels );
        void SetProjectionMatrix ( const Matrix4x4& aMatrix );
        void SetViewMatrix ( const Matrix4x4& aMatrix );
        void SetClearColor ( float R, float G, float B, float A );
        const Matrix4x4 & GetProjectionMatrix() const;
        const Matrix4x4 & GetViewMatrix() const;
        const Frustum & GetFrustum() const;
        void ResizeViewport ( int32_t aX, int32_t aY, uint32_t aWidth, uint32_t aHeight );
    private:
        void Initialize();
        void SwapBuffers();
        OpenGLRenderer& mOpenGLRenderer;
#if defined(_WIN32)
        HWND mWindowId {};
        HDC mDeviceContext{};
#elif defined(__unix__)
        Display* mDisplay {};
        ::Window mWindowId{None};
#endif
        Frustum mFrustum {};
        Matrix4x4 mProjectionMatrix{};
        Matrix4x4 mViewMatrix{};
        //OpenGLTexture mOverlay{Texture::Format::RGBA, Texture::Type::UNSIGNED_INT_8_8_8_8_REV};
        OpenGLFrameBuffer mFrameBuffer{};
        OpenGLMemoryPoolBuffer mMemoryPoolBuffer;
        OpenGLBuffer mMatrices{};
    };
}
#endif
