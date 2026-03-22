/*
Copyright (C) 2017-2019,2021,2022,2025,2026 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_WORLDEDITOR_H
#define AEONGAMES_WORLDEDITOR_H

#include <QMutex>
#include <QApplication>
#include <QSettings>
#include <string>
#include <filesystem>
#include "aeongames/Pipeline.hpp"
#include "aeongames/Mesh.hpp"
#include "aeongames/Material.hpp"
#include "aeongames/Renderer.hpp"
#include "aeongames/StringId.hpp"
Q_DECLARE_METATYPE ( AeonGames::StringId );
Q_DECLARE_METATYPE ( std::string );
Q_DECLARE_METATYPE ( std::filesystem::path );

#define qWorldEditorApp (reinterpret_cast<WorldEditor*> ( qApp ))

namespace AeonGames
{
    /** @brief Main application class for the world editor. */
    class WorldEditor final : public QApplication
    {
    public:
        /**
         * @brief Construct the world editor application.
         * @param argc Argument count.
         * @param argv Argument vector.
         */
        WorldEditor ( int &argc, char *argv[] );
        /** @brief Destructor. */
        ~WorldEditor() final;
        /** @brief Get the grid rendering pipeline.
         *  @return Reference to the grid pipeline. */
        const Pipeline& GetGridPipeline() const;
        /** @brief Get the grid mesh.
         *  @return Reference to the grid mesh. */
        const Mesh& GetGridMesh() const;
        /** @brief Get the X-axis grid material.
         *  @return Reference to the X grid material. */
        const Material& GetXGridMaterial() const;
        /** @brief Get the Y-axis grid material.
         *  @return Reference to the Y grid material. */
        const Material& GetYGridMaterial() const;
        /** @brief Get the solid color rendering pipeline.
         *  @return Reference to the solid color pipeline. */
        const Pipeline& GetSolidColorPipeline() const;
        /** @brief Get the solid color material.
         *  @return Reference to the solid color material. */
        const Material& GetSolidColorMaterial() const;
        /** @brief Get the axis-aligned bounding box wireframe mesh.
         *  @return Reference to the AABB wire mesh. */
        const Mesh& GetAABBWireMesh() const;
        /** @brief Get the Qt meta-type ID for StringId.
         *  @return Meta-type identifier. */
        int GetStringIdMetaType() const;
        /** @brief Get the Qt meta-type ID for std::string.
         *  @return Meta-type identifier. */
        int GetStringMetaType() const;
        /** @brief Get the Qt meta-type ID for std::filesystem::path.
         *  @return Meta-type identifier. */
        int GetPathMetaType() const;
        /**
         * @brief Attach a native window to the renderer.
         * @param aWindow Native window handle.
         */
        void AttachWindowToRenderer ( void* aWindow );
        /**
         * @brief Detach a native window from the renderer.
         * @param aWindow Native window handle.
         */
        void DetachWindowFromRenderer ( void* aWindow );
        /** @brief Get the current renderer.
         *  @return Pointer to the renderer, or nullptr if none. */
        Renderer* GetRenderer();
    private:
        int mStringIdMetaType{};
        int mStringMetaType{};
        int mPathMetaType{};
        QMutex mMutex{};
        std::string mRendererName{}; ///< @todo Multi-renderer support
        std::unique_ptr<Renderer> mRenderer{};
        std::unique_ptr<Pipeline> mGridPipeline{};
        std::unique_ptr<Mesh> mGridMesh{};
        std::unique_ptr<Material> mXGridMaterial{};
        std::unique_ptr<Material> mYGridMaterial{};
        std::unique_ptr<Pipeline> mSolidColorPipeline{};
        std::unique_ptr<Material> mSolidColorMaterial{};
        std::unique_ptr<Mesh> mAABBWireMesh{};
    };
}
#endif
