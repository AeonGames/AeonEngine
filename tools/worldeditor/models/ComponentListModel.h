/*
Copyright (C) 2018,2026 Rodrigo Jose Hernandez Cordoba

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

#ifndef AEONGAMES_COMPONENTLISTMODEL_H
#define AEONGAMES_COMPONENTLISTMODEL_H
#include <vector>
#include <memory>
#include "ListModel.h"

namespace AeonGames
{
    /** @brief List model that enumerates the components attached to a node. */
    class ComponentListModel : public ListModel
    {
        Q_OBJECT
    public:
        /**
         * @brief Construct the component list model.
         * @param parent Parent QObject.
         */
        ComponentListModel ( QObject *parent = nullptr );
        /** @brief Destructor. */
        virtual ~ComponentListModel();
        ///@name Qt QAbstractListModel overrides
        //@{
        /** @brief Return the number of rows under the given parent.
         *  @param index Parent model index.
         *  @return Number of rows. */
        int rowCount ( const QModelIndex & index = QModelIndex() ) const override;
        /** @brief Return the data for the given index and role.
         *  @param index Model index.
         *  @param role Data role.
         *  @return Data as a QVariant. */
        QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const override;
        /** @brief Return the header data for the given section, orientation, and role.
         *  @param section Header section index.
         *  @param orientation Horizontal or vertical header.
         *  @param role Data role.
         *  @return Header data as a QVariant. */
        QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const override;
        //@}
    };
}
#endif
