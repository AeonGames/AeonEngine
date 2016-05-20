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
        virtual ~Mesh() = default;
    protected:
        enum AttributeMask
        {
            POSITION_MASK  = 0b1,
            NORMAL_MASK    = 0b10,
            TANGENT_MASK   = 0b100,
            BITANGENT_MASK = 0b1000,
            UV_MASK        = 0b10000,
            WEIGHT_MASK    = 0b100000,
        };
        /// Header for MSH Files.
        struct MSHHeader
        {
            /// File ID, always "AEONMSH\0".
            char id[8];
            /// File Version version[0] = mayor, version[1] = minor.
            unsigned short version[2];
            /// Mesh default axis aligned bounding box, used for navigation collision detection.
            float aabb[6];
            /// Vertex Flags.
            uint32_t vertex_flags;
            /// Stride between static vertex attributes
            uint32_t vertex_count;
            /// Index data type.
            uint32_t index_type;
            /// Index count.
            uint32_t index_count;
        };
    };
}
#endif
