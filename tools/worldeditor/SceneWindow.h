/*
Copyright (C) 2018 Rodrigo Jose Hernandez Cordoba

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

#include <memory>
#include "ui_SceneWindow.h"
#include "models/SceneModel.h"

namespace AeonGames
{
    class Renderer;
    class EngineWindow;
    class SceneWindow : public QWidget, public Ui::SceneWindow
    {
        Q_OBJECT
    public:
        SceneWindow ( const std::shared_ptr<Renderer>& aRenderer, QWidget *parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags() );
        virtual ~SceneWindow();
        void setModel ( const QString& filename );
    private slots:
        void on_actionAddNode_triggered();
        void on_actionRemoveNode_triggered();
        void on_customContextMenuRequested ( const QPoint& aPoint );
    private:
        std::shared_ptr<Renderer> mRenderer;
        SceneModel mSceneModel{};
        EngineWindow* mEngineWindow{};
    };
}
#endif
