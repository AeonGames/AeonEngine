//
// Copyright (C) 2002-2005  3Dlabs Inc. Ltd.
// Copyright (C) 2013-2016 LunarG, Inc.
// Copyright (C) 2017-2018 Aeon Games
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

namespace AeonGames
{
    CompilerLinker::CompilerLinker ( TOptions aOptions ) : mOptions ( aOptions )
    {
        // Make sure that -E is not specified alongside linking (which includes SPV generation)
        if ( ( mOptions & EOptionOutputPreprocessed ) && ( mOptions & EOptionLinkProgram ) )
        {
            throw std::runtime_error ( "can't use -E when linking is selected" );
        }

        if ( ( mOptions & EOptionFlattenUniformArrays ) != 0 &&
             ( mOptions & EOptionReadHlsl ) == 0 )
        {
            throw std::runtime_error ( "uniform array flattening only valid when compiling HLSL source." );
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

    const char * CompilerLinker::GetStageName ( EShLanguage aStage ) const
    {
        switch ( aStage )
        {
        case EShLangVertex:
            return "Vertex";
        case EShLangTessControl:
            return "TessControl";
        case EShLangTessEvaluation:
            return "TessEvaluation";
        case EShLangGeometry:
            return "Geometry";
        case EShLangFragment:
            return "Fragment";
        case EShLangCompute:
            return "Compute";
        case EShLangCount:
            break;
        }
        return "Invalid Stage";
    }

    //
    // For linking mode: Will independently parse each compilation unit, but then put them
    // in the same program and link them together, making at most one linked module per
    // pipeline stage.
    //
    // Uses the new C++ interface instead of the old handle-based interface.
    //
    CompilerLinker::FailCode CompilerLinker::CompileAndLink()
    {
        bool compile_failed = false;
        bool link_failed = false;
        mLog.clear();
        /*  The original code used heap memory for program and shaders
            in order to destroy the program first and the shaders last,
            that shouldn't be necesary with this new approach,
            keep the order of declaration of the following two locals
            to ensure program is destroyed before shaders. */
        std::vector<glslang::TShader> shaders;
        glslang::TProgram program;
        EShMessages messages = EShMsgDefault;
        SetMessageOptions ( messages );
        //
        // Per-shader processing...
        //
        shaders.reserve ( mShaderCompilationUnits.size() );
        for ( size_t i = 0; i < mShaderCompilationUnits.size(); ++i )
        {
            if ( !mShaderCompilationUnits[i] )
            {
                continue;
            }
            shaders.emplace_back ( static_cast<EShLanguage> ( i ) );
            std::array<const char*, 1> source{ {mShaderCompilationUnits[i]} };
            auto stage_name = GetStageName ( static_cast<EShLanguage> ( i ) );
            shaders.back().setStringsWithLengthsAndNames ( source.data(), nullptr, &stage_name, 1 );
            shaders.back().setShiftSamplerBinding ( mBaseSamplerBinding[i] );
            shaders.back().setShiftTextureBinding ( mBaseTextureBinding[i] );
            shaders.back().setShiftImageBinding ( mBaseImageBinding[i] );
            shaders.back().setShiftUboBinding ( mBaseUboBinding[i] );
            shaders.back().setFlattenUniformArrays ( ( mOptions & EOptionFlattenUniformArrays ) != 0 );
            shaders.back().setNoStorageFormat ( ( mOptions & EOptionNoStorageFormat ) != 0 );
            if ( mOptions & EOptionAutoMapBindings )
            {
                shaders.back().setAutoMapBindings ( true );
            }
            const int defaultVersion = mOptions & EOptionDefaultDesktop ? 110 : 100;
            if ( !shaders.back().parse ( &glslang::DefaultTBuiltInResource, defaultVersion, false, messages ) )
            {
                compile_failed = true;
            }
        }
        /* We have to add the shaders
        AFTER we know the shaders array
        is no longer going be shifted around,
        IE: When we're not going to be adding any more to it. */
        for ( auto& i : shaders )
        {
            program.addShader ( &i );
        }
        //
        // Program-level processing...
        //
        // Link
        // Map IO
        if ( ( ! ( mOptions & EOptionOutputPreprocessed ) && !program.link ( messages ) ) ||
             ( ( mOptions & EOptionSpv ) && ( !program.mapIO() ) ) )
        {
            link_failed = true;
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
            if ( compile_failed || link_failed )
            {
                for ( auto& i : shaders )
                {
                    mLog.append ( i.getInfoLog() );
                    mLog.append ( i.getInfoDebugLog() );
                }
                mLog.append ( program.getInfoLog() );
                mLog.append ( program.getInfoDebugLog() );
            }
            else
            {
                for ( int stage = 0; stage < EShLangCount; ++stage )
                {
                    if ( program.getIntermediate ( ( EShLanguage ) stage ) )
                    {
                        mSpirV[stage].clear();
                        std::string warningsErrors;
                        spv::SpvBuildLogger logger;
                        glslang::GlslangToSpv ( *program.getIntermediate ( ( EShLanguage ) stage ), mSpirV[stage], &logger );
                    }
                }
            }
        }
        if ( compile_failed )
        {
            return EFailCompile;
        }
        else if ( link_failed )
        {
            return EFailLink;
        }
        return ESuccess;
    }

    const std::vector<uint32_t>& CompilerLinker::GetSpirV ( EShLanguage aStage ) const
    {
        return mSpirV[aStage];
    }

    const std::string & CompilerLinker::GetLog() const
    {
        return mLog;
    }
}

