/*
Copyright (C) 2016-2017 Rodrigo Jose Hernandez Cordoba

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
    class Window;
    class Mesh;
    class Pipeline;
    class Texture;
    class Model;
    class RenderModel;
    class ModelInstance;
    class Matrix4x4;
    class Transform;
    class Scene;
    class Renderer
    {
    public:
        virtual void Render ( const std::shared_ptr<const Scene>& aScene ) const = 0;
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
        ///@name Matrix Functions
        ///@{
        virtual void SetViewTransform ( const Transform aTransform ) = 0;
        virtual void SetProjectionMatrix ( const Matrix4x4& aMatrix ) = 0;
        ///@}
        virtual ~Renderer() = default;
    protected:
        DLL void SetRenderModel ( const std::shared_ptr<const Model>& aModel, std::unique_ptr<RenderModel> aRenderModel ) const;
        DLL const std::unique_ptr<RenderModel>& GetRenderModel ( const std::shared_ptr<const Model>& aModel ) const;
    };

    /**@name Factory Functions */
    /*@{*/
    DLL std::shared_ptr<Renderer> GetRenderer ( const std::string& aIdentifier );
    /** Registers a renderer loader for a specific identifier.*/
    DLL bool RegisterRendererLoader ( const std::string& aIdentifier, std::function<std::shared_ptr<Renderer>() > aLoader );
    /** Unregisters a renderer loader for a specific identifier.*/
    DLL bool UnregisterRendererLoader ( const std::string& aIdentifier );
    /** Enumerates Renderer loader identifiers via an enumerator functor.*/
    DLL void EnumerateRendererLoaders ( std::function<bool ( const std::string& ) > aEnumerator );
    /*@}*/
}
#endif
