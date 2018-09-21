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
        return QModelIndex();
    }

    QModelIndex ComponentModel::parent ( const QModelIndex & index ) const
    {
        return QModelIndex();
    }

    int ComponentModel::rowCount ( const QModelIndex & index ) const
    {
        return 0;
    }

    int ComponentModel::columnCount ( const QModelIndex & index ) const
    {
        return 2;
    }

    bool ComponentModel::hasChildren ( const QModelIndex & index ) const
    {
        return rowCount ( index ) > 0;
    }

    QVariant ComponentModel::data ( const QModelIndex & index, int role ) const
    {
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

    bool ComponentModel::setData ( const QModelIndex & index, const QVariant & value, int role )
    {
        if ( ( role == Qt::EditRole ) && ( index.isValid() ) && ( value.isValid() ) && ( index.column() == 1 ) )
        {
            emit dataChanged ( index, index );
            return true;
        }
        return false;
    }

    void ComponentModel::SetComponent ( Component* aComponent )
    {
        beginResetModel();
        mComponent = aComponent;
        endResetModel();
    }
}
