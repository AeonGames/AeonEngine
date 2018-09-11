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
#include <QAction>

#include "aeongames/ProtoBufClasses.h"
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4251 )
#endif
#include <google/protobuf/message.h>
#ifdef _MSC_VER
#pragma warning( pop )
#endif
#include "MessageModel.h"

namespace AeonGames
{
    MessageModel::Field::Field ( google::protobuf::Message* aMessage, const google::protobuf::FieldDescriptor* aFieldDescriptor, int aRepeatedIndex, Field* aParent ) :
        mMessage{ aMessage }, mFieldDescriptor{aFieldDescriptor}, mRepeatedIndex{aRepeatedIndex}, mParent{aParent}
    {
        if ( mFieldDescriptor->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE )
        {
            const google::protobuf::Descriptor* descriptor = mFieldDescriptor->message_type();
            mChildren.reserve ( descriptor->field_count() );
            google::protobuf::Message* message;
            if ( !mFieldDescriptor->is_repeated() )
            {
                message = mMessage->GetReflection()->MutableMessage ( mMessage, mFieldDescriptor );
            }
            else
            {
                message = mMessage->GetReflection()->MutableRepeatedMessage ( mMessage, mFieldDescriptor, mRepeatedIndex );
            }
            const google::protobuf::Reflection* reflection = message->GetReflection();
            for ( int i = 0; i < descriptor->field_count(); ++i )
            {
                const google::protobuf::FieldDescriptor* field = descriptor->field ( i );
                if ( !field->is_repeated() && reflection->HasField ( *message, field ) )
                {
                    mChildren.emplace_back ( message, field, 0, this );
                }
                else if ( field->is_repeated() && reflection->FieldSize ( *message, field ) )
                {
                    for ( int j = 0; j < reflection->FieldSize ( *message, field ); ++j )
                    {
                        mChildren.emplace_back ( message, field, j, this );
                    }
                }
            }
        }
    }

    MessageModel::Field::Field ( const MessageModel::Field& aField ) :
        mMessage{ aField.mMessage },
        mFieldDescriptor{ aField.mFieldDescriptor },
        mRepeatedIndex{aField.mRepeatedIndex},
        mParent{aField.mParent},
        mChildren{aField.mChildren}
    {
        for ( auto& i : mChildren )
        {
            i.mParent = this;
        }
    }
    MessageModel::Field& MessageModel::Field::operator= ( const MessageModel::Field& aField )
    {
        mMessage = aField.mMessage;
        mFieldDescriptor = aField.mFieldDescriptor;
        mRepeatedIndex = aField.mRepeatedIndex;
        mParent = aField.mParent;
        mChildren = aField.mChildren;
        for ( auto& i : mChildren )
        {
            i.mParent = this;
        }
        return *this;
    }
    MessageModel::Field::Field ( const MessageModel::Field&& aField ) :
        mMessage{ aField.mMessage },
        mFieldDescriptor{ aField.mFieldDescriptor },
        mRepeatedIndex{aField.mRepeatedIndex},
        mParent{aField.mParent},
        mChildren{std::move ( aField.mChildren ) }
    {
        for ( auto& i : mChildren )
        {
            i.mParent = this;
        }
    }
    MessageModel::Field& MessageModel::Field::operator= ( const MessageModel::Field&& aField )
    {
        mMessage = aField.mMessage;
        mFieldDescriptor = aField.mFieldDescriptor;
        mRepeatedIndex = aField.mRepeatedIndex;
        mParent = aField.mParent;
        mChildren = std::move ( aField.mChildren );
        for ( auto& i : mChildren )
        {
            i.mParent = this;
        }
        return *this;
    }

    google::protobuf::Message* MessageModel::Field::GetMessagePtr() const
    {
        return const_cast<google::protobuf::Message*> ( mMessage );
    }

    const google::protobuf::FieldDescriptor* MessageModel::Field::GetFieldDescriptor() const
    {
        return mFieldDescriptor;
    }

    int MessageModel::Field::GetRepeatedIndex() const
    {
        return mRepeatedIndex;
    }

    const MessageModel::Field* MessageModel::Field::GetParent() const
    {
        return mParent;
    }

    const std::vector<MessageModel::Field>& MessageModel::Field::GetChildren() const
    {
        return mChildren;
    }

    std::string MessageModel::Field::GetPrintableName() const
    {
        std::string field_name{ mFieldDescriptor->name() };
        std::replace ( field_name.begin(), field_name.end(), '_', ' ' );
        return field_name;
    }

    int MessageModel::Field::GetIndexAtParent() const
    {
        if ( mParent )
        {
            return static_cast<int> ( mParent->mChildren.end() - std::find_if ( mParent->mChildren.begin(), mParent->mChildren.end(), [this] ( const Field & aField )
            {
                return &aField == this;
            } ) );
        }
        return 0;
    }

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
        if ( !GetMessagePtr() )
        {
            return QModelIndex();
        }
        if ( !parent.isValid() )
        {
            if ( ( row >= 0 ) && ( row < static_cast<int> ( GetFields().size() ) ) )
            {
                return createIndex ( row, column, const_cast<MessageModel::Field*> ( &GetFields() [ row ] ) );
            }
        }
        else
        {
            const MessageModel::Field* parent_field = reinterpret_cast<const MessageModel::Field*> ( parent.internalPointer() );
            if ( ( row >= 0 ) && ( row < static_cast<int> ( parent_field->GetChildren().size() ) ) )
            {
                return createIndex ( row, column, const_cast<MessageModel::Field*> ( &parent_field->GetChildren() [row] ) );
            }
        }
        return QModelIndex();
    }

    QModelIndex MessageModel::parent ( const QModelIndex & index ) const
    {
        if ( index.isValid() )
        {
            const MessageModel::Field* parent_field = reinterpret_cast<const MessageModel::Field*> ( index.internalPointer() )->GetParent();
            if ( parent_field )
            {
                return createIndex ( ( parent_field->GetParent() ) ? parent_field->GetIndexAtParent() : GetFieldIndex ( parent_field ), 0, const_cast<MessageModel::Field*> ( parent_field ) );
            }
        }
        return QModelIndex();
    }

    int MessageModel::rowCount ( const QModelIndex & index ) const
    {
        if ( index.isValid() )
        {
            return static_cast<int> ( reinterpret_cast<const MessageModel::Field*> ( index.internalPointer() )->GetChildren().size() );
        }
        return static_cast<int> ( GetFields().size() );
    }

    int MessageModel::columnCount ( const QModelIndex & index ) const
    {
        if ( index.isValid() && reinterpret_cast<const MessageModel::Field*> ( index.internalPointer() )->GetChildren().size() )
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
        if ( GetMessagePtr() && index.isValid() )
        {
            const MessageModel::Field* field = reinterpret_cast<const MessageModel::Field*> ( index.internalPointer() );
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
                            return ( field_descriptor->is_repeated() ) ? QVariant::fromValue ( reflection->GetRepeatedDouble ( *message, field_descriptor, field->GetRepeatedIndex() ) ) : QVariant::fromValue ( reflection->GetDouble ( *message, field_descriptor ) );
                        case google::protobuf::FieldDescriptor::CPPTYPE_FLOAT:
                            return ( field_descriptor->is_repeated() ) ? QVariant::fromValue ( reflection->GetRepeatedFloat ( *message, field_descriptor, field->GetRepeatedIndex() ) ) : QVariant::fromValue ( reflection->GetFloat ( *message, field_descriptor ) );
                        case google::protobuf::FieldDescriptor::CPPTYPE_INT64:
                            return ( field_descriptor->is_repeated() ) ? QVariant::fromValue ( reflection->GetRepeatedInt64 ( *message, field_descriptor, field->GetRepeatedIndex() ) ) : QVariant::fromValue ( reflection->GetInt64 ( *message, field_descriptor ) );
                        case google::protobuf::FieldDescriptor::CPPTYPE_UINT64:
                            return ( field_descriptor->is_repeated() ) ? QVariant::fromValue ( reflection->GetRepeatedUInt64 ( *message, field_descriptor, field->GetRepeatedIndex() ) ) : QVariant::fromValue ( reflection->GetUInt64 ( *message, field_descriptor ) );
                        case google::protobuf::FieldDescriptor::CPPTYPE_INT32:
                            return ( field_descriptor->is_repeated() ) ? QVariant::fromValue ( reflection->GetRepeatedInt32 ( *message, field_descriptor, field->GetRepeatedIndex() ) ) : QVariant::fromValue ( reflection->GetInt32 ( *message, field_descriptor ) );
                        case google::protobuf::FieldDescriptor::CPPTYPE_BOOL:
                            return ( field_descriptor->is_repeated() ) ? QVariant::fromValue ( reflection->GetRepeatedBool ( *message, field_descriptor, field->GetRepeatedIndex() ) ) : QVariant::fromValue ( reflection->GetBool ( *message, field_descriptor ) );
                        case google::protobuf::FieldDescriptor::CPPTYPE_STRING:
                            return ( field_descriptor->is_repeated() ) ? QString ( reflection->GetRepeatedString ( *message, field_descriptor, field->GetRepeatedIndex() ).c_str() ) : QString ( reflection->GetString ( *message, field_descriptor ).c_str() );
                        case google::protobuf::FieldDescriptor::CPPTYPE_UINT32:
                            return ( field_descriptor->is_repeated() ) ? QVariant::fromValue ( reflection->GetRepeatedUInt32 ( *message, field_descriptor, field->GetRepeatedIndex() ) ) : QVariant::fromValue ( reflection->GetUInt32 ( *message, field_descriptor ) );
                        case google::protobuf::FieldDescriptor::CPPTYPE_ENUM:
                            return ( field_descriptor->is_repeated() ) ? QVariant::fromValue ( reflection->GetRepeatedEnumValue ( *message, field_descriptor, field->GetRepeatedIndex() ) ) : QVariant::fromValue ( reflection->GetEnumValue ( *message, field_descriptor ) );
                        default:
                            return QVariant();
                        }
                    }
                }
        }
        return QVariant();
    }

    Qt::ItemFlags MessageModel::flags ( const QModelIndex & index ) const
    {
        if ( index.isValid() && ( index.column() == 1 ) && reinterpret_cast<const MessageModel::Field*> ( index.internalPointer() )->GetFieldDescriptor()->cpp_type() != google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE )
        {
            return QAbstractItemModel::flags ( index ) | Qt::ItemIsEditable;
        }
        return QAbstractItemModel::flags ( index );
    }

    bool MessageModel::setData ( const QModelIndex & index, const QVariant & value, int role )
    {
        if ( GetMessagePtr() && ( role == Qt::EditRole ) && ( index.isValid() ) && ( value.isValid() ) && ( index.column() == 1 ) )
        {
            const MessageModel::Field* field = reinterpret_cast<const MessageModel::Field*> ( index.internalPointer() );
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
            default:
                return false;
            }
            emit dataChanged ( index, index );
            return true;
        }
        return false;
    }


    void MessageModel::SetMessage ( google::protobuf::Message* aMessage )
    {
        beginResetModel();
        mMessage = aMessage;
        mFields.clear();
        if ( !mMessage )
        {
            endResetModel();
            return;
        }
        mFields.reserve ( aMessage->GetDescriptor()->field_count() );
        const google::protobuf::Reflection* reflection = mMessage->GetReflection();
        for ( int i = 0; i < aMessage->GetDescriptor()->field_count(); ++i )
        {
            const google::protobuf::FieldDescriptor* field = aMessage->GetDescriptor()->field ( i );
            if ( !field->is_repeated() && reflection->HasField ( *mMessage, field ) )
            {
                mFields.emplace_back ( mMessage, field );
            }
            else if ( field->is_repeated() && reflection->FieldSize ( *mMessage, field ) )
            {
                for ( int j = 0; j < reflection->FieldSize ( *mMessage, field ); ++j )
                {
                    mFields.emplace_back ( mMessage, field, j );
                }
            }
        }
        endResetModel();
    }

    int MessageModel::GetFieldIndex ( const MessageModel::Field* aField ) const
    {
        return static_cast<int> ( mFields.end() - std::find_if ( mFields.begin(), mFields.end(), [&aField] ( const Field & field )
        {
            return &field == aField;
        } ) );
    }

    const std::vector<MessageModel::Field>& MessageModel::GetFields() const
    {
        return mFields;
    }

    google::protobuf::Message* MessageModel::GetMessagePtr() const
    {
        return mMessage;
    }

    QList<QAction *> MessageModel::GetMenuActions ( const QModelIndex & index )
    {
        QList<QAction *> actions{};

        google::protobuf::Message* message{mMessage};
        const google::protobuf::Reflection* reflection{message->GetReflection() };

        if ( index.isValid() )
        {
            if ( reinterpret_cast<const MessageModel::Field*> ( index.internalPointer() )->GetFieldDescriptor()->type() == google::protobuf::FieldDescriptor::TYPE_MESSAGE )
            {
                MessageModel::Field* field = reinterpret_cast<MessageModel::Field*> ( index.internalPointer() );
                if ( !field->GetFieldDescriptor()->is_repeated() )
                {
                    message = field->GetMessagePtr()->GetReflection()->MutableMessage ( field->GetMessagePtr(), field->GetFieldDescriptor() );
                }
                else
                {
                    message = field->GetMessagePtr()->GetReflection()->MutableRepeatedMessage ( field->GetMessagePtr(), field->GetFieldDescriptor(), field->GetRepeatedIndex() );
                }
                reflection = message->GetReflection();
            }
            else
            {
                /* No Context menu for leaf nodes */
                return actions;
            }
        }

        if ( !mMessage )
        {
            return actions;
        };

        for ( int i = 0; i < message->GetDescriptor()->field_count(); ++i )
        {
            const google::protobuf::FieldDescriptor* field = message->GetDescriptor()->field ( i );
            if ( !field->is_repeated() && !reflection->HasField ( *message, field ) )
            {
                QString text ( tr ( "Add " ) );
                std::string field_name{field->name() };
                std::replace ( field_name.begin(), field_name.end(), '_', ' ' );
                text.append ( field_name.c_str() );
                text.append ( tr ( " field" ) );
                QAction *action = new QAction ( QIcon ( ":/icons/icon_add" ), text, this );
                actions.append ( action );
                connect ( action, &QAction::triggered, this,
                          [this, message, reflection, field]()
                {
                    if ( !message || ! reflection || !field )
                    {
                        return;
                    }
                    switch ( field->cpp_type() )
                    {
                    case google::protobuf::FieldDescriptor::CPPTYPE_DOUBLE:
                        reflection->SetDouble ( message, field, field->default_value_double() );
                        break;
                    case google::protobuf::FieldDescriptor::CPPTYPE_FLOAT:
                        reflection->SetFloat ( message, field, field->default_value_float() );
                        break;
                    case google::protobuf::FieldDescriptor::CPPTYPE_INT64:
                        reflection->SetInt64 ( message, field, field->default_value_int64() );
                        break;
                    case google::protobuf::FieldDescriptor::CPPTYPE_UINT64:
                        reflection->SetUInt64 ( message, field, field->default_value_uint64() );
                        break;
                    case google::protobuf::FieldDescriptor::CPPTYPE_INT32:
                        reflection->SetInt32 ( message, field, field->default_value_int32() );
                        break;
                    case google::protobuf::FieldDescriptor::CPPTYPE_BOOL:
                        reflection->SetBool ( message, field, field->default_value_bool() );
                        break;
                    case google::protobuf::FieldDescriptor::CPPTYPE_STRING:
                        reflection->SetString ( message, field, field->default_value_string() );
                        break;
                    case google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE:
                        reflection->MutableMessage ( message, field );
                        break;
                    case google::protobuf::FieldDescriptor::CPPTYPE_UINT32:
                        reflection->SetUInt32 ( message, field, field->default_value_uint32() );
                        break;
                    case google::protobuf::FieldDescriptor::CPPTYPE_ENUM:
                        reflection->SetEnum ( message, field, field->default_value_enum() );
                        break;
                    }
                    /**@todo Change code so there is no need to call SetMessage. */
                    SetMessage ( mMessage );
                } );
            }
            else if ( field->is_repeated() )
            {
                QString text ( tr ( "Add " ) );
                text.append ( field->name().c_str() );
                text.append ( tr ( " field" ) );
                QAction *action = new QAction ( QIcon ( ":/icons/icon_add" ), text, this );
                actions.append ( action );
                connect ( action, &QAction::triggered, this,
                          [this, message, reflection, field]()
                {
                    if ( !message || ! reflection || !field )
                    {
                        return;
                    }
                    switch ( field->cpp_type() )
                    {
                    case google::protobuf::FieldDescriptor::CPPTYPE_DOUBLE:
                        reflection->AddDouble ( message, field, field->default_value_double() );
                        break;
                    case google::protobuf::FieldDescriptor::CPPTYPE_FLOAT:
                        reflection->AddFloat ( message, field, field->default_value_float() );
                        break;
                    case google::protobuf::FieldDescriptor::CPPTYPE_INT64:
                        reflection->AddInt64 ( message, field, field->default_value_int64() );
                        break;
                    case google::protobuf::FieldDescriptor::CPPTYPE_UINT64:
                        reflection->AddUInt64 ( message, field, field->default_value_uint64() );
                        break;
                    case google::protobuf::FieldDescriptor::CPPTYPE_INT32:
                        reflection->AddInt32 ( message, field, field->default_value_int32() );
                        break;
                    case google::protobuf::FieldDescriptor::CPPTYPE_BOOL:
                        reflection->AddBool ( message, field, field->default_value_bool() );
                        break;
                    case google::protobuf::FieldDescriptor::CPPTYPE_STRING:
                        reflection->AddString ( message, field, field->default_value_string() );
                        break;
                    case google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE:
                        reflection->AddMessage ( message, field );
                        break;
                    case google::protobuf::FieldDescriptor::CPPTYPE_UINT32:
                        reflection->AddUInt32 ( message, field, field->default_value_uint32() );
                        break;
                    case google::protobuf::FieldDescriptor::CPPTYPE_ENUM:
                        reflection->AddEnum ( message, field, field->default_value_enum() );
                        break;
                    }
                    /**@todo Change code so there is no need to call SetMessage. */
                    SetMessage ( mMessage );
                } );
            }
        }
        return actions;
    }
}
