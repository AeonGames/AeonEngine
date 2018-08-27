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
#include <QFileDialog>
#include <QMdiSubWindow>
#include <QSurfaceFormat>
#include "aeongames/Renderer.h"
#include "WorldEditor.h"
#include "MainWindow.h"
#include "SceneWindow.h"
#include "EngineWindow.h"

namespace AeonGames
{
    MainWindow::MainWindow() : QMainWindow(), Ui::MainWindow()
    {
        setupUi ( this );
        QSurfaceFormat surface_format = QSurfaceFormat::defaultFormat();
        surface_format.setDepthBufferSize ( 24 );
        surface_format.setSwapBehavior ( QSurfaceFormat::DoubleBuffer );
        QSurfaceFormat::setDefaultFormat ( surface_format );
    }

    MainWindow::~MainWindow() = default;

    void MainWindow::on_actionExit_triggered()
    {
        close();
    }

    void MainWindow::on_actionNew_triggered()
    {
        SceneWindow* sceneWindow;
        QMdiSubWindow*
        mdiSubWindow = mdiArea->addSubWindow ( sceneWindow = new SceneWindow ( mdiArea ) );
        mdiSubWindow->setAttribute ( Qt::WA_DeleteOnClose );
        mdiSubWindow->setWindowTitle ( tr ( "Untitled Scene" ) );
        mdiSubWindow->showMaximized();
        mdiSubWindow->setMinimumSize ( QSize ( 128, 128 ) );
    }

    void MainWindow::on_actionOpen_triggered()
    {
        QString filename = QFileDialog::getOpenFileName ( this,
                           tr ( "Open Scene" ),
                           tr ( "" ),
                           tr ( "Scene Files (*.scn *.txt)" ) );
        if ( ! ( filename.isEmpty() || filename.isNull() ) )
        {
            QFileInfo fileinfo ( filename );
            SceneWindow* sceneWindow;
            QMdiSubWindow*
            mdiSubWindow = mdiArea->addSubWindow ( sceneWindow = new SceneWindow ( mdiArea ) );
            mdiSubWindow->setAttribute ( Qt::WA_DeleteOnClose );
            mdiSubWindow->setWindowTitle ( fileinfo.absoluteFilePath() );
            mdiSubWindow->showMaximized();
            mdiSubWindow->setMinimumSize ( QSize ( 128, 128 ) );
            sceneWindow->Open ( fileinfo.absoluteFilePath().toStdString() );
        }
    }
    void MainWindow::on_actionSave_triggered()
    {
        QMdiSubWindow*
        mdiSubWindow = mdiArea->currentSubWindow ();
        if ( !mdiSubWindow )
        {
            return;
        }
        QString filename = QFileDialog::getSaveFileName ( this,
                           tr ( "Save Scene" ),
                           tr ( "" ),
                           tr ( "Scene Files (*.scn *.txt)" ) );
        if ( ! ( filename.isEmpty() || filename.isNull() ) )
        {
            QFileInfo fileinfo ( filename );
            SceneWindow* sceneWindow = reinterpret_cast<SceneWindow*> ( mdiSubWindow->widget() );
            if ( sceneWindow )
            {
                sceneWindow->Save ( filename.toStdString() );
            }
        }
    }
}
