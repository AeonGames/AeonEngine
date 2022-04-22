/*
Copyright (C) 2014,2015,2018,2019,2022 Rodrigo Jose Hernandez Cordoba

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
#include "SettingsDialog.h"
#include <QColorDialog>
namespace AeonGames
{
    SettingsDialog::SettingsDialog()
    {
        mUi.setupUi ( this );
        settings.beginGroup ( "MainWindow" );
        mUi.backgroundColorPushButton->setPalette ( QPalette ( settings.value ( "background color", QColor ( 57, 57, 57 ) ).value<QColor>() ) );
        settings.endGroup();
    }


    SettingsDialog::~SettingsDialog()
    {
    }

    void SettingsDialog::onChangeBackgroundColor()
    {
#if QT_VERSION < QT_VERSION_CHECK(5, 13, 0)
        QColor color = QColorDialog::getColor ( mUi.backgroundColorPushButton->palette().background().color() );
#else
        QColor color = QColorDialog::getColor ( mUi.backgroundColorPushButton->palette().window().color() );
#endif
        settings.beginGroup ( "MainWindow" );
        settings.setValue ( "background color", color );
        settings.endGroup();
        mUi.backgroundColorPushButton->setPalette ( QPalette ( color ) );
        emit backgroundColor ( color );
    }
}