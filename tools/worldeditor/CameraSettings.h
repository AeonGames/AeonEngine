/*
Copyright (C) 2019 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_CAMERA_SETTINGS_H
#define AEONGAMES_CAMERA_SETTINGS_H

#include "ui_CameraSettings.h"

namespace AeonGames
{
    class CameraSettings : public QDialog, public Ui::CameraSettings
    {
        Q_OBJECT
    public:
        CameraSettings (
            QWidget *parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags() );
        ~CameraSettings();
        float GetFieldOfView() const;
        float GetNear() const;
        float GetFar() const;
    private slots:
        void setFieldOfView ( double aFieldOfView );
        void setNear ( double aNear );
        void setFar ( double aFar );
        void clicked ( QAbstractButton* aButton );
    signals:
        void fieldOfViewChanged ( double aFieldOfView );
        void nearChanged ( double aNear );
        void farChanged ( double aFar );
    private:
    };
}
#endif
