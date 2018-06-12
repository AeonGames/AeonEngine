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
#include "RendererSelectDialog.h"
#include <QMessageBox>
#include <QFile>
#include <iostream>

namespace AeonGames
{
    const Renderer* WorldEditor::GetRenderer() const
    {
        return mRenderer.get();
    }
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
    WorldEditor::WorldEditor ( int &argc, char *argv[] ) : QApplication ( argc, argv )
    {
        /* Add a nice renderer selection window.*/
        QStringList renderer_list;
        EnumerateRendererLoaders ( [this, &renderer_list] ( const std::string & aIdentifier )->bool
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
            this->mRenderer = AeonGames::GetRenderer ( renderer_list.at ( 0 ).toStdString() );
        }
        else
        {
            RendererSelectDialog select_renderer;
            select_renderer.SetRenderers ( renderer_list );
            if ( select_renderer.exec() == QDialog::Accepted )
            {
                this->mRenderer = AeonGames::GetRenderer ( select_renderer.GetSelected().toStdString() );
            }
        }

        if ( mRenderer == nullptr )
        {
            throw std::runtime_error ( "No renderer selected, cannot continue." );
        }

        {
            QFile grid_pipeline_file ( ":/pipelines/grid.prg" );
            if ( !grid_pipeline_file.open ( QIODevice::ReadOnly ) )
            {
                throw std::runtime_error ( "Unable to open grid pipeline resource." );
            }
            QByteArray grid_pipeline_byte_array = grid_pipeline_file.readAll();
            mGridPipeline.Load ( grid_pipeline_byte_array.data(), grid_pipeline_byte_array.size() );

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
        {
            QFile grid_mesh_file ( ":/meshes/grid.msh" );
            if ( !grid_mesh_file.open ( QIODevice::ReadOnly ) )
            {
                throw std::runtime_error ( "Unable to open grid mesh resource." );
            }
            QByteArray grid_mesh_byte_array = grid_mesh_file.readAll();
            mGridMesh.Load ( grid_mesh_byte_array.data(), grid_mesh_byte_array.size() );
        }
        mRenderer->LoadRenderMesh ( reinterpret_cast<WorldEditor*> ( qApp )->GetGridMesh() );
        mRenderer->LoadRenderPipeline ( reinterpret_cast<WorldEditor*> ( qApp )->GetGridPipeline() );
        mRenderer->LoadRenderMaterial ( reinterpret_cast<WorldEditor*> ( qApp )->GetXGridMaterial() );
        mRenderer->LoadRenderMaterial ( reinterpret_cast<WorldEditor*> ( qApp )->GetYGridMaterial() );
    }
    WorldEditor::~WorldEditor()
    {
        mRenderer->UnloadRenderMesh ( reinterpret_cast<WorldEditor*> ( qApp )->GetGridMesh() );
        mRenderer->UnloadRenderPipeline ( reinterpret_cast<WorldEditor*> ( qApp )->GetGridPipeline() );
        mRenderer->UnloadRenderMaterial ( reinterpret_cast<WorldEditor*> ( qApp )->GetXGridMaterial() );
        mRenderer->UnloadRenderMaterial ( reinterpret_cast<WorldEditor*> ( qApp )->GetYGridMaterial() );
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