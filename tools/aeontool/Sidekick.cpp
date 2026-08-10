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

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : PROTOBUF_WARNINGS )
#endif
#include "aeongames/ProtoBufClasses.hpp"
#include <google/protobuf/text_format.h>
#include "model.pb.h"
#include "material.pb.h"
#ifdef _MSC_VER
#pragma warning( pop )
#endif

#include <png.h>
#include <sqlite3.h>

#include <array>
#include <cctype>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include "Sidekick.h"

namespace AeonGames
{
    namespace
    {
        /** Palettes are square and sized so the widest colour property block
            (u,v up to 15) still lands inside the texture. */
        constexpr uint32_t kPaletteSize = 32;
        /** Synty marks texels that no colour property owns with magenta. */
        constexpr std::array<uint8_t, 4> kUnusedTexel{255, 0, 255, 255};
        /** Synty's own material ships _Metallic 0 and _Glossiness 0.5, so Sidekick
            characters are flat-shaded non-metals. The engine's defaults are
            metallic 1 / roughness 1, which would discard the base colour, so the
            factors are always written out explicitly. */
        constexpr float kMetallicFactor = 0.0f;
        constexpr float kRoughnessFactor = 0.5f;

        /** @brief One Sidekick colour channel and the AeonEngine slot it feeds. */
        struct ChannelDesc
        {
            const char* mKey;      ///< Key inside a .sk ColorRows entry.
            const char* mSuffix;   ///< Emitted texture name suffix.
            const char* mBaseName; ///< Synty stock texture used to seed unwritten texels.
            const char* mSampler;  ///< Canonical sampler slot, nullptr when the engine has none.
            bool mInvert;          ///< Synty stores smoothness; the engine wants roughness.
        };

        /* Only the colour palette drives a sampler. The metallic, smoothness,
           reflection, emission and opacity channels store 0xFF0000 for every
           colour property the character leaves unset, which is a sentinel rather
           than a value: fed to a metallic-roughness shader it reads as metallic
           1.0 and roughness 0.045, i.e. a black mirror. They are still baked so
           the data is on disk once a shader knows how to interpret it. */
        constexpr std::array<ChannelDesc, 6> kChannels
        {
            {
                { "MainColor",  "ColorMap",      "T_ColorMap.png",      "DiffuseMap", false },
                { "Metallic",   "MetallicMap",   "T_MetallicMap.png",   nullptr,      false },
                { "Smoothness", "RoughnessMap",  "T_SmoothnessMap.png", nullptr,      true  },
                { "Reflection", "ReflectionMap", "T_ReflectionMap.png", nullptr,      false },
                { "Emission",   "EmissiveMap",   "T_EmissionMap.png",   nullptr,      false },
                { "Opacity",    "OpacityMap",    "T_OpacityMap.png",    nullptr,      false },
            }
        };

        struct Part
        {
            std::string mName;
            std::string mPartType;
        };

        struct ColorRow
        {
            uint32_t mProperty{};
            std::array<std::string, kChannels.size() > mValues{};
        };

        struct Recipe
        {
            std::string mName;
            std::vector<Part> mParts;
            std::vector<ColorRow> mColorRows;
        };

        /** @brief RGBA8 image, rows stored top-down. */
        struct Image
        {
            uint32_t mWidth{};
            uint32_t mHeight{};
            std::vector<uint8_t> mPixels;
        };

        std::string Trim ( const std::string& aText )
        {
            size_t begin = aText.find_first_not_of ( " \t\r\n" );
            if ( begin == std::string::npos )
            {
                return {};
            }
            size_t end = aText.find_last_not_of ( " \t\r\n" );
            return aText.substr ( begin, ( end - begin ) + 1 );
        }

        bool SplitKeyValue ( const std::string& aText, std::string& aKey, std::string& aValue )
        {
            size_t colon = aText.find ( ':' );
            if ( colon == std::string::npos )
            {
                return false;
            }
            aKey = Trim ( aText.substr ( 0, colon ) );
            aValue = Trim ( aText.substr ( colon + 1 ) );
            return !aKey.empty();
        }

        /** @brief Parse the line-oriented subset of YAML that Sidekick writes.

            Only the shape Synty emits is supported: top level scalars, the
            @c Parts and @c ColorRows sequences and the @c ColorSet mapping. */
        Recipe ParseRecipe ( const std::string& aFilePath )
        {
            std::ifstream file ( aFilePath );
            if ( !file )
            {
                std::ostringstream stream;
                stream << "Unable to open Sidekick recipe " << aFilePath;
                throw std::runtime_error ( stream.str() );
            }
            enum class Section
            {
                None,
                Parts,
                ColorRows,
                Ignored
            };
            Recipe recipe{};
            Section section{Section::None};
            std::string line{};
            while ( std::getline ( file, line ) )
            {
                std::string content = Trim ( line );
                if ( content.empty() )
                {
                    continue;
                }
                const size_t indent = line.find_first_not_of ( " \t" );
                std::string key{};
                std::string value{};
                if ( indent == 0 && content.back() == ':' && content.find ( "- " ) != 0 )
                {
                    std::string name = content.substr ( 0, content.size() - 1 );
                    section = ( name == "Parts" ) ? Section::Parts :
                              ( name == "ColorRows" ) ? Section::ColorRows :
                              Section::Ignored;
                    continue;
                }
                if ( indent == 0 && content.compare ( 0, 2, "- " ) != 0 )
                {
                    section = Section::None;
                    if ( SplitKeyValue ( content, key, value ) && key == "Name" )
                    {
                        recipe.mName = value;
                    }
                    continue;
                }
                const bool starts_item = ( content.compare ( 0, 2, "- " ) == 0 );
                if ( starts_item )
                {
                    content = Trim ( content.substr ( 2 ) );
                    if ( section == Section::Parts )
                    {
                        recipe.mParts.emplace_back();
                    }
                    else if ( section == Section::ColorRows )
                    {
                        recipe.mColorRows.emplace_back();
                    }
                }
                if ( !SplitKeyValue ( content, key, value ) )
                {
                    continue;
                }
                if ( section == Section::Parts && !recipe.mParts.empty() )
                {
                    if ( key == "Name" )
                    {
                        recipe.mParts.back().mName = value;
                    }
                    else if ( key == "PartType" )
                    {
                        recipe.mParts.back().mPartType = value;
                    }
                }
                else if ( section == Section::ColorRows && !recipe.mColorRows.empty() )
                {
                    if ( key == "ColorProperty" )
                    {
                        recipe.mColorRows.back().mProperty =
                                             static_cast<uint32_t> ( std::strtoul ( value.c_str(), nullptr, 10 ) );
                        continue;
                    }
                    for ( size_t i = 0; i < kChannels.size(); ++i )
                    {
                        if ( key == kChannels[i].mKey )
                        {
                            recipe.mColorRows.back().mValues[i] = value;
                            break;
                        }
                    }
                }
            }
            if ( recipe.mName.empty() )
            {
                recipe.mName = std::filesystem::path ( aFilePath ).stem().string();
            }
            return recipe;
        }

        bool ParseHexColor ( const std::string& aText, std::array<uint8_t, 3>& aColor )
        {
            if ( aText.size() != 6 )
            {
                return false;
            }
            for ( size_t i = 0; i < 3; ++i )
            {
                char* end = nullptr;
                const std::string component = aText.substr ( i * 2, 2 );
                const unsigned long parsed = std::strtoul ( component.c_str(), &end, 16 );
                if ( end == nullptr || *end != '\0' )
                {
                    return false;
                }
                aColor[i] = static_cast<uint8_t> ( parsed );
            }
            return true;
        }

        struct MemoryReader
        {
            const uint8_t* mData;
            size_t mSize;
            size_t mOffset;
        };

        void ReadFromMemory ( png_structp aPng, png_bytep aTarget, png_size_t aLength )
        {
            MemoryReader* reader = reinterpret_cast<MemoryReader*> ( png_get_io_ptr ( aPng ) );
            if ( reader->mOffset + aLength > reader->mSize )
            {
                png_error ( aPng, "Attempted to read past the end of the PNG buffer." );
                return;
            }
            std::memcpy ( aTarget, reader->mData + reader->mOffset, aLength );
            reader->mOffset += aLength;
        }

        /** @brief Decode a PNG into RGBA8.

            The whole file is read up front so that the only resources live
            across libpng's longjmp are raw allocations this function owns. */
        Image ReadPng ( const std::string& aFilePath )
        {
            std::ifstream file ( aFilePath, std::ios::in | std::ios::binary );
            if ( !file )
            {
                std::ostringstream stream;
                stream << "Unable to open texture " << aFilePath;
                throw std::runtime_error ( stream.str() );
            }
            std::vector<uint8_t> encoded ( ( std::istreambuf_iterator<char> ( file ) ),
                                           std::istreambuf_iterator<char>() );
            if ( encoded.size() < 8 || png_sig_cmp ( encoded.data(), 0, 8 ) != 0 )
            {
                std::ostringstream stream;
                stream << aFilePath << " is not a PNG file.";
                throw std::runtime_error ( stream.str() );
            }
            MemoryReader reader{encoded.data(), encoded.size(), 0};
            png_structp png = png_create_read_struct ( PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr );
            if ( png == nullptr )
            {
                throw std::runtime_error ( "Unable to create a PNG read struct." );
            }
            png_infop info = png_create_info_struct ( png );
            if ( info == nullptr )
            {
                png_destroy_read_struct ( &png, nullptr, nullptr );
                throw std::runtime_error ( "Unable to create a PNG info struct." );
            }
            png_bytep* rows = nullptr;
            png_bytep pixels = nullptr;
            if ( setjmp ( png_jmpbuf ( png ) ) )
            {
                std::free ( rows );
                std::free ( pixels );
                png_destroy_read_struct ( &png, &info, nullptr );
                std::ostringstream stream;
                stream << "libpng failed to decode " << aFilePath;
                throw std::runtime_error ( stream.str() );
            }
            png_set_read_fn ( png, &reader, ReadFromMemory );
            png_read_info ( png, info );
            const png_uint_32 width = png_get_image_width ( png, info );
            const png_uint_32 height = png_get_image_height ( png, info );
            const png_byte color_type = png_get_color_type ( png, info );
            const png_byte bit_depth = png_get_bit_depth ( png, info );
            if ( bit_depth == 16 )
            {
                png_set_strip_16 ( png );
            }
            if ( color_type == PNG_COLOR_TYPE_PALETTE )
            {
                png_set_palette_to_rgb ( png );
            }
            if ( color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8 )
            {
                png_set_expand_gray_1_2_4_to_8 ( png );
            }
            if ( color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA )
            {
                png_set_gray_to_rgb ( png );
            }
            if ( png_get_valid ( png, info, PNG_INFO_tRNS ) != 0 )
            {
                png_set_tRNS_to_alpha ( png );
            }
            png_set_filler ( png, 0xFF, PNG_FILLER_AFTER );
            png_read_update_info ( png, info );
            const size_t stride = static_cast<size_t> ( width ) * 4;
            pixels = static_cast<png_bytep> ( std::malloc ( stride * height ) );
            rows = static_cast<png_bytep*> ( std::malloc ( sizeof ( png_bytep ) * height ) );
            if ( pixels == nullptr || rows == nullptr )
            {
                png_error ( png, "Out of memory decoding PNG." );
            }
            for ( png_uint_32 y = 0; y < height; ++y )
            {
                rows[y] = pixels + ( static_cast<size_t> ( y ) * stride );
            }
            png_read_image ( png, rows );
            Image image{};
            image.mWidth = width;
            image.mHeight = height;
            image.mPixels.assign ( pixels, pixels + ( stride * height ) );
            std::free ( rows );
            std::free ( pixels );
            png_destroy_read_struct ( &png, &info, nullptr );
            return image;
        }

        void WritePng ( const Image& aImage, const std::string& aFilePath )
        {
            std::filesystem::create_directories ( std::filesystem::path ( aFilePath ).parent_path() );
            FILE* file = std::fopen ( aFilePath.c_str(), "wb" );
            if ( file == nullptr )
            {
                std::ostringstream stream;
                stream << "Unable to open " << aFilePath << " for writing.";
                throw std::runtime_error ( stream.str() );
            }
            png_structp png = png_create_write_struct ( PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr );
            if ( png == nullptr )
            {
                std::fclose ( file );
                throw std::runtime_error ( "Unable to create a PNG write struct." );
            }
            png_infop info = png_create_info_struct ( png );
            if ( info == nullptr )
            {
                png_destroy_write_struct ( &png, nullptr );
                std::fclose ( file );
                throw std::runtime_error ( "Unable to create a PNG info struct." );
            }
            png_bytep* rows = nullptr;
            if ( setjmp ( png_jmpbuf ( png ) ) )
            {
                std::free ( rows );
                png_destroy_write_struct ( &png, &info );
                std::fclose ( file );
                std::ostringstream stream;
                stream << "libpng failed to encode " << aFilePath;
                throw std::runtime_error ( stream.str() );
            }
            png_init_io ( png, file );
            png_set_IHDR ( png, info, aImage.mWidth, aImage.mHeight, 8, PNG_COLOR_TYPE_RGBA,
                           PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT );
            png_write_info ( png, info );
            rows = static_cast<png_bytep*> ( std::malloc ( sizeof ( png_bytep ) * aImage.mHeight ) );
            if ( rows == nullptr )
            {
                png_error ( png, "Out of memory encoding PNG." );
            }
            const size_t stride = static_cast<size_t> ( aImage.mWidth ) * 4;
            for ( uint32_t y = 0; y < aImage.mHeight; ++y )
            {
                rows[y] = const_cast<png_bytep> ( aImage.mPixels.data() + ( static_cast<size_t> ( y ) * stride ) );
            }
            png_write_image ( png, rows );
            png_write_end ( png, nullptr );
            std::free ( rows );
            png_destroy_write_struct ( &png, &info );
            std::fclose ( file );
        }

        /** @brief Paint the 2x2 texel block a colour property owns.

            Sidekick palettes use a bottom-left origin, so the requested row is
            mirrored into the top-down storage this tool keeps. */
        void SetPropertyBlock ( Image& aImage, uint32_t aU, uint32_t aV, const std::array<uint8_t, 3>& aColor )
        {
            const uint32_t x = aU * 2;
            const uint32_t y = aV * 2;
            if ( ( x + 1 ) >= aImage.mWidth || ( y + 1 ) >= aImage.mHeight )
            {
                std::ostringstream stream;
                stream << "Colour property at (" << aU << "," << aV << ") falls outside a "
                       << aImage.mWidth << "x" << aImage.mHeight << " palette.";
                throw std::runtime_error ( stream.str() );
            }
            for ( uint32_t dy = 0; dy < 2; ++dy )
            {
                const uint32_t row = aImage.mHeight - 1 - ( y + dy );
                for ( uint32_t dx = 0; dx < 2; ++dx )
                {
                    uint8_t* texel = aImage.mPixels.data() + ( ( ( static_cast<size_t> ( row ) * aImage.mWidth ) + x + dx ) * 4 );
                    texel[0] = aColor[0];
                    texel[1] = aColor[1];
                    texel[2] = aColor[2];
                }
            }
        }

        std::unordered_map<uint32_t, std::pair<uint32_t, uint32_t>> LoadColorProperties ( const std::string& aDatabaseFile )
        {
            sqlite3* database = nullptr;
            if ( sqlite3_open_v2 ( aDatabaseFile.c_str(), &database, SQLITE_OPEN_READONLY, nullptr ) != SQLITE_OK )
            {
                std::ostringstream stream;
                stream << "Unable to open Sidekick database " << aDatabaseFile << ": "
                       << ( ( database != nullptr ) ? sqlite3_errmsg ( database ) : "out of memory" );
                sqlite3_close ( database );
                throw std::runtime_error ( stream.str() );
            }
            sqlite3_stmt* statement = nullptr;
            if ( sqlite3_prepare_v2 ( database, "SELECT id, u, v FROM sk_color_property", -1, &statement, nullptr ) != SQLITE_OK )
            {
                std::ostringstream stream;
                stream << "Unable to query sk_color_property: " << sqlite3_errmsg ( database );
                sqlite3_close ( database );
                throw std::runtime_error ( stream.str() );
            }
            std::unordered_map<uint32_t, std::pair<uint32_t, uint32_t>> properties{};
            int result = SQLITE_OK;
            while ( ( result = sqlite3_step ( statement ) ) == SQLITE_ROW )
            {
                properties.emplace ( static_cast<uint32_t> ( sqlite3_column_int ( statement, 0 ) ),
                                     std::make_pair ( static_cast<uint32_t> ( sqlite3_column_int ( statement, 1 ) ),
                                                      static_cast<uint32_t> ( sqlite3_column_int ( statement, 2 ) ) ) );
            }
            sqlite3_finalize ( statement );
            const bool failed = ( result != SQLITE_DONE );
            std::string message = failed ? sqlite3_errmsg ( database ) : std::string{};
            sqlite3_close ( database );
            if ( failed )
            {
                std::ostringstream stream;
                stream << "Error while reading sk_color_property: " << message;
                throw std::runtime_error ( stream.str() );
            }
            return properties;
        }

        void WriteMessage ( const google::protobuf::Message& aMessage, const std::string& aFilePath,
                            const char* aMagic, bool aBinary )
        {
            std::filesystem::create_directories ( std::filesystem::path ( aFilePath ).parent_path() );
            if ( aBinary )
            {
                std::ofstream file ( aFilePath, std::ios::out | std::ios::binary );
                if ( !file )
                {
                    std::ostringstream stream;
                    stream << "Unable to open " << aFilePath << " for writing.";
                    throw std::runtime_error ( stream.str() );
                }
                file << aMagic << '\0';
                if ( !aMessage.SerializeToOstream ( &file ) )
                {
                    throw std::runtime_error ( "Failed to serialize message to binary format." );
                }
                return;
            }
            std::string text{};
            if ( !google::protobuf::TextFormat::PrintToString ( aMessage, &text ) )
            {
                throw std::runtime_error ( "Failed to serialize message to text format." );
            }
            std::ofstream file ( aFilePath, std::ios::out );
            if ( !file )
            {
                std::ostringstream stream;
                stream << "Unable to open " << aFilePath << " for writing.";
                throw std::runtime_error ( stream.str() );
            }
            file << aMagic << std::endl;
            file.write ( text.c_str(), text.length() );
        }
    }

    Sidekick::Sidekick() = default;
    Sidekick::~Sidekick() = default;

    bool Sidekick::ProcessArgs ( int argc, char** argv )
    {
        if ( argc < 2 || ( strcmp ( argv[1], "sidekick" ) != 0 ) )
        {
            std::ostringstream stream;
            stream << "Invalid tool name, expected sidekick, got "
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
            else if ( option == "-d" || option == "--database" )
            {
                mDatabaseFile = value ( i, "--database" );
            }
            else if ( option == "-o" || option == "--out" )
            {
                mOutputPath = value ( i, "--out" );
            }
            else if ( option == "-p" || option == "--prefix" )
            {
                mResourcePath = value ( i, "--prefix" );
            }
            else if ( option == "-t" || option == "--textures" )
            {
                mBaseTexturePath = value ( i, "--textures" );
            }
            else if ( option == "-n" || option == "--name" )
            {
                mName = value ( i, "--name" );
            }
            else if ( option == "--pipeline" )
            {
                mPipeline = value ( i, "--pipeline" );
            }
            else if ( option == "--skeleton" )
            {
                mSkeletonPath = value ( i, "--skeleton" );
            }
            else if ( option == "--mesh-extension" )
            {
                mMeshExtension = value ( i, "--mesh-extension" );
            }
            else if ( option == "-b" || option == "--binary" )
            {
                mBinary = true;
            }
            else if ( option == "-h" || option == "--help" )
            {
                std::cout << "Usage: aeontool sidekick [options]\n"
                          << "  -i, --in <file.sk>        Synty SIDEKICK character recipe (required)\n"
                          << "  -d, --database <file>     Synty_Sidekick.db, source of the palette (u,v) map (required)\n"
                          << "  -o, --out <dir>           Directory the assets are written to (default: .)\n"
                          << "  -p, --prefix <path>       Resource path the emitted assets reference each other by\n"
                          << "                            (default: the character name)\n"
                          << "  -t, --textures <dir>      Directory holding the Synty stock maps (T_ColorMap.png ...)\n"
                          << "                            used to seed texels no colour property owns\n"
                          << "  -n, --name <name>         Asset base name (default: the recipe's Name field)\n"
                          << "      --pipeline <path>     default_pipeline reference (default: shaders/clustered_phong)\n"
                          << "      --skeleton <path>     Skeleton reference (default: <prefix>/skeletons/skeleton.skl)\n"
                          << "      --mesh-extension <e>  Extension of the mesh assets to reference (default: msh)\n"
                          << "  -b, --binary              Write binary assets instead of text\n"
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
            throw std::runtime_error ( "No input recipe provided, use -i <file.sk>." );
        }
        if ( mDatabaseFile.empty() )
        {
            throw std::runtime_error ( "No Sidekick database provided, use -d <Synty_Sidekick.db>." );
        }
        return false;
    }

    int Sidekick::operator() ( int argc, char** argv )
    {
        if ( ProcessArgs ( argc, argv ) )
        {
            return 0;
        }

        const Recipe recipe = ParseRecipe ( mInputFile );
        const auto properties = LoadColorProperties ( mDatabaseFile );
        if ( mName.empty() )
        {
            mName = recipe.mName;
        }
        if ( mResourcePath.empty() )
        {
            mResourcePath = mName;
        }
        while ( !mResourcePath.empty() && mResourcePath.back() == '/' )
        {
            mResourcePath.pop_back();
        }
        if ( mSkeletonPath.empty() )
        {
            mSkeletonPath = mResourcePath + "/skeletons/skeleton.skl";
        }
        const std::string model_extension = mBinary ? ".mdl" : ".txt";
        const std::string material_extension = mBinary ? ".mtl" : ".txt";
        const std::filesystem::path output{mOutputPath};

        // Bake one palette per Sidekick colour channel.
        std::array<std::string, kChannels.size() > texture_paths{};
        for ( size_t channel = 0; channel < kChannels.size(); ++channel )
        {
            Image image{};
            if ( !mBaseTexturePath.empty() )
            {
                image = ReadPng ( ( std::filesystem::path ( mBaseTexturePath ) / kChannels[channel].mBaseName ).string() );
            }
            else
            {
                image.mWidth = kPaletteSize;
                image.mHeight = kPaletteSize;
                image.mPixels.resize ( static_cast<size_t> ( kPaletteSize ) * kPaletteSize * 4 );
                for ( size_t texel = 0; texel < image.mPixels.size(); texel += 4 )
                {
                    std::memcpy ( image.mPixels.data() + texel, kUnusedTexel.data(), kUnusedTexel.size() );
                }
            }
            size_t painted = 0;
            for ( const ColorRow& row : recipe.mColorRows )
            {
                const auto property = properties.find ( row.mProperty );
                if ( property == properties.end() )
                {
                    std::ostringstream stream;
                    stream << "Colour property " << row.mProperty << " is not present in " << mDatabaseFile << ".";
                    throw std::runtime_error ( stream.str() );
                }
                std::array<uint8_t, 3> color{};
                if ( !ParseHexColor ( row.mValues[channel], color ) )
                {
                    continue;
                }
                if ( kChannels[channel].mInvert )
                {
                    color[0] = static_cast<uint8_t> ( 255 - color[0] );
                    color[1] = static_cast<uint8_t> ( 255 - color[1] );
                    color[2] = static_cast<uint8_t> ( 255 - color[2] );
                }
                SetPropertyBlock ( image, property->second.first, property->second.second, color );
                ++painted;
            }
            std::ostringstream name;
            name << "T_" << mName << kChannels[channel].mSuffix << ".png";
            texture_paths[channel] = mResourcePath + "/textures/" + name.str();
            WritePng ( image, ( output / "textures" / name.str() ).string() );
            std::cout << "Baked " << painted << " colour properties into " << texture_paths[channel] << std::endl;
        }

        MaterialMsg material{};
        {
            PropertyMsg* base_color = material.add_property();
            base_color->set_name ( "BaseColorFactor" );
            base_color->mutable_vector4()->set_x ( 1.0f );
            base_color->mutable_vector4()->set_y ( 1.0f );
            base_color->mutable_vector4()->set_z ( 1.0f );
            base_color->mutable_vector4()->set_w ( 1.0f );
            PropertyMsg* metallic = material.add_property();
            metallic->set_name ( "MetallicFactor" );
            metallic->set_scalar_float ( kMetallicFactor );
            PropertyMsg* roughness = material.add_property();
            roughness->set_name ( "RoughnessFactor" );
            roughness->set_scalar_float ( kRoughnessFactor );
            PropertyMsg* emissive = material.add_property();
            emissive->set_name ( "EmissiveFactor" );
            emissive->mutable_vector3()->set_x ( 0.0f );
            emissive->mutable_vector3()->set_y ( 0.0f );
            emissive->mutable_vector3()->set_z ( 0.0f );
        }
        for ( size_t channel = 0; channel < kChannels.size(); ++channel )
        {
            if ( kChannels[channel].mSampler == nullptr )
            {
                continue;
            }
            SamplerMsg* sampler = material.add_sampler();
            sampler->set_name ( kChannels[channel].mSampler );
            sampler->mutable_image()->set_path ( texture_paths[channel] );
            // Palettes are flat colour blocks; filtering them bleeds neighbours.
            sampler->set_min_filter ( SamplerMsg::FILTER_NEAREST );
            sampler->set_mag_filter ( SamplerMsg::FILTER_NEAREST );
            sampler->set_mipmap_mode ( SamplerMsg::MIPMAP_NEAREST );
        }
        const std::string material_reference = mResourcePath + "/materials/" + mName + material_extension;
        WriteMessage ( material, ( output / "materials" / ( mName + material_extension ) ).string(), "AEONMTL", mBinary );

        ModelMsg model{};
        model.mutable_default_pipeline()->set_path ( mPipeline );
        model.mutable_default_material()->set_path ( material_reference );
        model.mutable_skeleton()->set_path ( mSkeletonPath );
        for ( const Part& part : recipe.mParts )
        {
            if ( part.mName.empty() )
            {
                continue;
            }
            AssemblyMsg* assembly = model.add_assembly();
            assembly->mutable_mesh()->set_path ( mResourcePath + "/meshes/" + part.mName + "." + mMeshExtension );
        }
        const std::string model_path = ( output / ( mName + model_extension ) ).string();
        WriteMessage ( model, model_path, "AEONMDL", mBinary );

        std::cout << "Wrote " << model.assembly_size() << " assemblies to " << model_path << std::endl;
        return 0;
    }
}
