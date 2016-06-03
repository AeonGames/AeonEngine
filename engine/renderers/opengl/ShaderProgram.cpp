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
#include "ShaderProgram.h"
#include "OpenGLFunctions.h"

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4251 )
#endif
#include <google/protobuf/text_format.h>
#include "shader_program.pb.h"
#ifdef _MSC_VER
#pragma warning( pop )
#endif

namespace AeonGames
{
#include "main.vert.h"
#include "main.frag.h"
}
namespace AeonGames
{
ShaderProgram::ShaderProgram ( const std::string& aFilename ) try :
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

    ShaderProgram::~ShaderProgram()
    {
    }

    void ShaderProgram::Initialize()
    {
        {
            ShaderProgramBuffer shader_program_buffer;
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
                if ( !shader_program_buffer.ParseFromIstream ( &file ) )
                {
                    throw std::runtime_error ( "Binary program parsing failed." );
                }
            }
            else
            {
                std::string text ( ( std::istreambuf_iterator<char> ( file ) ), std::istreambuf_iterator<char>() );
                if ( !google::protobuf::TextFormat::ParseFromString ( text, &shader_program_buffer ) )
                {
                    throw std::runtime_error ( "Text program parsing failed." );
                }
            }
            file.close();
        }

        {
            /* This is temporary code
            meant to generate a text
            template for shader program
            files.*/
            ShaderProgramBuffer shader_program_buffer;
            shader_program_buffer.set_glsl_version ( 330 );
            PropertyBuffer *property;

            property = shader_program_buffer.add_properties();
            property->set_display_name ( "Scalar Float" );
            property->set_uniform_name ( "scalar_float_value" );
            property->set_type ( PropertyBuffer_Type::PropertyBuffer_Type_FLOAT );
            property->set_scalar_float ( 10 );

            property = shader_program_buffer.add_properties();
            property->set_display_name ( "2D Vector" );
            property->set_uniform_name ( "vec2_value" );
            property->set_type ( PropertyBuffer_Type::PropertyBuffer_Type_FLOAT_VEC2 );
            Vector2Buffer vec2;
            property->mutable_vector2()->set_x ( 1 );
            property->mutable_vector2()->set_y ( 2 );

            property = shader_program_buffer.add_properties();
            property->set_display_name ( "3D Vector" );
            property->set_uniform_name ( "vec3_value" );
            property->set_type ( PropertyBuffer_Type::PropertyBuffer_Type_FLOAT_VEC3 );
            Vector2Buffer vec3;
            property->mutable_vector3()->set_x ( 1 );
            property->mutable_vector3()->set_y ( 2 );
            property->mutable_vector3()->set_z ( 3 );

            shader_program_buffer.set_vertex_shader ( reinterpret_cast<const char*> ( main_vert ), main_vert_len );
            shader_program_buffer.set_fragment_shader ( reinterpret_cast<const char*> ( main_frag ), main_frag_len );
            // Write Text Version
            std::string text_string;
            std::ofstream text_file ( "shader_program.txt", std::ifstream::out );
            google::protobuf::TextFormat::PrintToString ( shader_program_buffer, &text_string );
            text_file << "AEONPRG" << std::endl;
            text_file.write ( text_string.c_str(), text_string.length() );
            text_file.close();
        }
        //--------------------------------------------------
        mProgram = glCreateProgram();
        OPENGL_CHECK_ERROR_THROW;
        GLint compile_status;
        GLint link_status;
        //-------------------------
        uint32_t vertex_shader = glCreateShader ( GL_VERTEX_SHADER );
        OPENGL_CHECK_ERROR_THROW;
        const GLchar* vertex_shader_source = reinterpret_cast<const GLchar *> ( main_vert );
        GLint vertex_shader_len = main_vert_len;
        glShaderSource ( vertex_shader, 1, &vertex_shader_source, &vertex_shader_len );
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
                glGetShaderInfoLog ( vertex_shader, info_log_len, NULL, const_cast<GLchar*> ( log_string.data() ) );
                OPENGL_CHECK_ERROR_THROW;
            }
        }
        glAttachShader ( mProgram, vertex_shader );
        OPENGL_CHECK_ERROR_THROW;
        //-------------------------
        uint32_t fragment_shader = glCreateShader ( GL_FRAGMENT_SHADER );
        OPENGL_CHECK_ERROR_THROW;
        const GLchar* fragment_shader_source = reinterpret_cast<const GLchar *> ( main_frag );
        GLint fragment_shader_len = main_frag_len;
        glShaderSource ( fragment_shader, 1, &fragment_shader_source, &fragment_shader_len );
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
    }

    void ShaderProgram::Finalize()
    {
        if ( glIsProgram ( mProgram ) )
        {
            glDeleteProgram ( mProgram );
            mProgram = 0;
        }
    }
}