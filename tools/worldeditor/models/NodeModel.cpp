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

#include <QIcon>
#include <QFile>
#include <QMimeData>
#include <QDataStream>
#include <QByteArray>
#include <QXmlStreamWriter>
#include <QTextStream>
#include "NodeModel.h"
#include "aeongames/Scene.h"
#include "aeongames/Node.h"

namespace AeonGames
{
    NodeModel::NodeModel ( QObject *parent ) :
        QAbstractListModel ( parent ) {}

    NodeModel::~NodeModel() = default;

    QVariant NodeModel::headerData ( int section, Qt::Orientation orientation, int role ) const
    {
        if ( ( orientation == Qt::Horizontal ) && ( role == Qt::DisplayRole ) )
        {
            switch ( section )
            {
            case 0:
                return QString ( "Component" );
            default:
                return QVariant();
            }
        }
        return QVariant();
    }

    int NodeModel::rowCount ( const QModelIndex & index ) const
    {
        if ( !mNode || index.isValid() )
        {
            return 0;
        }
        return static_cast<int> ( mNode->GetComponents().Size() );
    }

    QVariant NodeModel::data ( const QModelIndex & index, int role ) const
    {
        if ( !mNode || !index.isValid() )
        {
            return QVariant();
        }
        if ( role == Qt::DisplayRole )
        {
            ///@todo consider changing GetTypeName() to return const(expr) char*
            return QString ( mNode->GetComponents() [index.row()]->GetTypeName().c_str() );
        }
        return QVariant();
    }
    void NodeModel::SetNode ( Node* aNode )
    {
        beginResetModel();
        mNode = aNode;
        endResetModel();
    }
    const Node* NodeModel::GetNode () const
    {
        return mNode;
    }
}
