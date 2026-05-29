/*
Copyright (C) 2026 Rodrigo Jose Hernandez Cordoba

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
#include <cstdint>
#include <memory>
#include <string>
#include "gtest/gtest.h"
#include "aeongames/AeonEngine.hpp"
#include "aeongames/Renderer.hpp"
#include "aeongames/Pipeline.hpp"
#include "aeongames/BufferAccessor.hpp"
#include "aeongames/CRC.hpp"
#ifdef _WIN32
#include "aeongames/Platform.hpp"
#endif

using namespace ::testing;
namespace AeonGames
{
#ifdef _WIN32
    /** @brief Create an off-screen pop-up window suitable for hosting a renderer
     *  surface without ever being shown. */
    static HWND CreateHiddenRenderWindow()
    {
        WNDCLASSEX wcex{};
        wcex.cbSize = sizeof ( WNDCLASSEX );
        wcex.style = CS_OWNDC;
        wcex.lpfnWndProc = DefWindowProc;
        wcex.hInstance = GetModuleHandle ( nullptr );
        wcex.lpszClassName = "AeonComputeTestWindow";
        RegisterClassEx ( &wcex );
        return CreateWindowEx ( 0, "AeonComputeTestWindow", "AeonComputeTest",
                                WS_POPUP, 0, 0, 64, 64, nullptr, nullptr,
                                GetModuleHandle ( nullptr ), nullptr );
    }

    /** @brief Dispatch shaders/noop_compute on the given backend and verify that
     *  the shader wrote each invocation's global index into an output SSBO.
     *
     *  The pipeline writes values[i] = i, so a correct round trip through the
     *  storage-buffer binding path produces an identity sequence on readback. */
    static void RunNoopComputeSmokeTest ( const char* aRendererName )
    {
        HWND hwnd = CreateHiddenRenderWindow();
        ASSERT_NE ( hwnd, nullptr );
        std::unique_ptr<Renderer> renderer = ConstructRenderer ( std::string ( aRendererName ), hwnd );
        if ( renderer == nullptr )
        {
            DestroyWindow ( hwnd );
            GTEST_SKIP() << aRendererName << " renderer unavailable on this host.";
        }
        // The hidden window never receives WM_SIZE, so size the swapchain and
        // viewport explicitly before recording any frame.
        renderer->ResizeViewport ( hwnd, 0, 0, 64, 64 );

        constexpr uint32_t local_size = 64;
        constexpr uint32_t element_count = 256;

        Pipeline pipeline;
        pipeline.LoadFromId ( "shaders/noop_compute.txt"_crc32 );
        renderer->LoadPipeline ( pipeline );

        renderer->BeginFrame ( hwnd );
        BufferAccessor ssbo = renderer->AllocateSingleFrameStorageMemory ( hwnd, element_count * sizeof ( uint32_t ) );
        const StorageBufferBinding bindings[] { { "Output"_crc32, &ssbo } };
        renderer->Dispatch ( hwnd, pipeline, element_count / local_size, 1, 1, bindings );
        renderer->Barrier ( hwnd );
        renderer->BeginRenderPass ( hwnd );
        renderer->EndRender ( hwnd );

        // Begin a second frame: this waits on the previous frame's completion
        // (the Vulkan fence) and makes the GPU writes visible, and re-makes the
        // OpenGL context current so the buffer can be mapped on this thread.
        renderer->BeginFrame ( hwnd );

        const uint32_t* data = static_cast<const uint32_t*> ( ssbo.Map() );
        ASSERT_NE ( data, nullptr );
        for ( uint32_t i = 0; i < element_count; ++i )
        {
            EXPECT_EQ ( data[i], i ) << "compute output mismatch at index " << i;
        }
        ssbo.Unmap();

        renderer.reset();
        DestroyWindow ( hwnd );
    }
#endif

    TEST ( ComputeTest, OpenGLNoopCompute )
    {
#ifdef _WIN32
        RunNoopComputeSmokeTest ( "OpenGL" );
#else
        GTEST_SKIP() << "Compute smoke test requires Win32 windowing.";
#endif
    }

    TEST ( ComputeTest, VulkanNoopCompute )
    {
#ifdef _WIN32
        RunNoopComputeSmokeTest ( "Vulkan" );
#else
        GTEST_SKIP() << "Compute smoke test requires Win32 windowing.";
#endif
    }
}
