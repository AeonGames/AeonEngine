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

#include <sqlite3.h>

#include <array>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "SidekickDatabase.h"

namespace AeonGames
{
    namespace
    {
        /** Synty stores a part's slot as a bare integer; the readable names live
            in its tooling rather than the database. The integer maps one to one
            onto the four letter code every mesh name carries. */
        struct PartTypeDesc
        {
            const char* mCode;
            const char* mName;
        };

        constexpr std::array<PartTypeDesc, 38> kPartTypes
        {
            {
                { "HEAD", "Head" }, { "HAIR", "Hair" },
                { "EBRL", "EyebrowLeft" }, { "EBRR", "EyebrowRight" },
                { "EYEL", "EyeLeft" }, { "EYER", "EyeRight" },
                { "EARL", "EarLeft" }, { "EARR", "EarRight" },
                { "FCHR", "FacialHair" }, { "TORS", "Torso" },
                { "AUPL", "ArmUpperLeft" }, { "AUPR", "ArmUpperRight" },
                { "ALWL", "ArmLowerLeft" }, { "ALWR", "ArmLowerRight" },
                { "HNDL", "HandLeft" }, { "HNDR", "HandRight" },
                { "HIPS", "Hips" }, { "LEGL", "LegLeft" }, { "LEGR", "LegRight" },
                { "FOTL", "FootLeft" }, { "FOTR", "FootRight" },
                { "AHED", "AttachmentHead" }, { "AFAC", "AttachmentFace" },
                { "ABAC", "AttachmentBack" }, { "AHPF", "AttachmentHipFront" },
                { "AHPB", "AttachmentHipBack" }, { "AHPL", "AttachmentHipLeft" },
                { "AHPR", "AttachmentHipRight" }, { "ASHL", "AttachmentShoulderLeft" },
                { "ASHR", "AttachmentShoulderRight" }, { "AEBL", "AttachmentElbowLeft" },
                { "AEBR", "AttachmentElbowRight" }, { "AKNL", "AttachmentKneeLeft" },
                { "AKNR", "AttachmentKneeRight" }, { "NOSE", "Nose" },
                { "TETH", "Teeth" }, { "TONG", "Tongue" }, { "WRAP", "Wrap" },
            }
        };

        /* The engine's own character library. Names and shapes are ours, so a
           later source -- or hand authored content -- can populate the same
           tables, and they can sit in the database that already holds world and
           player state. */
        const char* const kSchema =
            "PRAGMA journal_mode = OFF;"
            "CREATE TABLE character_species ("
            " id INTEGER PRIMARY KEY, name TEXT NOT NULL UNIQUE );"
            "CREATE TABLE character_part_type ("
            " id INTEGER PRIMARY KEY, code TEXT NOT NULL UNIQUE, name TEXT NOT NULL UNIQUE );"
            "CREATE TABLE character_part ("
            " id INTEGER PRIMARY KEY,"
            " part_type_id INTEGER NOT NULL REFERENCES character_part_type(id),"
            " name TEXT NOT NULL UNIQUE,"
            " mesh TEXT NOT NULL );"
            "CREATE TABLE character_part_species ("
            " part_id INTEGER NOT NULL REFERENCES character_part(id),"
            " species_id INTEGER NOT NULL REFERENCES character_species(id),"
            " PRIMARY KEY ( part_id, species_id ) );"
            "CREATE TABLE character_faction ("
            " id INTEGER PRIMARY KEY, name TEXT NOT NULL UNIQUE );"
            "CREATE TABLE character_outfit ("
            " id INTEGER PRIMARY KEY,"
            " faction_id INTEGER REFERENCES character_faction(id),"
            " species_id INTEGER REFERENCES character_species(id),"
            " name TEXT NOT NULL );"
            "CREATE TABLE character_outfit_part ("
            " outfit_id INTEGER NOT NULL REFERENCES character_outfit(id),"
            " part_id INTEGER NOT NULL REFERENCES character_part(id),"
            " PRIMARY KEY ( outfit_id, part_id ) );"
            "CREATE TABLE character_color_slot ("
            " id INTEGER PRIMARY KEY, name TEXT NOT NULL, u INTEGER NOT NULL, v INTEGER NOT NULL );"
            "CREATE TABLE character_palette ("
            " id INTEGER PRIMARY KEY,"
            " species_id INTEGER REFERENCES character_species(id),"
            " name TEXT NOT NULL );"
            "CREATE TABLE character_palette_entry ("
            " palette_id INTEGER NOT NULL REFERENCES character_palette(id),"
            " slot_id INTEGER NOT NULL REFERENCES character_color_slot(id),"
            " color INTEGER NOT NULL, metallic INTEGER NOT NULL, smoothness INTEGER NOT NULL,"
            " reflection INTEGER NOT NULL, emission INTEGER NOT NULL, opacity INTEGER NOT NULL,"
            " PRIMARY KEY ( palette_id, slot_id ) );"
            "CREATE INDEX character_outfit_by_faction ON character_outfit ( faction_id );"
            "CREATE INDEX character_part_by_type ON character_part ( part_type_id );"
            // Every part is cooked against one rig and shaded the same way, so
            // the library carries its own skeleton and shading rather than
            // relying on the runtime being told out of band.
            "CREATE TABLE character_library ("
            " id INTEGER PRIMARY KEY CHECK ( id = 1 ),"
            " skeleton TEXT NOT NULL, pipeline TEXT NOT NULL, material TEXT NOT NULL );"
            "CREATE TABLE character_animation ("
            " name TEXT PRIMARY KEY, path TEXT NOT NULL );";

        /** @brief Owns a sqlite handle so an exception cannot leak it. */
        class Database
        {
        public:
            Database ( const std::string& aPath, int aFlags )
            {
                if ( sqlite3_open_v2 ( aPath.c_str(), &mHandle, aFlags, nullptr ) != SQLITE_OK )
                {
                    std::ostringstream stream;
                    stream << "Unable to open " << aPath << ": "
                           << ( ( mHandle != nullptr ) ? sqlite3_errmsg ( mHandle ) : "out of memory" );
                    sqlite3_close ( mHandle );
                    throw std::runtime_error ( stream.str() );
                }
            }
            ~Database()
            {
                sqlite3_close ( mHandle );
            }
            Database ( const Database& ) = delete;
            Database& operator= ( const Database& ) = delete;
            sqlite3* Get() const
            {
                return mHandle;
            }
            void Execute ( const char* aSql ) const
            {
                char* message = nullptr;
                if ( sqlite3_exec ( mHandle, aSql, nullptr, nullptr, &message ) != SQLITE_OK )
                {
                    std::ostringstream stream;
                    stream << "SQL failed: " << ( ( message != nullptr ) ? message : "unknown error" );
                    sqlite3_free ( message );
                    throw std::runtime_error ( stream.str() );
                }
            }
        private:
            sqlite3* mHandle{nullptr};
        };

        /** @brief Owns a prepared statement and resets it between uses. */
        class Statement
        {
        public:
            Statement ( const Database& aDatabase, const char* aSql )
            {
                if ( sqlite3_prepare_v2 ( aDatabase.Get(), aSql, -1, &mHandle, nullptr ) != SQLITE_OK )
                {
                    std::ostringstream stream;
                    stream << "Unable to prepare '" << aSql << "': " << sqlite3_errmsg ( aDatabase.Get() );
                    throw std::runtime_error ( stream.str() );
                }
            }
            ~Statement()
            {
                sqlite3_finalize ( mHandle );
            }
            Statement ( const Statement& ) = delete;
            Statement& operator= ( const Statement& ) = delete;
            sqlite3_stmt* Get() const
            {
                return mHandle;
            }
            bool Step() const
            {
                const int result = sqlite3_step ( mHandle );
                if ( result == SQLITE_ROW )
                {
                    return true;
                }
                if ( result != SQLITE_DONE )
                {
                    std::ostringstream stream;
                    stream << "Statement failed: " << sqlite3_errmsg ( sqlite3_db_handle ( mHandle ) );
                    throw std::runtime_error ( stream.str() );
                }
                return false;
            }
            void Run() const
            {
                while ( Step() ) {}
            sqlite3_reset ( mHandle );
            sqlite3_clear_bindings ( mHandle );
        }
        void Bind ( int aIndex, int64_t aValue ) const
            {
                sqlite3_bind_int64 ( mHandle, aIndex, aValue );
            }
            void Bind ( int aIndex, const std::string& aValue ) const
            {
                sqlite3_bind_text ( mHandle, aIndex, aValue.c_str(), -1, SQLITE_TRANSIENT );
            }
            void BindNull ( int aIndex ) const
            {
                sqlite3_bind_null ( mHandle, aIndex );
            }
        private:
            sqlite3_stmt* mHandle{nullptr};
        };

        std::string Column ( const Statement& aStatement, int aIndex )
        {
            const unsigned char* text = sqlite3_column_text ( aStatement.Get(), aIndex );
            return ( text != nullptr ) ? reinterpret_cast<const char*> ( text ) : std::string{};
        }

        /** Synty stores each channel as an RRGGBB string; the engine keeps the
            packed integer so a palette row is a single comparison. */
        int64_t ParseColor ( const std::string& aText )
        {
            if ( aText.size() != 6 )
            {
                return 0;
            }
            char* end = nullptr;
            const unsigned long value = std::strtoul ( aText.c_str(), &end, 16 );
            return ( end != nullptr && *end == '\0' ) ? static_cast<int64_t> ( value ) : 0;
        }
    }

    SidekickDatabase::SidekickDatabase() = default;
    SidekickDatabase::~SidekickDatabase() = default;

    bool SidekickDatabase::ProcessArgs ( int argc, char** argv )
    {
        if ( argc < 2 || ( strcmp ( argv[1], "sidekickdb" ) != 0 ) )
        {
            std::ostringstream stream;
            stream << "Invalid tool name, expected sidekickdb, got "
                   << ( ( argc < 2 ) ? "nothing" : argv[1] ) << std::endl;
            throw std::runtime_error ( stream.str() );
        }
        auto value = [argc, argv] ( int& aIndex, const char* aOption ) -> const char*
        {
            if ( ++aIndex >= argc )
            {
                std::ostringstream stream;
                stream << "Missing value for " << aOption << ".";
                throw std::runtime_error ( stream.str() );
            }
            return argv[aIndex];
        };
        for ( int i = 2; i < argc; ++i )
        {
            const std::string option{argv[i]};
            if ( option == "-i" || option == "--in" )
            {
                mInputFile = value ( i, "--in" );
            }
            else if ( option == "-o" || option == "--out" )
            {
                mOutputFile = value ( i, "--out" );
            }
            else if ( option == "-m" || option == "--meshes" )
            {
                mMeshPath = value ( i, "--meshes" );
            }
            else if ( option == "-p" || option == "--prefix" )
            {
                mResourcePath = value ( i, "--prefix" );
            }
            else if ( option == "--mesh-extension" )
            {
                mMeshExtension = value ( i, "--mesh-extension" );
            }
            else if ( option == "--skeleton" )
            {
                mSkeleton = value ( i, "--skeleton" );
            }
            else if ( option == "--pipeline" )
            {
                mPipeline = value ( i, "--pipeline" );
            }
            else if ( option == "--material" )
            {
                mMaterial = value ( i, "--material" );
            }
            else if ( option == "-a" || option == "--animation" )
            {
                const std::string assignment{value ( i, "--animation" ) };
                const size_t separator = assignment.find ( '=' );
                if ( separator == std::string::npos )
                {
                    throw std::runtime_error ( "Expected --animation <name>=<path>, got " + assignment + "." );
                }
                mAnimations.emplace_back ( assignment.substr ( 0, separator ), assignment.substr ( separator + 1 ) );
            }
            else if ( option == "-h" || option == "--help" )
            {
                std::cout << "Usage: aeontool sidekickdb [options]\n"
                          << "  -i, --in <file.db>        Synty_Sidekick.db to read (required)\n"
                          << "  -o, --out <file.db>       Character library to write, replaced if present (required)\n"
                          << "  -m, --meshes <dir>        Directory of cooked part meshes; parts without one are\n"
                          << "                            dropped, along with any outfit that needed them\n"
                          << "  -p, --prefix <path>       Resource path the mesh references use\n"
                          << "                            (default: sidekick/parts)\n"
                          << "      --mesh-extension <e>  Extension of the cooked meshes (default: msh)\n"
                          << "      --skeleton <path>     Skeleton every part is rigged to\n"
                          << "                            (default: sidekick/parts/skeleton.skl)\n"
                          << "      --pipeline <path>     Pipeline characters are drawn with\n"
                          << "                            (default: shaders/clustered_phong)\n"
                          << "      --material <path>     Material the composed palette is applied to\n"
                          << "                            (default: sidekick/parts/character.mtl)\n"
                          << "  -a, --animation <n>=<p>   Animation available to every character, repeatable\n"
                          << "  -h, --help                Show this help" << std::endl;
                return true;
            }
            else
            {
                std::ostringstream stream;
                stream << "Unknown option " << option << ".";
                throw std::runtime_error ( stream.str() );
            }
        }
        if ( mInputFile.empty() )
        {
            throw std::runtime_error ( "No input database provided, use -i <Synty_Sidekick.db>." );
        }
        if ( mOutputFile.empty() )
        {
            throw std::runtime_error ( "No output database provided, use -o <file.db>." );
        }
        return false;
    }

    int SidekickDatabase::operator() ( int argc, char** argv )
    {
        if ( ProcessArgs ( argc, argv ) )
        {
            return 0;
        }

        const Database source{mInputFile, SQLITE_OPEN_READONLY};
        std::filesystem::remove ( mOutputFile );
        const std::filesystem::path parent{std::filesystem::path ( mOutputFile ).parent_path() };
        if ( !parent.empty() )
        {
            std::filesystem::create_directories ( parent );
        }
        const Database destination{mOutputFile, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE};
        destination.Execute ( kSchema );
        destination.Execute ( "BEGIN" );

        {
            Statement insert{destination, "INSERT INTO character_part_type ( id, code, name ) VALUES ( ?, ?, ? )"};
            for ( size_t i = 0; i < kPartTypes.size(); ++i )
            {
                insert.Bind ( 1, static_cast<int64_t> ( i + 1 ) );
                insert.Bind ( 2, std::string{kPartTypes[i].mCode} );
                insert.Bind ( 3, std::string{kPartTypes[i].mName} );
                insert.Run();
            }
        }

        {
            Statement read{source, "SELECT id, name FROM sk_species"};
            Statement insert{destination, "INSERT INTO character_species ( id, name ) VALUES ( ?, ? )"};
            while ( read.Step() )
            {
                insert.Bind ( 1, sqlite3_column_int64 ( read.Get(), 0 ) );
                insert.Bind ( 2, Column ( read, 1 ) );
                insert.Run();
            }
        }

        // Parts, filtered by what has actually been cooked.
        std::unordered_set<int64_t> parts;
        std::unordered_map<std::string, int64_t> parts_by_name;
        size_t missing_meshes = 0;
        {
            Statement read{source, "SELECT id, type, name FROM sk_part WHERE type BETWEEN 1 AND 38"};
            Statement insert{destination, "INSERT INTO character_part ( id, part_type_id, name, mesh ) VALUES ( ?, ?, ?, ? )"};
            while ( read.Step() )
            {
                const int64_t id = sqlite3_column_int64 ( read.Get(), 0 );
                const std::string name = Column ( read, 2 );
                if ( name.empty() )
                {
                    continue;
                }
                const std::string file = name + "." + mMeshExtension;
                if ( !mMeshPath.empty() && !std::filesystem::exists ( std::filesystem::path ( mMeshPath ) / file ) )
                {
                    ++missing_meshes;
                    continue;
                }
                insert.Bind ( 1, id );
                insert.Bind ( 2, sqlite3_column_int64 ( read.Get(), 1 ) );
                insert.Bind ( 3, name );
                insert.Bind ( 4, mResourcePath + "/" + file );
                insert.Run();
                parts.insert ( id );
                parts_by_name.emplace ( name, id );
            }
        }

        {
            Statement read{source, "SELECT ptr_part, ptr_species FROM sk_part_species_link"};
            Statement insert{destination, "INSERT OR IGNORE INTO character_part_species ( part_id, species_id ) VALUES ( ?, ? )"};
            while ( read.Step() )
            {
                const int64_t part = sqlite3_column_int64 ( read.Get(), 0 );
                if ( parts.find ( part ) == parts.end() )
                {
                    continue;
                }
                insert.Bind ( 1, part );
                insert.Bind ( 2, sqlite3_column_int64 ( read.Get(), 1 ) );
                insert.Run();
            }
        }

        {
            Statement read{source, "SELECT id, filter_term FROM sk_preset_filter"};
            Statement insert{destination, "INSERT INTO character_faction ( id, name ) VALUES ( ?, ? )"};
            while ( read.Step() )
            {
                std::string name = Column ( read, 1 );
                while ( !name.empty() && name.back() == ' ' )
                {
                    name.pop_back();
                }
                insert.Bind ( 1, sqlite3_column_int64 ( read.Get(), 0 ) );
                insert.Bind ( 2, name );
                insert.Run();
            }
        }

        // An outfit is only usable when every part it names survived.
        std::unordered_map<int64_t, int64_t> outfit_faction;
        {
            Statement read{source, "SELECT ptr_preset, ptr_filter FROM sk_preset_filter_row"};
            while ( read.Step() )
            {
                outfit_faction.emplace ( sqlite3_column_int64 ( read.Get(), 0 ),
                                         sqlite3_column_int64 ( read.Get(), 1 ) );
            }
        }
        std::unordered_map<int64_t, std::vector<int64_t>> outfit_parts;
        std::unordered_set<int64_t> incomplete;
        {
            // Outfits name their parts rather than pointing at them: ptr_part is
            // unset on more than half the rows and stale on most of the rest.
            Statement read{source, "SELECT ptr_part_preset, part_name, ptr_part FROM sk_part_preset_row"};
            while ( read.Step() )
            {
                const int64_t outfit = sqlite3_column_int64 ( read.Get(), 0 );
                const std::string name = Column ( read, 1 );
                if ( name.empty() )
                {
                    // A nameless row is a slot the outfit deliberately leaves
                    // empty, such as no facial hair, not a part gone missing.
                    continue;
                }
                const auto named = parts_by_name.find ( name );
                if ( named != parts_by_name.end() )
                {
                    outfit_parts[outfit].push_back ( named->second );
                    continue;
                }
                const int64_t part = sqlite3_column_int64 ( read.Get(), 2 );
                if ( parts.find ( part ) != parts.end() )
                {
                    outfit_parts[outfit].push_back ( part );
                    continue;
                }
                incomplete.insert ( outfit );
            }
        }

        size_t outfits = 0;
        {
            Statement read{source, "SELECT id, ptr_species, name FROM sk_part_preset"};
            Statement insert{destination, "INSERT INTO character_outfit ( id, faction_id, species_id, name ) VALUES ( ?, ?, ?, ? )"};
            Statement link{destination, "INSERT OR IGNORE INTO character_outfit_part ( outfit_id, part_id ) VALUES ( ?, ? )"};
            while ( read.Step() )
            {
                const int64_t id = sqlite3_column_int64 ( read.Get(), 0 );
                const auto members = outfit_parts.find ( id );
                if ( incomplete.find ( id ) != incomplete.end() || members == outfit_parts.end() )
                {
                    continue;
                }
                insert.Bind ( 1, id );
                const auto faction = outfit_faction.find ( id );
                if ( faction != outfit_faction.end() )
                {
                    insert.Bind ( 2, faction->second );
                }
                else
                {
                    insert.BindNull ( 2 );
                }
                insert.Bind ( 3, sqlite3_column_int64 ( read.Get(), 1 ) );
                insert.Bind ( 4, Column ( read, 2 ) );
                insert.Run();
                for ( int64_t part : members->second )
                {
                    link.Bind ( 1, id );
                    link.Bind ( 2, part );
                    link.Run();
                }
                ++outfits;
            }
        }

        {
            Statement read{source, "SELECT id, name, u, v FROM sk_color_property"};
            Statement insert{destination, "INSERT INTO character_color_slot ( id, name, u, v ) VALUES ( ?, ?, ?, ? )"};
            while ( read.Step() )
            {
                insert.Bind ( 1, sqlite3_column_int64 ( read.Get(), 0 ) );
                insert.Bind ( 2, Column ( read, 1 ) );
                insert.Bind ( 3, sqlite3_column_int64 ( read.Get(), 2 ) );
                insert.Bind ( 4, sqlite3_column_int64 ( read.Get(), 3 ) );
                insert.Run();
            }
        }

        size_t palettes = 0;
        {
            Statement read{source, "SELECT id, ptr_species, name FROM sk_color_preset"};
            Statement insert{destination, "INSERT INTO character_palette ( id, species_id, name ) VALUES ( ?, ?, ? )"};
            while ( read.Step() )
            {
                insert.Bind ( 1, sqlite3_column_int64 ( read.Get(), 0 ) );
                insert.Bind ( 2, sqlite3_column_int64 ( read.Get(), 1 ) );
                insert.Bind ( 3, Column ( read, 2 ) );
                insert.Run();
                ++palettes;
            }
        }

        {
            Statement read{source, "SELECT ptr_color_preset, ptr_color_property, color, metallic, smoothness,"
                                   " reflection, emission, opacity FROM sk_color_preset_row"};
            Statement insert{destination, "INSERT OR IGNORE INTO character_palette_entry ( palette_id, slot_id,"
                                          " color, metallic, smoothness, reflection, emission, opacity )"
                                          " VALUES ( ?, ?, ?, ?, ?, ?, ?, ? )"};
            while ( read.Step() )
            {
                insert.Bind ( 1, sqlite3_column_int64 ( read.Get(), 0 ) );
                insert.Bind ( 2, sqlite3_column_int64 ( read.Get(), 1 ) );
                for ( int channel = 0; channel < 6; ++channel )
                {
                    insert.Bind ( 3 + channel, ParseColor ( Column ( read, 2 + channel ) ) );
                }
                insert.Run();
            }
        }

        {
            Statement insert{destination, "INSERT INTO character_library ( id, skeleton, pipeline, material )"
                                          " VALUES ( 1, ?, ?, ? )"};
            insert.Bind ( 1, mSkeleton );
            insert.Bind ( 2, mPipeline );
            insert.Bind ( 3, mMaterial );
            insert.Run();
        }

        {
            Statement insert{destination, "INSERT INTO character_animation ( name, path ) VALUES ( ?, ? )"};
            for ( const auto& animation : mAnimations )
            {
                insert.Bind ( 1, animation.first );
                insert.Bind ( 2, animation.second );
                insert.Run();
            }
        }

        destination.Execute ( "COMMIT" );

        std::cout << std::dec << "Wrote " << parts.size() << " parts, " << outfits << " outfits and "
                  << palettes << " palettes to " << mOutputFile << std::endl;
        if ( missing_meshes != 0 )
        {
            std::cout << std::dec << missing_meshes << " parts skipped, no cooked mesh under " << mMeshPath << std::endl;
        }
        return 0;
    }
}
