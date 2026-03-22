/*
Copyright (C) 2016,2018,2025,2026 Rodrigo Jose Hernandez Cordoba

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
/** @file Plugin.hpp
 *  @brief Defines the plugin module interface for dynamically loaded plugins.
 */
#ifndef AEONGAMES_PLUGIN_H
#define AEONGAMES_PLUGIN_H
#ifdef __cplusplus
extern "C"
{
#endif
/** @brief Function pointer type for plugin startup. @return true on success. */
using StartUpPtr = bool ( * ) ();
/** @brief Function pointer type for plugin shutdown. */
using ShutDownPtr = void ( * ) ();
/** @brief Interface that every loadable plugin module must expose.
 *
 *  Each shared library plugin provides a static instance of this struct
 *  so the engine can query its name, description, and lifecycle callbacks.
 */
struct PluginModuleInterface
{
    const char* Name;           /**< @brief Human-readable plugin name. */
    const char* Description;    /**< @brief Short description of the plugin. */
    StartUpPtr StartUp;         /**< @brief Called to initialize the plugin. */
    ShutDownPtr ShutDown;       /**< @brief Called to tear down the plugin. */
};
#ifdef __cplusplus
}
#endif
#endif
