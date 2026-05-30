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
#include "aeongames/Frustum.hpp"
#include "aeongames/AABB.hpp"
#include "aeongames/Pipeline.hpp"
#include "aeongames/Material.hpp"
#include "aeongames/Mesh.hpp"
#include "aeongames/LogLevel.hpp"
#include "aeongames/Node.hpp"
#include "aeongames/Scene.hpp"
#include "aeongames/Material.hpp"
#include "aeongames/GpuLight.hpp"
#include "aeongames/MemoryPool.hpp" ///<- This is here just for the literals
#include "OpenGLWindow.hpp"
#include "OpenGLRenderer.hpp"
#include "OpenGLFunctions.hpp"
#include <sstream>
#include <iostream>
#include <algorithm>
#include <array>
#include <utility>
#include <cstring>
#include <cstdlib>

namespace AeonGames
{
    void OpenGLWindow::Initialize()
    {
        mMatrices.Initialize ( sizeof ( float ) * 16 * 3, GL_DYNAMIC_DRAW );
        // Per-frame Lights SSBO: 16-byte header (count) + MAX_LIGHTS_PER_FRAME
        // records. A storage buffer, not a UBO, because the light cap is large.
        // The header is zeroed so an empty scene draws unlit (count == 0) safely.
        {
            const GpuLightsHeader empty_header{};
            mLights.Initialize ( static_cast<GLsizei> ( GpuLightsBufferSize ), GL_DYNAMIC_DRAW, nullptr );
            mLights.WriteMemory ( 0, sizeof ( empty_header ), &empty_header );
        }
        // ClusterParams UBO: clustered-shading grid + inverse projection.
        // Zero/identity-initialized; populated on the first SetProjectionMatrix.
        {
            GpuClusterParams empty{};
            mClusterParams.Initialize ( sizeof ( GpuClusterParams ), GL_DYNAMIC_DRAW, &empty );
        }
        glBlendFunc ( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
        OPENGL_CHECK_ERROR_THROW;
        glEnable ( GL_BLEND );
        OPENGL_CHECK_ERROR_THROW;
        glDepthFunc ( GL_LESS );
        OPENGL_CHECK_ERROR_THROW;
        glEnable ( GL_DEPTH_TEST );
        OPENGL_CHECK_ERROR_THROW;
        glCullFace ( GL_BACK );
        OPENGL_CHECK_ERROR_THROW;
        glEnable ( GL_CULL_FACE );
        OPENGL_CHECK_ERROR_THROW;
        /// @todo Initial clear color should be configurable.
        glClearColor ( 0.5f, 0.5f, 0.5f, 1.0f );
        OPENGL_CHECK_ERROR_THROW;
        glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        OPENGL_CHECK_ERROR_THROW;
    }


    void OpenGLWindow::SetClearColor ( float R, float G, float B, float A )
    {
#if defined(_WIN32)
        mOpenGLRenderer.MakeCurrent ( mDeviceContext );
#elif defined(__unix__)
        mOpenGLRenderer.MakeCurrent ( mWindowId );
#endif
        glClearColor ( R, G, B, A );
    }

    static GLenum GetIndexType ( const Mesh& aMesh )
    {
        switch ( aMesh.GetIndexSize() )
        {
        case 1:
            return GL_UNSIGNED_BYTE;
        case 2:
            return GL_UNSIGNED_SHORT;
        case 4:
            return GL_UNSIGNED_INT;
        };
        std::cout << LogLevel::Error << "Invalid Index Size." << std::endl;
        throw std::runtime_error ( "Invalid Index Size." );
    }

    static const std::unordered_map<Topology, GLenum> TopologyMap
    {
        {POINT_LIST, GL_POINTS},
        {LINE_STRIP, GL_LINE_STRIP},
        {LINE_LIST, GL_LINES},
        {TRIANGLE_STRIP, GL_TRIANGLE_STRIP},
        {TRIANGLE_FAN, GL_TRIANGLE_FAN},
        {TRIANGLE_LIST, GL_TRIANGLES},
        {LINE_LIST_WITH_ADJACENCY, GL_LINES_ADJACENCY},
        {LINE_STRIP_WITH_ADJACENCY, GL_LINE_STRIP_ADJACENCY},
        {TRIANGLE_LIST_WITH_ADJACENCY, GL_TRIANGLES_ADJACENCY},
        {TRIANGLE_STRIP_WITH_ADJACENCY, GL_TRIANGLE_STRIP_ADJACENCY},
        {PATCH_LIST, GL_PATCHES},
    };

#if defined(__unix__)
    OpenGLWindow::OpenGLWindow ( OpenGLRenderer&  aOpenGLRenderer, Display* aDisplay, ::Window aWindow ) :
        mOpenGLRenderer{ aOpenGLRenderer },
        mDisplay{aDisplay},
        mWindowId{aWindow},
        mFrameBuffer{},
        mMemoryPoolBuffer{aOpenGLRenderer},
        mStorageMemoryPoolBuffer{aOpenGLRenderer}
    {
        mOpenGLRenderer.MakeCurrent ( mWindowId );
        mFrameBuffer.Initialize();
        mMemoryPoolBuffer.Initialize ( static_cast<GLsizei> ( 8_mb ) );
        mStorageMemoryPoolBuffer.Initialize ( static_cast<GLsizei> ( 8_mb ) );
        XWindowAttributes xwa;
        XGetWindowAttributes ( mDisplay, mWindowId, &xwa );
        glViewport ( xwa.x, xwa.y, xwa.width, xwa.height );
        Initialize();
    }

    OpenGLWindow::OpenGLWindow ( OpenGLWindow&& aOpenGLWindow ) :
        mOpenGLRenderer { aOpenGLWindow.mOpenGLRenderer },
        mFrameBuffer{std::move ( aOpenGLWindow.mFrameBuffer ) },
        mMemoryPoolBuffer{std::move ( aOpenGLWindow.mMemoryPoolBuffer ) },
        mStorageMemoryPoolBuffer{std::move ( aOpenGLWindow.mStorageMemoryPoolBuffer ) },
        mMatrices{std::move ( aOpenGLWindow.mMatrices ) },
        mLights{std::move ( aOpenGLWindow.mLights ) },
        mClusterParams{std::move ( aOpenGLWindow.mClusterParams ) }
    {
        std::swap ( mDisplay, aOpenGLWindow.mDisplay );
        std::swap ( mWindowId, aOpenGLWindow.mWindowId );
        std::swap ( mFrustum, aOpenGLWindow.mFrustum );
        std::swap ( mProjectionMatrix, aOpenGLWindow.mProjectionMatrix );
        std::swap ( mViewMatrix, aOpenGLWindow.mViewMatrix );
    }

    OpenGLWindow::~OpenGLWindow()
    {
        if ( mWindowId != None )
        {
            mOpenGLRenderer.MakeCurrent();
            mMemoryPoolBuffer.Finalize();
            mStorageMemoryPoolBuffer.Finalize();
            mMatrices.Finalize();
            mLights.Finalize();
            mClusterParams.Finalize();
            mFrameBuffer.Finalize();
            mDisplay =  nullptr;
            mWindowId = None;
        }
    }

    void OpenGLWindow::SwapBuffers()
    {
        glXSwapBuffers ( mDisplay, mWindowId );
    }

#elif defined(_WIN32)
    OpenGLWindow::OpenGLWindow ( OpenGLRenderer&  aOpenGLRenderer, HWND aWindow ) :
        mOpenGLRenderer{ aOpenGLRenderer },
        mWindowId{ aWindow },
        mDeviceContext{GetDC ( mWindowId ) },
        //mOverlay{Texture::Format::RGBA, Texture::Type::UNSIGNED_INT_8_8_8_8_REV},
        mFrameBuffer{},
        mMemoryPoolBuffer{aOpenGLRenderer},
        mStorageMemoryPoolBuffer{aOpenGLRenderer}
    {
        RECT rect{};
        GetWindowRect ( mWindowId, &rect );
        PIXELFORMATDESCRIPTOR pfd{};
        pfd.nSize = sizeof ( PIXELFORMATDESCRIPTOR );
        pfd.nVersion = 1;
        pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
        pfd.iPixelType = PFD_TYPE_RGBA;
        pfd.cColorBits = 32;
        pfd.cDepthBits = 32;
        pfd.iLayerType = PFD_MAIN_PLANE;
        int pf = ChoosePixelFormat ( mDeviceContext, &pfd );
        SetPixelFormat ( mDeviceContext, pf, &pfd );
        mOpenGLRenderer.MakeCurrent ( mDeviceContext );
        mFrameBuffer.Initialize();
        mMemoryPoolBuffer.Initialize ( static_cast<GLsizei> ( 8_mb ) );
        mStorageMemoryPoolBuffer.Initialize ( static_cast<GLsizei> ( 8_mb ) );
        glViewport ( rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top );
        OPENGL_CHECK_ERROR_THROW;
        Initialize();
    }

    OpenGLWindow::OpenGLWindow ( OpenGLWindow&& aOpenGLWindow ) :
        mOpenGLRenderer { aOpenGLWindow.mOpenGLRenderer },
        mFrameBuffer{std::move ( aOpenGLWindow.mFrameBuffer ) },
        mMemoryPoolBuffer{std::move ( aOpenGLWindow.mMemoryPoolBuffer ) },
        mStorageMemoryPoolBuffer{std::move ( aOpenGLWindow.mStorageMemoryPoolBuffer ) },
        mMatrices{std::move ( aOpenGLWindow.mMatrices ) },
        mLights{std::move ( aOpenGLWindow.mLights ) },
        mClusterParams{std::move ( aOpenGLWindow.mClusterParams ) }
    {
        std::swap ( mWindowId, aOpenGLWindow.mWindowId );
        std::swap ( mFrustum, aOpenGLWindow.mFrustum );
        std::swap ( mDeviceContext, aOpenGLWindow.mDeviceContext );
        std::swap ( mProjectionMatrix, aOpenGLWindow.mProjectionMatrix );
        std::swap ( mViewMatrix, aOpenGLWindow.mViewMatrix );
        std::swap ( mFrameLightGrid, aOpenGLWindow.mFrameLightGrid );
        std::swap ( mFrameLightIndexList, aOpenGLWindow.mFrameLightIndexList );
        std::swap ( mViewportWidth, aOpenGLWindow.mViewportWidth );
        std::swap ( mViewportHeight, aOpenGLWindow.mViewportHeight );
    }

    OpenGLWindow::~OpenGLWindow()
    {
        if ( mWindowId != nullptr )
        {
            mOpenGLRenderer.MakeCurrent();
            mMemoryPoolBuffer.Finalize();
            mStorageMemoryPoolBuffer.Finalize();
            mMatrices.Finalize();
            mLights.Finalize();
            mClusterParams.Finalize();
            mFrameBuffer.Finalize();
            ReleaseDC ( mWindowId, mDeviceContext );
            mWindowId = nullptr;
        }
    }

    void OpenGLWindow::SwapBuffers()
    {
        ::SwapBuffers ( mDeviceContext );
    }

#endif

    void OpenGLWindow::Render ( const Matrix4x4& aModelMatrix,
                                const Mesh& aMesh,
                                const Pipeline& aPipeline,
                                const Material* aMaterial,
                                const BufferAccessor* aSkeleton,
                                Topology aTopology,
                                uint32_t aVertexStart,
                                uint32_t aVertexCount,
                                uint32_t aInstanceCount,
                                uint32_t aFirstInstance ) const
    {
        mOpenGLRenderer.BindPipeline ( aPipeline );

        mMatrices.WriteMemory ( 0, sizeof ( float ) * 16, aModelMatrix.GetMatrix4x4() );
        mOpenGLRenderer.SetMatrices ( mMatrices );
        mOpenGLRenderer.SetLights ( mLights );
        mOpenGLRenderer.SetClusterParams ( mClusterParams );
        // Clustered Forward+ light lists, produced by the lighting compute
        // pipeline in BeginRender. Bound by name-CRC; BindStorageBuffer
        // silently skips pipelines that don't declare these blocks.
        if ( mFrameLightGrid.GetMemoryPoolBuffer() != nullptr )
        {
            mOpenGLRenderer.BindStorageBuffer ( Mesh::BindingLocations::LIGHT_GRID, mFrameLightGrid );
            mOpenGLRenderer.BindStorageBuffer ( Mesh::BindingLocations::LIGHT_INDEX_LIST, mFrameLightIndexList );
        }

        if ( aMaterial )
        {
            mOpenGLRenderer.SetMaterial ( *aMaterial );
        }

        if ( aSkeleton )
        {
            mOpenGLRenderer.SetSkeleton ( *aSkeleton );
        }

        /// @todo Add some sort of way to make use of the aFirstInstance parameter
        mOpenGLRenderer.BindMesh ( aMesh );
        if ( aMesh.GetIndexCount() )
        {
            glDrawElementsInstanced ( TopologyMap.at ( aTopology ), ( aVertexCount != 0xffffffff ) ? aVertexCount : aMesh.GetIndexCount(),
                                      GetIndexType ( aMesh ), reinterpret_cast<const uint8_t*> ( 0 ) + aMesh.GetIndexSize() *aVertexStart, aInstanceCount );
            OPENGL_CHECK_ERROR_NO_THROW;
        }
        else
        {
            glDrawArraysInstanced ( TopologyMap.at ( aTopology ), aVertexStart, ( aVertexCount != 0xffffffff ) ? aVertexCount : aMesh.GetVertexCount(), aInstanceCount );
            OPENGL_CHECK_ERROR_NO_THROW;
        }
    }

    BufferAccessor OpenGLWindow::AllocateSingleFrameUniformMemory ( size_t aSize )
    {
        return mMemoryPoolBuffer.Allocate ( aSize );
    }

    BufferAccessor OpenGLWindow::AllocateSingleFrameStorageMemory ( size_t aSize )
    {
        return mStorageMemoryPoolBuffer.Allocate ( aSize );
    }

    void OpenGLWindow::BeginRender ( const Pipeline* aComputePipeline )
    {
        BeginFrame();
        if ( aComputePipeline != nullptr )
        {
            DispatchClustering ( *aComputePipeline );
        }
        else
        {
            // No clustering this frame: still hand the clustered fragment
            // shader valid, empty light buffers so it reads zero lights per
            // cluster instead of sampling an unbound buffer.
            mFrameLightGrid = mStorageMemoryPoolBuffer.Allocate ( CLUSTER_COUNT * sizeof ( GpuLightGridCell ) );
            mFrameLightIndexList = mStorageMemoryPoolBuffer.Allocate ( CLUSTER_COUNT * MAX_LIGHTS_PER_CLUSTER * sizeof ( uint32_t ) );
            if ( void * grid = mFrameLightGrid.Map() )
            {
                std::memset ( grid, 0, CLUSTER_COUNT * sizeof ( GpuLightGridCell ) );
                mFrameLightGrid.Unmap();
            }
        }
        BeginRenderPass();
    }

    void OpenGLWindow::DispatchClustering ( const Pipeline& aComputePipeline )
    {
        // One workgroup per 64 clusters (clustering compute stages use local_size_x=64).
        constexpr uint32_t group_count = ( CLUSTER_COUNT + 63u ) / 64u;

        // Allocate the per-frame clustering buffers once. They are bound for
        // every compute stage; reflection silently drops the blocks a given
        // stage does not declare.
        BufferAccessor cluster_aabbs = mStorageMemoryPoolBuffer.Allocate ( CLUSTER_COUNT * sizeof ( GpuClusterAABB ) );
        mFrameLightGrid = mStorageMemoryPoolBuffer.Allocate ( CLUSTER_COUNT * sizeof ( GpuLightGridCell ) );
        mFrameLightIndexList = mStorageMemoryPoolBuffer.Allocate ( CLUSTER_COUNT * MAX_LIGHTS_PER_CLUSTER * sizeof ( uint32_t ) );

        const StorageBufferBinding bindings[]
        {
            { Mesh::BindingLocations::CLUSTER_AABBS, &cluster_aabbs },
            { Mesh::BindingLocations::LIGHT_GRID, &mFrameLightGrid },
            { Mesh::BindingLocations::LIGHT_INDEX_LIST, &mFrameLightIndexList },
        };

        // Dispatch every compute stage in declared order, inserting a barrier
        // between stages so each stage observes the previous stage's writes.
        const uint32_t stage_count = aComputePipeline.GetComputeStageCount();
        for ( uint32_t stage = 0; stage < stage_count; ++stage )
        {
            Dispatch ( aComputePipeline, group_count, 1, 1, bindings, stage );
            Barrier();
        }
    }

    void OpenGLWindow::BeginFrame()
    {
#if defined(_WIN32)
        mOpenGLRenderer.MakeCurrent ( mDeviceContext );
#elif defined(__unix__)
        mOpenGLRenderer.MakeCurrent ( mWindowId );
#endif
        mFrameBuffer.Bind();
    }

    void OpenGLWindow::BeginRenderPass()
    {
        glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        glEnable ( GL_DEPTH_TEST );
    }

    void OpenGLWindow::Dispatch ( const Pipeline& aPipeline,
                                  uint32_t aGroupCountX,
                                  uint32_t aGroupCountY,
                                  uint32_t aGroupCountZ,
                                  std::span<const StorageBufferBinding> aStorageBuffers,
                                  uint32_t aComputeStageIndex ) const
    {
        mOpenGLRenderer.BindComputePipeline ( aPipeline, aComputeStageIndex );
        mOpenGLRenderer.SetMatrices ( mMatrices );
        mOpenGLRenderer.SetLights ( mLights );
        mOpenGLRenderer.SetClusterParams ( mClusterParams );
        for ( const StorageBufferBinding& storage_buffer : aStorageBuffers )
        {
            if ( storage_buffer.mBuffer != nullptr )
            {
                mOpenGLRenderer.BindStorageBuffer ( storage_buffer.mBinding, *storage_buffer.mBuffer );
            }
        }
        glDispatchCompute ( aGroupCountX, aGroupCountY, aGroupCountZ );
        OPENGL_CHECK_ERROR_NO_THROW;
    }

    void OpenGLWindow::Barrier() const
    {
        glMemoryBarrier ( GL_SHADER_STORAGE_BARRIER_BIT );
        OPENGL_CHECK_ERROR_NO_THROW;
    }

    void OpenGLWindow::EndRender()
    {
        mFrameBuffer.Unbind();
        glClear ( GL_COLOR_BUFFER_BIT );
        OPENGL_CHECK_ERROR_NO_THROW;
        glDisable ( GL_DEPTH_TEST );
        OPENGL_CHECK_ERROR_NO_THROW;
        GLint dims[4] = {0};
        glGetIntegerv ( GL_VIEWPORT, dims );
        OPENGL_CHECK_ERROR_NO_THROW;
        glBlitNamedFramebuffer (
            mFrameBuffer.GetFBO(),
            0,
            dims[0],
            dims[1],
            dims[2],
            dims[3],
            dims[0],
            dims[1],
            dims[2],
            dims[3],
            GL_COLOR_BUFFER_BIT,
            GL_LINEAR );
        OPENGL_CHECK_ERROR_NO_THROW;
#if 0
        /* Bind and render overlay texture */
        glUseProgram ( mOpenGLRenderer.GetOverlayProgram() );
        OPENGL_CHECK_ERROR_NO_THROW;
        glBindBuffer ( GL_ARRAY_BUFFER, mOpenGLRenderer.GetOverlayQuad() );
        OPENGL_CHECK_ERROR_NO_THROW;
        glBindTexture ( GL_TEXTURE_2D, mOverlay.GetTextureId() );
        OPENGL_CHECK_ERROR_NO_THROW;
        glEnableVertexAttribArray ( 0 );
        OPENGL_CHECK_ERROR_NO_THROW;
        glVertexAttribPointer ( 0, 2, GL_FLOAT, GL_FALSE, sizeof ( float ) * 4, 0 );
        OPENGL_CHECK_ERROR_NO_THROW;
        glEnableVertexAttribArray ( 1 );
        OPENGL_CHECK_ERROR_NO_THROW;
        glVertexAttribPointer ( 1, 2, GL_FLOAT, GL_FALSE, sizeof ( float ) * 4, reinterpret_cast<const void*> ( sizeof ( float ) * 2 ) );
        OPENGL_CHECK_ERROR_NO_THROW;
        glDrawArrays ( GL_TRIANGLE_FAN, 0, 4 );
        OPENGL_CHECK_ERROR_NO_THROW;
#endif
        SwapBuffers();
        mMemoryPoolBuffer.Reset();
        mStorageMemoryPoolBuffer.Reset();
    }
    void OpenGLWindow::WriteOverlayPixels ( int32_t aXOffset, int32_t aYOffset, uint32_t aWidth, uint32_t aHeight, Texture::Format aFormat, Texture::Type aType, const uint8_t* aPixels )
    {
        //mOverlay.WritePixels ( aXOffset, aYOffset, aWidth, aHeight, aFormat, aType, aPixels );
    }

    void OpenGLWindow::SetProjectionMatrix ( const Matrix4x4& aMatrix )
    {
        mProjectionMatrix = aMatrix * Matrix4x4
        {
            // Flip Matrix
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, -1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        };
        mFrustum = mProjectionMatrix * mViewMatrix;
        mMatrices.WriteMemory ( sizeof ( float ) * 16, sizeof ( float ) * 16, mProjectionMatrix.GetMatrix4x4() );
        UpdateClusterParams();
    }

    void OpenGLWindow::UpdateClusterParams()
    {
        // Build from the render-space (post Z-flip) projection so the cluster
        // AABBs live in the same view space the fragments are shaded in.
        GpuClusterParams params{};
        params.inverse_projection = mProjectionMatrix.GetInvertedMatrix4x4();
        params.screen[0] = static_cast<float> ( mViewportWidth );
        params.screen[1] = static_cast<float> ( mViewportHeight );
        // Debug cluster heatmap toggle, read once from the environment.
        static const bool heatmap = []
        {
            const char* value = std::getenv ( "AEON_CLUSTER_HEATMAP" );
            return value != nullptr && value[0] != '\0' && value[0] != '0';
        } ();
        params.screen[2] = heatmap ? 1.0f : 0.0f;
        mClusterParams.WriteMemory ( 0, sizeof ( GpuClusterParams ), &params );
    }

    void OpenGLWindow::SetViewMatrix ( const Matrix4x4& aMatrix )
    {
        mViewMatrix = aMatrix;
        mFrustum = mProjectionMatrix * mViewMatrix;
        mMatrices.WriteMemory ( sizeof ( float ) * 16 * 2, sizeof ( float ) * 16, mViewMatrix.GetMatrix4x4() );
    }

    void OpenGLWindow::SetLights ( std::span<const GpuLight> aLights )
    {
        // Stream the light records into the SSBO: write the 16-byte header
        // (count) then the tightly packed array. Lights past
        // MAX_LIGHTS_PER_FRAME are silently truncated.
        const std::size_t capped = std::min<std::size_t> ( aLights.size(), MAX_LIGHTS_PER_FRAME );
        const GpuLightsHeader header{ static_cast<uint32_t> ( capped ), { 0, 0, 0 } };
#if defined(_WIN32)
        mOpenGLRenderer.MakeCurrent ( mDeviceContext );
#elif defined(__unix__)
        mOpenGLRenderer.MakeCurrent ( mWindowId );
#endif
        mLights.WriteMemory ( 0, sizeof ( header ), &header );
        if ( capped > 0 )
        {
            mLights.WriteMemory ( sizeof ( GpuLightsHeader ), capped * sizeof ( GpuLight ), aLights.data() );
        }
    }

    const Matrix4x4 & OpenGLWindow::GetProjectionMatrix() const
    {
        return mProjectionMatrix;
    }
    const Matrix4x4 & OpenGLWindow::GetViewMatrix() const
    {
        return mViewMatrix;
    }

    const Frustum & OpenGLWindow::GetFrustum() const
    {
        return mFrustum;
    }

    void OpenGLWindow::ResizeViewport ( int32_t aX, int32_t aY, uint32_t aWidth, uint32_t aHeight )
    {
        if ( aWidth && aHeight )
        {
#if defined(_WIN32)
            mOpenGLRenderer.MakeCurrent ( mDeviceContext );
#elif defined(__unix__)
            mOpenGLRenderer.MakeCurrent ( mWindowId );
#endif
            OPENGL_CHECK_ERROR_NO_THROW;
            glViewport ( aX, aY, aWidth, aHeight );
            OPENGL_CHECK_ERROR_THROW;
            mFrameBuffer.Resize ( aWidth, aHeight );
            //mOverlay.Resize ( aWidth, aHeight );
            mViewportWidth = aWidth;
            mViewportHeight = aHeight;
            UpdateClusterParams();
        }
    }
}
