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
#include <typeinfo>
#include <cassert>

#include "aeongames/ResourceId.h"
#include "aeongames/NodeData.h"
#include "aeongames/PropertyInfo.h"
#include "aeongames/ToString.h"
#include "NodeDataModel.h"

Q_DECLARE_METATYPE ( AeonGames::ResourceId );

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

#define QVariantFromValue(X) \
        if(aUntypedRef.HasType<X>())\
        {\
            return QVariant::fromValue(aUntypedRef.Get<X>());\
        }

    static QVariant GetQVariantFromUntypedRef ( const UntypedRef& aUntypedRef )
    {
        QVariantFromValue ( int );
        QVariantFromValue ( long );
        QVariantFromValue ( long long );
        QVariantFromValue ( unsigned );
        QVariantFromValue ( unsigned long );
        QVariantFromValue ( unsigned long long );
        QVariantFromValue ( float );
        QVariantFromValue ( double );
        return QString ( "Invalid Type" );
    }
#undef QVariantFromValue

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
                    return GetQVariantFromUntypedRef ( mNodeData->GetProperty ( mNodeData->GetPropertyInfoArray() [index.row()].GetStringId() ) );
                    break;
                }
        }
        return QVariant();
    }

    bool NodeDataModel::setData ( const QModelIndex & index, const QVariant & value, int role )
    {
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
        return false;
    }

    void NodeDataModel::SetNodeData ( NodeData* aNodeData )
    {
        beginResetModel();
        mNodeData = aNodeData;
        endResetModel();
    }
}
