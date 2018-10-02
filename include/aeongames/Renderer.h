/*
Copyright (C) 2016-2018 Rodrigo Jose Hernandez Cordoba

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

#include <string>
//#include <functional>
//#include <utility>
#include <memory>
#include "Platform.h"

namespace AeonGames
{
    class Mesh;
    class Pipeline;
    class Material;
    class Window;
    class Renderer
    {
    public:
        ///@name Window Factory
        ///@{
        /** Creates a Window object that acts as a wrapper for the Window Id provided.
            This factory function is used when rendering is to be done in a previously
            constructed window.
            @param aWindowId Implementation depended window handle.
            On Windows, a HWND, On X11 a Window handle.
            @return A unique pointer to a Window object referencing the specific renderer implementation.
        */
        virtual std::unique_ptr<Window> CreateWindowProxy ( void* aWindowId ) const = 0;
#if 0
        /*
        NOTE: The idea here is not having to deal with manual window creation and management if
        all you want to do is a single window/fullscreen game or application, but since
        I am not doing that just yet, I'll KISS and comment this out for future reference,
        after all, it may YAGNI.
        */
        /** Creates a standalone Window object based on the system the engine is running on.
            @return A unique pointer to a Window object referencing the specific renderer implementation.
        */
        virtual std::unique_ptr<Window> CreateWindowInstance() = 0;
#endif
        ///@}
        virtual void LoadRenderMesh ( const Mesh& aMesh ) const = 0;
        virtual void UnloadRenderMesh ( const Mesh& aMesh ) const = 0;
        virtual void LoadRenderPipeline ( const Pipeline& aPipeline ) const = 0;
        virtual void UnloadRenderPipeline ( const Pipeline& aPipeline ) const = 0;
        virtual void LoadRenderMaterial ( const Material& aMaterial ) const = 0;
        virtual void UnloadRenderMaterial ( const Material& aMaterial ) const = 0;
        DLL virtual ~Renderer() = 0;
    };
}
#endif
