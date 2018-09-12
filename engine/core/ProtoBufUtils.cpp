/*
Copyright (C) 2016,2018 Rodrigo Jose Hernandez Cordoba

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
#include "aeongames/ProtoBufUtils.h"
#include "aeongames/CRC.h"
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4251 )
#endif
#include "reference.pb.h"
#ifdef _MSC_VER
#pragma warning( pop )
#endif

namespace AeonGames
{
    uint32_t GetReferenceBufferId ( const ReferenceBuffer& reference_buffer )
    {
        switch ( reference_buffer.reference_case() )
        {
        case ReferenceBuffer::kPath:
            return crc32i ( reference_buffer.path().c_str(), reference_buffer.path().size() );
        case ReferenceBuffer::kId:
            return reference_buffer.id();
        default:
            return 0;
        }
    }
}
