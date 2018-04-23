/*
Copyright (C) 2016-2018 Rodrigo Jose Hernandez Cordoba

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
    class MeshBuffer;
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
        enum IndexType
        {
            /**@todo Refactor to make order better match sizes.
               This will involve changing the exporter and the msh format specification.*/
            BYTE = 0x00,
            UNSIGNED_BYTE = 0x01,
            SHORT = 0x02,
            UNSIGNED_SHORT = 0x03,
            INT = 0x04,
            UNSIGNED_INT = 0x05,
            FLOAT = 0x06,
            TWO_BYTES = 0x07,
            THREE_BYTES = 0x08,
            FOUR_BYTES = 0x09,
            DOUBLE = 0x0A
        };
        DLL Mesh ();
        DLL Mesh ( const std::string& aFilename );
        DLL Mesh ( const void* aBuffer, size_t aBufferSize );
        DLL ~Mesh();
        DLL void Load ( const std::string& aFilename );
        DLL void Load ( const void* aBuffer, size_t aBufferSize );
        DLL void Unload ();
        DLL uint32_t GetStride () const;
        DLL uint32_t GetIndexSize () const;
        DLL const float * const GetCenterRadii() const;
        DLL uint32_t GetVertexFlags() const;
        DLL uint32_t GetVertexCount() const;
        DLL uint32_t GetIndexType() const;
        DLL uint32_t GetIndexCount() const;
        DLL const std::string& GetVertexBuffer() const;
        DLL const std::string& GetIndexBuffer() const;
    private:
        void Load ( const MeshBuffer& aMeshBuffer );
        std::string mFilename;
        float mCenterRadii[6];
        uint32_t mVertexFlags = 0;
        uint32_t mVertexCount = 0;
        uint32_t mIndexType = 0;
        uint32_t mIndexCount = 0;
        std::string mVertexBuffer;
        std::string mIndexBuffer;
    };
}
#endif
