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
#include <QFileDialog>
#include <QMdiSubWindow>
#include <QSurfaceFormat>
#include <QMenu>
#include "WorldEditor.h"
#include "SceneWindow.h"
#include "EngineWindow.h"

namespace AeonGames
{
    SceneWindow::SceneWindow ( QWidget *parent, Qt::WindowFlags f ) :
        QWidget ( parent, f ),
        Ui::SceneWindow()
    {
        setupUi ( this );
        mEngineWindow = new EngineWindow();
        QWidget* widget = QWidget::createWindowContainer ( mEngineWindow, splitter );
        QSizePolicy size_policy ( QSizePolicy::Expanding, QSizePolicy::Expanding );
        size_policy.setHorizontalStretch ( 6 );
        size_policy.setVerticalStretch ( 6 );
        widget->setSizePolicy ( size_policy );
        splitter->addWidget ( widget );
        treeView->setModel ( &mSceneModel );
        treeView->addAction ( actionAddNode );
        treeView->addAction ( actionRemoveNode );
    }

    SceneWindow::~SceneWindow()
        = default;

    void SceneWindow::setModel ( const QString& filename )
    {
        mEngineWindow->setModel ( filename );
    }

    void SceneWindow::on_actionAddNode_triggered()
    {
        QModelIndex index = treeView->currentIndex();
        mSceneModel.InsertNode ( mSceneModel.rowCount ( index ), index );
        treeView->expand ( index );
    }

    void SceneWindow::on_actionRemoveNode_triggered()
    {
        QModelIndex index = treeView->currentIndex();
        mSceneModel.RemoveNode ( index.row(), index.parent() );
    }

    void SceneWindow::on_customContextMenuRequested ( const QPoint& aPoint )
    {
        QList<QAction *> actions;
        QModelIndex index = treeView->indexAt ( aPoint );
        actions.append ( actionAddNode );
        if ( index.isValid() )
        {
            actions.append ( actionRemoveNode );
        }
        treeView->setCurrentIndex ( index );
        QMenu::exec ( actions, treeView->mapToGlobal ( aPoint ) );
    }
}
