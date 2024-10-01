/*
Copyright (C) 2016-2022,2024 Rodrigo Jose Hernandez Cordoba

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
#include <QScrollArea>
#include <QColorSpace>
#include <iostream>
#include "aeongames/Renderer.h"
#include "aeongames/LogLevel.h"
#include "WorldEditor.h"
#include "MainWindow.h"
#include "SceneWindow.h"
#include "CameraSettings.h"
#include "NodeView.h"

namespace AeonGames
{
    MainWindow::MainWindow ( QWidget* parent, Qt::WindowFlags flags ) : QMainWindow{parent, flags}
    {
        mUi.setupUi ( this );

        QSurfaceFormat surface_format = QSurfaceFormat::defaultFormat();

        surface_format.setColorSpace ( QColorSpace::SRgb );
        surface_format.setRenderableType ( QSurfaceFormat::OpenGL );
        surface_format.setSwapBehavior ( QSurfaceFormat::DoubleBuffer );
        surface_format.setRedBufferSize ( 8 );
        surface_format.setGreenBufferSize ( 8 );
        surface_format.setBlueBufferSize ( 8 );
        surface_format.setAlphaBufferSize ( 8 );
        surface_format.setStencilBufferSize ( 8 );
        surface_format.setDepthBufferSize ( 24 );
        //surface_format.setSamples ( ? ); ///@< Find out what is a sensible value for multisampling
        QSurfaceFormat::setDefaultFormat ( surface_format );
    }

    MainWindow::~MainWindow() = default;

    void MainWindow::on_actionExit_triggered()
    {
        close();
    }

    void MainWindow::on_actionNewScene_triggered()
    {
        SceneWindow* sceneWindow;
        QMdiSubWindow*
        mdiSubWindow = mUi.mdiArea->addSubWindow ( sceneWindow = new SceneWindow ( mUi.mdiArea ) );
        mdiSubWindow->setAttribute ( Qt::WA_DeleteOnClose );
        mdiSubWindow->setWindowTitle ( tr ( "Untitled Scene" ) );
        mdiSubWindow->showMaximized();
        mdiSubWindow->setMinimumSize ( QSize ( 128, 128 ) );
        mUi.actionSave->setEnabled ( true );
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
            mdiSubWindow = mUi.mdiArea->addSubWindow ( sceneWindow = new SceneWindow ( mUi.mdiArea ) );
            mdiSubWindow->setAttribute ( Qt::WA_DeleteOnClose );
            mdiSubWindow->setWindowTitle ( fileinfo.absoluteFilePath() );
            mdiSubWindow->showMaximized();
            mdiSubWindow->setMinimumSize ( QSize ( 128, 128 ) );
            sceneWindow->Open ( fileinfo.absoluteFilePath().toStdString() );
            /** @todo handle open failure. */
            mUi.actionSave->setEnabled ( true );
        }
    }

    void MainWindow::on_actionSave_triggered()
    {
        QMdiSubWindow*
        mdiSubWindow = mUi.mdiArea->currentSubWindow ();
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
        if ( mCameraSettings == nullptr )
        {
            mCameraSettings = new CameraSettings{this};
            connect ( mCameraSettings, SIGNAL ( fieldOfViewChanged ( double ) ), this, SLOT ( fieldOfViewChanged ( double ) ) );
            connect ( mCameraSettings, SIGNAL ( nearChanged ( double ) ), this, SLOT ( nearChanged ( double ) ) );
            connect ( mCameraSettings, SIGNAL ( farChanged ( double ) ), this, SLOT ( farChanged ( double ) ) );
        }
        mCameraSettings->exec();
    }

    void MainWindow::on_actionNewShader_triggered()
    {
        QScrollArea* scrollArea{new QScrollArea{mUi.mdiArea}};
        NodeView* nodeView{new NodeView{scrollArea}};
        scrollArea->setWidget ( nodeView );
        scrollArea->setWidgetResizable ( true );
        QMdiSubWindow* mdiSubWindow{ mUi.mdiArea->addSubWindow ( scrollArea ) };
        mdiSubWindow->setAttribute ( Qt::WA_DeleteOnClose );
        mdiSubWindow->setWindowTitle ( tr ( "Untitled Shader" ) );
        mdiSubWindow->showMaximized();
        mdiSubWindow->setMinimumSize ( QSize ( 128, 128 ) );
        mUi.actionSave->setEnabled ( true );
    }

    void MainWindow::fieldOfViewChanged ( double aFieldOfView )
    {
        QMdiSubWindow*
        mdiSubWindow = mUi.mdiArea->currentSubWindow ();
        if ( !mdiSubWindow )
        {
            return;
        }
        reinterpret_cast<SceneWindow * > ( mdiSubWindow->widget() )->SetFieldOfView ( static_cast<float> ( aFieldOfView ) );
    }

    void MainWindow::nearChanged ( double aNear )
    {
        QMdiSubWindow*
        mdiSubWindow = mUi.mdiArea->currentSubWindow ();
        if ( !mdiSubWindow )
        {
            return;
        }
        reinterpret_cast<SceneWindow * > ( mdiSubWindow->widget() )->SetNear ( static_cast<float> ( aNear ) );
    }

    void MainWindow::farChanged ( double aFar )
    {
        QMdiSubWindow*
        mdiSubWindow = mUi.mdiArea->currentSubWindow ();
        if ( !mdiSubWindow )
        {
            return;
        }
        reinterpret_cast<SceneWindow * > ( mdiSubWindow->widget() )->SetFar ( static_cast<float> ( aFar ) );
    }
}
