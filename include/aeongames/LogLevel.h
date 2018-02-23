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
#ifndef AEONGAMES_LOGLEVEL_H
#define AEONGAMES_LOGLEVEL_H
#include "Platform.h"
#include <iostream>

namespace AeonGames
{
    class LogLevel
    {
    public:
        enum class Level
        {
            Info = 0,
            Warning,
            Error
        };
        DLL explicit LogLevel ( Level aLevel );
        DLL ~LogLevel();
        DLL friend std::ostream& operator<< ( std::ostream& os, const LogLevel& obj );
    private:
#ifdef _WIN32
        CONSOLE_SCREEN_BUFFER_INFO mConsoleScreenBufferInfo {};
        HANDLE mConsoleHandle = nullptr;
#endif
    };
}
#endif
