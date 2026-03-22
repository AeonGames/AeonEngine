/*
Copyright (C) 2018,2019,2026 Rodrigo Jose Hernandez Cordoba

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

#ifndef AEONGAMES_PROPERTYMODEL_H
#define AEONGAMES_PROPERTYMODEL_H
#include <QAbstractItemModel>
#include <QList>
#include <QAction>

namespace AeonGames
{
    /** @brief Abstract item model base class for property editing views. */
    class PropertyModel : public QAbstractItemModel
    {
        Q_OBJECT
    public:
        /**
         * @brief Construct the property model.
         * @param parent Parent QObject.
         */
        PropertyModel ( QObject *parent = nullptr );
        /** @brief Destructor. */
        virtual ~PropertyModel();
        ///@name Qt QAbstractItemModel overrides
        //@{
        /** @brief Return the parent of the given model index.
         *  @param index The child model index.
         *  @return Parent model index, or an invalid index if the item has no parent. */
        QModelIndex parent ( const QModelIndex & index ) const override;
        /** @brief Return the number of rows under the given parent.
         *  @param index Parent model index.
         *  @return Number of rows. */
        int rowCount ( const QModelIndex & index = QModelIndex() ) const override = 0;
        /** @brief Return the number of columns for children of the given parent.
         *  @param index Parent model index.
         *  @return Number of columns. */
        int columnCount ( const QModelIndex & index = QModelIndex() ) const override;
        /** @brief Return the header data for the given section, orientation, and role.
         *  @param section Header section index.
         *  @param orientation Horizontal or vertical header.
         *  @param role Data role.
         *  @return Header data as a QVariant. */
        QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const override;
        /** @brief Return whether the given parent index has any children.
         *  @param index Parent model index.
         *  @return True if the parent has children. */
        bool hasChildren ( const QModelIndex & index = QModelIndex() ) const override;
        /** @brief Return the item flags for the given model index.
         *  @param index Model index.
         *  @return Item flags. */
        Qt::ItemFlags flags ( const QModelIndex & index ) const override;
        /** @brief Return the index of the item at the given row and column under parent.
         *  @param row Row number.
         *  @param column Column number.
         *  @param parent Parent model index.
         *  @return Model index for the specified item. */
        QModelIndex index ( int row, int column, const QModelIndex & parent = QModelIndex() ) const override = 0;
        /** @brief Return the data for the given index and role.
         *  @param index Model index.
         *  @param role Data role.
         *  @return Data as a QVariant. */
        QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const override = 0;
        /** @brief Set the data for the given index and role.
         *  @param index Model index.
         *  @param value New value.
         *  @param role Data role.
         *  @return True if the data was set successfully. */
        bool setData ( const QModelIndex & index, const QVariant & value, int role ) override = 0;
        //@}
    };
}
#endif
