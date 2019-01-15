/*
Copyright (C) 2017-2019 Rodrigo Jose Hernandez Cordoba

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
#include "GridSettings.h"
#include "aeongames/Pipeline.h"
#include "aeongames/Mesh.h"
#include "aeongames/Material.h"
#include "aeongames/ResourceId.h"
Q_DECLARE_METATYPE ( AeonGames::ResourceId );

#define qWorldEditorApp (reinterpret_cast<WorldEditor*> ( qApp ))

namespace AeonGames
{
    class Renderer;
    class WorldEditor : public QApplication
    {
    public:
        WorldEditor ( int &argc, char *argv[] );
        ~WorldEditor() final;
        bool notify ( QObject *receiver, QEvent *event ) override;
        const GridSettings& GetGridSettings() const;
        const Pipeline& GetGridPipeline() const;
        const Mesh& GetGridMesh() const;
        const Material& GetXGridMaterial() const;
        const Material& GetYGridMaterial() const;
        const Pipeline& GetWirePipeline() const;
        const Mesh& GetAABBWireMesh() const;
        bool IsBlocked() const;
    private:
        int mResourceIdMetaType{};
        bool mIsBlocked{false};
        QMutex mMutex{};
        GridSettings mGridSettings{};
        std::unique_ptr<Pipeline> mGridPipeline{};
        std::unique_ptr<Mesh> mGridMesh{};
        std::unique_ptr<Material> mXGridMaterial{};
        std::unique_ptr<Material> mYGridMaterial{};
        std::unique_ptr<Pipeline> mWirePipeline{};
        std::unique_ptr<Mesh> mAABBWireMesh{};
    };
}
#endif
