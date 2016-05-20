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
#include <memory>
#include <string>

namespace AeonGames
{
    class Mesh;
    class Scene;
    class AeonEngine
    {
    public:
        /// Construct
        DLL AeonEngine();
        /// Destruct
        DLL ~AeonEngine();
        /// Move
        DLL AeonEngine ( AeonEngine&& aRhs ) noexcept;
        DLL AeonEngine& operator= ( AeonEngine&& aRhs ) noexcept;
        /// Copy
        DLL AeonEngine ( const AeonEngine& aRhs );
        DLL AeonEngine& operator= ( const AeonEngine& aRhs );
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
        DLL void SetScene ( Scene* aScene );
        DLL Scene* GetScene() const;

#ifdef _WIN32
        DLL bool InitializeRenderingWindow ( HINSTANCE aInstance, HWND aHwnd );
#else
        DLL bool InitializeRenderingWindow ( Display* aDisplay, Window aWindow );
#endif
        DLL void FinalizeRenderingWindow();
    private:
        struct Impl;
        std::unique_ptr<Impl> pImpl;
    };
}
#endif
