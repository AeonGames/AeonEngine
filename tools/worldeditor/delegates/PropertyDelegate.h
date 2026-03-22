/*
Copyright (C) 2019,2021,2026 Rodrigo Jose Hernandez Cordoba

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

#ifndef AEONGAMES_PROPERTYDELEGATE_H
#define AEONGAMES_PROPERTYDELEGATE_H
#include <QStyledItemDelegate>

namespace AeonGames
{
    /** @brief Custom item delegate for editing component properties in views. */
    class PropertyDelegate : public QStyledItemDelegate
    {
        Q_OBJECT

    public:
        /**
         * @brief Construct the property delegate.
         * @param parent Parent object.
         */
        PropertyDelegate ( QObject *parent = nullptr );

        /**
         * @brief Create an editor widget for the given model index.
         * @param parent Parent widget for the editor.
         * @param option Style options for the item.
         * @param index Model index of the item to edit.
         * @return Editor widget.
         */
        QWidget *createEditor ( QWidget *parent, const QStyleOptionViewItem &option,
                                const QModelIndex &index ) const override;

        /**
         * @brief Set the editor widget data from the model.
         * @param editor Editor widget.
         * @param index Model index to read from.
         */
        void setEditorData ( QWidget *editor, const QModelIndex &index ) const override;
        /**
         * @brief Write the editor widget data back to the model.
         * @param editor Editor widget.
         * @param model Model to write to.
         * @param index Model index to write to.
         */
        void setModelData ( QWidget *editor, QAbstractItemModel *model,
                            const QModelIndex &index ) const override;

        /**
         * @brief Update the editor geometry to match the view item area.
         * @param editor Editor widget.
         * @param option Style options for the item.
         * @param index Model index.
         */
        void updateEditorGeometry ( QWidget *editor,
                                    const QStyleOptionViewItem &option, const QModelIndex &index ) const override;
    };
}
#endif
