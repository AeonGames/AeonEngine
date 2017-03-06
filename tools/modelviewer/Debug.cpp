/*
Copyright (C) 2017 Rodrigo Jose Hernandez Cordoba

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

#include "Debug.h"
#include <iostream>
#include <vector>
#include <string>
#include <tuple>
#include <algorithm>
#ifdef _MSC_VER

LPCSTR WINAPI GetProcName ( _In_ HMODULE hModule, _In_ FARPROC lpAddress )
{
    if ( !hModule )
    {
        return nullptr;
    }
    IMAGE_NT_HEADERS *PE_HEADERS;
    IMAGE_EXPORT_DIRECTORY *EXPORT_DIRECTORY;
    DWORD PEOFFSET = 0;
    int nIndex = 0;
    int nLookupIndex = 0;
    PEOFFSET = *reinterpret_cast<DWORD*> ( reinterpret_cast<uint8_t*> ( hModule ) + 0x3c );
    PE_HEADERS = ( IMAGE_NT_HEADERS* ) ( reinterpret_cast<uint8_t*> ( hModule ) + PEOFFSET );
    EXPORT_DIRECTORY = ( IMAGE_EXPORT_DIRECTORY* ) ( PE_HEADERS->OptionalHeader.ImageBase + PE_HEADERS->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress );
    uint32_t* names = reinterpret_cast<uint32_t*> ( reinterpret_cast<uint8_t*> ( hModule ) + EXPORT_DIRECTORY->AddressOfNames );
    uint16_t* name_ordinals = reinterpret_cast<uint16_t*> ( reinterpret_cast<uint8_t*> ( hModule ) + EXPORT_DIRECTORY->AddressOfNameOrdinals );
    uint32_t* functions = reinterpret_cast<uint32_t*> ( reinterpret_cast<uint8_t*> ( hModule ) + EXPORT_DIRECTORY->AddressOfFunctions );

    std::vector<std::tuple<FARPROC, LPCSTR>> function_vector;
    function_vector.reserve ( EXPORT_DIRECTORY->NumberOfNames );
    for ( DWORD i = 0; i < EXPORT_DIRECTORY->NumberOfNames; ++i )
    {
        function_vector.emplace_back (
            reinterpret_cast<FARPROC> ( reinterpret_cast<uint8_t*> ( hModule ) + functions[name_ordinals[i]] ),
            reinterpret_cast<LPCSTR> ( reinterpret_cast<uint8_t*> ( hModule ) + names[i] )
        );
        std::cout <<
                  "Function Name " <<
                  reinterpret_cast<char*> ( reinterpret_cast<uint8_t*> ( hModule ) + names[i] ) <<
                  " Function Ordinal " <<
                  name_ordinals[i] <<
                  " Function Address " <<
                  reinterpret_cast<void*> ( reinterpret_cast<uint8_t*> ( hModule ) + functions[name_ordinals[i]] ) <<
                  std::endl;
    }
    std::sort ( function_vector.begin(), function_vector.end(),
                [] ( const std::tuple<FARPROC, std::string>& a, const std::tuple<FARPROC, std::string>& b ) -> bool
    {
        return std::get<0> ( a ) < std::get<0> ( b );
    } );
    auto function = std::lower_bound ( function_vector.begin(), function_vector.end(), lpAddress,
                                       [] ( const std::tuple<FARPROC, std::string>& a, const FARPROC b ) -> bool
    {
        return std::get<0> ( a ) < b;
    } );
    if ( function != function_vector.end() )
    {
        return std::get<1> ( *function );
    }
    return nullptr;
};

BOOL WINAPI GetModuleEntryByAddr ( _In_ FARPROC lpAddress, _In_ PMODULEENTRY32 lpModuleEntry )
{
    if ( !lpModuleEntry )
    {
        return FALSE;
    }
    memset ( lpModuleEntry, 0, sizeof ( MODULEENTRY32 ) );
    lpModuleEntry->dwSize = sizeof ( MODULEENTRY32 );
    HANDLE  hSnapshot;
    hSnapshot = CreateToolhelp32Snapshot ( TH32CS_SNAPMODULE, 0 );

    if ( ( hSnapshot != INVALID_HANDLE_VALUE ) &&
         Module32First ( hSnapshot, lpModuleEntry ) )
    {
        do
        {
            if ( DWORD ( PBYTE ( lpAddress ) - lpModuleEntry->modBaseAddr ) < lpModuleEntry->modBaseSize )
            {
                GetProcName ( lpModuleEntry->hModule, lpAddress );
                CloseHandle ( hSnapshot );
                return TRUE;
            }
        }
        while ( Module32Next ( hSnapshot, lpModuleEntry ) );
    }
    CloseHandle ( hSnapshot );
    return FALSE;
}

FARPROC WINAPI GetCallStackReturnAddress ( _In_ size_t index, _In_ PMODULEENTRY32 lpModuleEntry )
{
    /** @todo Find out how to get the same info with StackWalk64. */
    typedef struct STACK
    {
        STACK * Ebp;
        FARPROC   Ret_Addr;
    } STACK, *PSTACK;

    STACK   Stack = { 0, 0 };
    PSTACK  Ebp;

    Ebp = ( ( PSTACK ) &index ) - 1;

    if ( !IsBadReadPtr ( Ebp, sizeof ( PSTACK ) ) )
    {
        Ebp = Ebp->Ebp;
    }
    for ( size_t i = 0; i < index; ++i )
    {
        if ( !IsBadReadPtr ( Ebp, sizeof ( PSTACK ) ) && !IsBadCodePtr ( FARPROC ( Ebp->Ret_Addr ) ) )
        {
            Ebp = Ebp->Ebp;
        }
        else
        {
            return nullptr;
        }
    }
    return Ebp->Ret_Addr;
}
#endif