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
namespace google
{
    namespace protobuf
    {
        class Message;
        class Descriptor;
        class FieldDescriptor;
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
            int GetIndex() const;
            const google::protobuf::FieldDescriptor* mFieldDescriptor{};
            Field* mParent{};
            std::vector<Field> mChildren{};
        };
        MessageWrapper();
        MessageWrapper ( const google::protobuf::Message* aMessage );
        ~MessageWrapper();
        void SetMessage ( const google::protobuf::Message* aMessage );
    private:
        const google::protobuf::Message* mMessage{};
        std::vector<Field> mFields{};
    };
}
#endif
