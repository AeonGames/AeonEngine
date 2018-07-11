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
        QAbstractItemModel ( parent ) {}

    NodeModel::~NodeModel() = default;

    QVariant NodeModel::headerData ( int section, Qt::Orientation orientation, int role ) const
    {
        if ( ( orientation == Qt::Horizontal ) && ( role == Qt::DisplayRole ) )
        {
            switch ( section )
            {
            case 0:
                return QString ( "Property" );
            case 1:
                return QString ( "Value" );
            default:
                return QVariant();
            }
        }
        return QVariant();
    }

    QModelIndex NodeModel::index ( int row, int column, const QModelIndex & parent ) const
    {
        if ( !mNode )
        {
            return QModelIndex();
        }
        if ( !parent.isValid() )
        {
            if ( ( row >= 0 ) && ( row < static_cast<int> ( mNodeProperties->size() ) ) )
            {
                return createIndex ( row, column, &mNodeProperties->at ( row ).get() );
            }
        }
        else
        {
            const Node::Property* parent_property = reinterpret_cast<Node::Property*> ( parent.internalPointer() );
            if ( ( row >= 0 ) && ( row < static_cast<int> ( parent_property->GetSubProperties().size() ) ) )
            {
                const Node::Property* index_property = &parent_property->GetSubProperties() [row];
                return createIndex ( row, column, const_cast<Node::Property*> ( index_property ) );
            }
        }
        return QModelIndex();
    }

    QModelIndex NodeModel::parent ( const QModelIndex & index ) const
    {
        const Node::Property* index_property{};
        const Node::Property* parent_property{};
        if ( index.isValid() )
        {
            index_property = reinterpret_cast<Node::Property*> ( index.internalPointer() );
            parent_property = index_property->GetParent();
            if ( parent_property )
            {
                return createIndex ( static_cast<int> ( index_property->GetIndex() ), 0, const_cast<Node::Property*> ( parent_property ) );
            }
        }
        return QModelIndex();
    }

    int NodeModel::rowCount ( const QModelIndex & index ) const
    {
        if ( !mNode )
        {
            return 0;
        }
        if ( index.isValid() )
        {
            return static_cast<int> ( reinterpret_cast<Node::Property*> ( index.internalPointer() )->GetSubProperties().size() );
        }
        return static_cast<int> ( mNodeProperties->size() );
    }

    int NodeModel::columnCount ( const QModelIndex & index ) const
    {
        return 2;
    }

    bool NodeModel::hasChildren ( const QModelIndex & index ) const
    {
        if ( !mNode )
        {
            return false;
        }
        if ( index.isValid() )
        {
            return reinterpret_cast<Node::Property*> ( index.internalPointer() )->GetSubProperties().size();
        }
        return ( mNodeProperties->size() );
    }

    QVariant NodeModel::data ( const QModelIndex & index, int role ) const
    {
        if ( !mNode )
        {
            return QVariant();
        }
        if ( role == Qt::DisplayRole )
        {
            if ( index.isValid() )
            {
                switch ( index.column() )
                {
                case 0:
                {
                    const Node::Property* index_property = reinterpret_cast<Node::Property*> ( index.internalPointer() );
                    return QString ( index_property->GetDisplayName().c_str() );
                }
                break;
                case 1:
                    const Node::Property* index_property = reinterpret_cast<Node::Property*> ( index.internalPointer() );
                    return QString ( index_property->GetAsString ( mNode ).c_str() );
                }
            }
        }
        else if ( role == Qt::EditRole )
        {
            if ( index.isValid() && ( index.column() == 1 ) )
            {
                const Node::Property* index_property = reinterpret_cast<Node::Property*> ( index.internalPointer() );
                return QString ( index_property->GetAsString ( mNode ).c_str() );
            }
        }
        return QVariant();
    }

    Qt::ItemFlags NodeModel::flags ( const QModelIndex & index ) const
    {
        if ( index.isValid() && ( index.column() == 1 ) )
        {
            return QAbstractItemModel::flags ( index ) | Qt::ItemIsEditable;
        }
        return QAbstractItemModel::flags ( index );
    }

    bool NodeModel::setData ( const QModelIndex & index, const QVariant & value, int role )
    {
        if ( !mNode )
        {
            return false;
        }
        const Node::Property* index_property = reinterpret_cast<Node::Property*> ( index.internalPointer() );
        if ( ( role == Qt::EditRole ) && ( index.isValid() ) && ( value.isValid() ) && ( index.column() == 1 ) )
        {
            index_property->SetByString ( mNode, value.toString().toStdString() );
            emit dataChanged ( index, index );
            return true;
        }
        return false;
    }

    void NodeModel::SetNode ( Node* aNode )
    {
        beginResetModel();
        mNode = aNode;
        mNodeProperties = &aNode->GetProperties();
        endResetModel();
    }
}
