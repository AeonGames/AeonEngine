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
#include <iostream>
#include <regex>
#include <array>
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
    static const std::array<const char*, 7> AttributeStrings
    {
        "VertexPosition",
        "VertexNormal",
        "VertexTangent",
        "VertexBitangent",
        "VertexUV",
        "VertexWeightIndices",
        "VertexWeights"
    };

    Program::Program ( std::string  aFilename ) :
        mFilename ( std::move ( aFilename ) ), mAttributes ( 0 ), mVertexShader(), mFragmentShader()
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

    uint32_t Program::GetAttributes() const
    {
        return mAttributes;
    }

    uint32_t Program::GetLocation ( AttributeBits aAttributeBit ) const
    {
        return ffs ( aAttributeBit );
    }

    uint32_t Program::GetStride() const
    {
        uint32_t stride = 0;
        for ( uint32_t i = 0; i < ffs ( ~VertexAllBits ); ++i )
        {
            if ( mAttributes & ( 1 << i ) )
            {
                stride += GetSize ( static_cast<AttributeBits> ( 1 << i ) );
            }
        }
        return stride;
    }

    Program::AttributeFormat Program::GetFormat ( AttributeBits aAttributeBit ) const
    {
        return ( aAttributeBit & VertexUVBit ) ? Vector2Float :
               ( aAttributeBit & ( VertexWeightIndicesBit | VertexWeightsBit ) ) ? Vector4Byte : Vector3Float;
    }

    uint32_t Program::GetSize ( AttributeBits aAttributeBit ) const
    {
        switch ( GetFormat ( aAttributeBit ) )
        {
        case Vector2Float:
            return sizeof ( float ) * 2;
        case Vector3Float:
            return sizeof ( float ) * 3;
        case Vector4Byte:
            return sizeof ( uint8_t ) * 4;
        }
        return 0;
    }

    uint32_t Program::GetOffset ( AttributeBits aAttributeBit ) const
    {
        uint32_t offset = 0;
        for ( uint32_t i = 1; i != aAttributeBit; i = i << 1 )
        {
            if ( i & mAttributes )
            {
                offset += GetSize ( static_cast<AttributeBits> ( i ) );
            }
        }
        return offset;
    }

    void Program::Initialize()
    {
        static ProgramBuffer program_buffer;
        LoadProtoBufObject<ProgramBuffer> ( program_buffer, mFilename, "AEONPRG" );
        {
            mVertexShader.append ( "#version " + std::to_string ( program_buffer.glsl_version() ) + "\n" );
            mVertexShader.append (
                "layout(set = 0,binding = 0,std140) uniform Matrices{\n"
                "mat4 ViewMatrix;\n"
                "mat4 ProjectionMatrix;\n"
                "mat4 ModelMatrix;\n"
                "mat4 ViewProjectionMatrix;\n"
                "mat4 ModelViewMatrix;\n"
                "mat4 ModelViewProjectionMatrix;\n"
                "mat3 NormalMatrix;\n"
                "};\n"
            );

            /* Find out which attributes are being used and add them to the shader source */
            std::smatch attribute_matches;
            /**@note static const regex: construct once, use for ever.*/
            static const std::regex attribute_regex (
                "\\bVertexPosition\\b|"
                "\\bVertexNormal\\b|"
                "\\bVertexTangent\\b|"
                "\\bVertexBitangent\\b|"
                "\\bVertexUV\\b|"
                "\\bVertexWeightIndices\\b|"
                "\\bVertexWeights\\b" );
            std::string code = program_buffer.vertex_shader().code();
            while ( std::regex_search ( code, attribute_matches, attribute_regex ) )
            {
                std::cout << attribute_matches.str() << std::endl;
                for ( uint32_t i = 0; i < AttributeStrings.size(); ++i )
                {
                    if ( attribute_matches.str().substr ( 6 ) == AttributeStrings[i] + 6 )
                    {
                        if ( ! ( mAttributes & ( 1 << i ) ) )
                        {
                            mAttributes |= ( 1 << i );
                            mVertexShader.append ( "layout(location = " );
                            mVertexShader.append ( std::to_string ( i ) );
                            mVertexShader.append ( ") in vec3 " );
                            mVertexShader.append ( AttributeStrings[i] );
                            mVertexShader.append ( ";\n" );
                        }
                        break;
                    }
                }
                code = attribute_matches.suffix();
            }

            mFragmentShader.append ( "#version " + std::to_string ( program_buffer.glsl_version() ) + "\n" );
            mFragmentShader.append (
                "layout(set = 0,binding = 0,std140) uniform Matrices{\n"
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
            {
                mVertexShader.append ( program_buffer.vertex_shader().code() );
            }
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
