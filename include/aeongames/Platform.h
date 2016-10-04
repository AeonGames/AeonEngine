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
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#ifndef DLL
#ifdef AeonEngine_EXPORTS
#define DLL __declspec( dllexport )
#else
#define DLL __declspec( dllimport )
#endif
#ifndef PLUGIN
#define PLUGIN __declspec( dllexport )
#endif
#endif
#elif defined(__unix__)
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>
#ifndef DLL
#define DLL
#endif
#ifndef PLUGIN
#define PLUGIN
#endif
#endif

#if defined(__GNUC__)&&((__GNUC__ > 4 || (__GNUC__ == 4 && (__GNUC_MINOR__ > 8 || (__GNUC_MINOR__ == 8 && __GNUC_PATCHLEVEL__ > 4))))||defined (__i386__))
#define ENTRYPOINT __attribute__((force_align_arg_pointer))
#else
#define ENTRYPOINT
#endif

#endif
