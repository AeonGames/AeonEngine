/*
Copyright (C) 2017-2018 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_PIPELINE_H
#define AEONGAMES_PIPELINE_H

#include <cstdint>
#include <string>
#include <vector>
#include "aeongames/Platform.h"
#include "aeongames/Material.h"

namespace AeonGames
{
    class MaterialBuffer;
    class PipelineBuffer;
    class Pipeline
    {
    public:
        DLL virtual ~Pipeline() = 0;
        virtual void Load ( const std::string& aFilename ) = 0;
        virtual void Load ( uint32_t aId ) = 0;
        virtual void Load ( const void* aBuffer, size_t aBufferSize ) = 0;
        virtual void Load ( const PipelineBuffer& aPipelineBuffer ) = 0;
        virtual void Unload() = 0;
        virtual const Material& GetDefaultMaterial() const = 0;
    };
}
#endif
