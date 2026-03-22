/*
Copyright (C) 2013,2018,2025,2026 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_PACK_H
#define AEONGAMES_PACK_H
#include <string>
#include <unordered_map>
#include <cstdint>
#include "Tool.h"
#include "aeongames/Package.hpp"

namespace AeonGames
{
    /** @brief Tool for packing and unpacking game asset packages. */
    class Pack : public Tool
    {
    public:
        /** @brief Specifies the pack operation to perform. */
        enum Action
        {
            None = 0,   /**< No action. */
            Extract,    /**< Extract assets from a package. */
            Compress,   /**< Compress assets into a package. */
            Directory   /**< List the directory of a package. */
        };
        /** @brief Default constructor. */
        Pack();
        /** @brief Destructor. */
        ~Pack();
        /**
         * @brief Execute the pack tool.
         * @param argc Argument count.
         * @param argv Argument vector.
         * @return Exit status code.
         */
        int operator() ( int argc, char** argv ) override;
    private:
        void ProcessArgs ( int argc, char** argv );
        bool ProcessDirectory ( const std::string& path );
        int ExecCompress() const;
        int ExecExtract() const;
        int ExecDirectory() const;
        std::unordered_map<uint32_t, PKGDirectoryEntry> mDirectory;
        std::unordered_map<uint32_t, std::string> mStringTable;
        Action mAction{};
        std::string mRootPath;
        std::string mBaseName;
        std::string mInputPath;
        std::string mOutputFile;
    };
}
#endif
