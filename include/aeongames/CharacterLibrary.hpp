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
#ifndef AEONGAMES_CHARACTERLIBRARY_H
#define AEONGAMES_CHARACTERLIBRARY_H

#include "aeongames/Platform.hpp"
#include "aeongames/ResourceId.hpp"
#include <cstdint>
#include <string>
#include <vector>

namespace AeonGames
{
    class Database;

    /** @brief Builds characters out of interchangeable parts held in a database.
     *
     *  A character is not an asset here. The library stores the parts, the
     *  outfits that name a set of them and the palettes that colour them, and a
     *  character is whatever combination is asked for at run time. Nothing is
     *  written to disk: the palette becomes a texture in memory, the material
     *  and model are placed straight into the resource cache, and only the part
     *  meshes and the skeleton are read from the package.
     *
     *  Every part in a library is rigged to the same skeleton, which is what
     *  makes them interchangeable, so the library carries the skeleton, the
     *  pipeline and the material template it was cooked against rather than
     *  being told about them separately.
     */
    class CharacterLibrary
    {
    public:
        /** @brief A group of outfits, such as the faction an NPC belongs to. */
        struct Faction
        {
            int64_t mId;
            std::string mName;
        };
        /** @brief A named set of parts making up a whole character. */
        struct Outfit
        {
            int64_t mId;
            int64_t mFactionId;
            std::string mName;
        };
        /** @brief A named set of colours to paint a character with. */
        struct Palette
        {
            int64_t mId;
            std::string mName;
        };

        /** @brief Read a character library from an open database.
         *  @param aDatabase A database holding the character tables. It is not
         *         owned and must outlive the library.
         *  @throws std::runtime_error when it holds no character library. */
        DLL CharacterLibrary ( Database& aDatabase );
        DLL ~CharacterLibrary();

        /** @brief Every faction in the library. */
        DLL std::vector<Faction> GetFactions() const;
        /** @brief Every outfit belonging to @p aFaction. */
        DLL std::vector<Outfit> GetOutfits ( const Faction& aFaction ) const;
        /** @brief Every palette in the library. */
        DLL std::vector<Palette> GetPalettes() const;

        /** @brief Build the character wearing @p aOutfit painted with @p aPalette.
         *
         *  The same pair always identifies the same character, so a crowd of
         *  NPCs sharing an outfit and a palette shares one model, one material
         *  and one palette texture.
         *
         *  @return The composed model, ready to hand to a ModelComponent.
         *  @throws std::runtime_error when the outfit names no parts. */
        DLL ResourceId Compose ( const Outfit& aOutfit, const Palette& aPalette ) const;

    private:
        Database& mDatabase;
        std::string mSkeleton{};
        std::string mPipeline{};
        std::string mMaterial{};
        std::vector<std::pair<std::string, std::string>> mAnimations{};
    };
}
#endif
