/*
Copyright (C) 2014,2015,2018 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_SETTINGSDIALOG_H
#define AEONGAMES_SETTINGSDIALOG_H
#include <QDialog>
#include <QSettings>
#include "ui_SettingsDialog.h"
namespace AeonGames
{
    class SettingsDialog :
        public QDialog, public Ui::SettingsDialog
    {
        Q_OBJECT
    public:
        SettingsDialog();
        ~SettingsDialog();
    public slots:
        void onChangeBackgroundColor();
    signals:
        void backgroundColor ( QColor color );
    private:
        QSettings settings;
    };
}
#endif
