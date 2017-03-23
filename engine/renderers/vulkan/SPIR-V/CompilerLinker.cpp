//
// Copyright (C) 2002-2005  3Dlabs Inc. Ltd.
// Copyright (C) 2013-2016 LunarG, Inc.
// Copyright (C) 2017 Aeon Games
//
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
//    Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//
//    Redistributions in binary form must reproduce the above
//    copyright notice, this list of conditions and the following
//    disclaimer in the documentation and/or other materials provided
//    with the distribution.
//
//    Neither the name of 3Dlabs Inc. Ltd. nor the names of its
//    contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
// FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
// COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
// ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//

// this only applies to the standalone wrapper, not the front end in general
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "ResourceLimits.h"
#include "Worklist.h"
#include "glslang/Include/ShHandle.h"
#include "glslang/Include/revision.h"
#include "SPIRV/GlslangToSpv.h"
#include "SPIRV/GLSL.std.450.h"
#include "SPIRV/doc.h"
#include "SPIRV/disassemble.h"
#include <cstring>
#include <cstdlib>
#include <cctype>
#include <cmath>
#include <array>
#include <thread>
#include "CompilerLinker.h"

//
// Forward declarations.
//
void InfoLogMsg ( const char* msg, const char* name, const int num );

const char* ExecutableName = nullptr;
const char* binaryFileName = nullptr;
const char* entryPointName = nullptr;
const char* sourceEntryPointName = nullptr;
const char* shaderStageName = nullptr;
const char* variableName = nullptr;

std::array<unsigned int, EShLangCount> baseSamplerBinding;
std::array<unsigned int, EShLangCount> baseTextureBinding;
std::array<unsigned int, EShLangCount> baseImageBinding;
std::array<unsigned int, EShLangCount> baseUboBinding;

//
// Create the default name for saving a binary if -o is not provided.
//
const char* GetBinaryName ( EShLanguage stage )
{
    const char* name;
    if ( binaryFileName == nullptr )
    {
        switch ( stage )
        {
        case EShLangVertex:
            name = "vert.spv";
            break;
        case EShLangTessControl:
            name = "tesc.spv";
            break;
        case EShLangTessEvaluation:
            name = "tese.spv";
            break;
        case EShLangGeometry:
            name = "geom.spv";
            break;
        case EShLangFragment:
            name = "frag.spv";
            break;
        case EShLangCompute:
            name = "comp.spv";
            break;
        default:
            name = "unknown";
            break;
        }
    }
    else
    {
        name = binaryFileName;
    }

    return name;
}

//
// Give error and exit with failure code.
//
void Error ( const char* message )
{
    printf ( "%s: Error %s (use -h for usage)\n", ExecutableName, message );
    exit ( EFailUsage );
}

#if 0
//
// Process an optional binding base of the form:
//   --argname [stage] base
// Where stage is one of the forms accepted by FindLanguage, and base is an integer
//
void ProcessBindingBase ( int& argc, char**& argv, std::array<unsigned int, EShLangCount>& base )
{
    if ( !isdigit ( argv[1][0] ) )
    {
        // Parse form: --argname stage base
        const EShLanguage lang = FindLanguage ( argv[1], false );
        base[lang] = atoi ( argv[2] );
        argc -= 2;
        argv += 2;
    }
    else
    {
        // Parse form: --argname base
        for ( int lang = 0; lang < EShLangCount; ++lang )
        {
            base[lang] = atoi ( argv[1] );
        }

        argc--;
        argv++;
    }
}

//
// Do all command-line argument parsing.  This includes building up the work-items
// to be processed later, and saving all the command-line options.
//
// Does not return (it exits) if command-line is fatally flawed.
//
void ProcessArguments ( int argc, char* argv[] )
{
    baseSamplerBinding.fill ( 0 );
    baseTextureBinding.fill ( 0 );
    baseImageBinding.fill ( 0 );
    baseUboBinding.fill ( 0 );

    ExecutableName = argv[0];
    NumWorkItems = argc;  // will include some empties where the '-' options were, but it doesn't matter, they'll be 0
    Work = new glslang::TWorkItem*[NumWorkItems];
    for ( int w = 0; w < NumWorkItems; ++w )
    {
        Work[w] = nullptr;
    }

    argc--;
    argv++;
    for ( ; argc >= 1; argc--, argv++ )
    {
        if ( argv[0][0] == '-' )
        {
            switch ( argv[0][1] )
            {
            case '-':
            {
                std::string lowerword ( argv[0] + 2 );
                std::transform ( lowerword.begin(), lowerword.end(), lowerword.begin(), ::tolower );

                // handle --word style options
                if ( lowerword == "shift-sampler-bindings" || // synonyms
                     lowerword == "shift-sampler-binding"  ||
                     lowerword == "ssb" )
                {
                    ProcessBindingBase ( argc, argv, baseSamplerBinding );
                }
                else if ( lowerword == "shift-texture-bindings" ||   // synonyms
                          lowerword == "shift-texture-binding"  ||
                          lowerword == "stb" )
                {
                    ProcessBindingBase ( argc, argv, baseTextureBinding );
                }
                else if ( lowerword == "shift-image-bindings" ||   // synonyms
                          lowerword == "shift-image-binding"  ||
                          lowerword == "sib" )
                {
                    ProcessBindingBase ( argc, argv, baseImageBinding );
                }
                else if ( lowerword == "shift-ubo-bindings" ||   // synonyms
                          lowerword == "shift-ubo-binding"  ||
                          lowerword == "sub" )
                {
                    ProcessBindingBase ( argc, argv, baseUboBinding );
                }
                else if ( lowerword == "auto-map-bindings" ||   // synonyms
                          lowerword == "auto-map-binding"  ||
                          lowerword == "amb" )
                {
                    Options |= EOptionAutoMapBindings;
                }
                else if ( lowerword == "flatten-uniform-arrays" ||  // synonyms
                          lowerword == "flatten-uniform-array"  ||
                          lowerword == "fua" )
                {
                    Options |= EOptionFlattenUniformArrays;
                }
                else if ( lowerword == "no-storage-format" ||  // synonyms
                          lowerword == "nsf" )
                {
                    Options |= EOptionNoStorageFormat;
                }
                else if ( lowerword == "variable-name" ||  // synonyms
                          lowerword == "vn" )
                {
                    Options |= EOptionOutputHexadecimal;
                    variableName = argv[1];
                    if ( argc > 0 )
                    {
                        argc--;
                        argv++;
                    }
                    else
                    {
                        Error ( "no <C-variable-name> provided for --variable-name" );
                    }
                    break;
                }
                else if ( lowerword == "source-entrypoint" || // synonyms
                          lowerword == "sep" )
                {
                    sourceEntryPointName = argv[1];
                    if ( argc > 0 )
                    {
                        argc--;
                        argv++;
                    }
                    else
                    {
                        Error ( "no <entry-point> provided for --source-entrypoint" );
                    }
                    break;
                }
                else if ( lowerword == "keep-uncalled" ||  // synonyms
                          lowerword == "ku" )
                {
                    Options |= EOptionKeepUncalled;
                }
            }
            break;
            case 'H':
                Options |= EOptionHumanReadableSpv;
                if ( ( Options & EOptionSpv ) == 0 )
                {
                    // default to Vulkan
                    Options |= EOptionSpv;
                    Options |= EOptionVulkanRules;
                    Options |= EOptionLinkProgram;
                }
                break;
            case 'V':
                Options |= EOptionSpv;
                Options |= EOptionVulkanRules;
                Options |= EOptionLinkProgram;
                break;
            case 'S':
                shaderStageName = argv[1];
                if ( argc > 0 )
                {
                    argc--;
                    argv++;
                }
                else
                {
                    Error ( "no <stage> specified for -S" );
                }
                break;
            case 'G':
                Options |= EOptionSpv;
                Options |= EOptionLinkProgram;
                // undo a -H default to Vulkan
                Options &= ~EOptionVulkanRules;
                break;
            case 'E':
                Options |= EOptionOutputPreprocessed;
                break;
            case 'c':
                Options |= EOptionDumpConfig;
                break;
            case 'C':
                Options |= EOptionCascadingErrors;
                break;
            case 'd':
                Options |= EOptionDefaultDesktop;
                break;
            case 'D':
                Options |= EOptionReadHlsl;
                break;
            case 'e':
                // HLSL todo: entry point handle needs much more sophistication.
                // This is okay for one compilation unit with one entry point.
                entryPointName = argv[1];
                if ( argc > 0 )
                {
                    argc--;
                    argv++;
                }
                else
                {
                    Error ( "no <entry-point> provided for -e" );
                }
                break;
            case 'i':
                Options |= EOptionIntermediate;
                break;
            case 'l':
                Options |= EOptionLinkProgram;
                break;
            case 'm':
                Options |= EOptionMemoryLeakMode;
                break;
            case 'o':
                binaryFileName = argv[1];
                if ( argc > 0 )
                {
                    argc--;
                    argv++;
                }
                else
                {
                    Error ( "no <file> provided for -o" );
                }
                break;
            case 'q':
                Options |= EOptionDumpReflection;
                break;
            case 'r':
                Options |= EOptionRelaxedErrors;
                break;
            case 's':
                Options |= EOptionSuppressInfolog;
                break;
            case 't':
#ifdef _WIN32
                Options |= EOptionMultiThreaded;
#endif
                break;
            case 'v':
                Options |= EOptionDumpVersions;
                break;
            case 'w':
                Options |= EOptionSuppressWarnings;
                break;
            case 'x':
                Options |= EOptionOutputHexadecimal;
                break;
            default:
                break;
            }
        }
        else
        {
            std::string name ( argv[0] );
            if ( ! SetConfigFile ( name ) )
            {
                Work[argc] = new glslang::TWorkItem ( name );
                Worklist.add ( Work[argc] );
            }
        }
    }

    // Make sure that -E is not specified alongside linking (which includes SPV generation)
    if ( ( Options & EOptionOutputPreprocessed ) && ( Options & EOptionLinkProgram ) )
    {
        Error ( "can't use -E when linking is selected" );
    }

    // -o or -x makes no sense if there is no target binary
    if ( binaryFileName && ( Options & EOptionSpv ) == 0 )
    {
        Error ( "no binary generation requested (e.g., -V)" );
    }

    if ( ( Options & EOptionFlattenUniformArrays ) != 0 &&
         ( Options & EOptionReadHlsl ) == 0 )
    {
        Error ( "uniform array flattening only valid when compiling HLSL source." );
    }
}
#endif

// Outputs the given string, but only if it is non-null and non-empty.
// This prevents erroneous newlines from appearing.
void PutsIfNonEmpty ( const char* str )
{
    if ( str && str[0] )
    {
        puts ( str );
    }
}

// Outputs the given string to stderr, but only if it is non-null and non-empty.
// This prevents erroneous newlines from appearing.
void StderrIfNonEmpty ( const char* str )
{
    if ( str && str[0] )
    {
        fprintf ( stderr, "%s\n", str );
    }
}
#if 0
int compile ( int argc, char* argv[] )
{
    if ( Options & EOptionDumpConfig )
    {
        printf ( "%s", glslang::GetDefaultTBuiltInResourceString().c_str() );
        if ( Worklist.empty() )
        {
            return ESuccess;
        }
    }

    if ( Options & EOptionDumpVersions )
    {
        printf ( "Glslang Version: %s %s\n", GLSLANG_REVISION, GLSLANG_DATE );
        printf ( "ESSL Version: %s\n", glslang::GetEsslVersionString() );
        printf ( "GLSL Version: %s\n", glslang::GetGlslVersionString() );
        std::string spirvVersion;
        glslang::GetSpirvVersion ( spirvVersion );
        printf ( "SPIR-V Version %s\n", spirvVersion.c_str() );
        printf ( "GLSL.std.450 Version %d, Revision %d\n", GLSLstd450Version, GLSLstd450Revision );
        printf ( "Khronos Tool ID %d\n", glslang::GetKhronosToolId() );
        printf ( "GL_KHR_vulkan_glsl version %d\n", 100 );
        printf ( "ARB_GL_gl_spirv version %d\n", 100 );
        if ( Worklist.empty() )
        {
            return ESuccess;
        }
    }

    ProcessConfigFile();

    //
    // Two modes:
    // 1) linking all arguments together, single-threaded, new C++ interface
    // 2) independent arguments, can be tackled by multiple asynchronous threads, for testing thread safety, using the old handle interface
    //
    if ( Options & EOptionLinkProgram ||
         Options & EOptionOutputPreprocessed )
    {
        for ( int w = 0; w < NumWorkItems; ++w )
        {
            if ( Work[w] )
            {
                delete Work[w];
            }
        }
    }
    else
    {
        ShInitialize();

        bool printShaderNames = Worklist.size() > 1;

        if ( Options & EOptionMultiThreaded )
        {
            const int NumThreads = 16;
            std::array<std::thread, NumThreads> threads;
            for ( auto& t : threads )
            {
                try
                {
                    t = std::thread ( CompileShaders );
                }
                catch ( std::system_error e )
                {
                    printf ( "Failed to create thread: %s\n", e.what() );
                    return EFailThreadCreate;
                }
            }
            for ( auto& t : threads )
            {
                if ( t.joinable() )
                {
                    t.join();
                }
            }
        }
        else
        {
            CompileShaders();
        }

        // Print out all the resulting infologs
        for ( int w = 0; w < NumWorkItems; ++w )
        {
            if ( Work[w] )
            {
                if ( printShaderNames || Work[w]->results.size() > 0 )
                {
                    PutsIfNonEmpty ( Work[w]->name.c_str() );
                }
                PutsIfNonEmpty ( Work[w]->results.c_str() );
                delete Work[w];
            }
        }

        ShFinalize();
    }

    delete[] Work;

    if ( CompileFailed )
    {
        return EFailCompile;
    }
    if ( LinkFailed )
    {
        return EFailLink;
    }

    return 0;
}
#endif
#if 0
//
//   Deduce the language from the filename.  Files must end in one of the
//   following extensions:
//
//   .vert = vertex
//   .tesc = tessellation control
//   .tese = tessellation evaluation
//   .geom = geometry
//   .frag = fragment
//   .comp = compute
//
EShLanguage FindLanguage ( const std::string& name, bool parseSuffix )
{
    size_t ext = 0;

    // Search for a suffix on a filename: e.g, "myfile.frag".  If given
    // the suffix directly, we skip looking the '.'
    if ( parseSuffix )
    {
        ext = name.rfind ( '.' );
        if ( ext == std::string::npos )
        {
            return EShLangVertex;
        }
        ++ext;
    }

    std::string suffix = name.substr ( ext, std::string::npos );
    if ( shaderStageName )
    {
        suffix = shaderStageName;
    }

    if ( suffix == "vert" )
    {
        return EShLangVertex;
    }
    else if ( suffix == "tesc" )
    {
        return EShLangTessControl;
    }
    else if ( suffix == "tese" )
    {
        return EShLangTessEvaluation;
    }
    else if ( suffix == "geom" )
    {
        return EShLangGeometry;
    }
    else if ( suffix == "frag" )
    {
        return EShLangFragment;
    }
    else if ( suffix == "comp" )
    {
        return EShLangCompute;
    }
    return EShLangVertex;
}

//
// Read a file's data into a string, and compile it using the old interface ShCompile,
// for non-linkable results.
//
void CompileFile ( const char* fileName, ShHandle compiler )
{
    int ret = 0;
    char** shaderStrings = ReadFileData ( fileName );
    if ( ! shaderStrings )
    {
    }

    auto  lengths = new int[NumShaderStrings];

    // move to length-based strings, rather than null-terminated strings
    for ( int s = 0; s < NumShaderStrings; ++s )
    {
        lengths[s] = ( int ) strlen ( shaderStrings[s] );
    }

    if ( ! shaderStrings )
    {
        CompileFailed = true;
        return;
    }

    EShMessages messages = EShMsgDefault;
    SetMessageOptions ( messages );

    for ( int i = 0; i < ( ( Options & EOptionMemoryLeakMode ) ? 100 : 1 ); ++i )
    {
        for ( int j = 0; j < ( ( Options & EOptionMemoryLeakMode ) ? 100 : 1 ); ++j )
        {
            // ret = ShCompile(compiler, shaderStrings, NumShaderStrings, lengths, EShOptNone, &Resources, Options, (Options & EOptionDefaultDesktop) ? 110 : 100, false, messages);
            ret = ShCompile ( compiler, shaderStrings, NumShaderStrings, nullptr, EShOptNone, &Resources, Options, ( Options & EOptionDefaultDesktop ) ? 110 : 100, false, messages );
            // const char* multi[12] = { "# ve", "rsion", " 300 e", "s", "\n#err",
            //                         "or should be l", "ine 1", "string 5\n", "float glo", "bal",
            //                         ";\n#error should be line 2\n void main() {", "global = 2.3;}" };
            // const char* multi[7] = { "/", "/", "\\", "\n", "\n", "#", "version 300 es" };
            // ret = ShCompile(compiler, multi, 7, nullptr, EShOptNone, &Resources, Options, (Options & EOptionDefaultDesktop) ? 110 : 100, false, messages);
        }
    }

    delete [] lengths;
    FreeFileData ( shaderStrings );

    if ( ret == 0 )
    {
        CompileFailed = true;
    }
}

//
//   print usage to stdout
//
void usage()
{
    printf ( "Usage: glslangValidator [option]... [file]...\n"
             "\n"
             "Where: each 'file' ends in .<stage>, where <stage> is one of\n"
             "    .conf   to provide an optional config file that replaces the default configuration\n"
             "            (see -c option below for generating a template)\n"
             "    .vert   for a vertex shader\n"
             "    .tesc   for a tessellation control shader\n"
             "    .tese   for a tessellation evaluation shader\n"
             "    .geom   for a geometry shader\n"
             "    .frag   for a fragment shader\n"
             "    .comp   for a compute shader\n"
             "\n"
             "Compilation warnings and errors will be printed to stdout.\n"
             "\n"
             "To get other information, use one of the following options:\n"
             "Each option must be specified separately.\n"
             "  -V          create SPIR-V binary, under Vulkan semantics; turns on -l;\n"
             "              default file name is <stage>.spv (-o overrides this)\n"
             "  -G          create SPIR-V binary, under OpenGL semantics; turns on -l;\n"
             "              default file name is <stage>.spv (-o overrides this)\n"
             "  -H          print human readable form of SPIR-V; turns on -V\n"
             "  -E          print pre-processed GLSL; cannot be used with -l;\n"
             "              errors will appear on stderr.\n"
             "  -S <stage>  uses explicit stage specified, rather then the file extension.\n"
             "              valid choices are vert, tesc, tese, geom, frag, or comp\n"
             "  -c          configuration dump;\n"
             "              creates the default configuration file (redirect to a .conf file)\n"
             "  -C          cascading errors; risks crashes from accumulation of error recoveries\n"
             "  -d          default to desktop (#version 110) when there is no shader #version\n"
             "              (default is ES version 100)\n"
             "  -D          input is HLSL\n"
             "  -e          specify entry-point name\n"
             "  -h          print this usage message\n"
             "  -i          intermediate tree (glslang AST) is printed out\n"
             "  -l          link all input files together to form a single module\n"
             "  -m          memory leak mode\n"
             "  -o  <file>  save binary to <file>, requires a binary option (e.g., -V)\n"
             "  -q          dump reflection query database\n"
             "  -r          relaxed semantic error-checking mode\n"
             "  -s          silent mode\n"
             "  -t          multi-threaded mode\n"
             "  -v          print version strings\n"
             "  -w          suppress warnings (except as required by #extension : warn)\n"
             "  -x          save 32-bit hexadecimal numbers as text, requires a binary option (e.g., -V)\n"
             "\n"
             "  --shift-sampler-binding [stage] num     set base binding number for samplers\n"
             "  --ssb [stage] num                       synonym for --shift-sampler-binding\n"
             "\n"
             "  --shift-texture-binding [stage] num     set base binding number for textures\n"
             "  --stb [stage] num                       synonym for --shift-texture-binding\n"
             "\n"
             "  --shift-image-binding [stage] num       set base binding number for images (uav)\n"
             "  --sib [stage] num                       synonym for --shift-image-binding\n"
             "\n"
             "  --shift-UBO-binding [stage] num         set base binding number for UBOs\n"
             "  --sub [stage] num                       synonym for --shift-UBO-binding\n"
             "\n"
             "  --auto-map-bindings                     automatically bind uniform variables without\n"
             "                                          explicit bindings.\n"
             "  --amb                                   synonym for --auto-map-bindings\n"
             "\n"
             "  --flatten-uniform-arrays                flatten uniform texture & sampler arrays to scalars\n"
             "  --fua                                   synonym for --flatten-uniform-arrays\n"
             "\n"
             "  --no-storage-format                     use Unknown image format\n"
             "  --nsf                                   synonym for --no-storage-format\n"
             "\n"
             "  --source-entrypoint name                the given shader source function is renamed to be the entry point given in -e\n"
             "  --sep                                   synonym for --source-entrypoint\n"
             "\n"
             "  --keep-uncalled                         don't eliminate uncalled functions when linking\n"
             "  --ku                                    synonym for --keep-uncalled\n"
             "  --variable-name <name>                  Creates a C header file that contains a uint32_t array named <name> initialized with the shader binary code.\n"
             "  --vn <name>                             synonym for --variable-name <name>.\n"
           );

    exit ( EFailUsage );
}
#endif

#if !defined _MSC_VER && !defined MINGW_HAS_SECURE_API

#include <errno.h>

int fopen_s (
    FILE** pFile,
    const char* filename,
    const char* mode
)
{
    if ( !pFile || !filename || !mode )
    {
        return EINVAL;
    }

    FILE* f = fopen ( filename, mode );
    if ( ! f )
    {
        if ( errno != 0 )
        {
            return errno;
        }
        else
        {
            return ENOENT;
        }
    }
    *pFile = f;

    return 0;
}

#endif

void InfoLogMsg ( const char* msg, const char* name, const int num )
{
    if ( num >= 0 )
    {
        printf ( "#### %s %s %d INFO LOG ####\n", msg, name, num );
    }
    else
    {
        printf ( "#### %s %s INFO LOG ####\n", msg, name );
    }
}

namespace AeonGames
{
    CompilerLinker::CompilerLinker ( TOptions aOptions ) : mOptions ( aOptions )
    {
        // Make sure that -E is not specified alongside linking (which includes SPV generation)
        if ( ( mOptions & EOptionOutputPreprocessed ) && ( mOptions & EOptionLinkProgram ) )
        {
            Error ( "can't use -E when linking is selected" );
        }

        // -o or -x makes no sense if there is no target binary
        if ( binaryFileName && ( mOptions & EOptionSpv ) == 0 )
        {
            Error ( "no binary generation requested (e.g., -V)" );
        }

        if ( ( mOptions & EOptionFlattenUniformArrays ) != 0 &&
             ( mOptions & EOptionReadHlsl ) == 0 )
        {
            Error ( "uniform array flattening only valid when compiling HLSL source." );
        }
    }

    CompilerLinker::~CompilerLinker()
    {
    }

    void CompilerLinker::AddShaderSource ( EShLanguage aLanguage, const char* aSource )
    {
        mShaderCompilationUnits[aLanguage] = aSource;
    }

    void CompilerLinker::RemoveShaderSource ( EShLanguage aLanguage )
    {
        mShaderCompilationUnits[aLanguage] = nullptr;
    }

    //
    // Translate the meaningful subset of command-line options to parser-behavior options.
    //
    void CompilerLinker::SetMessageOptions ( EShMessages& messages ) const
    {
        if ( mOptions & EOptionRelaxedErrors )
        {
            messages = ( EShMessages ) ( messages | EShMsgRelaxedErrors );
        }
        if ( mOptions & EOptionIntermediate )
        {
            messages = ( EShMessages ) ( messages | EShMsgAST );
        }
        if ( mOptions & EOptionSuppressWarnings )
        {
            messages = ( EShMessages ) ( messages | EShMsgSuppressWarnings );
        }
        if ( mOptions & EOptionSpv )
        {
            messages = ( EShMessages ) ( messages | EShMsgSpvRules );
        }
        if ( mOptions & EOptionVulkanRules )
        {
            messages = ( EShMessages ) ( messages | EShMsgVulkanRules );
        }
        if ( mOptions & EOptionOutputPreprocessed )
        {
            messages = ( EShMessages ) ( messages | EShMsgOnlyPreprocessor );
        }
        if ( mOptions & EOptionReadHlsl )
        {
            messages = ( EShMessages ) ( messages | EShMsgReadHlsl );
        }
        if ( mOptions & EOptionCascadingErrors )
        {
            messages = ( EShMessages ) ( messages | EShMsgCascadingErrors );
        }
        if ( mOptions & EOptionKeepUncalled )
        {
            messages = ( EShMessages ) ( messages | EShMsgKeepUncalled );
        }
    }

    //
    // For linking mode: Will independently parse each compilation unit, but then put them
    // in the same program and link them together, making at most one linked module per
    // pipeline stage.
    //
    // Uses the new C++ interface instead of the old handle-based interface.
    //
    void CompilerLinker::CompileAndLink()
    {
        glslang::InitializeProcess();
        // keep track of what to free
        std::list<glslang::TShader*> shaders;

        EShMessages messages = EShMsgDefault;
        SetMessageOptions ( messages );

        //
        // Per-shader processing...
        //

        glslang::TProgram& program = *new glslang::TProgram;
        for ( size_t i = 0; i < mShaderCompilationUnits.size(); ++i )
        {
            if ( !mShaderCompilationUnits[i] )
            {
                continue;
            }
            auto  shader = new glslang::TShader ( static_cast<EShLanguage> ( i ) );
            std::array<const char*, 1> source{ mShaderCompilationUnits[i] };
            shader->setStringsWithLengthsAndNames ( source.data(), nullptr, nullptr, 0 );
            if ( entryPointName ) // HLSL todo: this needs to be tracked per compUnits
            {
                shader->setEntryPoint ( entryPointName );
            }
            if ( sourceEntryPointName )
            {
                shader->setSourceEntryPoint ( sourceEntryPointName );
            }

            shader->setShiftSamplerBinding ( baseSamplerBinding[i] );
            shader->setShiftTextureBinding ( baseTextureBinding[i] );
            shader->setShiftImageBinding ( baseImageBinding[i] );
            shader->setShiftUboBinding ( baseUboBinding[i] );
            shader->setFlattenUniformArrays ( ( mOptions & EOptionFlattenUniformArrays ) != 0 );
            shader->setNoStorageFormat ( ( mOptions & EOptionNoStorageFormat ) != 0 );

            if ( mOptions & EOptionAutoMapBindings )
            {
                shader->setAutoMapBindings ( true );
            }

            shaders.push_back ( shader );

            const int defaultVersion = mOptions & EOptionDefaultDesktop ? 110 : 100;

            if ( mOptions & EOptionOutputPreprocessed )
            {
                std::string str;
                glslang::TShader::ForbidIncluder includer;
                if ( shader->preprocess ( &glslang::DefaultTBuiltInResource, defaultVersion, ENoProfile, false, false,
                                          messages, &str, includer ) )
                {
                    PutsIfNonEmpty ( str.c_str() );
                }
                else
                {
                    mCompileFailed = true;
                }
                StderrIfNonEmpty ( shader->getInfoLog() );
                StderrIfNonEmpty ( shader->getInfoDebugLog() );
                continue;
            }
            if ( !shader->parse ( &glslang::DefaultTBuiltInResource, defaultVersion, false, messages ) )
            {
                mCompileFailed = true;
            }

            program.addShader ( shader );

            if ( ! ( mOptions & EOptionSuppressInfolog ) &&
                 ! ( mOptions & EOptionMemoryLeakMode ) )
            {
                PutsIfNonEmpty ( shader->getInfoLog() );
                PutsIfNonEmpty ( shader->getInfoDebugLog() );
            }
        }

        //
        // Program-level processing...
        //

        // Link
        if ( ! ( mOptions & EOptionOutputPreprocessed ) && !program.link ( messages ) )
        {
            mLinkFailed = true;
        }

        // Map IO
        if ( mOptions & EOptionSpv )
        {
            if ( !program.mapIO() )
            {
                mLinkFailed = true;
            }
        }

        // Report
        if ( ! ( mOptions & EOptionSuppressInfolog ) &&
             ! ( mOptions & EOptionMemoryLeakMode ) )
        {
            PutsIfNonEmpty ( program.getInfoLog() );
            PutsIfNonEmpty ( program.getInfoDebugLog() );
        }

        // Reflect
        if ( mOptions & EOptionDumpReflection )
        {
            program.buildReflection();
            program.dumpReflection();
        }

        // Dump SPIR-V
        if ( mOptions & EOptionSpv )
        {
            if ( mCompileFailed || mLinkFailed )
            {
                printf ( "SPIR-V is not generated for failed compile or link\n" );
            }
            else
            {
                for ( int stage = 0; stage < EShLangCount; ++stage )
                {
                    if ( program.getIntermediate ( ( EShLanguage ) stage ) )
                    {
                        std::vector<unsigned int> spirv;
                        std::string warningsErrors;
                        spv::SpvBuildLogger logger;
                        glslang::GlslangToSpv ( *program.getIntermediate ( ( EShLanguage ) stage ), spirv, &logger );

                        // Dump the spv to a file or stdout, etc., but only if not doing
                        // memory/perf testing, as it's not internal to programmatic use.
                        if ( ! ( mOptions & EOptionMemoryLeakMode ) )
                        {
                            printf ( "%s", logger.getAllMessages().c_str() );
                            if ( mOptions & EOptionOutputHexadecimal )
                            {
                                glslang::OutputSpvHex ( spirv, GetBinaryName ( ( EShLanguage ) stage ), variableName );
                            }
                            else
                            {
                                glslang::OutputSpvBin ( spirv, GetBinaryName ( ( EShLanguage ) stage ) );
                            }
                            if ( mOptions & EOptionHumanReadableSpv )
                            {
                                spv::Disassemble ( std::cout, spirv );
                            }
                        }
                    }
                }
            }
        }
        // Free everything up, program has to go before the shaders
        // because it might have merged stuff from the shaders, and
        // the stuff from the shaders has to have its destructors called
        // before the pools holding the memory in the shaders is freed.
        delete &program;
        while ( shaders.size() > 0 )
        {
            delete shaders.back();
            shaders.pop_back();
        }
        glslang::FinalizeProcess();
    }

    bool CompilerLinker::CompileFailed() const
    {
        return mCompileFailed;
    }

    bool CompilerLinker::LinkFailed() const
    {
        return mLinkFailed;
    }
}

