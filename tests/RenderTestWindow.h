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
#ifndef AEONGAMES_RENDERTESTWINDOW_H
#define AEONGAMES_RENDERTESTWINDOW_H

/** @file Off-screen window and renderer construction helpers shared by the
 *  GPU-dependent test suites. */

#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include "aeongames/Platform.hpp"
#include "aeongames/Renderer.hpp"
#if defined(__unix__) && !defined(__APPLE__)
#include <X11/Xlib.h>
#include <GL/glx.h>
#endif
#ifdef AEON_TEST_HAVE_VULKAN
#include <vulkan/vulkan.h>
#endif

namespace AeonGames
{
#if defined(__APPLE__)
    /** @brief Create/destroy the hidden AppKit surface implemented in the
     *  Objective-C++ test helper. The returned handle is an NSView*, matching
     *  application/macos.mm and both macOS-capable renderer plugins. */
    void* CreateHiddenRenderWindowMac();
    void DestroyHiddenRenderWindowMac ( void* aWindow );
#endif
#if defined(__unix__) && !defined(__APPLE__)
    /** @brief Process-wide X display owned by the test harness.
     *
     *  Kept open for the lifetime of the process because closing the connection
     *  would destroy every window created on it. */
    inline Display* TestDisplay()
    {
        static Display* display = XOpenDisplay ( nullptr );
        return display;
    }
#endif

    /** @brief Create an off-screen window suitable for hosting a renderer
     *  surface without ever being shown, or nullptr where unsupported.
     *
     *  Returned as void* because that is what ConstructRenderer takes: an HWND
     *  on Windows, an X11 window id on Linux. */
    inline void* CreateHiddenRenderWindow()
    {
#if defined(_WIN32)
        WNDCLASSEX wcex {};
        wcex.cbSize = sizeof ( WNDCLASSEX );
        wcex.style = CS_OWNDC;
        wcex.lpfnWndProc = DefWindowProc;
        wcex.hInstance = GetModuleHandle ( nullptr );
        wcex.lpszClassName = "AeonRenderTestWindow";
        RegisterClassEx ( &wcex );
        return CreateWindowEx ( 0, "AeonRenderTestWindow", "AeonRenderTest",
                                WS_POPUP, 0, 0, 64, 64, nullptr, nullptr,
                                GetModuleHandle ( nullptr ), nullptr );
#elif defined(__unix__) && !defined(__APPLE__)
        Display* display = TestDisplay();
        if ( display == nullptr )
        {
            return nullptr;
        }
        // The OpenGL back-end derives its GLX framebuffer config from the
        // window's visual, so the window needs a GLX visual even for tests that
        // only exercise compute.
        int visual_attribs[]
        {
            GLX_RGBA, GLX_DOUBLEBUFFER,
            GLX_RED_SIZE, 8, GLX_GREEN_SIZE, 8, GLX_BLUE_SIZE, 8, GLX_ALPHA_SIZE, 8,
            GLX_DEPTH_SIZE, 24, GLX_STENCIL_SIZE, 8,
            None
        };
        XVisualInfo* visual_info = glXChooseVisual ( display, DefaultScreen ( display ), visual_attribs );
        if ( visual_info == nullptr )
        {
            return nullptr;
        }
        ::Window root = DefaultRootWindow ( display );
        XSetWindowAttributes window_attributes{};
        window_attributes.colormap = XCreateColormap ( display, root, visual_info->visual, AllocNone );
        window_attributes.border_pixel = 0;
        // override_redirect keeps the window away from the window manager so it
        // is never mapped, focused or shown while the suite runs.
        window_attributes.override_redirect = True;
        ::Window window = XCreateWindow ( display, root, 0, 0, 64, 64, 0,
                                          visual_info->depth, InputOutput, visual_info->visual,
                                          CWBorderPixel | CWColormap | CWOverrideRedirect,
                                          &window_attributes );
        XFree ( visual_info );
        XSync ( display, False );
        return reinterpret_cast<void*> ( window );
#elif defined(__APPLE__)
        return CreateHiddenRenderWindowMac();
#else
        return nullptr;
#endif
    }

    /** @brief Destroy a window returned by CreateHiddenRenderWindow. */
    inline void DestroyHiddenRenderWindow ( void* aWindow )
    {
#if defined(_WIN32)
        DestroyWindow ( static_cast<HWND> ( aWindow ) );
#elif defined(__unix__) && !defined(__APPLE__)
        if ( Display * display = TestDisplay() )
        {
            XDestroyWindow ( display, reinterpret_cast< ::Window> ( aWindow ) );
            XFlush ( display );
        }
#elif defined(__APPLE__)
        DestroyHiddenRenderWindowMac ( aWindow );
#else
        ( void ) aWindow;
#endif
    }

#ifdef AEON_TEST_HAVE_VULKAN
    /** @brief Probe whether the host has a usable Vulkan implementation.
     *
     *  Returns true only when a Vulkan instance can be created and at least one
     *  physical device is enumerable. Headless CI runners may ship a Vulkan
     *  loader yet have no compatible driver, in which case instance creation
     *  returns VK_ERROR_INCOMPATIBLE_DRIVER and this returns false so the
     *  GPU-dependent tests skip instead of crashing. */
    inline bool IsVulkanAvailableOnHost()
    {
        VkApplicationInfo app_info{};
        app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        app_info.pApplicationName = "AeonRenderTests";
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
     *  exception lets the GPU-dependent tests skip gracefully rather than fail
     *  the suite. */
    inline std::unique_ptr<Renderer> TryConstructRenderer ( const char* aRendererName, void* aWindow )
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
}
#endif
