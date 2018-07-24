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
#include <fstream>
#include "WorldEditor.h"
#include "SceneWindow.h"
#include "EngineWindow.h"
#include "aeongames/Node.h"

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
        sceneTreeView->setModel ( &mSceneModel );
        propertiesTreeView->setModel ( &mNodeModel );
        mEngineWindow->setScene ( &mSceneModel.GetScene() );
        EnumerateNodeConstructors ( [this] ( const std::string & aNodeConstructor )
        {
            QString text ( tr ( "Add " ) );
            text.append ( aNodeConstructor.c_str() );
            QAction *action = new QAction ( QIcon ( ":/icons/icon_node" ), text, this );
            mNodeAddActions.append ( action );
            action->setStatusTip ( tr ( "Adds a new node of the specified type" ) );
            connect ( action, &QAction::triggered, this,
                      [this, aNodeConstructor]()
            {
                QModelIndex index = sceneTreeView->currentIndex();
                mSceneModel.InsertNode ( mSceneModel.rowCount ( index ), index, ConstructNode ( aNodeConstructor ) );
                sceneTreeView->expand ( index );
            } );
            return true;
        } );
    }

    SceneWindow::~SceneWindow()
        = default;

    void SceneWindow::on_actionRemoveNode_triggered()
    {
        QModelIndex index = sceneTreeView->currentIndex();
        mSceneModel.RemoveNode ( index.row(), index.parent() );
    }

    void SceneWindow::on_customContextMenuRequested ( const QPoint& aPoint )
    {
        QList<QAction *> actions;
        QModelIndex index = sceneTreeView->indexAt ( aPoint );
        actions.append ( mNodeAddActions );
        if ( index.isValid() )
        {
            actions.append ( actionRemoveNode );
        }
        sceneTreeView->setCurrentIndex ( index );
        QMenu::exec ( actions, sceneTreeView->mapToGlobal ( aPoint ) );
    }
    void SceneWindow::on_sceneTreeViewClicked ( const QModelIndex& aModelIndex )
    {
        if ( aModelIndex.isValid() )
        {
            if ( Node* node = reinterpret_cast<Node*> ( aModelIndex.internalPointer() ) )
            {
                mNodeModel.SetNode ( node );
                return;
            }
        }
        mNodeModel.SetNode ( nullptr );
    }
    void SceneWindow::Open ( const std::string& mFilename )
    {
        std::ifstream file;
        file.exceptions ( std::ifstream::failbit | std::ifstream::badbit );
        file.open ( mFilename, std::ifstream::in | std::ifstream::binary );
        std::string buffer ( ( std::istreambuf_iterator<char> ( file ) ), std::istreambuf_iterator<char>() );
        file.close();
        mSceneModel.Deserialize ( buffer );
    }
    void SceneWindow::Save ( const std::string& mFilename ) const
    {
        std::string scene = mSceneModel.Serialize ( false );
        std::ofstream scene_file ( mFilename, std::ifstream::out );
        scene_file.write ( scene.data(), scene.size() );
        scene_file.close();
    }
}
