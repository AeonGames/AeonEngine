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

#ifndef AEONGAMES_LISTMODEL_H
#define AEONGAMES_LISTMODEL_H
#include <QAbstractListModel>
#include <vector>
#include "aeongames/Node.hpp"

namespace AeonGames
{
    class ListModel : public QAbstractListModel
    {
        Q_OBJECT
    public:
        ListModel ( QObject *parent = nullptr );
        virtual ~ListModel();
        ///@name Qt QAbstractListModel overrides
        //@{
        int rowCount ( const QModelIndex & index = QModelIndex() ) const = 0;
        QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const = 0;
        QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const = 0;
        //@}
        void SetNode ( Node* aNode );
        const Node* GetNode () const;
        Node* GetNode ();
    protected:
        Node* mNode{};
    };
}
#endif
