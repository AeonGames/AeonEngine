/*
Copyright (C) 2018,2019,2024,2025 Rodrigo Jose Hernandez Cordoba

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
#include <string.h>
#if defined(__unix__) || defined(__MINGW32__)
#include "sys/stat.h"
#endif
#include "PipelineTool.h"
#include "aeongames/Pipeline.h"

namespace AeonGames
{
    PipelineTool::PipelineTool() = default;
    PipelineTool::~PipelineTool() = default;
    void PipelineTool::ProcessArgs ( int argc, char** argv )
    {
        if ( argc < 2 || ( strcmp ( argv[1], "pipeline" ) != 0 ) )
        {
            std::ostringstream stream;
            stream << "Invalid tool name, expected pipeline, got " << ( ( argc < 2 ) ? "nothing" : argv[1] ) << std::endl;
            throw std::runtime_error ( stream.str().c_str() );
        }
        for ( int i = 2; i < argc; ++i )
        {
            if ( argv[i][0] == '-' )
            {
                if ( argv[i][1] == '-' )
                {
                    if ( strncmp ( &argv[i][2], "in", sizeof ( "in" ) ) == 0 )
                    {
                        i++;
                        mInputFile = argv[i];
                    }
                    else if ( strncmp ( &argv[i][2], "out", sizeof ( "out" ) ) == 0 )
                    {
                        i++;
                        mOutputFile = argv[i];
                    }
                }
                else
                {
                    switch ( argv[i][1] )
                    {
                    case 'i':
                        i++;
                        mInputFile = argv[i];
                        break;
                    case 'o':
                        i++;
                        mOutputFile = argv[i];
                        break;
                    }
                }
            }
            else
            {
                mInputFile = argv[i];
            }
        }
        if ( mInputFile.empty() )
        {
            throw std::runtime_error ( "No Input file provided." );
        }
        else if ( mOutputFile.empty() )
        {
            mOutputFile = mInputFile;
        }
    }

    static xmlChar* GetParentElementProp ( xmlNodePtr node, const char* name, const char* prop )
    {
        xmlNodePtr parent = node->parent;
        while ( parent != nullptr )
        {
            if ( parent->type == XML_ELEMENT_NODE && xmlStrcmp ( parent->name, reinterpret_cast<const xmlChar * > ( name ) ) == 0 )
            {
                auto version {xmlGetProp ( parent, reinterpret_cast<const xmlChar*> ( prop ) ) };
                if ( version != nullptr )
                {
                    return version;
                }
                break;
            }
            parent = parent->parent;
        }
        return nullptr;
    }

    static void ProcessVariableNode ( xmlNodePtr node )
    {
        auto name {xmlGetProp ( node, reinterpret_cast<const xmlChar*> ( "name" ) ) };
        auto location {xmlGetProp ( node, reinterpret_cast<const xmlChar*> ( "location" ) ) };
        auto interpolation {xmlGetProp ( node, reinterpret_cast<const xmlChar*> ( "interpolation" ) ) };
        auto parent = node->parent;
        bool is_input{false};
        bool is_output{false};
        while ( parent != nullptr )
        {
            if ( xmlStrcmp ( parent->name, reinterpret_cast<const xmlChar * > ( "inputs" ) ) == 0 )
            {
                is_input = true;
                break;
            }
            else if ( xmlStrcmp ( parent->name, reinterpret_cast<const xmlChar * > ( "outputs" ) ) == 0 )
            {
                is_output = true;
                break;
            }
            parent = parent->parent;
        }
        std::cout <<
                  ( location ? "layout(location = " : "" ) <<
                  ( location ? reinterpret_cast<const char*> ( location ) : "" ) <<
                  ( location ? ") " : "" ) <<
                  ( interpolation ? reinterpret_cast<const char*> ( interpolation ) : "" ) << ( interpolation ? " " : "" ) <<
                  ( is_input ? "in " : "" ) <<
                  ( is_output ? "out " : "" ) <<
                  node->name << " " << name << ";\n";
    }

    const PipelineTool::ProcessorMap PipelineTool::XMLElementProcessors
    {
        {
            "block",
            {
                [] ( xmlNodePtr node )->void
                {
                    if ( node->parent == nullptr || xmlStrcmp ( node->parent->name, reinterpret_cast<const xmlChar * > ( "uniforms" ) ) != 0 )
                    {
                        throw std::runtime_error ( "Block element must be a child of a uniforms element." );
                    }
                    auto name {xmlGetProp ( node, reinterpret_cast<const xmlChar*> ( "name" ) ) };
                    auto binding {xmlGetProp ( node, reinterpret_cast<const xmlChar*> ( "binding" ) ) };
                    auto storage {xmlGetProp ( node, reinterpret_cast<const xmlChar*> ( "storage" ) ) };
                    std::cout <<
                              ( ( binding || storage ) ? "layout(" : "" ) <<
                              ( ( binding ) ? "binding = " : "" ) <<
                              ( ( binding ) ? reinterpret_cast<const char*> ( binding ) : "" ) <<
                              ( ( binding ) ? ", " : "" ) <<
                              ( ( storage ) ? reinterpret_cast<const char*> ( storage ) : "" ) <<
                              ( ( storage || binding ) ? ")" : "" ) <<
                    " uniform " << name << "{\n";
                },
                [] ( xmlNodePtr )->void
                {
                    std::cout << "};\n";
                }
            }
        },
        {
            "vertex",
            {
                [] ( xmlNodePtr node )->void
                {
                    std::cout << "//--Vertex shader starts here--//\n";
                    auto version {GetParentElementProp ( node, "pipeline", "version" ) };
                    if ( version != nullptr )
                    {
                        std::cout << "#version " << reinterpret_cast<const char*> ( version ) << std::endl;
                    }
                },
                [] ( xmlNodePtr )->void
                {
                    std::cout << "//--Vertex shader ends here--//\n";
                }
            }
        },
        {
            "fragment",
            {
                [] ( xmlNodePtr node )->void
                {
                    std::cout << "//--Fragment shader starts here--//\n";
                    auto version {GetParentElementProp ( node, "pipeline", "version" ) };
                    if ( version != nullptr )
                    {
                        std::cout << "#version " << reinterpret_cast<const char*> ( version ) << std::endl;
                    }
                },
                [] ( xmlNodePtr )->void
                {
                    std::cout << "//--Fragment shader ends here--//\n";
                }
            }
        },
        {
            "mat4",
            {
                ProcessVariableNode,
                {}
            }
        },
        {
            "vec2",
            {
                ProcessVariableNode,
                {}
            }
        },
        {
            "vec3",
            {
                ProcessVariableNode,
                {}
            }
        },
        {
            "vec4",
            {
                ProcessVariableNode,
                {}
            }
        },
        {
            "uint",
            {
                ProcessVariableNode,
                {}
            }
        }
    };

    void PipelineTool::ProcessNode ( xmlNodePtr node )
    {
        ProcessorMap::const_iterator processor{};
        switch ( node->type )
        {
        case XML_ELEMENT_NODE:
        {
            processor = XMLElementProcessors.find ( reinterpret_cast<const char*> ( node->name ) );
            if ( processor != XMLElementProcessors.end() )
            {
                std::get<0> ( processor->second ) ( node );
            }
        }
        break;
        case XML_TEXT_NODE:
        case XML_CDATA_SECTION_NODE:
            if ( node->parent != nullptr && xmlStrcmp ( node->parent->name, reinterpret_cast<const xmlChar * > ( "code" ) ) == 0 && !xmlIsBlankNode ( node ) )
            {
                std::cout << reinterpret_cast<const char*> ( node->content ) << std::endl;
            }
            break;
        default:
            break;
        }
        for ( xmlNodePtr child = node->children; child; child = child->next )
        {
            ProcessNode ( child );
        }
        if ( node->type == XML_ELEMENT_NODE && processor != XMLElementProcessors.end() && std::get<1> ( processor->second ) != nullptr )
        {
            std::get<1> ( processor->second ) ( node );
        }
    }

    int PipelineTool::operator() ( int argc, char** argv )
    {
        ProcessArgs ( argc, argv );
        xmlDocPtr document{xmlReadFile ( mInputFile.c_str(), nullptr, 0 ) };
        if ( document == nullptr )
        {
            xmlFreeDoc ( document );
            throw std::runtime_error ( "Error parsing XML file" );
        }
        xmlNodePtr root_element{ xmlDocGetRootElement ( document ) };
        if ( root_element == nullptr )
        {
            xmlFreeDoc ( document );
            throw std::runtime_error ( "Failed to get root element" );
        }
        else if ( xmlStrcmp ( root_element->name, BAD_CAST "pipeline" ) != 0 )
        {
            xmlFreeDoc ( document );
            throw std::runtime_error ( "Root element is not pipeline" );
        }
        ProcessNode ( root_element );
        xmlFreeDoc ( document );
        return 0;
    }
}
