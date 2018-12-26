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

#include "NodeDataListModel.h"
#include "aeongames/Scene.h"
#include "aeongames/Node.h"
#include "aeongames/StringId.h"

namespace AeonGames
{
    NodeDataListModel::NodeDataListModel ( QObject *parent ) :
        ListModel ( parent ) {}

    NodeDataListModel::~NodeDataListModel() = default;

    QVariant NodeDataListModel::headerData ( int section, Qt::Orientation orientation, int role ) const
    {
        if ( ( orientation == Qt::Horizontal ) && ( role == Qt::DisplayRole ) )
        {
            switch ( section )
            {
            case 0:
                return QString ( "Node Data" );
            default:
                return QVariant();
            }
        }
        return QVariant();
    }

    int NodeDataListModel::rowCount ( const QModelIndex & index ) const
    {
        if ( !mNode || index.isValid() )
        {
            return 0;
        }
        return static_cast<int> ( mNode->GetDataCount() );
    }

    QVariant NodeDataListModel::data ( const QModelIndex & index, int role ) const
    {
        if ( !mNode || !index.isValid() )
        {
            return QVariant();
        }
        if ( role == Qt::DisplayRole )
        {
            return QString ( mNode->GetDataByIndex ( index.row() )->GetId().GetString() );
        }
        return QVariant();
    }
}
