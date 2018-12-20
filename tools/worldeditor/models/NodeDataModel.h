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

#ifndef AEONGAMES_NODEDATAMODEL_H
#define AEONGAMES_NODEDATAMODEL_H
#include "aeongames/NodeData.h"
#include "PropertyModel.h"
namespace AeonGames
{
    class NodeDataModel : public PropertyModel
    {
        Q_OBJECT
    public:
        NodeDataModel ( QObject *parent = nullptr );
        virtual ~NodeDataModel();
        ///@name PropertyModel overrides
        //@{
        QModelIndex index ( int row, int column, const QModelIndex & parent = QModelIndex() ) const override;
        QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const override;
        bool setData ( const QModelIndex & index, const QVariant & value, int role ) override;
        //@}
        void SetNodeData ( NodeData* aNodeData );
    private:
        NodeData* mNodeData{};
    };
}
#endif
