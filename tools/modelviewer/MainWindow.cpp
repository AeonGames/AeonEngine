/*
Copyright (C) 2016,2017 Rodrigo Jose Hernandez Cordoba

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
#include "RendererSelectDialog.h"
#include "MainWindow.h"
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

        /* Add a nice renderer selection window.*/
        QStringList renderer_list;
        EnumerateRendererLoaders ( [this, &renderer_list] ( const std::string & aIdentifier )->bool
        {
            renderer_list.append ( QString::fromStdString ( aIdentifier ) );
            return true;
        } );

        if ( !renderer_list.size() )
        {
            throw std::runtime_error ( "No renderer available, cannot continue." );
        }
        if ( renderer_list.size() == 1 )
        {
            this->mRenderer = GetRenderer ( renderer_list.at ( 0 ).toStdString() );
        }
        else
        {
            RendererSelectDialog select_renderer;
            select_renderer.SetRenderers ( renderer_list );
            if ( select_renderer.exec() == QDialog::Accepted )
            {
                this->mRenderer = GetRenderer ( select_renderer.GetSelected().toStdString() );
            }
        }

        if ( mRenderer == nullptr )
        {
            throw std::runtime_error ( "No renderer selected, cannot continue." );
        }
    }

    MainWindow::~MainWindow()
        = default;

    void MainWindow::on_actionExit_triggered()
    {
        close();
    }

    void MainWindow::on_actionNew_triggered()
    {
    }

    void MainWindow::on_actionOpen_triggered()
    {
        QString filename = QFileDialog::getOpenFileName ( this,
                           tr ( "Open Model" ),
                           tr ( "" ),
                           tr ( "Model Files (*.mdl *.txt)" ) );
        if ( ! ( filename.isEmpty() || filename.isNull() ) )
        {
            QFileInfo fileinfo ( filename );
            EngineWindow* window = new EngineWindow ( mRenderer );
            QMdiSubWindow*
            mdiSubWindow = mdiArea->addSubWindow ( QWidget::createWindowContainer ( window, mdiArea ) );
            mdiSubWindow->setAttribute ( Qt::WA_DeleteOnClose );
            mdiSubWindow->setWindowTitle ( fileinfo.absoluteFilePath() );
            mdiSubWindow->showMaximized();
            mdiSubWindow->setMinimumSize ( QSize ( 128, 128 ) );
            window->setMesh ( fileinfo.absoluteFilePath() );
        }
    }
}
