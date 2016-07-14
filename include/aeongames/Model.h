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
#include <memory>
#include "aeongames/Node.h"
#include "aeongames/Platform.h"
//#include "aeongames/Program.h"

namespace AeonGames
{
    class Program;
    class Mesh;
    class Model : public Node
    {
    public:
        DLL Model();
        DLL ~Model() override;
        DLL void SetMesh ( const std::shared_ptr<Mesh>& aMesh );
        DLL void SetProgram ( const std::shared_ptr<Program>& aProgram );
        DLL void Update ( const double delta ) final;
        DLL void Render ( Renderer* aRenderer ) final;
    private:
        std::shared_ptr<Mesh> mMesh;
        std::shared_ptr<Program> mProgram;
    };
}
#endif
