/*
Copyright (C) 2016-2019 Rodrigo Jose Hernandez Cordoba

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
#include "CameraSettings.h"

namespace AeonGames
{
    MainWindow::MainWindow() : QMainWindow(), Ui::MainWindow()
    {
        setupUi ( this );
        QSurfaceFormat surface_format = QSurfaceFormat::defaultFormat();
        surface_format.setDepthBufferSize ( 24 );
        surface_format.setSwapBehavior ( QSurfaceFormat::DoubleBuffer );
        QSurfaceFormat::setDefaultFormat ( surface_format );
        mCameraSettings = new CameraSettings ( this );
        connect ( mCameraSettings, SIGNAL ( fieldOfViewChanged ( double ) ), this, SLOT ( fieldOfViewChanged ( double ) ) );
        connect ( mCameraSettings, SIGNAL ( nearChanged ( double ) ), this, SLOT ( nearChanged ( double ) ) );
        connect ( mCameraSettings, SIGNAL ( farChanged ( double ) ), this, SLOT ( farChanged ( double ) ) );
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
        actionSave->setEnabled ( true );
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
            /** @todo handle open failure. */
            actionSave->setEnabled ( true );
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
            SceneWindow* sceneWindow = reinterpret_cast<SceneWindow*> ( mdiSubWindow->widget() );
            if ( sceneWindow )
            {
                sceneWindow->Save ( filename.toStdString() );
            }
        }
    }

    void MainWindow::on_actionCamera_triggered()
    {
        mCameraSettings->show();
    }

    void MainWindow::fieldOfViewChanged ( double aFieldOfView )
    {
        QMdiSubWindow*
        mdiSubWindow = mdiArea->currentSubWindow ();
        if ( !mdiSubWindow )
        {
            return;
        }
        reinterpret_cast<SceneWindow*> ( mdiSubWindow->widget() )->SetFieldOfView ( static_cast<float> ( aFieldOfView ) );
    }
    void MainWindow::nearChanged ( double aNear )
    {
        QMdiSubWindow*
        mdiSubWindow = mdiArea->currentSubWindow ();
        if ( !mdiSubWindow )
        {
            return;
        }
        reinterpret_cast<SceneWindow*> ( mdiSubWindow->widget() )->SetNear ( static_cast<float> ( aNear ) );
    }
    void MainWindow::farChanged ( double aFar )
    {
        QMdiSubWindow*
        mdiSubWindow = mdiArea->currentSubWindow ();
        if ( !mdiSubWindow )
        {
            return;
        }
        reinterpret_cast<SceneWindow*> ( mdiSubWindow->widget() )->SetFar ( static_cast<float> ( aFar ) );
    }
}
