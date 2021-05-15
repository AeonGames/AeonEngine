/*
Copyright (C) 2018,2019,2021 Rodrigo Jose Hernandez Cordoba

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
#include <mutex>
#include "aeongames/ProtoBufClasses.h"
#include "aeongames/Node.h"
#include "aeongames/Mesh.h"
#include "aeongames/Pipeline.h"
#include "aeongames/Material.h"
#include "aeongames/Skeleton.h"
#include "aeongames/Animation.h"
#include "aeongames/Utilities.h"
#include "aeongames/ProtoBufHelpers.h"
#include "aeongames/ProtoBufUtils.h"
#include "aeongames/Window.h"
#include "aeongames/CRC.h"
#include "aeongames/AeonEngine.h"
#include "aeongames/ResourceCache.h"
#include "aeongames/Renderer.h"
#include "aeongames/Model.h"

namespace AeonGames
{
    Model::Model()
        = default;
    Model::~Model()
        = default;

    Model::Model ( uint32_t aId )
    {
        Resource::Load ( aId );
    }

    Model::Model ( const std::string&  aFilename )
    {
        Resource::Load ( aFilename );
    }

    Model::Model ( const void * aBuffer, size_t aBufferSize )
    {
        Resource::Load ( aBuffer, aBufferSize );
    }

    void Model::Load ( const ModelMsg& aModelMsg )
    {
        ResourceId default_pipeline{};
        ResourceId default_material{};

        // Default Pipeline ---------------------------------------------------------------------
        if ( aModelMsg.has_default_pipeline() )
        {
            default_pipeline = {"Pipeline"_crc32, GetReferenceMsgId ( aModelMsg.default_pipeline() ) } ;
            default_pipeline.Store();
        }

        // Default Material ---------------------------------------------------------------------
        if ( aModelMsg.has_default_material() )
        {
            default_material = {"Material"_crc32, GetReferenceMsgId ( aModelMsg.default_material() ) } ;
            default_material.Store();
        }

        // Skeleton -----------------------------------------------------------------------------
        if ( aModelMsg.has_skeleton() )
        {
            mSkeleton = { "Skeleton"_crc32, GetReferenceMsgId ( aModelMsg.skeleton() ) };
            mSkeleton.Store();
        }
        // Meshes -----------------------------------------------------------------------------
        mAssemblies.reserve ( aModelMsg.assembly_size() );
        for ( auto& assembly : aModelMsg.assembly() )
        {
            ResourceId mesh{};
            ResourceId pipeline{default_pipeline};
            ResourceId material{default_material};

            if ( assembly.has_mesh() )
            {
                mesh = {"Mesh"_crc32, GetReferenceMsgId ( assembly.mesh() ) } ;
                mesh.Store();
            }

            if ( assembly.has_pipeline() )
            {
                pipeline = {"Pipeline"_crc32, GetReferenceMsgId ( assembly.pipeline() ) } ;
                pipeline.Store();
            }

            if ( assembly.has_material() )
            {
                material = {"Material"_crc32, GetReferenceMsgId ( assembly.material() ) } ;
                material.Store();
            }
            mAssemblies.emplace_back ( mesh, pipeline, material );
        }
        // Animations -----------------------------------------------------------------------------
        mAnimations.reserve ( aModelMsg.animation_size() );
        for ( auto& animation : aModelMsg.animation() )
        {
            mAnimations.emplace_back ( ResourceId{"Animation"_crc32, GetReferenceMsgId ( animation ) } ).Store();
        }
    }

    void Model::Unload()
    {
        /**@todo refcount Resource Id's? */
        mAssemblies.clear();
        mAnimations.clear();
    }

    const std::vector<Model::Assembly>& Model::GetAssemblies() const
    {
        return mAssemblies;
    }

    const Skeleton* Model::GetSkeleton() const
    {
        return mSkeleton.Cast<Skeleton>();
    }

    const std::vector<ResourceId>& Model::GetAnimations() const
    {
        return mAnimations;
    }
}
