/*
Copyright (C) 2020 Rodrigo Jose Hernandez Cordoba

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
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <delayimp.h>
#include <cstring>

static FARPROC WINAPI load_exe_hook ( unsigned int event, DelayLoadInfo* info )
{
    if ( event != dliNotePreLoadLibrary )
    {
        return nullptr;
    }

    if ( strcmp ( "node.exe", info->szDll ) != 0 )
    {
        return nullptr;
    }
    return reinterpret_cast<FARPROC> ( GetModuleHandle ( nullptr ) );
}

decltype ( __pfnDliNotifyHook2 ) __pfnDliNotifyHook2 = load_exe_hook;

#endif
