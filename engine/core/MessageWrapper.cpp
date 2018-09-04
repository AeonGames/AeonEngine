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
#include "aeongames/MessageWrapper.h"
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

    MessageWrapper::Field& MessageWrapper::GetField ( const google::protobuf::FieldDescriptor * aFieldDescriptor )
    {
        auto field = std::find_if ( mFields.begin(), mFields.end(), [aFieldDescriptor] ( const MessageWrapper::Field & aField )
        {
            return aField.GetFieldDescriptor() == aFieldDescriptor;
        } );
        if ( field != mFields.end() )
        {
            return *field;
        }
        const google::protobuf::Reflection* reflection = mMessage->GetReflection();
        google::protobuf::Message* message = ( aFieldDescriptor->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE ) ? reflection->MutableMessage ( mMessage, aFieldDescriptor ) : mMessage;
        for ( int i = 0; i < mMessage->GetDescriptor()->field_count(); ++i )
        {
            if ( aFieldDescriptor == mMessage->GetDescriptor()->field ( i ) )
            {
                return mFields.emplace_back ( message, aFieldDescriptor );
            }
        }
        throw std::runtime_error ( "Message type does not contain the requested field type." );
    }

    MessageWrapper::Field& MessageWrapper::Field::GetField ( const google::protobuf::FieldDescriptor * aFieldDescriptor )
    {
        auto field = std::find_if ( mChildren.begin(), mChildren.end(), [aFieldDescriptor] ( const MessageWrapper::Field & aField )
        {
            return aField.GetFieldDescriptor() == aFieldDescriptor;
        } );
        if ( field != mChildren.end() )
        {
            return *field;
        }
        const google::protobuf::Reflection* reflection = mMessage->GetReflection();
        google::protobuf::Message* message = ( aFieldDescriptor->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE ) ? reflection->MutableMessage ( mMessage, aFieldDescriptor ) : mMessage;
        for ( int i = 0; i < mMessage->GetDescriptor()->field_count(); ++i )
        {
            if ( aFieldDescriptor == mMessage->GetDescriptor()->field ( i ) )
            {
                return mChildren.emplace_back ( message, aFieldDescriptor, 0, this );
            }
        }
        throw std::runtime_error ( "Message type does not contain the requested field type." );
    }
    void MessageWrapper::Field::SetInt32 ( int32_t aValue, int aIndex  )
    {
        if ( !mFieldDescriptor->is_repeated() )
        {
            mMessage->GetReflection()->SetInt32 ( mMessage, mFieldDescriptor, aValue );
        }
        else
        {
            mMessage->GetReflection()->SetRepeatedInt32 ( mMessage, mFieldDescriptor, aIndex, aValue );
        }
    }
    void MessageWrapper::Field::SetInt64 ( int64_t aValue, int aIndex  ) {}
    void MessageWrapper::Field::SetUInt32 ( uint32_t aValue, int aIndex  )
    {
        if ( !mFieldDescriptor->is_repeated() )
        {
            mMessage->GetReflection()->SetUInt32 ( mMessage, mFieldDescriptor, aValue );
        }
        else
        {
            mMessage->GetReflection()->SetRepeatedUInt32 ( mMessage, mFieldDescriptor, aIndex, aValue );
        }
    }
    void MessageWrapper::Field::SetUInt64 ( uint64_t aValue, int aIndex  ) {}
    void MessageWrapper::Field::SetFloat ( float aValue, int aIndex  ) {}
    void MessageWrapper::Field::SetDouble ( double aValue, int aIndex  ) {}
    void MessageWrapper::Field::SetBool ( bool aValue, int aIndex  ) {}
    void MessageWrapper::Field::SetString (  const std::string & aValue, int aIndex  ) {}
    void MessageWrapper::Field::SetEnum (  const google::protobuf::EnumValueDescriptor * aValue, int aIndex  ) {}
    void MessageWrapper::Field::SetEnumValue (  int aValue, int aIndex  ) {}
    void MessageWrapper::Field::AddInt32 (  int32_t aValue ) {}
    void MessageWrapper::Field::AddInt64 (  int64_t aValue ) {}
    void MessageWrapper::Field::AddUInt32 (  uint32_t aValue ) {}
    void MessageWrapper::Field::AddUInt64 (  uint64_t aValue ) {}
    void MessageWrapper::Field::AddFloat (  float aValue ) {}
    void MessageWrapper::Field::AddDouble (  double aValue ) {}
    void MessageWrapper::Field::AddBool (  bool aValue ) {}
    void MessageWrapper::Field::AddString (  const std::string & aValue ) {}
    void MessageWrapper::Field::AddEnum (  const google::protobuf::EnumValueDescriptor * aValue ) {}
    void MessageWrapper::Field::AddEnumValue (  int aValue ) {}
    void MessageWrapper::Field::AddMessage ( const google::protobuf::FieldDescriptor * aFieldDescriptor ) {}
}
