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

#ifndef AEONGAMES_COMPONENTMODEL_H
#define AEONGAMES_COMPONENTMODEL_H
#include <QAbstractItemModel>
#include <QList>
#include <QAction>
#include "aeongames/Component.h"
namespace AeonGames
{
    class ComponentModel : public QAbstractItemModel
    {
        Q_OBJECT
    public:
        ComponentModel ( QObject *parent = nullptr );
        virtual ~ComponentModel();
        ///@name Qt QAbstractItemModel overrides
        //@{
        QModelIndex index ( int row, int column, const QModelIndex & parent = QModelIndex() ) const override;
        QModelIndex parent ( const QModelIndex & index ) const override;
        int rowCount ( const QModelIndex & index = QModelIndex() ) const override;
        int columnCount ( const QModelIndex & index = QModelIndex() ) const override;
        QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const override;
        QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const override;
        bool hasChildren ( const QModelIndex & index = QModelIndex() ) const override;
        Qt::ItemFlags flags ( const QModelIndex & index ) const override;
        bool setData ( const QModelIndex & index, const QVariant & value, int role ) override;
        //@}
        void SetComponent ( Component* aComponent );
    private:
        Component* mComponent{};
        std::vector<PropertyRef> mProperties{};
    };
}
#endif