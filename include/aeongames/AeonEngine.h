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
#ifndef AEONGAMES_AEONENGINE_H
#define AEONGAMES_AEONENGINE_H
#include "Platform.h"
#include "Memory.h"
#include <string>

namespace AeonGames
{
    DLL bool Initialize();
    DLL void Finalize();

    class Scene;
    class Mesh;
    class Material;
    class Program;
    class AeonEngine
    {
    public:
        /// Construct
        DLL AeonEngine();
        /// Destruct
        DLL ~AeonEngine();
        /// No Move
        AeonEngine ( AeonEngine&& ) noexcept = delete;
        AeonEngine& operator= ( AeonEngine&& ) noexcept = delete;
        /// No Copy
        AeonEngine ( const AeonEngine& ) = delete;
        AeonEngine& operator= ( const AeonEngine& ) = delete;
        /**
        Advance the simulation a single step.
        @param aDeltaTime Time delta to advance the simulation.
        */
        DLL void Step ( double aDeltaTime );
        /**
        Run standalone.
        @return program return value.
        */
        DLL int Run();
        DLL void SetScene ( Scene* aScene );
        DLL Scene* GetScene() const;
    private:
        struct Impl;
        std::unique_ptr<Impl> pImpl;
    };
}
#endif
