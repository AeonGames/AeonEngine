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

#include <QIcon>
#include <QFile>
#include <QMimeData>
#include <QDataStream>
#include <QByteArray>
#include <QXmlStreamWriter>
#include <QTextStream>
#include "MessageModel.h"

#include "aeongames/ProtoBufClasses.h"
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4251 )
#endif
#include <google/protobuf/message.h>
#ifdef _MSC_VER
#pragma warning( pop )
#endif

namespace AeonGames
{
    MessageModel::MessageModel ( QObject *parent ) :
        QAbstractItemModel ( parent ) {}

    MessageModel::~MessageModel() = default;

    QVariant MessageModel::headerData ( int section, Qt::Orientation orientation, int role ) const
    {
        if ( ( orientation == Qt::Horizontal ) && ( role == Qt::DisplayRole ) )
        {
            switch ( section )
            {
            case 0:
                return QString ( "Property" );
            case 1:
                return QString ( "Value" );
            default:
                return QVariant();
            }
        }
        return QVariant();
    }

    QModelIndex MessageModel::index ( int row, int column, const QModelIndex & parent ) const
    {
        if ( !mMessageWrapper.GetMessagePtr() )
        {
            return QModelIndex();
        }
        if ( !parent.isValid() )
        {
            if ( ( row >= 0 ) && ( row < static_cast<int> ( mMessageWrapper.GetFields().size() ) ) )
            {
                return createIndex ( row, column, const_cast<MessageWrapper::Field*> ( &mMessageWrapper.GetFields() [ row ] ) );
            }
        }
        else
        {
            const MessageWrapper::Field* parent_field = reinterpret_cast<const MessageWrapper::Field*> ( parent.internalPointer() );
            if ( ( row >= 0 ) && ( row < static_cast<int> ( parent_field->GetChildren().size() ) ) )
            {
                return createIndex ( row, column, const_cast<MessageWrapper::Field*> ( &parent_field->GetChildren() [row] ) );
            }
        }
        return QModelIndex();
    }

    QModelIndex MessageModel::parent ( const QModelIndex & index ) const
    {
        if ( index.isValid() )
        {
            const MessageWrapper::Field* parent_field = reinterpret_cast<const MessageWrapper::Field*> ( index.internalPointer() )->GetParent();
            if ( parent_field )
            {
                return createIndex ( ( parent_field->GetParent() ) ? parent_field->GetIndexAtParent() : mMessageWrapper.GetFieldIndex ( parent_field ), 0, const_cast<MessageWrapper::Field*> ( parent_field ) );
            }
        }
        return QModelIndex();
    }

    int MessageModel::rowCount ( const QModelIndex & index ) const
    {
        if ( index.isValid() )
        {
            return static_cast<int> ( reinterpret_cast<const MessageWrapper::Field*> ( index.internalPointer() )->GetChildren().size() );
        }
        return static_cast<int> ( mMessageWrapper.GetFields().size() );
    }

    int MessageModel::columnCount ( const QModelIndex & index ) const
    {
        return 2;
    }

    bool MessageModel::hasChildren ( const QModelIndex & index ) const
    {
        return rowCount ( index ) > 0;
    }

    QVariant MessageModel::data ( const QModelIndex & index, int role ) const
    {
        if ( mMessageWrapper.GetMessagePtr() && index.isValid() )
        {
            const MessageWrapper::Field* field = reinterpret_cast<const MessageWrapper::Field*> ( index.internalPointer() );
            if ( role == Qt::DisplayRole )
            {
                switch ( index.column() )
                {
                case 0:
                {
                    std::string field_name{ field->GetPrintableName() };
                    if ( field->GetFieldDescriptor()->is_repeated() )
                    {
                        field_name += "[";
                        field_name += std::to_string ( field->GetRepeatedIndex() );
                        field_name += "]";
                    }
                    return QString ( field_name.c_str() );
                }
                break;
                case 1:
                    return QString ( "Not Yet!" );
                }
            }
            else if ( role == Qt::EditRole )
            {
                if ( index.column() == 1  )
                {
                    return QString ( "Not Yet!" );
                }
            }
        }
        return QVariant();
    }

    Qt::ItemFlags MessageModel::flags ( const QModelIndex & index ) const
    {
        if ( index.isValid() && ( index.column() == 1 ) )
        {
            return QAbstractItemModel::flags ( index ) | Qt::ItemIsEditable;
        }
        return QAbstractItemModel::flags ( index );
    }

    bool MessageModel::setData ( const QModelIndex & index, const QVariant & value, int role )
    {
        if ( mMessageWrapper.GetMessagePtr() && ( role == Qt::EditRole ) && ( index.isValid() ) && ( value.isValid() ) && ( index.column() == 1 ) )
        {
            ///@todo Get index_field reflection and change data
            emit dataChanged ( index, index );
            return true;
        }
        return false;
    }

    void MessageModel::SetMessage ( google::protobuf::Message* aMessage )
    {
        beginResetModel();
        mMessageWrapper.SetMessage ( aMessage );
        endResetModel();
    }
    const MessageWrapper& MessageModel::GetMessageWrapper() const
    {
        return mMessageWrapper;
    }
}
