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
#include <Windows.h>
#include <windowsx.h>
#include <cassert>
#include <cstdint>

namespace AeonGames
{
    class AeonEngine;
    class GameWindow
    {
    public:
        GameWindow ( AeonEngine& aEngine, LONG aWidth = 800, LONG aHeight = 600 );
        ~GameWindow();
        int Run();
    private:
        static LRESULT CALLBACK WindowProc ( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
        static ATOM mClassAtom;
        void Initialize ( HINSTANCE hInstance, LONG aWidth, LONG aHeight );
        void Finalize();
        static void Register ( HINSTANCE hInstance );
        LRESULT OnSize ( WPARAM type, WORD newwidth, WORD newheight );
        LRESULT OnPaint();
        void RenderLoop();
        HWND mWindowHandle = nullptr;
        AeonEngine& mAeonEngine;
    };
}