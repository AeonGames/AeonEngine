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
#include <iostream>
#include "WorldEditor.h"
#include "SceneWindow.h"
#include "EngineWindow.h"
#include "MessageWrapper.h"
#include "aeongames/Node.h"
#include "aeongames/ProtoBufClasses.h"
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4251 )
#endif
#include <google/protobuf/message.h>
#ifdef _MSC_VER
#pragma warning( pop )
#endif

namespace AeonGames
{
    SceneWindow::SceneWindow ( QWidget *parent, Qt::WindowFlags f ) :
        QWidget ( parent, f ),
        Ui::SceneWindow()
    {
        setupUi ( this );
        addNodeToolButton->setDefaultAction ( actionAddNode );
        removeNodeToolButton->setDefaultAction ( actionRemoveNode );
        removeComponentToolButton->setDefaultAction ( actionRemoveComponent );
        mEngineWindow = new EngineWindow();
        QWidget* widget = QWidget::createWindowContainer ( mEngineWindow, splitter );
        QSizePolicy size_policy ( QSizePolicy::Expanding, QSizePolicy::Expanding );
        size_policy.setHorizontalStretch ( 1 );
        size_policy.setVerticalStretch ( 1 );
        widget->setSizePolicy ( size_policy );
        splitter->addWidget ( widget );
        sceneTreeView->setModel ( &mSceneModel );
        nodeListView->setModel ( &mNodeModel );
        componentTreeView->setModel ( &mMessageModel );
        mEngineWindow->setScene ( &mSceneModel.GetScene() );
        EnumerateComponentConstructors ( [this] ( const std::string & aComponentConstructor )
        {
            QString text ( tr ( "Add " ) );
            text.append ( aComponentConstructor.c_str() );
            text.append ( tr ( " Component" ) );
            QAction *action = new QAction ( QIcon ( ":/icons/icon_add" ), text, this );
            mComponentAddActions.append ( action );
            action->setStatusTip ( tr ( "Adds a new component of the specified type to the selected node" ) );
            connect ( action, &QAction::triggered, this,
                      [this, aComponentConstructor]()
            {
                QModelIndex index = sceneTreeView->currentIndex();
                if ( index.isValid() )
                {
                    Node* node = reinterpret_cast<Node*> ( index.internalPointer() );
                    mNodeModel.SetNode ( node );
                    mMessageModel.SetMessage ( node->GetComponents() [node->AddComponent ( *node->StoreComponent ( ConstructComponent ( aComponentConstructor ) ) )]->GetProperties() );
                }
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

    void SceneWindow::on_actionRemoveComponent_triggered()
    {
        QModelIndex index = nodeListView->currentIndex();
        ///@todo implement removing component from node
        ( void ) index;
    }

    void SceneWindow::on_actionAddNode_triggered()
    {
        QModelIndex index = sceneTreeView->currentIndex();
        mSceneModel.InsertNode ( mSceneModel.rowCount ( index ), index, std::make_unique<Node>() );
        sceneTreeView->expand ( index );
    }

    void SceneWindow::on_sceneContextMenuRequested ( const QPoint& aPoint )
    {
        QList<QAction *> actions;
        QModelIndex index = sceneTreeView->indexAt ( aPoint );
        actions.append ( actionAddNode );
        if ( index.isValid() )
        {
            actions.append ( actionRemoveNode );
        }
        sceneTreeView->setCurrentIndex ( index );
        QMenu::exec ( actions, sceneTreeView->mapToGlobal ( aPoint ) );
    }

    void SceneWindow::on_nodeContextMenuRequested ( const QPoint& aPoint )
    {
        QList<QAction *> actions;
        QModelIndex index = nodeListView->indexAt ( aPoint );
        actions.append ( mComponentAddActions );
        if ( index.isValid() )
        {
            actions.append ( actionRemoveComponent );
        }
        nodeListView->setCurrentIndex ( index );
        QMenu::exec ( actions, nodeListView->mapToGlobal ( aPoint ) );
    }

    void SceneWindow::on_componentContextMenuRequested ( const QPoint& aPoint )
    {
        QList<QAction *> actions;
        QModelIndex index = componentTreeView->indexAt ( aPoint );
        if ( index.isValid() && reinterpret_cast<const MessageWrapper::Field*> ( index.internalPointer() )->GetFieldDescriptor()->type() == google::protobuf::FieldDescriptor::TYPE_MESSAGE )
        {
            /**@todo Actions to add fields to sub-fields */
        }
        else if ( !index.isValid() && mMessageModel.GetMessageWrapper().GetMessagePtr() )
        {
            google::protobuf::Message* message = mMessageModel.GetMessageWrapper().GetMessagePtr();
            const google::protobuf::Reflection* reflection = message->GetReflection();
            for ( int i = 0; i < message->GetDescriptor()->field_count(); ++i )
            {
                const google::protobuf::FieldDescriptor* field = message->GetDescriptor()->field ( i );
                if ( !field->is_repeated() && !reflection->HasField ( *message, field ) )
                {
                    QString text ( tr ( "Add " ) );
                    text.append ( field->name().c_str() );
                    text.append ( tr ( " field" ) );
                    ///@todo This is a memory leak, fix it
                    QAction *action = new QAction ( QIcon ( ":/icons/icon_add" ), text, this );
                    actions.append ( action );
                    connect ( action, &QAction::triggered, this,
                              [this, message, reflection, field]()
                    {
                        if ( !message || ! reflection || !field )
                        {
                            return;
                        }
                        switch ( field->cpp_type() )
                        {
                        case google::protobuf::FieldDescriptor::CPPTYPE_DOUBLE:
                            reflection->SetDouble ( message, field, field->default_value_double() );
                            break;
                        case google::protobuf::FieldDescriptor::CPPTYPE_FLOAT:
                            reflection->SetFloat ( message, field, field->default_value_float() );
                            break;
                        case google::protobuf::FieldDescriptor::CPPTYPE_INT64:
                            reflection->SetInt64 ( message, field, field->default_value_int64() );
                            break;
                        case google::protobuf::FieldDescriptor::CPPTYPE_UINT64:
                            reflection->SetUInt64 ( message, field, field->default_value_uint64() );
                            break;
                        case google::protobuf::FieldDescriptor::CPPTYPE_INT32:
                            reflection->SetInt32 ( message, field, field->default_value_int32() );
                            break;
                        case google::protobuf::FieldDescriptor::CPPTYPE_BOOL:
                            reflection->SetBool ( message, field, field->default_value_bool() );
                            break;
                        case google::protobuf::FieldDescriptor::CPPTYPE_STRING:
                            reflection->SetString ( message, field, field->default_value_string() );
                            break;
                        case google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE:
                            reflection->MutableMessage ( message, field );
                            break;
                        case google::protobuf::FieldDescriptor::CPPTYPE_UINT32:
                            reflection->SetUInt32 ( message, field, field->default_value_uint32() );
                            break;
                        case google::protobuf::FieldDescriptor::CPPTYPE_ENUM:
                            reflection->SetEnum ( message, field, field->default_value_enum() );
                            break;
                        }
                    } );
                }
            }
        }
        componentTreeView->setCurrentIndex ( index );
        QMenu::exec ( actions, componentTreeView->mapToGlobal ( aPoint ) );
    }

    void SceneWindow::UpdateLocalTransformData ( const Node* aNode )
    {
        ///@todo Calling blockSignals on the parent doesn't block children
        bool signals_blocked = blockSignals ( true );
        if ( aNode )
        {
            const Transform& local = aNode->GetLocalTransform();
            localScaleX->setValue ( local.GetScale() [0] );
            localScaleY->setValue ( local.GetScale() [1] );
            localScaleZ->setValue ( local.GetScale() [2] );
            localRotationPitch->setValue ( local.GetRotation().GetEuler() [0] );
            localRotationRoll->setValue ( local.GetRotation().GetEuler() [1] );
            localRotationYaw->setValue ( local.GetRotation().GetEuler() [2] );
            localTranslationX->setValue ( local.GetTranslation() [0] );
            localTranslationY->setValue ( local.GetTranslation() [1] );
            localTranslationZ->setValue ( local.GetTranslation() [2] );
        }
        else
        {
            localScaleX->setValue ( 1 );
            localScaleY->setValue ( 1 );
            localScaleZ->setValue ( 1 );
            localRotationPitch->setValue ( 0 );
            localRotationRoll->setValue ( 0 );
            localRotationYaw->setValue ( 0 );
            localTranslationX->setValue ( 0 );
            localTranslationY->setValue ( 0 );
            localTranslationZ->setValue ( 0 );
        }
        blockSignals ( signals_blocked );
    }

    void SceneWindow::UpdateGlobalTransformData ( const Node* aNode )
    {
        ///@todo Calling blockSignals on the parent doesn't block children
        bool signals_blocked = blockSignals ( true );
        if ( aNode )
        {
            const Transform& global = aNode->GetGlobalTransform();
            globalScaleX->setValue ( global.GetScale() [0] );
            globalScaleY->setValue ( global.GetScale() [1] );
            globalScaleZ->setValue ( global.GetScale() [2] );
            globalRotationPitch->setValue ( global.GetRotation().GetEuler() [0] );
            globalRotationRoll->setValue ( global.GetRotation().GetEuler() [1] );
            globalRotationYaw->setValue ( global.GetRotation().GetEuler() [2] );
            globalTranslationX->setValue ( global.GetTranslation() [0] );
            globalTranslationY->setValue ( global.GetTranslation() [1] );
            globalTranslationZ->setValue ( global.GetTranslation() [2] );
        }
        else
        {
            globalScaleX->setValue ( 1 );
            globalScaleY->setValue ( 1 );
            globalScaleZ->setValue ( 1 );
            globalRotationPitch->setValue ( 0 );
            globalRotationRoll->setValue ( 0 );
            globalRotationYaw->setValue ( 0 );
            globalTranslationX->setValue ( 0 );
            globalTranslationY->setValue ( 0 );
            globalTranslationZ->setValue ( 0 );
        }
        blockSignals ( signals_blocked );
    }

    void SceneWindow::on_localTransformChanged()
    {
        std::cout << __func__ << std::endl;
        QModelIndex index = sceneTreeView->currentIndex();
        if ( index.isValid() )
        {
            if ( Node* node = reinterpret_cast<Node*> ( index.internalPointer() ) )
            {
                Transform local
                {
                    {
                        static_cast<float> ( localScaleX->value() ),
                        static_cast<float> ( localScaleY->value() ),
                        static_cast<float> ( localScaleZ->value() )
                    },
                    {
                        static_cast<float> ( localRotationPitch->value() ),
                        static_cast<float> ( localRotationRoll->value() ),
                        static_cast<float> ( localRotationYaw->value() )
                    },
                    {
                        static_cast<float> ( localTranslationX->value() ),
                        static_cast<float> ( localTranslationY->value() ),
                        static_cast<float> ( localTranslationZ->value() )
                    }
                };
                node->SetLocalTransform ( local );
                UpdateGlobalTransformData ( node );
            }
        }
    }

    void SceneWindow::on_globalTransformChanged()
    {
        std::cout << __func__ << std::endl;
        QModelIndex index = sceneTreeView->currentIndex();
        if ( index.isValid() )
        {
            if ( Node* node = reinterpret_cast<Node*> ( index.internalPointer() ) )
            {
                Transform global
                {
                    {
                        static_cast<float> ( globalScaleX->value() ),
                        static_cast<float> ( globalScaleY->value() ),
                        static_cast<float> ( globalScaleZ->value() )
                    },
                    {
                        static_cast<float> ( globalRotationPitch->value() ),
                        static_cast<float> ( globalRotationRoll->value() ),
                        static_cast<float> ( globalRotationYaw->value() )
                    },
                    {
                        static_cast<float> ( globalTranslationX->value() ),
                        static_cast<float> ( globalTranslationY->value() ),
                        static_cast<float> ( globalTranslationZ->value() )
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
            if ( Node* node = reinterpret_cast<Node*> ( aModelIndex.internalPointer() ) )
            {
                mNodeModel.SetNode ( node );
                mMessageModel.SetMessage ( nullptr );
                UpdateLocalTransformData ( node );
                UpdateGlobalTransformData ( node );
                return;
            }
        }
        mNodeModel.SetNode ( nullptr );
        mMessageModel.SetMessage ( nullptr );
        UpdateLocalTransformData ( nullptr );
        UpdateGlobalTransformData ( nullptr );
    }

    void SceneWindow::on_nodeListViewClicked ( const QModelIndex& aModelIndex )
    {
        if ( aModelIndex.isValid() && mNodeModel.GetNode() )
        {
            mMessageModel.SetMessage ( mNodeModel.GetNode()->GetComponents() [aModelIndex.row()]->GetProperties() );
            return;
        }
        mMessageModel.SetMessage ( nullptr );
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
