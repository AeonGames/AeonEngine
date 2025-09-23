/*
Copyright (C) 2016,2017,2021,2025 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_PROTOBUFCLASSES_H
#define AEONGAMES_PROTOBUFCLASSES_H
#ifdef _WIN32
#ifndef DLL_PROTOBUF
#ifdef ProtoBufClasses_EXPORTS
#define DLL_PROTOBUF __declspec( dllexport )
#else
#if defined(__MINGW32__) || defined(__MINGW64__)
#define DLL_PROTOBUF
#else
#define DLL_PROTOBUF __declspec( dllimport )
#endif
#endif
#endif
#elif defined(__unix__) || defined(__APPLE__)
#ifndef DLL_PROTOBUF
#define DLL_PROTOBUF __attribute__((visibility("default")))
#endif
#endif
#endif
