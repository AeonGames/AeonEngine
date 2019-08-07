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
#include "OpenGLOverlay.h"
#include "aeongames/CRC.h"
#include "aeongames/AeonEngine.h"

namespace AeonGames
{
    OpenGLOverlay::OpenGLOverlay ( const OpenGLRenderer& aOpenGLRenderer, uint32_t aWidth, uint32_t aHeight ) :
        mOpenGLRenderer{aOpenGLRenderer},
        mImage{aWidth, aHeight, Image::Format::RGBA, Image::Type::UNSIGNED_BYTE}
    {
    }

    OpenGLOverlay::~OpenGLOverlay() = default;

    void OpenGLOverlay::Resize ( uint32_t aWidth, uint32_t aHeight )
    {
        mImage.Resize ( aWidth, aHeight );
    }

    void OpenGLOverlay::WritePixels ( int32_t aXOffset, int32_t aYOffset, uint32_t aWidth, uint32_t aHeight, const uint8_t* aPixels )
    {
        mImage.WritePixels ( aXOffset, aYOffset, aWidth, aHeight, Image::Format::RGBA, Image::Type::UNSIGNED_BYTE, aPixels );
    }
}
