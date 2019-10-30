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
#include "aeongames/Frustum.h"
#include "aeongames/AABB.h"
#include "aeongames/Pipeline.h"
#include "aeongames/Material.h"
#include "aeongames/Mesh.h"
#include "aeongames/LogLevel.h"
#include "aeongames/Node.h"
#include "aeongames/Scene.h"
#include "OpenGLWindow.h"
#include "OpenGLRenderer.h"
#include "OpenGLPipeline.h"
#include "OpenGLMaterial.h"
#include "OpenGLMesh.h"
#include "OpenGLFunctions.h"
#include "aeongames/MemoryPool.h" ///<- This is here just for the literals
#include <sstream>
#include <iostream>
#include <algorithm>
#include <array>
#include <utility>

namespace AeonGames
{
    const GLchar vertex_shader_code[] =
        R"(#version 450 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 Pos;
out vec2 TexCoords;

void main()
{
    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0); 
    Pos = aPos;
    TexCoords = aTexCoords;
}
)";
const GLint vertex_shader_len { sizeof(vertex_shader_code) /*/ sizeof(vertex_shader_code[0])*/};
const GLchar* const vertex_shader_code_ptr = vertex_shader_code;

const GLchar fragment_shader_code[] =
R"(#version 450 core
out vec4 FragColor;
  
in vec2 Pos;
in vec2 TexCoords;

layout (location = 0) uniform sampler2D screenTexture;

void main()
{ 
    //FragColor = vec4((Pos.x+1.0)/2,(Pos.y+1.0)/2,0,1);
    FragColor = texture(screenTexture, TexCoords);
}
)";
const GLint fragment_shader_len { sizeof(fragment_shader_code) /*/ sizeof(fragment_shader_code[0])*/};
const GLchar* const fragment_shader_code_ptr = fragment_shader_code;

const float vertices[] = {  
    // positions   // texCoords
    -1.0f,  1.0f,  0.0f, 1.0f,
    -1.0f, -1.0f,  0.0f, 0.0f,
    1.0f, -1.0f,  1.0f, 0.0f,
    1.0f,  1.0f,  1.0f, 1.0f
};
const GLuint vertex_size{sizeof(vertices)};

    void OpenGLWindow::Initialize()
    {
        glGenVertexArrays ( 1, &mVAO );
        OPENGL_CHECK_ERROR_THROW;
        glBindVertexArray ( mVAO );
        OPENGL_CHECK_ERROR_THROW;

        // Frame Buffer
        glGenFramebuffers ( 1, &mFBO );
        OPENGL_CHECK_ERROR_THROW;
        glBindFramebuffer ( GL_FRAMEBUFFER, mFBO );
        OPENGL_CHECK_ERROR_THROW;

        // Color Buffer
        glGenTextures ( 1, &mColorBuffer );
        OPENGL_CHECK_ERROR_THROW;
        glBindTexture ( GL_TEXTURE_2D, mColorBuffer );
        OPENGL_CHECK_ERROR_THROW;
        glTexImage2D ( GL_TEXTURE_2D, 0, GL_RGB, 800, 600, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr );
        OPENGL_CHECK_ERROR_THROW;
        glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
        OPENGL_CHECK_ERROR_THROW;
        glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
        OPENGL_CHECK_ERROR_THROW;
        glBindTexture ( GL_TEXTURE_2D, 0 );
        OPENGL_CHECK_ERROR_THROW;

        glFramebufferTexture2D ( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mColorBuffer, 0 );
        OPENGL_CHECK_ERROR_THROW;
        // Render Buffer
        glGenRenderbuffers ( 1, &mRBO );
        OPENGL_CHECK_ERROR_THROW;
        glBindRenderbuffer ( GL_RENDERBUFFER, mRBO );
        OPENGL_CHECK_ERROR_THROW;
        glRenderbufferStorage ( GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 800, 600 );
        OPENGL_CHECK_ERROR_THROW;
        glBindRenderbuffer ( GL_RENDERBUFFER, 0 );
        OPENGL_CHECK_ERROR_THROW;

        glFramebufferRenderbuffer ( GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, mRBO );
        OPENGL_CHECK_ERROR_THROW;
        if ( glCheckFramebufferStatus ( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE )
        {
            throw std::runtime_error ( "Incomplete Framebuffer." );
        }
        OPENGL_CHECK_ERROR_THROW;

        glBindFramebuffer ( GL_FRAMEBUFFER, 0 );
        OPENGL_CHECK_ERROR_THROW;
//----------------------------------------------------------------
    GLint compile_status{};
    mProgram = glCreateProgram();
    OPENGL_CHECK_ERROR_THROW;
    GLuint vertex_shader = glCreateShader ( GL_VERTEX_SHADER );
    OPENGL_CHECK_ERROR_THROW;
    glShaderSource (vertex_shader,1,&vertex_shader_code_ptr,&vertex_shader_len );
    OPENGL_CHECK_ERROR_THROW;
    glCompileShader ( vertex_shader );
    OPENGL_CHECK_ERROR_THROW;
    glGetShaderiv ( vertex_shader, GL_COMPILE_STATUS, &compile_status );
    OPENGL_CHECK_ERROR_THROW;
    if ( compile_status != GL_TRUE )
    {
        GLint info_log_len;
        glGetShaderiv ( vertex_shader, GL_INFO_LOG_LENGTH, &info_log_len );
        OPENGL_CHECK_ERROR_THROW;
        std::string log_string;
        log_string.resize ( info_log_len );
        if ( info_log_len > 1 )
        {
            glGetShaderInfoLog ( vertex_shader, info_log_len, nullptr, const_cast<GLchar*> ( log_string.data() ) );
            OPENGL_CHECK_ERROR_THROW;
            std::cout << vertex_shader_code << std::endl;
            std::cout << log_string << std::endl;
        }
    }
    glAttachShader ( mProgram, vertex_shader );
    OPENGL_CHECK_ERROR_THROW;
    //-------------------------
    uint32_t fragment_shader = glCreateShader ( GL_FRAGMENT_SHADER );
    OPENGL_CHECK_ERROR_THROW;
    glShaderSource ( fragment_shader, 1, &fragment_shader_code_ptr, &fragment_shader_len );
    OPENGL_CHECK_ERROR_THROW;
    glCompileShader ( fragment_shader );
    OPENGL_CHECK_ERROR_THROW;
    glGetShaderiv ( fragment_shader, GL_COMPILE_STATUS, &compile_status );
    OPENGL_CHECK_ERROR_THROW;
    if ( compile_status != GL_TRUE )
    {
        GLint info_log_len;
        glGetShaderiv ( fragment_shader, GL_INFO_LOG_LENGTH, &info_log_len );
        OPENGL_CHECK_ERROR_THROW;
        std::string log_string;
        log_string.resize ( info_log_len );
        if ( info_log_len > 1 )
        {
            glGetShaderInfoLog ( fragment_shader, info_log_len, nullptr, const_cast<GLchar*> ( log_string.data() ) );
            OPENGL_CHECK_ERROR_THROW;
            std::cout << fragment_shader_code << std::endl;
            std::cout << log_string << std::endl;
        }
    }
    glAttachShader ( mProgram, fragment_shader );
    OPENGL_CHECK_ERROR_THROW;

    glLinkProgram ( mProgram );
    OPENGL_CHECK_ERROR_THROW;
    glDetachShader ( mProgram, vertex_shader );
    OPENGL_CHECK_ERROR_THROW;
    glDetachShader ( mProgram, fragment_shader );
    OPENGL_CHECK_ERROR_THROW;
    glDeleteShader ( vertex_shader );
    OPENGL_CHECK_ERROR_THROW;
    glDeleteShader ( fragment_shader );
    OPENGL_CHECK_ERROR_THROW;
    glUseProgram(mProgram);
    OPENGL_CHECK_ERROR_THROW;
    glUniform1i ( 0, 0 );
    OPENGL_CHECK_ERROR_THROW;
//----------------------------------------------------------------

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

		mMatrices.Load({ {"ModelMatrix", Matrix4x4{}},{"ProjectionMatrix", Matrix4x4{}}, {"ViewMatrix", Matrix4x4{}}},{});

        mScreenQuad.Initialize(vertex_size, GL_STATIC_DRAW, vertices);
    }

    void OpenGLWindow::Finalize()
    {
        if ( glIsRenderbuffer ( mRBO ) )
        {
            OPENGL_CHECK_ERROR_NO_THROW;
            glDeleteRenderbuffers ( 1, &mRBO );
            OPENGL_CHECK_ERROR_NO_THROW;
            mRBO = 0;
        }
        if ( glIsTexture ( mColorBuffer ) )
        {
            OPENGL_CHECK_ERROR_NO_THROW;
            glDeleteTextures ( 1, &mColorBuffer );
            OPENGL_CHECK_ERROR_NO_THROW;
            mColorBuffer = 0;
        }
        if ( glIsFramebuffer ( mFBO ) )
        {
            OPENGL_CHECK_ERROR_NO_THROW;
            glDeleteFramebuffers ( 1, &mFBO );
            OPENGL_CHECK_ERROR_NO_THROW;
            mFBO = 0;
        }
        OPENGL_CHECK_ERROR_NO_THROW;

        if(glIsProgram(mProgram))
        {
            OPENGL_CHECK_ERROR_NO_THROW;
            glUseProgram(0);
            OPENGL_CHECK_ERROR_NO_THROW;
            glDeleteProgram(mProgram);
            OPENGL_CHECK_ERROR_NO_THROW;
            mProgram = 0;
        }

        mMatrices.Unload();

        mScreenQuad.Finalize();

        if ( glIsVertexArray ( mVAO ) )
        {
            OPENGL_CHECK_ERROR_NO_THROW;
            glDeleteVertexArrays ( 1, &mVAO );
            OPENGL_CHECK_ERROR_NO_THROW;
            mVAO = 0;
        }
        OPENGL_CHECK_ERROR_NO_THROW;
    }

    OpenGLWindow::OpenGLWindow ( const OpenGLRenderer& aOpenGLRenderer, int32_t aX, int32_t aY, uint32_t aWidth, uint32_t aHeight, bool aFullScreen ) :
        Window{aX, aY, aWidth, aHeight, aFullScreen}, mOpenGLRenderer { aOpenGLRenderer },mMemoryPoolBuffer{aOpenGLRenderer,8_mb},
        mFullScreen{aFullScreen}
    {
    }
    OpenGLWindow::OpenGLWindow ( const OpenGLRenderer&  aOpenGLRenderer, void* aWindowId ) :
        Window{aWindowId}, mOpenGLRenderer{ aOpenGLRenderer },mMemoryPoolBuffer{aOpenGLRenderer,8_mb}
    {
    }

    OpenGLWindow::~OpenGLWindow()
    {
    }

    void* OpenGLWindow::GetWindowId() const
    {
        return mWindowId;
    }

    void OpenGLWindow::Render ( const Matrix4x4& aModelMatrix,
                                const Mesh& aMesh,
                                const Pipeline& aPipeline,
                                const Material* aMaterial,
                                const BufferAccessor* aSkeleton,
                                uint32_t aVertexStart,
                                uint32_t aVertexCount,
                                uint32_t aInstanceCount,
                                uint32_t aFirstInstance ) const
    {
        const OpenGLMesh& opengl_mesh{reinterpret_cast<const OpenGLMesh&> ( aMesh ) };
        const OpenGLPipeline& opengl_pipeline{reinterpret_cast<const OpenGLPipeline&> ( aPipeline ) };
        opengl_pipeline.Use (reinterpret_cast<const OpenGLMaterial*>(aMaterial), aSkeleton );
        OPENGL_CHECK_ERROR_NO_THROW;

        mMatrices.Set(0,aModelMatrix);

        glBindBuffer ( GL_UNIFORM_BUFFER, mMatrices.GetPropertiesBufferId() );
        OPENGL_CHECK_ERROR_THROW;
        glBindBufferBase ( GL_UNIFORM_BUFFER, 0, mMatrices.GetPropertiesBufferId() );
        OPENGL_CHECK_ERROR_THROW;

        /// @todo Add some sort of way to make use of the aFirstInstance parameter
        opengl_mesh.BindVertexArray();
        OPENGL_CHECK_ERROR_NO_THROW;
        if ( aMesh.GetIndexCount() )
        {
            glBindBuffer ( GL_ELEMENT_ARRAY_BUFFER, opengl_mesh.GetIndexBufferId() );
            OPENGL_CHECK_ERROR_NO_THROW;
            glDrawElementsInstanced ( opengl_pipeline.GetTopology(), ( aVertexCount != 0xffffffff ) ? aVertexCount : aMesh.GetIndexCount(),
                                      opengl_mesh.GetIndexType(), reinterpret_cast<const uint8_t*> ( 0 ) + aMesh.GetIndexSize() *aVertexStart, aInstanceCount );
            OPENGL_CHECK_ERROR_NO_THROW;
        }
        else
        {
            glDrawArraysInstanced ( opengl_pipeline.GetTopology(), aVertexStart, ( aVertexCount != 0xffffffff ) ? aVertexCount : aMesh.GetVertexCount(), aInstanceCount );
            OPENGL_CHECK_ERROR_NO_THROW;
        }
    }

    const GLuint OpenGLWindow::GetMatricesBuffer() const
    {
        return mMatrices.GetPropertiesBufferId();
    }

    void OpenGLWindow::OnSetProjectionMatrix()
    {
        mMatrices.Set ( 1, mProjectionMatrix * Matrix4x4
        {
            // Flip Matrix
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, -1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        } );
    }
    void OpenGLWindow::OnSetViewMatrix()
    {
        mMatrices.Set ( 2, mViewMatrix );
    }

    BufferAccessor OpenGLWindow::AllocateSingleFrameUniformMemory(size_t aSize)
    {
        return mMemoryPoolBuffer.Allocate ( aSize );
    }

    void OpenGLWindow::OnResizeViewport ( int32_t aX, int32_t aY, uint32_t aWidth, uint32_t aHeight )
    {
        if ( aWidth && aHeight )
        {
            MakeCurrent();
            OPENGL_CHECK_ERROR_NO_THROW;
            glViewport ( aX, aY, aWidth, aHeight );
            OPENGL_CHECK_ERROR_THROW;
            glBindTexture ( GL_TEXTURE_2D, mColorBuffer );
            OPENGL_CHECK_ERROR_THROW;
            glTexImage2D ( GL_TEXTURE_2D, 0, GL_RGB, aWidth, aHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr );
            OPENGL_CHECK_ERROR_THROW;
            glBindRenderbuffer ( GL_RENDERBUFFER, mRBO );
            OPENGL_CHECK_ERROR_THROW;
            glRenderbufferStorage ( GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, aWidth, aHeight );
            OPENGL_CHECK_ERROR_THROW;
            glBindRenderbuffer ( GL_RENDERBUFFER, 0 );
            OPENGL_CHECK_ERROR_THROW;
        }
    }

    void OpenGLWindow::BeginRender()
    {
        MakeCurrent();
        glBindFramebuffer ( GL_FRAMEBUFFER, mFBO );
        glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        glEnable ( GL_DEPTH_TEST );
    }

    void OpenGLWindow::EndRender()
    {
        glBindFramebuffer ( GL_FRAMEBUFFER, 0 );
        OPENGL_CHECK_ERROR_NO_THROW;
        glClear ( GL_COLOR_BUFFER_BIT );
        OPENGL_CHECK_ERROR_NO_THROW;
        glDisable ( GL_DEPTH_TEST );
        OPENGL_CHECK_ERROR_NO_THROW;
        glUseProgram ( mProgram );
        OPENGL_CHECK_ERROR_NO_THROW;
#ifndef SINGLE_VAO
        glBindVertexArray ( mVAO );
        OPENGL_CHECK_ERROR_THROW;
#endif
        glBindBuffer ( GL_ARRAY_BUFFER, mScreenQuad.GetBufferId() );
        OPENGL_CHECK_ERROR_NO_THROW;
        glBindTexture ( GL_TEXTURE_2D, mColorBuffer );
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
        SwapBuffers();
        mMemoryPoolBuffer.Reset();
    }
}
