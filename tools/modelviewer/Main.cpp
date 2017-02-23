/*
Copyright (C) 2017,2016 Rodrigo Jose Hernandez Cordoba

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
#include <iostream>
#include <QMessageBox>
#include "MainWindow.h"
#include "ModelViewer.h"
#include "aeongames/AeonEngine.h"

int ENTRYPOINT main ( int argc, char *argv[] )
{
#ifdef _MSC_VER
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
    AeonGames::ModelViewer application ( argc, argv );
    application.setWindowIcon ( QIcon ( ":/icons/magnifying_glass" ) );
    application.setOrganizationName ( "AeonGames" );
    application.setOrganizationDomain ( "aeongames.com" );
    application.setApplicationName ( "AeonGames Model Viewer" );
    AeonGames::MainWindow* mainWindow;
    try
    {
        mainWindow = new AeonGames::MainWindow();
        mainWindow->showNormal();
        retval = application.exec();
    }
    catch ( std::runtime_error& e )
    {
        std::cout << e.what() << std::endl;
        QMessageBox::critical ( nullptr, application.applicationName(),
                                e.what(),
                                QMessageBox::Ok,
                                QMessageBox::Ok );
        retval = -1;
    }
    delete mainWindow;
    AeonGames::Finalize();
    return retval;
}
