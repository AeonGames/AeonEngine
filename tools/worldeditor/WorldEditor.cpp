/*
Copyright (C) 2017-2019,2021,2022,2025 Rodrigo Jose Hernandez Cordoba

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
#include "aeongames/Material.h"
#include "RendererSelectDialog.h"
#include <QMessageBox>
#include <QFile>
#include <iostream>

namespace AeonGames
{
    const Pipeline& WorldEditor::GetGridPipeline() const
    {
        return *mGridPipeline;
    }
    const Mesh& WorldEditor::GetGridMesh() const
    {
        return *mGridMesh;
    }
    const Material& WorldEditor::GetXGridMaterial() const
    {
        return *mXGridMaterial;
    }
    const Material& WorldEditor::GetYGridMaterial() const
    {
        return *mYGridMaterial;
    }
    const Pipeline& WorldEditor::GetSolidColorPipeline() const
    {
        return *mSolidColorPipeline;
    }
    const Material& WorldEditor::GetSolidColorMaterial() const
    {
        return *mSolidColorMaterial;
    }
    const Mesh& WorldEditor::GetAABBWireMesh() const
    {
        return *mAABBWireMesh;
    }

    int WorldEditor::GetStringIdMetaType() const
    {
        return mStringIdMetaType;
    }

    int WorldEditor::GetStringMetaType() const
    {
        return mStringMetaType;
    }

    int WorldEditor::GetPathMetaType() const
    {
        return mPathMetaType;
    }

    Renderer* WorldEditor::GetRenderer()
    {
        return mRenderer.get();
    }

    static void LoadPipeline ( Pipeline& aPipeline, const std::string& aFileName )
    {
        QFile pipeline_file ( aFileName.c_str() );
        if ( !pipeline_file.open ( QIODevice::ReadOnly ) )
        {
            throw std::runtime_error ( "Unable to open pipeline." );
        }
        QByteArray pipeline_byte_array = pipeline_file.readAll();
        aPipeline.LoadFromMemory ( pipeline_byte_array.data(), pipeline_byte_array.size() );
    }
    static void LoadMaterial ( Material& aMaterial, const std::string& aFileName )
    {
        QFile material_file ( aFileName.c_str() );
        if ( !material_file.open ( QIODevice::ReadOnly ) )
        {
            throw std::runtime_error ( "Unable to open material." );
        }
        QByteArray material_byte_array = material_file.readAll();
        aMaterial.LoadFromMemory ( material_byte_array.data(), material_byte_array.size() );
    }

    static void LoadMesh ( Mesh& aMesh, const std::string& aFileName )
    {
        QFile mesh_file ( aFileName.c_str() );
        if ( !mesh_file.open ( QIODevice::ReadOnly ) )
        {
            throw std::runtime_error ( "Unable to open mesh." );
        }
        QByteArray mesh_byte_array = mesh_file.readAll();
        aMesh.LoadFromMemory ( mesh_byte_array.data(), mesh_byte_array.size() );
    }

    WorldEditor::WorldEditor ( int &argc, char *argv[] ) : QApplication ( argc, argv ),
        mStringIdMetaType{qRegisterMetaType<StringId>() },
        mStringMetaType{qRegisterMetaType<std::string>() },
        mPathMetaType{qRegisterMetaType<std::filesystem::path>() }
    {
        setWindowIcon ( QIcon ( ":/icons/magnifying_glass" ) );
        setOrganizationName ( "AeonGames" );
        setOrganizationDomain ( "aeongames.com" );
        setApplicationName ( "AeonGames World Editor" );

        /* Workspace default settings */
        QSettings settings{};
        settings.beginGroup ( "Workspace" );

        QSizeF Scale = settings.value ( "Scale", QSizeF ( 780.0f, 780.0f ) ).toSizeF();
        QColor OddLineColor = settings.value ( "OddLineColor", QColor ( 74, 74, 74 ) ).value<QColor>();
        QColor EvenLineColor = settings.value ( "EvenLineColor", QColor ( 74, 74, 74 ) ).value<QColor>();
        QColor XLineColor = settings.value ( "XLineColor", QColor ( 255, 0, 0 ) ).value<QColor>();
        QColor YLineColor = settings.value ( "YLineColor", QColor ( 0, 255, 0 ) ).value<QColor>();
        QColor BorderLineColor = settings.value ( "BorderLineColor", QColor ( 74, 74, 74 ) ).value<QColor>();
        uint32_t HorizontalSpacing = settings.value ( "HorizontalSpacing", uint32_t ( 16 ) ).toUInt();
        uint32_t VerticalSpacing = settings.value ( "VerticalSpacing", uint32_t ( 16 ) ).toUInt();
        settings.endGroup();


        /* Add a nice renderer selection window.*/
        QStringList renderer_list;
        EnumerateRendererConstructors ( [&renderer_list] ( const std::string & aIdentifier )->bool
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
            mRendererName = renderer_list.at ( 0 ).toStdString();
        }
        else
        {
            RendererSelectDialog select_renderer;
            select_renderer.SetRenderers ( renderer_list );
            if ( select_renderer.exec() == QDialog::Accepted )
            {
                mRendererName = select_renderer.GetSelected().toStdString();
            }
        }

        if ( mRendererName.empty() )
        {
            throw std::runtime_error ( "No renderer selected, cannot continue." );
        }

        {
            mGridPipeline = std::make_unique<Pipeline>();
            mSolidColorPipeline = std::make_unique<Pipeline>();
            mXGridMaterial = std::make_unique<Material>();
            mYGridMaterial = std::make_unique<Material>();
            mSolidColorMaterial = std::make_unique<Material>();

            LoadMaterial ( *mSolidColorMaterial, ":/materials/solidcolor.mtl" );
            LoadPipeline ( *mGridPipeline, ":/pipelines/grid.pln" );
            LoadPipeline ( *mSolidColorPipeline, ":/pipelines/solid_color.pln" );
            LoadMaterial ( *mXGridMaterial, ":/materials/grid.mtl" );

            mXGridMaterial->Set ( { "Scale", Vector3{static_cast<float> ( Scale.width() ), static_cast<float> ( Scale.height() ), 1.0f} } );
            mXGridMaterial->Set ( { "StartingPosition", Vector3{0.0f, - static_cast<float> ( Scale.height() / 2 ), 0.0f} } );
            mXGridMaterial->Set ( { "Offset", Vector3{0.0f, static_cast<float> ( Scale.height() / static_cast<float> ( HorizontalSpacing ) ), 0.0f} } );
            mXGridMaterial->Set ( { "LineCount", static_cast<uint32_t> ( HorizontalSpacing + 1 ) } );
            mXGridMaterial->Set ( {"OddLineColor",
                                   Vector4
            {
                static_cast<float> ( OddLineColor.redF() ),
                static_cast<float> ( OddLineColor.greenF() ),
                static_cast<float> ( OddLineColor.blueF() ),
                static_cast<float> ( OddLineColor.alphaF() )
            }} );
            mXGridMaterial->Set ( {"EvenLineColor",
                                   Vector4
            {
                static_cast<float> ( EvenLineColor.redF() ),
                static_cast<float> ( EvenLineColor.greenF() ),
                static_cast<float> ( EvenLineColor.blueF() ),
                static_cast<float> ( EvenLineColor.alphaF() )
            }} );
            mXGridMaterial->Set ( {"CentralLineColor",
                                   Vector4
            {
                static_cast<float> ( XLineColor.redF() ),
                static_cast<float> ( XLineColor.greenF() ),
                static_cast<float> ( XLineColor.blueF() ),
                static_cast<float> ( XLineColor.alphaF() )
            }} );
            mXGridMaterial->Set ( {"BorderLineColor",
                                   Vector4
            {
                static_cast<float> ( BorderLineColor.redF() ),
                static_cast<float> ( BorderLineColor.greenF() ),
                static_cast<float> ( BorderLineColor.blueF() ),
                static_cast<float> ( BorderLineColor.alphaF() )
            }} );

            LoadMaterial ( *mYGridMaterial, ":/materials/grid.mtl" );
            mYGridMaterial->Set ( { "Scale", Vector3{static_cast<float> ( Scale.width() ), static_cast<float> ( Scale.height() ), 1.0f} } );
            mYGridMaterial->Set ( { "StartingPosition", Vector3{ - static_cast<float> ( Scale.width() / 2 ), 0.0f, 0.0f} } );
            mYGridMaterial->Set ( { "Offset", Vector3{ ( static_cast<float> ( Scale.width() ) / ( VerticalSpacing ) ), 0.0f, 0.0f} } );
            mYGridMaterial->Set ( { "LineCount", static_cast<uint32_t> ( VerticalSpacing + 1 ) } );
            mYGridMaterial->Set ( {"OddLineColor",
                                   Vector4
            {
                static_cast<float> ( OddLineColor.redF() ),
                static_cast<float> ( OddLineColor.greenF() ),
                static_cast<float> ( OddLineColor.blueF() ),
                static_cast<float> ( OddLineColor.alphaF() )
            }} );
            mYGridMaterial->Set ( {"EvenLineColor",
                                   Vector4
            {
                static_cast<float> ( EvenLineColor.redF() ),
                static_cast<float> ( EvenLineColor.greenF() ),
                static_cast<float> ( EvenLineColor.blueF() ),
                static_cast<float> ( EvenLineColor.alphaF() )
            }} );
            mYGridMaterial->Set ( {"CentralLineColor",
                                   Vector4
            {
                static_cast<float> ( YLineColor.redF() ),
                static_cast<float> ( YLineColor.greenF() ),
                static_cast<float> ( YLineColor.blueF() ),
                static_cast<float> ( YLineColor.alphaF() )
            }} );
            mYGridMaterial->Set ( {"BorderLineColor",
                                   Vector4
            {
                static_cast<float> ( BorderLineColor.redF() ),
                static_cast<float> ( BorderLineColor.greenF() ),
                static_cast<float> ( BorderLineColor.blueF() ),
                static_cast<float> ( BorderLineColor.alphaF() )
            }} );
        }

        mGridMesh = std::make_unique<Mesh>();
        mAABBWireMesh = std::make_unique<Mesh>();
        LoadMesh ( *mGridMesh, ":/meshes/grid.msh" );
        LoadMesh ( *mAABBWireMesh, ":/meshes/aabb_wire.msh" );
    }

    WorldEditor::~WorldEditor()
    {
        mGridMesh.reset();
        mGridPipeline.reset();
        mXGridMaterial.reset();
        mYGridMaterial.reset();
        mAABBWireMesh.reset();
        mSolidColorMaterial.reset();
        mSolidColorPipeline.reset();
        mRenderer.reset();
    }

    void WorldEditor::AttachWindowToRenderer ( void* aWindow )
    {
        if ( mRenderer == nullptr )
        {
            mRenderer = AeonGames::ConstructRenderer ( mRendererName, aWindow );
        }
        else
        {
            mRenderer->AttachWindow ( aWindow );
        }
    }
    void WorldEditor::DetachWindowFromRenderer ( void* aWindow )
    {
        mRenderer->DetachWindow ( aWindow );
    }
}
