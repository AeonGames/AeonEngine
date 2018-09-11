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
#include "ModelComponent.h"
#include "aeongames/Node.h"
#include "aeongames/Mesh.h"
#include "aeongames/Pipeline.h"
#include "aeongames/Material.h"
#include "aeongames/Skeleton.h"
#include "aeongames/Animation.h"
#include "aeongames/Utilities.h"
#include "aeongames/Window.h"
#include "aeongames/CRC.h"

#include "aeongames/ProtoBufClasses.h"
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
    ModelComponent::~ModelComponent() = default;

    const std::string& ModelComponent::GetTypeName() const
    {
        static std::string type_name{"Model"};
        return type_name;
    }

    uint32_t ModelComponent::GetTypeId() const
    {
        return "Model"_crc32;
    }

    std::vector<uint32_t> ModelComponent::GetDependencies() const
    {
        return std::vector<uint32_t> {};
    }

    void ModelComponent::Update ( Node& aNode, double aDelta )
    {
        ///@todo Should Update and Render not be abstract?
        ( void ) aNode;
        ( void ) aDelta;

        /** @todo We don't want the following code to run as part of the game loop.*/
        std::shared_ptr<Pipeline> default_pipeline;
        std::shared_ptr<Material> default_material;

        // Default Pipeline ---------------------------------------------------------------------
        if ( mProperties.has_default_pipeline() )
        {
            switch ( mProperties.default_pipeline().reference_case() )
            {
            case ReferenceBuffer::kPath:
                default_pipeline = Pipeline::GetPipeline ( mProperties.default_pipeline().path() );
                break;
            case ReferenceBuffer::kId:
                default_pipeline = Pipeline::GetPipeline ( mProperties.default_pipeline().id() );
                break;
            default:
                break;
            }
        }

        // Default Material ---------------------------------------------------------------------
        if ( mProperties.has_default_material() )
        {
            switch ( mProperties.default_material().reference_case() )
            {
            case ReferenceBuffer::kPath:
                default_material = Material::GetMaterial ( mProperties.default_material().path() );
                break;
            case ReferenceBuffer::kId:
                default_material = Material::GetMaterial ( mProperties.default_material().id() );
                break;
            default:
                break;
            }
        }

        // Skeleton -----------------------------------------------------------------------------
        if ( mProperties.has_skeleton() )
        {
            switch ( mProperties.skeleton().reference_case() )
            {
            case ReferenceBuffer::kPath:
                mSkeleton = Skeleton::GetSkeleton ( mProperties.skeleton().path() );
                break;
            case ReferenceBuffer::kId:
                mSkeleton = Skeleton::GetSkeleton ( mProperties.skeleton().id() );
                break;
            default:
                break;
            }
        }
        // Meshes -----------------------------------------------------------------------------
        mMeshes.reserve ( mProperties.assembly_size() );
        for ( int i = 0; i < mProperties.assembly_size(); ++i )
        {
            std::shared_ptr<Mesh> mesh{};
            std::shared_ptr<Pipeline> pipeline{default_pipeline};
            std::shared_ptr<Material> material{default_material};

            if ( mProperties.assembly ( i ).has_mesh() )
            {
                switch ( mProperties.assembly ( i ).mesh().reference_case() )
                {
                case ReferenceBuffer::kPath:
                    mesh = Mesh::GetMesh ( mProperties.assembly ( i ).mesh().path() );
                    break;
                case ReferenceBuffer::kId:
                    mesh = Mesh::GetMesh ( mProperties.assembly ( i ).mesh().id() );
                    break;
                default:
                    break;
                }
            }

            if ( mProperties.assembly ( i ).has_pipeline() )
            {
                switch ( mProperties.assembly ( i ).pipeline().reference_case() )
                {
                case ReferenceBuffer::kPath:
                    pipeline = Pipeline::GetPipeline ( mProperties.assembly ( i ).pipeline().path() );
                    break;
                case ReferenceBuffer::kId:
                    pipeline = Pipeline::GetPipeline ( mProperties.assembly ( i ).pipeline().id() );
                    break;
                default:
                    break;
                }
            }

            if ( mProperties.assembly ( i ).has_material() )
            {
                switch ( mProperties.assembly ( i ).material().reference_case() )
                {
                case ReferenceBuffer::kPath:
                    material = Material::GetMaterial ( mProperties.assembly ( i ).material().path() );
                    break;
                case ReferenceBuffer::kId:
                    material = Material::GetMaterial ( mProperties.assembly ( i ).material().id() );
                    break;
                default:
                    break;
                }
            }
            mMeshes.emplace_back ( mesh, pipeline, material );
        }

        mAnimations.reserve ( mProperties.animation_size() );
        for ( auto& animation : mProperties.animation() )
        {
            switch ( animation.reference_case() )
            {
            case ReferenceBuffer::kPath:
                mAnimations.emplace_back ( Animation::GetAnimation ( animation.path() ) );
                break;
            case ReferenceBuffer::kId:
                mAnimations.emplace_back ( Animation::GetAnimation ( animation.id() ) );
                break;
            default:
                break;
            }
        }
    }

    void ModelComponent::Render ( const Node& aNode, const Window& aWindow ) const
    {
        for ( auto& i : mMeshes )
        {
            if ( std::get<0> ( i ) && std::get<1> ( i ) )
            {
                aWindow.Render ( aNode.GetGlobalTransform(), *std::get<0> ( i ), *std::get<1> ( i ), std::get<2> ( i ).get() );
            }
        }
    }

    google::protobuf::Message* ModelComponent::GetProperties() const
    {
        return const_cast<ModelBuffer*> ( &mProperties );
    }
}
