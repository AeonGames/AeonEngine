/*
Copyright (C) 2016-2022,2025,2026 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_OPENGLRENDERER_HPP
#define AEONGAMES_OPENGLRENDERER_HPP

#include <unordered_map>
#include "aeongames/Renderer.hpp"
#include "aeongames/Transform.hpp"
#include "aeongames/Matrix4x4.hpp"
#include "OpenGLFunctions.hpp"
#include "OpenGLBuffer.hpp"
#include "OpenGLMesh.hpp"
#include "OpenGLPipeline.hpp"
#include "OpenGLMaterial.hpp"
#include "OpenGLTexture.hpp"
#include "OpenGLWindow.hpp"
#ifdef Status
#undef Status
#endif
#include "aeongames/ProtoBufClasses.hpp"

namespace AeonGames
{
    class Mesh;
    class Pipeline;
    class Material;
    class UniformBuffer;
    class OpenGLMesh;
    class OpenGLModel;
    class OpenGLRenderer : public Renderer
    {
    public:
        OpenGLRenderer ( void* aWindow );
        ~OpenGLRenderer();
        void LoadMesh ( const Mesh& aMesh ) final;
        void UnloadMesh ( const Mesh& aMesh ) final;
        void* GetContext() const;
        GLuint GetVertexArrayObject() const;
        GLuint GetOverlayProgram() const;
        GLuint GetOverlayQuad() const;

        void BindMesh ( const Mesh& aMesh );
        void BindPipeline ( const Pipeline& aPipeline );
        void SetMaterial ( const Material& aMaterial );

        void SetSkeleton ( const BufferAccessor& aSkeletonBuffer ) const;
        void SetMatrices ( const OpenGLBuffer& aMatricesBuffer ) const;
        void LoadPipeline ( const Pipeline& aPipeline ) final;
        void UnloadPipeline ( const Pipeline& aPipeline ) final;
        void LoadMaterial ( const Material& aMaterial ) final;
        void UnloadMaterial ( const Material& aMaterial ) final;
        void LoadTexture ( const Texture& aTexture ) final;
        void UnloadTexture ( const Texture& aTexture ) final;
        GLuint GetTextureId ( const Texture& aTexture );

        void AttachWindow ( void* aWindowId ) final;
        void DetachWindow ( void* aWindowId ) final;
        void SetProjectionMatrix ( void* aWindowId, const Matrix4x4& aMatrix ) final;
        void SetViewMatrix ( void* aWindowId, const Matrix4x4& aMatrix ) final;
        void SetClearColor ( void* aWindowId, float R, float G, float B, float A ) final;
        void ResizeViewport ( void* aWindowId, int32_t aX, int32_t aY, uint32_t aWidth, uint32_t aHeight ) final;
        void BeginRender ( void* aWindowId ) final;
        void EndRender ( void* aWindowId ) final;
        void Render ( void* aWindowId,
                      const Matrix4x4& aModelMatrix,
                      const Mesh& aMesh,
                      const Pipeline& aPipeline,
                      const Material* aMaterial = nullptr,
                      const BufferAccessor* aSkeleton = nullptr,
                      Topology aTopology = Topology::TRIANGLE_LIST,
                      uint32_t aVertexStart = 0,
                      uint32_t aVertexCount = 0xffffffff,
                      uint32_t aInstanceCount = 1,
                      uint32_t aFirstInstance = 0 ) const final;
        const Frustum& GetFrustum ( void* aWindowId ) const final;
        BufferAccessor AllocateSingleFrameUniformMemory ( void* aWindowId, size_t aSize ) final;
#if defined(_WIN32)
        bool MakeCurrent ( HDC aDeviceContext = nullptr );
#elif defined(__unix__)
        Display* GetDisplay() const;
        bool MakeCurrent ( ::Window aWindow = None );
#endif
    protected:
        void InitializeOverlay();
        void FinalizeOverlay();
#if defined(_WIN32)
        HWND mWindowId {};
        HDC mDeviceContext{};
        HGLRC mOpenGLContext{};
#elif defined(__unix__)
        GLXContext mOpenGLContext {None};
#endif
        /// General VAO
        GLuint mVertexArrayObject{};
        /** \addtogroup Overlay functionality.
         * Both the shader program and the buffer descriving the window/screen quad are
         * pretty much constant and usable without modifications by any Opengl window,
         * so it makes sence that they reside inside the renderer object from which the windows
         * are creatted.
         *
         @{*/
        /// Raw overlay shader program.
        GLuint mOverlayProgram{};
        /// Overlay quadrilateral.
        OpenGLBuffer mOverlayQuad{};
        /**@}*/
        OpenGLPipeline* mCurrentPipeline{nullptr};
        std::unordered_map<size_t, OpenGLPipeline> mPipelineStore{};
        std::unordered_map<size_t, OpenGLMaterial> mMaterialStore{};
        std::unordered_map<size_t, OpenGLMesh> mMeshStore{};
        std::unordered_map<size_t, OpenGLTexture> mTextureStore{};
        std::unordered_map<void*, OpenGLWindow> mWindowStore{};
    private:
        static std::atomic<size_t> mRendererCount;
#if defined(__unix__)
        static Display* mDisplay;
#endif
    };
}
#endif
