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
#ifndef AEONGAMES_GUIOVERLAY_H
#define AEONGAMES_GUIOVERLAY_H

#include <cstdint>
#include <memory>
#include <functional>
#include <string>
#include <vector>
#include "aeongames/Platform.hpp"

namespace AeonGames
{
    class StringId;
    /** Abstract base class for GUI overlay systems.
     *
     * Defines the interface for GUI overlays that rasterize to a CPU-side
     * RGBA pixel buffer. The engine composites this buffer as a full-screen
     * textured quad on top of the 3D scene. Concrete implementations provide
     * the actual GUI toolkit (e.g., AeonGUI).
     */
    class GuiOverlay
    {
    public:
        /** Virtual destructor. */
        DLL virtual ~GuiOverlay() = 0;

        ///@name Frame Lifecycle
        ///@{
        /** Begins a new GUI frame.
         * @param aWindowId Platform dependent window handle.
         * @param aDelta Elapsed time since the last frame, in seconds.
         */
        virtual void BeginFrame ( void* aWindowId, double aDelta ) = 0;
        /** Ends the current GUI frame, triggering rasterization of the widget tree.
         * After this call, GetPixels() returns valid pixel data.
         * @param aWindowId Platform dependent window handle.
         */
        virtual void EndFrame ( void* aWindowId ) = 0;
        ///@}

        ///@name Pixel Buffer Access
        ///@{
        /** Returns a pointer to the rasterized RGBA pixel buffer.
         * Valid only after EndFrame() has been called.
         * @return Pointer to RGBA pixel data, or nullptr if no frame has been rendered.
         */
        virtual const uint8_t* GetPixels() const = 0;
        /** Returns the width of the overlay buffer in pixels. */
        virtual uint32_t GetWidth() const = 0;
        /** Returns the height of the overlay buffer in pixels. */
        virtual uint32_t GetHeight() const = 0;
        ///@}

        ///@name Input Handling
        ///@{
        /** Notifies the overlay of a mouse move event.
         * @param aX X coordinate in window space.
         * @param aY Y coordinate in window space.
         * @return true if the event was consumed by the GUI.
         */
        virtual bool OnMouseMove ( int32_t aX, int32_t aY ) = 0;
        /** Notifies the overlay of a mouse button event.
         * @param aButton Button identifier.
         * @param aPressed true if the button was pressed, false if released.
         * @param aX X coordinate in window space.
         * @param aY Y coordinate in window space.
         * @return true if the event was consumed by the GUI.
         */
        virtual bool OnMouseButton ( int32_t aButton, bool aPressed, int32_t aX, int32_t aY ) = 0;
        /** Notifies the overlay of a key event.
         * @param aKey Key identifier.
         * @param aPressed true if the key was pressed, false if released.
         * @return true if the event was consumed by the GUI.
         */
        virtual bool OnKeyEvent ( uint32_t aKey, bool aPressed ) = 0;
        /** Notifies the overlay of a text input event.
         * @param aCodepoint Unicode codepoint of the character.
         * @return true if the event was consumed by the GUI.
         */
        virtual bool OnTextInput ( uint32_t aCodepoint ) = 0;
        ///@}

        ///@name Resize
        ///@{
        /** Notifies the overlay that the window has been resized.
         * @param aWidth New width in pixels.
         * @param aHeight New height in pixels.
         */
        virtual void Resize ( uint32_t aWidth, uint32_t aHeight ) = 0;
        ///@}

        ///@name Navigation
        ///@{
        /** Navigates the overlay to the specified URL or file path.
         * @param aUrl URL or file path to load.
         */
        virtual void Navigate ( const std::string& aUrl ) = 0;
        ///@}
    };
    /**@name Factory Functions */
    /*@{*/
    /** Constructs a GuiOverlay identified by a numeric identifier.
     * @param aIdentifier Numeric overlay identifier.
     * @param aWindow Platform dependent window handle.
     * @return A unique_ptr to the newly created GuiOverlay.
     */
    DLL std::unique_ptr<GuiOverlay> ConstructGuiOverlay ( uint32_t aIdentifier, void* aWindow );
    /** Constructs a GuiOverlay identified by a string name.
     * @param aIdentifier String overlay identifier.
     * @param aWindow Platform dependent window handle.
     * @return A unique_ptr to the newly created GuiOverlay.
     */
    DLL std::unique_ptr<GuiOverlay> ConstructGuiOverlay ( const std::string& aIdentifier, void* aWindow );
    /** Constructs a GuiOverlay identified by a StringId.
     * @param aIdentifier StringId overlay identifier.
     * @param aWindow Platform dependent window handle.
     * @return A unique_ptr to the newly created GuiOverlay.
     */
    DLL std::unique_ptr<GuiOverlay> ConstructGuiOverlay ( const StringId& aIdentifier, void* aWindow );
    /** Registers a GuiOverlay constructor for a specific identifier.*/
    DLL bool RegisterGuiOverlayConstructor ( const StringId& aIdentifier, const std::function<std::unique_ptr<GuiOverlay> ( void* ) >& aConstructor );
    /** Unregisters a GuiOverlay constructor for a specific identifier.*/
    DLL bool UnregisterGuiOverlayConstructor ( const StringId& aIdentifier );
    /** Enumerates GuiOverlay constructor identifiers via an enumerator functor.*/
    DLL void EnumerateGuiOverlayConstructors ( const std::function<bool ( const StringId& ) >& aEnumerator );
    /** Returns the names of all registered GuiOverlay constructors.
     * @return A vector of registered overlay constructor name strings.
     */
    DLL std::vector<std::string> GetGuiOverlayConstructorNames();
    /*@}*/
}
#endif
