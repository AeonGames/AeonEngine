/*
Copyright (C) 2016,2019,2021 Rodrigo Jose Hernandez Cordoba

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

#ifndef AEONGAMES_CONFIGURATION_H
#define AEONGAMES_CONFIGURATION_H

#include <string>
#include <vector>
#include "aeongames/ProtoBufClasses.h"
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : PROTOBUF_WARNINGS )
#endif
#include "configuration.pb.h"
#ifdef _MSC_VER
#pragma warning( pop )
#endif

namespace AeonGames
{
    class Configuration
    {
    public:
        Configuration ( const std::string& aFilename );
        ~Configuration();
    private:
        std::string mFilename;
        ConfigurationMsg mConfigurationMsg;
    };
}
#endif
