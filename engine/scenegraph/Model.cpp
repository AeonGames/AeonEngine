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
#include "aeongames/Program.h"
#include "aeongames/Mesh.h"
#include "aeongames/Renderer.h"

namespace AeonGames
{
    Model::Model() : Node(), mMesh ( nullptr ), mProgram ( nullptr )
    {
    }

    Model::~Model()
    {
    }

    void Model::SetMesh ( const std::shared_ptr<Mesh>& aMesh )
    {
        mMesh = aMesh;
    }

    void Model::SetProgram ( const std::shared_ptr<Program>& aProgram )
    {
        mProgram = aProgram;
    }

    void Model::Update ( const double delta )
    {

    }

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
}