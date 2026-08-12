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

#include "aeongames/CharacterLibrary.hpp"

#include "aeongames/ProtoBufClasses.hpp"
#include "aeongames/CRC.hpp"
#include "aeongames/Database.hpp"
#include "aeongames/Material.hpp"
#include "aeongames/Model.hpp"
#include "aeongames/ResourceCache.hpp"
#include "aeongames/Texture.hpp"

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : PROTOBUF_WARNINGS )
#endif
#include "model.pb.h"
#ifdef _MSC_VER
#pragma warning( pop )
#endif

#include <array>
#include <sstream>
#include <stdexcept>

namespace AeonGames
{
    namespace
    {
        /** Palette coordinates address a 2x2 block, so the texture is twice the
            16x16 grid of colour slots. */
        constexpr uint32_t kPaletteSize = 32;
        constexpr uint32_t kBlockSize = 2;
        /** Texels no colour slot owns are left magenta, matching the palettes
            the cooker writes, so an unassigned slot is obvious rather than black. */
        constexpr std::array<uint8_t, 4> kUnusedTexel{255, 0, 255, 255};
        constexpr const char* kColorSampler = "DiffuseMap";

        uint32_t Crc ( const std::string& aString )
        {
            return crc32i ( aString.data(), aString.size() );
        }
    }

    CharacterLibrary::CharacterLibrary ( Database& aDatabase ) : mDatabase{aDatabase}
    {
        std::unique_ptr<Database::Statement> library =
            mDatabase.Prepare ( "SELECT skeleton, pipeline, material FROM character_library WHERE id = 1" );
        if ( !library->Step() )
        {
            throw std::runtime_error ( "The database holds no character library." );
        }
        mSkeleton = library->GetText ( 0 );
        mPipeline = library->GetText ( 1 );
        mMaterial = library->GetText ( 2 );

        std::unique_ptr<Database::Statement> animations =
            mDatabase.Prepare ( "SELECT name, path FROM character_animation ORDER BY name" );
        while ( animations->Step() )
        {
            mAnimations.emplace_back ( animations->GetText ( 0 ), animations->GetText ( 1 ) );
        }
    }

    CharacterLibrary::~CharacterLibrary() = default;

    std::vector<CharacterLibrary::Faction> CharacterLibrary::GetFactions() const
    {
        std::vector<Faction> factions{};
        std::unique_ptr<Database::Statement> select =
            mDatabase.Prepare ( "SELECT id, name FROM character_faction ORDER BY name" );
        while ( select->Step() )
        {
            factions.emplace_back ( Faction{select->GetInt ( 0 ), select->GetText ( 1 ) } );
        }
        return factions;
    }

    std::vector<CharacterLibrary::Outfit> CharacterLibrary::GetOutfits ( const Faction& aFaction ) const
    {
        std::vector<Outfit> outfits{};
        std::unique_ptr<Database::Statement> select = mDatabase.Prepare (
                "SELECT id, faction_id, name FROM character_outfit WHERE faction_id = ?1 ORDER BY name" );
        select->Bind ( 1, aFaction.mId );
        while ( select->Step() )
        {
            outfits.emplace_back ( Outfit{select->GetInt ( 0 ), select->GetInt ( 1 ), select->GetText ( 2 ) } );
        }
        return outfits;
    }

    std::vector<CharacterLibrary::Palette> CharacterLibrary::GetPalettes() const
    {
        std::vector<Palette> palettes{};
        std::unique_ptr<Database::Statement> select =
            mDatabase.Prepare ( "SELECT id, name FROM character_palette ORDER BY name" );
        while ( select->Step() )
        {
            palettes.emplace_back ( Palette{select->GetInt ( 0 ), select->GetText ( 1 ) } );
        }
        return palettes;
    }

    ResourceId CharacterLibrary::Compose ( const Outfit& aOutfit, const Palette& aPalette ) const
    {
        std::ostringstream name;
        name << "character/" << aOutfit.mId << '/' << aPalette.mId;
        const uint32_t model_id = Crc ( name.str() );
        if ( GetResource ( model_id ).GetRaw() != nullptr )
        {
            return {"Model"_crc32, model_id};
        }

        const uint32_t palette_id = Crc ( name.str() + "/palette" );
        {
            std::vector<uint8_t> pixels ( static_cast<size_t> ( kPaletteSize ) * kPaletteSize * kUnusedTexel.size() );
            for ( size_t texel = 0; texel < pixels.size(); texel += kUnusedTexel.size() )
            {
                std::copy ( kUnusedTexel.begin(), kUnusedTexel.end(), pixels.begin() + texel );
            }
            std::unique_ptr<Database::Statement> select = mDatabase.Prepare (
                    "SELECT slot.u, slot.v, entry.color FROM character_palette_entry AS entry"
                    " JOIN character_color_slot AS slot ON slot.id = entry.slot_id"
                    " WHERE entry.palette_id = ?1" );
            select->Bind ( 1, aPalette.mId );
            while ( select->Step() )
            {
                const uint32_t x = static_cast<uint32_t> ( select->GetInt ( 0 ) ) * kBlockSize;
                const uint32_t y = static_cast<uint32_t> ( select->GetInt ( 1 ) ) * kBlockSize;
                const int64_t color = select->GetInt ( 2 );
                if ( ( x + kBlockSize ) > kPaletteSize || ( y + kBlockSize ) > kPaletteSize )
                {
                    std::ostringstream stream;
                    stream << "Colour slot at (" << x / kBlockSize << "," << y / kBlockSize
                           << ") falls outside a " << kPaletteSize << "x" << kPaletteSize << " palette.";
                    throw std::runtime_error ( stream.str() );
                }
                for ( uint32_t dy = 0; dy < kBlockSize; ++dy )
                {
                    // Palette coordinates start at the bottom left, images at the top.
                    const uint32_t row = kPaletteSize - 1 - ( y + dy );
                    for ( uint32_t dx = 0; dx < kBlockSize; ++dx )
                    {
                        uint8_t* texel = pixels.data() +
                                         ( ( ( static_cast<size_t> ( row ) * kPaletteSize ) + x + dx ) * kUnusedTexel.size() );
                        texel[0] = static_cast<uint8_t> ( ( color >> 16 ) & 0xff );
                        texel[1] = static_cast<uint8_t> ( ( color >> 8 ) & 0xff );
                        texel[2] = static_cast<uint8_t> ( color & 0xff );
                        texel[3] = 255;
                    }
                }
            }
            std::unique_ptr<Texture> texture = std::make_unique<Texture>();
            texture->Resize ( kPaletteSize, kPaletteSize, pixels.data(),
                              Texture::Format::RGBA, Texture::Type::UNSIGNED_BYTE );
            StoreResource ( palette_id, UniqueAnyPtr{std::move ( texture ) } );
        }

        const uint32_t material_id = Crc ( name.str() + "/material" );
        {
            const Material* material_template = ResourceId{"Material", mMaterial}.Get<Material>();
            if ( material_template == nullptr )
            {
                throw std::runtime_error ( "Unable to load the character material " + mMaterial + "." );
            }
            std::unique_ptr<Material> material = std::make_unique<Material> ( *material_template );
            material->SetSampler ( kColorSampler, ResourceId{"Texture"_crc32, palette_id} );
            StoreResource ( material_id, UniqueAnyPtr{std::move ( material ) } );
        }

        ModelMsg model_msg{};
        model_msg.mutable_default_pipeline()->set_path ( mPipeline );
        model_msg.mutable_skeleton()->set_path ( mSkeleton );
        // The material and the palette only exist in the cache, so they are
        // referenced by id; a path would send the loader looking on disk.
        model_msg.mutable_default_material()->set_id ( material_id );
        {
            std::unique_ptr<Database::Statement> select = mDatabase.Prepare (
                    "SELECT part.mesh FROM character_outfit_part AS outfit_part"
                    " JOIN character_part AS part ON part.id = outfit_part.part_id"
                    " WHERE outfit_part.outfit_id = ?1 ORDER BY part.part_type_id" );
            select->Bind ( 1, aOutfit.mId );
            while ( select->Step() )
            {
                model_msg.add_assembly()->mutable_mesh()->set_path ( select->GetText ( 0 ) );
            }
        }
        if ( model_msg.assembly_size() == 0 )
        {
            throw std::runtime_error ( "Outfit " + aOutfit.mName + " names no parts." );
        }
        for ( const auto& animation : mAnimations )
        {
            AnimationRefMsg* reference = model_msg.add_animation();
            reference->set_name ( animation.first );
            reference->mutable_reference()->set_path ( animation.second );
        }

        std::unique_ptr<Model> model = std::make_unique<Model>();
        model->LoadFromPBMsg ( model_msg );
        StoreResource ( model_id, UniqueAnyPtr{std::move ( model ) } );
        return {"Model"_crc32, model_id};
    }
}
