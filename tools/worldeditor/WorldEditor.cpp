/*
Copyright (C) 2017,2018 Rodrigo Jose Hernandez Cordoba

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
#include "WorldEditor.h"
#include <QMessageBox>
#include <QFile>
#include <iostream>

namespace AeonGames
{
    WorldEditor::WorldEditor ( int &argc, char *argv[] ) : QApplication ( argc, argv )
    {
        {
            QFile grid_pipeline_file ( ":/pipelines/grid.prg" );
            if ( !grid_pipeline_file.open ( QIODevice::ReadOnly ) )
            {
                throw std::runtime_error ( "Unable to open grid pipeline resource." );
            }
            QByteArray grid_pipeline_byte_array = grid_pipeline_file.readAll();
            mGridPipeline.Load ( grid_pipeline_byte_array.data(), grid_pipeline_byte_array.size() );
        }
        {
            QFile grid_mesh_file ( ":/meshes/grid.msh" );
            if ( !grid_mesh_file.open ( QIODevice::ReadOnly ) )
            {
                throw std::runtime_error ( "Unable to open grid mesh resource." );
            }
            QByteArray grid_mesh_byte_array = grid_mesh_file.readAll();
            mGridMesh.Load ( grid_mesh_byte_array.data(), grid_mesh_byte_array.size() );
        }
    }
    WorldEditor::~WorldEditor()
        = default;
    bool WorldEditor::notify ( QObject *receiver, QEvent *event )
    {
        try
        {
            return QApplication::notify ( receiver, event );
        }
        catch ( std::runtime_error& e )
        {
            std::cout << e.what();
            QMessageBox::critical ( nullptr, applicationName(),
                                    e.what(),
                                    QMessageBox::Ok,
                                    QMessageBox::Ok );
            return false;
        }
    }
}