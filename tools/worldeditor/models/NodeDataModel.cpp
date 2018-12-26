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
#include <QAction>
#include <typeinfo>
#include <cassert>

#include "aeongames/ResourceId.h"
#include "aeongames/NodeData.h"
#include "aeongames/PropertyInfo.h"
#include "NodeDataModel.h"

namespace AeonGames
{
    NodeDataModel::NodeDataModel ( QObject *parent ) :
        PropertyModel ( parent ) {}

    NodeDataModel::~NodeDataModel() = default;


    QModelIndex NodeDataModel::index ( int row, int column, const QModelIndex & parent ) const
    {
        if ( mNodeData && row < static_cast<int> ( mNodeData->GetPropertyCount() ) )
        {
            return createIndex ( row, column );
        }
        return QModelIndex();
    }

    int NodeDataModel::rowCount ( const QModelIndex & index ) const
    {
        if ( mNodeData && !index.isValid() )
        {
            return mNodeData->GetPropertyCount();
        }
        // Only root may have children/rows
        return 0;
    }

    QVariant NodeDataModel::data ( const QModelIndex & index, int role ) const
    {
        if ( mNodeData && index.isValid() )
        {
            if ( role == Qt::EditRole || role == Qt::DisplayRole )
                switch ( index.column() )
                {
                case 0:
                    if ( role == Qt::DisplayRole )
                    {
                        return QString ( mNodeData->GetPropertyInfoArray() [index.row()].GetStringId().GetString() );
                    }
                    break;
                case 1:
                    //return PropertyRefToVariant[mProperties[index.row()].GetTypeInfo().hash_code()] ( mProperties[index.row()] );
                    break;
                }
        }
        return QVariant();
    }


    bool NodeDataModel::setData ( const QModelIndex & index, const QVariant & value, int role )
    {
#if 0
        if ( ( role == Qt::EditRole ) && ( index.isValid() ) && ( value.isValid() ) && ( index.column() == 1 ) )
        {
            SetPropertyRef[mProperties[index.row()].GetTypeInfo().hash_code()] ( mProperties[index.row()], value );
            emit dataChanged ( index, index );
            return true;
        }
#endif
        return false;
    }

    void NodeDataModel::SetNodeData ( NodeData* aNodeData )
    {
        beginResetModel();
        mNodeData = aNodeData;
        endResetModel();
    }
}
