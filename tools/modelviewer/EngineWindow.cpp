/*
Copyright 2016 Rodrigo Jose Hernandez Cordoba

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
#include <cassert>
#include "EngineWindow.h"
#include "aeongames/Model.h"

namespace AeonGames
{
    EngineWindow::EngineWindow ( QWindow *parent ) : QWindow ( parent ), mTimer(), mAeonEngine(), mScene(),
        mCameraRotation ( QQuaternion::fromAxisAndAngle ( 0.0f, 0.0f, 1.0f, 45.0f ) * QQuaternion::fromAxisAndAngle ( 1.0f, 0.0f, 0.0f, -30.0f ) ),
        mCameraLocation ( 45.9279297f, -45.9279358f, 37.4999969f, 1 ),
        mProjectionMatrix(),
        mViewMatrix()
    {
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
        */
        auto current_flags = flags();
        if ( ! ( current_flags & Qt::MSWindowsOwnDC ) )
        {
            setFlags ( current_flags | Qt::MSWindowsOwnDC );
        }
        if ( !mAeonEngine.RegisterRenderingWindow ( winId() ) )
        {
            throw std::runtime_error ( "Window registration failed." );
        }
        connect ( &mTimer, SIGNAL ( timeout() ), this, SLOT ( requestUpdate() ) );
        updateViewMatrix();
        mAeonEngine.SetScene ( &mScene );
    }

    EngineWindow::~EngineWindow()
    {
        mAeonEngine.UnregisterRenderingWindow ( winId() );
    }

    void EngineWindow::setMesh ( const QString & filename )
    {
        static Model* model = nullptr;
        if ( !model )
        {
            mScene.AddNode ( model = new Model );
        }
        mMesh = mAeonEngine.GetMesh ( filename.toUtf8().constData() );
        //mProgram = mAeonEngine.GetProgram ( "game/shaders/plain_red.txt" );
        mProgram = mAeonEngine.GetProgram ( "game/shaders/fixed_phong.txt" );
        assert ( model && "Model is nullptr" );
        model->SetMesh ( mMesh );
        model->SetProgram ( mProgram );
    }

    void EngineWindow::resizeEvent ( QResizeEvent * aResizeEvent )
    {
        mAeonEngine.Resize ( winId(), aResizeEvent->size().width(), aResizeEvent->size().height() );
        static const QMatrix4x4 flipMatrix (
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, -1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f );
        mProjectionMatrix.setToIdentity();
        float half_radius = ( static_cast<float> ( aResizeEvent->size().width() ) / static_cast<float> ( aResizeEvent->size().height() ) ) / 2;
        mProjectionMatrix.frustum ( -half_radius, half_radius, -0.5, 0.5, 1, 16000 );
        mProjectionMatrix = mProjectionMatrix * flipMatrix;
        mAeonEngine.SetProjectionMatrix ( mProjectionMatrix.constData() );
    }

    void EngineWindow::exposeEvent ( QExposeEvent * aExposeEvent )
    {
        Q_UNUSED ( aExposeEvent );
        if ( isExposed() )
        {
            mTimer.start ( 0 );
            mStopWatch.start();
        }
        else
        {
            mTimer.stop();
            mStopWatch.invalidate();
        }
    }
    bool EngineWindow::event ( QEvent * aEvent )
    {
        switch ( aEvent->type() )
        {
        case QEvent::UpdateRequest:
            mAeonEngine.Step ( 0.0 );
            return true;
        default:
            return QWindow::event ( aEvent );
        }
    }
    void EngineWindow::updateViewMatrix()
    {
        mViewMatrix.setToIdentity();
        mViewMatrix.rotate ( mCameraRotation );
        mViewMatrix.setColumn ( 3, mCameraLocation );
        mViewMatrix = mViewMatrix.inverted();
        mAeonEngine.SetViewMatrix ( mViewMatrix.constData() );
    }

    void EngineWindow::keyPressEvent ( QKeyEvent * event )
    {
        switch ( event->key() )
        {
        case Qt::Key_W:
            mCameraLocation += ( mCameraRotation.rotatedVector ( forward ) * 10.0f );
            break;
        case Qt::Key_S:
            mCameraLocation -= ( mCameraRotation.rotatedVector ( forward ) * 10.0f );
            break;
        case Qt::Key_D:
            mCameraLocation += ( mCameraRotation.rotatedVector ( right ) * 10.0f );
            break;
        case Qt::Key_A:
            mCameraLocation -= ( mCameraRotation.rotatedVector ( right ) * 10.0f );
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
            event->accept();
        }
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
        mCameraLocation += ( mCameraRotation.rotatedVector ( forward ) * ( event->angleDelta().y() / 2.0f ) );
        updateViewMatrix();
        event->accept();
    }
}
