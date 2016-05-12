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
#ifndef AEONGAMES_RENDERER_H
#define AEONGAMES_RENDERER_H

#include "Platform.h"

namespace AeonGames
{
    class Renderer
    {
    public:
#if _WIN32
        virtual bool InitializeRenderingWindow ( HINSTANCE aInstance, HWND aHwnd ) = 0;
        virtual void FinalizeRenderingWindow ( HINSTANCE aInstance, HWND aHwnd ) = 0;
#else
        virtual bool InitializeRenderingWindow ( Display* aDisplay, Window aWindow ) = 0;
        virtual void FinalizeRenderingWindow ( Display* aDisplay, Window aWindow ) = 0;
#endif
    protected:
        virtual ~Renderer() = default;
    };
}
#endif
