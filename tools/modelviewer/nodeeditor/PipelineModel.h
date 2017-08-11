/*
Copyright (C) 2017 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_PIPELINEMODEL_H
#define AEONGAMES_PIPELINEMODEL_H

#include <iostream>
#include <QtCore/QObject>
#include <QtWidgets/QLabel>
#include <nodes/DataModelRegistry.hpp>
#include <nodes/NodeDataModel.hpp>
#include "aeongames/Memory.h"

namespace AeonGames
{
    class PipelineModel : public QtNodes::NodeDataModel
    {
    public:
        PipelineModel();
        virtual ~PipelineModel() {}
        QString caption() const override;
        QString name() const override;
        std::unique_ptr<NodeDataModel> clone() const override;
        unsigned int nPorts ( QtNodes::PortType portType ) const override;
        QtNodes::NodeDataType dataType ( QtNodes::PortType portType, QtNodes::PortIndex portIndex ) const override;
        std::shared_ptr<QtNodes::NodeData> outData ( QtNodes::PortIndex port ) override;
        void setInData ( std::shared_ptr<QtNodes::NodeData> nodeData, QtNodes::PortIndex port ) override;
        QWidget * embeddedWidget() override;
        bool resizable() const override;
    protected:
        bool eventFilter ( QObject *object, QEvent *event ) override;
    private:
        Q_OBJECT
    };
}
#endif