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
            Field ( google::protobuf::Message* aMessage, const google::protobuf::FieldDescriptor* aFieldDescriptor, int aRepeatedIndex = 0, Field* aParent = nullptr );
            Field ( const Field& aField );
            Field& operator= ( const Field& aField );
            Field ( const Field&& aField );
            Field& operator= ( const Field&& aField );
            int GetIndexAtParent() const;
            int GetRepeatedIndex() const;
            std::string GetPrintableName() const;
            google::protobuf::Message* GetMessagePtr() const;
            const google::protobuf::FieldDescriptor* GetFieldDescriptor() const;
            const Field* GetParent() const;
            const std::vector<Field>& GetChildren() const;
            Field& GetField ( const google::protobuf::FieldDescriptor * aFieldDescriptor );
        private:
            google::protobuf::Message* mMessage{};
            const google::protobuf::FieldDescriptor* mFieldDescriptor{};
            int mRepeatedIndex{};
            Field* mParent{};
            std::vector<Field> mChildren{};
        };
        MessageWrapper();
        MessageWrapper ( google::protobuf::Message* aMessage );
        ~MessageWrapper();
        void SetMessage ( google::protobuf::Message* aMessage );
        google::protobuf::Message* GetMessagePtr() const;
        const std::vector<Field>& GetFields() const;
        int GetFieldIndex ( const Field* aField ) const;
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
        Field& GetField ( const google::protobuf::FieldDescriptor * aFieldDescriptor );
        /**}*/
    private:
        google::protobuf::Message* mMessage{};
        std::vector<Field> mFields{};
    };
}
#endif
