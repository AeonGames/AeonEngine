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
#ifndef AEONGAMES_MESSAGEWRAPPER_H
#define AEONGAMES_MESSAGEWRAPPER_H
#include <vector>
#include <string>
#include "aeongames/Platform.h"

namespace google
{
    namespace protobuf
    {
        class Message;
        class Descriptor;
        class FieldDescriptor;
        class EnumValueDescriptor;
    }
}
namespace AeonGames
{
    class MessageWrapper
    {
    public:
        class Field
        {
        public:
            DLL Field ( google::protobuf::Message* aMessage, const google::protobuf::FieldDescriptor* aFieldDescriptor, int aRepeatedIndex = 0, Field* aParent = nullptr );
            DLL Field ( const Field& aField );
            DLL Field& operator= ( const Field& aField );
            DLL Field ( const Field&& aField );
            DLL Field& operator= ( const Field&& aField );
            DLL int GetIndexAtParent() const;
            DLL int GetRepeatedIndex() const;
            DLL std::string GetPrintableName() const;
            DLL google::protobuf::Message* GetMessagePtr() const;
            DLL const google::protobuf::FieldDescriptor* GetFieldDescriptor() const;
            DLL const Field* GetParent() const;
            DLL const std::vector<Field>& GetChildren() const;
            DLL Field& GetField ( const google::protobuf::FieldDescriptor * aFieldDescriptor );
        private:
            google::protobuf::Message* mMessage{};
            const google::protobuf::FieldDescriptor* mFieldDescriptor{};
            int mRepeatedIndex{};
            Field* mParent{};
            std::vector<Field> mChildren{};
        };
        DLL MessageWrapper();
        DLL MessageWrapper ( google::protobuf::Message* aMessage );
        DLL ~MessageWrapper();
        DLL void SetMessage ( google::protobuf::Message* aMessage );
        DLL google::protobuf::Message* GetMessagePtr() const;
        DLL const std::vector<Field>& GetFields() const;
        DLL int GetFieldIndex ( const Field* aField ) const;
        /**@name Message Interface*/
        /**{*/
#if 0
        Field& SetInt32 ( const google::protobuf::FieldDescriptor * aFieldDescriptor, int32_t aValue, int aIndex = 0 );
        Field& SetInt64 ( const google::protobuf::FieldDescriptor * aFieldDescriptor, int64_t aValue, int aIndex = 0 );
        Field& SetUInt32 ( const google::protobuf::FieldDescriptor * aFieldDescriptor, uint32_t aValue, int aIndex = 0 );
        Field& SetUInt64 ( const google::protobuf::FieldDescriptor * aFieldDescriptor, uint64_t aValue, int aIndex = 0 );
        Field& SetFloat ( const google::protobuf::FieldDescriptor * aFieldDescriptor, float aValue, int aIndex = 0 );
        Field& SetDouble ( const google::protobuf::FieldDescriptor * aFieldDescriptor, double aValue, int aIndex = 0 );
        Field& SetBool ( const google::protobuf::FieldDescriptor * aFieldDescriptor, bool aValue, int aIndex = 0 );
        Field& SetString ( const google::protobuf::FieldDescriptor * aFieldDescriptor, const std::string & aValue, int aIndex = 0 );
        Field& SetEnum ( const google::protobuf::FieldDescriptor * aFieldDescriptor, const google::protobuf::EnumValueDescriptor * aValue, int aIndex = 0 );
        Field& SetEnumValue ( const google::protobuf::FieldDescriptor * aFieldDescriptor, int aValue, int aIndex = 0 );
        Field& AddInt32 ( const google::protobuf::FieldDescriptor * aFieldDescriptor, int32_t aValue );
        Field& AddInt64 ( const google::protobuf::FieldDescriptor * aFieldDescriptor, int64_t aValue );
        Field& AddUInt32 ( const google::protobuf::FieldDescriptor * aFieldDescriptor, uint32_t aValue );
        Field& AddUInt64 ( const google::protobuf::FieldDescriptor * aFieldDescriptor, uint64_t aValue );
        Field& AddFloat ( const google::protobuf::FieldDescriptor * aFieldDescriptor, float aValue );
        Field& AddDouble ( const google::protobuf::FieldDescriptor * aFieldDescriptor, double aValue );
        Field& AddBool ( const google::protobuf::FieldDescriptor * aFieldDescriptor, bool aValue );
        Field& AddString ( const google::protobuf::FieldDescriptor * aFieldDescriptor, const std::string & aValue );
        Field& AddEnum ( const google::protobuf::FieldDescriptor * aFieldDescriptor, const google::protobuf::EnumValueDescriptor * aValue );
        Field& AddEnumValue ( const google::protobuf::FieldDescriptor * aFieldDescriptor, int aValue );
        Field& AddMessage ( const google::protobuf::FieldDescriptor * aFieldDescriptor );
#endif
        DLL Field& GetField ( const google::protobuf::FieldDescriptor * aFieldDescriptor );
        /**}*/
    private:
        google::protobuf::Message* mMessage{};
        std::vector<Field> mFields{};
    };
}
#endif
