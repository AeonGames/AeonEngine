/*
Copyright (C) 2017,2018 Rodrigo Jose Hernandez Cordoba

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
#include "aeongames/ModelInstance.h"
#include "aeongames/Frustum.h"
#include "aeongames/AABB.h"
#include "aeongames/Scene.h"
#include "aeongames/Node.h"
#include "OpenGLWindow.h"
#include "OpenGLRenderer.h"
#include "OpenGLModel.h"
#include "OpenGLFunctions.h"
#include <sstream>
#include <iostream>
#include <algorithm>
#include <array>

namespace AeonGames
{
    OpenGLWindow::OpenGLWindow ( void* aWindowId, const std::shared_ptr<const OpenGLRenderer> aOpenGLRenderer ) :
        mOpenGLRenderer ( aOpenGLRenderer ), mWindowId ( aWindowId )
    {
        try
        {
            Initialize();
        }
        catch ( ... )
        {
            Finalize();
            throw;
        }
    }

    OpenGLWindow::~OpenGLWindow()
    {
        Finalize();
    }

    void* OpenGLWindow::GetWindowId() const
    {
        return mWindowId;
    }

    void OpenGLWindow::ResizeViewport ( uint32_t aWidth, uint32_t aHeight )
    {
        if ( aWidth && aHeight )
        {
#ifdef WIN32
            HDC hdc = GetDC ( reinterpret_cast<HWND> ( mWindowId ) );
            wglMakeCurrent ( hdc, reinterpret_cast<HGLRC> ( mOpenGLRenderer->GetOpenGLContext() ) );
            OPENGL_CHECK_ERROR_NO_THROW;
            ReleaseDC ( reinterpret_cast<HWND> ( mWindowId ), hdc );
#else
            glXMakeCurrent ( static_cast<Display*> ( mOpenGLRenderer->GetWindowId() ),
                             reinterpret_cast<::Window> ( mWindowId ),
                             static_cast<GLXContext> ( mOpenGLRenderer->GetOpenGLContext() ) );
            OPENGL_CHECK_ERROR_NO_THROW;
#endif
            glViewport ( 0, 0, aWidth, aHeight );
            OPENGL_CHECK_ERROR_NO_THROW;
        }
    }

    void OpenGLWindow::Render ( const std::shared_ptr<Scene>& aScene ) const
    {
#ifdef _WIN32
        HDC hdc = GetDC ( reinterpret_cast<HWND> ( mWindowId ) );
        wglMakeCurrent ( hdc, static_cast<HGLRC> ( mOpenGLRenderer->GetOpenGLContext() ) );
        glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        ReleaseDC ( reinterpret_cast<HWND> ( mWindowId ), hdc );
#else
        glXMakeCurrent ( static_cast<Display*> ( mOpenGLRenderer->GetWindowId() ),
                         reinterpret_cast<::Window> ( mWindowId ),
                         static_cast<GLXContext> ( mOpenGLRenderer->GetOpenGLContext() ) );
        glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
#endif

        Matrix4x4 view_matrix{ mViewTransform.GetInvertedMatrix() };
        Matrix4x4 projection_matrix =
            mProjectionMatrix * Matrix4x4
        {
            // Flip Matrix
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, -1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        };

        glBindBuffer ( GL_UNIFORM_BUFFER, mMatricesBuffer );
        OPENGL_CHECK_ERROR_NO_THROW;
        glBufferSubData ( GL_UNIFORM_BUFFER, sizeof ( float ) * 16, sizeof ( float ) * 16, view_matrix.GetMatrix4x4() );
        OPENGL_CHECK_ERROR_NO_THROW;
        glBufferSubData ( GL_UNIFORM_BUFFER, 0, sizeof ( float ) * 16, ( projection_matrix ).GetMatrix4x4() );
        OPENGL_CHECK_ERROR_NO_THROW;
        glBindBufferBase ( GL_UNIFORM_BUFFER, 0, mMatricesBuffer );

        Frustum frustum ( projection_matrix * view_matrix );
        aScene->LoopTraverseDFSPreOrder ( [this, &frustum, &projection_matrix, &view_matrix] ( Node & aNode )
        {
            const ModelInstance* model_instance = reinterpret_cast<const ModelInstance*> ( aNode.GetAttribute ( ModelInstance::TypeId ) );
            const OpenGLModel* opengl_model = reinterpret_cast<const OpenGLModel*> ( aNode.GetAttribute ( OpenGLModel::TypeId ) );
            if ( opengl_model )
            {
                if ( frustum.Intersects ( aNode.GetGlobalAABB() ) )
                {
                    // We dont really need to pass the matrices here, but we already have them so why not.
                    opengl_model->Render ( model_instance, projection_matrix, view_matrix );
                }
            }
            else
            {
                /* This is lazy loading */
                aNode.SetAttribute ( OpenGLModel::TypeId, std::make_shared<OpenGLModel> ( model_instance->GetModel(), mOpenGLRenderer ) );
            }
        } );

#if _WIN32
        SwapBuffers ( hdc );
        ReleaseDC ( reinterpret_cast<HWND> ( mWindowId ), hdc );
#else
        glXSwapBuffers ( static_cast<Display*> ( mOpenGLRenderer->GetWindowId() ),
                         reinterpret_cast<::Window> ( mWindowId ) );
#endif
    }

    const GLuint OpenGLWindow::GetMatricesBuffer() const
    {
        return mMatricesBuffer;
    }

    void OpenGLWindow::Initialize()
    {
        if ( !mOpenGLRenderer )
        {
            throw std::runtime_error ( "Pointer to OpenGL Renderer is nullptr." );
        }
#if _WIN32
        HDC hdc = GetDC ( static_cast<HWND> ( mWindowId ) );
        PIXELFORMATDESCRIPTOR pfd{};
        pfd.nSize = sizeof ( PIXELFORMATDESCRIPTOR );
        pfd.nVersion = 1;
        pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
        pfd.iPixelType = PFD_TYPE_RGBA;
        pfd.cColorBits = 32;
        pfd.cDepthBits = 32;
        pfd.iLayerType = PFD_MAIN_PLANE;
        int pf = ChoosePixelFormat ( hdc, &pfd );
        SetPixelFormat ( hdc, pf, &pfd );
        wglMakeCurrent ( hdc, static_cast<HGLRC> ( mOpenGLRenderer->GetOpenGLContext() ) );
        ReleaseDC ( static_cast<HWND> ( mWindowId ), hdc );
        RECT rect;
        GetClientRect ( static_cast<HWND> ( mWindowId ), &rect );
        glViewport ( 0, 0, rect.right, rect.bottom );
        OPENGL_CHECK_ERROR_THROW;
#else
        XWindowAttributes x_window_attributes {};
        XGetWindowAttributes ( static_cast<Display*> ( mOpenGLRenderer->GetWindowId() ),
                               reinterpret_cast<::Window> ( mWindowId ), &x_window_attributes );
        std::cout << "Visual " << x_window_attributes.visual <<
                  " Default " << DefaultVisual ( static_cast<Display*> ( mOpenGLRenderer->GetWindowId() ),
                          DefaultScreen ( static_cast<Display*> ( mOpenGLRenderer->GetWindowId() ) ) ) << std::endl;
        XSetErrorHandler ( [] ( Display * display, XErrorEvent * error_event ) -> int
        {
            std::cout << "Error Code " << static_cast<int> ( error_event->error_code ) << std::endl;
            return 0;
        } );
        if ( !glXMakeCurrent (  static_cast<Display*> ( mOpenGLRenderer->GetWindowId() ),
                                reinterpret_cast<::Window> ( mWindowId ),
                                static_cast<GLXContext> ( mOpenGLRenderer->GetOpenGLContext() ) ) )
        {
            std::ostringstream stream;
            stream << "Failed to make OpenGL current to XWindow. Error: ";
            throw std::runtime_error ( stream.str().c_str() );
        }
        XSetErrorHandler ( nullptr );
#endif
        glClearColor ( 0.5f, 0.5f, 0.5f, 1.0f );
        OPENGL_CHECK_ERROR_NO_THROW;
        glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        OPENGL_CHECK_ERROR_NO_THROW;
        glBlendFunc ( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
        OPENGL_CHECK_ERROR_NO_THROW;
        glEnable ( GL_BLEND );
        OPENGL_CHECK_ERROR_NO_THROW;
        glDepthFunc ( GL_LESS );
        OPENGL_CHECK_ERROR_NO_THROW;
        glEnable ( GL_DEPTH_TEST );
        OPENGL_CHECK_ERROR_NO_THROW;
        glCullFace ( GL_BACK );
        OPENGL_CHECK_ERROR_NO_THROW;
        glEnable ( GL_CULL_FACE );
        OPENGL_CHECK_ERROR_NO_THROW;
        glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        OPENGL_CHECK_ERROR_NO_THROW;

        // Initialize Matrix Buffer
        glGenBuffers ( 1, &mMatricesBuffer );
        OPENGL_CHECK_ERROR_NO_THROW;
        glBindBuffer ( GL_UNIFORM_BUFFER, mMatricesBuffer );
        OPENGL_CHECK_ERROR_NO_THROW;
        float matrices[32] =
        {
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f,
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        };
        glBufferData ( GL_UNIFORM_BUFFER, sizeof ( float ) * 32,
                       matrices, GL_DYNAMIC_DRAW );
        OPENGL_CHECK_ERROR_NO_THROW;
    }

    void OpenGLWindow::Finalize()
    {
        if ( glIsBuffer ( mMatricesBuffer ) )
        {
            OPENGL_CHECK_ERROR_NO_THROW;
            glDeleteBuffers ( 1, &mMatricesBuffer );
            OPENGL_CHECK_ERROR_NO_THROW;
            mMatricesBuffer = 0;
        }
        OPENGL_CHECK_ERROR_NO_THROW;
    }
}
