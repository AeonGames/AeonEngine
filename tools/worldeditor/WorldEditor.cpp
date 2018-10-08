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
#include "aeongames/Vector3.h"
#include "aeongames/Vector4.h"
#include "aeongames/Renderer.h"
#include "aeongames/AeonEngine.h"
#include "RendererSelectDialog.h"
#include <QMessageBox>
#include <QFile>
#include <iostream>

namespace AeonGames
{
    const GridSettings& WorldEditor::GetGridSettings() const
    {
        return mGridSettings;
    }
    const Pipeline& WorldEditor::GetGridPipeline() const
    {
        return mGridPipeline;
    }
    const Mesh& WorldEditor::GetGridMesh() const
    {
        return mGridMesh;
    }
    const Material& WorldEditor::GetXGridMaterial() const
    {
        return mXGridMaterial;
    }
    const Material& WorldEditor::GetYGridMaterial() const
    {
        return mYGridMaterial;
    }
    const Pipeline& WorldEditor::GetWirePipeline() const
    {
        return mWirePipeline;
    }
    const Mesh& WorldEditor::GetAABBWireMesh() const
    {
        return mAABBWireMesh;
    }

    static void LoadPipeline ( Pipeline& aPipeline, const std::string& aFileName )
    {
        QFile pipeline_file ( aFileName.c_str() );
        if ( !pipeline_file.open ( QIODevice::ReadOnly ) )
        {
            throw std::runtime_error ( "Unable to open pipeline." );
        }
        QByteArray pipeline_byte_array = pipeline_file.readAll();
        aPipeline.Load ( pipeline_byte_array.data(), pipeline_byte_array.size() );
    }

    static void LoadMesh ( Mesh& aMesh, const std::string& aFileName )
    {
        QFile mesh_file ( aFileName.c_str() );
        if ( !mesh_file.open ( QIODevice::ReadOnly ) )
        {
            throw std::runtime_error ( "Unable to open mesh." );
        }
        QByteArray mesh_byte_array = mesh_file.readAll();
        aMesh.Load ( mesh_byte_array.data(), mesh_byte_array.size() );
    }

    WorldEditor::WorldEditor ( int &argc, char *argv[] ) : QApplication ( argc, argv )
    {
        /* Add a nice renderer selection window.*/
        QStringList renderer_list;
        EnumerateRendererConstructors ( [this, &renderer_list] ( const std::string & aIdentifier )->bool
        {
            renderer_list.append ( QString::fromStdString ( aIdentifier ) );
            return true;
        } );

        if ( !renderer_list.size() )
        {
            throw std::runtime_error ( "No renderer available, cannot continue." );
        }
        if ( renderer_list.size() == 1 )
        {
            AeonGames::SetRenderer ( renderer_list.at ( 0 ).toStdString() );
        }
        else
        {
            RendererSelectDialog select_renderer;
            select_renderer.SetRenderers ( renderer_list );
            if ( select_renderer.exec() == QDialog::Accepted )
            {
                AeonGames::SetRenderer ( select_renderer.GetSelected().toStdString() );
            }
        }

        if ( !GetRenderer() )
        {
            throw std::runtime_error ( "No renderer selected, cannot continue." );
        }

        {
            LoadPipeline ( mGridPipeline, ":/pipelines/grid.prg" );
            LoadPipeline ( mWirePipeline, ":/pipelines/solid_wire.prg" );

            mXGridMaterial = mGridPipeline.GetDefaultMaterial();
            mXGridMaterial.Set ( "Scale", Vector3{mGridSettings.width(), mGridSettings.height(), 1.0f} );
            mXGridMaterial.Set ( "StartingPosition", Vector3{0.0f, - ( mGridSettings.height() / 2 ), 0.0f} );
            mXGridMaterial.Set ( "Offset", Vector3{0.0f, ( mGridSettings.height() / static_cast<float> ( mGridSettings.horizontalSpacing() ) ), 0.0f} );
            mXGridMaterial.Set ( "LineCount", mGridSettings.horizontalSpacing() + 1 );
            mXGridMaterial.Set ( "OddLineColor",
                                 Vector4
            {
                static_cast<float> ( mGridSettings.oddLineColor().redF() ),
                static_cast<float> ( mGridSettings.oddLineColor().greenF() ),
                static_cast<float> ( mGridSettings.oddLineColor().blueF() ),
                static_cast<float> ( mGridSettings.oddLineColor().alphaF() )
            } );
            mXGridMaterial.Set ( "EvenLineColor",
                                 Vector4
            {
                static_cast<float> ( mGridSettings.evenLineColor().redF() ),
                static_cast<float> ( mGridSettings.evenLineColor().greenF() ),
                static_cast<float> ( mGridSettings.evenLineColor().blueF() ),
                static_cast<float> ( mGridSettings.evenLineColor().alphaF() )
            } );
            mXGridMaterial.Set ( "CentralLineColor",
                                 Vector4
            {
                static_cast<float> ( mGridSettings.xLineColor().redF() ),
                static_cast<float> ( mGridSettings.xLineColor().greenF() ),
                static_cast<float> ( mGridSettings.xLineColor().blueF() ),
                static_cast<float> ( mGridSettings.xLineColor().alphaF() )
            } );
            mXGridMaterial.Set ( "BorderLineColor",
                                 Vector4
            {
                static_cast<float> ( mGridSettings.borderLineColor().redF() ),
                static_cast<float> ( mGridSettings.borderLineColor().greenF() ),
                static_cast<float> ( mGridSettings.borderLineColor().blueF() ),
                static_cast<float> ( mGridSettings.borderLineColor().alphaF() )
            } );

            mYGridMaterial = mGridPipeline.GetDefaultMaterial();
            mYGridMaterial.Set ( "Scale", Vector3{mGridSettings.width(), mGridSettings.height(), 1.0f} );
            mYGridMaterial.Set ( "StartingPosition", Vector3{ - ( mGridSettings.width() / 2 ), 0.0f, 0.0f} );
            mYGridMaterial.Set ( "Offset", Vector3{ ( mGridSettings.width() / static_cast<float> ( mGridSettings.verticalSpacing() ) ), 0.0f, 0.0f} );
            mYGridMaterial.Set ( "LineCount", mGridSettings.verticalSpacing() + 1 );
            mYGridMaterial.Set ( "OddLineColor",
                                 Vector4
            {
                static_cast<float> ( mGridSettings.oddLineColor().redF() ),
                static_cast<float> ( mGridSettings.oddLineColor().greenF() ),
                static_cast<float> ( mGridSettings.oddLineColor().blueF() ),
                static_cast<float> ( mGridSettings.oddLineColor().alphaF() )
            } );
            mYGridMaterial.Set ( "EvenLineColor",
                                 Vector4
            {
                static_cast<float> ( mGridSettings.evenLineColor().redF() ),
                static_cast<float> ( mGridSettings.evenLineColor().greenF() ),
                static_cast<float> ( mGridSettings.evenLineColor().blueF() ),
                static_cast<float> ( mGridSettings.evenLineColor().alphaF() )
            } );
            mYGridMaterial.Set ( "CentralLineColor",
                                 Vector4
            {
                static_cast<float> ( mGridSettings.yLineColor().redF() ),
                static_cast<float> ( mGridSettings.yLineColor().greenF() ),
                static_cast<float> ( mGridSettings.yLineColor().blueF() ),
                static_cast<float> ( mGridSettings.yLineColor().alphaF() )
            } );
            mYGridMaterial.Set ( "BorderLineColor",
                                 Vector4
            {
                static_cast<float> ( mGridSettings.borderLineColor().redF() ),
                static_cast<float> ( mGridSettings.borderLineColor().greenF() ),
                static_cast<float> ( mGridSettings.borderLineColor().blueF() ),
                static_cast<float> ( mGridSettings.borderLineColor().alphaF() )
            } );
        }
        LoadMesh ( mGridMesh, ":/meshes/grid.msh" );
        LoadMesh ( mAABBWireMesh, ":/meshes/aabb_wire.msh" );
#if 0
        GetRenderer()->LoadRenderMesh ( mGridMesh );
        GetRenderer()->LoadRenderPipeline ( mGridPipeline );
        GetRenderer()->LoadRenderMaterial ( mXGridMaterial );
        GetRenderer()->LoadRenderMaterial ( mYGridMaterial );

        GetRenderer()->LoadRenderMesh ( mAABBWireMesh );
        GetRenderer()->LoadRenderPipeline ( mWirePipeline );
        GetRenderer()->LoadRenderMaterial ( mWirePipeline.GetDefaultMaterial() );
#endif
    }
    WorldEditor::~WorldEditor()
    {
#if 0
        GetRenderer()->UnloadRenderMesh ( mGridMesh );
        GetRenderer()->UnloadRenderPipeline ( mGridPipeline );
        GetRenderer()->UnloadRenderMaterial ( mXGridMaterial );
        GetRenderer()->UnloadRenderMaterial ( mYGridMaterial );

        GetRenderer()->UnloadRenderMesh ( mAABBWireMesh );
        GetRenderer()->UnloadRenderPipeline ( mWirePipeline );
        GetRenderer()->UnloadRenderMaterial ( mWirePipeline.GetDefaultMaterial() );
#endif
    }

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