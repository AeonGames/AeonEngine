/*
Copyright (C) 2018 Rodrigo Jose Hernandez Cordoba

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

#ifndef AEONGAMES_MESSAGEMODEL_H
#define AEONGAMES_MESSAGEMODEL_H
#include <QAbstractItemModel>

namespace google
{
    namespace protobuf
    {
        class Message;
        class Descriptor;
        class FieldDescriptor;
    }
}
namespace AeonGames
{
    class MessageModel : public QAbstractItemModel
    {
        Q_OBJECT
    public:
        MessageModel ( QObject *parent = nullptr );
        virtual ~MessageModel();
        class Field
        {
        public:
            Field ( google::protobuf::Message* aMessage, const google::protobuf::FieldDescriptor* aFieldDescriptor, int aRepeatedIndex = 0, Field* aParent = nullptr );
            Field ( const Field& aField );
            Field& operator= ( const Field& aField );
            Field ( const Field&& aField );
            Field& operator= ( const Field&& aField );
            int GetIndexAtParent() const;
            int GetRepeatedIndex() const;
            std::string GetPrintableName() const;
            google::protobuf::Message* GetMessagePtr() const;
            const google::protobuf::FieldDescriptor* GetFieldDescriptor() const;
            const Field* GetParent() const;
            const std::vector<Field>& GetChildren() const;
        private:
            google::protobuf::Message* mMessage{};
            const google::protobuf::FieldDescriptor* mFieldDescriptor{};
            int mRepeatedIndex{};
            Field* mParent{};
            std::vector<Field> mChildren{};
        };
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
        //@}
        void SetMessage ( google::protobuf::Message* aMessage );
        google::protobuf::Message* GetMessagePtr() const;
        const std::vector<Field>& GetFields() const;
        int GetFieldIndex ( const Field* aField ) const;
    private:
        google::protobuf::Message* mMessage{};
        std::vector<Field> mFields{};
    };
}
#endif
