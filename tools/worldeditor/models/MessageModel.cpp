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
        if ( index.isValid() && reinterpret_cast<const MessageWrapper::Field*> ( index.internalPointer() )->GetChildren().size() )
        {
            return 1;
        }
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
            if ( role == Qt::EditRole || role == Qt::DisplayRole )
                switch ( index.column() )
                {
                case 0:
                    if ( role == Qt::DisplayRole )
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
                    if ( field->GetFieldDescriptor()->cpp_type() != google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE )
                    {
                        google::protobuf::Message* message = field->GetMessagePtr();
                        const google::protobuf::Reflection* reflection = message->GetReflection();
                        const google::protobuf::FieldDescriptor* field_descriptor = field->GetFieldDescriptor();
                        switch ( field_descriptor->cpp_type() )
                        {
                        case google::protobuf::FieldDescriptor::CPPTYPE_DOUBLE:
                            return ( field_descriptor->is_repeated() ) ? QVariant ( reflection->GetRepeatedDouble ( *message, field_descriptor, field->GetRepeatedIndex() ) ) : QVariant ( reflection->GetDouble ( *message, field_descriptor ) );
                        case google::protobuf::FieldDescriptor::CPPTYPE_FLOAT:
                            return ( field_descriptor->is_repeated() ) ? QVariant ( reflection->GetRepeatedFloat ( *message, field_descriptor, field->GetRepeatedIndex() ) ) : QVariant ( reflection->GetFloat ( *message, field_descriptor ) );
                        case google::protobuf::FieldDescriptor::CPPTYPE_INT64:
                            return ( field_descriptor->is_repeated() ) ? QVariant ( reflection->GetRepeatedInt64 ( *message, field_descriptor, field->GetRepeatedIndex() ) ) : QVariant ( reflection->GetInt64 ( *message, field_descriptor ) );
                        case google::protobuf::FieldDescriptor::CPPTYPE_UINT64:
                            return ( field_descriptor->is_repeated() ) ? QVariant ( reflection->GetRepeatedUInt64 ( *message, field_descriptor, field->GetRepeatedIndex() ) ) : QVariant ( reflection->GetUInt64 ( *message, field_descriptor ) );
                        case google::protobuf::FieldDescriptor::CPPTYPE_INT32:
                            return ( field_descriptor->is_repeated() ) ? QVariant ( reflection->GetRepeatedInt32 ( *message, field_descriptor, field->GetRepeatedIndex() ) ) : QVariant ( reflection->GetInt32 ( *message, field_descriptor ) );
                        case google::protobuf::FieldDescriptor::CPPTYPE_BOOL:
                            return ( field_descriptor->is_repeated() ) ? QVariant ( reflection->GetRepeatedBool ( *message, field_descriptor, field->GetRepeatedIndex() ) ) : QVariant ( reflection->GetBool ( *message, field_descriptor ) );
                        case google::protobuf::FieldDescriptor::CPPTYPE_STRING:
                            return ( field_descriptor->is_repeated() ) ? QString ( reflection->GetRepeatedString ( *message, field_descriptor, field->GetRepeatedIndex() ).c_str() ) : QString ( reflection->GetString ( *message, field_descriptor ).c_str() );
                        case google::protobuf::FieldDescriptor::CPPTYPE_UINT32:
                            return ( field_descriptor->is_repeated() ) ? QVariant ( reflection->GetRepeatedUInt32 ( *message, field_descriptor, field->GetRepeatedIndex() ) ) : QVariant ( reflection->GetUInt32 ( *message, field_descriptor ) );
                        case google::protobuf::FieldDescriptor::CPPTYPE_ENUM:
                            return ( field_descriptor->is_repeated() ) ? QVariant ( reflection->GetRepeatedEnumValue ( *message, field_descriptor, field->GetRepeatedIndex() ) ) : QVariant ( reflection->GetEnumValue ( *message, field_descriptor ) );
                        }
                    }
                }
        }
        return QVariant();
    }

    Qt::ItemFlags MessageModel::flags ( const QModelIndex & index ) const
    {
        if ( index.isValid() && ( index.column() == 1 ) && reinterpret_cast<const MessageWrapper::Field*> ( index.internalPointer() )->GetFieldDescriptor()->cpp_type() != google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE )
        {
            return QAbstractItemModel::flags ( index ) | Qt::ItemIsEditable;
        }
        return QAbstractItemModel::flags ( index );
    }

    bool MessageModel::setData ( const QModelIndex & index, const QVariant & value, int role )
    {
        if ( mMessageWrapper.GetMessagePtr() && ( role == Qt::EditRole ) && ( index.isValid() ) && ( value.isValid() ) && ( index.column() == 1 ) )
        {
            const MessageWrapper::Field* field = reinterpret_cast<const MessageWrapper::Field*> ( index.internalPointer() );
            google::protobuf::Message* message = field->GetMessagePtr();
            const google::protobuf::Reflection* reflection = message->GetReflection();
            const google::protobuf::FieldDescriptor* field_descriptor = field->GetFieldDescriptor();
            switch ( field_descriptor->cpp_type() )
            {
            case google::protobuf::FieldDescriptor::CPPTYPE_DOUBLE:
                if ( value.canConvert<double>() )
                {
                    ( field_descriptor->is_repeated() ) ? reflection->SetRepeatedDouble ( message, field_descriptor, field->GetRepeatedIndex(), value.toDouble() ) : reflection->SetDouble ( message, field_descriptor, value.toDouble() );
                }
                break;
            case google::protobuf::FieldDescriptor::CPPTYPE_FLOAT:
                if ( value.canConvert<float>() )
                {
                    ( field_descriptor->is_repeated() ) ? reflection->SetRepeatedFloat ( message, field_descriptor, field->GetRepeatedIndex(), value.toFloat() ) : reflection->SetFloat ( message, field_descriptor, value.toFloat() );
                }
                break;
            case google::protobuf::FieldDescriptor::CPPTYPE_INT64:
                if ( value.canConvert<int64_t>() )
                {
                    ( field_descriptor->is_repeated() ) ? reflection->SetRepeatedInt64 ( message, field_descriptor, field->GetRepeatedIndex(), value.toInt() ) : reflection->SetInt64 ( message, field_descriptor, value.toInt() );
                }
                break;
            case google::protobuf::FieldDescriptor::CPPTYPE_UINT64:
                if ( value.canConvert<uint64_t>() )
                {
                    ( field_descriptor->is_repeated() ) ? reflection->SetRepeatedUInt64 ( message, field_descriptor, field->GetRepeatedIndex(), value.toUInt() ) : reflection->SetUInt64 ( message, field_descriptor, value.toUInt() );
                }
                break;
            case google::protobuf::FieldDescriptor::CPPTYPE_INT32:
                if ( value.canConvert<int32_t>() )
                {
                    ( field_descriptor->is_repeated() ) ? reflection->SetRepeatedInt32 ( message, field_descriptor, field->GetRepeatedIndex(), value.toInt() ) : reflection->SetInt32 ( message, field_descriptor, value.toInt() );
                }
                break;
            case google::protobuf::FieldDescriptor::CPPTYPE_BOOL:
                if ( value.canConvert<bool>() )
                {
                    ( field_descriptor->is_repeated() ) ? reflection->SetRepeatedBool ( message, field_descriptor, field->GetRepeatedIndex(), value.toBool() ) : reflection->SetBool ( message, field_descriptor, value.toBool() );
                }
                break;
            case google::protobuf::FieldDescriptor::CPPTYPE_STRING:
                if ( value.canConvert<QString>() )
                {
                    ( field_descriptor->is_repeated() ) ? reflection->SetRepeatedString ( message, field_descriptor, field->GetRepeatedIndex(), value.toString().toStdString() ) : reflection->SetString ( message, field_descriptor, value.toString().toStdString() );
                }
                break;
            case google::protobuf::FieldDescriptor::CPPTYPE_UINT32:
                if ( value.canConvert<uint32_t>() )
                {
                    ( field_descriptor->is_repeated() ) ? reflection->SetRepeatedUInt32 ( message, field_descriptor, field->GetRepeatedIndex(), value.toUInt() ) : reflection->SetUInt32 ( message, field_descriptor, value.toUInt() );
                }
                break;
            case google::protobuf::FieldDescriptor::CPPTYPE_ENUM:
                if ( value.canConvert<uint32_t>() )
                {
                    ( field_descriptor->is_repeated() ) ? reflection->SetRepeatedEnumValue ( message, field_descriptor, field->GetRepeatedIndex(), value.toUInt() ) : reflection->SetEnumValue ( message, field_descriptor, value.toUInt() );
                }
                break;
            }
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
