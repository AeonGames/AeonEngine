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

#ifdef __linux__
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4251 )
#endif
#include <google/protobuf/text_format.h>
#include "program.pb.h"
#ifdef _MSC_VER
#pragma warning( pop )
#endif

#include "OpenGLProgram.h"
#include "OpenGLFunctions.h"

namespace AeonGames
{
    OpenGLProgram::OpenGLProgram ( const std::string& aFilename )
try :
        mFilename ( aFilename ),
                  mProgram ( 0 )
    {
        Initialize();
    }
    catch ( ... )
    {
        Finalize();
        throw;
    }

    OpenGLProgram::~OpenGLProgram()
    {
    }
#if 0
    void OpenGLProgram::SetViewMatrix ( const float aMatrix[16] )
    {
        if ( mViewMatrixLocation >= 0 )
        {
            glUniformMatrix4fv ( mViewMatrixLocation, 1, false, aMatrix );
            OPENGL_CHECK_ERROR_NO_THROW;
        }
    }

    void OpenGLProgram::SetProjectionMatrix ( const float aMatrix[16] )
    {
        if ( mProjectionMatrixLocation >= 0 )
        {
            glUniformMatrix4fv ( mProjectionMatrixLocation, 1, false, aMatrix );
            OPENGL_CHECK_ERROR_NO_THROW;
        }
    }

    void OpenGLProgram::SetModelMatrix ( const float aMatrix[16] )
    {
        if ( mModelMatrixLocation >= 0 )
        {
            glUniformMatrix4fv ( mModelMatrixLocation, 1, false, aMatrix );
            OPENGL_CHECK_ERROR_NO_THROW;
        }
    }

    void OpenGLProgram::SetViewProjectionMatrix ( const float aMatrix[16] )
    {
        if ( mViewProjectionMatrixLocation >= 0 )
        {
            glUniformMatrix4fv ( mViewProjectionMatrixLocation, 1, false, aMatrix );
            OPENGL_CHECK_ERROR_NO_THROW;
        }
    }

    void OpenGLProgram::SetModelViewMatrix ( const float aMatrix[16] )
    {
        if ( mModelViewMatrixLocation >= 0 )
        {
            glUniformMatrix4fv ( mModelViewMatrixLocation, 1, false, aMatrix );
            OPENGL_CHECK_ERROR_NO_THROW;
        }
    }

    void OpenGLProgram::SetModelViewProjectionMatrix ( const float aMatrix[16] )
    {
        if ( mModelViewProjectionMatrixLocation >= 0 )
        {
            glUniformMatrix4fv ( mModelViewProjectionMatrixLocation, 1, false, aMatrix );
            OPENGL_CHECK_ERROR_NO_THROW;
        }
    }

    void OpenGLProgram::SetNormalMatrix ( const float aMatrix[9] )
    {
        if ( mNormalMatrixLocation >= 0 )
        {
            glUniformMatrix3fv ( mNormalMatrixLocation, 1, false, aMatrix );
            OPENGL_CHECK_ERROR_NO_THROW;
        }
    }
#endif
    void OpenGLProgram::Use() const
    {
        glUseProgram ( mProgram );
        OPENGL_CHECK_ERROR_NO_THROW;
    }

    uint32_t OpenGLProgram::GetMatricesBlockIndex() const
    {
        return mMatricesBlockIndex;
    }

    void OpenGLProgram::Initialize()
    {
        static ProgramBuffer program_buffer;
        {
            struct stat stat_buffer;
            if ( stat ( mFilename.c_str(), &stat_buffer ) != 0 )
            {
                std::ostringstream stream;
                stream << "File " << mFilename << " Not Found (error code:" << errno << ")";
                throw std::runtime_error ( stream.str().c_str() );
            }
            std::ifstream file;
            file.exceptions ( std::ifstream::failbit | std::ifstream::badbit );
            file.open ( mFilename, std::ifstream::in | std::ifstream::binary );
            char magick_number[8] = { 0 };
            file.read ( magick_number, sizeof ( magick_number ) );
            file.exceptions ( std::ifstream::badbit );

            if ( strncmp ( magick_number, "AEONPRG", 7 ) )
            {
                file.close();
                std::ostringstream stream;
                stream << "File" << mFilename << " Is not in AeonGames PRG format.";
                throw std::runtime_error ( stream.str().c_str() );
            }
            else if ( magick_number[7] == '\0' )
            {
                if ( !program_buffer.ParseFromIstream ( &file ) )
                {
                    throw std::runtime_error ( "Binary program parsing failed." );
                }
            }
            else
            {
                std::string text ( ( std::istreambuf_iterator<char> ( file ) ), std::istreambuf_iterator<char>() );
                if ( !google::protobuf::TextFormat::ParseFromString ( text, &program_buffer ) )
                {
                    throw std::runtime_error ( "Text program parsing failed." );
                }
            }
            file.close();
        }

        std::string vertex_shader_source;
        {
            vertex_shader_source.append ( "#version " + std::to_string ( program_buffer.glsl_version() ) + "\n" );
            vertex_shader_source.append (
                "layout(location = 0) in vec3 VertexPosition;\n"
                "layout(location = 1) in vec3 VertexNormal;\n"
                "layout(location = 2) in vec3 VertexTangent;\n"
                "layout(location = 3) in vec3 VertexBitangent;\n"
                "layout(location = 4) in vec2 VertexUV;\n"
                "layout(location = 5) in vec4 VertexWeightIndices;\n"
                "layout(location = 6) in vec4 VertexWeights;\n" );
            vertex_shader_source.append (
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
            mDefaultValues.clear();
            mDefaultValues.reserve ( program_buffer.properties().size() );
            for ( auto& i : program_buffer.properties() )
            {
                std::string type_name;
                std::string default_value ( "" );
                switch ( i.type() )
                {
                case PropertyBuffer_Type_FLOAT:
                    mDefaultValues.emplace_back ( i.scalar_float() );
                    type_name = "float ";
                    if ( ( program_buffer.glsl_version() >= 120 ) && ( i.default_value_case() == PropertyBuffer::DefaultValueCase::kScalarFloat ) )
                    {
                        default_value = " = " + std::to_string ( i.scalar_float() );
                    }
                    break;
                case PropertyBuffer_Type_FLOAT_VEC2:
                    mDefaultValues.emplace_back ( i.vector2().x(), i.vector2().y() );
                    type_name = "vec2 ";
                    if ( ( program_buffer.glsl_version() >= 120 ) && ( i.default_value_case() == PropertyBuffer::DefaultValueCase::kVector2 ) )
                    {
                        default_value = " = vec2( " +
                                        std::to_string ( i.vector2().x() ) + ", " +
                                        std::to_string ( i.vector2().y() ) + ")";
                    }
                    break;
                case PropertyBuffer_Type_FLOAT_VEC3:
                    mDefaultValues.emplace_back ( i.vector3().x(), i.vector3().y(), i.vector3().z() );
                    type_name = "vec3 ";
                    if ( ( program_buffer.glsl_version() >= 120 ) && ( i.default_value_case() == PropertyBuffer::DefaultValueCase::kVector3 ) )
                    {
                        default_value = " = vec3( " +
                                        std::to_string ( i.vector3().x() ) + ", " +
                                        std::to_string ( i.vector3().y() ) + ", " +
                                        std::to_string ( i.vector3().z() ) + ")";
                    }
                    break;
                case PropertyBuffer_Type_FLOAT_VEC4:
                    mDefaultValues.emplace_back ( i.vector4().x(), i.vector4().y(), i.vector4().z(), i.vector4().w() );
                    type_name = "vec4 ";
                    if ( ( program_buffer.glsl_version() >= 120 ) && ( i.default_value_case() == PropertyBuffer::DefaultValueCase::kVector4 ) )
                    {
                        default_value = " = vec4( " +
                                        std::to_string ( i.vector4().x() ) + ", " +
                                        std::to_string ( i.vector4().y() ) + ", " +
                                        std::to_string ( i.vector4().z() ) + ", " +
                                        std::to_string ( i.vector4().w() ) + ")";
                    }
                    break;
                case PropertyBuffer_Type_SAMPLER_2D:
                    type_name = "sampler2D ";
                    /* To be continued ... */
                    break;
                case PropertyBuffer_Type_SAMPLER_CUBE:
                    type_name = "samplerCube ";
                    /* To be continued ... */
                    break;
                default:
                    assert ( 0 && "Unknown Type." );
                }
                vertex_shader_source.append ( "uniform " + type_name + i.uniform_name() + default_value + ";\n" );
            }
            vertex_shader_source.append ( program_buffer.vertex_shader().code() );
            vertex_shader_source.append (
                "void main()\n"
                "{\n"
                "gl_Position = ModelViewProjectionMatrix * vec4(VertexPosition, 1.0);\n" +
                program_buffer.vertex_shader().entry_point() +
                "\n}\n" );
        }
        std::string fragment_shader_source;
        {
            fragment_shader_source.append ( "#version " + std::to_string ( program_buffer.glsl_version() ) + "\n" );
            fragment_shader_source.append (
                "layout(std140) uniform Matrices{\n"
                "mat4 ViewMatrix;\n"
                "mat4 ProjectionMatrix;\n"
                "mat4 ModelMatrix;\n"
                "mat4 ViewProjectionMatrix;\n"
                "mat4 ModelViewMatrix;\n"
                "mat4 ModelViewProjectionMatrix;\n"
                "mat3 NormalMatrix;\n"
                "};\n" );
            for ( auto& i : program_buffer.properties() )
            {
                std::string type_name;
                std::string default_value ( "" );
                switch ( i.type() )
                {
                case PropertyBuffer_Type_FLOAT:
                    type_name = " float ";
                    if ( ( program_buffer.glsl_version() >= 120 ) && ( i.default_value_case() == PropertyBuffer::DefaultValueCase::kScalarFloat ) )
                    {
                        default_value = " = " + std::to_string ( i.scalar_float() );
                    }
                    break;
                case PropertyBuffer_Type_FLOAT_VEC2:
                    type_name = " vec2 ";
                    if ( ( program_buffer.glsl_version() >= 120 ) && ( i.default_value_case() == PropertyBuffer::DefaultValueCase::kVector2 ) )
                    {
                        default_value = " = vec2( " +
                                        std::to_string ( i.vector2().x() ) + ", " +
                                        std::to_string ( i.vector2().y() ) + ")";
                    }
                    break;
                case PropertyBuffer_Type_FLOAT_VEC3:
                    type_name = " vec3 ";
                    if ( ( program_buffer.glsl_version() >= 120 ) && ( i.default_value_case() == PropertyBuffer::DefaultValueCase::kVector3 ) )
                    {
                        default_value = " = vec3( " +
                                        std::to_string ( i.vector3().x() ) + ", " +
                                        std::to_string ( i.vector3().y() ) + ", " +
                                        std::to_string ( i.vector3().z() ) + ")";
                    }
                    break;
                case PropertyBuffer_Type_FLOAT_VEC4:
                    type_name = " vec4 ";
                    if ( ( program_buffer.glsl_version() >= 120 ) && ( i.default_value_case() == PropertyBuffer::DefaultValueCase::kVector4 ) )
                    {
                        default_value = " = vec4( " +
                                        std::to_string ( i.vector4().x() ) + ", " +
                                        std::to_string ( i.vector4().y() ) + ", " +
                                        std::to_string ( i.vector4().z() ) + ", " +
                                        std::to_string ( i.vector4().w() ) + ")";
                    }
                    break;
                case PropertyBuffer_Type_SAMPLER_2D:
                    type_name = " sampler2D ";
                    /* To be continued ... */
                    break;
                case PropertyBuffer_Type_SAMPLER_CUBE:
                    type_name = " samplerCube ";
                    /* To be continued ... */
                    break;
                default:
                    assert ( 0 && "Unknown Type." );
                }
                fragment_shader_source.append ( "uniform " + type_name + i.uniform_name() + default_value + ";\n" );
            }
            fragment_shader_source.append ( program_buffer.fragment_shader().code() );
            fragment_shader_source.append (
                "void main()\n"
                "{\n" +
                program_buffer.fragment_shader().entry_point() +
                "\n}\n" );
        }

        //--------------------------------------------------
        // Begin OpenGL Specific code
        //--------------------------------------------------
        mProgram = glCreateProgram();
        OPENGL_CHECK_ERROR_THROW;
        GLint compile_status;
        GLint link_status;
        //-------------------------
        uint32_t vertex_shader = glCreateShader ( GL_VERTEX_SHADER );
        OPENGL_CHECK_ERROR_THROW;

        const GLchar* vertex_shader_source_ptr = reinterpret_cast<const GLchar *> ( vertex_shader_source.data() );
        GLint vertex_shader_len = static_cast<GLint> ( vertex_shader_source.length() );

        glShaderSource (
            vertex_shader,
            1,
            &vertex_shader_source_ptr,
            &vertex_shader_len );
        OPENGL_CHECK_ERROR_THROW;


        glCompileShader ( vertex_shader );
        OPENGL_CHECK_ERROR_THROW;
        glGetShaderiv ( vertex_shader, GL_COMPILE_STATUS, &compile_status );
        OPENGL_CHECK_ERROR_THROW;
        if ( compile_status != GL_TRUE )
        {
            GLint info_log_len;
            glGetShaderiv ( vertex_shader, GL_INFO_LOG_LENGTH, &info_log_len );
            OPENGL_CHECK_ERROR_THROW;
            std::string log_string;
            log_string.resize ( info_log_len );
            if ( info_log_len > 1 )
            {
                glGetShaderInfoLog ( vertex_shader, info_log_len, nullptr, const_cast<GLchar*> ( log_string.data() ) );
                OPENGL_CHECK_ERROR_THROW;
                std::cout << log_string << std::endl;
            }
        }
        glAttachShader ( mProgram, vertex_shader );
        OPENGL_CHECK_ERROR_THROW;
        //-------------------------

        uint32_t fragment_shader = glCreateShader ( GL_FRAGMENT_SHADER );
        OPENGL_CHECK_ERROR_THROW;

        const GLchar* fragment_shader_source_ptr = reinterpret_cast<const GLchar *> ( fragment_shader_source.c_str() );
        GLint fragment_shader_len = static_cast<GLint> ( fragment_shader_source.length() );

        glShaderSource ( fragment_shader, 1, &fragment_shader_source_ptr, &fragment_shader_len );
        OPENGL_CHECK_ERROR_THROW;
        glCompileShader ( fragment_shader );
        OPENGL_CHECK_ERROR_THROW;
        glGetShaderiv ( fragment_shader, GL_COMPILE_STATUS, &compile_status );
        OPENGL_CHECK_ERROR_THROW;
        if ( compile_status != GL_TRUE )
        {
            GLint info_log_len;
            glGetShaderiv ( fragment_shader, GL_INFO_LOG_LENGTH, &info_log_len );
            OPENGL_CHECK_ERROR_THROW;
            std::string log_string;
            log_string.resize ( info_log_len );
            if ( info_log_len > 1 )
            {
                glGetShaderInfoLog ( fragment_shader, info_log_len, nullptr, const_cast<GLchar*> ( log_string.data() ) );
                std::cout << log_string << std::endl;
                OPENGL_CHECK_ERROR_THROW;
            }
        }
        glAttachShader ( mProgram, fragment_shader );
        OPENGL_CHECK_ERROR_THROW;
        //-------------------------
        glLinkProgram ( mProgram );
        OPENGL_CHECK_ERROR_THROW;
        glGetProgramiv ( mProgram, GL_LINK_STATUS, &link_status );
        OPENGL_CHECK_ERROR_THROW;
        if ( link_status != GL_TRUE )

        {
            GLint info_log_len;
            glGetProgramiv ( mProgram, GL_INFO_LOG_LENGTH, &info_log_len );
            OPENGL_CHECK_ERROR_THROW;
            std::string log_string;
            log_string.resize ( info_log_len );
            if ( info_log_len > 1 )
            {
                glGetProgramInfoLog ( mProgram, info_log_len, nullptr, const_cast<GLchar*> ( log_string.data() ) );
                OPENGL_CHECK_ERROR_THROW;
            }
        }
        glDetachShader ( mProgram, vertex_shader );
        OPENGL_CHECK_ERROR_THROW;
        glDetachShader ( mProgram, fragment_shader );
        OPENGL_CHECK_ERROR_THROW;
        glDeleteShader ( vertex_shader );
        OPENGL_CHECK_ERROR_THROW;
        glDeleteShader ( fragment_shader );
        OPENGL_CHECK_ERROR_THROW;
#if 0
        // Get Matrix uniform locations
        mViewMatrixLocation = glGetUniformLocation ( mProgram, "ViewMatrix" );
        OPENGL_CHECK_ERROR_THROW;
        mProjectionMatrixLocation = glGetUniformLocation ( mProgram, "ProjectionMatrix" );
        OPENGL_CHECK_ERROR_THROW;
        mModelMatrixLocation = glGetUniformLocation ( mProgram, "ModelMatrix" );
        OPENGL_CHECK_ERROR_THROW;
        mViewProjectionMatrixLocation = glGetUniformLocation ( mProgram, "ViewProjectionMatrix" );
        OPENGL_CHECK_ERROR_THROW;
        mModelViewMatrixLocation = glGetUniformLocation ( mProgram, "ModelViewMatrix" );
        OPENGL_CHECK_ERROR_THROW;
        mModelViewProjectionMatrixLocation = glGetUniformLocation ( mProgram, "ModelViewProjectionMatrix" );
        OPENGL_CHECK_ERROR_THROW;
        mNormalMatrixLocation = glGetUniformLocation ( mProgram, "NormalMatrix" );
        OPENGL_CHECK_ERROR_THROW;
#else
        mMatricesBlockIndex = glGetUniformBlockIndex ( mProgram, "Matrices" );
        OPENGL_CHECK_ERROR_THROW;
        GLint block_size;
        glGetActiveUniformBlockiv ( mProgram, mMatricesBlockIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &block_size );
        OPENGL_CHECK_ERROR_THROW;
        assert ( block_size >= ( sizeof ( float ) * 16 * 6 ) + ( sizeof ( float ) * 9 ) );
        glUniformBlockBinding ( mProgram, mMatricesBlockIndex, 0 );
        OPENGL_CHECK_ERROR_THROW;
#endif
        assert ( ( program_buffer.properties().size() >= mDefaultValues.size() ) && "Difference between program properties and default values." );
        for ( int i = 0; i < mDefaultValues.size(); ++i )
        {
            mDefaultValues[i].SetLocation ( glGetUniformLocation ( mProgram, program_buffer.properties().Get ( i ).uniform_name().c_str() ) );
        }
        program_buffer.Clear();
    }

    void OpenGLProgram::Finalize()
    {
        if ( glIsProgram ( mProgram ) )
        {
            glDeleteProgram ( mProgram );
            mProgram = 0;
        }
    }
}
