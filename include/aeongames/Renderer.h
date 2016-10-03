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
#include <memory>
#include <string>
#include <functional>
namespace AeonGames
{
    class GameWindow;
    class Mesh;
    class Program;
    class Renderer
    {
    public:
        ///@name Rendering Functions
        ///@{
        virtual void BeginRender() const = 0;
        virtual void EndRender() const = 0;
        virtual void Render ( const std::shared_ptr<Mesh>& aMesh, const std::shared_ptr<Program>& aProgram ) const = 0;
        ///@}
        ///@name Resource Access Functions
        ///@{
        virtual std::shared_ptr<Mesh> GetMesh ( const std::string& aFilename ) const = 0;
        virtual std::shared_ptr<Program> GetProgram ( const std::string& aFilename ) const = 0;
        ///@}
        ///@name Window Functions
        ///@{
        virtual bool RegisterRenderingWindow ( uintptr_t aWindowId ) = 0;
        virtual void UnregisterRenderingWindow ( uintptr_t aWindowId ) = 0;
        virtual void Resize ( uintptr_t aWindowId, uint32_t aWidth, uint32_t aHeight ) const = 0;
        ///@}
        ///@name Matrix Functions
        ///@{
        virtual void SetViewMatrix ( const float aMatrix[16] ) = 0;
        virtual void SetProjectionMatrix ( const float aMatrix[16] ) = 0;
        virtual void SetModelMatrix ( const float aMatrix[16] ) = 0;
        ///@}
    protected:
        virtual ~Renderer() = default;
    };

    /** Factory Function */
    DLL std::shared_ptr<Renderer> GetImage ( const std::string& aFilename );
    /** Registers an image loader for a filename extension.*/
    DLL bool RegisterImageLoader ( const std::string& aExt, std::function<std::shared_ptr<Renderer> ( const std::string& ) > aLoader );
    /** Unregisters an image loader for a filename extension.*/
    DLL bool UnregisterImageLoader ( const std::string& aExt );
}
#endif
