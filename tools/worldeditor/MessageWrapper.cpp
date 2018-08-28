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
    int MessageWrapper::Field::GetIndex() const
    {
        if ( mParent )
        {
            return mChildren.end() - std::find_if ( mParent->mChildren.begin(), mParent->mChildren.end(), [this] ( const Field & aField )
            {
                return &aField == this;
            } );
        }
        return 0;
    }
    MessageWrapper::MessageWrapper() = default;
    MessageWrapper::MessageWrapper ( const google::protobuf::Message* aMessage ) : mMessage{aMessage}
    {
        SetMessage ( mMessage );
    }
    MessageWrapper::~MessageWrapper() = default;
    void MessageWrapper::SetMessage ( const google::protobuf::Message* aMessage )
    {
        mMessage = aMessage;
        mFields.clear();
        if ( !mMessage )
        {
            return;
        }
        mFields.reserve ( aMessage->GetDescriptor()->field_count() );
        for ( int i = 0; i < aMessage->GetDescriptor()->field_count(); ++i )
        {
            aMessage->GetDescriptor()->field ( i );
            /**@todo Continue populating the field tree.*/
        }
    }
}
