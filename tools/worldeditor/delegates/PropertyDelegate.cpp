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
#include <limits>
#include <QSpinBox>

namespace AeonGames
{
    PropertyDelegate::PropertyDelegate ( QObject *parent ) : QStyledItemDelegate ( parent ) {}

    template<class T> QWidget *BuildSpinboxEditor ( QWidget* parent )
    {
        QSpinBox *editor = new QSpinBox ( parent );
        editor->setFrame ( false );
        editor->setMinimum ( std::numeric_limits<T>::min() );
        editor->setMaximum ( std::numeric_limits<T>::min() );
        return editor;
    }

    QWidget *PropertyDelegate::createEditor ( QWidget *parent,
            const QStyleOptionViewItem &option,
            const QModelIndex &index ) const
    {
        QVariant value = index.model()->data ( index, Qt::EditRole );
        int user_type{value.userType() };
        switch ( user_type )
        {
        case QMetaType::ULongLong:
            return BuildSpinboxEditor<unsigned long long> ( parent );
        }
        return QStyledItemDelegate::createEditor ( parent, option, index );
    }
    void PropertyDelegate::setEditorData ( QWidget *editor,
                                           const QModelIndex &index ) const
    {
        QVariant value = index.model()->data ( index, Qt::EditRole );
        int user_type{value.userType() };
        switch ( user_type )
        {
        case QMetaType::ULongLong:
        {
            QSpinBox *spinBox = static_cast<QSpinBox*> ( editor );
            spinBox->setValue ( value.value<unsigned long long>() );
        }
        return;
        }
        QStyledItemDelegate::setEditorData ( editor, index );
    }
    void PropertyDelegate::setModelData ( QWidget *editor, QAbstractItemModel *model,
                                          const QModelIndex &index ) const
    {
        QVariant value = index.model()->data ( index, Qt::EditRole );
        int user_type{value.userType() };
        switch ( user_type )
        {
        case QMetaType::ULongLong:
        {
            QSpinBox *spinBox = static_cast<QSpinBox*> ( editor );
            spinBox->interpretText();
            unsigned long long value = spinBox->value();
            model->setData ( index, value, Qt::EditRole );
        }
        return;
        }
        QStyledItemDelegate::setModelData ( editor, model, index );
    }
    void PropertyDelegate::updateEditorGeometry ( QWidget *editor,
            const QStyleOptionViewItem &option, const QModelIndex &index ) const
    {
        QVariant value = index.model()->data ( index, Qt::EditRole );
        int user_type{value.userType() };
        switch ( user_type )
        {
        case QMetaType::ULongLong:
        {
            editor->setGeometry ( option.rect );
        }
        return;
        }
        QStyledItemDelegate::updateEditorGeometry ( editor, option, index );
    }
}
