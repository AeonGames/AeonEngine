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
#ifndef AEONGAMES_MODEL_H
#define AEONGAMES_MODEL_H
#include <vector>
#include <tuple>
#include <memory>
#include "aeongames/Platform.h"
#include "aeongames/ResourceId.h"

namespace AeonGames
{
    class Mesh;
    class Pipeline;
    class Material;
    class Skeleton;
    class Animation;
    class ModelBuffer;
    class Model
    {
    public:
        using Assembly = typename std::tuple <
                         ResourceId,
                         ResourceId,
                         ResourceId >;
        DLL Model();
        DLL Model ( uint32_t aId );
        DLL Model ( const std::string& aFilename );
        DLL Model ( const void* aBuffer, size_t aBufferSize );
        DLL ~Model();
        DLL void Load ( uint32_t aId );
        DLL void Load ( const std::string& aFilename );
        DLL void Load ( const void* aBuffer, size_t aBufferSize );
        DLL void Unload ();
        DLL const std::vector<Assembly>& GetAssemblies() const;
        DLL const Skeleton* GetSkeleton() const;
        DLL const std::vector<ResourceId>& GetAnimations() const;
    private:
        void Load ( const ModelBuffer& aModelBuffer );
        ResourceId mSkeleton;
        std::vector<Assembly> mAssemblies{};
        std::vector<ResourceId> mAnimations;
    };
}
#endif
