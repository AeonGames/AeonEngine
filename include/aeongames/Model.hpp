/*
Copyright (C) 2018,2021,2025,2026 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_MODEL_H
#define AEONGAMES_MODEL_H
#include <vector>
#include <tuple>
#include <memory>
#include "aeongames/Platform.hpp"
#include "aeongames/ResourceId.hpp"
#include "aeongames/Resource.hpp"

namespace AeonGames
{
    class Mesh;
    class Pipeline;
    class Material;
    class Skeleton;
    class Animation;
    class ModelMsg;
    /** @brief Represents a 3D model composed of assemblies (pipeline, material, mesh), a skeleton, and animations. */
    class Model : public Resource
    {
    public:
        /** @brief A tuple of ResourceIds representing a pipeline, material, and mesh combination. */
        using Assembly = typename std::tuple <
                         ResourceId,
                         ResourceId,
                         ResourceId >;
        DLL Model();
        DLL ~Model();
        /** @brief Load model data from a protobuf message.
         *  @param aModelMsg Protobuf message containing model data. */
        DLL void LoadFromPBMsg ( const ModelMsg& aModelMsg );
        /** @brief Load model data from a memory buffer.
         *  @param aBuffer Pointer to the buffer containing model data.
         *  @param aBufferSize Size of the buffer in bytes. */
        DLL void LoadFromMemory ( const void* aBuffer, size_t aBufferSize ) final;
        /** @brief Unload model data and free resources. */
        DLL void Unload () final;
        /** @brief Get the list of assemblies that compose this model.
         *  @return Reference to the vector of assemblies. */
        DLL const std::vector<Assembly>& GetAssemblies() const;
        /** @brief Get the skeleton associated with this model.
         *  @return Pointer to the skeleton, or nullptr if none. */
        DLL const Skeleton* GetSkeleton() const;
        /** @brief Get the list of animation resource identifiers.
         *  @return Reference to the vector of animation ResourceIds. */
        DLL const std::vector<ResourceId>& GetAnimations() const;
        /** @brief Load renderer-specific resources for this model.
         *  @param aRenderer reference to the renderer. */
        DLL void LoadRendererResources ( Renderer& aRenderer ) const;
        /** @brief Unload renderer-specific resources for this model.
         *  @param aRenderer reference to the renderer. */
        DLL void UnloadRendererResources ( Renderer& aRenderer ) const;
    private:
        ResourceId mSkeleton;
        std::vector<Assembly> mAssemblies{};
        std::vector<ResourceId> mAnimations;
    };
}
#endif
