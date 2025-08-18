/*
Copyright (C) 2018,2019,2025 Rodrigo Jose Hernandez Cordoba

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

#include "ComponentListModel.h"
#include "aeongames/Scene.hpp"
#include "aeongames/Node.hpp"
#include "aeongames/StringId.hpp"

namespace AeonGames
{
    ComponentListModel::ComponentListModel ( QObject *parent ) :
        ListModel ( parent ) {}

    ComponentListModel::~ComponentListModel() = default;

    QVariant ComponentListModel::headerData ( int section, Qt::Orientation orientation, int role ) const
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

    int ComponentListModel::rowCount ( const QModelIndex & index ) const
    {
        if ( !mNode || index.isValid() )
        {
            return 0;
        }
        return static_cast<int> ( mNode->GetComponentCount() );
    }

    QVariant ComponentListModel::data ( const QModelIndex & index, int role ) const
    {
        if ( !mNode || !index.isValid() )
        {
            return QVariant();
        }
        if ( role == Qt::DisplayRole )
        {
            return QString ( mNode->GetComponentByIndex ( index.row() )->GetId().GetString() );
        }
        return QVariant();
    }
}
