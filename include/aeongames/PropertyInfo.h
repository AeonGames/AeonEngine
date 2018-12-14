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
#ifndef AEONGAMES_PROPERTYINFO_H
#define AEONGAMES_PROPERTYINFO_H
#include <cstdint>
#include <typeinfo>
#include <functional>
#include <type_traits>
#include "aeongames/StringId.h"

namespace AeonGames
{
    class PropertyInfo
    {
    public:
        /// @name Construction/Copy/Destruction
        ///@{
        PropertyInfo ( const StringId& aId, const std::type_info& aTypeInfo ) : mId{aId}, mTypeInfo{aTypeInfo} {}
        ///@}
        const StringId& GetId() const
        {
            return mId;
        }
        const std::type_info& GetTypeInfo() const
        {
            return mTypeInfo;
        }
    private:
        StringId mId;
        const std::type_info& mTypeInfo;
    };
}
#endif
