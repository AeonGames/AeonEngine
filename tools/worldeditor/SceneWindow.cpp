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
#include <QFileDialog>
#include <QMdiSubWindow>
#include <QSurfaceFormat>
#include "SceneWindow.h"
#include "EngineWindow.h"

namespace AeonGames
{
    SceneWindow::SceneWindow ( const std::shared_ptr<Renderer>& aRenderer, QWidget *parent, Qt::WindowFlags f ) :
        QWidget ( parent, f ),
        Ui::SceneWindow(),
        mRenderer ( aRenderer )
    {
        setupUi ( this );
        mEngineWindow = new EngineWindow ( mRenderer );
        QWidget* widget = QWidget::createWindowContainer ( mEngineWindow, splitter );
        QSizePolicy size_policy ( QSizePolicy::Expanding , QSizePolicy::Expanding );
        size_policy.setHorizontalStretch ( 2 );
        widget->setSizePolicy ( size_policy );
        splitter->addWidget ( widget );
    }

    SceneWindow::~SceneWindow()
        = default;

    void SceneWindow::setModel ( const QString& filename )
    {
        mEngineWindow->setModel ( filename );
    }
}
