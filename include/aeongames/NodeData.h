/*
Copyright (C) 2014-2018 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_NODEDATA_H
#define AEONGAMES_NODEDATA_H
#include <string>
#include <cstdint>
#include <vector>
#include <tuple>
#include "aeongames/Platform.h"
#include "aeongames/CRC.h"
namespace AeonGames
{
    class NodeData
    {
    public:
        class TypeInfo
        {
        public:
            template<std::size_t aNameSize>
            constexpr TypeInfo ( const char ( &aName ) [aNameSize] ) :
                mName{aName}, mId{crc32r ( aName ) } {}
            constexpr uint32_t GetId() const
            {
                return mId;
            };
            constexpr const char* GetName() const
            {
                return mName;
            };
        private:
            const char* mName{};
            uint32_t mId{};
        };
        DLL virtual ~NodeData() = 0;
        virtual const uint32_t GetTypeId() const = 0;
        virtual const char* GetTypeName() const = 0;
    };
}
#endif
