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

#ifndef AEONGAMES_COMPONENTMODEL_H
#define AEONGAMES_COMPONENTMODEL_H
#include "aeongames/Component.hpp"
#include "PropertyModel.h"
namespace AeonGames
{
    /** @brief Item model for editing a single component's properties. */
    class ComponentModel : public PropertyModel
    {
        Q_OBJECT
    public:
        /**
         * @brief Construct the component model.
         * @param parent Parent QObject.
         */
        ComponentModel ( QObject *parent = nullptr );
        /** @brief Destructor. */
        virtual ~ComponentModel();
        ///@name PropertyModel overrides
        //@{
        /** @brief Return the index of the item at the given row and column under parent.
         *  @param row Row number.
         *  @param column Column number.
         *  @param parent Parent model index.
         *  @return Model index for the specified item. */
        QModelIndex index ( int row, int column, const QModelIndex & parent = QModelIndex() ) const override;
        /** @brief Return the number of rows under the given parent.
         *  @param index Parent model index.
         *  @return Number of rows. */
        int rowCount ( const QModelIndex & index = QModelIndex() ) const override;
        /** @brief Return the data for the given index and role.
         *  @param index Model index.
         *  @param role Data role.
         *  @return Data as a QVariant. */
        QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const override;
        /** @brief Set the data for the given index and role.
         *  @param index Model index.
         *  @param value New value.
         *  @param role Data role.
         *  @return True if the data was set successfully. */
        bool setData ( const QModelIndex & index, const QVariant & value, int role ) override;
        //@}
        /**
         * @brief Set the component whose properties are represented by this model.
         * @param aComponent Pointer to the component.
         */
        void SetComponent ( Component* aComponent );
    private:
        Component* mComponent{};
    };
}
#endif
