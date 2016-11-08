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
#if 0
        /* We don't really want to be copying and moving engine objects... or do we?*/
        /// Move
        DLL AeonEngine ( AeonEngine&& aRhs ) noexcept;
        DLL AeonEngine& operator= ( AeonEngine&& aRhs ) noexcept;
        /// Copy
        DLL AeonEngine ( const AeonEngine& aRhs );
        DLL AeonEngine& operator= ( const AeonEngine& aRhs );
#else
        /// Move
        AeonEngine ( AeonEngine&& ) noexcept = delete;
        AeonEngine& operator= ( AeonEngine&& ) noexcept = delete;
        /// Copy
        AeonEngine ( const AeonEngine& ) = delete;
        AeonEngine& operator= ( const AeonEngine& ) = delete;
#endif
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
        DLL std::shared_ptr<Mesh> GetMesh ( const std::string& aFilename ) const;
        DLL std::shared_ptr<Program> GetProgram ( const std::string& aFilename ) const;
        DLL void SetScene ( Scene* aScene );
        DLL Scene* GetScene() const;

        DLL bool RegisterRenderingWindow ( uintptr_t aWindowId );
        DLL void UnregisterRenderingWindow ( uintptr_t aWindowId );
        DLL void Resize ( uintptr_t aWindowId, uint32_t aWidth, uint32_t aHeight ) const;

        DLL void SetProjectionMatrix ( const float aMatrix[16] );
        DLL void SetViewMatrix ( const float aMatrix[16] );

    private:
        struct Impl;
        std::unique_ptr<Impl> pImpl;
    };
}
#endif
