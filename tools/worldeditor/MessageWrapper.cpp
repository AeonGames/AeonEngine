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
#include "MessageWrapper.h"
#include "aeongames/ProtoBufClasses.h"
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4251 )
#endif
#include <google/protobuf/message.h>
#ifdef _MSC_VER
#pragma warning( pop )
#endif
#include <algorithm>
namespace AeonGames
{
    MessageWrapper::Field::Field ( google::protobuf::Message* aMessage, const google::protobuf::FieldDescriptor* aFieldDescriptor, int aRepeatedIndex, Field* aParent ) :
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

    MessageWrapper::Field::Field ( const MessageWrapper::Field& aField ) :
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
    MessageWrapper::Field& MessageWrapper::Field::operator= ( const MessageWrapper::Field& aField )
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
    MessageWrapper::Field::Field ( const MessageWrapper::Field&& aField ) :
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
    MessageWrapper::Field& MessageWrapper::Field::operator= ( const MessageWrapper::Field&& aField )
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

    google::protobuf::Message* MessageWrapper::Field::GetMessagePtr() const
    {
        return const_cast<google::protobuf::Message*> ( mMessage );
    }

    const google::protobuf::FieldDescriptor* MessageWrapper::Field::GetFieldDescriptor() const
    {
        return mFieldDescriptor;
    }

    int MessageWrapper::Field::GetRepeatedIndex() const
    {
        return mRepeatedIndex;
    }

    const MessageWrapper::Field* MessageWrapper::Field::GetParent() const
    {
        return mParent;
    }

    const std::vector<MessageWrapper::Field>& MessageWrapper::Field::GetChildren() const
    {
        return mChildren;
    }

    std::string MessageWrapper::Field::GetPrintableName() const
    {
        std::string field_name{ mFieldDescriptor->name() };
        std::replace ( field_name.begin(), field_name.end(), '_', ' ' );
        return field_name;
    }

    int MessageWrapper::Field::GetIndexAtParent() const
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
    MessageWrapper::MessageWrapper() = default;
    MessageWrapper::MessageWrapper ( google::protobuf::Message* aMessage ) : mMessage{aMessage}
    {
        SetMessage ( mMessage );
    }
    MessageWrapper::~MessageWrapper() = default;
    void MessageWrapper::SetMessage ( google::protobuf::Message* aMessage )
    {
#if 0
        if ( mMessage == aMessage )
        {
            return;
        };
#endif
        mMessage = aMessage;
        mFields.clear();
        if ( !mMessage )
        {
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
    }

    int MessageWrapper::GetFieldIndex ( const MessageWrapper::Field* aField ) const
    {
        return static_cast<int> ( mFields.end() - std::find_if ( mFields.begin(), mFields.end(), [&aField] ( const Field & field )
        {
            return &field == aField;
        } ) );
    }

    const std::vector<MessageWrapper::Field>& MessageWrapper::GetFields() const
    {
        return mFields;
    }

    google::protobuf::Message* MessageWrapper::GetMessagePtr() const
    {
        return mMessage;
    }
}
