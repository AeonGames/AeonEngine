/*
Copyright (C) 2016-2021 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_AEONENGINE_H
#define AEONGAMES_AEONENGINE_H
#include <memory>
#include <string>
#include <vector>
#include <functional>
#include "Platform.h"

namespace AeonGames
{
    class Renderer;
    DLL bool InitializeGlobalEnvironment ( int argc = 0, char *argv[] = nullptr );
    DLL void FinalizeGlobalEnvironment();
    DLL std::vector<std::string> GetResourcePath();
    DLL void SetResourcePath ( const std::vector<std::string>& aPath );
    /*! Returns the resource size referenced by its CRC value. */
    DLL size_t GetResourceSize ( uint32_t crc );
    /*! Returns the resource size referenced by its file name. */
    DLL size_t GetResourceSize ( const std::string& aFileName );
    /*! Returns the resource path referenced by its id. */
    DLL std::string GetResourcePath ( uint32_t crc );
    /*! Loads a specific resource referenced by its CRC into the provided buffer. */
    DLL void LoadResource ( uint32_t crc, void* buffer, size_t buffer_size );
    /*! Loads a specific resource referenced by its path into the provided buffer. */
    DLL void LoadResource ( const std::string& aFileName, void* buffer, size_t buffer_size );
    /** @name Global Renderer
     * The idea of having a global renderer as opposed to being able to construct
     * multiple renderers at will as it used to be is to simplify renderer resource
     * management, if generic classes can access the current renderer at creation,
     * they can build their renderable counterparts right away, this way the end user
     * needs not worry to initialize render resources and lazy loading of render resources
     * at the render loop is avoided at runtime.
     **/
    ///@{
    /** Retrieve a pointer to the current global renderer, may be null if not set.
     * @return Pointer to the current global renderer.
     */
    DLL Renderer* GetRenderer();
    /** Builds and stores a renderer object given its implementation identifier.
     * @param aIdentifier Implementation identifier for the renderer to construct,
     * usually retrived with EnumerateRendererConstructors.
     * @return A Pointer to the newly constructed renderer.
     * @note Will throw a runtime_error exception if the renderer is already set.
     * This may change in the future if a reason to reset or change the renderer at runtime arises.
    */
    DLL const Renderer* SetRenderer ( const std::string& aIdentifier );
    ///@}
}
#endif
