/*
Copyright (C) 2015,2018,2019 Rodrigo Jose Hernandez Cordoba

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

#ifndef AEONGAMES_SCENEMODEL_H
#define AEONGAMES_SCENEMODEL_H
#include <QAbstractItemModel>
#include <vector>
#include "aeongames/Scene.h"
#include "aeongames/Node.h"

namespace AeonGames
{
    class SceneModel : public QAbstractItemModel
    {
        Q_OBJECT
    public:
        SceneModel ( QObject *parent = nullptr );
        virtual ~SceneModel();
        ///@name Qt QAbstractItemModel overrides
        //@{
        QModelIndex index ( int row, int column, const QModelIndex & parent = QModelIndex() ) const override;
        QModelIndex parent ( const QModelIndex & index ) const override;
        int rowCount ( const QModelIndex & index = QModelIndex() ) const override;
        int columnCount ( const QModelIndex & index = QModelIndex() ) const override;
        QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const override;
        QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const override;
        bool hasChildren ( const QModelIndex & index = QModelIndex() ) const override;
        Qt::ItemFlags flags ( const QModelIndex & index ) const override;
        bool setData ( const QModelIndex & index, const QVariant & value, int role ) override;
        bool moveRows ( const QModelIndex & sourceParent, int sourceRow, int count, const QModelIndex & destinationParent, int destinationRow ) override;
        Qt::DropActions supportedDropActions() const override;
        bool dropMimeData ( const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent ) override;
        QMimeData * mimeData ( const QModelIndexList & indexes ) const override;
        QStringList mimeTypes() const override;
        //@}
        void InsertNode ( int row, const QModelIndex & parent = QModelIndex(), std::unique_ptr<Node> aNode = {} );
        void RemoveNode ( int row, const QModelIndex & parent = QModelIndex() );
        void SetCameraNode ( const QModelIndex & index = QModelIndex() );
        std::string Serialize ( bool aAsBinary = true ) const;
        void Deserialize ( const std::string& aSerializedScene );
        const Scene& GetScene() const;
    private:
        Scene mScene{};
    };
}
#endif
