/*
Copyright (C) 2018,2024,2025,2026 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_PIPELINETOOL_H
#define AEONGAMES_PIPELINETOOL_H
#include <string>
#include <string_view>
#include <stdexcept>
#include <unordered_map>
#include <functional>
#include <tuple>
#include <vector>
#include <utility>
#include "Tool.h"

namespace AeonGames
{
    /** @brief Tool for processing pipeline asset files. */
    class PipelineTool : public Tool
    {
    public:
        /** @brief Default constructor. */
        PipelineTool();
        /** @brief Destructor. */
        ~PipelineTool();
        /**
         * @brief Execute the pipeline tool.
         * @param argc Argument count.
         * @param argv Argument vector.
         * @return Exit status code.
         */
        int operator() ( int argc, char** argv ) override;
    private:
        bool ProcessArgs ( int argc, char** argv );
        std::string mInputFile;
        std::string mOutputFile;
        /// Per-stage shader source file overrides as (extension, path) pairs in
        /// command-line order. Order matters for compute stages, which may appear
        /// multiple times (".comp").
        std::vector<std::pair<std::string, std::string>> mStageFiles;
    };
}
#endif
