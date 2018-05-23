/*
Copyright (C) 2016-2018 Rodrigo Jose Hernandez Cordoba

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
#include <mutex>
#include "MainWindow.h"
#include "WorldEditor.h"
#include "Debug.h"
#include "aeongames/AeonEngine.h"

#ifdef _MSC_VER
FILE* gAllocationLog = NULL;
int AllocHook ( int allocType, void *userData, size_t size, int
                blockType, long requestNumber, const unsigned char *filename, int
                lineNumber )
{
    static std::mutex m;
    /** Seems like deallocations always have zero size and requestNumber */
    if ( ( blockType == _CRT_BLOCK ) /*|| (!size && !requestNumber)*/ )
    {
        return TRUE;
    }
    if ( gAllocationLog && allocType != _HOOK_FREE )
    {
        std::lock_guard<std::mutex> hold ( m );
#if 0
        if ( requestNumber == 3236 )
        {
            PrintCallStack ( gAllocationLog );
        }
#endif
        fprintf ( gAllocationLog, "%s %ld Size %llu\n",
                  ( allocType == _HOOK_ALLOC ) ? "ALLOCATION" :
                  ( allocType == _HOOK_REALLOC ) ? "REALLOCATION" : "DEALLOCATION",
                  requestNumber,
                  size );
    }
    return TRUE;
}
#endif

int ENTRYPOINT main ( int argc, char *argv[] )
{
#ifdef _MSC_VER
#if 0
    SymInitialize ( GetCurrentProcess(), nullptr, TRUE );
    gAllocationLog = fopen ( "allocation.log", "wt" );
    InitializeModuleTable();
    _CrtSetAllocHook ( AllocHook );
#endif
    /*  Call _CrtDumpMemoryLeaks on exit, required because Qt does a lot of static allocations,
    which are detected as false positives.
    http://msdn.microsoft.com/en-us/library/5at7yxcs%28v=vs.71%29.aspx */
    _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
    // Use _CrtSetBreakAlloc( ) to set breakpoints on allocations.
#endif
    if ( !AeonGames::Initialize() )
    {
        return -1;
    }
    int retval = 0;
    AeonGames::WorldEditor worldeditor ( argc, argv );
    worldeditor.setWindowIcon ( QIcon ( ":/icons/magnifying_glass" ) );
    worldeditor.setOrganizationName ( "AeonGames" );
    worldeditor.setOrganizationDomain ( "aeongames.com" );
    worldeditor.setApplicationName ( "AeonGames World Editor" );
    AeonGames::MainWindow* mainWindow;
    try
    {
        mainWindow = new AeonGames::MainWindow();
        mainWindow->showNormal();
        retval = worldeditor.exec();
        // Make sure renderer resources are deleted before engine finalization.
        worldeditor.GetGridMesh().SetRenderMesh ( nullptr );
        worldeditor.GetGridPipeline().GetDefaultMaterial().SetRenderMaterial ( nullptr );
        worldeditor.GetGridPipeline().SetRenderPipeline ( nullptr );
        worldeditor.GetXGridMaterial().SetRenderMaterial ( nullptr );
        worldeditor.GetYGridMaterial().SetRenderMaterial ( nullptr );
        delete mainWindow;
    }
    catch ( std::runtime_error& e )
    {
        std::cout << e.what() << std::endl;
        QMessageBox::critical ( nullptr, worldeditor.applicationName(),
                                e.what(),
                                QMessageBox::Ok,
                                QMessageBox::Ok );
        retval = -1;
    }
    AeonGames::Finalize();
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
