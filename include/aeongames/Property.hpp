/*
Copyright (C) 2014-2019,2025,2026 Rodrigo Jose Hernandez Cordoba

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
/** @file Property.hpp
 *  @brief Defines the Property variant type used for generic value storage.
 */
#ifndef AEONGAMES_PROPERTY_H
#define AEONGAMES_PROPERTY_H
#include <variant>
#include <string>
#include <filesystem>

namespace AeonGames
{
    /** @brief A variant type that can hold any commonly used property value.
     *
     *  Supports integral types, floating-point types, strings, and filesystem paths.
     */
    using Property = std::variant
                     <
                     int,
                     long,
                     long long,
                     unsigned,
                     unsigned long,
                     unsigned long long,
                     float,
                     double,
                     std::string,
                     std::filesystem::path
                     >;
}
#endif
