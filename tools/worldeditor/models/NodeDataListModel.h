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

#ifndef AEONGAMES_NODEDATALISTMODEL_H
#define AEONGAMES_NODEDATALISTMODEL_H
#include <vector>
#include <memory>
#include "ListModel.h"

namespace AeonGames
{
    class NodeDataListModel : public ListModel
    {
        Q_OBJECT
    public:
        NodeDataListModel ( QObject *parent = nullptr );
        virtual ~NodeDataListModel();
        ///@name Qt QAbstractListModel overrides
        //@{
        int rowCount ( const QModelIndex & index = QModelIndex() ) const override;
        QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const override;
        QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const override;
        //@}
    };
}
#endif
