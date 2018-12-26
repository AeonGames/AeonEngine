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
#include "PropertyModel.h"

namespace AeonGames
{
    PropertyModel::PropertyModel ( QObject *parent ) :
        QAbstractItemModel ( parent ) {}

    PropertyModel::~PropertyModel() = default;

    QVariant PropertyModel::headerData ( int section, Qt::Orientation orientation, int role ) const
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

    QModelIndex PropertyModel::parent ( const QModelIndex & index ) const
    {
        return QModelIndex();
    }

    int PropertyModel::columnCount ( const QModelIndex & index ) const
    {
        return 2;
    }

    bool PropertyModel::hasChildren ( const QModelIndex & index ) const
    {
        return rowCount() > 0;
    }


    Qt::ItemFlags PropertyModel::flags ( const QModelIndex & index ) const
    {
        if ( index.isValid() && ( index.column() == 1 )  )
        {
            return QAbstractItemModel::flags ( index ) | Qt::ItemIsEditable;
        }
        return QAbstractItemModel::flags ( index );
    }
}
