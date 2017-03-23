/*
Copyright (C) 2017 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_COMPILERLINKER_H
#define AEONGAMES_COMPILERLINKER_H

#include <array>
#include <string>
#include "glslang/Public/ShaderLang.h"

// Command-line options
enum TOptions
{
    EOptionNone = 0,
    EOptionIntermediate = ( 1 << 0 ),
    EOptionSuppressInfolog = ( 1 << 1 ),
    EOptionMemoryLeakMode = ( 1 << 2 ),
    EOptionRelaxedErrors = ( 1 << 3 ),
    EOptionGiveWarnings = ( 1 << 4 ),
    EOptionLinkProgram = ( 1 << 5 ),
    EOptionMultiThreaded = ( 1 << 6 ),
    EOptionDumpConfig = ( 1 << 7 ),
    EOptionDumpReflection = ( 1 << 8 ),
    EOptionSuppressWarnings = ( 1 << 9 ),
    EOptionDumpVersions = ( 1 << 10 ),
    EOptionSpv = ( 1 << 11 ),
    EOptionHumanReadableSpv = ( 1 << 12 ),
    EOptionVulkanRules = ( 1 << 13 ),
    EOptionDefaultDesktop = ( 1 << 14 ),
    EOptionOutputPreprocessed = ( 1 << 15 ),
    EOptionOutputHexadecimal = ( 1 << 16 ),
    EOptionReadHlsl = ( 1 << 17 ),
    EOptionCascadingErrors = ( 1 << 18 ),
    EOptionAutoMapBindings = ( 1 << 19 ),
    EOptionFlattenUniformArrays = ( 1 << 20 ),
    EOptionNoStorageFormat = ( 1 << 21 ),
    EOptionKeepUncalled = ( 1 << 22 ),
};

//
// Return codes from main/exit().
//
enum TFailCode
{
    ESuccess = 0,
    EFailUsage,
    EFailCompile,
    EFailLink,
    EFailCompilerCreate,
    EFailThreadCreate,
    EFailLinkerCreate
};

namespace AeonGames
{
    class CompilerLinker
    {
    public:
        CompilerLinker ( TOptions aOptions );
        ~CompilerLinker();
        void AddShaderSource ( EShLanguage aLanguage, const char* aSource );
        void RemoveShaderSource ( EShLanguage aLanguage );
        void CompileAndLink();
        bool CompileFailed() const;
        bool LinkFailed() const;
    private:
        void SetMessageOptions ( EShMessages& messages ) const;
        TOptions mOptions;
        bool mCompileFailed = false;
        bool mLinkFailed = false;
        /**/
        std::array<const char*, EShLangCount> mShaderCompilationUnits;
    };
}
#endif