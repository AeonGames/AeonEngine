/*
Copyright (C) 2016-2018 Rodrigo Jose Hernandez Cordoba

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
#include "aeongames/Utilities.h"
#include "ProtoBufHelpers.h"
#include "aeongames/ProtoBufClasses.h"
#include <cassert>

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4251 )
#endif
#include "model.pb.h"
#ifdef _MSC_VER
#pragma warning( pop )
#endif
#include "aeongames/ResourceCache.h"
#include "aeongames/Pipeline.h"
#include "aeongames/Material.h"
#include "aeongames/Mesh.h"
#include "aeongames/Skeleton.h"
#include "aeongames/Animation.h"
#include "aeongames/Renderer.h"

namespace AeonGames
{
    Model::Model ( const std::string& aFilename ) : mFilename ( aFilename )
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
        = default;

    const std::string & Model::GetFilename() const
    {
        return mFilename;
    }

    const std::vector<std::tuple<
    std::shared_ptr<Pipeline>,
        std::shared_ptr<Material>,
        std::shared_ptr<Mesh>>>& Model::GetMeshes() const
    {
        return mMeshes;
    }

    const AABB& Model::GetCenterRadii() const
    {
        return mCenterRadii;
    }

    const std::shared_ptr<Skeleton>& Model::GetSkeleton() const
    {
        return mSkeleton;
    }

    const std::vector<std::shared_ptr<const Animation>>& Model::GetAnimations() const
    {
        return mAnimations;
    }

    void Model::Initialize()
    {
        static ModelBuffer model_buffer;
        std::shared_ptr<Pipeline> default_pipeline;
        std::shared_ptr<Material> default_material;
        LoadProtoBufObject<ModelBuffer> ( model_buffer, mFilename, "AEONMDL" );
        if ( model_buffer.has_default_pipeline() )
        {
            if ( !model_buffer.default_pipeline().has_buffer() )
            {
                default_pipeline = Get<Pipeline> ( model_buffer.default_pipeline().file(),
                                                   model_buffer.default_pipeline().file() );
            }
        }
        if ( model_buffer.has_default_material() )
        {
            if ( !model_buffer.default_material().has_buffer() )
            {
                default_material = Get<Material> ( model_buffer.default_pipeline().file(),
                                                   model_buffer.default_material().file() );
            }
        }
        if ( model_buffer.has_skeleton() )
        {
            if ( !model_buffer.skeleton().has_buffer() )
            {
                mSkeleton = Get<Skeleton> ( model_buffer.skeleton().file(),
                                            model_buffer.skeleton().file() );
            }
        }
        mMeshes.reserve ( model_buffer.assembly_size() );
        mCenterRadii = {};
        for ( int i = 0; i < model_buffer.assembly_size(); ++i )
        {
            std::shared_ptr<Pipeline> pipeline = ( model_buffer.assembly ( i ).has_pipeline() ) ?
                                                 Get<Pipeline> ( model_buffer.assembly ( i ).pipeline().file(),
                                                         model_buffer.assembly ( i ).pipeline().file() ) : default_pipeline;
            std::shared_ptr<Material> material = ( model_buffer.assembly ( i ).has_material() ) ?
                                                 Get<Material> ( model_buffer.assembly ( i ).material().file(),
                                                         model_buffer.assembly ( i ).material().file() ) : default_material;

            if ( !model_buffer.assembly ( i ).mesh().has_buffer() )
            {
                mMeshes.emplace_back ( pipeline, material, Get<Mesh> ( model_buffer.assembly ( i ).mesh().file(),
                                       model_buffer.assembly ( i ).mesh().file() ) );
            }
            mCenterRadii += std::get<2> ( mMeshes.back() )->GetAABB();
        }

        if ( model_buffer.animation_size() )
        {
            mAnimations.reserve ( model_buffer.animation_size() );
            for ( auto& animation : model_buffer.animation() )
            {
                if ( !animation.has_buffer() )
                {
                    mAnimations.emplace_back ( Get<Animation> ( animation.file(), animation.file() ) );
                }
                else
                {
                    throw std::runtime_error ( "Embeded animation buffers in model files not implemented yet." );
                }
            }
        }

        model_buffer.Clear();
    }

    void Model::Finalize()
    {
    }
}
