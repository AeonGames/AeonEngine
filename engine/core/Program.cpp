/*
Copyright (C) 2017-2016 Rodrigo Jose Hernandez Cordoba

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
#include <fstream>
#include <sstream>
#include <ostream>
#include <regex>
#include "aeongames/ProtoBufClasses.h"
#include "ProtoBufHelpers.h"
#include "aeongames/Utilities.h"
#include "aeongames/Program.h"

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4251 )
#endif
#include "program.pb.h"
#ifdef _MSC_VER
#pragma warning( pop )
#endif


namespace AeonGames
{
    Program::Program ( std::string  aFilename ) :
        mFilename ( std::move ( aFilename ) ), mVertexShader(), mFragmentShader()
    {
        try
        {
            Initialize();
        }
        catch ( ... )
        {
            Finalize();
            throw;
        }
    }

    Program::~Program()
        = default;

    const std::string & Program::GetVertexShaderSource() const
    {
        return mVertexShader;
    }

    const std::string & Program::GetFragmentShaderSource() const
    {
        return mFragmentShader;
    }

    const std::vector<Uniform>& Program::GetUniformMetaData() const
    {
        return mUniformMetaData;
    }

    void Program::Initialize()
    {
        static ProgramBuffer program_buffer;
        LoadProtoBufObject<ProgramBuffer> ( program_buffer, mFilename, "AEONPRG" );
        {
            mVertexShader.append ( "#version " + std::to_string ( program_buffer.glsl_version() ) + "\n" );
            mVertexShader.append (
                "layout(location = 0) in vec3 VertexPosition;\n"
                "layout(location = 1) in vec3 VertexNormal;\n"
                "layout(location = 2) in vec3 VertexTangent;\n"
                "layout(location = 3) in vec3 VertexBitangent;\n"
                "layout(location = 4) in vec2 VertexUV;\n"
                "layout(location = 5) in vec4 VertexWeightIndices;\n"
                "layout(location = 6) in vec4 VertexWeights;\n" );
            mVertexShader.append (
                "layout(std140) uniform Matrices{\n"
                "mat4 ViewMatrix;\n"
                "mat4 ProjectionMatrix;\n"
                "mat4 ModelMatrix;\n"
                "mat4 ViewProjectionMatrix;\n"
                "mat4 ModelViewMatrix;\n"
                "mat4 ModelViewProjectionMatrix;\n"
                "mat3 NormalMatrix;\n"
                "};\n"
            );

            mFragmentShader.append ( "#version " + std::to_string ( program_buffer.glsl_version() ) + "\n" );
            mFragmentShader.append (
                "layout(std140) uniform Matrices{\n"
                "mat4 ViewMatrix;\n"
                "mat4 ProjectionMatrix;\n"
                "mat4 ModelMatrix;\n"
                "mat4 ViewProjectionMatrix;\n"
                "mat4 ModelViewMatrix;\n"
                "mat4 ModelViewProjectionMatrix;\n"
                "mat3 NormalMatrix;\n"
                "};\n" );

            mUniformMetaData.clear();
            mUniformMetaData.reserve ( program_buffer.property().size() );
            if ( program_buffer.property().size() > 0 )
            {
                std::string properties ( "layout(std140) uniform Properties{\n" );
                std::string samplers ( "//----SAMPLERS-START----\n" );
                for ( auto& i : program_buffer.property() )
                {
                    switch ( i.type() )
                    {
                    case PropertyBuffer_Type_FLOAT:
                        mUniformMetaData.emplace_back ( i.uniform_name(), i.scalar_float() );
                        properties.append ( mUniformMetaData.back().GetDeclaration() );
                        break;
                    case PropertyBuffer_Type_FLOAT_VEC2:
                        mUniformMetaData.emplace_back ( i.uniform_name(), i.vector2().x(), i.vector2().y() );
                        properties.append ( mUniformMetaData.back().GetDeclaration() );
                        break;
                    case PropertyBuffer_Type_FLOAT_VEC3:
                        mUniformMetaData.emplace_back ( i.uniform_name(), i.vector3().x(), i.vector3().y(), i.vector3().z() );
                        properties.append ( mUniformMetaData.back().GetDeclaration() );
                        break;
                    case PropertyBuffer_Type_FLOAT_VEC4:
                        mUniformMetaData.emplace_back ( i.uniform_name(), i.vector4().x(), i.vector4().y(), i.vector4().z(), i.vector4().w() );
                        properties.append ( mUniformMetaData.back().GetDeclaration() );
                        break;
                    case PropertyBuffer_Type_SAMPLER_2D:
                        mUniformMetaData.emplace_back ( i.uniform_name(), i.texture() );
                        samplers.append ( mUniformMetaData.back().GetDeclaration() );
                        break;
                    case PropertyBuffer_Type_SAMPLER_CUBE:
                        //type_name = "samplerCube ";
                        /* To be continued ... */
                        break;
                    default:
                        throw std::runtime_error ( "Unknown Type." );
                    }
                }
                properties.append ( "};\n" );
                samplers.append ( "//----SAMPLERS-END----\n" );
                mVertexShader.append ( properties );
                mVertexShader.append ( samplers );
                mFragmentShader.append ( properties );
                mFragmentShader.append ( samplers );
            }
            switch ( program_buffer.vertex_shader().source_case() )
            {
            case ShaderBuffer::SourceCase::kCode:
                mVertexShader.append ( program_buffer.vertex_shader().code() );
                break;
            default:
                throw std::runtime_error ( "ByteCode Shader Type not implemented yet." );
            }
            switch ( program_buffer.fragment_shader().source_case() )
            {
            case ShaderBuffer::SourceCase::kCode:
                mFragmentShader.append ( program_buffer.fragment_shader().code() );
                break;
            default:
                throw std::runtime_error ( "ByteCode Shader Type not implemented yet." );
            }
        }
        program_buffer.Clear();
#if 0
        std::ofstream shader_vert ( "shader.vert" );
        std::ofstream shader_frag ( "shader.frag" );
        shader_vert << mVertexShader;
        shader_frag << mFragmentShader;
        shader_vert.close();
        shader_frag.close();
#endif
    }

    void Program::Finalize()
    {
    }
}
