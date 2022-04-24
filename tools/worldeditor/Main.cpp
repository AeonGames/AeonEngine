/*
Copyright (C) 2016-2019,2022 Rodrigo Jose Hernandez Cordoba

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
#include <cstdio>
#include <iostream>
#include <QMessageBox>
#include <QDebug>
#include <mutex>
#include "MainWindow.h"
#include "WorldEditor.h"
#include "aeongames/AeonEngine.h"

#ifdef _MSC_VER
#include <tlhelp32.h>
#include <Dbghelp.h>
#pragma comment(lib, "Dbghelp.lib")

FILE* gAllocationLog = NULL;
int AllocHook ( int allocType, void *userData, size_t size, int
                blockType, long requestNumber, const unsigned char *filename, int
                lineNumber )
{
    static std::mutex m;
    /** Seems like deallocations always have zero size and requestNumber */
    if ( ( blockType == _CRT_BLOCK ) )
    {
        return TRUE;
    }
    if ( gAllocationLog && allocType != _HOOK_FREE )
    {
        STACKFRAME stack_frame{};
        CONTEXT context{};
        uint8_t symbol_buffer[sizeof ( IMAGEHLP_SYMBOL64 ) + MAX_PATH] {};
        PIMAGEHLP_SYMBOL64 symbol = reinterpret_cast<PIMAGEHLP_SYMBOL64> ( symbol_buffer );
        DWORD64 displacement{};
        char name[1024] {};
        std::lock_guard<std::mutex> hold ( m );

        RtlCaptureContext ( &context );
        stack_frame.AddrPC.Offset = context.Rip;
        stack_frame.AddrPC.Mode = AddrModeFlat;
        stack_frame.AddrStack.Offset = context.Rsp;
        stack_frame.AddrStack.Mode = AddrModeFlat;
        stack_frame.AddrFrame.Offset = context.Rbp;
        stack_frame.AddrFrame.Mode = AddrModeFlat;

        symbol->SizeOfStruct = sizeof ( IMAGEHLP_SYMBOL64 );
        symbol->MaxNameLength = MAX_PATH;

        fprintf ( gAllocationLog, "%s %ld Size %llu File %s Line %d\n",
                  ( allocType == _HOOK_ALLOC ) ? "ALLOCATION" :
                  ( allocType == _HOOK_REALLOC ) ? "REALLOCATION" : "DEALLOCATION",
                  requestNumber,
                  size, ( filename != NULL ) ? reinterpret_cast<const char*> ( filename ) : static_cast<const char*> ( "Unknown" ), lineNumber );

        while ( StackWalk64 ( IMAGE_FILE_MACHINE_AMD64, GetCurrentProcess(), GetCurrentThread(), &stack_frame, &context, NULL, SymFunctionTableAccess64, SymGetModuleBase64, NULL ) )
        {
            SymGetSymFromAddr64 ( GetCurrentProcess(), ( ULONG64 ) stack_frame.AddrPC.Offset, &displacement, symbol );
            UnDecorateSymbolName ( symbol->Name, ( PSTR ) name, 1024, UNDNAME_COMPLETE );
            fprintf ( gAllocationLog, "%s\n", name );
        }

        fflush ( gAllocationLog );
    }
    return TRUE;
}
#endif

int ENTRYPOINT main ( int argc, char *argv[] )
{
#ifdef _MSC_VER
#if 0
    // This now works but the log gets huge and makes the process really slow to start up
    // P.S. Looks like all leaks are related to Qt.
    SymInitialize ( GetCurrentProcess(), nullptr, TRUE );
    gAllocationLog = fopen ( "allocation.log", "wt" );
    _CrtSetAllocHook ( AllocHook );
#endif
    /*  Call _CrtDumpMemoryLeaks on exit, required because Qt does a lot of static allocations,
    which are detected as false positives.
    http://msdn.microsoft.com/en-us/library/5at7yxcs%28v=vs.71%29.aspx */
    _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
    // Use _CrtSetBreakAlloc( ) to set breakpoints on allocations.
#endif
    if ( !AeonGames::InitializeGlobalEnvironment() )
    {
        return -1;
    }
    int retval = 0;
    /*  Insanity Check note:
        The following context within a context of stack variables
        works to ensure that mainWindow is destroyed before worldEditor.
        IT SHOULD JUST WORK. IF IT BREAKS AGAIN ONLY IN MINGW64,
        BUT NOT ON CLANG64 or UCRT64 OR VISUAL STUDIO BLAME MINGW64
        DLLS, UPDATE THEM AND DO NOT SPEND ANY MORE TIME ON THIS,
        AS IT IS ALL WASTED TIME. */
    {
        AeonGames::WorldEditor worldeditor ( argc, argv );
        {
            AeonGames::MainWindow mainWindow{};
            mainWindow.showNormal();
            retval = worldeditor.exec();
        }
    }
    AeonGames::FinalizeGlobalEnvironment();
#ifdef _MSC_VER
#if 0
    if ( gAllocationLog )
    {
        _CrtSetAllocHook ( nullptr );
        FinalizeModuleTable();
        fclose ( gAllocationLog );
        gAllocationLog = NULL;
    };
#endif
#endif
    return retval;
}
