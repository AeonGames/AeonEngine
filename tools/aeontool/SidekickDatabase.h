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
#ifndef AEONGAMES_SIDEKICKDATABASE_H
#define AEONGAMES_SIDEKICKDATABASE_H
#include <string>
#include <utility>
#include <vector>
#include "Tool.h"

namespace AeonGames
{
    /** @brief Transcodes a Synty SIDEKICK database into the engine's character
        library schema.

        The Sidekick database describes every part, outfit and palette Synty
        ships, keyed by its own table and column names. Rather than teach the
        runtime that schema, this rewrites the parts the engine cares about into
        tables the engine owns, so a character library can later be fed from
        somewhere other than Synty and can live alongside world and player data
        in the same database.

        Parts are matched against the cooked meshes on disk: a part with no mesh
        is dropped, and so is any outfit that needed it, which is what keeps the
        runtime from offering a character it cannot build. */
    class SidekickDatabase : public Tool
    {
    public:
        /** @brief Default constructor. */
        SidekickDatabase();
        /** @brief Destructor. */
        ~SidekickDatabase() override;
        /**
         * @brief Execute the transcode.
         * @param argc Argument count.
         * @param argv Argument vector.
         * @return Exit status code.
         */
        int operator() ( int argc, char** argv ) override;
    private:
        /** @return true when the arguments requested help and no work should run. */
        bool ProcessArgs ( int argc, char** argv );
        std::string mInputFile{};
        std::string mOutputFile{};
        std::string mMeshPath{};
        std::string mResourcePath{"sidekick/parts"};
        std::string mMeshExtension{"msh"};
        std::string mSkeleton{"sidekick/parts/skeleton.skl"};
        std::string mPipeline{"shaders/clustered_phong"};
        std::string mMaterial{"sidekick/parts/character.mtl"};
        std::vector<std::pair<std::string, std::string>> mAnimations{};
    };
}
#endif
