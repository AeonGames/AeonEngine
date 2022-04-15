/*
Copyright (C) 2019,2022 Rodrigo Jose Hernandez Cordoba

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
#include "WorldEditor.h"
#include <iostream>
#include <limits>
#include <algorithm>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QLineEdit>

namespace AeonGames
{
    PropertyDelegate::PropertyDelegate ( QObject *parent ) : QStyledItemDelegate ( parent ) {}

    template<class T> QWidget *BuildSpinboxEditor ( QWidget* parent )
    {
        QSpinBox *editor = new QSpinBox ( parent );
        editor->setFrame ( false );
        editor->setMinimum (
            static_cast<int> (
                std::max (
                    static_cast<long long> ( std::numeric_limits<T>::min() ),
                    static_cast<long long> ( std::numeric_limits<int>::min() ) ) ) );
        editor->setMaximum (
            static_cast<int> (
                std::min (
                    static_cast<unsigned long long> ( std::numeric_limits<T>::max() ),
                    static_cast<unsigned long long> ( std::numeric_limits<int>::max() ) ) ) );
        return editor;
    }

    template<class T> void SetSpinboxEditorValue ( QWidget *editor, const QVariant& value )
    {
        QSpinBox *spinBox = static_cast<QSpinBox*> ( editor );
        spinBox->setValue ( value.value<T>() );
    }

    template<class T> void SetSpinboxModelData ( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index )
    {
        QSpinBox *spinBox = static_cast<QSpinBox*> ( editor );
        spinBox->interpretText();
        model->setData ( index, QVariant::fromValue ( static_cast<T> ( spinBox->value() ) ), Qt::EditRole );
    }

    QWidget *PropertyDelegate::createEditor ( QWidget *parent,
            const QStyleOptionViewItem &option,
            const QModelIndex &index ) const
    {
        QVariant value = index.model()->data ( index, Qt::EditRole );
        int user_type{value.userType() };
        switch ( user_type )
        {
        case QMetaType::Int:
            return BuildSpinboxEditor<int> ( parent );
        case QMetaType::Long:
            return BuildSpinboxEditor<long> ( parent );
        case QMetaType::LongLong:
            return BuildSpinboxEditor<long long> ( parent );
        case QMetaType::UInt:
            return BuildSpinboxEditor<unsigned int> ( parent );
        case QMetaType::ULong:
            return BuildSpinboxEditor<unsigned long> ( parent );
        case QMetaType::ULongLong:
            return BuildSpinboxEditor<unsigned long long> ( parent );
        case QMetaType::Float:
        {
            auto float_spinbox = new QDoubleSpinBox ( parent );
            float_spinbox->setMinimum (
                std::numeric_limits<float>::min() );
            float_spinbox->setMaximum (
                std::numeric_limits<float>::max() );
            return float_spinbox;
        }
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
        case QMetaType::Int:
            SetSpinboxEditorValue<int> ( editor, value );
            return;
        case QMetaType::Long:
            SetSpinboxEditorValue<long> ( editor, value );
            return;
        case QMetaType::LongLong:
            SetSpinboxEditorValue<long long> ( editor, value );
            return;
        case QMetaType::UInt:
            SetSpinboxEditorValue<unsigned int> ( editor, value );
            return;
        case QMetaType::ULong:
            SetSpinboxEditorValue<unsigned long> ( editor, value );
            return;
        case QMetaType::ULongLong:
            SetSpinboxEditorValue<unsigned long long> ( editor, value );
            return;
        case QMetaType::Float:
        {
            QDoubleSpinBox* spinBox = static_cast<QDoubleSpinBox*> ( editor );
            spinBox->setValue ( value.value<float>() );
        }
        return;
        default:
            if ( user_type == qWorldEditorApp->GetPathMetaType() )
            {
                QLineEdit* line_edit{static_cast<QLineEdit*> ( editor ) };
                line_edit->setText ( QString::fromStdString ( value.value<std::filesystem::path>().string() ) );
                return;
            }
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
        case QMetaType::Int:
            SetSpinboxModelData<int> ( editor, model, index );
            return;
        case QMetaType::Long:
            SetSpinboxModelData<long> ( editor, model, index );
            return;
        case QMetaType::LongLong:
            SetSpinboxModelData<long long> ( editor, model, index );
            return;
        case QMetaType::UInt:
            SetSpinboxModelData<unsigned int> ( editor, model, index );
            return;
        case QMetaType::ULong:
            SetSpinboxModelData<unsigned long> ( editor, model, index );
            return;
        case QMetaType::ULongLong:
            SetSpinboxModelData<unsigned long long> ( editor, model, index );
            return;
        case QMetaType::Float:
        {
            QDoubleSpinBox* spinBox = static_cast<QDoubleSpinBox*> ( editor );
            spinBox->interpretText();
            model->setData ( index, QVariant::fromValue ( static_cast<float> ( spinBox->value() ) ), Qt::EditRole );
        }
        return;
        default:
            if ( user_type == qWorldEditorApp->GetPathMetaType() )
            {
                QLineEdit* line_edit{static_cast<QLineEdit*> ( editor ) };
                model->setData ( index, line_edit->text(), Qt::EditRole );
                return;
            }
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
