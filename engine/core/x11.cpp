/*
Copyright (C) 2017-2020 Rodrigo Jose Hernandez Cordoba

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
#include <ctime>
#include <ratio>
#include <chrono>
#include "aeongames/Platform.h"
#include "aeongames/Window.h"
namespace AeonGames
{
    Window::Window ( int32_t aX, int32_t aY, uint32_t aWidth, uint32_t aHeight, bool aFullScreen ) :
        mAspectRatio { static_cast<float> ( aWidth ) / static_cast<float> ( aHeight ) } {}
    DLL Window::~Window() = default;
    void Window::Run ( Scene& aScene )
    {
    }
}
#endif
