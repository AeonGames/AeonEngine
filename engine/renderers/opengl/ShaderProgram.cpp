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
#if 0
    /*
    This code has to be moved to a converter tool
    */
    class CodeFieldValuePrinter : public google::protobuf::TextFormat::FieldValuePrinter
    {
    public:
        CodeFieldValuePrinter() : google::protobuf::TextFormat::FieldValuePrinter()
        {
        };
        virtual std::string PrintString ( const std::string & val ) const override
        {
            std::string pattern ( "\\\\n" );
            std::regex newline ( pattern );
            std::string printed ( google::protobuf::TextFormat::FieldValuePrinter::PrintString ( val ) );
            printed = std::regex_replace ( printed, newline, "$&\"\n\"" );
            return printed;
        }
    };
#endif
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
        static ShaderProgramBuffer shader_program_buffer;
#if 1
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
                if ( !shader_program_buffer.ParseFromIstream ( &file ) )
                {
                    throw std::runtime_error ( "Binary program parsing failed." );
                }
            }
            else
            {
                std::string text ( ( std::istreambuf_iterator<char> ( file ) ), std::istreambuf_iterator<char>() );
                std::cout << text << std::endl;
                if ( !google::protobuf::TextFormat::ParseFromString ( text, &shader_program_buffer ) )
                {
                    throw std::runtime_error ( "Text program parsing failed." );
                }
            }
            file.close();
        }
#endif
#if 1
        std::string vertex_shader_source;
        vertex_shader_source.append ( "#version " + std::to_string ( shader_program_buffer.glsl_version() ) + "\n" );
        vertex_shader_source.append (
            "layout(location = 0) in vec3 VertexPosition;\n"
            "layout(location = 1) in vec3 VertexNormal;\n"
            "layout(location = 2) in vec3 VertexTangent;\n"
            "layout(location = 3) in vec3 VertexBitangent;\n"
            "layout(location = 4) in vec2 VertexUV;\n"
            "layout(location = 5) in vec4 VertexWeightIndices;\n"
            "layout(location = 6) in vec4 VertexWeights;\n" );
        vertex_shader_source.append (
            "uniform mat4 ModelViewMatrix;\n"
            "uniform mat4 ModelViewProjectionMatrix;\n"
            "uniform mat3 NormalMatrix;\n" );
        for ( auto& i : shader_program_buffer.properties() )
        {
            std::string type_name;
            std::string default_value ( "" );
            switch ( i.type() )
            {
            case PropertyBuffer_Type_FLOAT:
                type_name = "float ";
                if ( ( shader_program_buffer.glsl_version() >= 120 ) && ( i.default_value_case() == PropertyBuffer::DefaultValueCase::kScalarFloat ) )
                {
                    default_value = " = " + std::to_string ( i.scalar_float() );
                }
                break;
            case PropertyBuffer_Type_FLOAT_VEC2:
                type_name = "vec2 ";
                if ( ( shader_program_buffer.glsl_version() >= 120 ) && ( i.default_value_case() == PropertyBuffer::DefaultValueCase::kVector2 ) )
                {
                    default_value = " = vec2( " +
                                    std::to_string ( i.vector2().x() ) + ", " +
                                    std::to_string ( i.vector2().y() ) + ")";
                }
                break;
            case PropertyBuffer_Type_FLOAT_VEC3:
                type_name = "vec3 ";
                if ( ( shader_program_buffer.glsl_version() >= 120 ) && ( i.default_value_case() == PropertyBuffer::DefaultValueCase::kVector3 ) )
                {
                    default_value = " = vec3( " +
                                    std::to_string ( i.vector3().x() ) + ", " +
                                    std::to_string ( i.vector3().y() ) + ", " +
                                    std::to_string ( i.vector3().z() ) + ")";
                }
                break;
            case PropertyBuffer_Type_FLOAT_VEC4:
                type_name = "vec4 ";
                if ( ( shader_program_buffer.glsl_version() >= 120 ) && ( i.default_value_case() == PropertyBuffer::DefaultValueCase::kVector4 ) )
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
            }
            vertex_shader_source.append ( "uniform " + type_name + i.uniform_name() + default_value + ";\n" );
        }
        vertex_shader_source.append ( shader_program_buffer.vertex_shader().code() );
        vertex_shader_source.append (
            "void main()\n"
            "{\n"
            "gl_Position = ModelViewProjectionMatrix * vec4(VertexPosition, 1.0);\n" +
            shader_program_buffer.vertex_shader().entry_point() +
            "\n}\n" );
#endif

#if 0
        {
            /* This is temporary code
            meant to generate a text
            template for shader program
            files.*/
            // Write Text Version
            google::protobuf::TextFormat::Printer printer;
#if 1
            if ( !printer.RegisterFieldValuePrinter (
                     shader_program_buffer.vertex_shader().GetDescriptor()->FindFieldByName ( "code" ),
                     new CodeFieldValuePrinter ) )
            {
                std::cout << "Failed to register field value printer." << std::endl;
            }
#endif
            std::string text_string;
            std::ofstream text_file ( "shader_program.txt", std::ifstream::out );
            //google::protobuf::TextFormat::PrintToString ( shader_program_buffer, &text_string );
            printer.PrintToString ( shader_program_buffer, &text_string );
            text_file << "AEONPRG" << std::endl;
            text_file.write ( text_string.c_str(), text_string.length() );
            text_file.close();
        }
#endif
        //--------------------------------------------------
        mProgram = glCreateProgram();
        OPENGL_CHECK_ERROR_THROW;
        GLint compile_status;
        GLint link_status;
        //-------------------------
        uint32_t vertex_shader = glCreateShader ( GL_VERTEX_SHADER );
        OPENGL_CHECK_ERROR_THROW;


        std::cout << vertex_shader_source << std::endl;
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
                glGetShaderInfoLog ( vertex_shader, info_log_len, NULL, const_cast<GLchar*> ( log_string.data() ) );
                OPENGL_CHECK_ERROR_THROW;
                std::cout << log_string << std::endl;
            }
        }
        glAttachShader ( mProgram, vertex_shader );
        OPENGL_CHECK_ERROR_THROW;
        //-------------------------

        std::string fragment_shader_source;

        fragment_shader_source.append ( "#version " + std::to_string ( shader_program_buffer.glsl_version() ) + "\n" );
        fragment_shader_source.append (
            "uniform mat4 ModelViewMatrix;\n"
            "uniform mat4 ModelViewProjectionMatrix;\n"
            "uniform mat3 NormalMatrix;\n" );
        for ( auto& i : shader_program_buffer.properties() )
        {
            std::string type_name;
            std::string default_value ( "" );
            switch ( i.type() )
            {
            case PropertyBuffer_Type_FLOAT:
                type_name = " float ";
                if ( ( shader_program_buffer.glsl_version() >= 120 ) && ( i.default_value_case() == PropertyBuffer::DefaultValueCase::kScalarFloat ) )
                {
                    default_value = " = " + std::to_string ( i.scalar_float() );
                }
                break;
            case PropertyBuffer_Type_FLOAT_VEC2:
                type_name = " vec2 ";
                if ( ( shader_program_buffer.glsl_version() >= 120 ) && ( i.default_value_case() == PropertyBuffer::DefaultValueCase::kVector2 ) )
                {
                    default_value = " = vec2( " +
                                    std::to_string ( i.vector2().x() ) + ", " +
                                    std::to_string ( i.vector2().y() ) + ")";
                }
                break;
            case PropertyBuffer_Type_FLOAT_VEC3:
                type_name = " vec3 ";
                if ( ( shader_program_buffer.glsl_version() >= 120 ) && ( i.default_value_case() == PropertyBuffer::DefaultValueCase::kVector3 ) )
                {
                    default_value = " = vec3( " +
                                    std::to_string ( i.vector3().x() ) + ", " +
                                    std::to_string ( i.vector3().y() ) + ", " +
                                    std::to_string ( i.vector3().z() ) + ")";
                }
                break;
            case PropertyBuffer_Type_FLOAT_VEC4:
                type_name = " vec4 ";
                if ( ( shader_program_buffer.glsl_version() >= 120 ) && ( i.default_value_case() == PropertyBuffer::DefaultValueCase::kVector4 ) )
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
            }
            fragment_shader_source.append ( "uniform " + type_name + i.uniform_name() + default_value + ";\n" );
        }
        fragment_shader_source.append ( shader_program_buffer.fragment_shader().code() );
        fragment_shader_source.append (
            "void main()\n"
            "{\n" +
            shader_program_buffer.fragment_shader().entry_point() +
            "\n}\n" );


        std::cout << fragment_shader_source << std::endl;

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