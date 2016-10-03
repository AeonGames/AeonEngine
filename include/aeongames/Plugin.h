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
#ifndef AEONGAMES_PLUGIN_H
#define AEONGAMES_PLUGIN_H
#ifdef __cplusplus
extern "C"
{
#endif
typedef bool ( *StartUpPtr ) ( void );
typedef void ( *ShutDownPtr ) ( void );
typedef struct
{
    const char* Name;
    const char* Description;
    StartUpPtr StartUp;
    ShutDownPtr ShutDown;
} PluginModuleInterface;
#ifdef __cplusplus
}
#endif
#endif
