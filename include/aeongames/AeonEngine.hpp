/*
Copyright (C) 2016-2021,2025,2026 Rodrigo Jose Hernandez Cordoba

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
#include "Platform.hpp"

namespace AeonGames
{
    class Renderer;
    class InputSystem;
    /** @brief Initialize the global engine environment.
     *  @param argc Number of command-line arguments.
     *  @param argv Array of command-line argument strings.
     *  @return true if initialization succeeded, false otherwise.
     */
    DLL bool InitializeGlobalEnvironment ( int argc = 0, char *argv[] = nullptr );
    /** @brief Shut down the global engine environment and release resources. */
    DLL void FinalizeGlobalEnvironment();
    /** @brief Get the list of resource search paths.
     *  @return A vector of directory paths used to locate resources.
     */
    DLL std::vector<std::string> GetResourcePath();
    /** @brief Set the list of resource search paths.
     *  @param aPath A vector of directory paths to use for resource lookup.
     */
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
    /** @name Global Input System
     * Provides game code and components with access to the active InputSystem
     * without requiring them to know about the Window class.
     **/
    ///@{
    /** @brief Set the global InputSystem pointer.
     *  @param aInputSystem Pointer to the active InputSystem, or nullptr to clear.
     */
    DLL void SetInputSystem ( InputSystem* aInputSystem );
    /** @brief Get the global InputSystem pointer.
     *  @return Pointer to the active InputSystem, or nullptr if none is set.
     */
    DLL InputSystem* GetInputSystem();
    ///@}
}
#endif
