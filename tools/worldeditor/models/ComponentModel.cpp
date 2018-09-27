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

#include "aeongames/Model.h"
#include "ComponentModel.h"

namespace AeonGames
{
    ComponentModel::ComponentModel ( QObject *parent ) :
        QAbstractItemModel ( parent ) {}

    ComponentModel::~ComponentModel() = default;

    QVariant ComponentModel::headerData ( int section, Qt::Orientation orientation, int role ) const
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

    QModelIndex ComponentModel::index ( int row, int column, const QModelIndex & parent ) const
    {
        if ( row < static_cast<int> ( mProperties.size() ) )
        {
            return createIndex ( row, column, nullptr );
        }
        return QModelIndex();
    }

    QModelIndex ComponentModel::parent ( const QModelIndex & index ) const
    {
        return QModelIndex();
    }

    int ComponentModel::rowCount ( const QModelIndex & index ) const
    {
        if ( !index.isValid() )
        {
            return static_cast<int> ( mProperties.size() );
        }
        // Only root may have children/rows
        return 0;
    }

    int ComponentModel::columnCount ( const QModelIndex & index ) const
    {
        return 2;
    }

    bool ComponentModel::hasChildren ( const QModelIndex & index ) const
    {
        return rowCount() > 0;
    }

    static std::unordered_map<size_t, std::function<QVariant ( const PropertyRef& aRef ) >> PropertyRefToVariant =
    {
        {
            typeid ( size_t ).hash_code(),
            [] ( const PropertyRef & aRef ) -> QVariant
            {
                return QVariant ( static_cast<qulonglong> ( aRef.Get<size_t>() ) );
            }
        },
        {
            typeid ( double ).hash_code(),
            [] ( const PropertyRef & aRef ) -> QVariant
            {
                return aRef.Get<double>();
            }
        },
        {
            typeid ( std::shared_ptr<Model> ).hash_code(),
            [] ( const PropertyRef & aRef ) -> QVariant
            {
                try
                {
                    return ( Model::GetPath ( aRef.Get<std::shared_ptr<Model>>() ).c_str() );
                }
                catch ( std::runtime_error& e )
                {
                    return QVariant();
                }
            }
        }
    };


    QVariant ComponentModel::data ( const QModelIndex & index, int role ) const
    {
        if ( index.isValid() )
        {
            if ( role == Qt::EditRole || role == Qt::DisplayRole )
                switch ( index.column() )
                {
                case 0:
                    if ( role == Qt::DisplayRole )
                    {
                        return QString ( mProperties[index.row()].GetName() );
                    }
                    break;
                case 1:
                    return PropertyRefToVariant[mProperties[index.row()].GetTypeInfo().hash_code()] ( mProperties[index.row()] );
                }
        }
        return QVariant();
    }

    Qt::ItemFlags ComponentModel::flags ( const QModelIndex & index ) const
    {
        if ( index.isValid() && ( index.column() == 1 )  )
        {
            return QAbstractItemModel::flags ( index ) | Qt::ItemIsEditable;
        }
        return QAbstractItemModel::flags ( index );
    }

    static std::unordered_map<size_t, std::function<void ( const PropertyRef&, const QVariant& ) >> SetPropertyRef =
    {
        {
            typeid ( size_t ).hash_code(),
            [] ( const PropertyRef & aRef, const QVariant & aVariant )
            {
                aRef.Get<size_t>() = static_cast<size_t> ( aVariant.toULongLong() );
            }
        },
        {
            typeid ( double ).hash_code(),
            [] ( const PropertyRef & aRef, const QVariant & aVariant )
            {
                aRef.Get<double>() = aVariant.toDouble();
            }
        },
        {
            typeid ( std::shared_ptr<Model> ).hash_code(),
            [] ( const PropertyRef & aRef, const QVariant & aVariant )
            {
                try
                {
                    aRef.Get<std::shared_ptr<Model>>() = Model::GetModel ( aVariant.toString().toStdString() );
                }
                catch ( std::runtime_error& e )
                {
                    aRef.Get<std::shared_ptr<Model>>().reset();
                }
            }
        },
    };

    bool ComponentModel::setData ( const QModelIndex & index, const QVariant & value, int role )
    {
        if ( ( role == Qt::EditRole ) && ( index.isValid() ) && ( value.isValid() ) && ( index.column() == 1 ) )
        {
            SetPropertyRef[mProperties[index.row()].GetTypeInfo().hash_code()] ( mProperties[index.row()], value );
            emit dataChanged ( index, index );
            return true;
        }
        return false;
    }

    void ComponentModel::SetComponent ( Component* aComponent )
    {
        beginResetModel();
        mComponent = aComponent;
        mProperties.clear();
        if ( mComponent )
        {
            mProperties = mComponent->GetProperties();
        }
        endResetModel();
    }
}
