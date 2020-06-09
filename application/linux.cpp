/*
Copyright (C) 2016,2018-2020 Rodrigo Jose Hernandez Cordoba

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
#ifdef __unix__
#include "aeongames/AeonEngine.h"
#include "aeongames/Renderer.h"
#include "aeongames/StringId.h"
#include "aeongames/LogLevel.h"
#include "aeongames/Window.h"
#include "aeongames/Utilities.h"
#include "aeongames/LogLevel.h"
#include <cassert>
#include <iostream>
#include <cstdint>
#include <vector>
#include <string>
#include <stdexcept>
#include <cassert>
#include <X11/Xlib.h>

int Main ( int argc, char *argv[] );

int ENTRYPOINT main ( int argc, char *argv[] )
{
    XSetErrorHandler ( [] ( Display * display, XErrorEvent * error_event ) -> int
    {
        char error_string[1024];
        XGetErrorText ( display, error_event->error_code, error_string, 1024 );
        std::cout << AeonGames::LogLevel::Error << "Error Code " << static_cast<int> ( error_event->error_code ) << " " << error_string << std::endl;
        return 0;
    } );
    return Main ( argc, argv );
}
#endif