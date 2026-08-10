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
#include "aeongames/Database.hpp"
#include "aeongames/Material.hpp"
#include "aeongames/Model.hpp"
#include "aeongames/StringId.hpp"
#include "aeongames/Texture.hpp"
#include "gtest/gtest.h"

#include <filesystem>
#include <memory>
#include <string>

namespace AeonGames
{
    namespace
    {
        /** The library the transcoder writes, reduced to what the runtime reads.
            Parts point at stock assets so no character pack is needed to test. */
        constexpr char kSchema[] =
            "CREATE TABLE character_library ("
            " id INTEGER PRIMARY KEY CHECK ( id = 1 ),"
            " skeleton TEXT NOT NULL, pipeline TEXT NOT NULL, material TEXT NOT NULL );"
            "CREATE TABLE character_animation ( name TEXT PRIMARY KEY, path TEXT NOT NULL );"
            "CREATE TABLE character_faction ( id INTEGER PRIMARY KEY, name TEXT NOT NULL UNIQUE );"
            "CREATE TABLE character_part_type ("
            " id INTEGER PRIMARY KEY, code TEXT NOT NULL UNIQUE, name TEXT NOT NULL UNIQUE );"
            "CREATE TABLE character_part ("
            " id INTEGER PRIMARY KEY, part_type_id INTEGER NOT NULL, name TEXT NOT NULL UNIQUE,"
            " mesh TEXT NOT NULL );"
            "CREATE TABLE character_outfit ("
            " id INTEGER PRIMARY KEY, faction_id INTEGER, species_id INTEGER, name TEXT NOT NULL );"
            "CREATE TABLE character_outfit_part ("
            " outfit_id INTEGER NOT NULL, part_id INTEGER NOT NULL,"
            " PRIMARY KEY ( outfit_id, part_id ) );"
            "CREATE TABLE character_color_slot ("
            " id INTEGER PRIMARY KEY, name TEXT NOT NULL, u INTEGER NOT NULL, v INTEGER NOT NULL );"
            "CREATE TABLE character_palette ("
            " id INTEGER PRIMARY KEY, species_id INTEGER, name TEXT NOT NULL );"
            "CREATE TABLE character_palette_entry ("
            " palette_id INTEGER NOT NULL, slot_id INTEGER NOT NULL, color INTEGER NOT NULL,"
            " metallic INTEGER NOT NULL, smoothness INTEGER NOT NULL, reflection INTEGER NOT NULL,"
            " emission INTEGER NOT NULL, opacity INTEGER NOT NULL,"
            " PRIMARY KEY ( palette_id, slot_id ) );";

        constexpr uint32_t kPaletteSize = 32;
        constexpr int64_t kTunicColor = 0x336699;

        class CharacterLibraryTest : public ::testing::Test
        {
        protected:
            void SetUp() override
            {
                mRoot = std::filesystem::temp_directory_path() /
                        ( "aeon-character-test-" + std::to_string ( ::testing::UnitTest::GetInstance()->random_seed() ) );
                std::filesystem::remove_all ( mRoot );
                std::filesystem::create_directories ( mRoot );
                mDatabase = ConstructDatabase ( StringId{"SQLite"} );
                ASSERT_NE ( mDatabase, nullptr );
                mDatabase->Open ( ( mRoot / "characters.db" ).string(), Database::OpenMode::ReadWriteCreate );
                mDatabase->Execute ( kSchema );
                mDatabase->Execute (
                    "INSERT INTO character_library VALUES ( 1, 'aerin/skeletons/skeleton.txt',"
                    " 'shaders/clustered_phong', 'aerin/materials/body.txt' );"
                    "INSERT INTO character_animation VALUES ( 'Idle', 'aerin/animations/Idle.txt' );"
                    "INSERT INTO character_faction VALUES ( 1, 'Villagers' ), ( 2, 'Guards' );"
                    "INSERT INTO character_part_type VALUES ( 1, 'TORS', 'Torso' ), ( 2, 'LEGS', 'Legs' );"
                    "INSERT INTO character_part VALUES"
                    " ( 1, 1, 'Body', 'aerin/meshes/Body.txt' ),"
                    " ( 2, 2, 'Bottoms', 'aerin/meshes/Bottoms.txt' ),"
                    " ( 3, 1, 'Tops', 'aerin/meshes/Tops.txt' );"
                    "INSERT INTO character_outfit VALUES ( 1, 1, NULL, 'Peasant' ), ( 2, 2, NULL, 'Sentry' ),"
                    " ( 3, 1, NULL, 'Empty' );"
                    "INSERT INTO character_outfit_part VALUES ( 1, 1 ), ( 1, 2 ), ( 2, 1 ), ( 2, 3 );"
                    "INSERT INTO character_color_slot VALUES ( 1, 'Tunic', 1, 1 ), ( 2, 'Skin', 0, 0 );"
                    "INSERT INTO character_palette VALUES ( 1, NULL, 'Autumn' ), ( 2, NULL, 'Winter' );"
                    "INSERT INTO character_palette_entry VALUES"
                    " ( 1, 1, 3368601, 0, 0, 0, 0, 0 ),"
                    " ( 1, 2, 16777215, 0, 0, 0, 0, 0 ),"
                    " ( 2, 1, 255, 0, 0, 0, 0, 0 );" );
                mLibrary = std::make_unique<CharacterLibrary> ( *mDatabase );
            }
            void TearDown() override
            {
                mLibrary.reset();
                mDatabase.reset();
                std::error_code error{};
                std::filesystem::remove_all ( mRoot, error );
            }
            std::filesystem::path mRoot{};
            std::unique_ptr<Database> mDatabase{};
            std::unique_ptr<CharacterLibrary> mLibrary{};
        };
    }

    TEST_F ( CharacterLibraryTest, ListsFactionsOutfitsAndPalettes )
    {
        const std::vector<CharacterLibrary::Faction> factions = mLibrary->GetFactions();
        ASSERT_EQ ( factions.size(), 2u );
        EXPECT_EQ ( factions[0].mName, "Guards" );
        EXPECT_EQ ( factions[1].mName, "Villagers" );

        const std::vector<CharacterLibrary::Outfit> outfits = mLibrary->GetOutfits ( factions[0] );
        ASSERT_EQ ( outfits.size(), 1u );
        EXPECT_EQ ( outfits[0].mName, "Sentry" );
        EXPECT_EQ ( outfits[0].mFactionId, factions[0].mId );

        EXPECT_EQ ( mLibrary->GetPalettes().size(), 2u );
    }

    TEST_F ( CharacterLibraryTest, ComposesAModelOutOfTheOutfitParts )
    {
        const CharacterLibrary::Outfit outfit{1, 1, "Peasant"};
        const CharacterLibrary::Palette palette{1, "Autumn"};
        const ResourceId id = mLibrary->Compose ( outfit, palette );
        ASSERT_TRUE ( id );

        const Model* model = id.Cast<Model>();
        ASSERT_NE ( model, nullptr );
        EXPECT_EQ ( model->GetAssemblies().size(), 2u );
        EXPECT_NE ( model->GetSkeleton(), nullptr );
        ASSERT_EQ ( model->GetAnimationNames().size(), 1u );
        EXPECT_EQ ( model->GetAnimationNames().front(), "Idle" );
    }

    /** A crowd of NPCs in the same outfit and colours is one model, not many. */
    TEST_F ( CharacterLibraryTest, SharesOneModelPerOutfitAndPalette )
    {
        const CharacterLibrary::Outfit outfit{1, 1, "Peasant"};
        const CharacterLibrary::Palette autumn{1, "Autumn"};
        const CharacterLibrary::Palette winter{2, "Winter"};

        const ResourceId first = mLibrary->Compose ( outfit, autumn );
        const ResourceId again = mLibrary->Compose ( outfit, autumn );
        const ResourceId recolored = mLibrary->Compose ( outfit, winter );

        EXPECT_EQ ( first.GetPath(), again.GetPath() );
        EXPECT_EQ ( first.Cast<Model>(), again.Cast<Model>() );
        EXPECT_NE ( first.GetPath(), recolored.GetPath() );
    }

    /** The palette never reaches disk; it is built into a texture and bound to
        the material the composed model uses. */
    TEST_F ( CharacterLibraryTest, PaintsThePaletteIntoTheMaterialTexture )
    {
        const ResourceId id = mLibrary->Compose ( { 1, 1, "Peasant" }, { 1, "Autumn" } );
        const Model* model = id.Cast<Model>();
        ASSERT_NE ( model, nullptr );
        ASSERT_FALSE ( model->GetAssemblies().empty() );

        Material* material = std::get<2> ( model->GetAssemblies().front() ).Cast<Material>();
        ASSERT_NE ( material, nullptr );
        Texture* palette = material->GetSampler ( "DiffuseMap" ).Cast<Texture>();
        ASSERT_NE ( palette, nullptr );
        EXPECT_EQ ( palette->GetWidth(), kPaletteSize );
        EXPECT_EQ ( palette->GetHeight(), kPaletteSize );

        const std::vector<uint8_t>& pixels = palette->GetPixels();
        ASSERT_EQ ( pixels.size(), static_cast<size_t> ( kPaletteSize ) * kPaletteSize * 4 );
        // Slot (1,1) covers a 2x2 block at (2,2) counted from the bottom left.
        const size_t row = kPaletteSize - 1 - 2;
        const size_t texel = ( ( row * kPaletteSize ) + 2 ) * 4;
        EXPECT_EQ ( pixels[texel + 0], ( kTunicColor >> 16 ) & 0xff );
        EXPECT_EQ ( pixels[texel + 1], ( kTunicColor >> 8 ) & 0xff );
        EXPECT_EQ ( pixels[texel + 2], kTunicColor & 0xff );
        EXPECT_EQ ( pixels[texel + 3], 255 );

        // A slot the palette never assigns stays magenta rather than black.
        const size_t unused = ( ( ( kPaletteSize / 2 ) * kPaletteSize ) + ( kPaletteSize / 2 ) ) * 4;
        EXPECT_EQ ( pixels[unused + 0], 255 );
        EXPECT_EQ ( pixels[unused + 1], 0 );
        EXPECT_EQ ( pixels[unused + 2], 255 );
    }

    TEST_F ( CharacterLibraryTest, RecolouringReusesTheMeshesButNotTheMaterial )
    {
        const Model* autumn = mLibrary->Compose ( { 1, 1, "Peasant" }, { 1, "Autumn" } ).Cast<Model>();
        const Model* winter = mLibrary->Compose ( { 1, 1, "Peasant" }, { 2, "Winter" } ).Cast<Model>();
        ASSERT_NE ( autumn, nullptr );
        ASSERT_NE ( winter, nullptr );

        EXPECT_EQ ( std::get<0> ( autumn->GetAssemblies().front() ).GetPath(),
                    std::get<0> ( winter->GetAssemblies().front() ).GetPath() );
        EXPECT_NE ( std::get<2> ( autumn->GetAssemblies().front() ).GetPath(),
                    std::get<2> ( winter->GetAssemblies().front() ).GetPath() );
    }

    TEST_F ( CharacterLibraryTest, RefusesAnOutfitWithNoParts )
    {
        EXPECT_THROW ( mLibrary->Compose ( { 3, 1, "Empty" }, { 1, "Autumn" } ), std::runtime_error );
    }

    TEST_F ( CharacterLibraryTest, RefusesADatabaseThatHoldsNoLibrary )
    {
        std::unique_ptr<Database> empty = ConstructDatabase ( StringId{"SQLite"} );
        ASSERT_NE ( empty, nullptr );
        empty->Open ( ( mRoot / "empty.db" ).string(), Database::OpenMode::ReadWriteCreate );
        empty->Execute ( kSchema );
        EXPECT_THROW ( CharacterLibrary{*empty}, std::runtime_error );
    }
}
