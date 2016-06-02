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

#ifndef AEONGAMES_GAMEWINDOW_H
#define AEONGAMES_GAMEWINDOW_H

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <unistd.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>

#include "aeongames/AeonEngine.h"

namespace AeonGames
{
    class GameWindow
    {
    public:
        GameWindow ( AeonEngine& aAeonEngine );
        ~GameWindow();
        int Run();
    private:
        void Initialize();
        void Finalize();
        AeonEngine& mAeonEngine;
        Display* mDisplay = nullptr;
        Window mWindow;
        Atom mWMDeleteWindow;
    };
}
#endif