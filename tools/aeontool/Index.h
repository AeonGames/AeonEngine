/*
Copyright (C) 2026 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_INDEX_H
#define AEONGAMES_INDEX_H
#include <string>
#include "Tool.h"

namespace AeonGames
{
    /** @brief Tool for generating a CRC32 -> path index file from a cooked
        game asset folder (defaults to a folder named 'game').

        The generated index contains one entry per file under the root
        folder, each entry being the file's CRC32 (computed over the path
        relative to the root, using forward slashes) followed by the
        relative path string. */
    class Index : public Tool
    {
    public:
        /** @brief Default constructor. */
        Index();
        /** @brief Destructor. */
        ~Index() override;
        /**
         * @brief Execute the index tool.
         * @param argc Argument count.
         * @param argv Argument vector.
         * @return Exit status code.
         */
        int operator() ( int argc, char** argv ) override;
    private:
        void ProcessArgs ( int argc, char** argv );
        std::string mRootPath{"game"};
        std::string mOutputFile{};
        bool mBinary{false};
    };
}
#endif
