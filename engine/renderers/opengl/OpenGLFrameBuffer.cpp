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
#include "aeongames/ProtoBufClasses.h"
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : PROTOBUF_WARNINGS )
#endif
#include "framebuffer.pb.h"
#ifdef _MSC_VER
#pragma warning( pop )
#endif
#include "ProtoBufHelpers.h"
#include "OpenGLFrameBuffer.h"
#include "aeongames/CRC.h"
#include "aeongames/AeonEngine.h"

namespace AeonGames
{
    OpenGLFrameBuffer::OpenGLFrameBuffer ( uint32_t aPath )
    {
        if ( aPath )
        {
            Load ( aPath );
        }
    }

    OpenGLFrameBuffer::~OpenGLFrameBuffer()
    {
        Unload();
    }
    void OpenGLFrameBuffer::Load ( const std::string& aFilename )
    {
        Load ( crc32i ( aFilename.c_str(), aFilename.size() ) );
    }
    void OpenGLFrameBuffer::Load ( uint32_t aId )
    {
        std::vector<uint8_t> buffer ( GetResourceSize ( aId ), 0 );
        LoadResource ( aId, buffer.data(), buffer.size() );
        try
        {
            Load ( buffer.data(), buffer.size() );
        }
        catch ( ... )
        {
            Unload();
            throw;
        }
    }
    void OpenGLFrameBuffer::Load ( const void* aBuffer, size_t aBufferSize )
    {
        static FrameBufferBuffer framebuffer_buffer;
        LoadProtoBufObject ( framebuffer_buffer, aBuffer, aBufferSize, "AEONFBR" );
        Load ( framebuffer_buffer );
        framebuffer_buffer.Clear();
    }
    void OpenGLFrameBuffer::Load ( const FrameBufferBuffer& aFrameBufferBuffer )
    {
        ///@todo Write Me
    }
    void OpenGLFrameBuffer::Unload ()
    {
        ///@todo Write Me
    }
}
