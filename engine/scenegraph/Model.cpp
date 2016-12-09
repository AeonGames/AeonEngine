/*
Copyright 2016 Rodrigo Jose Hernandez Cordoba

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
#include <cassert>
#include "aeongames/Model.h"
#include "aeongames/Utilities.h"
#include "ProtoBufHelpers.h"
#include "aeongames/ProtoBufClasses.h"
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4251 )
#endif
#include "model.pb.h"
#ifdef _MSC_VER
#pragma warning( pop )
#endif
#include "aeongames/ResourceCache.h"
#include "aeongames/Program.h"
#include "aeongames/Material.h"
#include "aeongames/Mesh.h"
#include "aeongames/Renderer.h"

namespace AeonGames
{
    Model::Model ( const std::string & aFilename ) : mFilename ( aFilename )
    {
        try
        {
            Initialize();
        }
        catch ( ... )
        {
            Finalize();
            throw;
        }
    }

    Model::~Model()
    {
    }

    const std::string & Model::GetFilename() const
    {
        return mFilename;
    }

    const std::shared_ptr<Program> Model::GetProgram() const
    {
        return mProgram;
    }

    const std::shared_ptr<Material> Model::GetMaterial() const
    {
        return mMaterial;
    }

    const std::shared_ptr<Mesh> Model::GetMesh() const
    {
        return mMesh;
    }

    void Model::Initialize()
    {
        static ModelBuffer model_buffer;
        LoadProtoBufObject<ModelBuffer> ( model_buffer, mFilename, "AEONMDL" );
        mProgram = Get<Program> ( model_buffer.program().file() );
        mMaterial = Get<Material> ( model_buffer.material().file() );
        mMesh = Get<Mesh> ( model_buffer.material().file() );
        model_buffer.Clear();
    }

    void Model::Finalize()
    {
    }

#if 0
    void Model::Render ( Renderer * aRenderer )
    {
        assert ( aRenderer );
        if ( aRenderer )
        {
            float model_matrix[16];
            GetGlobalTransform().GetMatrix ( model_matrix );
            aRenderer->SetModelMatrix ( model_matrix );
            aRenderer->Render ( mMesh, mProgram );
        }
    }
#endif
}
