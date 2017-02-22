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

#include <fstream>
#include <sstream>
#include <exception>
#include <vector>
#include <cassert>
#include <cstring>
#include "aeongames/ProtoBufClasses.h"
#include "ProtoBufHelpers.h"
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4251 )
#endif
#include "mesh.pb.h"
#ifdef _MSC_VER
#pragma warning( pop )
#endif

#include "aeongames/Utilities.h"
#include "aeongames/Mesh.h"
#include "VulkanMesh.h"

namespace AeonGames
{
    VulkanMesh::VulkanMesh ( const std::shared_ptr<Mesh> aMesh ) :
        mMesh ( aMesh )
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
    VulkanMesh::~VulkanMesh()
    {
        Finalize();
    }

    void VulkanMesh::Render() const
    {
        auto& triangle_groups = mMesh->GetTriangleGroups();
        for ( size_t i = 0; i < triangle_groups.size(); ++i )
        {
        }
    }

    void VulkanMesh::Initialize()
    {
        auto& triangle_groups = mMesh->GetTriangleGroups();
        mBuffers.resize ( triangle_groups.size() );
        for ( size_t i = 0; i < triangle_groups.size(); ++i )
        {
        }
    }

    void VulkanMesh::Finalize()
    {
        for ( auto& i : mBuffers )
        {
        }
    }
}
