/*
Copyright (C) 2018,2019,2022,2024 Rodrigo Jose Hernandez Cordoba

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
#include <iostream>
#include <array>
#include <tuple>
#include "WorldEditor.h"
#include "SceneWindow.h"
#include "EngineWindow.h"
#include "aeongames/Node.h"
#include "aeongames/ProtoBufClasses.h"
#include "aeongames/StringId.h"
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : PROTOBUF_WARNINGS )
#endif
#include <google/protobuf/message.h>
#ifdef _MSC_VER
#pragma warning( pop )
#endif

namespace AeonGames
{
    SceneWindow::SceneWindow ( QWidget *parent, Qt::WindowFlags f ) :
        QWidget ( parent, f )
    {
        mUi.setupUi ( this );
        mEngineWindow = new EngineWindow();
        QWidget* widget = QWidget::createWindowContainer ( mEngineWindow, mUi.splitter );
        QSizePolicy size_policy ( QSizePolicy::Ignored, QSizePolicy::Ignored );
        size_policy.setHorizontalStretch ( 1 );
        size_policy.setVerticalStretch ( 1 );
        widget->setSizePolicy ( size_policy );
        mUi.splitter->addWidget ( widget );
        mUi.sceneTreeView->setModel ( &mSceneModel );
        mUi.componentListView->setModel ( &mComponentListModel );
        mUi.componentPropertyTreeView->setModel ( &mComponentModel );
        mUi.componentPropertyTreeView->setItemDelegate ( &mPropertyDelegate );
        mEngineWindow->setScene ( &mSceneModel.GetScene() );

        EnumerateComponentConstructors ( [this] ( const StringId & aComponentConstructor )
        {
            QString text ( tr ( "Add " ) );
            text.append ( aComponentConstructor.GetString() );
            text.append ( tr ( " Data" ) );
            QAction *action = new QAction ( QIcon ( ":/icons/icon_add" ), text, this );
            mComponentAddActions.append ( action );
            action->setStatusTip ( tr ( "Adds a new component of the specified type to the selected node" ) );
            connect ( action, &QAction::triggered, this,
                      [this, aComponentConstructor]()
            {
                QModelIndex index = mUi.sceneTreeView->currentIndex();
                if ( index.isValid() )
                {
                    Node* node = reinterpret_cast<Node*> ( index.internalPointer() );
                    mComponentListModel.SetNode ( node );
                    mComponentModel.SetComponent ( node->AddComponent ( ConstructComponent ( aComponentConstructor ) ) );
                }
            } );
            return true;
        } );
    }

    SceneWindow::~SceneWindow() = default;

    void SceneWindow::on_actionRemoveNode_triggered()
    {
        QModelIndex index = mUi.sceneTreeView->currentIndex();
        mSceneModel.RemoveNode ( index.row(), index.parent() );
    }

    void SceneWindow::on_actionSetCameraNode_triggered()
    {
        QModelIndex index = mUi.sceneTreeView->currentIndex();
        mSceneModel.SetCameraNode ( index );
    }

    void SceneWindow::on_actionRemoveComponent_triggered()
    {
        QModelIndex index = mUi.componentListView->currentIndex();
        ///@todo implement removing data from node
        ( void ) index;
    }

    void SceneWindow::on_actionAddNode_triggered()
    {
        QModelIndex index = mUi.sceneTreeView->currentIndex();
        mSceneModel.InsertNode ( mSceneModel.rowCount ( index ), index, std::make_unique<Node>() );
        mUi.sceneTreeView->expand ( index );
    }

    void SceneWindow::on_sceneContextMenuRequested ( const QPoint& aPoint )
    {
        QList<QAction *> actions;
        QModelIndex index = mUi.sceneTreeView->indexAt ( aPoint );
        actions.append ( mUi.actionAddNode );
        if ( index.isValid() )
        {
            actions.append ( mUi.actionRemoveNode );
            actions.append ( mUi.actionSetCameraNode );
        }
        mUi.sceneTreeView->setCurrentIndex ( index );
        QMenu::exec ( actions, mUi.sceneTreeView->mapToGlobal ( aPoint ) );
    }

    void SceneWindow::on_componentContextMenuRequested ( const QPoint& aPoint )
    {
        if ( !mUi.sceneTreeView->currentIndex().isValid() )
        {
            return;
        }
        QList<QAction *> actions;
        actions.append ( mComponentAddActions );
        QModelIndex index = mUi.componentListView->indexAt ( aPoint );
        if ( index.isValid() )
        {
            actions.append ( mUi.actionRemoveComponent );
        }
        mUi.componentListView->setCurrentIndex ( index );
        if ( actions.size() )
        {
            QMenu::exec ( actions, mUi.componentListView->mapToGlobal ( aPoint ) );
        }
    }

    void SceneWindow::UpdateLocalTransformData ( const Node* aNode )
    {
        static std::array<std::tuple<QWidget*, bool>, 9> widgets
        {
            {
                {mUi.localScaleX, {}},
                {mUi.localScaleY, {}},
                {mUi.localScaleZ, {}},
                {mUi.localRotationPitch, {}},
                {mUi.localRotationRoll, {}},
                {mUi.localRotationYaw, {}},
                {mUi.localTranslationX, {}},
                {mUi.localTranslationY, {}},
                {mUi.localTranslationZ, {}}
            }
        };
        for ( auto& i : widgets )
        {
            std::get<1> ( i ) = std::get<0> ( i )->blockSignals ( true );
        }
        if ( aNode )
        {
            const Transform& local = aNode->GetLocalTransform();
            mUi.localScaleX->setValue ( local.GetScale() [0] );
            mUi.localScaleY->setValue ( local.GetScale() [1] );
            mUi.localScaleZ->setValue ( local.GetScale() [2] );
            mUi.localRotationPitch->setValue ( local.GetRotation().GetEuler() [0] );
            mUi.localRotationRoll->setValue ( local.GetRotation().GetEuler() [1] );
            mUi.localRotationYaw->setValue ( local.GetRotation().GetEuler() [2] );
            mUi.localTranslationX->setValue ( local.GetTranslation() [0] );
            mUi.localTranslationY->setValue ( local.GetTranslation() [1] );
            mUi.localTranslationZ->setValue ( local.GetTranslation() [2] );
        }
        else
        {
            mUi.localScaleX->setValue ( 1 );
            mUi.localScaleY->setValue ( 1 );
            mUi.localScaleZ->setValue ( 1 );
            mUi.localRotationPitch->setValue ( 0 );
            mUi.localRotationRoll->setValue ( 0 );
            mUi.localRotationYaw->setValue ( 0 );
            mUi.localTranslationX->setValue ( 0 );
            mUi.localTranslationY->setValue ( 0 );
            mUi.localTranslationZ->setValue ( 0 );
        }
        for ( auto& i : widgets )
        {
            std::get<0> ( i )->blockSignals ( std::get<1> ( i ) );
        }
    }

    void SceneWindow::UpdateGlobalTransformData ( const Node* aNode )
    {
        static std::array<std::tuple<QWidget*, bool>, 9> widgets
        {
            {
                {mUi.globalScaleX, {}},
                {mUi.globalScaleY, {}},
                {mUi.globalScaleZ, {}},
                {mUi.globalRotationPitch, {}},
                {mUi.globalRotationRoll, {}},
                {mUi.globalRotationYaw, {}},
                {mUi.globalTranslationX, {}},
                {mUi.globalTranslationY, {}},
                {mUi.globalTranslationZ, {}}
            }
        };
        for ( auto& i : widgets )
        {
            std::get<1> ( i ) = std::get<0> ( i )->blockSignals ( true );
        }
        if ( aNode )
        {
            const Transform& global = aNode->GetGlobalTransform();
            mUi.globalScaleX->setValue ( global.GetScale() [0] );
            mUi.globalScaleY->setValue ( global.GetScale() [1] );
            mUi.globalScaleZ->setValue ( global.GetScale() [2] );
            mUi.globalRotationPitch->setValue ( global.GetRotation().GetEuler() [0] );
            mUi.globalRotationRoll->setValue ( global.GetRotation().GetEuler() [1] );
            mUi.globalRotationYaw->setValue ( global.GetRotation().GetEuler() [2] );
            mUi.globalTranslationX->setValue ( global.GetTranslation() [0] );
            mUi.globalTranslationY->setValue ( global.GetTranslation() [1] );
            mUi.globalTranslationZ->setValue ( global.GetTranslation() [2] );
        }
        else
        {
            mUi.globalScaleX->setValue ( 1 );
            mUi.globalScaleY->setValue ( 1 );
            mUi.globalScaleZ->setValue ( 1 );
            mUi.globalRotationPitch->setValue ( 0 );
            mUi.globalRotationRoll->setValue ( 0 );
            mUi.globalRotationYaw->setValue ( 0 );
            mUi.globalTranslationX->setValue ( 0 );
            mUi.globalTranslationY->setValue ( 0 );
            mUi.globalTranslationZ->setValue ( 0 );
        }
        for ( auto& i : widgets )
        {
            std::get<0> ( i )->blockSignals ( std::get<1> ( i ) );
        }
    }

    void SceneWindow::on_localTransformChanged()
    {
        QModelIndex index = mUi.sceneTreeView->currentIndex();
        if ( index.isValid() )
        {
            if ( Node * node = reinterpret_cast<Node * > ( index.internalPointer() ) )
            {
                Transform local
                {
                    {
                        static_cast<float> ( mUi.localScaleX->value() ),
                        static_cast<float> ( mUi.localScaleY->value() ),
                        static_cast<float> ( mUi.localScaleZ->value() )
                    },
                    {
                        static_cast<float> ( mUi.localRotationPitch->value() ),
                        static_cast<float> ( mUi.localRotationRoll->value() ),
                        static_cast<float> ( mUi.localRotationYaw->value() )
                    },
                    {
                        static_cast<float> ( mUi.localTranslationX->value() ),
                        static_cast<float> ( mUi.localTranslationY->value() ),
                        static_cast<float> ( mUi.localTranslationZ->value() )
                    }
                };
                node->SetLocalTransform ( local );
                UpdateGlobalTransformData ( node );
            }
        }
    }

    void SceneWindow::on_globalTransformChanged()
    {
        QModelIndex index = mUi.sceneTreeView->currentIndex();
        if ( index.isValid() )
        {
            if ( Node * node = reinterpret_cast<Node * > ( index.internalPointer() ) )
            {
                Transform global
                {
                    {
                        static_cast<float> ( mUi.globalScaleX->value() ),
                        static_cast<float> ( mUi.globalScaleY->value() ),
                        static_cast<float> ( mUi.globalScaleZ->value() )
                    },
                    {
                        static_cast<float> ( mUi.globalRotationPitch->value() ),
                        static_cast<float> ( mUi.globalRotationRoll->value() ),
                        static_cast<float> ( mUi.globalRotationYaw->value() )
                    },
                    {
                        static_cast<float> ( mUi.globalTranslationX->value() ),
                        static_cast<float> ( mUi.globalTranslationY->value() ),
                        static_cast<float> ( mUi.globalTranslationZ->value() )
                    }
                };
                node->SetGlobalTransform ( global );
                UpdateLocalTransformData ( node );
            }
        }
    }

    void SceneWindow::on_sceneTreeViewClicked ( const QModelIndex& aModelIndex )
    {
        if ( aModelIndex.isValid() )
        {
            if ( Node * node = reinterpret_cast<Node * > ( aModelIndex.internalPointer() ) )
            {
                mComponentListModel.SetNode ( node );
                mComponentModel.SetComponent ( nullptr );
                UpdateLocalTransformData ( node );
                UpdateGlobalTransformData ( node );
                return;
            }
        }
        mComponentListModel.SetNode ( nullptr );
        mComponentModel.SetComponent ( nullptr );
        UpdateLocalTransformData ( nullptr );
        UpdateGlobalTransformData ( nullptr );
    }

    void SceneWindow::on_componentListViewClicked ( const QModelIndex& aModelIndex )
    {
        if ( aModelIndex.isValid() && mComponentListModel.GetNode() )
        {
            mComponentModel.SetComponent ( mComponentListModel.GetNode()->GetComponentByIndex ( aModelIndex.row() ) );
        }
        else
        {
            mComponentModel.SetComponent ( nullptr );
        }
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
    void SceneWindow::SetFieldOfView ( float aFieldOfView )
    {
        mEngineWindow->SetFieldOfView ( aFieldOfView );
    }
    void SceneWindow::SetNear ( float aNear )
    {
        mEngineWindow->SetNear ( aNear );
    }
    void SceneWindow::SetFar ( float aFar )
    {
        mEngineWindow->SetFar ( aFar );
    }
}
