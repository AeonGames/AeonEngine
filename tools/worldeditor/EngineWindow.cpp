/*
Copyright (C) 2016-2019,2021 Rodrigo Jose Hernandez Cordoba

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
#include <QResizeEvent>
#include <QString>
#include <QStringList>
#ifdef __unix__
#include <QX11Info>
#endif
#include <cassert>
#include <cmath>
#include <iostream>
#include <exception>
#include <stdexcept>
#include "WorldEditor.h"
#include "EngineWindow.h"
#include "aeongames/Renderer.h"
#include "aeongames/Model.h"
#include "aeongames/Animation.h"
#include "aeongames/Mesh.h"
#include "aeongames/ResourceCache.h"
#include "aeongames/Window.h"
#include "aeongames/Frustum.h"
#include "aeongames/CRC.h"
#include "aeongames/Node.h"

namespace AeonGames
{
    EngineWindow::EngineWindow ( QWindow *parent ) :
        QWindow ( parent ),
        mTimer(),
        mStopWatch(),
        mWindow(),
        mFrustumVerticalHalfAngle ( 0 ), mStep ( 10 ),
        mCameraRotation ( QQuaternion::fromAxisAndAngle ( 0.0f, 0.0f, 1.0f, 45.0f ) * QQuaternion::fromAxisAndAngle ( 1.0f, 0.0f, 0.0f, -30.0f ) ),
        // Stand back 3 meters.
        mCameraLocation ( -QVector3D ( mCameraRotation.rotatedVector ( forward ) * 300.0f ), 1 ),
        mViewMatrix()
    {
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
        mWindow =  qWorldEditorApp->GetRenderer()->CreateWindowProxy ( reinterpret_cast<void*> ( winId() ) );
        if ( !mWindow )
        {
            throw std::runtime_error ( "Window creation failed." );
        }
        connect ( &mTimer, SIGNAL ( timeout() ), this, SLOT ( requestUpdate() ) );

        auto& settings = qWorldEditorApp->GetSettings();
        settings.beginGroup ( "Camera" );
        mFieldOfView = settings.value ( "FieldOfView", 60.0f ).toFloat();
        mNear = settings.value ( "Near", 1.0f ).toFloat();
        mFar = settings.value ( "Far", 1600.0f ).toFloat();
        settings.endGroup();

        updateViewMatrix();
#if 0
        float half_radius = ( static_cast<float> ( geometry().size().width() ) / static_cast<float> ( geometry().size().height() ) ) / 2;
        float v1[2] = { 1, 0 };
        float v2[2] = { 1, half_radius };
        float length = sqrtf ( ( v2[0] * v2[0] ) + ( v2[1] * v2[1] ) );
        v2[0] /= length;
        v2[1] /= length;
        mFrustumVerticalHalfAngle = acosf ( ( v1[0] * v2[0] ) + ( v1[1] * v2[1] ) );
#endif
    }

    EngineWindow::~EngineWindow()
    {
        stop();
        mTimer.disconnect();
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

    void EngineWindow::setScene ( const Scene* aScene )
    {
        mScene = aScene;
    }
    void EngineWindow::SetFieldOfView ( float aFieldOfView )
    {
        mFieldOfView = aFieldOfView;
        Matrix4x4 projection{};
        projection.Perspective ( mFieldOfView, mWindow->GetAspectRatio(), mNear, mFar );
        mWindow->SetProjectionMatrix ( projection );
    }
    void EngineWindow::SetNear ( float aNear )
    {
        mNear = aNear;
        Matrix4x4 projection{};
        projection.Perspective ( mFieldOfView, mWindow->GetAspectRatio(), mNear, mFar );
        mWindow->SetProjectionMatrix ( projection );
    }
    void EngineWindow::SetFar ( float aFar )
    {
        mFar = aFar;
        Matrix4x4 projection{};
        projection.Perspective ( mFieldOfView, mWindow->GetAspectRatio(), mNear, mFar );
        mWindow->SetProjectionMatrix ( projection );
    }
#if 0
    // Commented pending Refactor
    void EngineWindow::setModel ( const QString & filename )
    {
        /**@todo We probably don't want to expose the Resource Cache this way to avoid misuse.*/
        mNode->AttachComponent ( ModelInstance::TypeId, {},
                                 std::make_shared<ModelInstance> (
                                     Get<Model> ( filename.toUtf8().constData(),
                                             filename.toUtf8().constData() ) ) );
        assert ( mNode->GetComponent ( ModelInstance::TypeId ) && "ModelInstance is a nullptr" );
        mRenderer->Unload ( *mNode );
        mRenderer->Load ( *mNode );
        if ( ModelInstance* model_instance = reinterpret_cast<ModelInstance*> ( mNode->GetComponent ( ModelInstance::TypeId ) ) )
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

            if ( mFrustumVerticalHalfAngle == 0.0f )
            {
                throw std::runtime_error ( "mFrustumVerticalHalfAngle is zero." );
            }

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
#endif

    void EngineWindow::resizeEvent ( QResizeEvent * aResizeEvent )
    {
        if ( aResizeEvent->size() != aResizeEvent->oldSize() && mWindow && aResizeEvent->size().width() && aResizeEvent->size().height() )
        {
#ifdef Q_OS_WIN
            // This is a workaround
            QMargins margins{QWindow::frameMargins() };
            mWindow->ResizeViewport (
                margins.left(),
                margins.top(),
                aResizeEvent->size().width(), aResizeEvent->size().height() );
#else
            mWindow->ResizeViewport (
                0,
                0,
                aResizeEvent->size().width(), aResizeEvent->size().height() );
#endif
            Matrix4x4 projection {};
            projection.Perspective ( mFieldOfView, mWindow->GetAspectRatio(), mNear, mFar );
            mWindow->SetProjectionMatrix ( projection );
#if 0
            static const QMatrix4x4 flipMatrix (
                1.0f, 0.0f, 0.0f, 0.0f,
                0.0f, 0.0f, -1.0f, 0.0f,
                0.0f, -1.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 0.0f, 1.0f );
            mProjectionMatrix.setToIdentity();
            float half_radius = ( static_cast<float> ( aResizeEvent->size().width() ) / static_cast<float> ( aResizeEvent->size().height() ) ) / 2;
            mProjectionMatrix.frustum ( -half_radius, half_radius, -0.5, 0.5, 1, 1600 );
            mProjectionMatrix = mProjectionMatrix * flipMatrix;
            mWindow->SetProjectionMatrix ( mProjectionMatrix.constData() );
            // Calculate frustum half vertical angle (for fitting nodes into frustum)
            float v1[2] = { 1, 0 };
            float v2[2] = { 1, mWindow->GetHalfAspectRatio() };
            float length = sqrtf ( ( v2[0] * v2[0] ) + ( v2[1] * v2[1] ) );
            v2[0] /= length;
            v2[1] /= length;
            mFrustumVerticalHalfAngle = acosf ( ( v1[0] * v2[0] ) + ( v1[1] * v2[1] ) );
#endif
            start();
        }
        else
        {
            stop();
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
            if ( !geometry().width() || !geometry().height() || qWorldEditorApp->IsBlocked() || !qWorldEditorApp->GetRenderer() )
            {
                return QWindow::event ( aEvent );
            }
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
                if ( mScene )
                {
                    const_cast<Scene*> ( mScene )->Update ( delta, mWindow.get() );
                }
                mWindow->BeginRender();
                mWindow->Render ( Transform{},
                                  qWorldEditorApp->GetGridMesh(),
                                  qWorldEditorApp->GetGridPipeline(),
                                  &qWorldEditorApp->GetXGridMaterial(), nullptr, 0, 2,
                                  qWorldEditorApp->GetGridSettings().horizontalSpacing() + 1 );
                mWindow->Render ( Transform{},
                                  qWorldEditorApp->GetGridMesh(),
                                  qWorldEditorApp->GetGridPipeline(),
                                  &qWorldEditorApp->GetYGridMaterial(), nullptr, 2, 2,
                                  qWorldEditorApp->GetGridSettings().verticalSpacing() + 1 );
                /** @todo This should be the code path for edit mode,
                 * game mode should just render the scene using Window::Render(const Scene&)
                */
                if ( mScene && mScene->GetChildrenCount() )
                {
                    Frustum frustum ( mWindow->GetProjectionMatrix() * mWindow->GetViewMatrix() );
                    mScene->LoopTraverseDFSPreOrder ( [this, &frustum] ( const Node & aNode )
                    {
                        if ( &aNode == mScene->GetCamera() )
                        {
                            Matrix4x4 projection_matrix{};
                            projection_matrix.Perspective ( mScene->GetFieldOfView(), mWindow->GetAspectRatio(), mScene->GetNear(), mScene->GetFar() );
                            projection_matrix.Invert();
                            mWindow->Render ( aNode.GetGlobalTransform() * projection_matrix,
                                              qWorldEditorApp->GetAABBWireMesh(),
                                              qWorldEditorApp->GetWirePipeline(),
                                              &qWorldEditorApp->GetWireMaterial() );
                        }

                        AABB transformed_aabb = aNode.GetGlobalTransform() * aNode.GetAABB();
                        if ( frustum.Intersects ( transformed_aabb ) )
                        {
                            // Call Node specific rendering function.
                            aNode.Render ( *mWindow );
                            // Render Node AABBss
                            mWindow->Render ( transformed_aabb.GetTransform(),
                                              qWorldEditorApp->GetAABBWireMesh(),
                                              qWorldEditorApp->GetWirePipeline(),
                                              &qWorldEditorApp->GetWireMaterial() );
                            // Render Node Root
                            mWindow->Render ( aNode.GetGlobalTransform(),
                                              qWorldEditorApp->GetAABBWireMesh(),
                                              qWorldEditorApp->GetWirePipeline(),
                                              &qWorldEditorApp->GetWireMaterial() );
                            // Render AABB Center
                            mWindow->Render (   Transform{Vector3{1, 1, 1},
                                                          Quaternion{1, 0, 0, 0},
                                                          Vector3{transformed_aabb.GetCenter() }},
                                                qWorldEditorApp->GetAABBWireMesh(),
                                                qWorldEditorApp->GetWirePipeline(),
                                                &qWorldEditorApp->GetWireMaterial() );
                        }
                    } );
                }
                mWindow->EndRender();
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
        mWindow->SetViewMatrix ( view_transform.GetInverted().GetMatrix() );
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
        if ( event->buttons() & Qt::LeftButton )
        {
            QPoint movement = event->globalPos() - mLastCursorPosition;
            mLastCursorPosition = event->globalPos();
            mCameraRotation = QQuaternion::fromAxisAndAngle ( 0, 0, 1, - ( static_cast<float> ( movement.x() ) / 2.0f ) ) * mCameraRotation;
            mCameraRotation = mCameraRotation * QQuaternion::fromAxisAndAngle ( 1, 0, 0, - ( static_cast<float> ( movement.y() ) / 2.0f ) );
            updateViewMatrix();
        }
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
