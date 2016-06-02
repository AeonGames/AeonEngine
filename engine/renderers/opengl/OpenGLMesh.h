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
#ifndef AEONGAMES_OPENGLMESH_H
#define AEONGAMES_OPENGLMESH_H

#include "aeongames/Mesh.h"
#include <exception>
#include <string>

namespace AeonGames
{
    class OpenGLMesh : public Mesh
    {
    public:
        OpenGLMesh ( const std::string& aFilename );
        ~OpenGLMesh();
    private:
        void Initialize();
        void Finalize();
        uint32_t GetStride ( uint32_t aFlags ) const;
        uint32_t GetIndexSize ( uint32_t aIndexType ) const;
        std::string mFilename;
        //MSHHeader mHeader;
        uint32_t mArray;
        uint32_t mBuffer;
    };
}
#endif