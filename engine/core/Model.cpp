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
    Model::Model ( const std::string&  aFilename )
    {
        try
        {
            Load ( aFilename );
        }
        catch ( ... )
        {
            Unload();
            throw;
        }
    }

    Model::Model ( const void * aBuffer, size_t aBufferSize )
    {
        if ( !aBuffer && !aBufferSize )
        {
            throw std::runtime_error ( "Cannot initialize model object with null data." );
            return;
        }
        try
        {
            Load ( aBuffer, aBufferSize );
        }
        catch ( ... )
        {
            Unload();
            throw;
        }
    }

    void Model::Load ( const std::string& aFilename )
    {
        static std::mutex m;
        static ModelBuffer pipeline_buffer;
        std::lock_guard<std::mutex> hold ( m );
        LoadProtoBufObject<ModelBuffer> ( pipeline_buffer, aFilename, "AEONMDL" );
        Load ( pipeline_buffer );
        pipeline_buffer.Clear();
    }

    void Model::Load ( const void* aBuffer, size_t aBufferSize )
    {
        static std::mutex m;
        static ModelBuffer pipeline_buffer;
        std::lock_guard<std::mutex> hold ( m );
        LoadProtoBufObject<ModelBuffer> ( pipeline_buffer, aBuffer, aBufferSize, "AEONMDL" );
        Load ( pipeline_buffer );
        pipeline_buffer.Clear();
    }

    void Model::Load ( const ModelBuffer& aModelBuffer )
    {
        std::shared_ptr<Pipeline> default_pipeline{};
        std::shared_ptr<Material> default_material{};

        // Default Pipeline ---------------------------------------------------------------------
        if ( aModelBuffer.has_default_pipeline() )
        {
            default_pipeline = Pipeline::GetPipeline ( GetReferenceBufferId ( aModelBuffer.default_pipeline() ) );
        }

        // Default Material ---------------------------------------------------------------------
        if ( aModelBuffer.has_default_material() )
        {
            default_material = Material::GetMaterial ( GetReferenceBufferId ( aModelBuffer.default_material() ) );
        }

        // Skeleton -----------------------------------------------------------------------------
        if ( aModelBuffer.has_skeleton() )
        {
            mSkeleton = Skeleton::GetSkeleton ( GetReferenceBufferId ( aModelBuffer.skeleton() ) );
        }
        // Meshes -----------------------------------------------------------------------------
        mAssemblies.reserve ( aModelBuffer.assembly_size() );
        for ( auto& assembly : aModelBuffer.assembly() )
        {
            std::shared_ptr<Mesh> mesh{};
            std::shared_ptr<Pipeline> pipeline{default_pipeline};
            std::shared_ptr<Material> material{default_material};

            if ( assembly.has_mesh() )
            {
                mesh = Mesh::GetMesh ( GetReferenceBufferId ( assembly.mesh() ) );
            }

            if ( assembly.has_pipeline() )
            {
                pipeline = Pipeline::GetPipeline ( GetReferenceBufferId ( assembly.pipeline() ) );
            }

            if ( assembly.has_material() )
            {
                material = Material::GetMaterial ( GetReferenceBufferId ( assembly.material() ) );
            }
            mAssemblies.emplace_back ( mesh, pipeline, material );
#if 0
            if ( GetRenderer() )
            {
                GetRenderer()->LoadRenderMesh ( *mesh );
                GetRenderer()->LoadRenderPipeline ( *pipeline );
                GetRenderer()->LoadRenderMaterial ( *material );
            }
#endif
        }
        // Animations -----------------------------------------------------------------------------
        mAnimations.reserve ( aModelBuffer.animation_size() );
        for ( auto& animation : aModelBuffer.animation() )
        {
            mAnimations.emplace_back ( Animation::GetAnimation ( GetReferenceBufferId ( animation ) ) );
        }
    }

    void Model::Unload()
    {
        mSkeleton.reset();
        mAssemblies.clear();
        mAnimations.clear();
    }

    const std::vector<Model::Assembly>& Model::GetAssemblies() const
    {
        return mAssemblies;
    }

    const Skeleton* Model::GetSkeleton() const
    {
        return mSkeleton.get();
    }

    const std::vector<std::shared_ptr<const Animation>>& Model::GetAnimations() const
    {
        return mAnimations;
    }

    // Statics -----------------------------------------------------------------
    const std::shared_ptr<Model> Model::GetModel ( uint32_t aId )
    {
        return ResourceCache<uint32_t, Model>::Get ( aId, aId );
    }
    const std::shared_ptr<Model> Model::GetModel ( const std::string& aPath )
    {
        uint32_t id = crc32i ( aPath.c_str(), aPath.size() );
        return Model::GetModel ( id );
    }
    uint32_t Model::GetId ( const std::shared_ptr<Model>& aModel )
    {
        return ResourceCache<uint32_t, Model>::GetKey ( aModel );
    }
    const std::string Model::GetPath ( const std::shared_ptr<Model>& aModel )
    {
        return GetResourcePath ( GetId ( aModel ) );
    }
    // -------------------------------------------------------------------------
}
