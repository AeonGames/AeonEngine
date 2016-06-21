/*
Copyright 2016 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_MESH_H
#define AEONGAMES_MESH_H
#include <cstdint>

namespace AeonGames
{
    class Mesh
    {
    public:
        virtual void Render() const = 0;
    protected:
        virtual ~Mesh() = default;
        enum AttributeMask
        {
            POSITION_MASK  = 0b1,
            NORMAL_MASK    = 0b10,
            TANGENT_MASK   = 0b100,
            BITANGENT_MASK = 0b1000,
            UV_MASK        = 0b10000,
            WEIGHT_MASK    = 0b100000,
        };
    };
}
#endif
