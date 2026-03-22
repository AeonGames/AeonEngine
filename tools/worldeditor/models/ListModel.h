/*
Copyright (C) 2018,2019,2025,2026 Rodrigo Jose Hernandez Cordoba

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
    /** @brief Abstract list model base class associated with a scene node. */
    class ListModel : public QAbstractListModel
    {
        Q_OBJECT
    public:
        /**
         * @brief Construct the list model.
         * @param parent Parent QObject.
         */
        ListModel ( QObject *parent = nullptr );
        /** @brief Destructor. */
        virtual ~ListModel();
        ///@name Qt QAbstractListModel overrides
        //@{
        /** @brief Return the number of rows under the given parent.
         *  @param index Parent model index.
         *  @return Number of rows. */
        int rowCount ( const QModelIndex & index = QModelIndex() ) const = 0;
        /** @brief Return the data for the given index and role.
         *  @param index Model index.
         *  @param role Data role.
         *  @return Data as a QVariant. */
        QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const = 0;
        /** @brief Return the header data for the given section, orientation, and role.
         *  @param section Header section index.
         *  @param orientation Horizontal or vertical header.
         *  @param role Data role.
         *  @return Header data as a QVariant. */
        QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const = 0;
        //@}
        /**
         * @brief Set the node associated with this model.
         * @param aNode Pointer to the node.
         */
        void SetNode ( Node* aNode );
        /** @brief Get a const pointer to the associated node.
         *  @return Const pointer to the node. */
        const Node* GetNode () const;
        /** @brief Get a mutable pointer to the associated node.
         *  @return Pointer to the node. */
        Node* GetNode ();
    protected:
        Node* mNode{}; ///< Pointer to the associated node.
    };
}
#endif
