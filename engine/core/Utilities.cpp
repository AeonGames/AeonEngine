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
#include <algorithm>
#include <iostream>
#include <cstring>
#include "aeongames/Platform.h"
#include "aeongames/Utilities.h"
namespace AeonGames
{
    const std::string GetFileExtension ( const std::string& aFilePath )
    {
        ///@todo Use splitpath instead?
        std::size_t extpos = aFilePath.find_last_of ( "/\\" );
        extpos = ( extpos == std::string::npos ) ? 0 : extpos;
        extpos = aFilePath.find_first_of ( '.', extpos );
        if ( extpos != std::string::npos )
        {
            std::string extension = aFilePath.substr ( extpos );
            std::transform ( extension.begin(), extension.end(), extension.begin(), tolower );
            return extension;
        }
        return std::string();
    }

    bool FileExists ( const std::string& aFilePath )
    {
        struct stat stat_buffer {};
        return ( ::stat ( aFilePath.c_str(), &stat_buffer ) == 0 ) ? true : false;
    }

    OptionHandler::OptionHandler ( const char aShortOption, const char* aLongOption, void ( *aHandler ) ( const char*, void* ), void* aUserData ) :
        mHandler{aHandler},
        mShortOption{aShortOption},
        mLongOption{aLongOption},
        mUserData{aUserData} {};
    OptionHandler::~OptionHandler() = default;
    const char OptionHandler::GetShortOption() const
    {
        return mShortOption;
    }
    const char* OptionHandler::GetLongOption() const
    {
        return mLongOption;
    }
    void* OptionHandler::GetUserData() const
    {
        return mUserData;
    }
    void OptionHandler::operator() ( const char* aArgument, void* aUserData ) const
    {
        mHandler ( aArgument, aUserData );
    }

    void ProcessOpts ( int argc, char *argv[], const OptionHandler* aOptionHandler, size_t aOptionHandlerCount )
    {
        for ( int i = 0; i < argc; ++i )
        {
            if ( argv[i][0] == '-' )
            {
                const OptionHandler* handler = nullptr;
                if ( argv[i][1] == '-' )
                {
                    // Long Option
                    handler = std::find_if ( aOptionHandler, aOptionHandler + aOptionHandlerCount,
                                             [argv, i] ( const OptionHandler & handler )
                    {
                        return strcmp ( &argv[i][2], handler.GetLongOption() ) == 0;
                    } );
                }
                else
                {
                    // Short Option
                    handler = std::find_if ( aOptionHandler, aOptionHandler + aOptionHandlerCount,
                                             [argv, i] ( const OptionHandler & handler )
                    {
                        return argv[i][1] == handler.GetShortOption();
                    } );
                }
                if ( handler && handler != ( aOptionHandler + aOptionHandlerCount ) )
                {
                    char* argument = ( ( i + 1 ) < argc ) ? ( argv[i + 1][0] != '-' ) ? argv[i + 1] : nullptr : nullptr;
                    ( *handler ) ( argument, handler->GetUserData() );
                }
            }
        }
    }
}
