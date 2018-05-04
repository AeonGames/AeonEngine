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
#ifndef AEONGAMES_OPENGLBUFFER_H
#define AEONGAMES_OPENGLBUFFER_H
#include <cstddef>
namespace AeonGames
{
    class OpenGLRenderer;
    class OpenGLBuffer
    {
    public:
        OpenGLBuffer ( const OpenGLRenderer& aOpenGLRenderer );
        OpenGLBuffer ( const OpenGLRenderer& aOpenGLRenderer, const size_t aSize, const size_t aUsage, const size_t aProperties, const void *aData = nullptr );
        ~OpenGLBuffer();
        void Initialize ( const size_t aSize, const size_t aUsage, const size_t aProperties, const void *aData = nullptr );
        void Finalize();
        void WriteMemory ( const size_t aOffset, const  size_t aSize, const void *aData = nullptr ) const ;
        void* Map ( const size_t aOffset, const size_t aSize ) const;
        void Unmap() const;
        size_t GetSize() const;
    private:
        void Initialize ( const void *aData );
        const OpenGLRenderer& mOpenGLRenderer;
        uint32_t mBuffer{};
        size_t mSize{};
        size_t mUsage{};
        size_t mProperties{};
    };
}
#endif
