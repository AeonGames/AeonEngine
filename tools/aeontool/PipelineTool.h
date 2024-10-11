/*
Copyright (C) 2018,2024 Rodrigo Jose Hernandez Cordoba

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
#include <libxml/tree.h>
#include <libxml/parser.h>
#include "Tool.h"

namespace AeonGames
{
    class PipelineTool : public Tool
    {
    public:
        PipelineTool();
        ~PipelineTool();
        int operator() ( int argc, char** argv ) override;
    private:
        void ProcessArgs ( int argc, char** argv );
        void ProcessNode ( xmlNodePtr aNode );
        std::string mInputFile;
        std::string mOutputFile;
        static const std::unordered_map<std::string_view, std::function<void ( xmlNodePtr ) >> XMLNodeProcessors;
    };
}
#endif
