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
#include <memory>
#include <google/protobuf/stubs/common.h>
#include "aeongames/AeonEngine.h"
#include "renderers/vulkan/VulkanRenderer.h"
#include "renderers/opengl/OpenGLRenderer.h"
#include "scenegraph/Scene.h"
#include "GameWindow.h"

#if __cplusplus < 201300L && __cplusplus >= 201103L
// Taken from EMC++ Item 21
namespace std
{
    template<typename T, typename... Ts>
    std::unique_ptr<T> make_unique ( Ts&&... params )
    {
        return std::unique_ptr<T> ( new T ( std::forward<Ts> ( params )... ) );
    }
}
#endif

namespace AeonGames
{
    /*  This is a Hack so google::protobuf::ShutdownProtobufLibrary gets called on process exit.
        This is nice because no reference count is required in the engine class,
        there is no need for free form initialization/finalization functions
        and client code needs not even know that the protobuffers library needs to be
        shutdown before exit.
        Also, it is platform independent since there is no DLLMain on Linux. */
    static bool gShutdownProtobufLibraryAtExit = std::atexit ( google::protobuf::ShutdownProtobufLibrary ) == 0;

    struct AeonEngine::Impl
    {
        //VulkanRenderer mVulkanRenderer;
        OpenGLRenderer mRenderer;
        Scene* mScene;
    };

    AeonEngine::AeonEngine() :
        pImpl ( std::make_unique<Impl>() )
    {
    }

    AeonEngine::~AeonEngine() = default;

    AeonEngine::AeonEngine ( AeonEngine && aRhs ) noexcept = default;

    AeonEngine & AeonEngine::operator= ( AeonEngine && aRhs ) noexcept = default;

    AeonEngine::AeonEngine ( const AeonEngine & aRhs )
        : pImpl ( nullptr )
    {
        if ( aRhs.pImpl )
        {
            pImpl = std::make_unique<Impl> ( *aRhs.pImpl );
        }
    }

    AeonEngine & AeonEngine::operator= ( const AeonEngine & aRhs )
    {
        if ( !aRhs.pImpl )
        {
            pImpl.reset();
        }
        else if ( !pImpl )
        {
            pImpl = std::make_unique<Impl> ( *aRhs.pImpl );
        }
        else
        {
            *pImpl = *aRhs.pImpl;
        }
        return *this;
    }

    void AeonEngine::Step ( double aDeltaTime )
    {
        pImpl->mRenderer.BeginRender();
        if ( pImpl->mScene != nullptr )
        {
            //pImpl->mScene->
            //pImpl->mScene->Render();
            //pImpl->mRenderer.Render(aDeltaTime);
        }
        pImpl->mRenderer.EndRender();
    }

    int AeonEngine::Run()
    {
        GameWindow game_window ( *this );
        return game_window.Run();
    }

    std::shared_ptr<Mesh> AeonEngine::GetMesh ( const std::string & aFilename ) const
    {
        return pImpl->mRenderer.GetMesh ( aFilename );
    }

    void AeonEngine::SetScene ( Scene * aScene )
    {
        pImpl->mScene = aScene;
    }

    Scene * AeonEngine::GetScene() const
    {
        return pImpl->mScene;
    }

#if _WIN32
    bool AeonEngine::InitializeRenderingWindow ( HINSTANCE aInstance, HWND aHwnd )
    {
        return pImpl->mRenderer.InitializeRenderingWindow ( aInstance, aHwnd );
    }
#else
    bool AeonEngine::InitializeRenderingWindow ( Display* aDisplay, Window aWindow )
    {
        return pImpl->mOpenGLRenderer.InitializeRenderingWindow ( aDisplay, aWindow );
    }
#endif

    void AeonEngine::FinalizeRenderingWindow()
    {
        return pImpl->mRenderer.FinalizeRenderingWindow();
    }
}
