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
#ifndef AEONGAMES_SIDEKICK_H
#define AEONGAMES_SIDEKICK_H
#include <string>
#include "Tool.h"

namespace AeonGames
{
    /** @brief Tool for converting a Synty SIDEKICK character recipe into
        AeonEngine assets.

        Takes a Sidekick @c .sk recipe plus the Sidekick SQLite database and
        emits a model (AEONMDL), a material (AEONMTL) and the baked 32x32
        palette textures the Sidekick meshes sample through their first UV set.

        Each colour property listed in the recipe occupies a 2x2 texel block at
        @c (u*2,v*2) of the palette, where @c (u,v) comes from the database's
        @c sk_color_property table and the texture origin is bottom-left.

        Geometry is not handled here: aeontool cannot read FBX. The mesh and
        skeleton assets the emitted model references are produced by the Blender
        exporters (see @c tools/blender/sidekick_batch.py). */
    class Sidekick : public Tool
    {
    public:
        /** @brief Default constructor. */
        Sidekick();
        /** @brief Destructor. */
        ~Sidekick() override;
        /**
         * @brief Execute the sidekick tool.
         * @param argc Argument count.
         * @param argv Argument vector.
         * @return Exit status code.
         */
        int operator() ( int argc, char** argv ) override;
    private:
        /** @return true when the arguments requested help and no work should run. */
        bool ProcessArgs ( int argc, char** argv );
        std::string mInputFile{};
        std::string mDatabaseFile{};
        std::string mOutputPath{"."};
        std::string mResourcePath{};
        std::string mBaseTexturePath{};
        std::string mPipeline{"shaders/clustered_phong"};
        std::string mMeshExtension{"msh"};
        std::string mSkeletonPath{};
        std::string mName{};
        bool mBinary{false};
    };
}
#endif
