/*
Copyright (C) 2018,2019 Rodrigo Jose Hernandez Cordoba

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

#include <QIcon>
#include <QFile>
#include <QMimeData>
#include <QDataStream>
#include <QByteArray>
#include <QXmlStreamWriter>
#include <QTextStream>
#include <QAction>
#include <QVariant>

#include <typeinfo>
#include <cassert>
#include <iostream>

#include "aeongames/ResourceId.h"
#include "aeongames/Component.h"
#include "aeongames/ToString.h"
#include "aeongames/StringId.h"
#include "ComponentModel.h"
#include "WorldEditor.h"

namespace AeonGames
{
    ComponentModel::ComponentModel ( QObject *parent ) :
        PropertyModel ( parent ) {}

    ComponentModel::~ComponentModel() = default;


    QModelIndex ComponentModel::index ( int row, int column, const QModelIndex & parent ) const
    {
        if ( mComponent && row < static_cast<int> ( mComponent->GetPropertyCount() ) )
        {
            return createIndex ( row, column );
        }
        return QModelIndex();
    }

    int ComponentModel::rowCount ( const QModelIndex & index ) const
    {
        if ( mComponent && !index.isValid() )
        {
            return mComponent->GetPropertyCount();
        }
        // Only root may have children/rows
        return 0;
    }

    template<typename... Types>
    static inline QVariant fromStdVariant ( const std::variant<Types...> &value )
    {
#if QT_VERSION >= QT_VERSION_CHECK ( 5, 11, 0 )
        return QVariant::fromStdVariant ( value );
#else
        if ( value.valueless_by_exception() )
        {
            return QVariant();
        }
        return std::visit ( [] ( const auto & arg )
        {
            return QVariant::fromValue ( arg );
        }, value );

#endif
    }

    QVariant ComponentModel::data ( const QModelIndex & index, int role ) const
    {
        if ( mComponent && index.isValid() )
        {
            if ( role == Qt::EditRole || role == Qt::DisplayRole )
                switch ( index.column() )
                {
                case 0:
                    if ( role == Qt::DisplayRole )
                    {
                        return QString ( mComponent->GetPropertyInfoArray() [index.row()].GetString() );
                    }
                    break;
                case 1:
                {
                    Property property{ mComponent->GetProperty ( mComponent->GetPropertyInfoArray() [index.row()] ) };
                    if ( std::holds_alternative<std::string> ( property ) )
                    {
                        return QString::fromStdString ( std::get<std::string> ( property ) );
                    }
                    else if ( std::holds_alternative<std::filesystem::path> ( property ) )
                    {
                        return QString::fromStdString ( std::get<std::filesystem::path> ( property ).string() );
                    }
                    else
                    {
                        return fromStdVariant ( property );
                    }
                }
                }
        }
        return QVariant();
    }

    bool ComponentModel::setData ( const QModelIndex & index, const QVariant & value, int role )
    {
        if ( ( role == Qt::EditRole ) && ( index.isValid() ) && ( value.isValid() ) && ( index.column() == 1 ) )
        {
            Property property;
            int user_type{value.userType() };
            std::cout << __func__ << " UserType: " << user_type;
            switch ( user_type )
            {
            case QMetaType::Int:    // 2 int
                property = value.value<int>();
                break;
            case QMetaType::UInt:   //3 unsigned int
                property = value.value<unsigned int>();
                break;
            case QMetaType::Double: //6 double
                property = value.value<double>();
                break;
            case QMetaType::Long:   //32 long
                property = value.value<long>();
                break;
            case QMetaType::LongLong:   //4 LongLong
                property = value.value<long long>();
                break;
            case QMetaType::ULong:  //35 unsigned long
                property = value.value<unsigned long>();
                break;
            case QMetaType::ULongLong:  //5 ULongLong
                property = value.value<unsigned long long>();
                break;
            case QMetaType::Float:  //38 float
                property = value.value<float>();
                break;
            case QMetaType::QString:    //10 QString
                property = value.value<QString>().toStdString();
                break;
            default:
                std::cout << " No Change" << std::endl;
                return false;
            }
            mComponent->SetProperty ( mComponent->GetPropertyInfoArray() [index.row()], property );
            std::cout << " Value Changed" << std::endl;
            emit dataChanged ( index, index );
            return true;
        }
        return false;
    }

    void ComponentModel::SetComponent ( Component* aComponent )
    {
        beginResetModel();
        mComponent = aComponent;
        endResetModel();
    }
}
