/*
Copyright (C) 2018 Rodrigo Jose Hernandez Cordoba

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
#include "aeongames/Model.h"
#include "aeongames/Node.h"
#include "aeongames/Mesh.h"
#include "aeongames/Pipeline.h"
#include "aeongames/Material.h"
#include "aeongames/Skeleton.h"
#include "aeongames/Animation.h"
#include "aeongames/Utilities.h"
#include "ProtoBufHelpers.h"
#include "aeongames/ProtoBufUtils.h"
#include "aeongames/Window.h"
#include "aeongames/CRC.h"
#include "aeongames/AeonEngine.h"
#include "aeongames/ResourceCache.h"
#include "aeongames/ProtoBufClasses.h"
#include "aeongames/Renderer.h"
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4251 )
#endif
#include "reference.pb.h"
#include "model.pb.h"
#ifdef _MSC_VER
#pragma warning( pop )
#endif

namespace AeonGames
{
    Model::Model()
        = default;
    Model::~Model()
        = default;

    Model::Model ( uint32_t aId )
    {
        Load ( aId );
    }

    Model::Model ( const std::string&  aFilename )
    {
        Load ( aFilename );
    }

    Model::Model ( const void * aBuffer, size_t aBufferSize )
    {
        Load ( aBuffer, aBufferSize );
    }

    void Model::Load ( const std::string& aFilename )
    {
        Load ( crc32i ( aFilename.c_str(), aFilename.size() ) );
    }

    void Model::Load ( uint32_t aId )
    {
        std::vector<uint8_t> buffer ( GetResourceSize ( aId ), 0 );
        LoadResource ( aId, buffer.data(), buffer.size() );
        try
        {
            Load ( buffer.data(), buffer.size() );
        }
        catch ( ... )
        {
            Unload();
            throw;
        }
    }

    void Model::Load ( const void* aBuffer, size_t aBufferSize )
    {
        static std::mutex m;
        static ModelBuffer model_buffer;
        std::lock_guard<std::mutex> hold ( m );
        LoadProtoBufObject<ModelBuffer> ( model_buffer, aBuffer, aBufferSize, "AEONMDL" );
        Load ( model_buffer );
        model_buffer.Clear();
    }

    void Model::Load ( const ModelBuffer& aModelBuffer )
    {
        ResourceId default_pipeline{};
        ResourceId default_material{};

        // Default Pipeline ---------------------------------------------------------------------
        if ( aModelBuffer.has_default_pipeline() )
        {
            default_pipeline = {"Pipeline"_crc32, GetReferenceBufferId ( aModelBuffer.default_pipeline() ) } ;
            default_pipeline.Store();
        }

        // Default Material ---------------------------------------------------------------------
        if ( aModelBuffer.has_default_material() )
        {
            default_material = {"Material"_crc32, GetReferenceBufferId ( aModelBuffer.default_material() ) } ;
            default_material.Store();
        }

        // Skeleton -----------------------------------------------------------------------------
        if ( aModelBuffer.has_skeleton() )
        {
            mSkeleton = { "Skeleton"_crc32, GetReferenceBufferId ( aModelBuffer.skeleton() ) };
            mSkeleton.Store();
        }
        // Meshes -----------------------------------------------------------------------------
        mAssemblies.reserve ( aModelBuffer.assembly_size() );
        for ( auto& assembly : aModelBuffer.assembly() )
        {
            ResourceId mesh{};
            ResourceId pipeline{default_pipeline};
            ResourceId material{default_material};

            if ( assembly.has_mesh() )
            {
                mesh = {"Mesh"_crc32, GetReferenceBufferId ( assembly.mesh() ) } ;
                mesh.Store();
            }

            if ( assembly.has_pipeline() )
            {
                pipeline = {"Pipeline"_crc32, GetReferenceBufferId ( assembly.pipeline() ) } ;
                pipeline.Store();
            }

            if ( assembly.has_material() )
            {
                material = {"Material"_crc32, GetReferenceBufferId ( assembly.material() ) } ;
                material.Store();
            }
            mAssemblies.emplace_back ( mesh, pipeline, material );
        }
        // Animations -----------------------------------------------------------------------------
        mAnimations.reserve ( aModelBuffer.animation_size() );
        for ( auto& animation : aModelBuffer.animation() )
        {
            mAnimations.emplace_back ( ResourceId{"Animation"_crc32, GetReferenceBufferId ( animation ) } ).Store();
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
        /** @todo implement resource id to pointer */
        assert ( 0 && "Implement resource id to pointer." );
        return nullptr;
    }

    const std::vector<ResourceId>& Model::GetAnimations() const
    {
        return mAnimations;
    }
}
