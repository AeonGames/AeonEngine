/*
Copyright (C) 2018,2019,2022 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_SCENEWINDOW_H
#define AEONGAMES_SCENEWINDOW_H

#include <QList>
#include <memory>
#include "ui_SceneWindow.h"
#include "models/SceneModel.h"
#include "models/ComponentListModel.h"
#include "models/ComponentModel.h"
#include "delegates/PropertyDelegate.h"

namespace AeonGames
{
    class Renderer;
    class EngineWindow;
    class SceneWindow : public QWidget
    {
        Q_OBJECT
    public:
        SceneWindow ( QWidget *parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags() );
        virtual ~SceneWindow();
        void Open ( const std::string& mFilename );
        void Save ( const std::string& mFilename ) const;
        void SetFieldOfView ( float aFieldOfView );
        void SetNear ( float aNear );
        void SetFar ( float aFar );
    private slots:
        void on_actionAddNode_triggered();
        void on_actionRemoveNode_triggered();
        void on_actionSetCameraNode_triggered();
        void on_actionRemoveComponent_triggered();
        void on_sceneContextMenuRequested ( const QPoint& aPoint );
        void on_sceneTreeViewClicked ( const QModelIndex& aModelIndex );
        void on_componentContextMenuRequested ( const QPoint& aPoint );
        void on_componentListViewClicked ( const QModelIndex& aModelIndex );
        void on_localTransformChanged();
        void on_globalTransformChanged();
    private:
        void UpdateLocalTransformData ( const Node* aNode );
        void UpdateGlobalTransformData ( const Node* aNode );
        Ui::SceneWindow mUi;
        ///@todo SceneWindow should own the scene, not SceneModel
        SceneModel mSceneModel{};
        ComponentListModel mComponentListModel{};
        ComponentModel mComponentModel{};
        PropertyDelegate mPropertyDelegate{};
        EngineWindow* mEngineWindow{};
        QList<QAction *> mComponentAddActions{};
    };
}
#endif
