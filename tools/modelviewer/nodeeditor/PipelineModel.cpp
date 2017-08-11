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
#include <QtCore/QEvent>
#include <QtCore/QDir>
#include <QtWidgets/QFileDialog>
#include <nodes/DataModelRegistry.hpp>
#include "PipelineModel.h"

namespace AeonGames
{
    PipelineModel::PipelineModel()
    {
    }

    unsigned int PipelineModel::nPorts ( QtNodes::PortType portType ) const
    {
        return 0;
    }

    bool PipelineModel::eventFilter ( QObject *object, QEvent *event )
    {
        return false;
    }

    QtNodes::NodeDataType PipelineModel::dataType ( QtNodes::PortType, QtNodes::PortIndex ) const
    {
        return {"none", "none"};
    }

    std::shared_ptr<QtNodes::NodeData> PipelineModel::outData ( QtNodes::PortIndex )
    {
        return nullptr;
    }

    void PipelineModel::setInData ( std::shared_ptr<QtNodes::NodeData> nodeData, QtNodes::PortIndex )
    {
        emit dataUpdated ( 0 );
    }

    QString PipelineModel::name() const
    {
        return QString ( "Pipeline" );
    }

    QString PipelineModel::caption() const
    {
        return QString ( "Pipeline" );
    }

    std::unique_ptr<QtNodes::NodeDataModel> PipelineModel::clone() const
    {
        return std::unique_ptr<PipelineModel>();
    }

    QWidget * PipelineModel::embeddedWidget()
    {
        return nullptr;
    }

    bool PipelineModel::resizable() const
    {
        return true;
    }
}