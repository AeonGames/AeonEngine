/*
Copyright (C) 2019 Rodrigo Jose Hernandez Cordoba

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

#include "PropertyDelegate.h"

namespace AeonGames
{
    PropertyDelegate::PropertyDelegate ( QObject *parent ) : QStyledItemDelegate ( parent ) {}

    QWidget *PropertyDelegate::createEditor ( QWidget *parent,
            const QStyleOptionViewItem &option,
            const QModelIndex &index ) const
    {
#if 0
        QSpinBox *editor = new QSpinBox ( parent );
        editor->setFrame ( false );
        editor->setMinimum ( 0 );
        editor->setMaximum ( 100 );

        return editor;
#endif
        return QStyledItemDelegate::createEditor ( parent, option, index );
    }
    void PropertyDelegate::setEditorData ( QWidget *editor,
                                           const QModelIndex &index ) const
    {
#if 0
        int value = index.model()->data ( index, Qt::EditRole ).toInt();

        QSpinBox *spinBox = static_cast<QSpinBox*> ( editor );
        spinBox->setValue ( value );
#endif
        QStyledItemDelegate::setEditorData ( editor, index );
    }
    void PropertyDelegate::setModelData ( QWidget *editor, QAbstractItemModel *model,
                                          const QModelIndex &index ) const
    {
#if 0
        QSpinBox *spinBox = static_cast<QSpinBox*> ( editor );
        spinBox->interpretText();
        int value = spinBox->value();

        model->setData ( index, value, Qt::EditRole );
#endif
        QStyledItemDelegate::setModelData ( editor, model, index );
    }
    void PropertyDelegate::updateEditorGeometry ( QWidget *editor,
            const QStyleOptionViewItem &option, const QModelIndex &index ) const
    {
#if 0
        editor->setGeometry ( option.rect );
#endif
        QStyledItemDelegate::updateEditorGeometry ( editor, option, index );
    }
}
