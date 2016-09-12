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

#include "Configuration.h"
#include "aeongames/Utilities.h"
#include "aeongames/ProtoBufClasses.h"
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4251 )
#endif
#include <google/protobuf/text_format.h>
#include "configuration.pb.h"
#ifdef _MSC_VER
#pragma warning( pop )
#endif

namespace AeonGames
{
    Configuration::Configuration ( const std::string& aFilename )
        : mFilename ( aFilename )
    {
        ConfigurationBuffer configuration_buffer;
        if ( FileExists ( aFilename ) )
        {

        }
    }

    Configuration::~Configuration()
    {

    }
}
