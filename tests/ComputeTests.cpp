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
#include <algorithm>
#include "gtest/gtest.h"
#include "aeongames/AeonEngine.hpp"
#include "aeongames/Renderer.hpp"
#include "aeongames/Pipeline.hpp"
#include "aeongames/BufferAccessor.hpp"
#include "aeongames/CRC.hpp"
#include "aeongames/Matrix4x4.hpp"
#include "aeongames/GpuClusterParams.hpp"
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

    /** @brief Dispatch shaders/cluster_build on the given backend and verify the
     *  generated per-cluster view-space AABBs are sane.
     *
     *  cluster_build derives the near/far depth (view-space Y in this engine)
     *  from the inverse projection and slices the frustum logarithmically. With
     *  a known perspective projection we can assert that every cluster AABB is
     *  non-degenerate and that the grid as a whole spans [near, far] in depth. */
    static void RunClusterBuildTest ( const char* aRendererName )
    {
        HWND hwnd = CreateHiddenRenderWindow();
        ASSERT_NE ( hwnd, nullptr );
        std::unique_ptr<Renderer> renderer = ConstructRenderer ( std::string ( aRendererName ), hwnd );
        if ( renderer == nullptr )
        {
            DestroyWindow ( hwnd );
            GTEST_SKIP() << aRendererName << " renderer unavailable on this host.";
        }
        renderer->ResizeViewport ( hwnd, 0, 0, 64, 64 );

        constexpr float near_plane = 1.0f;
        constexpr float far_plane = 100.0f;
        Matrix4x4 projection{};
        projection.Perspective ( 90.0f, 1.0f, near_plane, far_plane );
        renderer->SetProjectionMatrix ( hwnd, projection );

        constexpr uint32_t local_size = 64;
        const uint32_t group_count = ( CLUSTER_COUNT + local_size - 1 ) / local_size;

        Pipeline pipeline;
        pipeline.LoadFromId ( "shaders/cluster_build.txt"_crc32 );
        renderer->LoadPipeline ( pipeline );

        renderer->BeginFrame ( hwnd );
        BufferAccessor aabb_buffer =
            renderer->AllocateSingleFrameStorageMemory ( hwnd, CLUSTER_COUNT * sizeof ( GpuClusterAABB ) );
        const StorageBufferBinding bindings[] { { "ClusterAABBs"_crc32, &aabb_buffer } };
        renderer->Dispatch ( hwnd, pipeline, group_count, 1, 1, bindings );
        renderer->Barrier ( hwnd );
        renderer->BeginRenderPass ( hwnd );
        renderer->EndRender ( hwnd );

        // Second BeginFrame waits on the previous frame's fence (Vulkan) and
        // re-makes the GL context current so the buffer can be mapped here.
        renderer->BeginFrame ( hwnd );

        const GpuClusterAABB* aabbs = static_cast<const GpuClusterAABB*> ( aabb_buffer.Map() );
        ASSERT_NE ( aabbs, nullptr );

        float global_min_depth = far_plane * 1000.0f;
        float global_max_depth = -far_plane * 1000.0f;
        uint32_t degenerate = 0;
        for ( uint32_t i = 0; i < CLUSTER_COUNT; ++i )
        {
            const Vector4& lo = aabbs[i].min_point;
            const Vector4& hi = aabbs[i].max_point;
            // Every component of max must be >= the matching component of min.
            if ( ! ( hi.GetX() >= lo.GetX() && hi.GetY() >= lo.GetY() && hi.GetZ() >= lo.GetZ() ) )
            {
                ++degenerate;
            }
            global_min_depth = std::min ( global_min_depth, lo.GetY() );
            global_max_depth = std::max ( global_max_depth, hi.GetY() );
        }
        aabb_buffer.Unmap();

        EXPECT_EQ ( degenerate, 0u ) << "found degenerate cluster AABBs";
        // The grid's depth extent (view-space Y) should span the frustum.
        EXPECT_NEAR ( global_min_depth, near_plane, near_plane * 0.05f )
                << "near depth of cluster grid does not match the projection";
        EXPECT_NEAR ( global_max_depth, far_plane, far_plane * 0.05f )
                << "far depth of cluster grid does not match the projection";

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

    TEST ( ComputeTest, OpenGLClusterBuild )
    {
#ifdef _WIN32
        RunClusterBuildTest ( "OpenGL" );
#else
        GTEST_SKIP() << "Cluster build test requires Win32 windowing.";
#endif
    }

    TEST ( ComputeTest, VulkanClusterBuild )
    {
#ifdef _WIN32
        RunClusterBuildTest ( "Vulkan" );
#else
        GTEST_SKIP() << "Cluster build test requires Win32 windowing.";
#endif
    }
}
