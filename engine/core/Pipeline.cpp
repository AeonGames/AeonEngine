/*
Copyright (C) 2016-2019 Rodrigo Jose Hernandez Cordoba

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
#include "aeongames/AeonEngine.h"
#include "aeongames/CRC.h"
#include "aeongames/ProtoBufClasses.h"
#include "ProtoBufHelpers.h"
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : PROTOBUF_WARNINGS )
#endif
#include "pipeline.pb.h"
#include "property.pb.h"
#ifdef _MSC_VER
#pragma warning( pop )
#endif
#include "aeongames/Pipeline.h"

namespace AeonGames
{
    Pipeline::~Pipeline()
        = default;
    void Pipeline::Load ( const std::string& aFilename )
    {
        Load ( crc32i ( aFilename.c_str(), aFilename.size() ) );
    }
    void Pipeline::Load ( uint32_t aId )
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
    void Pipeline::Load ( const void* aBuffer, size_t aBufferSize )
    {
        static PipelineBuffer pipeline_buffer;
        LoadProtoBufObject ( pipeline_buffer, aBuffer, aBufferSize, "AEONPLN" );
        Load ( pipeline_buffer );
        pipeline_buffer.Clear();
    }
}
