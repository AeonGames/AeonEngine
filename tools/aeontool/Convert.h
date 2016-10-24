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
#ifndef AEONGAMES_CONVERT_H
#define AEONGAMES_CONVERT_H
#include <string>
#include <stdexcept>

namespace AeonGames
{
    class Convert
    {
    public:
        Convert ( int argc, char** argv );
        ~Convert();
        int Run();
    private:
        enum class FileType
        {
            UNKNOWN = 0,
            AEONPRGB,
            AEONPRGT,
            AEONMTLB,
            AEONMTLT,
            AEONMSHB,
            AEONMSHT,
        };
        FileType GetFileType ( const char* aMagic ) const;
        std::string mInputFile;
        std::string mOutputFile;
    };
}
#endif