/*
Copyright (C) 2015,2018,2019,2025,2026 Rodrigo Jose Hernandez Cordoba

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
#include "aeongames/Scene.hpp"
#include "aeongames/Node.hpp"

namespace AeonGames
{
    /** @brief Item model representing a scene's node tree for Qt views. */
    class SceneModel : public QAbstractItemModel
    {
        Q_OBJECT
    public:
        /**
         * @brief Construct the scene model.
         * @param parent Parent QObject.
         */
        SceneModel ( QObject *parent = nullptr );
        /** @brief Destructor. */
        virtual ~SceneModel();
        ///@name Qt QAbstractItemModel overrides
        //@{
        /** @brief Return the index of the item at the given row and column under parent.
         *  @param row Row number.
         *  @param column Column number.
         *  @param parent Parent model index.
         *  @return Model index for the specified item. */
        QModelIndex index ( int row, int column, const QModelIndex & parent = QModelIndex() ) const override;
        /** @brief Return the parent of the given model index.
         *  @param index The child model index.
         *  @return Parent model index, or an invalid index if the item has no parent. */
        QModelIndex parent ( const QModelIndex & index ) const override;
        /** @brief Return the number of rows under the given parent.
         *  @param index Parent model index.
         *  @return Number of rows. */
        int rowCount ( const QModelIndex & index = QModelIndex() ) const override;
        /** @brief Return the number of columns for children of the given parent.
         *  @param index Parent model index.
         *  @return Number of columns. */
        int columnCount ( const QModelIndex & index = QModelIndex() ) const override;
        /** @brief Return the data for the given index and role.
         *  @param index Model index.
         *  @param role Data role.
         *  @return Data as a QVariant. */
        QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const override;
        /** @brief Return the header data for the given section, orientation, and role.
         *  @param section Header section index.
         *  @param orientation Horizontal or vertical header.
         *  @param role Data role.
         *  @return Header data as a QVariant. */
        QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const override;
        /** @brief Return whether the given parent index has any children.
         *  @param index Parent model index.
         *  @return True if the parent has children. */
        bool hasChildren ( const QModelIndex & index = QModelIndex() ) const override;
        /** @brief Return the item flags for the given model index.
         *  @param index Model index.
         *  @return Item flags. */
        Qt::ItemFlags flags ( const QModelIndex & index ) const override;
        /** @brief Set the data for the given index and role.
         *  @param index Model index.
         *  @param value New value.
         *  @param role Data role.
         *  @return True if the data was set successfully. */
        bool setData ( const QModelIndex & index, const QVariant & value, int role ) override;
        /** @brief Move rows from one parent to another.
         *  @param sourceParent Parent index of the source rows.
         *  @param sourceRow Starting row to move.
         *  @param count Number of rows to move.
         *  @param destinationParent Parent index of the destination.
         *  @param destinationRow Destination row position.
         *  @return True if the rows were moved successfully. */
        bool moveRows ( const QModelIndex & sourceParent, int sourceRow, int count, const QModelIndex & destinationParent, int destinationRow ) override;
        /** @brief Return the drop actions supported by this model.
         *  @return Supported drop actions. */
        Qt::DropActions supportedDropActions() const override;
        /** @brief Handle data supplied by a drag-and-drop operation.
         *  @param data MIME data from the drop.
         *  @param action The drop action performed.
         *  @param row Target row.
         *  @param column Target column.
         *  @param parent Parent model index.
         *  @return True if the data was handled successfully. */
        bool dropMimeData ( const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent ) override;
        /** @brief Return serialized MIME data for the given model indexes.
         *  @param indexes List of model indexes to serialize.
         *  @return Pointer to a QMimeData object containing the serialized data. */
        QMimeData * mimeData ( const QModelIndexList & indexes ) const override;
        /** @brief Return the list of MIME types that can be used to describe model indexes.
         *  @return List of supported MIME type strings. */
        QStringList mimeTypes() const override;
        //@}
        /**
         * @brief Insert a node into the scene tree.
         * @param row Row position for the new node.
         * @param parent Parent model index.
         * @param aNode Optional node to insert; a default node is created if empty.
         */
        void InsertNode ( int row, const QModelIndex & parent = QModelIndex(), std::unique_ptr<Node> aNode = {} );
        /**
         * @brief Remove a node from the scene tree.
         * @param row Row position of the node to remove.
         * @param parent Parent model index.
         */
        void RemoveNode ( int row, const QModelIndex & parent = QModelIndex() );
        /**
         * @brief Set the camera to the node at the given index.
         * @param index Model index of the camera node.
         */
        void SetCameraNode ( const QModelIndex & index = QModelIndex() );
        /**
         * @brief Serialize the scene to a string.
         * @param aAsBinary If true, serialize as binary; otherwise as text.
         * @return Serialized scene data.
         */
        std::string Serialize ( bool aAsBinary = true ) const;
        /**
         * @brief Deserialize a scene from a string.
         * @param aSerializedScene Serialized scene data.
         */
        void Deserialize ( const std::string& aSerializedScene );
        /** @brief Get a const reference to the underlying scene.
         *  @return Const reference to the scene. */
        const Scene& GetScene() const;
    private:
        Scene mScene{};
    };
}
#endif
