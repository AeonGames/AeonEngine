#include "..\..\include\aeongames\Model.h"
/*
Copyright (C) 2016-2017 Rodrigo Jose Hernandez Cordoba

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
#include "aeongames/Pipeline.h"
#include "aeongames/Material.h"
#include "aeongames/Mesh.h"
#include "aeongames/Skeleton.h"
#include "aeongames/Animation.h"
#include "aeongames/Renderer.h"

namespace AeonGames
{
    Model::Model ( std::string  aFilename ) : mFilename ( std::move ( aFilename ) )
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

    const float * const Model::GetCenterRadii() const
    {
        return mCenterRadii;
    }

    const std::shared_ptr<Skeleton>& Model::GetSkeleton() const
    {
        return mSkeleton;
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
                default_pipeline = Get<Pipeline> ( model_buffer.default_pipeline().file() );
            }
        }
        if ( model_buffer.has_default_material() )
        {
            if ( !model_buffer.default_material().has_buffer() )
            {
                default_material = Get<Material> ( model_buffer.default_material().file() );
            }
        }
        if ( model_buffer.has_skeleton() )
        {
            if ( !model_buffer.skeleton().has_buffer() )
            {
                mSkeleton = Get<Skeleton> ( model_buffer.skeleton().file() );
            }
        }
        mMeshes.reserve ( model_buffer.assembly_size() );
        float min[3] {std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max() };
        float max[3] { std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min() };
        for ( int i = 0; i < model_buffer.assembly_size(); ++i )
        {
            std::shared_ptr<Pipeline> pipeline = ( model_buffer.assembly ( i ).has_pipeline() ) ? Get<Pipeline> ( model_buffer.assembly ( i ).pipeline().file() ) : default_pipeline;
            std::shared_ptr<Material> material = ( model_buffer.assembly ( i ).has_material() ) ? Get<Material> ( model_buffer.assembly ( i ).material().file() ) : default_material;

            if ( !model_buffer.assembly ( i ).mesh().has_buffer() )
            {
                mMeshes.emplace_back ( pipeline, material, Get<Mesh> ( model_buffer.assembly ( i ).mesh().file() ) );
            }
            const float *const center_radii = std::get<2> ( mMeshes.back() )->GetCenterRadii();
            min[0] = std::min ( min[0], center_radii[0] - center_radii[3] );
            min[1] = std::min ( min[1], center_radii[1] - center_radii[4] );
            min[2] = std::min ( min[2], center_radii[2] - center_radii[5] );
            max[0] = std::max ( max[0], center_radii[0] + center_radii[3] );
            max[1] = std::max ( max[1], center_radii[1] + center_radii[4] );
            max[2] = std::max ( max[2], center_radii[2] + center_radii[5] );
        }
        mCenterRadii[0] = ( min[0] + max[0] ) / 2;
        mCenterRadii[1] = ( min[1] + max[1] ) / 2;
        mCenterRadii[2] = ( min[2] + max[2] ) / 2;
        mCenterRadii[3] = max[0] - mCenterRadii[0];
        mCenterRadii[4] = max[1] - mCenterRadii[1];
        mCenterRadii[5] = max[2] - mCenterRadii[1];

        if ( model_buffer.animation_size() )
        {
            mAnimations.reserve ( model_buffer.animation_size() );
            for ( auto& animation : model_buffer.animation() )
            {
                if ( !animation.has_buffer() )
                {
                    mAnimations.emplace_back ( Get<Animation> ( animation.file() ) );
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
