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
#include <set>
#include <span>
#include <vector>
#include "gtest/gtest.h"
#include "aeongames/AeonEngine.hpp"
#include "aeongames/Component.hpp"
#include "aeongames/CRC.hpp"
#include "aeongames/GuiOverlay.hpp"
#include "aeongames/Material.hpp"
#include "aeongames/Matrix4x4.hpp"
#include "aeongames/Mesh.hpp"
#include "aeongames/Node.hpp"
#include "aeongames/Pipeline.hpp"
#include "aeongames/Property.hpp"
#include "aeongames/ProtoBufClasses.hpp"
#include "aeongames/ProtoBufHelpers.hpp"
#include "aeongames/Renderer.hpp"
#include "aeongames/Scene.hpp"
#include "aeongames/StringId.hpp"
#include "aeongames/Texture.hpp"
#include "aeongames/Transform.hpp"
#include "mesh.pb.h"
#include "pipeline.pb.h"
#include "RenderTestWindow.h"

namespace AeonGames
{
    namespace
    {
        constexpr uint32_t kVertexStride = 56;

        void WriteFloat ( std::vector<uint8_t>& aBuffer, size_t aOffset, float aValue )
        {
            std::memcpy ( aBuffer.data() + aOffset, &aValue, sizeof ( aValue ) );
        }

        /** Which triangles the quad fixture emits. Most tests want both
         *  windings so they stay independent of front-face state; the culling
         *  regression tests need exactly one. */
        enum class QuadWinding
        {
            Both,
            Forward,
            Reversed
        };

        void BuildTexturedQuad ( Mesh& aMesh, QuadWinding aWinding = QuadWinding::Both )
        {
            constexpr float extent = 0.8f;
            const float positions[4][3]
            {
                {-extent, -extent, 0.5f},
                { extent, -extent, 0.5f},
                { extent,  extent, 0.5f},
                {-extent,  extent, 0.5f},
            };
            const float texcoords[4][2]
            {
                {0.0f, 1.0f}, {1.0f, 1.0f}, {1.0f, 0.0f}, {0.0f, 0.0f}
            };
            // Duplicate both windings so the fixture is independent of API Y
            // conventions and front-face state.
            const uint32_t triangles[12] {0, 1, 2, 0, 2, 3, 2, 1, 0, 3, 2, 0};
            const uint32_t index_count = aWinding == QuadWinding::Both ? 12u : 6u;
            const uint32_t* indices = aWinding == QuadWinding::Reversed ? triangles + 6 : triangles;
            std::vector<uint8_t> vertices ( 4 * kVertexStride, 0 );
            for ( uint32_t vertex = 0; vertex < 4; ++vertex )
            {
                const size_t base = static_cast<size_t> ( vertex ) * kVertexStride;
                WriteFloat ( vertices, base + 0, positions[vertex][0] );
                WriteFloat ( vertices, base + 4, positions[vertex][1] );
                WriteFloat ( vertices, base + 8, positions[vertex][2] );
                WriteFloat ( vertices, base + 20, 1.0f );
                WriteFloat ( vertices, base + 24, 1.0f );
                WriteFloat ( vertices, base + 40, 1.0f );
                WriteFloat ( vertices, base + 48, texcoords[vertex][0] );
                WriteFloat ( vertices, base + 52, texcoords[vertex][1] );
            }

            MeshMsg message;
            message.mutable_center()->set_z ( 0.5f );
            message.mutable_radii()->set_x ( extent );
            message.mutable_radii()->set_y ( extent );
            message.mutable_radii()->set_z ( 0.01f );
            auto add_attribute = [&message] ( AttributeMsg_AttributeSemantic aSemantic,
                                              uint32_t aSize, uint32_t aOffset )
            {
                AttributeMsg* attribute = message.add_attribute();
                attribute->set_semantic ( aSemantic );
                attribute->set_size ( aSize );
                attribute->set_type ( AttributeMsg_AttributeType_FLOAT );
                attribute->set_offset ( aOffset );
                attribute->set_flags ( AttributeMsg_AttributeFlags_NONE );
            };
            add_attribute ( AttributeMsg_AttributeSemantic_POSITION, 3, 0 );
            add_attribute ( AttributeMsg_AttributeSemantic_NORMAL, 3, 12 );
            add_attribute ( AttributeMsg_AttributeSemantic_TANGENT, 3, 24 );
            add_attribute ( AttributeMsg_AttributeSemantic_BITANGENT, 3, 36 );
            add_attribute ( AttributeMsg_AttributeSemantic_TEXCOORD, 2, 48 );
            message.set_vertexstride ( kVertexStride );
            message.set_vertexcount ( 4 );
            message.set_vertexbuffer ( vertices.data(), vertices.size() );
            message.set_indexsize ( sizeof ( uint32_t ) );
            message.set_indexcount ( index_count );
            message.set_indexbuffer ( indices, index_count * sizeof ( uint32_t ) );
            aMesh.LoadFromPBMsg ( message );
        }

        size_t CountCoveredPixels ( const Texture& aTexture, std::set<uint32_t>& aColors )
        {
            const std::vector<uint8_t>& pixels = aTexture.GetPixels();
            const uint32_t background = static_cast<uint32_t> ( pixels[0] ) |
                                        static_cast<uint32_t> ( pixels[1] ) << 8 |
                                        static_cast<uint32_t> ( pixels[2] ) << 16;
            size_t covered = 0;
            for ( size_t offset = 0; offset < pixels.size(); offset += 4 )
            {
                const uint32_t color = static_cast<uint32_t> ( pixels[offset] ) |
                                       static_cast<uint32_t> ( pixels[offset + 1] ) << 8 |
                                       static_cast<uint32_t> ( pixels[offset + 2] ) << 16;
                if ( color != background )
                {
                    ++covered;
                    aColors.insert ( color );
                }
            }
            return covered;
        }

        Texture Capture ( Renderer& aRenderer, void* aWindow, const Mesh& aMesh,
                          const Pipeline& aPipeline, const Material& aMaterial,
                          std::span<const Matrix4x4> aTransforms )
        {
            Texture capture;
            aRenderer.RequestCapture ( aWindow );
            aRenderer.BeginFrame ( aWindow );
            aRenderer.SetProjectionMatrix ( aWindow, Matrix4x4{} );
            aRenderer.SetViewMatrix ( aWindow, Matrix4x4{} );
            aRenderer.BeginRenderPass ( aWindow );
            if ( aTransforms.size() == 1 )
            {
                aRenderer.Render ( aWindow, aTransforms.front(), aMesh, aPipeline, &aMaterial );
            }
            else
            {
                aRenderer.RenderInstanced ( aWindow, aTransforms, aMesh, aPipeline, &aMaterial );
            }
            aRenderer.EndRender ( aWindow );
            aRenderer.Finish ( aWindow );
            EXPECT_TRUE ( aRenderer.ReadPixels ( aWindow, capture ) );
            return capture;
        }

        class TestRenderComponent final : public Component
        {
        public:
            TestRenderComponent ( const Mesh& aMesh, const Pipeline& aPipeline, const Material& aMaterial ) :
                mMesh{aMesh}, mPipeline{aPipeline}, mMaterial{aMaterial} {}

            const StringId& GetId() const final
            {
                static const StringId id{"MetalRendererParityComponent"};
                return id;
            }
            size_t GetPropertyCount() const final
            {
                return 0;
            }
            const StringId* GetPropertyInfoArray() const final
            {
                return nullptr;
            }
            Property GetProperty ( const StringId& ) const final
            {
                return {};
            }
            void SetProperty ( uint32_t, const Property& ) final {}
            void Update ( Node&, double ) final {}
            void Collect ( const Node& aNode, std::vector<RenderItem>& aQueue ) const final
            {
                aQueue.push_back ( RenderItem{&mMesh, &mPipeline, &mMaterial, nullptr, aNode.GetGlobalTransform() } );
            }
            void ProcessMessage ( Node&, uint32_t, const void* ) final {}

        private:
            const Mesh& mMesh;
            const Pipeline& mPipeline;
            const Material& mMaterial;
        };

        Node* AddDrawable ( Scene& aScene, const Mesh& aMesh, const Pipeline& aPipeline,
                            const Material& aMaterial, const Vector3& aTranslation,
                            const Vector3& aRotation = {},
                            const Vector3& aScale = Vector3{0.45f, 0.45f, 1.0f} )
        {
            auto node = std::make_unique<Node>();
            node->SetLocalTransform ( Transform{aScale, aRotation, aTranslation} );
            node->SetAABB ( AABB{Vector3{}, Vector3{1.0f, 1.0f, 1.0f}} );
            node->AddComponent ( std::make_unique<TestRenderComponent> ( aMesh, aPipeline, aMaterial ) );
            return aScene.Add ( std::move ( node ) );
        }

        class SolidGuiOverlay final : public GuiOverlay
        {
        public:
            SolidGuiOverlay ( uint32_t aWidth, uint32_t aHeight, const std::array<uint8_t, 4>& aColor ) :
                SolidGuiOverlay{aWidth, aHeight, aColor, aColor} {}

            /** Vertically split overlay: the top half of the source image gets
             *  aTopColor. Renders an orientation the capture can assert on. */
            SolidGuiOverlay ( uint32_t aWidth, uint32_t aHeight, const std::array<uint8_t, 4>& aTopColor,
                              const std::array<uint8_t, 4>& aBottomColor ) :
                mWidth{aWidth}, mHeight{aHeight}, mPixels ( static_cast<size_t> ( aWidth ) * aHeight * 4 )
            {
                for ( uint32_t row = 0; row < aHeight; ++row )
                {
                    const std::array<uint8_t, 4>& color = row < aHeight / 2 ? aTopColor : aBottomColor;
                    for ( uint32_t column = 0; column < aWidth; ++column )
                    {
                        std::memcpy ( mPixels.data() + ( static_cast<size_t> ( row ) * aWidth + column ) * 4,
                                      color.data(), color.size() );
                    }
                }
            }

            void BeginFrame ( void*, double ) final {}
            void EndFrame ( void* ) final {}
            const uint8_t* GetPixels() const final
            {
                return mPixels.data();
            }
            uint32_t GetWidth() const final
            {
                return mWidth;
            }
            uint32_t GetHeight() const final
            {
                return mHeight;
            }
            bool OnMouseMove ( int32_t, int32_t ) final
            {
                return false;
            }
            bool OnMouseButton ( int32_t, bool, int32_t, int32_t ) final
            {
                return false;
            }
            bool OnKeyEvent ( KeyCode, bool ) final
            {
                return false;
            }
            bool OnTextInput ( uint32_t ) final
            {
                return false;
            }
            void Resize ( uint32_t, uint32_t ) final {}
            void Navigate ( const std::string& ) final {}

        private:
            uint32_t mWidth;
            uint32_t mHeight;
            std::vector<uint8_t> mPixels;
        };
    }

#ifdef AEON_TEST_HAVE_METAL
    TEST ( RendererParityTest, MetalBindlessTexturedStaticAndInstanced )
    {
        void* window = CreateHiddenRenderWindow();
        ASSERT_NE ( window, nullptr );
        std::unique_ptr<Renderer> renderer = TryConstructRenderer ( "Metal", window );
        ASSERT_NE ( renderer, nullptr );
        renderer->ResizeViewport ( window, 0, 0, 64, 64 );

        Mesh mesh;
        BuildTexturedQuad ( mesh );
        renderer->LoadMesh ( mesh );
        Pipeline pipeline;
        pipeline.LoadFromId ( "shaders/bindless_unlit.txt"_crc32 );
        renderer->LoadPipeline ( pipeline );
        Material material;
        material.LoadFromId ( "polesign/materials/default.txt"_crc32 );
        renderer->LoadMaterial ( material );

        const Matrix4x4 identity{};
        const Texture single = Capture ( *renderer, window, mesh, pipeline, material,
                                         std::span<const Matrix4x4> {&identity, 1} );
        std::set<uint32_t> single_colors;
        const size_t single_covered = CountCoveredPixels ( single, single_colors );
        const size_t pixel_count = static_cast<size_t> ( single.GetWidth() ) * single.GetHeight();
        EXPECT_GT ( single_covered, pixel_count / 3 );
        EXPECT_GT ( single_colors.size(), 16u ) << "textured draw has no meaningful color variation";

        const Matrix4x4 transforms[2]
        {
            Matrix4x4 {
                0.45f, 0.0f, 0.0f, 0.0f,
                0.0f, 0.45f, 0.0f, 0.0f,
                0.0f, 0.0f, 1.0f, 0.0f,
                -0.5f, 0.0f, 0.0f, 1.0f},
            Matrix4x4 {
                0.45f, 0.0f, 0.0f, 0.0f,
                0.0f, 0.45f, 0.0f, 0.0f,
                0.0f, 0.0f, 1.0f, 0.0f,
                0.5f, 0.0f, 0.0f, 1.0f},
        };
        const Texture instanced = Capture ( *renderer, window, mesh, pipeline, material, transforms );
        std::set<uint32_t> instanced_colors;
        const size_t instanced_covered = CountCoveredPixels ( instanced, instanced_colors );
        EXPECT_GT ( instanced_covered, pixel_count / 6 );
        EXPECT_GT ( instanced_colors.size(), 16u );
        EXPECT_NE ( instanced.GetPixels(), single.GetPixels() );

        renderer.reset();
        DestroyHiddenRenderWindow ( window );
    }

    TEST ( RendererParityTest, MetalRenderSceneBuildsAndSubmitsQueue )
    {
        void* window = CreateHiddenRenderWindow();
        ASSERT_NE ( window, nullptr );
        std::unique_ptr<Renderer> renderer = TryConstructRenderer ( "Metal", window );
        ASSERT_NE ( renderer, nullptr );
        renderer->ResizeViewport ( window, 0, 0, 64, 64 );

        Mesh mesh;
        BuildTexturedQuad ( mesh );
        renderer->LoadMesh ( mesh );
        Pipeline pipeline;
        pipeline.LoadFromId ( "shaders/bindless_unlit.txt"_crc32 );
        renderer->LoadPipeline ( pipeline );
        Material material;
        material.LoadFromId ( "polesign/materials/default.txt"_crc32 );
        renderer->LoadMaterial ( material );

        Scene scene;
        AddDrawable ( scene, mesh, pipeline, material, Vector3{-0.5f, 0.0f, 0.0f} );
        AddDrawable ( scene, mesh, pipeline, material, Vector3{ 0.5f, 0.0f, 0.0f} );

        Texture capture;
        renderer->RequestCapture ( window );
        renderer->BeginFrame ( window );
        renderer->SetProjectionMatrix ( window, Matrix4x4{} );
        renderer->SetViewMatrix ( window, Matrix4x4{} );
        renderer->RenderScene ( window, scene );
        renderer->Finish ( window );
        ASSERT_TRUE ( renderer->ReadPixels ( window, capture ) );

        std::set<uint32_t> colors;
        const size_t covered = CountCoveredPixels ( capture, colors );
        const size_t pixel_count = static_cast<size_t> ( capture.GetWidth() ) * capture.GetHeight();
        EXPECT_GT ( covered, pixel_count / 6 );
        EXPECT_GT ( colors.size(), 16u );

        renderer.reset();
        DestroyHiddenRenderWindow ( window );
    }

    TEST ( RendererParityTest, MetalCompositesGuiOverlay )
    {
        void* window = CreateHiddenRenderWindow();
        ASSERT_NE ( window, nullptr );
        std::unique_ptr<Renderer> renderer = TryConstructRenderer ( "Metal", window );
        ASSERT_NE ( renderer, nullptr );
        renderer->ResizeViewport ( window, 0, 0, 64, 64 );
        renderer->SetClearColor ( window, 0.0f, 0.0f, 0.0f, 1.0f );
        const SolidGuiOverlay overlay{64, 64, {0, 0, 128, 128}};

        Texture capture;
        renderer->RequestCapture ( window );
        renderer->BeginFrame ( window );
        renderer->BeginRenderPass ( window );
        renderer->RenderOverlay ( window, overlay );
        renderer->EndRender ( window );
        renderer->Finish ( window );
        ASSERT_TRUE ( renderer->ReadPixels ( window, capture ) );

        const size_t center = ( static_cast<size_t> ( capture.GetHeight() / 2 ) * capture.GetWidth() +
                                capture.GetWidth() / 2 ) * 4;
        const std::vector<uint8_t>& pixels = capture.GetPixels();
        EXPECT_GE ( pixels[center], 201 );
        EXPECT_LE ( pixels[center], 209 );
        EXPECT_LE ( pixels[center + 1], 2 );
        EXPECT_LE ( pixels[center + 2], 2 );
        EXPECT_EQ ( pixels[center + 3], 255 );

        renderer.reset();
        DestroyHiddenRenderWindow ( window );
    }

    // The Metal backend renders through a Y-flipped viewport so its clip space
    // matches the Vulkan +Y-down convention every shared shader is authored
    // for. Without it the scene rasterizes mirrored, which the tone-map pass
    // then unflips -- hiding the problem on the 3D image while leaving the
    // clip-space GUI quad upside down and every triangle's winding reversed.
    TEST ( RendererParityTest, MetalOverlayKeepsSourceRowOrder )
    {
        void* window = CreateHiddenRenderWindow();
        ASSERT_NE ( window, nullptr );
        std::unique_ptr<Renderer> renderer = TryConstructRenderer ( "Metal", window );
        ASSERT_NE ( renderer, nullptr );
        renderer->ResizeViewport ( window, 0, 0, 64, 64 );
        renderer->SetClearColor ( window, 0.0f, 0.0f, 0.0f, 1.0f );
        // Premultiplied BGRA: the top half is red, the bottom half fully
        // transparent, so the capture shows which source row landed on top.
        const SolidGuiOverlay overlay{64, 64, {0, 0, 128, 128}, {0, 0, 0, 0}};

        Texture capture;
        renderer->RequestCapture ( window );
        renderer->BeginFrame ( window );
        renderer->BeginRenderPass ( window );
        renderer->RenderOverlay ( window, overlay );
        renderer->EndRender ( window );
        renderer->Finish ( window );
        ASSERT_TRUE ( renderer->ReadPixels ( window, capture ) );

        const uint32_t width = capture.GetWidth();
        const uint32_t height = capture.GetHeight();
        const std::vector<uint8_t>& pixels = capture.GetPixels();
        auto red_at = [&] ( uint32_t aRow )
        {
            return pixels[ ( static_cast<size_t> ( aRow ) * width + width / 2 ) * 4];
        };
        EXPECT_GE ( red_at ( height / 4 ), 201u ) << "overlay is vertically flipped";
        EXPECT_LE ( red_at ( height * 3 / 4 ), 2u ) << "overlay is vertically flipped";

        renderer.reset();
        DestroyHiddenRenderWindow ( window );
    }

    TEST ( RendererParityTest, MetalCullsBackFacesNotFrontFaces )
    {
        void* window = CreateHiddenRenderWindow();
        ASSERT_NE ( window, nullptr );
        std::unique_ptr<Renderer> renderer = TryConstructRenderer ( "Metal", window );
        ASSERT_NE ( renderer, nullptr );
        renderer->ResizeViewport ( window, 0, 0, 64, 64 );

        Pipeline pipeline;
        pipeline.LoadFromId ( "shaders/bindless_unlit.txt"_crc32 );
        renderer->LoadPipeline ( pipeline );
        ASSERT_EQ ( pipeline.GetRasterState().cull_mode, PipelineCullMode::BACK );
        ASSERT_EQ ( pipeline.GetRasterState().front_face, PipelineFrontFace::COUNTER_CLOCKWISE );
        Material material;
        material.LoadFromId ( "polesign/materials/default.txt"_crc32 );
        renderer->LoadMaterial ( material );

        // Under the engine's Vulkan-style clip space the {0,1,2} winding of this
        // clip-space quad is the back face and {2,1,0} the front face -- the
        // same relationship the shared fullscreen triangle relies on. A missing
        // Y flip mirrors rasterization and swaps the two.
        const Matrix4x4 identity{};
        Mesh back_facing;
        BuildTexturedQuad ( back_facing, QuadWinding::Forward );
        renderer->LoadMesh ( back_facing );
        const Texture culled = Capture ( *renderer, window, back_facing, pipeline, material,
                                         std::span<const Matrix4x4> {&identity, 1} );

        Mesh front_facing;
        BuildTexturedQuad ( front_facing, QuadWinding::Reversed );
        renderer->LoadMesh ( front_facing );
        const Texture drawn = Capture ( *renderer, window, front_facing, pipeline, material,
                                        std::span<const Matrix4x4> {&identity, 1} );

        const size_t pixel_count = static_cast<size_t> ( drawn.GetWidth() ) * drawn.GetHeight();
        std::set<uint32_t> culled_colors;
        std::set<uint32_t> drawn_colors;
        EXPECT_EQ ( CountCoveredPixels ( culled, culled_colors ), 0u )
                << "back-facing winding was not culled";
        EXPECT_GT ( CountCoveredPixels ( drawn, drawn_colors ), pixel_count / 3 )
                << "front-facing winding was culled";

        renderer.reset();
        DestroyHiddenRenderWindow ( window );
    }

    TEST ( RendererParityTest, MetalRendersClusteredMaterial )
    {
        void* window = CreateHiddenRenderWindow();
        ASSERT_NE ( window, nullptr );
        std::unique_ptr<Renderer> renderer = TryConstructRenderer ( "Metal", window );
        ASSERT_NE ( renderer, nullptr );
        renderer->ResizeViewport ( window, 0, 0, 64, 64 );
        renderer->SetClearColor ( window, 0.0f, 0.0f, 0.0f, 1.0f );

        Matrix4x4 projection{};
        projection.Perspective ( 60.0f, 1.0f, 1.0f, 100.0f );
        renderer->SetProjectionMatrix ( window, projection );
        renderer->SetViewMatrix ( window, Matrix4x4{} );
        GpuGlobals globals{};
        globals.ambient = Vector4{1.0f, 1.0f, 1.0f, 0.5f};
        constexpr float ambient_dc = 0.282095f * 4.0f * 3.14159265358979323846f * 0.5f;
        globals.sh[0] = Vector4{ambient_dc, ambient_dc, ambient_dc, 0.0f};
        renderer->SetGlobals ( window, globals );

        Mesh mesh;
        BuildTexturedQuad ( mesh );
        renderer->LoadMesh ( mesh );
        Pipeline lighting;
        lighting.LoadFromId ( "shaders/lighting.txt"_crc32 );
        renderer->LoadPipeline ( lighting );
        Pipeline shading;
        shading.LoadFromId ( "shaders/clustered_phong.txt"_crc32 );
        renderer->LoadPipeline ( shading );
        Material material;
        material.LoadFromId ( "polesign/materials/default.txt"_crc32 );
        renderer->LoadMaterial ( material );
        const Matrix4x4 model
        {
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 5.0f, 0.0f, 1.0f
        };

        Texture capture;
        renderer->RequestCapture ( window );
        renderer->BeginRender ( window, &lighting );
        renderer->Render ( window, model, mesh, shading, &material, Topology::TRIANGLE_LIST,
                           0, 0xffffffff, 1, 0, nullptr, RenderPass::DepthPrePass );
        renderer->EndDepthPrePass ( window, &lighting );
        renderer->Render ( window, model, mesh, shading, &material );
        renderer->EndRender ( window );
        renderer->Finish ( window );
        ASSERT_TRUE ( renderer->ReadPixels ( window, capture ) );

        std::set<uint32_t> colors;
        const size_t covered = CountCoveredPixels ( capture, colors );
        const size_t pixel_count = static_cast<size_t> ( capture.GetWidth() ) * capture.GetHeight();
        EXPECT_GT ( covered, pixel_count / 20 );
        EXPECT_GT ( colors.size(), 16u );

        renderer.reset();
        DestroyHiddenRenderWindow ( window );
    }

    TEST ( RendererParityTest, MetalExecutesAllShadowPasses )
    {
        void* window = CreateHiddenRenderWindow();
        ASSERT_NE ( window, nullptr );
        RendererSettings settings{};
        settings.mDirectionalShadowMapResolution = 32;
        settings.mSpotShadowMapResolution = 32;
        settings.mPointShadowMapResolution = 32;
        std::unique_ptr<Renderer> renderer = ConstructRenderer ( std::string {"Metal"}, window, settings );
        ASSERT_NE ( renderer, nullptr );
        renderer->ResizeViewport ( window, 0, 0, 64, 64 );
        renderer->SetClearColor ( window, 0.0f, 0.25f, 0.5f, 1.0f );

        Mesh mesh;
        BuildTexturedQuad ( mesh );
        renderer->LoadMesh ( mesh );
        Pipeline pipeline;
        pipeline.LoadFromId ( "shaders/bindless_unlit.txt"_crc32 );
        renderer->LoadPipeline ( pipeline );
        const Matrix4x4 identity{};

        GpuSpotShadowParams spot_params{};
        spot_params.params[3] = 1.0f;
        renderer->SetSpotShadowParams ( window, spot_params );
        GpuPointShadowParams point_params{};
        for ( Matrix4x4& matrix : point_params.point_light_view_projection )
        {
            matrix = identity;
        }
        point_params.caster_position_radius[0] = Vector4{0.0f, 0.0f, 0.0f, 10.0f};
        point_params.params[3] = 1.0f;
        renderer->SetPointShadowParams ( window, point_params );

        renderer->RequestCapture ( window );
        renderer->BeginRender ( window );
        renderer->BeginSpotShadowPass ( window, 0, identity );
        renderer->Render ( window, identity, mesh, pipeline, nullptr, Topology::TRIANGLE_LIST,
                           0, 0xffffffff, 1, 0, nullptr, RenderPass::ShadowPass );
        renderer->EndSpotShadowPass ( window );
        renderer->BeginPointShadowPass ( window, 0 );
        renderer->Render ( window, identity, mesh, pipeline, nullptr, Topology::TRIANGLE_LIST,
                           0, 0xffffffff, 1, 0, nullptr, RenderPass::ShadowPass );
        renderer->EndPointShadowPass ( window );
        renderer->BeginShadowPass ( window, identity );
        renderer->Render ( window, identity, mesh, pipeline, nullptr, Topology::TRIANGLE_LIST,
                           0, 0xffffffff, 1, 0, nullptr, RenderPass::ShadowPass );
        renderer->EndShadowPass ( window );
        renderer->EndRender ( window );
        renderer->Finish ( window );

        Texture capture;
        ASSERT_TRUE ( renderer->ReadPixels ( window, capture ) );
        const size_t center = ( static_cast<size_t> ( capture.GetHeight() / 2 ) * capture.GetWidth() +
                                capture.GetWidth() / 2 ) * 4;
        const std::vector<uint8_t>& pixels = capture.GetPixels();
        EXPECT_LE ( pixels[center], 2 );
        EXPECT_GE ( pixels[center + 1], 159 );
        EXPECT_LE ( pixels[center + 1], 167 );
        EXPECT_GE ( pixels[center + 2], 201 );
        EXPECT_LE ( pixels[center + 2], 209 );

        renderer.reset();
        DestroyHiddenRenderWindow ( window );
    }

    TEST ( RendererParityTest, MetalSamplesDirectionalShadow )
    {
        void* window = CreateHiddenRenderWindow();
        ASSERT_NE ( window, nullptr );
        RendererSettings settings{};
        settings.mDirectionalShadowMapResolution = 32;
        std::unique_ptr<Renderer> renderer = ConstructRenderer ( std::string {"Metal"}, window, settings );
        ASSERT_NE ( renderer, nullptr );
        renderer->ResizeViewport ( window, 0, 0, 64, 64 );
        renderer->SetClearColor ( window, 0.0f, 0.0f, 0.0f, 1.0f );
        renderer->SetProjectionMatrix ( window, Matrix4x4{} );
        renderer->SetViewMatrix ( window, Matrix4x4{} );

        GpuGlobals globals{};
        constexpr float ambient_dc = 0.282095f * 4.0f * 3.14159265358979323846f * 0.05f;
        globals.sh[0] = Vector4{ambient_dc, ambient_dc, ambient_dc, 0.0f};
        renderer->SetGlobals ( window, globals );
        GpuLight light{};
        light.color_intensity = Vector4{1.0f, 1.0f, 1.0f, 3.0f};
        light.direction_cosOuter = Vector4{0.0f, 0.0f, 1.0f, 0.0f};
        light.type = static_cast<uint32_t> ( LightType::Directional );
        renderer->SetLights ( window, std::span<const GpuLight> {&light, 1} );

        Mesh mesh;
        BuildTexturedQuad ( mesh );
        renderer->LoadMesh ( mesh );
        Pipeline lighting;
        lighting.LoadFromId ( "shaders/lighting.txt"_crc32 );
        renderer->LoadPipeline ( lighting );
        Pipeline shading;
        shading.LoadFromId ( "shaders/clustered_phong.txt"_crc32 );
        renderer->LoadPipeline ( shading );
        Material material;
        material.LoadFromId ( "polesign/materials/default.txt"_crc32 );
        renderer->LoadMaterial ( material );
        const Matrix4x4 receiver{};
        const Matrix4x4 occluder
        {
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, -0.5f, 1.0f
        };
        auto render = [&] ( bool aWithShadow )
        {
            Texture capture;
            renderer->RequestCapture ( window );
            renderer->BeginRender ( window, &lighting );
            if ( aWithShadow )
            {
                renderer->BeginShadowPass ( window, Matrix4x4{} );
                renderer->Render ( window, occluder, mesh, shading, nullptr, Topology::TRIANGLE_LIST,
                                   0, 0xffffffff, 1, 0, nullptr, RenderPass::ShadowPass );
                renderer->EndShadowPass ( window );
            }
            renderer->Render ( window, receiver, mesh, shading, &material, Topology::TRIANGLE_LIST,
                               0, 0xffffffff, 1, 0, nullptr, RenderPass::DepthPrePass );
            renderer->EndDepthPrePass ( window, &lighting );
            renderer->Render ( window, receiver, mesh, shading, &material );
            renderer->EndRender ( window );
            renderer->Finish ( window );
            EXPECT_TRUE ( renderer->ReadPixels ( window, capture ) );
            return capture;
        };

        const Texture lit = render ( false );
        const Texture shadowed = render ( true );
        const size_t center = ( static_cast<size_t> ( lit.GetHeight() / 2 ) * lit.GetWidth() +
                                lit.GetWidth() / 2 ) * 4;
        const std::vector<uint8_t>& lit_pixels = lit.GetPixels();
        const std::vector<uint8_t>& shadowed_pixels = shadowed.GetPixels();
        const uint32_t lit_luma = lit_pixels[center] + lit_pixels[center + 1] + lit_pixels[center + 2];
        const uint32_t shadowed_luma = shadowed_pixels[center] + shadowed_pixels[center + 1] +
                                       shadowed_pixels[center + 2];
        EXPECT_GT ( lit_luma, shadowed_luma + 20u );

        renderer.reset();
        DestroyHiddenRenderWindow ( window );
    }

    TEST ( RendererParityTest, MetalRendersEnvironmentSkybox )
    {
        void* window = CreateHiddenRenderWindow();
        ASSERT_NE ( window, nullptr );
        RendererSettings settings{};
        settings.mSkyboxEnvironmentFaceSize = 8;
        settings.mPrefilteredEnvironmentFaceSize = 8;
        settings.mPrefilteredEnvironmentMipCount = 3;
        std::unique_ptr<Renderer> renderer = ConstructRenderer ( std::string {"Metal"}, window, settings );
        ASSERT_NE ( renderer, nullptr );
        renderer->ResizeViewport ( window, 0, 0, 64, 64 );

        std::vector<float> environment_pixels ( 16 * 8 * 3 );
        for ( size_t pixel = 0; pixel < environment_pixels.size() / 3; ++pixel )
        {
            environment_pixels[pixel * 3 + 0] = 0.1f;
            environment_pixels[pixel * 3 + 1] = 0.6f;
            environment_pixels[pixel * 3 + 2] = 0.2f;
        }
        Texture environment;
        environment.Resize ( 16, 8, reinterpret_cast<const uint8_t*> ( environment_pixels.data() ),
                             Texture::Format::RGB, Texture::Type::FLOAT );
        renderer->SetEnvironmentMap ( window, &environment );

        Matrix4x4 projection{};
        projection.Perspective ( 60.0f, 1.0f, 1.0f, 100.0f );
        renderer->SetProjectionMatrix ( window, projection );
        renderer->SetViewMatrix ( window, Matrix4x4{} );
        Texture capture;
        renderer->RequestCapture ( window );
        renderer->BeginRender ( window );
        renderer->EndRender ( window );
        renderer->Finish ( window );
        ASSERT_TRUE ( renderer->ReadPixels ( window, capture ) );

        const size_t center = ( static_cast<size_t> ( capture.GetHeight() / 2 ) * capture.GetWidth() +
                                capture.GetWidth() / 2 ) * 4;
        const std::vector<uint8_t>& pixels = capture.GetPixels();
        EXPECT_GT ( pixels[center + 1], pixels[center] + 50 );
        EXPECT_GT ( pixels[center + 1], pixels[center + 2] + 25 );
        EXPECT_GT ( pixels[center + 1], 180 );

        renderer.reset();
        DestroyHiddenRenderWindow ( window );
    }

    TEST ( RendererParityTest, MetalRendersSharedDebugGeometry )
    {
        void* window = CreateHiddenRenderWindow();
        ASSERT_NE ( window, nullptr );
        std::unique_ptr<Renderer> renderer = TryConstructRenderer ( "Metal", window );
        ASSERT_NE ( renderer, nullptr );
        renderer->ResizeViewport ( window, 0, 0, 64, 64 );
        renderer->SetClearColor ( window, 0.0f, 0.0f, 0.0f, 1.0f );
        Matrix4x4 projection{};
        projection.Perspective ( 60.0f, 1.0f, 1.0f, 100.0f );
        renderer->SetProjectionMatrix ( window, projection );
        renderer->SetViewMatrix ( window, Matrix4x4{} );

        Scene scene;
        auto node = std::make_unique<Node>();
        node->SetLocalTransform ( Transform{Quaternion{}, Vector3{0.0f, 5.0f, 0.0f}} );
        node->SetAABB ( AABB{Vector3{}, Vector3{1.0f, 1.0f, 1.0f}} );
        scene.Add ( std::move ( node ) );
        DebugRenderSettings debug{};
        debug.mDrawGrid = false;
        debug.mDrawNodeAABBs = true;
        debug.mDrawOctree = false;
        debug.mDrawCameraFrustums = false;
        renderer->SetDebugRenderSettings ( debug );
        renderer->SetDebugRendering ( true );

        Texture capture;
        renderer->RequestCapture ( window );
        renderer->BeginFrame ( window );
        renderer->RenderScene ( window, scene );
        renderer->Finish ( window );
        ASSERT_TRUE ( renderer->ReadPixels ( window, capture ) );

        std::set<uint32_t> colors;
        EXPECT_GT ( CountCoveredPixels ( capture, colors ), 20u );
        EXPECT_FALSE ( colors.empty() );

        renderer.reset();
        DestroyHiddenRenderWindow ( window );
    }

    TEST ( RendererParityTest, MetalRenderSceneSubmitsGpuCulledBatch )
    {
        void* window = CreateHiddenRenderWindow();
        ASSERT_NE ( window, nullptr );
        std::unique_ptr<Renderer> renderer = TryConstructRenderer ( "Metal", window );
        ASSERT_NE ( renderer, nullptr );
        renderer->ResizeViewport ( window, 0, 0, 64, 64 );
        Matrix4x4 projection{};
        projection.Perspective ( 60.0f, 1.0f, 1.0f, 100.0f );
        renderer->SetProjectionMatrix ( window, projection );
        renderer->SetViewMatrix ( window, Matrix4x4{} );
        GpuGlobals globals{};
        constexpr float ambient_dc = 0.282095f * 4.0f * 3.14159265358979323846f * 0.5f;
        globals.sh[0] = Vector4{ambient_dc, ambient_dc, ambient_dc, 0.0f};
        renderer->SetGlobals ( window, globals );

        Mesh mesh;
        BuildTexturedQuad ( mesh );
        renderer->LoadMesh ( mesh );
        Pipeline shading;
        shading.LoadFromId ( "shaders/clustered_phong.txt"_crc32 );
        renderer->LoadPipeline ( shading );
        Material material;
        material.LoadFromId ( "polesign/materials/default.txt"_crc32 );
        renderer->LoadMaterial ( material );
        Scene scene;
        scene.SetLightingPipeline ( ResourceId{"Pipeline"_crc32, "shaders/lighting.txt"} );
        AddDrawable ( scene, mesh, shading, material, Vector3{-0.8f, 5.0f, 0.0f} );
        AddDrawable ( scene, mesh, shading, material, Vector3{ 0.8f, 5.0f, 0.0f} );

        Texture capture;
        renderer->RequestCapture ( window );
        renderer->BeginFrame ( window );
        renderer->RenderScene ( window, scene );
        renderer->Finish ( window );
        ASSERT_TRUE ( renderer->ReadPixels ( window, capture ) );
        std::set<uint32_t> colors;
        EXPECT_GT ( CountCoveredPixels ( capture, colors ), 8u );
        EXPECT_GT ( colors.size(), 8u );

        renderer.reset();
        DestroyHiddenRenderWindow ( window );
    }

    TEST ( RendererParityTest, MetalHiZOcclusionRejectsHiddenDraw )
    {
        void* window = CreateHiddenRenderWindow();
        ASSERT_NE ( window, nullptr );
        std::unique_ptr<Renderer> renderer = TryConstructRenderer ( "Metal", window );
        ASSERT_NE ( renderer, nullptr );
        renderer->ResizeViewport ( window, 0, 0, 64, 64 );
        Matrix4x4 projection{};
        projection.Perspective ( 60.0f, 1.0f, 1.0f, 100.0f );
        renderer->SetProjectionMatrix ( window, projection );
        renderer->SetViewMatrix ( window, Matrix4x4{} );
        GpuGlobals globals{};
        constexpr float ambient_dc = 0.282095f * 4.0f * 3.14159265358979323846f * 0.5f;
        globals.sh[0] = Vector4{ambient_dc, ambient_dc, ambient_dc, 0.0f};
        renderer->SetGlobals ( window, globals );

        Mesh mesh;
        BuildTexturedQuad ( mesh );
        renderer->LoadMesh ( mesh );
        const size_t source_size = GetResourceSize ( "shaders/clustered_phong.txt" );
        ASSERT_GT ( source_size, 8u );
        std::vector<uint8_t> source ( source_size );
        LoadResource ( "shaders/clustered_phong.txt", source.data(), source.size() );
        PipelineMsg message;
        LoadProtoBufObject ( message, source.data(), source.size(), "AEONPLN"_mgk );
        message.mutable_depth_stencil()->set_depth_test ( PipelineMsg_Toggle_DISABLED );
        message.mutable_depth_stencil()->set_depth_write ( PipelineMsg_Toggle_DISABLED );
        Pipeline shading;
        shading.LoadFromPBMsg ( message );
        renderer->LoadPipeline ( shading );
        Material material;
        material.LoadFromId ( "polesign/materials/default.txt"_crc32 );
        renderer->LoadMaterial ( material );

        auto render = [&] ( bool aIncludeHidden )
        {
            Scene scene;
            scene.SetLightingPipeline ( ResourceId{"Pipeline"_crc32, "shaders/lighting.txt"} );
            const Vector3 camera_facing{90.0f, 0.0f, 0.0f};
            AddDrawable ( scene, mesh, shading, material, Vector3{0.0f, 4.0f, 0.0f}, camera_facing,
                          Vector3{1.5f, 1.5f, 1.0f} );
            if ( aIncludeHidden )
            {
                AddDrawable ( scene, mesh, shading, material, Vector3{0.0f, 8.0f, 0.0f}, camera_facing,
                              Vector3{0.25f, 0.25f, 1.0f} );
            }
            Texture capture;
            renderer->RequestCapture ( window );
            renderer->BeginFrame ( window );
            renderer->RenderScene ( window, scene );
            renderer->Finish ( window );
            EXPECT_TRUE ( renderer->ReadPixels ( window, capture ) );
            return capture;
        };

        const Texture baseline = render ( false );
        const Texture with_hidden = render ( true );
        const std::vector<uint8_t>& baseline_pixels = baseline.GetPixels();
        const std::vector<uint8_t>& hidden_pixels = with_hidden.GetPixels();
        for ( uint32_t y = 28; y < 36; ++y )
        {
            for ( uint32_t x = 28; x < 36; ++x )
            {
                const size_t offset = ( static_cast<size_t> ( y ) * baseline.GetWidth() + x ) * 4;
                EXPECT_EQ ( std::memcmp ( baseline_pixels.data() + offset,
                                          hidden_pixels.data() + offset, 4 ), 0 );
            }
        }

        renderer.reset();
        DestroyHiddenRenderWindow ( window );
    }

    TEST ( RendererParityTest, MetalRendersShippedMainScene )
    {
        void* window = CreateHiddenRenderWindow();
        ASSERT_NE ( window, nullptr );
        std::unique_ptr<Renderer> renderer = TryConstructRenderer ( "Metal", window );
        ASSERT_NE ( renderer, nullptr );
        renderer->ResizeViewport ( window, 0, 0, 320, 180 );
        Scene scene;
        const size_t scene_size = GetResourceSize ( "scenes/main.txt" );
        ASSERT_GT ( scene_size, 8u );
        std::vector<char> scene_data ( scene_size );
        LoadResource ( "scenes/main.txt", scene_data.data(), scene_data.size() );
        scene.Deserialize ( std::string {scene_data.begin(), scene_data.end() } );
        scene.Update ( 1.0 / 60.0 );

        renderer->BeginFrame ( window );
        ASSERT_NE ( scene.GetCamera(), nullptr );
        renderer->SetViewMatrix ( window, scene.GetViewMatrix() );
        Matrix4x4 projection{};
        projection.Perspective ( scene.GetFieldOfView(), 320.0f / 180.0f, scene.GetNear(), scene.GetFar() );
        renderer->SetProjectionMatrix ( window, projection );
        renderer->SetLights ( window, scene.GetFrameLights() );
        renderer->SetGlobals ( window, scene.GetGlobals() );
        renderer->SetEnvironmentMap ( window, scene.GetEnvironmentMap() );
        scene.LoopTraverseDFSPreOrder ( [&] ( const Node & aNode )
        {
            aNode.Skin ( *renderer, window );
        } );
        auto render = [&] ( const GuiOverlay * aOverlay )
        {
            Texture capture;
            renderer->RequestCapture ( window );
            renderer->RenderScene ( window, scene, aOverlay );
            renderer->Finish ( window );
            EXPECT_TRUE ( renderer->ReadPixels ( window, capture ) );
            return capture;
        };
        const Texture capture = render ( nullptr );
        std::set<uint32_t> colors;
        EXPECT_GT ( CountCoveredPixels ( capture, colors ), 100u );
        EXPECT_GT ( colors.size(), 8u );
        renderer->BeginFrame ( window );
        scene.LoopTraverseDFSPreOrder ( [&] ( const Node & aNode )
        {
            aNode.Skin ( *renderer, window );
        } );
        const SolidGuiOverlay transparent_overlay{320, 180, {0, 0, 0, 0}};
        const Texture with_overlay = render ( &transparent_overlay );
        EXPECT_EQ ( capture.GetPixels(), with_overlay.GetPixels() );

        renderer.reset();
        DestroyHiddenRenderWindow ( window );
    }
#endif
}