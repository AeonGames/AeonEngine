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
#ifndef AEONGAMES_MAINWINDOW_H
#define AEONGAMES_MAINWINDOW_H

#include <memory>
#include "ui_MainWindow.h"

namespace AeonGames
{
    class NodeEditor;
    class CameraSettings;
    class Renderer;
    class EngineWindow;
    class MainWindow : public QMainWindow, public Ui::MainWindow
    {
        Q_OBJECT
    public:
        MainWindow();
        ~MainWindow();
    private slots:
        void on_actionNewScene_triggered();
        void on_actionNewShader_triggered();
        void on_actionOpen_triggered();
        void on_actionSave_triggered();
        void on_actionExit_triggered();
        void on_actionCamera_triggered();
        void fieldOfViewChanged ( double aFieldOfView );
        void nearChanged ( double aNear );
        void farChanged ( double aFar );
    private:
        CameraSettings* mCameraSettings{};
    };
}
#endif
