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
#include "aeongames/GpuShadowParams.hpp"
#include "aeongames/MemoryPool.hpp" ///<- This is here just for the literals
#include "OpenGLWindow.hpp"
#include "OpenGLRenderer.hpp"
#include "OpenGLMesh.hpp"
#include "OpenGLBuffer.hpp"
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
    // Texture unit the directional shadow map is bound to during shading. Must
    // match `layout(binding = N) uniform sampler2DShadow ShadowMap;` in the GL
    // path of the shading fragment shaders.
    static constexpr GLuint SHADOW_MAP_TEXTURE_UNIT = 8;
    // Texture unit the spot shadow map array is bound to during shading. Must
    // match `layout(binding = N) uniform sampler2DArrayShadow SpotShadowMap;` in
    // the GL path of the shading fragment shaders.
    static constexpr GLuint SPOT_SHADOW_MAP_TEXTURE_UNIT = 9;
    // Texture unit the point shadow map array is bound to during shading. Must
    // match `layout(binding = N) uniform sampler2DArrayShadow PointShadowMap;`
    // in the GL path of the shading fragment shaders.
    static constexpr GLuint POINT_SHADOW_MAP_TEXTURE_UNIT = 10;

    void OpenGLWindow::Initialize()
    {
        mMatrices.Initialize ( sizeof ( float ) * 16 * 2, GL_DYNAMIC_DRAW );
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
        InitializeShadowMap();
        InitializeSpotShadowMap();
        InitializePointShadowMap();
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
        mClusterParams{std::move ( aOpenGLWindow.mClusterParams ) },
        mShadowParams{std::move ( aOpenGLWindow.mShadowParams ) },
        mSpotShadowParams{std::move ( aOpenGLWindow.mSpotShadowParams ) },
        mSpotShadowDepthScratch{std::move ( aOpenGLWindow.mSpotShadowDepthScratch ) },
        mPointShadowParams{std::move ( aOpenGLWindow.mPointShadowParams ) },
        mPointShadowDepthScratch{std::move ( aOpenGLWindow.mPointShadowDepthScratch ) }
    {
        std::swap ( mDisplay, aOpenGLWindow.mDisplay );
        std::swap ( mWindowId, aOpenGLWindow.mWindowId );
        std::swap ( mFrustum, aOpenGLWindow.mFrustum );
        std::swap ( mProjectionMatrix, aOpenGLWindow.mProjectionMatrix );
        std::swap ( mViewMatrix, aOpenGLWindow.mViewMatrix );
        std::swap ( mShadowDepthTexture, aOpenGLWindow.mShadowDepthTexture );
        std::swap ( mShadowFrameBuffer, aOpenGLWindow.mShadowFrameBuffer );
        std::swap ( mSpotShadowDepthTexture, aOpenGLWindow.mSpotShadowDepthTexture );
        std::swap ( mSpotShadowFrameBuffer, aOpenGLWindow.mSpotShadowFrameBuffer );
        std::swap ( mPointShadowDepthTexture, aOpenGLWindow.mPointShadowDepthTexture );
        std::swap ( mPointShadowFrameBuffer, aOpenGLWindow.mPointShadowFrameBuffer );
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
            FinalizeShadowMap();
            FinalizeSpotShadowMap();
            FinalizePointShadowMap();
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
        GetClientRect ( mWindowId, &rect );
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
        glViewport ( 0, 0, rect.right - rect.left, rect.bottom - rect.top );
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
        mClusterParams{std::move ( aOpenGLWindow.mClusterParams ) },
        mShadowParams{std::move ( aOpenGLWindow.mShadowParams ) },
        mSpotShadowParams{std::move ( aOpenGLWindow.mSpotShadowParams ) },
        mSpotShadowDepthScratch{std::move ( aOpenGLWindow.mSpotShadowDepthScratch ) },
        mPointShadowParams{std::move ( aOpenGLWindow.mPointShadowParams ) },
        mPointShadowDepthScratch{std::move ( aOpenGLWindow.mPointShadowDepthScratch ) }
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
        std::swap ( mShadowDepthTexture, aOpenGLWindow.mShadowDepthTexture );
        std::swap ( mShadowFrameBuffer, aOpenGLWindow.mShadowFrameBuffer );
        std::swap ( mSpotShadowDepthTexture, aOpenGLWindow.mSpotShadowDepthTexture );
        std::swap ( mSpotShadowFrameBuffer, aOpenGLWindow.mSpotShadowFrameBuffer );
        std::swap ( mPointShadowDepthTexture, aOpenGLWindow.mPointShadowDepthTexture );
        std::swap ( mPointShadowFrameBuffer, aOpenGLWindow.mPointShadowFrameBuffer );
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
            FinalizeShadowMap();
            FinalizeSpotShadowMap();
            FinalizePointShadowMap();
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
                                Topology aTopology,
                                uint32_t aVertexStart,
                                uint32_t aVertexCount,
                                uint32_t aInstanceCount,
                                uint32_t aFirstInstance,
                                const BufferAccessor* aSkinnedVertices,
                                RenderPass aRenderPass ) const
    {
        // Resolve the optional pre-skinned vertex buffer produced by the compute
        // skinning pre-pass. When present it is bound as the vertex array source
        // in place of the mesh's rest-pose vertices.
        GLuint skinned_vertex_buffer_id = 0;
        size_t skinned_vertex_offset = 0;
        // The pre-skinned buffer uses a compact 56-byte stride (weight data
        // dropped); the attribute pointers must use it instead of the mesh's
        // own 64-byte stride.
        size_t skinned_vertex_stride = 0;
        if ( aSkinnedVertices != nullptr && aSkinnedVertices->GetMemoryPoolBuffer() != nullptr )
        {
            skinned_vertex_buffer_id =
                reinterpret_cast<const OpenGLBuffer&> ( aSkinnedVertices->GetMemoryPoolBuffer()->GetBuffer() ).GetBufferId();
            skinned_vertex_offset = aSkinnedVertices->GetOffset();
            skinned_vertex_stride = 56;
        }
        // During the shadow pass, substitute the renderer-owned depth-only
        // pipeline: it rasterizes geometry into the shadow map's depth texture
        // using the light's view-projection and ignores material/lighting state.
        if ( aRenderPass == RenderPass::ShadowPass )
        {
            mOpenGLRenderer.BindPipeline ( mInPointShadowPass ? mPointShadowDepthPipeline : mShadowDepthPipeline );
            // Point and spot passes feed their per-caster (or per-face) matrix
            // through a scratch UBO bound at the same ShadowParams slot the depth
            // vertex shader reads; the directional pass uses the real buffer.
            mOpenGLRenderer.SetShadowParams ( mInPointShadowPass ? mPointShadowDepthScratch
                                              : mInSpotShadowPass ? mSpotShadowDepthScratch
                                              : mShadowParams );
            BindObjectMatrices ( { &aModelMatrix, 1 } );
            mOpenGLRenderer.BindMesh ( aMesh, skinned_vertex_buffer_id, skinned_vertex_offset, skinned_vertex_stride );
            if ( aMesh.GetIndexCount() )
            {
                glDrawElementsInstancedBaseInstance ( TopologyMap.at ( aTopology ), ( aVertexCount != 0xffffffff ) ? aVertexCount : aMesh.GetIndexCount(),
                                                      GetIndexType ( aMesh ), reinterpret_cast<const uint8_t*> ( 0 ) + aMesh.GetIndexSize() *aVertexStart, aInstanceCount, aFirstInstance );
                OPENGL_CHECK_ERROR_NO_THROW;
            }
            else
            {
                glDrawArraysInstancedBaseInstance ( TopologyMap.at ( aTopology ), aVertexStart, ( aVertexCount != 0xffffffff ) ? aVertexCount : aMesh.GetVertexCount(), aInstanceCount, aFirstInstance );
                OPENGL_CHECK_ERROR_NO_THROW;
            }
            return;
        }
        // During the depth pre-pass, substitute the renderer-owned marking
        // pipeline: it records only the cluster each fragment occupies into the
        // ClusterActive SSBO and ignores material and lighting state.
        if ( aRenderPass == RenderPass::DepthPrePass )
        {
            mOpenGLRenderer.BindPipeline ( mClusterMarkPipeline );
            mOpenGLRenderer.SetMatrices ( mMatrices );
            mOpenGLRenderer.SetClusterParams ( mClusterParams );
            BindObjectMatrices ( { &aModelMatrix, 1 } );
            if ( mFrameClusterActive.GetMemoryPoolBuffer() != nullptr )
            {
                mOpenGLRenderer.BindStorageBuffer ( Mesh::BindingLocations::CLUSTER_ACTIVE, mFrameClusterActive );
            }
            mOpenGLRenderer.BindMesh ( aMesh, skinned_vertex_buffer_id, skinned_vertex_offset, skinned_vertex_stride );
            if ( aMesh.GetIndexCount() )
            {
                glDrawElementsInstancedBaseInstance ( TopologyMap.at ( aTopology ), ( aVertexCount != 0xffffffff ) ? aVertexCount : aMesh.GetIndexCount(),
                                                      GetIndexType ( aMesh ), reinterpret_cast<const uint8_t*> ( 0 ) + aMesh.GetIndexSize() *aVertexStart, aInstanceCount, aFirstInstance );
                OPENGL_CHECK_ERROR_NO_THROW;
            }
            else
            {
                glDrawArraysInstancedBaseInstance ( TopologyMap.at ( aTopology ), aVertexStart, ( aVertexCount != 0xffffffff ) ? aVertexCount : aMesh.GetVertexCount(), aInstanceCount, aFirstInstance );
                OPENGL_CHECK_ERROR_NO_THROW;
            }
            return;
        }

        mOpenGLRenderer.BindPipeline ( aPipeline );

        mOpenGLRenderer.SetMatrices ( mMatrices );
        mOpenGLRenderer.SetLights ( mLights );
        mOpenGLRenderer.SetClusterParams ( mClusterParams );
        BindObjectMatrices ( { &aModelMatrix, 1 } );
        // Clustered Forward+ light lists, produced by the lighting compute
        // pipeline in BeginRender. Bound by name-CRC; BindStorageBuffer
        // silently skips pipelines that don't declare these blocks.
        if ( mFrameLightGrid.GetMemoryPoolBuffer() != nullptr )
        {
            mOpenGLRenderer.BindStorageBuffer ( Mesh::BindingLocations::LIGHT_GRID, mFrameLightGrid );
            mOpenGLRenderer.BindStorageBuffer ( Mesh::BindingLocations::LIGHT_INDEX_LIST, mFrameLightIndexList );
        }
        // Directional shadow map: bind its params and depth texture. Pipelines
        // that don't sample shadows ignore the unused UBO/texture unit.
        mOpenGLRenderer.SetShadowParams ( mShadowParams );
        glBindTextureUnit ( SHADOW_MAP_TEXTURE_UNIT, mShadowDepthTexture );
        // Spot shadow maps: bind the per-caster params UBO and the depth array.
        mOpenGLRenderer.SetSpotShadowParams ( mSpotShadowParams );
        glBindTextureUnit ( SPOT_SHADOW_MAP_TEXTURE_UNIT, mSpotShadowDepthTexture );
        // Point shadow maps: bind the per-caster params UBO and the cube-face array.
        mOpenGLRenderer.SetPointShadowParams ( mPointShadowParams );
        glBindTextureUnit ( POINT_SHADOW_MAP_TEXTURE_UNIT, mPointShadowDepthTexture );

        if ( aMaterial )
        {
            mOpenGLRenderer.SetMaterial ( *aMaterial );
        }

        // aFirstInstance is forwarded as the base instance so a single per-frame
        // instance buffer can hold every batch and each draw selects its slice;
        // this mirrors the Vulkan path's firstInstance argument.
        mOpenGLRenderer.BindMesh ( aMesh, skinned_vertex_buffer_id, skinned_vertex_offset, skinned_vertex_stride );
        if ( aMesh.GetIndexCount() )
        {
            glDrawElementsInstancedBaseInstance ( TopologyMap.at ( aTopology ), ( aVertexCount != 0xffffffff ) ? aVertexCount : aMesh.GetIndexCount(),
                                                  GetIndexType ( aMesh ), reinterpret_cast<const uint8_t*> ( 0 ) + aMesh.GetIndexSize() *aVertexStart, aInstanceCount, aFirstInstance );
            OPENGL_CHECK_ERROR_NO_THROW;
        }
        else
        {
            glDrawArraysInstancedBaseInstance ( TopologyMap.at ( aTopology ), aVertexStart, ( aVertexCount != 0xffffffff ) ? aVertexCount : aMesh.GetVertexCount(), aInstanceCount, aFirstInstance );
            OPENGL_CHECK_ERROR_NO_THROW;
        }
    }

    void OpenGLWindow::RenderInstanced ( std::span<const Matrix4x4> aModelMatrices,
                                         const Mesh& aMesh,
                                         const Pipeline& aPipeline,
                                         const Material* aMaterial,
                                         Topology aTopology,
                                         uint32_t aVertexStart,
                                         uint32_t aVertexCount,
                                         RenderPass aRenderPass )
    {
        const uint32_t instance_count = static_cast<uint32_t> ( aModelMatrices.size() );
        if ( instance_count == 0 )
        {
            return;
        }
        // Shadow pass: rasterize the batch into the shadow map depth texture
        // with the renderer-owned depth-only pipeline.
        if ( aRenderPass == RenderPass::ShadowPass )
        {
            mOpenGLRenderer.BindPipeline ( mInPointShadowPass ? mPointShadowDepthPipeline : mShadowDepthPipeline );
            // Point and spot passes feed their per-caster (or per-face) matrix
            // through a scratch UBO bound at the same ShadowParams slot the depth
            // vertex shader reads; the directional pass uses the real buffer.
            mOpenGLRenderer.SetShadowParams ( mInPointShadowPass ? mPointShadowDepthScratch
                                              : mInSpotShadowPass ? mSpotShadowDepthScratch
                                              : mShadowParams );
            BindObjectMatrices ( aModelMatrices );
            mOpenGLRenderer.BindMesh ( aMesh );
            if ( aMesh.GetIndexCount() )
            {
                glDrawElementsInstancedBaseInstance ( TopologyMap.at ( aTopology ), ( aVertexCount != 0xffffffff ) ? aVertexCount : aMesh.GetIndexCount(),
                                                      GetIndexType ( aMesh ), reinterpret_cast<const uint8_t*> ( 0 ) + aMesh.GetIndexSize() *aVertexStart, instance_count, 0 );
                OPENGL_CHECK_ERROR_NO_THROW;
            }
            else
            {
                glDrawArraysInstancedBaseInstance ( TopologyMap.at ( aTopology ), aVertexStart, ( aVertexCount != 0xffffffff ) ? aVertexCount : aMesh.GetVertexCount(), instance_count, 0 );
                OPENGL_CHECK_ERROR_NO_THROW;
            }
            return;
        }
        // During the depth pre-pass every draw uses the renderer-owned marking
        // pipeline; the same object-matrix buffer drives it.
        if ( aRenderPass == RenderPass::DepthPrePass )
        {
            mOpenGLRenderer.BindPipeline ( mClusterMarkPipeline );
            mOpenGLRenderer.SetMatrices ( mMatrices );
            mOpenGLRenderer.SetClusterParams ( mClusterParams );
            if ( mFrameClusterActive.GetMemoryPoolBuffer() != nullptr )
            {
                mOpenGLRenderer.BindStorageBuffer ( Mesh::BindingLocations::CLUSTER_ACTIVE, mFrameClusterActive );
            }
            BindObjectMatrices ( aModelMatrices );
            mOpenGLRenderer.BindMesh ( aMesh );
            if ( aMesh.GetIndexCount() )
            {
                glDrawElementsInstancedBaseInstance ( TopologyMap.at ( aTopology ), ( aVertexCount != 0xffffffff ) ? aVertexCount : aMesh.GetIndexCount(),
                                                      GetIndexType ( aMesh ), reinterpret_cast<const uint8_t*> ( 0 ) + aMesh.GetIndexSize() *aVertexStart, instance_count, 0 );
                OPENGL_CHECK_ERROR_NO_THROW;
            }
            else
            {
                glDrawArraysInstancedBaseInstance ( TopologyMap.at ( aTopology ), aVertexStart, ( aVertexCount != 0xffffffff ) ? aVertexCount : aMesh.GetVertexCount(), instance_count, 0 );
                OPENGL_CHECK_ERROR_NO_THROW;
            }
            return;
        }

        mOpenGLRenderer.BindPipeline ( aPipeline );
        mOpenGLRenderer.SetMatrices ( mMatrices );
        mOpenGLRenderer.SetLights ( mLights );
        mOpenGLRenderer.SetClusterParams ( mClusterParams );
        if ( mFrameLightGrid.GetMemoryPoolBuffer() != nullptr )
        {
            mOpenGLRenderer.BindStorageBuffer ( Mesh::BindingLocations::LIGHT_GRID, mFrameLightGrid );
            mOpenGLRenderer.BindStorageBuffer ( Mesh::BindingLocations::LIGHT_INDEX_LIST, mFrameLightIndexList );
        }
        // Directional shadow map: bind its params and depth texture.
        mOpenGLRenderer.SetShadowParams ( mShadowParams );
        glBindTextureUnit ( SHADOW_MAP_TEXTURE_UNIT, mShadowDepthTexture );
        // Spot shadow maps: bind the per-caster params UBO and the depth array.
        mOpenGLRenderer.SetSpotShadowParams ( mSpotShadowParams );
        glBindTextureUnit ( SPOT_SHADOW_MAP_TEXTURE_UNIT, mSpotShadowDepthTexture );
        // Point shadow maps: bind the per-caster params UBO and the cube-face array.
        mOpenGLRenderer.SetPointShadowParams ( mPointShadowParams );
        glBindTextureUnit ( POINT_SHADOW_MAP_TEXTURE_UNIT, mPointShadowDepthTexture );
        BindObjectMatrices ( aModelMatrices );
        if ( aMaterial )
        {
            mOpenGLRenderer.SetMaterial ( *aMaterial );
        }
        mOpenGLRenderer.BindMesh ( aMesh );
        if ( aMesh.GetIndexCount() )
        {
            glDrawElementsInstancedBaseInstance ( TopologyMap.at ( aTopology ), ( aVertexCount != 0xffffffff ) ? aVertexCount : aMesh.GetIndexCount(),
                                                  GetIndexType ( aMesh ), reinterpret_cast<const uint8_t*> ( 0 ) + aMesh.GetIndexSize() *aVertexStart, instance_count, 0 );
            OPENGL_CHECK_ERROR_NO_THROW;
        }
        else
        {
            glDrawArraysInstancedBaseInstance ( TopologyMap.at ( aTopology ), aVertexStart, ( aVertexCount != 0xffffffff ) ? aVertexCount : aMesh.GetVertexCount(), instance_count, 0 );
            OPENGL_CHECK_ERROR_NO_THROW;
        }
    }

    void OpenGLWindow::BindObjectMatrices ( std::span<const Matrix4x4> aMatrices ) const
    {
        const size_t size = aMatrices.size() * sizeof ( float ) * 16;
        BufferAccessor object_matrices = mStorageMemoryPoolBuffer.Allocate ( size );
        object_matrices.WriteMemory ( 0, size, aMatrices.data() );
        mOpenGLRenderer.BindStorageBuffer ( Mesh::BindingLocations::INSTANCE_MATRICES, object_matrices );
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
            // Enable active-cluster culling for this frame and refresh the
            // ClusterParams UBO so the light-cull stage sees screen.w = 1.
            mActiveCullEnabled = true;
            UpdateClusterParams();
            // Lazily load the renderer-owned marking pipeline that substitutes
            // the scene's draw pipelines during the depth pre-pass.
            if ( !mClusterMarkLoaded )
            {
                mClusterMarkPipeline.LoadFromFile ( "shaders/cluster_mark" );
                mClusterMarkLoaded = true;
            }
            // Stage 0: build the cluster AABBs, reset the index allocator and
            // clear the per-cluster active flags before the mark pass runs.
            DispatchClusterBuild ( *aComputePipeline );
            Barrier();
            // Begin the depth pre-pass; the application's first geometry
            // traversal records into it with the marking pipeline substituted.
            BeginRenderPass();
        }
        else
        {
            mActiveCullEnabled = false;
            UpdateClusterParams();
            // No clustering this frame: still hand the clustered fragment
            // shader valid, empty light buffers so it reads zero lights per
            // cluster instead of sampling an unbound buffer.
            mFrameLightGrid = mStorageMemoryPoolBuffer.Allocate ( CLUSTER_COUNT * sizeof ( GpuLightGridCell ) );
            mFrameLightIndexList = mStorageMemoryPoolBuffer.Allocate ( LIGHT_INDEX_LIST_CAPACITY * sizeof ( uint32_t ) );
            if ( void * grid = mFrameLightGrid.Map() )
            {
                std::memset ( grid, 0, CLUSTER_COUNT * sizeof ( GpuLightGridCell ) );
                mFrameLightGrid.Unmap();
            }
            BeginRenderPass();
        }
    }

    void OpenGLWindow::EndDepthPrePass ( const Pipeline* aComputePipeline )
    {
        // The mark pass wrote the per-cluster active flags from the fragment
        // shader; make those writes visible to the light-cull compute stage.
        Barrier();
        if ( aComputePipeline != nullptr )
        {
            DispatchLightCull ( *aComputePipeline );
        }
        // Begin the main color pass; the application's second geometry
        // traversal shades normally using the now-populated light grid.
        BeginRenderPass();
    }

    void OpenGLWindow::InitializeShadowMap()
    {
        // Sampleable depth texture used as the directional shadow map. Hardware
        // depth comparison (sampler2DShadow) is enabled so the fragment shader
        // can PCF-filter the depth test in a single texture() lookup.
        glGenTextures ( 1, &mShadowDepthTexture );
        OPENGL_CHECK_ERROR_THROW;
        glBindTexture ( GL_TEXTURE_2D, mShadowDepthTexture );
        glTexImage2D ( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F,
                       SHADOW_MAP_RESOLUTION, SHADOW_MAP_RESOLUTION, 0,
                       GL_DEPTH_COMPONENT, GL_FLOAT, nullptr );
        OPENGL_CHECK_ERROR_THROW;
        glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
        glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
        glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
        glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
        // White border so fragments projecting outside the light frustum sample
        // depth 1.0 and are therefore always lit (never spuriously shadowed).
        const float border[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
        glTexParameterfv ( GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border );
        glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE );
        glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL );
        OPENGL_CHECK_ERROR_THROW;

        // Depth-only framebuffer; no color attachment.
        glGenFramebuffers ( 1, &mShadowFrameBuffer );
        glBindFramebuffer ( GL_FRAMEBUFFER, mShadowFrameBuffer );
        glFramebufferTexture2D ( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, mShadowDepthTexture, 0 );
        glDrawBuffer ( GL_NONE );
        glReadBuffer ( GL_NONE );
        OPENGL_CHECK_ERROR_THROW;
        glBindFramebuffer ( GL_FRAMEBUFFER, 0 );

        // ShadowParams UBO: light view-projection + filtering params. Default-
        // init (enabled == 0) so the shading pass skips shadowing until a caster
        // matrix is uploaded in BeginShadowPass.
        {
            GpuShadowParams empty{};
            mShadowParams.Initialize ( sizeof ( GpuShadowParams ), GL_DYNAMIC_DRAW, &empty );
        }
    }

    void OpenGLWindow::FinalizeShadowMap()
    {
        mShadowParams.Finalize();
        if ( mShadowFrameBuffer != 0 )
        {
            glDeleteFramebuffers ( 1, &mShadowFrameBuffer );
            mShadowFrameBuffer = 0;
        }
        if ( mShadowDepthTexture != 0 )
        {
            glDeleteTextures ( 1, &mShadowDepthTexture );
            mShadowDepthTexture = 0;
        }
    }

    void OpenGLWindow::BeginShadowPass ( const Matrix4x4& aLightViewProjection )
    {
        // Lazily load the renderer-owned depth-only pipeline that substitutes
        // the scene's draw pipelines during the shadow pass.
        if ( !mShadowDepthLoaded )
        {
            mShadowDepthPipeline.LoadFromFile ( "shaders/shadow_depth" );
            mShadowDepthLoaded = true;
        }
        // Upload the light's world-space view-projection and mark shadowing
        // enabled for the shading pass that samples this map.
        GpuShadowParams params{};
        params.light_view_projection = aLightViewProjection;
        params.params[3] = 1.0f; // enabled
        mShadowParams.WriteMemory ( 0, sizeof ( params ), &params );

        glBindFramebuffer ( GL_FRAMEBUFFER, mShadowFrameBuffer );
        glViewport ( 0, 0, SHADOW_MAP_RESOLUTION, SHADOW_MAP_RESOLUTION );
        glClear ( GL_DEPTH_BUFFER_BIT );
        glEnable ( GL_DEPTH_TEST );
        // Slope-scaled depth bias pushes caster depths away from the light so
        // surfaces do not shadow themselves. Without it the single whole-scene
        // shadow map (coarse texels) produces large blocky self-shadow acne.
        glEnable ( GL_POLYGON_OFFSET_FILL );
        glPolygonOffset ( 2.5f, 4.0f );
        OPENGL_CHECK_ERROR_NO_THROW;
    }

    void OpenGLWindow::EndShadowPass()
    {
        // Disable the shadow-pass depth bias before the main passes resume.
        glDisable ( GL_POLYGON_OFFSET_FILL );
        // Restore the main framebuffer and full-window viewport for the depth
        // pre-pass and shading passes that follow.
        mFrameBuffer.Bind();
        glViewport ( 0, 0, static_cast<GLsizei> ( mViewportWidth ), static_cast<GLsizei> ( mViewportHeight ) );
        OPENGL_CHECK_ERROR_NO_THROW;
    }

    void OpenGLWindow::InitializeSpotShadowMap()
    {
        // Sampleable depth texture ARRAY: one layer per spot shadow caster.
        // Hardware depth comparison (sampler2DArrayShadow) is enabled so the
        // shading fragment shader PCF-filters each layer in one texture() lookup.
        glGenTextures ( 1, &mSpotShadowDepthTexture );
        OPENGL_CHECK_ERROR_THROW;
        glBindTexture ( GL_TEXTURE_2D_ARRAY, mSpotShadowDepthTexture );
        glTexImage3D ( GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT32F,
                       SPOT_SHADOW_MAP_RESOLUTION, SPOT_SHADOW_MAP_RESOLUTION,
                       MAX_SPOT_SHADOW_CASTERS, 0,
                       GL_DEPTH_COMPONENT, GL_FLOAT, nullptr );
        OPENGL_CHECK_ERROR_THROW;
        glTexParameteri ( GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
        glTexParameteri ( GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
        glTexParameteri ( GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
        glTexParameteri ( GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
        // White border so fragments projecting outside a caster's cone sample
        // depth 1.0 and are therefore always lit (never spuriously shadowed).
        const float border[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
        glTexParameterfv ( GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, border );
        glTexParameteri ( GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE );
        glTexParameteri ( GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL );
        OPENGL_CHECK_ERROR_THROW;

        // Depth-only framebuffer; a single array layer is attached per pass in
        // BeginSpotShadowPass via glFramebufferTextureLayer.
        glGenFramebuffers ( 1, &mSpotShadowFrameBuffer );
        glBindFramebuffer ( GL_FRAMEBUFFER, mSpotShadowFrameBuffer );
        glDrawBuffer ( GL_NONE );
        glReadBuffer ( GL_NONE );
        OPENGL_CHECK_ERROR_THROW;
        glBindFramebuffer ( GL_FRAMEBUFFER, 0 );

        // Spot ShadowParams UBO (all caster matrices + positions + filtering
        // params), sampled by the shading pass. Default-init (count == 0) so the
        // shading pass shadows nothing until casters are uploaded.
        {
            GpuSpotShadowParams empty{};
            mSpotShadowParams.Initialize ( sizeof ( GpuSpotShadowParams ), GL_DYNAMIC_DRAW, &empty );
        }
        // Per-pass depth matrix scratch: a single GpuShadowParams whose
        // light_view_projection the shared depth-only pipeline's vertex shader
        // reads. Each spot pass writes its caster matrix here, leaving the
        // directional ShadowParams untouched.
        {
            GpuShadowParams empty{};
            mSpotShadowDepthScratch.Initialize ( sizeof ( GpuShadowParams ), GL_DYNAMIC_DRAW, &empty );
        }
    }

    void OpenGLWindow::FinalizeSpotShadowMap()
    {
        mSpotShadowDepthScratch.Finalize();
        mSpotShadowParams.Finalize();
        if ( mSpotShadowFrameBuffer != 0 )
        {
            glDeleteFramebuffers ( 1, &mSpotShadowFrameBuffer );
            mSpotShadowFrameBuffer = 0;
        }
        if ( mSpotShadowDepthTexture != 0 )
        {
            glDeleteTextures ( 1, &mSpotShadowDepthTexture );
            mSpotShadowDepthTexture = 0;
        }
    }

    void OpenGLWindow::SetSpotShadowParams ( const GpuSpotShadowParams& aSpotShadowParams )
    {
        mSpotShadowParams.WriteMemory ( 0, sizeof ( GpuSpotShadowParams ), &aSpotShadowParams );
    }

    void OpenGLWindow::BeginSpotShadowPass ( uint32_t aSlot, const Matrix4x4& aLightViewProjection )
    {
        // Lazily load the renderer-owned depth-only pipeline (shared with the
        // directional pass) that substitutes the scene's draw pipelines.
        if ( !mShadowDepthLoaded )
        {
            mShadowDepthPipeline.LoadFromFile ( "shaders/shadow_depth" );
            mShadowDepthLoaded = true;
        }
        // Write this caster's matrix into the depth scratch UBO the depth-only
        // vertex shader reads, and route the shadow-pass draw branch to it.
        GpuShadowParams params{};
        params.light_view_projection = aLightViewProjection;
        params.params[3] = 1.0f; // enabled (unused by the depth vertex shader)
        mSpotShadowDepthScratch.WriteMemory ( 0, sizeof ( params ), &params );
        mInSpotShadowPass = true;

        glBindFramebuffer ( GL_FRAMEBUFFER, mSpotShadowFrameBuffer );
        // Attach only this caster's array layer so the pass renders into it.
        glFramebufferTextureLayer ( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                                    mSpotShadowDepthTexture, 0, static_cast<GLint> ( aSlot ) );
        glViewport ( 0, 0, SPOT_SHADOW_MAP_RESOLUTION, SPOT_SHADOW_MAP_RESOLUTION );
        glClear ( GL_DEPTH_BUFFER_BIT );
        glEnable ( GL_DEPTH_TEST );
        // Slope-scaled depth bias to fight self-shadow acne, as in the
        // directional pass.
        glEnable ( GL_POLYGON_OFFSET_FILL );
        glPolygonOffset ( 2.5f, 4.0f );
        OPENGL_CHECK_ERROR_NO_THROW;
    }

    void OpenGLWindow::EndSpotShadowPass()
    {
        mInSpotShadowPass = false;
        glDisable ( GL_POLYGON_OFFSET_FILL );
        // Restore the main framebuffer and full-window viewport for the passes
        // that follow.
        mFrameBuffer.Bind();
        glViewport ( 0, 0, static_cast<GLsizei> ( mViewportWidth ), static_cast<GLsizei> ( mViewportHeight ) );
        OPENGL_CHECK_ERROR_NO_THROW;
    }

    void OpenGLWindow::InitializePointShadowMap()
    {
        // Sampleable depth CUBE MAP ARRAY: one cube (six faces) per caster, so
        // the shading pass samples by world direction and the GPU selects the
        // face and filters seamlessly across face edges.
        glEnable ( GL_TEXTURE_CUBE_MAP_SEAMLESS );
        glGenTextures ( 1, &mPointShadowDepthTexture );
        OPENGL_CHECK_ERROR_THROW;
        glBindTexture ( GL_TEXTURE_CUBE_MAP_ARRAY, mPointShadowDepthTexture );
        glTexImage3D ( GL_TEXTURE_CUBE_MAP_ARRAY, 0, GL_DEPTH_COMPONENT32F,
                       POINT_SHADOW_MAP_RESOLUTION, POINT_SHADOW_MAP_RESOLUTION,
                       POINT_SHADOW_FACES * MAX_POINT_SHADOW_CASTERS, 0,
                       GL_DEPTH_COMPONENT, GL_FLOAT, nullptr );
        OPENGL_CHECK_ERROR_THROW;
        glTexParameteri ( GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
        glTexParameteri ( GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
        glTexParameteri ( GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
        glTexParameteri ( GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
        glTexParameteri ( GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE );
        glTexParameteri ( GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE );
        glTexParameteri ( GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL );
        OPENGL_CHECK_ERROR_THROW;

        // Depth-only framebuffer; a single cube-face layer is attached per pass.
        glGenFramebuffers ( 1, &mPointShadowFrameBuffer );
        glBindFramebuffer ( GL_FRAMEBUFFER, mPointShadowFrameBuffer );
        glDrawBuffer ( GL_NONE );
        glReadBuffer ( GL_NONE );
        OPENGL_CHECK_ERROR_THROW;
        glBindFramebuffer ( GL_FRAMEBUFFER, 0 );

        {
            GpuPointShadowParams empty{};
            mPointShadowParams.Initialize ( sizeof ( GpuPointShadowParams ), GL_DYNAMIC_DRAW, &empty );
        }
        {
            GpuShadowParams empty{};
            mPointShadowDepthScratch.Initialize ( sizeof ( GpuPointDepthParams ), GL_DYNAMIC_DRAW, &empty );
        }
    }

    void OpenGLWindow::FinalizePointShadowMap()
    {
        mPointShadowDepthScratch.Finalize();
        mPointShadowParams.Finalize();
        if ( mPointShadowFrameBuffer != 0 )
        {
            glDeleteFramebuffers ( 1, &mPointShadowFrameBuffer );
            mPointShadowFrameBuffer = 0;
        }
        if ( mPointShadowDepthTexture != 0 )
        {
            glDeleteTextures ( 1, &mPointShadowDepthTexture );
            mPointShadowDepthTexture = 0;
        }
    }

    void OpenGLWindow::SetPointShadowParams ( const GpuPointShadowParams& aPointShadowParams )
    {
        mPointShadowParamsCpu = aPointShadowParams;
        mPointShadowParams.WriteMemory ( 0, sizeof ( GpuPointShadowParams ), &aPointShadowParams );
    }

    void OpenGLWindow::BeginPointShadowPass ( uint32_t aCaster )
    {
        if ( !mPointShadowDepthLoaded )
        {
            mPointShadowDepthPipeline.LoadFromFile ( "shaders/point_shadow_depth" );
            mPointShadowDepthLoaded = true;
        }
        // One draw renders all six cube faces: the geometry shader reads the six
        // face matrices + light position/radius and routes each face to its
        // cube-array layer. base_layer is absolute (caster*6) because the whole
        // cube-map array is attached as a layered target.
        GpuPointDepthParams depth_params{};
        for ( uint32_t face = 0; face < POINT_SHADOW_FACES; ++face )
        {
            depth_params.face_view_projection[face] =
                mPointShadowParamsCpu.point_light_view_projection[aCaster * POINT_SHADOW_FACES + face];
        }
        depth_params.light_position_radius = mPointShadowParamsCpu.caster_position_radius[aCaster];
        depth_params.face_params = Vector4 { static_cast<float> ( aCaster * POINT_SHADOW_FACES ), 0.0f, 0.0f, 0.0f };
        mPointShadowDepthScratch.WriteMemory ( 0, sizeof ( depth_params ), &depth_params );
        mInPointShadowPass = true;

        glBindFramebuffer ( GL_FRAMEBUFFER, mPointShadowFrameBuffer );
        // Attach the whole cube-map array as a layered target; the geometry
        // shader's gl_Layer selects the caster's six faces.
        glFramebufferTexture ( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, mPointShadowDepthTexture, 0 );
        glViewport ( 0, 0, POINT_SHADOW_MAP_RESOLUTION, POINT_SHADOW_MAP_RESOLUTION );
        // The layered attachment covers every caster's layers, so clear the whole
        // array once, before the first caster, to avoid wiping a caster already
        // rendered this frame.
        if ( aCaster == 0 )
        {
            glClear ( GL_DEPTH_BUFFER_BIT );
        }
        glEnable ( GL_DEPTH_TEST );
        OPENGL_CHECK_ERROR_NO_THROW;
    }

    void OpenGLWindow::EndPointShadowPass()
    {
        mInPointShadowPass = false;
        mFrameBuffer.Bind();
        glViewport ( 0, 0, static_cast<GLsizei> ( mViewportWidth ), static_cast<GLsizei> ( mViewportHeight ) );
        OPENGL_CHECK_ERROR_NO_THROW;
    }

    void OpenGLWindow::DispatchClusterBuild ( const Pipeline& aComputePipeline )
    {
        // One workgroup per 64 clusters (clustering compute stages use local_size_x=64).
        constexpr uint32_t group_count = ( CLUSTER_COUNT + 63u ) / 64u;

        // Allocate the per-frame clustering buffers. They persist as members so
        // both this build dispatch and the post-mark light-cull dispatch bind
        // the same storage; reflection silently drops the blocks a given stage
        // does not declare.
        mFrameClusterAABBs = mStorageMemoryPoolBuffer.Allocate ( CLUSTER_COUNT * sizeof ( GpuClusterAABB ) );
        mFrameLightGrid = mStorageMemoryPoolBuffer.Allocate ( CLUSTER_COUNT * sizeof ( GpuLightGridCell ) );
        mFrameLightIndexList = mStorageMemoryPoolBuffer.Allocate ( LIGHT_INDEX_LIST_CAPACITY * sizeof ( uint32_t ) );
        // Global atomic allocator for the flat LightIndexList (R1). cluster_build
        // zeroes it from invocation 0, so no host-side initialisation is needed.
        mFrameLightIndexCounter = mStorageMemoryPoolBuffer.Allocate ( sizeof ( uint32_t ) );
        // Per-cluster active flags (R2): cluster_build clears them, the mark
        // pass sets them, the light-cull stage reads them.
        mFrameClusterActive = mStorageMemoryPoolBuffer.Allocate ( CLUSTER_COUNT * sizeof ( uint32_t ) );

        const StorageBufferBinding bindings[]
        {
            { Mesh::BindingLocations::CLUSTER_AABBS, &mFrameClusterAABBs },
            { Mesh::BindingLocations::LIGHT_GRID, &mFrameLightGrid },
            { Mesh::BindingLocations::LIGHT_INDEX_LIST, &mFrameLightIndexList },
            { Mesh::BindingLocations::LIGHT_INDEX_COUNTER, &mFrameLightIndexCounter },
            { Mesh::BindingLocations::CLUSTER_ACTIVE, &mFrameClusterActive },
        };

        // Stage 0 only: build AABBs, reset the allocator and clear active flags.
        Dispatch ( aComputePipeline, group_count, 1, 1, bindings, 0 );
    }

    void OpenGLWindow::DispatchLightCull ( const Pipeline& aComputePipeline )
    {
        constexpr uint32_t group_count = ( CLUSTER_COUNT + 63u ) / 64u;

        const StorageBufferBinding bindings[]
        {
            { Mesh::BindingLocations::CLUSTER_AABBS, &mFrameClusterAABBs },
            { Mesh::BindingLocations::LIGHT_GRID, &mFrameLightGrid },
            { Mesh::BindingLocations::LIGHT_INDEX_LIST, &mFrameLightIndexList },
            { Mesh::BindingLocations::LIGHT_INDEX_COUNTER, &mFrameLightIndexCounter },
            { Mesh::BindingLocations::CLUSTER_ACTIVE, &mFrameClusterActive },
        };

        // Stages 1..N: cull lights against the AABBs, skipping the clusters the
        // depth pre-pass left inactive. A barrier between stages keeps each
        // stage's writes visible to the next and, after the last stage, to the
        // fragment shader of the main color pass.
        const uint32_t stage_count = aComputePipeline.GetComputeStageCount ( mOpenGLRenderer.GetName() );
        for ( uint32_t stage = 1; stage < stage_count; ++stage )
        {
            Dispatch ( aComputePipeline, group_count, 1, 1, bindings, stage );
            Barrier();
        }
    }

    void OpenGLWindow::BeginFrame()
    {
        // Idempotent within a frame: the application may call BeginFrame()
        // explicitly to run a pre-render-pass compute phase (e.g. skinning)
        // before BeginRender(), which also calls BeginFrame().
        if ( mFrameBegun )
        {
            return;
        }
        mFrameBegun = true;
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
        // Opaque scene geometry must not blend: it is drawn in arbitrary order
        // and many diffuse textures carry alpha < 1, which would otherwise make
        // surfaces translucent and show overlapping geometry through each other.
        // The GUI overlay re-enables blending for itself and restores this.
        glDisable ( GL_BLEND );
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

    void OpenGLWindow::Skin ( const Pipeline& aSkinningPipeline,
                              const Mesh& aMesh,
                              const BufferAccessor& aSkinningMatrices,
                              const BufferAccessor& aSkinnedVertices ) const
    {
        if ( aMesh.GetVertexCount() == 0 )
        {
            return;
        }
        mOpenGLRenderer.BindComputePipeline ( aSkinningPipeline, 0 );
        mOpenGLRenderer.BindStorageBuffer ( Mesh::BindingLocations::SKINNING_MATRICES, aSkinningMatrices );
        if ( const OpenGLMesh * mesh = mOpenGLRenderer.GetOpenGLMesh ( aMesh ) )
        {
            mOpenGLRenderer.BindStorageBufferId ( Mesh::BindingLocations::SOURCE_VERTICES,
                                                  mesh->GetVertexBufferId(),
                                                  0,
                                                  static_cast<size_t> ( aMesh.GetVertexCount() ) * aMesh.GetStride() );
        }
        mOpenGLRenderer.BindStorageBuffer ( Mesh::BindingLocations::SKINNED_VERTICES, aSkinnedVertices );
        uint32_t group_count = ( aMesh.GetVertexCount() + 63u ) / 64u;
        glDispatchCompute ( group_count, 1, 1 );
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
        mFrameBegun = false;
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
        mMatrices.WriteMemory ( 0, sizeof ( float ) * 16, mProjectionMatrix.GetMatrix4x4() );
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
        // screen.w gates active-cluster culling in the light-cull stage: it is
        // only set once the depth pre-pass mark stage has run this frame.
        params.screen[3] = mActiveCullEnabled ? 1.0f : 0.0f;
        mClusterParams.WriteMemory ( 0, sizeof ( GpuClusterParams ), &params );
    }

    void OpenGLWindow::SetViewMatrix ( const Matrix4x4& aMatrix )
    {
        mViewMatrix = aMatrix;
        mFrustum = mProjectionMatrix * mViewMatrix;
        mMatrices.WriteMemory ( sizeof ( float ) * 16, sizeof ( float ) * 16, mViewMatrix.GetMatrix4x4() );
    }

    void OpenGLWindow::SetLights ( std::span<const GpuLight> aLights )
    {
        // Frustum-cull the lights before uploading: any point/spot light whose
        // bounding sphere lies entirely outside the view frustum cannot affect
        // a visible pixel, so dropping it here shrinks the set the per-cluster
        // light cull has to iterate. Directional lights are always kept.
        mVisibleLights.clear();
        for ( const GpuLight& light : aLights )
        {
            if ( mVisibleLights.size() >= MAX_LIGHTS_PER_FRAME )
            {
                break;
            }
            if ( mFrustum.Intersects ( light ) )
            {
                mVisibleLights.push_back ( light );
            }
        }
        // Stream the light records into the SSBO: write the 16-byte header
        // (count) then the tightly packed array.
        const std::size_t capped = mVisibleLights.size();
        const GpuLightsHeader header{ static_cast<uint32_t> ( capped ), { 0, 0, 0 } };
#if defined(_WIN32)
        mOpenGLRenderer.MakeCurrent ( mDeviceContext );
#elif defined(__unix__)
        mOpenGLRenderer.MakeCurrent ( mWindowId );
#endif
        mLights.WriteMemory ( 0, sizeof ( header ), &header );
        if ( capped > 0 )
        {
            mLights.WriteMemory ( sizeof ( GpuLightsHeader ), capped * sizeof ( GpuLight ), mVisibleLights.data() );
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

    const BufferAccessor & OpenGLWindow::GetFrameLightGrid() const
    {
        return mFrameLightGrid;
    }

    const BufferAccessor & OpenGLWindow::GetFrameClusterActive() const
    {
        return mFrameClusterActive;
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
