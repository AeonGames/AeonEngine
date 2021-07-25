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
#ifndef AEONGAMES_ENGINEWINDOW_H
#define AEONGAMES_ENGINEWINDOW_H
#include <QWindow>
#include <QTimer>
#include <QElapsedTimer>
#include <QQuaternion>
#include <QVector3D>
#include <QVector4D>
#include <QMatrix4x4>
#include <QPoint>
#include <QCloseEvent>

#include "WorldEditor.h"
#include "aeongames/AeonEngine.h"
#include "aeongames/Scene.h"
#include "aeongames/Node.h"
#include "aeongames/Matrix4x4.h"

namespace AeonGames
{
    class Renderer;
    class Scene;
    class Node;
    class Window;
    class EngineWindow : public QWindow
    {
        Q_OBJECT
    public:
        EngineWindow ( QWindow *parent = nullptr );
        ~EngineWindow();
        void stop();
        void start();
        void setScene ( const Scene* aScene );
        void SetFieldOfView ( float aFieldOfView );
        void SetNear ( float aNear );
        void SetFar ( float aFar );
    private:
        const QVector3D right
        {
            1.0f, 0.0f, 0.0f
        };
        const QVector3D forward{ 0.0f, 1.0f, 0.0f };
        void resizeEvent ( QResizeEvent *aResizeEvent ) final;
        void exposeEvent ( QExposeEvent *aExposeEvent ) final;
        void keyPressEvent ( QKeyEvent * event ) final;
        void keyReleaseEvent ( QKeyEvent * event ) final;
        void mouseMoveEvent ( QMouseEvent * event ) final;
        void mousePressEvent ( QMouseEvent * event ) final;
        void mouseReleaseEvent ( QMouseEvent * event ) final;
        void wheelEvent ( QWheelEvent *event ) final;
        bool event ( QEvent* aEvent ) final;
        void updateViewMatrix();
        void* mWinId{ nullptr };
        const Scene* mScene{nullptr};
        QPoint mLastCursorPosition;
        QTimer mTimer;
        QElapsedTimer mStopWatch;
        float mFrustumVerticalHalfAngle;
        float mAspectRatio{1.0f};
        float mStep;
        float mFieldOfView{};
        float mNear{};
        float mFar{};
        /* We're using QT classes for now... */
        QQuaternion mCameraRotation;
        QVector4D mCameraLocation;
        QMatrix4x4 mViewMatrix;
    };
}
#endif
