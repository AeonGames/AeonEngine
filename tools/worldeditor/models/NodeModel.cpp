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
            if ( ( row >= 0 ) && ( row < static_cast<int> ( mNode->GetPropertyCount() ) ) )
            {
                return createIndex ( row, column );
            }
        }
        else
        {
            ///@todo implement sub-properties
        }
        return QModelIndex();
    }

    QModelIndex NodeModel::parent ( const QModelIndex & index ) const
    {
        ///@todo implement sub-properties
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
            ///@todo implement sub-properties
        }
        return static_cast<int> ( mNode->GetPropertyCount() );
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
            ///@todo implement sub-properties
            return false;
        }
        return ( mNode->GetPropertyCount() );
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
                    return QString ( mNode->GetPropertyDescriptor ( index.row() ).GetDisplayName().c_str() );
                    break;
                case 1:
                    return QString ( "Not Yet!" );
                    break;
                }
            }
        }
        else if ( role == Qt::EditRole )
        {
            if ( index.isValid() && ( index.column() == 1 ) )
            {
                return QString ( "Not Yet!" );
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
        if ( ( role == Qt::EditRole ) && index.isValid() && ( index.column() == 1 ) )
        {
            /** @todo parse value as expected by the property and call mNode->SetProperty. */
            emit dataChanged ( index, index );
            return true;
        }
        return false;
    }

    void NodeModel::SetNode ( Node* aNode )
    {
        beginResetModel();
        mNode = aNode;
        endResetModel();
    }
}
