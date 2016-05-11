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
#include "aeongames/LogLevel.h"
namespace AeonGames
{
    LogLevel::LogLevel ( Level aLevel )
    {
        mConsoleScreenBufferInfo;
        mConsoleHandle = GetStdHandle ( STD_OUTPUT_HANDLE );
        GetConsoleScreenBufferInfo ( mConsoleHandle, &mConsoleScreenBufferInfo );
        switch ( aLevel )
        {
        case Level::Info:
            SetConsoleTextAttribute ( mConsoleHandle, FOREGROUND_GREEN | FOREGROUND_INTENSITY );
            break;
        case Level::Warning:
            SetConsoleTextAttribute ( mConsoleHandle, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY );
            break;
        case Level::Error:
            SetConsoleTextAttribute ( mConsoleHandle, FOREGROUND_RED | FOREGROUND_INTENSITY );
            break;
        }
    }

    LogLevel::~LogLevel()
    {
        SetConsoleTextAttribute ( mConsoleHandle, mConsoleScreenBufferInfo.wAttributes );
    }

    std::ostream& operator<< ( std::ostream& os, const LogLevel& obj )
    {
        return os;
    }
}

