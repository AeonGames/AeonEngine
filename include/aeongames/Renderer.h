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
#ifndef AEONGAMES_RENDERER_H
#define AEONGAMES_RENDERER_H

#include <string>
#include <memory>
#include <functional>
#include "Platform.h"

namespace AeonGames
{
    class StringId;
    class UniformBuffer;
    class Image;
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

        /** Creates a standalone Window object based on the system the engine is running on.
            @return A unique pointer to a Window object referencing the specific renderer implementation.
        */
        virtual std::unique_ptr<Window> CreateWindowInstance ( int32_t aX, int32_t aY, uint32_t aWidth, uint32_t aHeight, bool aFullScreen ) const = 0;

        ///@}
        virtual std::unique_ptr<Mesh> CreateMesh ( uint32_t aPath = 0 ) const = 0;
        virtual std::unique_ptr<Pipeline> CreatePipeline ( uint32_t aPath = 0 ) const = 0;
        virtual std::unique_ptr<Material> CreateMaterial ( uint32_t aPath = 0 ) const = 0;
        virtual std::unique_ptr<Image> CreateImage ( uint32_t aPath = 0 ) const = 0;
        virtual std::unique_ptr<UniformBuffer> CreateUniformBuffer ( size_t aSize, const void* aData = nullptr ) const = 0;
        DLL virtual ~Renderer() = 0;
    };
    /**@name Factory Functions */
    /*@{*/
    DLL std::unique_ptr<Renderer> ConstructRenderer ( uint32_t aIdentifier );
    DLL std::unique_ptr<Renderer> ConstructRenderer ( const std::string& aIdentifier );
    DLL std::unique_ptr<Renderer> ConstructRenderer ( const StringId& aIdentifier );
    /** Registers a Renderer loader for a specific identifier.*/
    DLL bool RegisterRendererConstructor ( const StringId& aIdentifier, const std::function<std::unique_ptr<Renderer>() >& aConstructor );
    /** Unregisters a Renderer loader for a specific identifier.*/
    DLL bool UnregisterRendererConstructor ( const StringId& aIdentifier );
    /** Enumerates Renderer loader identifiers via an enumerator functor.*/
    DLL void EnumerateRendererConstructors ( const std::function<bool ( const StringId& ) >& aEnumerator );
    /*@}*/
}
#endif
