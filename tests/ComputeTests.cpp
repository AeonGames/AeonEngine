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
#include <vector>
#include <cstring>
#include <exception>
#include <iostream>
#include "gtest/gtest.h"
#include "aeongames/AeonEngine.hpp"
#include "aeongames/Renderer.hpp"
#include "aeongames/Pipeline.hpp"
#include "aeongames/BufferAccessor.hpp"
#include "aeongames/Mesh.hpp"
#include "aeongames/CRC.hpp"
#include "aeongames/Matrix4x4.hpp"
#include "aeongames/GpuClusterParams.hpp"
#include "aeongames/GpuLight.hpp"
#ifdef _WIN32
#include <string_view>
#include "aeongames/Platform.hpp"
#ifdef AEON_TEST_HAVE_VULKAN
#include <vulkan/vulkan.h>
#endif
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

#ifdef AEON_TEST_HAVE_VULKAN
    /** @brief Probe whether the host has a usable Vulkan implementation.
     *
     *  Returns true only when a Vulkan instance can be created and at least one
     *  physical device is enumerable. Headless CI runners may ship a Vulkan
     *  loader yet have no compatible driver, in which case instance creation
     *  returns VK_ERROR_INCOMPATIBLE_DRIVER and this returns false so the
     *  GPU-dependent tests skip instead of crashing. */
    static bool IsVulkanAvailableOnHost()
    {
        VkApplicationInfo app_info{};
        app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        app_info.pApplicationName = "AeonComputeTests";
        app_info.applicationVersion = VK_MAKE_VERSION ( 1, 0, 0 );
        app_info.pEngineName = "AeonEngine";
        app_info.engineVersion = VK_MAKE_VERSION ( 1, 0, 0 );
        app_info.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        create_info.pApplicationInfo = &app_info;

        VkInstance instance = VK_NULL_HANDLE;
        if ( vkCreateInstance ( &create_info, nullptr, &instance ) != VK_SUCCESS )
        {
            return false;
        }

        uint32_t physical_device_count = 0;
        VkResult result = vkEnumeratePhysicalDevices ( instance, &physical_device_count, nullptr );
        vkDestroyInstance ( instance, nullptr );

        return result == VK_SUCCESS && physical_device_count > 0;
    }
#endif

    /** @brief Construct a renderer, returning nullptr (instead of throwing or
     *  aborting) when the backend is unavailable on the host.
     *
     *  Headless CI runners have no GPU/driver, so renderer construction fails:
     *  OpenGL throws (e.g. "Failed retrieving a pointer to wglGetExtensionsString")
     *  and Vulkan throws on VK_ERROR_INCOMPATIBLE_DRIVER. Swallowing the
     *  exception lets the GPU-dependent tests GTEST_SKIP() gracefully rather
     *  than fail the suite. */
    static std::unique_ptr<Renderer> TryConstructRenderer ( const char* aRendererName, void* aWindow )
    {
#ifdef AEON_TEST_HAVE_VULKAN
        // Some headless runners ship a Vulkan loader but no compatible driver.
        // On those hosts VulkanRenderer construction does not fail cleanly and
        // later dereferences a null instance, raising a Win32 SEH access
        // violation (0xc0000005) that GTest reports as a hard failure instead
        // of a skip. Probe for a usable Vulkan instance/device up front so we
        // can bail out before touching the broken backend.
        if ( std::string_view{ aRendererName } == "Vulkan" && !IsVulkanAvailableOnHost() )
        {
            std::cerr << aRendererName << " renderer unavailable on this host: no compatible Vulkan driver." << std::endl;
            return nullptr;
        }
#endif
        try
        {
            return ConstructRenderer ( std::string ( aRendererName ), aWindow );
        }
        catch ( const std::exception& e )
        {
            std::cerr << aRendererName << " renderer unavailable on this host: " << e.what() << std::endl;
            return nullptr;
        }
        catch ( ... )
        {
            std::cerr << aRendererName << " renderer unavailable on this host." << std::endl;
            return nullptr;
        }
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
        std::unique_ptr<Renderer> renderer = TryConstructRenderer ( aRendererName, hwnd );
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

        // Re-make the GL context current on this thread (OpenGL) so the buffer
        // can be mapped here, then drain the GPU so the compute writes are
        // complete before read-back. With multiple frames in flight BeginFrame
        // no longer waits on the previous frame, so Finish() is the sync point.
        renderer->BeginFrame ( hwnd );
        renderer->Finish ( hwnd );

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
        std::unique_ptr<Renderer> renderer = TryConstructRenderer ( aRendererName, hwnd );
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
        BufferAccessor counter_buffer =
            renderer->AllocateSingleFrameStorageMemory ( hwnd, sizeof ( uint32_t ) );
        // cluster_build clears the per-cluster active flags (Phase R2), so the
        // buffer must be bound even though this test does not read it back.
        BufferAccessor active_buffer =
            renderer->AllocateSingleFrameStorageMemory ( hwnd, CLUSTER_COUNT * sizeof ( uint32_t ) );
        const StorageBufferBinding bindings[]
        {
            { "ClusterAABBs"_crc32, &aabb_buffer },
            { "LightIndexCounter"_crc32, &counter_buffer },
            { "ClusterActive"_crc32, &active_buffer }
        };
        renderer->Dispatch ( hwnd, pipeline, group_count, 1, 1, bindings );
        renderer->Barrier ( hwnd );
        renderer->BeginRenderPass ( hwnd );
        renderer->EndRender ( hwnd );

        // Re-make the GL context current here, then Finish() to drain the GPU so
        // the compute writes are visible before read-back (with frames in flight
        // BeginFrame no longer waits on the previous frame).
        renderer->BeginFrame ( hwnd );
        renderer->Finish ( hwnd );

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

    /** @brief Dispatch shaders/cluster_build then shaders/light_cull on the
     *  given backend and verify the per-cluster light grid is populated.
     *
     *  A single point light is placed on the optical axis (engine +Y is the
     *  view-space depth axis) at a depth inside the frustum, with a radius
     *  large enough to overlap a band of clusters. cluster_build produces the
     *  AABBs, light_cull tests the light against each cluster and writes
     *  (offset, count) into LightGrid plus the light index into LightIndexList.
     *  We assert at least one cluster picked up the light, that no cell exceeds
     *  the fixed cap, and that the populated cells reference light index 0. */
    static void RunLightCullTest ( const char* aRendererName )
    {
        HWND hwnd = CreateHiddenRenderWindow();
        ASSERT_NE ( hwnd, nullptr );
        std::unique_ptr<Renderer> renderer = TryConstructRenderer ( aRendererName, hwnd );
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
        // Identity view matrix: view space == world space for this fixture.
        renderer->SetViewMatrix ( hwnd, Matrix4x4{} );

        // One point light on the optical axis (engine +Y is the depth axis),
        // mid-frustum, with a generous radius so it overlaps several clusters.
        GpuLight light{};
        light.position_radius = Vector4{ 0.0f, 10.0f, 0.0f, 5.0f };
        light.color_intensity = Vector4{ 1.0f, 1.0f, 1.0f, 1.0f };
        light.type = static_cast<uint32_t> ( LightType::Point );
        const GpuLight lights[] { light };
        renderer->SetLights ( hwnd, lights );

        constexpr uint32_t local_size = 64;
        const uint32_t group_count = ( CLUSTER_COUNT + local_size - 1 ) / local_size;

        Pipeline cluster_build;
        cluster_build.LoadFromId ( "shaders/cluster_build.txt"_crc32 );
        renderer->LoadPipeline ( cluster_build );
        Pipeline light_cull;
        light_cull.LoadFromId ( "shaders/light_cull.txt"_crc32 );
        renderer->LoadPipeline ( light_cull );

        renderer->BeginFrame ( hwnd );
        BufferAccessor aabb_buffer =
            renderer->AllocateSingleFrameStorageMemory ( hwnd, CLUSTER_COUNT * sizeof ( GpuClusterAABB ) );
        BufferAccessor grid_buffer =
            renderer->AllocateSingleFrameStorageMemory ( hwnd, CLUSTER_COUNT * sizeof ( GpuLightGridCell ) );
        BufferAccessor index_buffer =
            renderer->AllocateSingleFrameStorageMemory (
                hwnd, LIGHT_INDEX_LIST_CAPACITY * sizeof ( uint32_t ) );
        BufferAccessor counter_buffer =
            renderer->AllocateSingleFrameStorageMemory ( hwnd, sizeof ( uint32_t ) );
        // Per-cluster active flags: cluster_build clears them, light_cull reads
        // them when active-cluster culling is enabled (Phase R2).
        BufferAccessor active_buffer =
            renderer->AllocateSingleFrameStorageMemory ( hwnd, CLUSTER_COUNT * sizeof ( uint32_t ) );

        const StorageBufferBinding build_bindings[]
        {
            { "ClusterAABBs"_crc32, &aabb_buffer },
            { "LightIndexCounter"_crc32, &counter_buffer },
            { "ClusterActive"_crc32, &active_buffer }
        };
        renderer->Dispatch ( hwnd, cluster_build, group_count, 1, 1, build_bindings );
        renderer->Barrier ( hwnd );

        const StorageBufferBinding cull_bindings[]
        {
            { "ClusterAABBs"_crc32, &aabb_buffer },
            { "LightGrid"_crc32, &grid_buffer },
            { "LightIndexList"_crc32, &index_buffer },
            { "LightIndexCounter"_crc32, &counter_buffer },
            { "ClusterActive"_crc32, &active_buffer }
        };
        renderer->Dispatch ( hwnd, light_cull, group_count, 1, 1, cull_bindings );
        renderer->Barrier ( hwnd );
        renderer->BeginRenderPass ( hwnd );
        renderer->EndRender ( hwnd );

        // Re-make the GL context current here, then Finish() to drain the GPU so
        // the compute writes are visible before read-back (with frames in flight
        // BeginFrame no longer waits on the previous frame).
        renderer->BeginFrame ( hwnd );
        renderer->Finish ( hwnd );

        const GpuLightGridCell* grid = static_cast<const GpuLightGridCell*> ( grid_buffer.Map() );
        ASSERT_NE ( grid, nullptr );
        uint32_t total_lights = 0;
        uint32_t over_cap = 0;
        uint32_t bad_offset = 0;
        for ( uint32_t i = 0; i < CLUSTER_COUNT; ++i )
        {
            total_lights += grid[i].count;
            if ( grid[i].count > MAX_LIGHTS_PER_CLUSTER )
            {
                ++over_cap;
            }
            // Flat-index storage: each populated cell reserves a compact range
            // that stays inside the shared list (offset + count <= capacity).
            if ( grid[i].count > 0 &&
                 grid[i].offset + grid[i].count > LIGHT_INDEX_LIST_CAPACITY )
            {
                ++bad_offset;
            }
        }
        grid_buffer.Unmap();

        EXPECT_GT ( total_lights, 0u )
                << "no cluster picked up the single point light";
        EXPECT_EQ ( over_cap, 0u )
                << "a cluster exceeded MAX_LIGHTS_PER_CLUSTER";
        EXPECT_EQ ( bad_offset, 0u )
                << "a cluster reserved a range outside the flat light-index list";

        // The global allocator must have handed out exactly total_lights slots.
        const uint32_t* counter = static_cast<const uint32_t*> ( counter_buffer.Map() );
        ASSERT_NE ( counter, nullptr );
        EXPECT_EQ ( *counter, total_lights )
                << "flat light-index allocator count disagrees with per-cluster sums";
        counter_buffer.Unmap();

        renderer.reset();
        DestroyWindow ( hwnd );
    }

    /** @brief Dispatch the combined shaders/lighting pipeline, which carries
     *  both clustering compute stages (cluster-build then light-cull) as a
     *  single ordered multi-compute Pipeline, and verify the per-cluster light
     *  grid is populated.
     *
     *  This exercises the multi-stage path end to end: one Pipeline resource,
     *  two compute stages selected by index, sharing a merged descriptor-set
     *  layout. The fixture mirrors RunLightCullTest but loads a single pipeline
     *  instead of two separate ones. */
    static void RunCombinedLightingTest ( const char* aRendererName )
    {
        HWND hwnd = CreateHiddenRenderWindow();
        ASSERT_NE ( hwnd, nullptr );
        std::unique_ptr<Renderer> renderer = TryConstructRenderer ( aRendererName, hwnd );
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
        renderer->SetViewMatrix ( hwnd, Matrix4x4{} );

        GpuLight light{};
        light.position_radius = Vector4{ 0.0f, 10.0f, 0.0f, 5.0f };
        light.color_intensity = Vector4{ 1.0f, 1.0f, 1.0f, 1.0f };
        light.type = static_cast<uint32_t> ( LightType::Point );
        const GpuLight lights[] { light };
        renderer->SetLights ( hwnd, lights );

        constexpr uint32_t local_size = 64;
        const uint32_t group_count = ( CLUSTER_COUNT + local_size - 1 ) / local_size;

        // One pipeline resource carrying both compute stages in dispatch order.
        Pipeline lighting;
        lighting.LoadFromId ( "shaders/lighting.txt"_crc32 );
        ASSERT_EQ ( lighting.GetComputeStageCount(), 2u )
                << "combined lighting pipeline must carry two compute stages";
        renderer->LoadPipeline ( lighting );

        renderer->BeginFrame ( hwnd );
        BufferAccessor aabb_buffer =
            renderer->AllocateSingleFrameStorageMemory ( hwnd, CLUSTER_COUNT * sizeof ( GpuClusterAABB ) );
        BufferAccessor grid_buffer =
            renderer->AllocateSingleFrameStorageMemory ( hwnd, CLUSTER_COUNT * sizeof ( GpuLightGridCell ) );
        BufferAccessor index_buffer =
            renderer->AllocateSingleFrameStorageMemory (
                hwnd, LIGHT_INDEX_LIST_CAPACITY * sizeof ( uint32_t ) );
        BufferAccessor counter_buffer =
            renderer->AllocateSingleFrameStorageMemory ( hwnd, sizeof ( uint32_t ) );
        // Per-cluster active flags: stage 0 clears them, stage 1 reads them when
        // active-cluster culling is enabled (Phase R2).
        BufferAccessor active_buffer =
            renderer->AllocateSingleFrameStorageMemory ( hwnd, CLUSTER_COUNT * sizeof ( uint32_t ) );

        // All SSBOs are bound for every stage; reflection drops the
        // blocks a given stage does not declare (matching DispatchClustering).
        const StorageBufferBinding bindings[]
        {
            { "ClusterAABBs"_crc32, &aabb_buffer },
            { "LightGrid"_crc32, &grid_buffer },
            { "LightIndexList"_crc32, &index_buffer },
            { "LightIndexCounter"_crc32, &counter_buffer },
            { "ClusterActive"_crc32, &active_buffer }
        };

        // Stage 0: build the cluster AABBs.
        renderer->Dispatch ( hwnd, lighting, group_count, 1, 1, bindings, 0 );
        renderer->Barrier ( hwnd );
        // Stage 1: cull lights against those AABBs.
        renderer->Dispatch ( hwnd, lighting, group_count, 1, 1, bindings, 1 );
        renderer->Barrier ( hwnd );
        renderer->BeginRenderPass ( hwnd );
        renderer->EndRender ( hwnd );

        // Drain the GPU so the compute writes are visible before read-back
        // (BeginFrame no longer waits on the previous frame with frames in
        // flight); the BeginFrame re-makes the GL context current on this thread.
        renderer->BeginFrame ( hwnd );
        renderer->Finish ( hwnd );

        const GpuLightGridCell* grid = static_cast<const GpuLightGridCell*> ( grid_buffer.Map() );
        ASSERT_NE ( grid, nullptr );
        uint32_t total_lights = 0;
        uint32_t over_cap = 0;
        uint32_t bad_offset = 0;
        for ( uint32_t i = 0; i < CLUSTER_COUNT; ++i )
        {
            total_lights += grid[i].count;
            if ( grid[i].count > MAX_LIGHTS_PER_CLUSTER )
            {
                ++over_cap;
            }
            // Flat-index storage: each populated cell reserves a compact range
            // that stays inside the shared list (offset + count <= capacity).
            if ( grid[i].count > 0 &&
                 grid[i].offset + grid[i].count > LIGHT_INDEX_LIST_CAPACITY )
            {
                ++bad_offset;
            }
        }
        grid_buffer.Unmap();

        EXPECT_GT ( total_lights, 0u )
                << "no cluster picked up the single point light";
        EXPECT_EQ ( over_cap, 0u )
                << "a cluster exceeded MAX_LIGHTS_PER_CLUSTER";
        EXPECT_EQ ( bad_offset, 0u )
                << "a cluster reserved a range outside the flat light-index list";

        // The global allocator must have handed out exactly total_lights slots.
        const uint32_t* counter = static_cast<const uint32_t*> ( counter_buffer.Map() );
        ASSERT_NE ( counter, nullptr );
        EXPECT_EQ ( *counter, total_lights )
                << "flat light-index allocator count disagrees with per-cluster sums";
        counter_buffer.Unmap();

        renderer.reset();
        DestroyWindow ( hwnd );
    }

    /** @brief End-to-end depth-pre-pass active-cluster culling test.
     *
     *  Drives the full mark/cull frame lifecycle the application render loop
     *  uses: BeginRender(lighting) clears the per-cluster active flags and
     *  builds the cluster AABBs, the depth pre-pass renders a real mesh through
     *  the cluster_mark pipeline (each covered fragment flags its cluster
     *  active), EndDepthPrePass(lighting) runs light-cull with active-cluster
     *  gating (screen.w == 1), and EndRender presents.
     *
     *  With gating enabled the light grid may only assign lights to clusters the
     *  mark stage flagged active, so the asserted invariant is: every cluster
     *  holding one or more lights is active. The scene is also checked to be
     *  non-degenerate (some clusters active, some inactive, some lit) so the
     *  invariant cannot pass vacuously. */
    static void RunActiveClusterCullTest ( const char* aRendererName )
    {
        HWND hwnd = CreateHiddenRenderWindow();
        ASSERT_NE ( hwnd, nullptr );
        std::unique_ptr<Renderer> renderer = TryConstructRenderer ( aRendererName, hwnd );
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
        renderer->SetViewMatrix ( hwnd, Matrix4x4{} );

        // A single broad point light overlapping a wide swath of clusters. The
        // mark stage only flags the clusters the mesh actually covers, so the
        // gate must cull this light from every unmarked cluster it would
        // otherwise reach.
        GpuLight light{};
        light.position_radius = Vector4{ 0.0f, 10.0f, 0.0f, 30.0f };
        light.color_intensity = Vector4{ 1.0f, 1.0f, 1.0f, 1.0f };
        light.type = static_cast<uint32_t> ( LightType::Point );
        const GpuLight lights[] { light };
        renderer->SetLights ( hwnd, lights );

        // The combined lighting pipeline carries both clustering compute stages
        // and drives the BeginRender (cluster-build) / EndDepthPrePass
        // (light-cull) mark+cull lifecycle.
        Pipeline lighting;
        lighting.LoadFromId ( "shaders/lighting.txt"_crc32 );
        renderer->LoadPipeline ( lighting );

        // A real mesh whose vertex layout matches the cluster_mark pipeline
        // (position/normal/tangent/bitangent/uv). cube/suzanne carry only
        // position+normal and are incompatible with the mark pipeline.
        Mesh mesh;
        mesh.LoadFromId ( "polesign/meshes/PoleSign.txt"_crc32 );
        renderer->LoadMesh ( mesh );

        // Required graphics-pipeline argument to Render(); ignored while the
        // depth pre-pass substitutes the cluster_mark pipeline.
        Pipeline shading;
        shading.LoadFromId ( "shaders/clustered_phong.txt"_crc32 );
        renderer->LoadPipeline ( shading );

        // Place the mesh in front of the camera (camera at origin looking +Y).
        const Matrix4x4 model
        {
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 10.0f, 0.0f, 1.0f
        };

        // One full gated frame: mark stage (BeginRender + Render) then gated
        // light-cull (EndDepthPrePass) then present (EndRender).
        renderer->BeginRender ( hwnd, &lighting );
        renderer->Render ( hwnd, model, mesh, shading );
        renderer->EndDepthPrePass ( hwnd, &lighting );
        renderer->EndRender ( hwnd );

        // Re-make the GL context current here, then Finish() to drain the GPU so
        // the compute writes are visible before read-back (with frames in flight
        // BeginFrame no longer waits on the previous frame).
        renderer->BeginFrame ( hwnd );
        renderer->Finish ( hwnd );

        const BufferAccessor* grid_accessor = renderer->GetFrameLightGrid ( hwnd );
        const BufferAccessor* active_accessor = renderer->GetFrameClusterActive ( hwnd );
        ASSERT_NE ( grid_accessor, nullptr );
        ASSERT_NE ( active_accessor, nullptr );

        // The two frame buffers are sub-allocations of the same single-frame
        // storage pool, so they cannot be host-mapped simultaneously. Read each
        // into a host copy in turn before comparing them.
        std::vector<GpuLightGridCell> grid ( CLUSTER_COUNT );
        {
            const GpuLightGridCell* mapped =
                static_cast<const GpuLightGridCell*> ( grid_accessor->Map() );
            ASSERT_NE ( mapped, nullptr );
            std::copy ( mapped, mapped + CLUSTER_COUNT, grid.begin() );
            grid_accessor->Unmap();
        }
        std::vector<uint32_t> active ( CLUSTER_COUNT );
        {
            const uint32_t* mapped =
                static_cast<const uint32_t*> ( active_accessor->Map() );
            ASSERT_NE ( mapped, nullptr );
            std::copy ( mapped, mapped + CLUSTER_COUNT, active.begin() );
            active_accessor->Unmap();
        }

        uint32_t lit_clusters = 0;
        uint32_t active_clusters = 0;
        uint32_t inactive_clusters = 0;
        uint32_t leaked_clusters = 0;
        for ( uint32_t i = 0; i < CLUSTER_COUNT; ++i )
        {
            const bool is_active = active[i] != 0u;
            const bool is_lit = grid[i].count > 0;
            active_clusters += is_active ? 1u : 0u;
            inactive_clusters += is_active ? 0u : 1u;
            lit_clusters += is_lit ? 1u : 0u;
            // Gate invariant: a light may only land in an active cluster.
            if ( is_lit && !is_active )
            {
                ++leaked_clusters;
            }
        }

        EXPECT_EQ ( leaked_clusters, 0u )
                << "a light was assigned to a cluster the mark stage left inactive";
        EXPECT_GT ( active_clusters, 0u )
                << "the mesh marked no clusters active in the depth pre-pass";
        EXPECT_GT ( inactive_clusters, 0u )
                << "every cluster was marked active; the gate had nothing to cull";
        EXPECT_GT ( lit_clusters, 0u )
                << "the light reached no marked cluster; setup is degenerate";

        renderer.reset();
        DestroyWindow ( hwnd );
    }

    /** @brief Reinterpret a float's bits as an unsigned 32-bit word, matching the
     *  raw-word layout the skinning compute shader reads. */
    static uint32_t FloatBits ( float aValue )
    {
        uint32_t bits = 0;
        std::memcpy ( &bits, &aValue, sizeof ( bits ) );
        return bits;
    }

    /** @brief Reinterpret a raw 32-bit word as a float (skinning readback). */
    static float BitsFloat ( uint32_t aBits )
    {
        float value = 0.0f;
        std::memcpy ( &value, &aBits, sizeof ( value ) );
        return value;
    }

    /** @brief Dispatch shaders/skinning over a synthetic skinned vertex buffer
     *  and verify the GPU produced the expected linear-blend skinned vertices.
     *
     *  The source uses the standard 64-byte skinned vertex layout (position,
     *  normal, tangent, bitangent, uv, packed weight indices, packed weights).
     *  Every vertex is fully weighted to joint 0, whose matrix is a pure
     *  translation, so the expected result is position + translation with the
     *  directional attributes (identity 3x3) and the uv words left unchanged.
     *  The skinning output drops the weight data, producing the compact 56-byte
     *  (14-word) layout the no-skeleton draw pipeline consumes. This proves the
     *  compute-skinning math, the interleaved word unpacking and the
     *  source/skeleton/output SSBO binding path end to end. */
    static void RunSkinningTest ( const char* aRendererName )
    {
        HWND hwnd = CreateHiddenRenderWindow();
        ASSERT_NE ( hwnd, nullptr );
        std::unique_ptr<Renderer> renderer = TryConstructRenderer ( aRendererName, hwnd );
        if ( renderer == nullptr )
        {
            DestroyWindow ( hwnd );
            GTEST_SKIP() << aRendererName << " renderer unavailable on this host.";
        }
        renderer->ResizeViewport ( hwnd, 0, 0, 64, 64 );

        constexpr uint32_t local_size = 64;
        constexpr uint32_t vertex_words = 16; // 64-byte source vertex stride.
        constexpr uint32_t skinned_words = 14; // 56-byte compact skinned stride.
        constexpr uint32_t vertex_count = 100; // spans two work groups.

        // Joint 0 is a pure translation; its 3x3 part is identity.
        const float tx = 2.0f, ty = -3.0f, tz = 0.5f;
        const float joint0[16] =
        {
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            tx,   ty,   tz,   1.0f
        };

        // Build the synthetic source vertex buffer.
        std::vector<uint32_t> source ( vertex_count * vertex_words, 0u );
        for ( uint32_t v = 0; v < vertex_count; ++v )
        {
            uint32_t* vert = source.data() + v * vertex_words;
            // Position varies per vertex so a missed translation is visible.
            vert[0] = FloatBits ( static_cast<float> ( v ) );
            vert[1] = FloatBits ( static_cast<float> ( v ) * 2.0f );
            vert[2] = FloatBits ( static_cast<float> ( v ) * 3.0f );
            // Normal / tangent / bitangent: fixed orthonormal basis.
            vert[3] = FloatBits ( 0.0f );
            vert[4] = FloatBits ( 0.0f );
            vert[5] = FloatBits ( 1.0f );
            vert[6] = FloatBits ( 1.0f );
            vert[7] = FloatBits ( 0.0f );
            vert[8] = FloatBits ( 0.0f );
            vert[9]  = FloatBits ( 0.0f );
            vert[10] = FloatBits ( 1.0f );
            vert[11] = FloatBits ( 0.0f );
            // UV.
            vert[12] = FloatBits ( 0.25f );
            vert[13] = FloatBits ( 0.75f );
            // Weight indices: all four influences reference joint 0.
            vert[14] = 0u;
            // Weight values: x = 255/255 = 1.0, the rest 0 (unpackUnorm4x8).
            vert[15] = 0x000000FFu;
        }

        renderer->BeginFrame ( hwnd );

        const size_t source_size = source.size() * sizeof ( uint32_t );
        BufferAccessor source_buffer = renderer->AllocateSingleFrameStorageMemory ( hwnd, source_size );
        source_buffer.WriteMemory ( 0, source_size, source.data() );

        BufferAccessor matrices_buffer = renderer->AllocateSingleFrameStorageMemory ( hwnd, sizeof ( joint0 ) );
        matrices_buffer.WriteMemory ( 0, sizeof ( joint0 ), joint0 );

        BufferAccessor skinned_buffer = renderer->AllocateSingleFrameStorageMemory ( hwnd, vertex_count * skinned_words * sizeof ( uint32_t ) );

        const StorageBufferBinding bindings[]
        {
            { Mesh::BindingLocations::SKINNING_MATRICES, &matrices_buffer },
            { Mesh::BindingLocations::SOURCE_VERTICES, &source_buffer },
            { Mesh::BindingLocations::SKINNED_VERTICES, &skinned_buffer },
        };

        Pipeline skinning;
        skinning.LoadFromId ( "shaders/skinning.txt"_crc32 );
        renderer->LoadPipeline ( skinning );

        const uint32_t group_count = ( vertex_count + local_size - 1 ) / local_size;
        renderer->Dispatch ( hwnd, skinning, group_count, 1, 1, bindings );
        renderer->Barrier ( hwnd );
        renderer->BeginRenderPass ( hwnd );
        renderer->EndRender ( hwnd );

        // Drain the GPU so the compute writes are visible before read-back
        // (BeginFrame no longer waits on the previous frame with frames in
        // flight); the BeginFrame re-makes the GL context current on this thread.
        renderer->BeginFrame ( hwnd );
        renderer->Finish ( hwnd );

        const uint32_t* out = static_cast<const uint32_t*> ( skinned_buffer.Map() );
        ASSERT_NE ( out, nullptr );
        for ( uint32_t v = 0; v < vertex_count; ++v )
        {
            const uint32_t* vert = out + v * skinned_words;
            // Position translated by joint 0.
            EXPECT_NEAR ( BitsFloat ( vert[0] ), static_cast<float> ( v ) + tx, 1e-4f )
                    << "position.x mismatch at vertex " << v;
            EXPECT_NEAR ( BitsFloat ( vert[1] ), static_cast<float> ( v ) * 2.0f + ty, 1e-4f )
                    << "position.y mismatch at vertex " << v;
            EXPECT_NEAR ( BitsFloat ( vert[2] ), static_cast<float> ( v ) * 3.0f + tz, 1e-4f )
                    << "position.z mismatch at vertex " << v;
            // Directional attributes pass through a pure translation unchanged.
            EXPECT_NEAR ( BitsFloat ( vert[5] ), 1.0f, 1e-5f ) << "normal.z at vertex " << v;
            EXPECT_NEAR ( BitsFloat ( vert[6] ), 1.0f, 1e-5f ) << "tangent.x at vertex " << v;
            EXPECT_NEAR ( BitsFloat ( vert[10] ), 1.0f, 1e-5f ) << "bitangent.y at vertex " << v;
            // UV is copied verbatim; the weight words are dropped from output.
            EXPECT_EQ ( vert[12], FloatBits ( 0.25f ) ) << "uv.x at vertex " << v;
            EXPECT_EQ ( vert[13], FloatBits ( 0.75f ) ) << "uv.y at vertex " << v;
        }
        skinned_buffer.Unmap();

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

    TEST ( ComputeTest, OpenGLLightCull )
    {
#ifdef _WIN32
        RunLightCullTest ( "OpenGL" );
#else
        GTEST_SKIP() << "Light cull test requires Win32 windowing.";
#endif
    }

    TEST ( ComputeTest, VulkanLightCull )
    {
#ifdef _WIN32
        RunLightCullTest ( "Vulkan" );
#else
        GTEST_SKIP() << "Light cull test requires Win32 windowing.";
#endif
    }

    TEST ( ComputeTest, OpenGLCombinedLighting )
    {
#ifdef _WIN32
        RunCombinedLightingTest ( "OpenGL" );
#else
        GTEST_SKIP() << "Combined lighting test requires Win32 windowing.";
#endif
    }

    TEST ( ComputeTest, VulkanCombinedLighting )
    {
#ifdef _WIN32
        RunCombinedLightingTest ( "Vulkan" );
#else
        GTEST_SKIP() << "Combined lighting test requires Win32 windowing.";
#endif
    }

    TEST ( ComputeTest, OpenGLActiveClusterCull )
    {
#ifdef _WIN32
        RunActiveClusterCullTest ( "OpenGL" );
#else
        GTEST_SKIP() << "Active-cluster cull test requires Win32 windowing.";
#endif
    }

    TEST ( ComputeTest, VulkanActiveClusterCull )
    {
#ifdef _WIN32
        RunActiveClusterCullTest ( "Vulkan" );
#else
        GTEST_SKIP() << "Active-cluster cull test requires Win32 windowing.";
#endif
    }

    TEST ( ComputeTest, OpenGLSkinning )
    {
#ifdef _WIN32
        RunSkinningTest ( "OpenGL" );
#else
        GTEST_SKIP() << "Skinning test requires Win32 windowing.";
#endif
    }

    TEST ( ComputeTest, VulkanSkinning )
    {
#ifdef _WIN32
        RunSkinningTest ( "Vulkan" );
#else
        GTEST_SKIP() << "Skinning test requires Win32 windowing.";
#endif
    }
}
