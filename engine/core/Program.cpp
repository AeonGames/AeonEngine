/*
Copyright 2016 Rodrigo Jose Hernandez Cordoba

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
    Program::Program ( const std::string& aFilename ) :
        mFilename ( aFilename ), mVertexShader(), mFragmentShader()
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
    {
    }

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
            mVertexShader.append ( "#extension GL_ARB_bindless_texture : require\n"
                                   "layout (bindless_sampler) uniform;\n" );
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
            mFragmentShader.append ( "#extension GL_ARB_bindless_texture : require\n"
                                     "layout (bindless_sampler) uniform;\n" );
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
                mVertexShader.append ( "layout(std140) uniform Properties{\n" );
                mFragmentShader.append ( "layout(std140) uniform Properties{\n" );
                for ( auto& i : program_buffer.property() )
                {
                    switch ( i.type() )
                    {
                    case PropertyBuffer_Type_FLOAT:
                        mUniformMetaData.emplace_back ( i.uniform_name(), i.scalar_float() );
                        mVertexShader.append ( mUniformMetaData.back().GetDeclaration() );
                        mFragmentShader.append ( mUniformMetaData.back().GetDeclaration() );
                        break;
                    case PropertyBuffer_Type_FLOAT_VEC2:
                        mUniformMetaData.emplace_back ( i.uniform_name(), i.vector2().x(), i.vector2().y() );
                        mVertexShader.append ( mUniformMetaData.back().GetDeclaration() );
                        mFragmentShader.append ( mUniformMetaData.back().GetDeclaration() );
                        break;
                    case PropertyBuffer_Type_FLOAT_VEC3:
                        mUniformMetaData.emplace_back ( i.uniform_name(), i.vector3().x(), i.vector3().y(), i.vector3().z() );
                        mVertexShader.append ( mUniformMetaData.back().GetDeclaration() );
                        mFragmentShader.append ( mUniformMetaData.back().GetDeclaration() );
                        break;
                    case PropertyBuffer_Type_FLOAT_VEC4:
                        mUniformMetaData.emplace_back ( i.uniform_name(), i.vector4().x(), i.vector4().y(), i.vector4().z(), i.vector4().w() );
                        mVertexShader.append ( mUniformMetaData.back().GetDeclaration() );
                        mFragmentShader.append ( mUniformMetaData.back().GetDeclaration() );
                        break;
                    case PropertyBuffer_Type_SAMPLER_2D:
                        mUniformMetaData.emplace_back ( i.uniform_name(), i.texture() );
                        mVertexShader.append ( mUniformMetaData.back().GetDeclaration() );
                        mFragmentShader.append ( mUniformMetaData.back().GetDeclaration() );
                        break;
                    case PropertyBuffer_Type_SAMPLER_CUBE:
                        //type_name = "samplerCube ";
                        /* To be continued ... */
                        break;
                    default:
                        assert ( 0 && "Unknown Type." );
                    }
                }
                mVertexShader.append ( "};\n" );
                mFragmentShader.append ( "};\n" );
            }
            mVertexShader.append ( program_buffer.vertex_shader().code() );
            mFragmentShader.append ( program_buffer.fragment_shader().code() );
        }
        program_buffer.Clear();
    }

    void Program::Finalize()
    {
    }
}
