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
//#include <QtGui>
//#include <QApplication>
#include <QFileDialog>
//#include <QFileInfo>
//#include <QDesktopWidget>
//#include <QDir>
//#include <QMessageBox>
//#include <QMdiSubWindow>
#include "MainWindow.h"
#include "EngineWindow.h"

namespace AeonGames
{
    MainWindow::MainWindow() : QMainWindow(), Ui::MainWindow()
    {
        setupUi ( this );
        engineWindow = new EngineWindow();
        engineWindowContainer = QWidget::createWindowContainer ( engineWindow );
        engineWindowContainer->setParent ( centralwidget );
        verticalLayout->addWidget ( engineWindowContainer );
    }

    MainWindow::~MainWindow()
    {
    }

    void MainWindow::on_actionExit_triggered()
    {
        close();
    }

    void MainWindow::on_actionOpen_triggered()
    {
        QString filename = QFileDialog::getOpenFileName ( this, tr ( "Open Mesh" ), tr ( "" ), tr ( "Mesh Files (*.msh *.txt)" ) );
        if ( ! ( filename.isEmpty() || filename.isNull() ) )
        {
            engineWindow->setMesh ( filename );
        }
    }
}
