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
#include <array>
#include <cctype>
#include <cerrno>
#include <cstring>
#include <fstream>
#include <iostream>
#include <spawn.h>
#include <stdexcept>
#include <string_view>
#include <sys/wait.h>
#include <unordered_map>
#include <vector>
#include "aeongames/ProtoBufClasses.hpp"
#include "CodeFieldValuePrinter.hpp"
#include "pipeline.pb.h"
#include <google/protobuf/struct.pb.h>
#include <google/protobuf/text_format.h>
#include <google/protobuf/util/json_util.h>
#include "MetalShaderTool.h"

extern char** environ;

namespace AeonGames
{
    namespace
    {
        struct StageDescription
        {
            const char* glslang_name;
            const char* entry_name;
            PipelineMsg_ShaderInterface_Stage stage;
        };

        constexpr std::array<StageDescription, 6> kStages
        {
            {
                {"vert", "aeon_vertex", PipelineMsg_ShaderInterface_Stage_STAGE_VERTEX},
                {"frag", "aeon_fragment", PipelineMsg_ShaderInterface_Stage_STAGE_FRAGMENT},
                {"comp", "aeon_compute", PipelineMsg_ShaderInterface_Stage_STAGE_COMPUTE},
                {"tesc", "aeon_tess_control", PipelineMsg_ShaderInterface_Stage_STAGE_TESS_CONTROL},
                {"tese", "aeon_tess_evaluation", PipelineMsg_ShaderInterface_Stage_STAGE_TESS_EVALUATION},
                {"geom", "aeon_geometry", PipelineMsg_ShaderInterface_Stage_STAGE_GEOMETRY}
            }};

        bool RendererInSet ( std::string_view aSet, std::string_view aRenderer )
        {
            size_t start = 0;
            while ( true )
            {
                const size_t comma = aSet.find ( ',', start );
                const size_t end = comma == std::string_view::npos ? aSet.size() : comma;
                if ( aSet.substr ( start, end - start ) == aRenderer )
                {
                    return true;
                }
                if ( comma == std::string_view::npos )
                {
                    return false;
                }
                start = comma + 1;
            }
        }

        template<typename Map>
        const typename Map::mapped_type* ResolveVariant ( const Map& aVariants, std::string_view aRenderer )
        {
            for ( const auto& entry : aVariants )
            {
                if ( !entry.first.empty() && RendererInSet ( entry.first, aRenderer ) )
                {
                    return &entry.second;
                }
            }
            const auto it = aVariants.find ( std::string{} );
            return it != aVariants.end() ? &it->second : nullptr;
        }

        template<typename Map>
        void RemoveRendererFromSelectors ( Map* aVariants, std::string_view aRenderer )
        {
            std::vector<std::string> erase_keys;
            std::vector<std::pair<std::string, typename Map::mapped_type>> replacements;
            for ( const auto& entry : *aVariants )
            {
                if ( entry.first.empty() || !RendererInSet ( entry.first, aRenderer ) )
                {
                    continue;
                }
                std::string remaining;
                std::string_view selector{entry.first};
                size_t start = 0;
                while ( true )
                {
                    const size_t comma = selector.find ( ',', start );
                    const size_t end = comma == std::string_view::npos ? selector.size() : comma;
                    const std::string_view name = selector.substr ( start, end - start );
                    if ( name != aRenderer )
                    {
                        if ( !remaining.empty() )
                        {
                            remaining += ',';
                        }
                        remaining.append ( name );
                    }
                    if ( comma == std::string_view::npos )
                    {
                        break;
                    }
                    start = comma + 1;
                }
                erase_keys.push_back ( entry.first );
                if ( !remaining.empty() )
                {
                    replacements.emplace_back ( std::move ( remaining ), entry.second );
                }
            }
            for ( const std::string& key : erase_keys )
            {
                aVariants->erase ( key );
            }
            for ( auto& replacement : replacements )
            {
                ( *aVariants ) [replacement.first] = std::move ( replacement.second );
            }
        }

        std::string ReadFile ( const std::filesystem::path& aPath )
        {
            std::ifstream stream ( aPath, std::ios::binary );
            if ( !stream )
            {
                throw std::runtime_error ( "failed to open " + aPath.string() );
            }
            return std::string ( std::istreambuf_iterator<char> ( stream ), std::istreambuf_iterator<char>() );
        }

        void WriteFile ( const std::filesystem::path& aPath, std::string_view aContents )
        {
            std::ofstream stream ( aPath, std::ios::binary );
            if ( !stream )
            {
                throw std::runtime_error ( "failed to create " + aPath.string() );
            }
            stream.write ( aContents.data(), static_cast<std::streamsize> ( aContents.size() ) );
        }

        void Run ( const std::vector<std::string>& aArguments )
        {
            std::vector<char*> arguments;
            arguments.reserve ( aArguments.size() + 1 );
            for ( const std::string& argument : aArguments )
            {
                arguments.push_back ( const_cast<char*> ( argument.c_str() ) );
            }
            arguments.push_back ( nullptr );
            pid_t process = 0;
            const int spawn_result = posix_spawn ( &process, arguments[0], nullptr, nullptr, arguments.data(), environ );
            if ( spawn_result != 0 )
            {
                throw std::runtime_error ( "failed to launch " + aArguments[0] + ": " + std::strerror ( spawn_result ) );
            }
            int status = 0;
            if ( waitpid ( process, &status, 0 ) < 0 )
            {
                throw std::runtime_error ( "failed waiting for " + aArguments[0] + ": " + std::strerror ( errno ) );
            }
            if ( !WIFEXITED ( status ) || WEXITSTATUS ( status ) != 0 )
            {
                throw std::runtime_error ( aArguments[0] + " failed" );
            }
        }

        const google::protobuf::Value* Field ( const google::protobuf::Struct& aObject, const char* aName )
        {
            const auto it = aObject.fields().find ( aName );
            return it != aObject.fields().end() ? &it->second : nullptr;
        }

        uint32_t Number ( const google::protobuf::Struct& aObject, const char* aName, uint32_t aDefault = 0 )
        {
            const google::protobuf::Value* value = Field ( aObject, aName );
            return value != nullptr && value->kind_case() == google::protobuf::Value::kNumberValue
                   ? static_cast<uint32_t> ( value->number_value() ) : aDefault;
        }

        std::string String ( const google::protobuf::Struct& aObject, const char* aName )
        {
            const google::protobuf::Value* value = Field ( aObject, aName );
            return value != nullptr && value->kind_case() == google::protobuf::Value::kStringValue
                   ? value->string_value() : std::string{};
        }

        uint32_t ArrayCount ( const google::protobuf::Struct& aObject )
        {
            const google::protobuf::Value* array = Field ( aObject, "array" );
            if ( array == nullptr || array->kind_case() != google::protobuf::Value::kListValue ||
                 array->list_value().values().empty() )
            {
                return 1;
            }
            const google::protobuf::Value& count = array->list_value().values ( 0 );
            return count.kind_case() == google::protobuf::Value::kNumberValue
                   ? static_cast<uint32_t> ( count.number_value() ) : 1;
        }

        void AddResources ( const google::protobuf::Struct& aReflection, const char* aKey,
                            PipelineMsg_ShaderResource_Type aType,
                            PipelineMsg_ShaderInterface* aInterface )
        {
            const google::protobuf::Value* resources = Field ( aReflection, aKey );
            if ( resources == nullptr || resources->kind_case() != google::protobuf::Value::kListValue )
            {
                return;
            }
            for ( const google::protobuf::Value& value : resources->list_value().values() )
            {
                if ( value.kind_case() != google::protobuf::Value::kStructValue )
                {
                    continue;
                }
                const google::protobuf::Struct& source = value.struct_value();
                PipelineMsg_ShaderResource* resource = aInterface->add_resource();
                resource->set_name ( String ( source, "name" ) );
                resource->set_set ( Number ( source, "set" ) );
                resource->set_binding ( Number ( source, "binding" ) );
                resource->set_count ( ArrayCount ( source ) );
                resource->set_type ( aType );
            }
        }

        void PopulateReflection ( const std::filesystem::path& aPath,
                                  PipelineMsg_ShaderInterface* aInterface )
        {
            google::protobuf::Struct reflection;
            const auto status = google::protobuf::util::JsonStringToMessage ( ReadFile ( aPath ), &reflection );
            if ( !status.ok() )
            {
                throw std::runtime_error ( "failed to parse SPIRV-Cross reflection: " + status.ToString() );
            }
            const google::protobuf::Value* entry_points = Field ( reflection, "entryPoints" );
            if ( entry_points != nullptr && entry_points->kind_case() == google::protobuf::Value::kListValue &&
                 !entry_points->list_value().values().empty() )
            {
                const google::protobuf::Struct& entry = entry_points->list_value().values ( 0 ).struct_value();
                const google::protobuf::Value* workgroup = Field ( entry, "workgroup_size" );
                if ( workgroup != nullptr && workgroup->kind_case() == google::protobuf::Value::kListValue &&
                     workgroup->list_value().values_size() == 3 )
                {
                    aInterface->set_local_size_x ( static_cast<uint32_t> ( workgroup->list_value().values ( 0 ).number_value() ) );
                    aInterface->set_local_size_y ( static_cast<uint32_t> ( workgroup->list_value().values ( 1 ).number_value() ) );
                    aInterface->set_local_size_z ( static_cast<uint32_t> ( workgroup->list_value().values ( 2 ).number_value() ) );
                }
            }
            const google::protobuf::Value* outputs = Field ( reflection, "outputs" );
            if ( outputs != nullptr && outputs->kind_case() == google::protobuf::Value::kListValue )
            {
                uint32_t output_count = 0;
                for ( const google::protobuf::Value& value : outputs->list_value().values() )
                {
                    if ( value.kind_case() == google::protobuf::Value::kStructValue )
                    {
                        output_count = std::max ( output_count, Number ( value.struct_value(), "location" ) + 1 );
                    }
                }
                aInterface->set_fragment_output_count ( output_count );
            }
            AddResources ( reflection, "ubos", PipelineMsg_ShaderResource_Type_UNIFORM_BUFFER, aInterface );
            AddResources ( reflection, "ssbos", PipelineMsg_ShaderResource_Type_STORAGE_BUFFER, aInterface );
            AddResources ( reflection, "textures", PipelineMsg_ShaderResource_Type_COMBINED_IMAGE_SAMPLER, aInterface );
            AddResources ( reflection, "separate_images", PipelineMsg_ShaderResource_Type_SAMPLED_IMAGE, aInterface );
            AddResources ( reflection, "separate_samplers", PipelineMsg_ShaderResource_Type_SAMPLER, aInterface );
            AddResources ( reflection, "images", PipelineMsg_ShaderResource_Type_STORAGE_IMAGE, aInterface );
            const google::protobuf::Value* inputs = Field ( reflection, "inputs" );
            if ( inputs != nullptr && inputs->kind_case() == google::protobuf::Value::kListValue )
            {
                for ( const google::protobuf::Value& value : inputs->list_value().values() )
                {
                    if ( value.kind_case() != google::protobuf::Value::kStructValue )
                    {
                        continue;
                    }
                    const google::protobuf::Struct& source = value.struct_value();
                    const std::string type = String ( source, "type" );
                    PipelineMsg_ShaderInterface_Input* input = aInterface->add_input();
                    input->set_name ( String ( source, "name" ) );
                    input->set_location ( Number ( source, "location" ) );
                    input->set_components ( !type.empty() && std::isdigit ( static_cast<unsigned char> ( type.back() ) )
                                            ? static_cast<uint32_t> ( type.back() - '0' ) : 1u );
                    input->set_scalar_type (
                        type.starts_with ( "u" ) ? PipelineMsg_ShaderInterface_Input_ScalarType_SCALAR_UINT :
                        type.starts_with ( "i" ) ? PipelineMsg_ShaderInterface_Input_ScalarType_SCALAR_INT :
                        PipelineMsg_ShaderInterface_Input_ScalarType_SCALAR_FLOAT );
                }
            }
        }

        void AssignMetalArgumentBindings ( PipelineMsg_ShaderInterface* aInterface )
        {
            std::unordered_map<uint32_t, uint32_t> next_binding;
            for ( PipelineMsg_ShaderResource& resource : *aInterface->mutable_resource() )
            {
                uint32_t& binding = next_binding[resource.set()];
                resource.set_binding ( binding );
                const uint32_t count = std::max ( 1u, resource.count() );
                binding += resource.type() == PipelineMsg_ShaderResource_Type_COMBINED_IMAGE_SAMPLER
                           ? count * 2 : count;
            }
        }

        void ReadPipeline ( const std::filesystem::path& aPath, PipelineMsg& aPipeline )
        {
            std::ifstream stream ( aPath, std::ios::binary );
            std::string magic;
            std::getline ( stream, magic );
            if ( magic != "AEONPLN" )
            {
                throw std::runtime_error ( "input is not an AEONPLN text pipeline: " + aPath.string() );
            }
            const std::string text { std::istreambuf_iterator<char> ( stream ), std::istreambuf_iterator<char>() };
            if ( !google::protobuf::TextFormat::ParseFromString ( text, &aPipeline ) )
            {
                throw std::runtime_error ( "failed to parse pipeline: " + aPath.string() );
            }
        }

        void WritePipeline ( const std::filesystem::path& aPath, const PipelineMsg& aPipeline )
        {
            google::protobuf::TextFormat::Printer printer;
            for ( int field_number = 1; field_number <= aPipeline.GetDescriptor()->field_count(); ++field_number )
            {
                const auto* field = aPipeline.GetDescriptor()->FindFieldByNumber ( field_number );
                if ( field != nullptr )
                {
                    printer.RegisterFieldValuePrinter ( field, new CodeFieldValuePrinter );
                }
            }
            std::string text;
            if ( !printer.PrintToString ( aPipeline, &text ) )
            {
                throw std::runtime_error ( "failed to serialize Metal pipeline" );
            }
            std::filesystem::create_directories ( aPath.parent_path() );
            WriteFile ( aPath, std::string {"AEONPLN\n"} + text );
        }
    }

    void MetalShaderTool::ProcessArguments ( int aArgumentCount, char** aArguments )
    {
        for ( int i = 1; i < aArgumentCount; ++i )
        {
            const std::string_view option{aArguments[i]};
            if ( i + 1 >= aArgumentCount )
            {
                throw std::runtime_error ( "missing value for " + std::string {option} );
            }
            const std::string value{aArguments[++i]};
            if ( option == "--input" ) mInput = value;
            else if ( option == "--output" ) mOutput = value;
            else if ( option == "--work-dir" ) mWorkDirectory = value;
            else if ( option == "--glslang" ) mGlslang = value;
            else if ( option == "--spirv-cross" ) mSpirvCross = value;
            else if ( option == "--metal-compiler" ) mMetalCompiler = value;
            else if ( option == "--sdk-root" ) mSdkRoot = value;
            else throw std::runtime_error ( "unknown option " + std::string {option} );
        }
        if ( mInput.empty() || mOutput.empty() || mWorkDirectory.empty() ||
             mGlslang.empty() || mSpirvCross.empty() || mMetalCompiler.empty() || mSdkRoot.empty() )
        {
            throw std::runtime_error ( "required options: --input --output --work-dir --glslang --spirv-cross --metal-compiler --sdk-root" );
        }
    }

    int MetalShaderTool::operator() ( int aArgumentCount, char** aArguments )
    {
        ProcessArguments ( aArgumentCount, aArguments );
        std::filesystem::create_directories ( mWorkDirectory );
        PipelineMsg pipeline;
        ReadPipeline ( mInput, pipeline );
        PipelineMsg_ShaderInterfaces& interfaces = ( *pipeline.mutable_shader_interface() ) ["Metal"];
        interfaces.clear_stage();

        auto compile = [&] ( const std::string & aSource, const StageDescription & aStage,
                             uint32_t aComputeIndex ) -> std::string
        {
            const std::string suffix = std::string {aStage.glslang_name} + std::to_string ( aComputeIndex );
            const std::filesystem::path source_path = mWorkDirectory / ( suffix + ".glsl" );
            const std::filesystem::path spirv_path = mWorkDirectory / ( suffix + ".spv" );
            const std::filesystem::path msl_path = mWorkDirectory / ( suffix + ".metal" );
            const std::filesystem::path air_path = mWorkDirectory / ( suffix + ".air" );
            const std::filesystem::path reflection_path = mWorkDirectory / ( suffix + ".json" );
            WriteFile ( source_path, aSource );
            Run ( {
                mGlslang, "-V", "--target-env", "vulkan1.2", "-DMETAL=1", "-S", aStage.glslang_name,
                source_path.string(), "-o", spirv_path.string() } );
            Run ( {mSpirvCross, spirv_path.string(), "--reflect", "--output", reflection_path.string() } );
            std::string entry_name = aStage.entry_name;
            if ( aStage.stage == PipelineMsg_ShaderInterface_Stage_STAGE_COMPUTE )
            {
                entry_name += std::to_string ( aComputeIndex );
            }
            std::vector<std::string> cross_arguments
            {
                mSpirvCross, spirv_path.string(), "--msl", "--msl-version", "30000",
                "--msl-argument-buffers", "--msl-argument-buffer-tier", "2",
                "--rename-entry-point", "main", entry_name, aStage.glslang_name,
                "--output", msl_path.string()
            };
            if ( aSource.find ( "global_textures[]" ) != std::string::npos )
            {
                // Runtime-sized texture/sampler arrays must live in a device
                // argument buffer on Metal; set 2 is the engine's bindless set.
                cross_arguments.insert ( cross_arguments.end() - 2,
                { "--msl-device-argument-buffer", "2" } );
            }
            if ( aSource.find ( "gl_ViewIndex" ) != std::string::npos )
            {
                cross_arguments.insert ( cross_arguments.end() - 2, "--msl-multiview" );
            }
            Run ( cross_arguments );
            Run ( {mMetalCompiler, "-isysroot", mSdkRoot, "-std=metal3.0", "-c",
                   msl_path.string(), "-o", air_path.string() } );
            PipelineMsg_ShaderInterface* shader_interface = interfaces.add_stage();
            shader_interface->set_stage ( aStage.stage );
            shader_interface->set_compute_stage_index ( aComputeIndex );
            shader_interface->set_entry_point ( entry_name );
            PopulateReflection ( reflection_path, shader_interface );
            AssignMetalArgumentBindings ( shader_interface );
            return ReadFile ( msl_path );
        };

        auto process_graphics = [&] ( auto * aVariants, const StageDescription & aStage )
        {
            const std::string* resolved = ResolveVariant ( *aVariants, "Metal" );
            if ( resolved == nullptr )
            {
                return;
            }
            const std::string source = *resolved;
            RemoveRendererFromSelectors ( aVariants, "Metal" );
            ( *aVariants ) ["Metal"] = source.empty() ? std::string{} :
                                       compile ( source, aStage, 0 );
        };
        process_graphics ( pipeline.mutable_vert(), kStages[0] );
        process_graphics ( pipeline.mutable_frag(), kStages[1] );
        process_graphics ( pipeline.mutable_tesc(), kStages[3] );
        process_graphics ( pipeline.mutable_tese(), kStages[4] );
        process_graphics ( pipeline.mutable_geom(), kStages[5] );

        const PipelineMsg_ComputeStages* resolved_compute = ResolveVariant ( pipeline.comp(), "Metal" );
        if ( resolved_compute != nullptr )
        {
            const std::vector<std::string> sources ( resolved_compute->stage().begin(), resolved_compute->stage().end() );
            RemoveRendererFromSelectors ( pipeline.mutable_comp(), "Metal" );
            PipelineMsg_ComputeStages& metal_compute = ( *pipeline.mutable_comp() ) ["Metal"];
            metal_compute.clear_stage();
            for ( uint32_t i = 0; i < sources.size(); ++i )
            {
                metal_compute.add_stage ( compile ( sources[i], kStages[2], i ) );
            }
        }
        WritePipeline ( mOutput, pipeline );
        return 0;
    }
}