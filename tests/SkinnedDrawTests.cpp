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
#include <cstring>
#include <memory>
#include <string>
#include <vector>
#include "gtest/gtest.h"
#include "aeongames/AeonEngine.hpp"
#include "aeongames/Renderer.hpp"
#include "aeongames/Pipeline.hpp"
#include "aeongames/Mesh.hpp"
#include "aeongames/Matrix4x4.hpp"
#include "aeongames/BufferAccessor.hpp"
#include "aeongames/Texture.hpp"
#include "aeongames/CRC.hpp"
#include "aeongames/ProtoBufClasses.hpp"
#include "mesh.pb.h"
#include "RenderTestWindow.h"

using namespace ::testing;
namespace AeonGames
{
    namespace
    {
        /// Source vertices carry the weighted 64-byte layout the skinning pass reads.
        constexpr uint32_t kSourceStride = 64;
        /// Quad corners in NDC; z sits mid-range so it passes either depth convention.
        constexpr float kQuadExtent = 0.8f;
        constexpr float kQuadDepth = 0.5f;

        void WriteFloat ( std::vector<uint8_t>& aBuffer, size_t aOffset, float aValue )
        {
            std::memcpy ( aBuffer.data() + aOffset, &aValue, sizeof ( aValue ) );
        }

        /** @brief Build a weighted quad whose every vertex is bound to joint 0.
         *
         *  The layout matches what the skinning compute pass expects: position,
         *  normal, tangent, bitangent and uv followed by the packed weight
         *  indices and values it consumes and drops. */
        void BuildSkinnedQuad ( Mesh& aMesh )
        {
            const float corners[4][3]
            {
                { -kQuadExtent, -kQuadExtent, kQuadDepth },
                {  kQuadExtent, -kQuadExtent, kQuadDepth },
                {  kQuadExtent,  kQuadExtent, kQuadDepth },
                { -kQuadExtent,  kQuadExtent, kQuadDepth },
            };
            // Both windings, so the quad survives whichever face the backend
            // culls: Vulkan's flipped Y makes a front face in GL a back face here.
            const uint32_t triangles[12] { 0, 1, 2, 0, 2, 3, 2, 1, 0, 3, 2, 0 };
            constexpr uint32_t vertex_count = 12;
            std::vector<uint8_t> vertices ( vertex_count * kSourceStride, 0 );
            for ( uint32_t v = 0; v < vertex_count; ++v )
            {
                const size_t base = static_cast<size_t> ( v ) * kSourceStride;
                const float* position = corners[triangles[v]];
                WriteFloat ( vertices, base + 0, position[0] );
                WriteFloat ( vertices, base + 4, position[1] );
                WriteFloat ( vertices, base + 8, position[2] );
                // Normal / tangent / bitangent: a fixed orthonormal basis. Their
                // values are irrelevant to coverage but they occupy the bytes a
                // wrong stride would misread as positions.
                WriteFloat ( vertices, base + 20, 1.0f ); // normal.z
                WriteFloat ( vertices, base + 24, 1.0f ); // tangent.x
                WriteFloat ( vertices, base + 40, 1.0f ); // bitangent.y
                WriteFloat ( vertices, base + 48, 0.25f ); // uv.x
                WriteFloat ( vertices, base + 52, 0.75f ); // uv.y
                // Weight indices all reference joint 0; first weight is 1.0.
                vertices[base + 60] = 0xFF;
            }

            MeshMsg message;
            auto add_attribute = [&message] ( AttributeMsg_AttributeSemantic aSemantic,
                                              uint32_t aSize,
                                              AttributeMsg_AttributeType aType,
                                              uint32_t aOffset,
                                              AttributeMsg_AttributeFlags aFlags )
            {
                auto* attribute = message.add_attribute();
                attribute->set_semantic ( aSemantic );
                attribute->set_size ( aSize );
                attribute->set_type ( aType );
                attribute->set_offset ( aOffset );
                attribute->set_flags ( aFlags );
            };
            add_attribute ( AttributeMsg_AttributeSemantic_POSITION, 3, AttributeMsg_AttributeType_FLOAT, 0, AttributeMsg_AttributeFlags_NONE );
            add_attribute ( AttributeMsg_AttributeSemantic_NORMAL, 3, AttributeMsg_AttributeType_FLOAT, 12, AttributeMsg_AttributeFlags_NONE );
            add_attribute ( AttributeMsg_AttributeSemantic_TANGENT, 3, AttributeMsg_AttributeType_FLOAT, 24, AttributeMsg_AttributeFlags_NONE );
            add_attribute ( AttributeMsg_AttributeSemantic_BITANGENT, 3, AttributeMsg_AttributeType_FLOAT, 36, AttributeMsg_AttributeFlags_NONE );
            add_attribute ( AttributeMsg_AttributeSemantic_TEXCOORD, 2, AttributeMsg_AttributeType_FLOAT, 48, AttributeMsg_AttributeFlags_NONE );
            add_attribute ( AttributeMsg_AttributeSemantic_WEIGHT_INDEX, 4, AttributeMsg_AttributeType_UNSIGNED_BYTE, 56, AttributeMsg_AttributeFlags_INTEGER );
            add_attribute ( AttributeMsg_AttributeSemantic_WEIGHT_VALUE, 4, AttributeMsg_AttributeType_UNSIGNED_BYTE, 60, AttributeMsg_AttributeFlags_NORMALIZED );
            message.set_vertexstride ( kSourceStride );
            message.set_vertexcount ( vertex_count );
            message.set_vertexbuffer ( vertices.data(), vertices.size() );
            aMesh.LoadFromPBMsg ( message );
        }

        /// @brief Count pixels whose colour differs from the image's top-left corner.
        size_t CountCoveredPixels ( const Texture& aTexture )
        {
            const std::vector<uint8_t>& pixels = aTexture.GetPixels();
            if ( pixels.size() < 4 )
            {
                return 0;
            }
            size_t covered = 0;
            for ( size_t i = 0; i < pixels.size(); i += 4 )
            {
                if ( pixels[i] != pixels[0] || pixels[i + 1] != pixels[1] || pixels[i + 2] != pixels[2] )
                {
                    ++covered;
                }
            }
            return covered;
        }

        /// @brief Count pixels differing between two equally sized captures.
        size_t CountDifferingPixels ( const Texture& aLeft, const Texture& aRight )
        {
            const std::vector<uint8_t>& left = aLeft.GetPixels();
            const std::vector<uint8_t>& right = aRight.GetPixels();
            if ( left.size() != right.size() )
            {
                return left.size() + right.size();
            }
            size_t differing = 0;
            for ( size_t i = 0; i < left.size(); i += 4 )
            {
                if ( left[i] != right[i] || left[i + 1] != right[i + 1] || left[i + 2] != right[i + 2] )
                {
                    ++differing;
                }
            }
            return differing;
        }
    }

    /** @brief Draw a quad twice, once at rest pose and once through the compute
     *  skinning pre-pass with identity joint matrices, and require the two
     *  captures to be identical.
     *
     *  Identity skinning copies position/normal/tangent/bitangent/uv through
     *  unchanged, so the only thing that differs between the two draws is where
     *  the vertices are fetched from: the mesh's own weighted 64-byte buffer
     *  versus the compute pass's packed 56-byte output. Any disagreement about
     *  the vertex stride therefore shows up as scattered geometry in the skinned
     *  capture while the rest-pose capture stays correct.
     *
     *  This is the regression guard for the Vulkan skinned vertex stride bug,
     *  where the pipeline took its binding stride from the source mesh and read
     *  the packed buffer 8 bytes out of step per vertex. The existing compute
     *  skinning test cannot catch it: it verifies the buffer the compute pass
     *  writes, which was always correct, not how the draw reads it. */
    static void RunSkinnedDrawMatchesRestPoseTest ( const char* aRendererName )
    {
        void* window = CreateHiddenRenderWindow();
        if ( window == nullptr )
        {
            GTEST_SKIP() << "No off-screen render window available on this platform.";
        }
        std::unique_ptr<Renderer> renderer = TryConstructRenderer ( aRendererName, window );
        if ( renderer == nullptr )
        {
            DestroyHiddenRenderWindow ( window );
            GTEST_SKIP() << aRendererName << " renderer unavailable on this host.";
        }
        renderer->ResizeViewport ( window, 0, 0, 64, 64 );

        Mesh mesh;
        BuildSkinnedQuad ( mesh );
        renderer->LoadMesh ( mesh );

        // plain_red declares a single vec3 position input, so the capture is a
        // clean coverage mask with no material or lighting state to set up.
        Pipeline draw_pipeline;
        draw_pipeline.LoadFromId ( "shaders/plain_red.txt"_crc32 );
        renderer->LoadPipeline ( draw_pipeline );

        Pipeline skinning_pipeline;
        skinning_pipeline.LoadFromId ( "shaders/skinning.txt"_crc32 );
        renderer->LoadPipeline ( skinning_pipeline );

        const Matrix4x4 model{};

        Texture rest_pose;
        renderer->RequestCapture ( window );
        renderer->BeginFrame ( window );
        // The view/projection buffers are per frame in flight, so they have to be
        // written inside every frame, not once up front.
        renderer->SetProjectionMatrix ( window, Matrix4x4{} );
        renderer->SetViewMatrix ( window, Matrix4x4{} );
        renderer->BeginRenderPass ( window );
        renderer->Render ( window, model, mesh, draw_pipeline );
        renderer->EndRender ( window );
        renderer->Finish ( window );
        if ( !renderer->ReadPixels ( window, rest_pose ) )
        {
            renderer.reset();
            DestroyHiddenRenderWindow ( window );
            GTEST_SKIP() << aRendererName << " cannot read back this surface.";
        }

        // Without visible geometry the comparison below would pass trivially.
        const size_t covered = CountCoveredPixels ( rest_pose );
        const size_t total = static_cast<size_t> ( rest_pose.GetWidth() ) * rest_pose.GetHeight();
        ASSERT_GT ( covered, total / 4 )
                << "rest-pose draw covered " << covered << " of " << total
                << " pixels; the quad is not reaching the framebuffer so this test proves nothing";

        Texture skinned;
        renderer->RequestCapture ( window );
        renderer->BeginFrame ( window );
        renderer->SetProjectionMatrix ( window, Matrix4x4{} );
        renderer->SetViewMatrix ( window, Matrix4x4{} );
        const float identity_joint[16]
        {
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        };
        BufferAccessor skinning_matrices = renderer->AllocateSingleFrameStorageMemory ( window, sizeof ( identity_joint ) );
        skinning_matrices.WriteMemory ( 0, sizeof ( identity_joint ), identity_joint );
        BufferAccessor skinned_vertices = renderer->AllocateSingleFrameStorageMemory (
                                              window, static_cast<size_t> ( mesh.GetVertexCount() ) * Mesh::kSkinnedVertexStride );
        renderer->Skin ( window, skinning_pipeline, mesh, skinning_matrices, skinned_vertices );
        renderer->Barrier ( window );
        renderer->BeginRenderPass ( window );
        renderer->Render ( window, model, mesh, draw_pipeline, nullptr, Topology::TRIANGLE_LIST,
                           0, 0xffffffff, 1, 0, &skinned_vertices );
        renderer->EndRender ( window );
        renderer->Finish ( window );
        ASSERT_TRUE ( renderer->ReadPixels ( window, skinned ) );

        EXPECT_EQ ( skinned.GetWidth(), rest_pose.GetWidth() );
        EXPECT_EQ ( skinned.GetHeight(), rest_pose.GetHeight() );
        const size_t differing = CountDifferingPixels ( rest_pose, skinned );
        EXPECT_EQ ( differing, 0u )
                << differing << " of " << total
                << " pixels differ between the rest-pose and identity-skinned draws; the skinned "
           "vertex buffer is being read with the wrong stride or offset";

        renderer.reset();
        DestroyHiddenRenderWindow ( window );
    }

#ifdef AEON_TEST_HAVE_OPENGL
    TEST ( SkinnedDrawTest, OpenGLIdentitySkinMatchesRestPose )
    {
        RunSkinnedDrawMatchesRestPoseTest ( "OpenGL" );
    }
#endif

#ifdef AEON_TEST_HAVE_VULKAN_WINDOW
    TEST ( SkinnedDrawTest, VulkanIdentitySkinMatchesRestPose )
    {
        RunSkinnedDrawMatchesRestPoseTest ( "Vulkan" );
    }
#endif

#ifdef AEON_TEST_HAVE_METAL
    TEST ( SkinnedDrawTest, MetalIdentitySkinMatchesRestPose )
    {
        RunSkinnedDrawMatchesRestPoseTest ( "Metal" );
    }
#endif
}
