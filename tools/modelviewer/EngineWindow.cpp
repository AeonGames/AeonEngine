/*
Copyright (C) 2016-2018 Rodrigo Jose Hernandez Cordoba

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
#define _USE_MATH_DEFINES
#include <QResizeEvent>
#include <QString>
#include <QStringList>
#include <cassert>
#include <cmath>
#include <iostream>
#include <exception>
#include <stdexcept>
#include "EngineWindow.h"
#include "aeongames/Renderer.h"
#include "aeongames/Model.h"
#include "aeongames/ModelInstance.h"
#include "aeongames/Animation.h"
#include "aeongames/Mesh.h"
#include "aeongames/ResourceCache.h"
#include "aeongames/Window.h"
#include "aeongames/Scene.h"
#include "aeongames/Node.h"

namespace AeonGames
{
    EngineWindow::EngineWindow ( const std::shared_ptr<Renderer> aRenderer, QWindow *parent ) :
        QWindow ( parent ),
        mTimer(),
        mStopWatch(),
        mRenderer ( aRenderer ),
        mWindow(),
        mScene ( std::make_shared<Scene>() ),
        mNode ( std::make_shared<Node>() ),
        mFrustumVerticalHalfAngle ( 0 ), mStep ( 0 ),
        mCameraRotation ( QQuaternion::fromAxisAndAngle ( 0.0f, 0.0f, 1.0f, 45.0f ) * QQuaternion::fromAxisAndAngle ( 1.0f, 0.0f, 0.0f, -30.0f ) ),
        mCameraLocation ( 45.9279297f, -45.9279358f, 37.4999969f, 1 ),
        mProjectionMatrix(),
        mViewMatrix()
    {
        /*
        @todo The id argument should be set to something other than 0,
        std::hash<std::string>()("Animation") is tempting
        BUT we dont want to be calculating string hashes on runtime.
        */
        mNode->AttachUpdater ( 0, {}, [] ( Node & aNode, double aDelta )
        {
            if ( ModelInstance* model_instance = reinterpret_cast<ModelInstance*> ( aNode.GetAttribute ( ModelInstance::TypeId ) ) )
            {
                model_instance->StepAnimation ( aDelta );
            }
        } );
        mScene->AddNode ( mNode );
        // Hopefully these settings are optimal for Vulkan as well as OpenGL
        setSurfaceType ( QSurface::OpenGLSurface );

        /* On Windows we want the window to own its device context.
        This code works on Qt 5.6.0, but if it does not
        or a new non Qt application needs to set it,
        a window class style seems to be mutable by using
        SetClassLongPtr( ..., GCL_STYLE, ... );
        instead.
        https://msdn.microsoft.com/en-us/library/windows/desktop/ms633589%28v=vs.85%29.aspx
        http://www.gamedev.net/topic/319124-ogl-and-windows/#entry3055268

        This flag is Windows specific, so Linux or Mac implementations
        will probably just ignore it, I have not verified this though.
        UPDATE: Linux compilation does not complain about this code.
        */
        auto current_flags = flags();
        if ( ! ( current_flags & Qt::MSWindowsOwnDC ) )
        {
            setFlags ( current_flags | Qt::MSWindowsOwnDC );
        }
        mWindow = mRenderer->CreateWindowProxy ( reinterpret_cast<void*> ( winId() ) );
        if ( !mWindow )
        {
            throw std::runtime_error ( "Window creation failed." );
        }
        connect ( &mTimer, SIGNAL ( timeout() ), this, SLOT ( requestUpdate() ) );
        updateViewMatrix();
        float half_radius = ( static_cast<float> ( geometry().size().width() ) / static_cast<float> ( geometry().size().height() ) ) / 2;
        float v1[2] = { 1, 0 };
        float v2[2] = { 1, half_radius };
        float length = sqrtf ( ( v2[0] * v2[0] ) + ( v2[1] * v2[1] ) );
        v2[0] /= length;
        v2[1] /= length;
        mFrustumVerticalHalfAngle = acosf ( ( v1[0] * v2[0] ) + ( v1[1] * v2[1] ) );
    }

    EngineWindow::~EngineWindow()
    {
        // Force model deletion
        stop();
        mWindow.reset();
    }

    void EngineWindow::stop()
    {
        if ( mStopWatch.isValid() && mTimer.isActive() )
        {
            mTimer.stop();
            mStopWatch.invalidate();
        }
    }

    void EngineWindow::start()
    {
        if ( !mStopWatch.isValid() && !mTimer.isActive() )
        {
            mTimer.start ( 0 );
            mStopWatch.start();
        }
    }

    void EngineWindow::setModel ( const QString & filename )
    {
        /**@todo We probably don't want to expose the Resource Cache this way to avoid misuse.*/
        mNode->SetAttribute ( ModelInstance::TypeId, std::make_shared<ModelInstance> ( Get<Model> ( filename.toUtf8().constData(), filename.toUtf8().constData() ) ) );
        assert ( mNode->GetAttribute ( ModelInstance::TypeId ) && "ModelInstance is a nullptr" );
        if ( ModelInstance* model_instance = reinterpret_cast<ModelInstance*> ( mNode->GetAttribute ( ModelInstance::TypeId ) ) )
        {
            // Adjust camera position so model fits the frustum tightly.
            float diameter = model_instance->GetModel()->GetCenterRadii().GetRadii().GetMaxAxisLenght() * 2;
            std::cout << "Diameter: " << diameter << std::endl;
            // Add the near value to the radius just in case the actual object contains the eye position.
            float half_radius = ( static_cast<float> ( geometry().size().width() ) / static_cast<float> ( geometry().size().height() ) ) / 2;
            float v1[2] = { 1, 0 };
            float v2[2] = { 1, half_radius };
            float length = sqrtf ( ( v2[0] * v2[0] ) + ( v2[1] * v2[1] ) );
            v2[0] /= length;
            v2[1] /= length;
            mFrustumVerticalHalfAngle = acosf ( ( v1[0] * v2[0] ) + ( v1[1] * v2[1] ) );

            assert ( mFrustumVerticalHalfAngle != 0.0f );
            float eye_length = ( diameter ) / std::tan ( mFrustumVerticalHalfAngle );
            mCameraLocation = QVector4D (
                                  QVector3D (
                                      model_instance->GetModel()->GetCenterRadii().GetCenter() [0],
                                      model_instance->GetModel()->GetCenterRadii().GetCenter() [1],
                                      model_instance->GetModel()->GetCenterRadii().GetCenter() [2] ) +
                                  ( mCameraRotation.rotatedVector ( -forward ) * eye_length ), 1 );
            updateViewMatrix();
            mStep = eye_length / 100.0f;
        }
    }

    void EngineWindow::resizeEvent ( QResizeEvent * aResizeEvent )
    {
        if ( aResizeEvent->size().width() && aResizeEvent->size().height() )
        {
            mWindow->ResizeViewport ( aResizeEvent->size().width(), aResizeEvent->size().height() );
            static const QMatrix4x4 flipMatrix (
                1.0f, 0.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 1.0f, 0.0f,
                0.0f, -1.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 0.0f, 1.0f );
            mProjectionMatrix.setToIdentity();
            float half_radius = ( static_cast<float> ( aResizeEvent->size().width() ) / static_cast<float> ( aResizeEvent->size().height() ) ) / 2;
            mProjectionMatrix.frustum ( -half_radius, half_radius, 0.5, -0.5, 1, 1600 );
            mProjectionMatrix = mProjectionMatrix * flipMatrix;
            mWindow->SetProjectionMatrix ( mProjectionMatrix.constData() );
            // Calculate frustum half vertical angle (for fitting models into frustum)
            float v1[2] = { 1, 0 };
            float v2[2] = { 1, half_radius };
            float length = sqrtf ( ( v2[0] * v2[0] ) + ( v2[1] * v2[1] ) );
            v2[0] /= length;
            v2[1] /= length;
            mFrustumVerticalHalfAngle = acosf ( ( v1[0] * v2[0] ) + ( v1[1] * v2[1] ) );
        }
    }

    void EngineWindow::exposeEvent ( QExposeEvent * aExposeEvent )
    {
        Q_UNUSED ( aExposeEvent );
        if ( isExposed() )
        {
            start();
        }
        else
        {
            stop();
        }
    }

    bool EngineWindow::event ( QEvent * aEvent )
    {
        switch ( aEvent->type() )
        {
        case QEvent::UpdateRequest:
        {
            double delta = 0.0;
            if ( mStopWatch.isValid() )
            {
                delta = mStopWatch.restart() * 1e-3f;
                if ( delta > 1e-1f )
                {
                    delta = 1.0 / 30.0;
                }
            }
            mScene->Update ( delta );
            mWindow->Render ( mScene );
            return true;
        }
        default:
            return QWindow::event ( aEvent );
        }
    }

    void EngineWindow::updateViewMatrix()
    {
        Transform view_transform;
        view_transform.SetTranslation ( Vector3 ( mCameraLocation.x(), mCameraLocation.y(), mCameraLocation.z() ) );
        view_transform.SetRotation ( Quaternion ( mCameraRotation.scalar(), mCameraRotation.x(), mCameraRotation.y(), mCameraRotation.z() ) );
        mWindow->SetViewTransform ( view_transform );
    }

    void EngineWindow::keyPressEvent ( QKeyEvent * event )
    {
        switch ( event->key() )
        {
        case Qt::Key_W:
            mCameraLocation += ( mCameraRotation.rotatedVector ( forward ) * mStep );
            break;
        case Qt::Key_S:
            mCameraLocation -= ( mCameraRotation.rotatedVector ( forward ) * mStep );
            break;
        case Qt::Key_D:
            mCameraLocation += ( mCameraRotation.rotatedVector ( right ) * mStep );
            break;
        case Qt::Key_A:
            mCameraLocation -= ( mCameraRotation.rotatedVector ( right ) * mStep );
            break;
        }
        updateViewMatrix();
        event->accept();
    }

    void EngineWindow::keyReleaseEvent ( QKeyEvent * event )
    {
        event->accept();
    }

    void EngineWindow::mouseMoveEvent ( QMouseEvent * event )
    {
#if 1
        if ( event->buttons() & Qt::LeftButton )
        {
            QPoint movement = event->globalPos() - mLastCursorPosition;
            mLastCursorPosition = event->globalPos();
            mCameraRotation = QQuaternion::fromAxisAndAngle ( 0, 0, 1, - ( static_cast<float> ( movement.x() ) / 2.0f ) ) * mCameraRotation;
            mCameraRotation = mCameraRotation * QQuaternion::fromAxisAndAngle ( 1, 0, 0, - ( static_cast<float> ( movement.y() ) / 2.0f ) );
            updateViewMatrix();
        }
#endif
        event->accept();
    }

    void EngineWindow::mousePressEvent ( QMouseEvent * event )
    {
        if ( event->button() & Qt::LeftButton )
        {
            mLastCursorPosition = event->globalPos();
        }
        event->accept();
    }

    void EngineWindow::mouseReleaseEvent ( QMouseEvent * event )
    {
        event->accept();
    }

    void EngineWindow::wheelEvent ( QWheelEvent *event )
    {
        mCameraLocation += ( ( mCameraRotation.rotatedVector ( forward ) * mStep ) *
                             ( event->angleDelta().y() / std::abs ( event->angleDelta().y() ) ) );
        updateViewMatrix();
        event->accept();
    }
}
