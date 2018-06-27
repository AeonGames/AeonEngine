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
        return QVariant();
    }

    QModelIndex NodeModel::index ( int row, int column, const QModelIndex & parent ) const
    {
        return QModelIndex();
    }

    QModelIndex NodeModel::parent ( const QModelIndex & index ) const
    {
        return QModelIndex();
    }

    int NodeModel::rowCount ( const QModelIndex & index ) const
    {
        return 0;
    }

    int NodeModel::columnCount ( const QModelIndex & index ) const
    {
        return 2;
    }

    bool NodeModel::hasChildren ( const QModelIndex & index ) const
    {
        return false;
    }

    QVariant NodeModel::data ( const QModelIndex & index, int role ) const
    {
        return QVariant();
    }

    Qt::ItemFlags NodeModel::flags ( const QModelIndex & index ) const
    {
        return QAbstractItemModel::flags ( index );
    }

    bool NodeModel::setData ( const QModelIndex & index, const QVariant & value, int role )
    {
        return false;
    }

    void NodeModel::SetNode ( Node* aNode )
    {
        mNode = aNode;
    }
}
