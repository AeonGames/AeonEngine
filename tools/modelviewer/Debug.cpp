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
#include <unordered_map>
#include <string>
#include <tuple>
#include <algorithm>
#ifdef _MSC_VER

#pragma comment(lib, "Dbghelp.lib")

static std::vector<MODULEENTRY32> ModuleTable;

void InitializeModuleTable()
{
    MODULEENTRY32 module_entry;
    memset ( &module_entry, 0, sizeof ( MODULEENTRY32 ) );
    module_entry.dwSize = sizeof ( MODULEENTRY32 );
    HANDLE  hSnapshot;
    hSnapshot = CreateToolhelp32Snapshot ( TH32CS_SNAPMODULE, 0 );
    size_t module_count = 0;
    if ( ( hSnapshot != INVALID_HANDLE_VALUE ) &&
         Module32First ( hSnapshot, &module_entry ) )
    {
        do
        {
            module_count++;
        }
        while ( Module32Next ( hSnapshot, &module_entry ) );
    }
    ModuleTable.reserve ( module_count );
    if ( ( hSnapshot != INVALID_HANDLE_VALUE ) &&
         Module32First ( hSnapshot, &module_entry ) )
    {
        do
        {
            ModuleTable.emplace_back ( module_entry );
        }
        while ( Module32Next ( hSnapshot, &module_entry ) );
    }
    CloseHandle ( hSnapshot );
}

void FinalizeModuleTable()
{
    ModuleTable.clear();
}

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

#if 0
    static std::unordered_map<HMODULE, std::vector<std::tuple<FARPROC, LPCSTR>>> module_map;

    if ( module_map.find ( hModule ) == module_map.end() )
    {
        module_map[hModule].reserve ( EXPORT_DIRECTORY->NumberOfNames );
        for ( DWORD i = 0; i < EXPORT_DIRECTORY->NumberOfNames; ++i )
        {
            module_map[hModule].emplace_back (
                reinterpret_cast<FARPROC> ( reinterpret_cast<uint8_t*> ( hModule ) + functions[name_ordinals[i]] ),
                reinterpret_cast<LPCSTR> ( reinterpret_cast<uint8_t*> ( hModule ) + names[i] )
            );
#if 0
            std::cout <<
                      "Function Name " <<
                      reinterpret_cast<char*> ( reinterpret_cast<uint8_t*> ( hModule ) + names[i] ) <<
                      " Function Ordinal " <<
                      name_ordinals[i] <<
                      " Function Address " <<
                      reinterpret_cast<void*> ( reinterpret_cast<uint8_t*> ( hModule ) + functions[name_ordinals[i]] ) <<
                      std::endl;
#endif
        }
        std::sort ( module_map[hModule].begin(), module_map[hModule].end(),
                    [] ( const std::tuple<FARPROC, LPCSTR>& a, const std::tuple<FARPROC, LPCSTR>& b ) -> bool
        {
            return std::get<0> ( a ) < std::get<0> ( b );
        } );
    }
    auto function = std::lower_bound ( module_map[hModule].begin(), module_map[hModule].end(), lpAddress,
                                       [] ( const std::tuple<FARPROC, LPCSTR>& a, const FARPROC b ) -> bool
    {
        return std::get<0> ( a ) < b;
    } );
    if ( function != module_map[hModule].end() )
    {
        return std::get<1> ( *function );
    }
    return nullptr;
#endif
    return nullptr;
};

BOOL WINAPI GetModuleEntryByAddr ( _In_ FARPROC lpAddress, _Out_ PMODULEENTRY32 lpModuleEntry )
{
    if ( !lpModuleEntry )
    {
        return FALSE;
    }
    for ( auto& i : ModuleTable )
    {
        if ( DWORD ( PBYTE ( lpAddress ) - i.modBaseAddr ) < i.modBaseSize )
        {
            memcpy ( lpModuleEntry, &i, sizeof ( MODULEENTRY32 ) );
            return TRUE;
        }
    }
    return FALSE;
}

FARPROC WINAPI GetCallStackReturnAddress ( _In_ size_t index )
{
    CONTEXT context;
    STACKFRAME64 stack_frame;
    RtlCaptureContext ( &context );

    memset ( &stack_frame, 0, sizeof ( STACKFRAME64 ) );
    stack_frame.AddrPC.Offset = context.Rip;
    stack_frame.AddrPC.Mode = AddrModeFlat;
    stack_frame.AddrFrame.Offset = context.Rsp;
    stack_frame.AddrFrame.Mode = AddrModeFlat;
    stack_frame.AddrStack.Offset = context.Rsp;
    stack_frame.AddrStack.Mode = AddrModeFlat;

    while ( StackWalk64 (
                IMAGE_FILE_MACHINE_AMD64,
                GetCurrentProcess(),
                GetCurrentThread(),
                &stack_frame,
                &context,
                nullptr,
                SymFunctionTableAccess64,
                SymGetModuleBase64,
                nullptr ) )
    {
        struct Symbol64
        {
            IMAGEHLP_SYMBOL64  symbol;
            char               name[255];
        } symbol;
        DWORD64             displacement = 0;
        symbol.symbol.SizeOfStruct = sizeof ( IMAGEHLP_SYMBOL64 );
        symbol.symbol.MaxNameLength = 255 * sizeof ( char );

        //SymGetSymFromAddr64(GetCurrentProcess(), (ULONG64)stack_frame.AddrPC.Offset, &displacement, &symbol.symbol);
        SymGetSymFromAddr64 ( GetCurrentProcess(), ( ULONG64 ) stack_frame.AddrReturn.Offset, &displacement, &symbol.symbol );
        index++;
        //UnDecorateSymbolName(symbol.Name, (PSTR)name, 256, UNDNAME_COMPLETE);
    }
    return nullptr;
#if 0
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
    return ( Ebp ) ? Ebp->Ret_Addr : nullptr;
#endif
}


void WINAPI PrintCallStack ( _In_ FILE* file )
{
    CONTEXT context;
    STACKFRAME64 stack_frame;
    RtlCaptureContext ( &context );

    memset ( &stack_frame, 0, sizeof ( STACKFRAME64 ) );
    stack_frame.AddrPC.Offset = context.Rip;
    stack_frame.AddrPC.Mode = AddrModeFlat;
    stack_frame.AddrFrame.Offset = context.Rsp;
    stack_frame.AddrFrame.Mode = AddrModeFlat;
    stack_frame.AddrStack.Offset = context.Rsp;
    stack_frame.AddrStack.Mode = AddrModeFlat;


    struct Symbol64
    {
        IMAGEHLP_SYMBOL64  symbol;
        char               name[255];
    } symbol;
    DWORD64             displacement = 0;
    symbol.symbol.SizeOfStruct = sizeof ( IMAGEHLP_SYMBOL64 );
    symbol.symbol.MaxNameLength = 255 * sizeof ( char );


    while ( StackWalk64 (
                IMAGE_FILE_MACHINE_AMD64,
                GetCurrentProcess(),
                GetCurrentThread(),
                &stack_frame,
                &context,
                nullptr,
                SymFunctionTableAccess64,
                SymGetModuleBase64,
                nullptr ) )
    {
        SymGetSymFromAddr64 ( GetCurrentProcess(), ( ULONG64 ) stack_frame.AddrPC.Offset, &displacement, &symbol.symbol );
        //SymGetSymFromAddr64(GetCurrentProcess(), (ULONG64)stack_frame.AddrReturn.Offset, &displacement, &symbol.symbol);
        fprintf ( file, "%s\n", symbol.symbol.Name );
        //UnDecorateSymbolName(symbol.Name, (PSTR)name, 256, UNDNAME_COMPLETE);
    }
}
#endif