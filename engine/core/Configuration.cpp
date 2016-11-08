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

#include <fstream>
#include <iostream>
#include <sstream>
#include <exception>
#include "Configuration.h"
#include "aeongames/Utilities.h"
#include "ProtoBufHelpers.h"

namespace AeonGames
{

    Configuration::Configuration ( const std::string& aFilename )
        : mFilename ( aFilename )
    {
        try
        {
            mConfigurationBuffer =
                LoadProtoBufObject<ConfigurationBuffer> ( aFilename, "AEONCFG" );
        }
        catch ( std::runtime_error e )
        {
            std::cout << "Warning: " << e.what() << std::endl;
        }
    }

    Configuration::~Configuration()
    {

    }
}
