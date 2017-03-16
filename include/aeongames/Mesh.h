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
        enum AttributeMask
        {
            POSITION_BIT = 0b1,
            NORMAL_BIT = 0b10,
            TANGENT_BIT = 0b100,
            BITANGENT_BIT = 0b1000,
            UV_BIT = 0b10000,
            WEIGHT_BIT = 0b100000,
        };
        struct TriangleGroup
        {
            float mCenterRadii[6];
            uint32_t mVertexFlags = 0;
            uint32_t mVertexCount = 0;
            uint32_t mIndexType = 0;
            uint32_t mIndexCount = 0;
            std::string mVertexBuffer;
            std::string mIndexBuffer;
        };
        Mesh ( std::string  aFilename );
        ~Mesh();
        DLL uint32_t GetStride ( uint32_t aFlags ) const;
        DLL uint32_t GetIndexSize ( uint32_t aIndexType ) const;
        DLL const float * const GetCenterRadii() const;
        DLL const std::vector<TriangleGroup>& GetTriangleGroups() const;
    private:
        void Initialize();
        void Finalize();
        std::string mFilename;
        float mCenterRadii[6];
        std::vector<TriangleGroup> mTriangleGroups;
    };
}
#endif
