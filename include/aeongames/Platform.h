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
#ifndef AEONGAMES_PLATFORM_H
#define AEONGAMES_PLATFORM_H
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#ifndef DLL
#ifdef DLL_EXPORT
#define DLL __declspec( dllexport )
#else
#define DLL __declspec( dllimport )
#endif
#else
#define DLL
#endif
#endif
#endif

#if defined(MINGW32) || defined(MINGW64)
#define ENTRYPOINT __attribute__((force_align_arg_pointer))
#else
#define ENTRYPOINT
#endif