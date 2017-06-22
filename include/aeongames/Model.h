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
#ifndef AEONGAMES_MODEL_H
#define AEONGAMES_MODEL_H
#include "aeongames/Memory.h"
#include <vector>
#include "aeongames/Platform.h"

namespace AeonGames
{
    class Pipeline;
    class Material;
    class Mesh;
    class Model
    {
    public:
        DLL Model ( std::string  aFilename );
        DLL ~Model();
        DLL const std::string& GetFilename() const;
        DLL const std::vector<std::tuple<
        std::shared_ptr<Pipeline>,
            std::shared_ptr<Material>,
            std::shared_ptr<Mesh>>>& GetMeshes() const;
        DLL const float * const GetCenterRadii() const;
    private:
        float mCenterRadii[6];
        std::string mFilename;
        std::vector<std::tuple<
        std::shared_ptr<Pipeline>,
            std::shared_ptr<Material>,
            std::shared_ptr<Mesh>>
            > mMeshes;
        void Initialize();
        void Finalize();
    };
}
#endif
