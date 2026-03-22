/*
Copyright (C) 2016-2019,2021,2022,2025,2026 Rodrigo Jose Hernandez Cordoba

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
#include "aeongames/AeonEngine.hpp"
#include "aeongames/Scene.hpp"
#include "aeongames/Node.hpp"
#include "aeongames/Matrix4x4.hpp"

namespace AeonGames
{
    class Renderer;
    class Scene;
    class Node;
    class Window;
    /** @brief Rendering window that hosts the engine viewport. */
    class EngineWindow : public QWindow
    {
        Q_OBJECT
    public:
        /**
         * @brief Construct the engine window.
         * @param parent Parent window.
         */
        EngineWindow ( QWindow *parent = nullptr );
        /** @brief Destructor. */
        ~EngineWindow();
        /** @brief Stop the rendering loop. */
        void stop();
        /** @brief Start the rendering loop. */
        void start();
        /**
         * @brief Set the scene to render.
         * @param aScene Pointer to the scene, or nullptr to clear.
         */
        void setScene ( const Scene* aScene );
        /**
         * @brief Set the camera field of view.
         * @param aFieldOfView Field of view in degrees.
         */
        void SetFieldOfView ( float aFieldOfView );
        /**
         * @brief Set the near clipping plane distance.
         * @param aNear Near plane distance.
         */
        void SetNear ( float aNear );
        /**
         * @brief Set the far clipping plane distance.
         * @param aFar Far plane distance.
         */
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
        QPointF mLastCursorPosition;
        QTimer mTimer;
        QElapsedTimer mStopWatch;
        float mFrustumVerticalHalfAngle;
        float mAspectRatio{1.0f};
        float mStep;
        float mFieldOfView{};
        float mNear{};
        float mFar{};
        bool mIsClosing{ false };
        /* Settings Cache */
        uint32_t mHorizontalSpacing{};
        uint32_t mVerticalSpacing{};
        /* We're using QT classes for now... */
        QQuaternion mCameraRotation;
        QVector3D mCameraLocation;
        QMatrix4x4 mViewMatrix;
    };
}
#endif
