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
#ifndef AEONGAMES_OPENGLRENDERER_H
#define AEONGAMES_OPENGLRENDERER_H

#include <unordered_map>
#include "aeongames/Renderer.h"
#include "aeongames/Transform.h"
#include "aeongames/Matrix4x4.h"
#include "OpenGLFunctions.h"
#include "OpenGLBuffer.h"
#ifdef Status
#undef Status
#endif
#include "aeongames/ProtoBufClasses.h"

namespace AeonGames
{
    class Mesh;
    class Pipeline;
    class Material;
    class UniformBuffer;
    class OpenGLMesh;
    class OpenGLPipeline;
    class OpenGLModel;
    class OpenGLMaterial;
    class OpenGLRenderer : public Renderer
    {
    public:
        OpenGLRenderer();
        virtual ~OpenGLRenderer() = 0;
        std::unique_ptr<Window> CreateWindowProxy ( void* aWindowId ) const final;
        std::unique_ptr<Window> CreateWindowInstance ( int32_t aX, int32_t aY, uint32_t aWidth, uint32_t aHeight, bool aFullScreen ) const final;
        void LoadMesh ( const Mesh& aMesh ) final;
        void UnloadMesh ( const Mesh& aMesh ) final;
        std::unique_ptr<Pipeline> CreatePipeline ( uint32_t aPath ) final;
        std::unique_ptr<Material> CreateMaterial ( uint32_t aPath ) final;
        std::unique_ptr<Texture> CreateTexture ( uint32_t aPath ) const final;
        std::unique_ptr<Buffer> CreateBuffer ( size_t aSize, const void* aData = nullptr ) const final;
        virtual bool MakeCurrent() const = 0;
        virtual void* GetContext() const = 0;
        GLuint GetVertexArrayObject() const;
        GLuint GetOverlayProgram() const;
        GLuint GetOverlayQuad() const;
        void BindMesh ( const Mesh& aMesh ) const final;
        void UsePipeline ( const Pipeline& aPipeline, const Material* aMaterial = nullptr, const BufferAccessor* aSkeletonBuffer = nullptr ) const final;
        void LoadPipeline ( const Pipeline& aPipeline ) final;
        void UnloadPipeline ( const Pipeline& aPipeline ) final;
        void LoadMaterial ( const Material& aMaterial ) final;
        void UnloadMaterial ( const Material& aMaterial ) final;
    protected:
        void InitializeOverlay();
        void FinalizeOverlay();
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
        std::unordered_map<size_t, std::vector<OpenGLBuffer>> mBufferStore{};
        std::unordered_map<size_t, GLuint> mProgramStore{};
    };
}
#endif
