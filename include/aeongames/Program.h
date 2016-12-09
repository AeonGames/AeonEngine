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
#ifndef AEONGAMES_PROGRAM_H
#define AEONGAMES_PROGRAM_H

#include <cstdint>
#include <string>
#include "Uniform.h"
#include "aeongames/Platform.h"

namespace AeonGames
{
    class Program
    {
    public:
        Program ( const std::string& aFilename );
        ~Program();
        DLL const std::string& GetVertexShaderSource() const;
        DLL const std::string& GetFragmentShaderSource() const;
        DLL const std::vector<Uniform>& GetUniformMetaData() const;
    private:
        void Initialize();
        void Finalize();
        std::string mFilename;
        std::string mVertexShader;
        std::string mFragmentShader;
        std::vector<Uniform> mUniformMetaData;
    };
}
#endif