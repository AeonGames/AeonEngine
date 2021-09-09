/*
Copyright (C) 2021 Rodrigo Jose Hernandez Cordoba

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
#include "WorldEditor.h"
#include "NodeEditor.h"
#include <QLabel>

namespace AeonGames
{
    NodeEditor::NodeEditor (
        QWidget *parent, Qt::WindowFlags f ) :
        QWidget ( parent, f ), Ui::NodeEditor()
    {
        setupUi ( this );
        mScene = new QGraphicsScene ( this );
        mScene->addWidget ( new QLabel ( "Hello World" ) );
        mView->setScene ( mScene );
    }
    NodeEditor::~NodeEditor()
    {
        mScene->clear();
        mScene->deleteLater();
    }
}
