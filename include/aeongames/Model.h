/*
Copyright 2016 Rodrigo Jose Hernandez Cordoba

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
    class Program;
    class Material;
    class Mesh;
    class Model
    {
    public:
        DLL Model ( const std::string& aFilename );
        DLL ~Model();
    private:
        std::string mFilename;
        std::shared_ptr<Program> mProgram;
        std::shared_ptr<Material> mMaterial;
        std::shared_ptr<Mesh> mMesh;
        void Initialize();
        void Finalize();
    };
}
#endif
