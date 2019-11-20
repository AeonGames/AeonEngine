/*
Copyright (C) 2019 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_OPENGLFRAMEBUFFER_H
#define AEONGAMES_OPENGLFRAMEBUFFER_H
#include <cstddef>
#include "OpenGLFunctions.h"
#include "aeongames/FrameBuffer.h"

namespace AeonGames
{
    class OpenGLRenderer;
    class OpenGLFrameBuffer : public FrameBuffer
    {
    public:
        OpenGLFrameBuffer ( uint32_t aPath = 0 );
        ~OpenGLFrameBuffer() final;
        void Load ( const FrameBufferBuffer& aFrameBufferBuffer ) final;
        void Unload () final;
        void Resize ( uint32_t aWidth, uint32_t aHeight ) final;
        void Bind() final;
        void Unbind() final;
        GLuint GetFBO() const; /// This is temporary
    private:
        GLuint mFBO {};
        GLuint mColorBuffer {};
        GLuint mRBO {};
    };
}
#endif
