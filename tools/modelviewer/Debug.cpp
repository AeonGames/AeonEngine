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
#ifdef _MSC_VER

void GetProcName ( HMODULE hModule, void* address )
{
    if ( !hModule )
    {
        return;
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

    for ( DWORD i = 0; i < EXPORT_DIRECTORY->NumberOfNames; ++i )
    {
        std::cout <<
                  "Function Name " <<
                  reinterpret_cast<char*> ( reinterpret_cast<uint8_t*> ( hModule ) + names[i] ) <<
                  " Function Ordinal " <<
                  name_ordinals[i] <<
                  " Function Address " <<
                  reinterpret_cast<void*> ( reinterpret_cast<uint8_t*> ( hModule ) + functions[name_ordinals[i]] ) <<
                  std::endl;
    }
};

BOOL WINAPI GetModuleByRetAddr ( PBYTE Ret_Addr, PCHAR Module_Name )
{
    MODULEENTRY32 M { sizeof ( M ) };
    HANDLE  hSnapshot;
    Module_Name[0] = 0;
    hSnapshot = CreateToolhelp32Snapshot ( TH32CS_SNAPMODULE, 0 );

    if ( ( hSnapshot != INVALID_HANDLE_VALUE ) &&
         Module32First ( hSnapshot, &M ) )
    {
        do
        {
            if ( DWORD ( Ret_Addr - M.modBaseAddr ) < M.modBaseSize )
            {
                GetProcName ( M.hModule, Ret_Addr );
                lstrcpyn ( Module_Name, M.szExePath, MAX_PATH );
                break;
            }
        }
        while ( Module32Next ( hSnapshot, &M ) );
    }
    CloseHandle ( hSnapshot );
    return !!Module_Name[0];
}

void GetCallStack ( int test, std::vector<std::string>& aFunctionNames )
{
    CHAR    Module_Name[MAX_PATH];

    typedef struct STACK
    {
        STACK * Ebp;
        PBYTE   Ret_Addr;
    } STACK, *PSTACK;

    STACK   Stack = { 0, 0 };
    PSTACK  Ebp;

    Ebp = ( ( PSTACK ) &test ) - 1;

    if ( !IsBadReadPtr ( Ebp, sizeof ( PSTACK ) ) )
    {
        Ebp = Ebp->Ebp;
    }
    for ( int Ret_Addr_I = 0;
          ( Ret_Addr_I < CALL_TRACE_MAX ) && !IsBadReadPtr ( Ebp, sizeof ( PSTACK ) ) && !IsBadCodePtr ( FARPROC ( Ebp->Ret_Addr ) );
          Ret_Addr_I++, Ebp = Ebp->Ebp )
    {
        if ( GetModuleByRetAddr ( Ebp->Ret_Addr, Module_Name ) )
        {
            aFunctionNames.push_back ( Module_Name );
        }
    }
}
#endif