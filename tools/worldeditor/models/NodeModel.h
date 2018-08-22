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

#ifndef AEONGAMES_NODEMODEL_H
#define AEONGAMES_NODEMODEL_H
#include <QAbstractListModel>
#include <vector>
#include "aeongames/Memory.h"
#include "aeongames/Node.h"

namespace AeonGames
{
    class NodeModel : public QAbstractListModel
    {
        Q_OBJECT
    public:
        NodeModel ( QObject *parent = nullptr );
        virtual ~NodeModel();
        ///@name Qt QAbstractListModel overrides
        //@{
        int rowCount ( const QModelIndex & index = QModelIndex() ) const override;
        QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const override;
        QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const override;
        //@}
        void SetNode ( Node* aNode );
        const Node* GetNode () const;
    private:
        Node* mNode{};
    };
}
#endif
