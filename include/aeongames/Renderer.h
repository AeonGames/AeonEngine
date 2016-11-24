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
#include "aeongames/Memory.h"
#include <string>
#include <functional>
#include <utility>

namespace AeonGames
{
    class Mesh;
    class Program;
    class Texture;
    class Model;
    class Renderer
    {
    public:
        ///@name Rendering Functions
        ///@{
        virtual void BeginRender() const = 0;
        virtual void EndRender() const = 0;
        virtual void Render ( const std::shared_ptr<Model> aModel ) const = 0;
        ///@}
        ///@name Resource Allocation Functions
        ///@{
        ///@todo This should take a const std::shared_ptr
        virtual bool AllocateModelRenderData ( std::shared_ptr<Model> aModel ) = 0;
        ///@}
        ///@name Window Functions
        ///@{
        virtual bool AddRenderingWindow ( uintptr_t aWindowId ) = 0;
        virtual void RemoveRenderingWindow ( uintptr_t aWindowId ) = 0;
        virtual void Resize ( uintptr_t aWindowId, uint32_t aWidth, uint32_t aHeight ) const = 0;
        ///@}
        ///@name Matrix Functions
        ///@{
        virtual void SetViewMatrix ( const float aMatrix[16] ) = 0;
        virtual void SetProjectionMatrix ( const float aMatrix[16] ) = 0;
        virtual void SetModelMatrix ( const float aMatrix[16] ) = 0;
        ///@}
        virtual ~Renderer() = default;
    };

    /**@name Factory Functions */
    /*@{*/
    DLL std::unique_ptr<Renderer> GetRenderer ( const std::string& aIdentifier );
    /** Registers a renderer loader for a specific identifier.*/
    DLL bool RegisterRendererLoader ( const std::string& aIdentifier, std::function<std::unique_ptr<Renderer>() > aLoader );
    /** Unregisters a renderer loader for a specific identifier.*/
    DLL bool UnregisterRendererLoader ( const std::string& aIdentifier );
    /** Enumerates Renderer loader identifiers via an enumerator functor.*/
    DLL void EnumerateRendererLoaders ( std::function<bool ( const std::string& ) > aEnumerator );
    /*@}*/
}
#endif
