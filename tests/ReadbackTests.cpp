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

#include <algorithm>
#include <cstdint>
#include <memory>
#include <vector>
#include "gtest/gtest.h"
#include "aeongames/AeonEngine.hpp"
#include "aeongames/Renderer.hpp"
#include "aeongames/Texture.hpp"
#include "RenderTestWindow.h"

using namespace ::testing;
namespace AeonGames
{
    /** @brief Capture two identical empty frames and verify the readback contract.
     *
     *  Deliberately asserts no particular colour: EndRender composites a skybox
     *  into the HDR target and tonemaps before resolving, so the final surface is
     *  not the clear colour, and what it does hold is a rendering decision rather
     *  than a property of the readback. What is checked is everything the
     *  readback itself owns: that the capture happens, that the destination is
     *  sized to the surface, that both back-ends deliver RGBA8 whatever their
     *  native surface format is (Vulkan swapchains are typically BGRA), that the
     *  whole buffer is populated rather than truncated, and that repeating the
     *  capture is deterministic. */
    static void RunEmptyFrameReadbackTest ( const char* aRendererName )
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

        auto capture_frame = [&renderer, window] ( Texture & aTexture ) -> bool
        {
            renderer->RequestCapture ( window );
            renderer->BeginFrame ( window );
            renderer->BeginRenderPass ( window );
            renderer->EndRender ( window );
            renderer->Finish ( window );
            return renderer->ReadPixels ( window, aTexture );
        };

        Texture first;
        if ( !capture_frame ( first ) )
        {
            renderer.reset();
            DestroyHiddenRenderWindow ( window );
            GTEST_SKIP() << aRendererName << " cannot read back this surface.";
        }

        EXPECT_EQ ( first.GetFormat(), Texture::Format::RGBA )
                << "readback must normalize to RGBA whatever the surface format is";
        EXPECT_EQ ( first.GetType(), Texture::Type::UNSIGNED_BYTE );
        ASSERT_GT ( first.GetWidth(), 0u );
        ASSERT_GT ( first.GetHeight(), 0u );
        const std::vector<uint8_t> pixels = first.GetPixels();
        ASSERT_EQ ( pixels.size(), static_cast<size_t> ( first.GetWidth() ) * first.GetHeight() * 4 );

        // A short read would leave the tail zeroed, so require the surface to be
        // opaque everywhere; both back-ends present an alpha of 255.
        size_t transparent = 0;
        bool any_colour = false;
        for ( size_t i = 0; i < pixels.size(); i += 4 )
        {
            if ( pixels[i + 3] != 255 )
            {
                ++transparent;
            }
            any_colour = any_colour || pixels[i] != 0 || pixels[i + 1] != 0 || pixels[i + 2] != 0;
        }
        EXPECT_EQ ( transparent, 0u ) << "captured buffer is not fully populated";
        EXPECT_TRUE ( any_colour ) << "captured frame is entirely black; nothing was read";

        Texture second;
        ASSERT_TRUE ( capture_frame ( second ) );
        EXPECT_EQ ( second.GetWidth(), first.GetWidth() );
        EXPECT_EQ ( second.GetHeight(), first.GetHeight() );
        EXPECT_EQ ( second.GetPixels(), pixels )
                << "capturing the same frame twice produced different pixels";

        renderer.reset();
        DestroyHiddenRenderWindow ( window );
    }

#ifdef AEON_TEST_HAVE_OPENGL
    TEST ( ReadbackTest, OpenGLEmptyFrame )
    {
        RunEmptyFrameReadbackTest ( "OpenGL" );
    }
#endif

#ifdef AEON_TEST_HAVE_VULKAN_WINDOW
    TEST ( ReadbackTest, VulkanEmptyFrame )
    {
        RunEmptyFrameReadbackTest ( "Vulkan" );
    }
#endif

#ifdef AEON_TEST_HAVE_METAL
    TEST ( ReadbackTest, MetalEmptyFrame )
    {
        RunEmptyFrameReadbackTest ( "Metal" );
    }
#endif
}
