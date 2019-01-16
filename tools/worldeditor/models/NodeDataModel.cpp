/*
Copyright (C) 2018,2019 Rodrigo Jose Hernandez Cordoba

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
#include <QVariant>

#include <typeinfo>
#include <cassert>
#include <string>

#include "aeongames/ResourceId.h"
#include "aeongames/NodeData.h"
#include "aeongames/ToString.h"
#include "aeongames/StringId.h"
#include "NodeDataModel.h"

Q_DECLARE_METATYPE ( AeonGames::StringId );
Q_DECLARE_METATYPE ( std::string );

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

    /** @todo Implement FromStdVariant for backward compatibility. */
    static_assert ( QT_VERSION >= QT_VERSION_CHECK ( 5, 11, 0 ), "QVariant::fromStdVariant not implemented on this version of Qt." );

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
                        return QString ( mNodeData->GetPropertyInfoArray() [index.row()].GetString() );
                    }
                    break;
                case 1:
                    return QVariant::fromStdVariant ( mNodeData->GetProperty ( mNodeData->GetPropertyInfoArray() [index.row()] ) );
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
            const PropertyInfo& property_info = mNodeData->GetPropertyInfoArray() [index.row()];
            size_t hash_code = property_info.GetTypeInfo().hash_code();
            if ( ( hash_code == typeid ( int ).hash_code() ) || ( hash_code == typeid ( long ).hash_code() ) )
            {
                mNodeData->SetProperty ( mNodeData->GetPropertyInfoArray() [index.row()].GetStringId(), value.toInt() );
            }
            else if ( hash_code == typeid ( long long ).hash_code() )
            {
                mNodeData->SetProperty ( mNodeData->GetPropertyInfoArray() [index.row()].GetStringId(), value.toLongLong() );
            }
            else if ( ( hash_code == typeid ( unsigned ).hash_code() ) || ( hash_code == typeid ( unsigned long ).hash_code() ) )
            {
                mNodeData->SetProperty ( mNodeData->GetPropertyInfoArray() [index.row()].GetStringId(), value.toUInt() );
            }
            else if ( hash_code == typeid ( unsigned long long ).hash_code() )
            {
                mNodeData->SetProperty ( mNodeData->GetPropertyInfoArray() [index.row()].GetStringId(), value.toULongLong() );
            }
            else if ( hash_code == typeid ( float ).hash_code() )
            {
                mNodeData->SetProperty ( mNodeData->GetPropertyInfoArray() [index.row()].GetStringId(), value.toFloat() );
            }
            else if ( hash_code == typeid ( double ).hash_code() )
            {
                mNodeData->SetProperty ( mNodeData->GetPropertyInfoArray() [index.row()].GetStringId(), value.toDouble() );
            }
            else
            {
                return false;
            }
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
