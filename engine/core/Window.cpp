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
#include <stdexcept>
#include <iostream>
#include "aeongames/Window.h"
#include "aeongames/LogLevel.h"
namespace AeonGames
{
    std::unordered_map<void*, Window*> Window::WindowMap{};
    Window* Window::GetWindowFromId ( void* aId )
    {
        auto i = WindowMap.find ( aId );
        if ( i != WindowMap.end() )
        {
            return ( *i ).second;
        }
        return nullptr;
    }
    void Window::SetWindowForId ( void* aId, Window* aWindow )
    {
        auto i = WindowMap.find ( aId );
        if ( i != WindowMap.end() )
        {
            throw std::runtime_error ( "Window Id already set." );
        }
        WindowMap.emplace ( std::pair<void*, Window*> {aId, aWindow} );
    }
    void Window::RemoveWindowForId ( void* aId )
    {
        auto i = WindowMap.find ( aId );
        if ( i != WindowMap.end() )
        {
            WindowMap.erase ( aId );
        }
        else
        {
            std::cout << LogLevel::Warning << "Window not found." << std::endl;
        }
    }
    Window::~Window() = default;
}
