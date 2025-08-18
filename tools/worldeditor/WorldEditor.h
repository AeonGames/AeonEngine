/*
Copyright (C) 2017-2019,2021,2022,2025 Rodrigo Jose Hernandez Cordoba

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
    class WorldEditor final : public QApplication
    {
    public:
        WorldEditor ( int &argc, char *argv[] );
        ~WorldEditor() final;
        const Pipeline& GetGridPipeline() const;
        const Mesh& GetGridMesh() const;
        const Material& GetXGridMaterial() const;
        const Material& GetYGridMaterial() const;
        const Pipeline& GetSolidColorPipeline() const;
        const Material& GetSolidColorMaterial() const;
        const Mesh& GetAABBWireMesh() const;
        int GetStringIdMetaType() const;
        int GetStringMetaType() const;
        int GetPathMetaType() const;
        void AttachWindowToRenderer ( void* aWindow );
        void DetachWindowFromRenderer ( void* aWindow );
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
