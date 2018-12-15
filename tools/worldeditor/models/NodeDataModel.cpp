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
#include "aeongames/Model.h"
#include "NodeDataModel.h"

namespace AeonGames
{
    NodeDataModel::NodeDataModel ( QObject *parent ) :
        QAbstractItemModel ( parent ) {}

    NodeDataModel::~NodeDataModel() = default;

    QVariant NodeDataModel::headerData ( int section, Qt::Orientation orientation, int role ) const
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

    QModelIndex NodeDataModel::index ( int row, int column, const QModelIndex & parent ) const
    {
#if 0
        if ( row < static_cast<int> ( mProperties.size() ) )
        {
            return createIndex ( row, column, nullptr );
        }
#endif
        return QModelIndex();
    }

    QModelIndex NodeDataModel::parent ( const QModelIndex & index ) const
    {
        return QModelIndex();
    }

    int NodeDataModel::rowCount ( const QModelIndex & index ) const
    {
#if 0
        if ( !index.isValid() )
        {
            return static_cast<int> ( mProperties.size() );
        }
#endif
        // Only root may have children/rows
        return 0;
    }

    int NodeDataModel::columnCount ( const QModelIndex & index ) const
    {
        return 2;
    }

    bool NodeDataModel::hasChildren ( const QModelIndex & index ) const
    {
        return rowCount() > 0;
    }

#if 0
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
            typeid ( ResourceId ).hash_code(),
            [] ( const PropertyRef & aRef ) -> QVariant
            {
                try
                {
                    return ( QString::fromStdString ( GetResourcePath ( aRef.Get<ResourceId>().GetPath() ) ) );
                }
                catch ( std::runtime_error& e )
                {
                    std::cout << e.what() << std::endl;
                    return QVariant();
                }
            }
        }
    };
#endif

    QVariant NodeDataModel::data ( const QModelIndex & index, int role ) const
    {
        if ( index.isValid() )
        {
            if ( role == Qt::EditRole || role == Qt::DisplayRole )
                switch ( index.column() )
                {
                case 0:
                    if ( role == Qt::DisplayRole )
                    {
                        //return QString ( mProperties[index.row()].GetName() );
                    }
                    break;
                case 1:
                    //return PropertyRefToVariant[mProperties[index.row()].GetTypeInfo().hash_code()] ( mProperties[index.row()] );
                    break;
                }
        }
        return QVariant();
    }

    Qt::ItemFlags NodeDataModel::flags ( const QModelIndex & index ) const
    {
        if ( index.isValid() && ( index.column() == 1 )  )
        {
            return QAbstractItemModel::flags ( index ) | Qt::ItemIsEditable;
        }
        return QAbstractItemModel::flags ( index );
    }

#if 0
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
            typeid ( ResourceId ).hash_code(),
            [] ( const PropertyRef & aRef, const QVariant & aVariant )
            {
                try
                {
                    // Force Load if required
                    aRef.Get<ResourceId>() = ResourceId{"Model"_crc32, aVariant.toString().toStdString() };
                    aRef.Get<ResourceId>().Store();
                }
                catch ( std::runtime_error& e )
                {
                    std::cout << e.what() << std::endl;
                }
            }
        },
    };
#endif

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
#if 0
        mProperties.clear();
        if ( mNodeData )
        {
            mProperties = mNodeData->GetProperties();
        }
#endif
        endResetModel();
    }
}
