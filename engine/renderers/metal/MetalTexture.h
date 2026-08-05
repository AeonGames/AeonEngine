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
#ifndef AEONGAMES_METALTEXTURE_H
#define AEONGAMES_METALTEXTURE_H

#include <memory>

namespace AeonGames
{
    class Texture;
    /** @brief Sampleable Metal texture uploaded from an engine Texture. */
    class MetalTexture
    {
    public:
        MetalTexture ( void* aDevice, void* aCommandQueue, const Texture& aTexture );
        ~MetalTexture();
        MetalTexture ( MetalTexture&& ) noexcept;
        MetalTexture& operator= ( MetalTexture&& ) noexcept;
        MetalTexture ( const MetalTexture& ) = delete;
        MetalTexture& operator= ( const MetalTexture& ) = delete;

        const Texture& GetTexture() const;
        /** @return Unretained bridge to id<MTLTexture>. */
        void* GetNativeTexture() const;

    private:
        class Impl;
        std::unique_ptr<Impl> mImpl;
    };
}
#endif