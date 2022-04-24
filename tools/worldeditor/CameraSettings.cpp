/*
Copyright (C) 2019,2022 Rodrigo Jose Hernandez Cordoba

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
#include "CameraSettings.h"
#include <iostream>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QDialogButtonBox>

namespace AeonGames
{
    CameraSettings::CameraSettings (
        QWidget *parent, Qt::WindowFlags f ) : QDialog ( parent, f )
    {
        mUi.setupUi ( this );
        QSettings settings{};
        settings.beginGroup ( "Camera" );
        setFieldOfView ( settings.value ( "FieldOfView", 60.0f ).toDouble() );
        setNear ( settings.value ( "Near", 1.0f ).toDouble() );
        setFar ( settings.value ( "Far", 1600.0f ).toDouble() );
        settings.endGroup();
    }

    CameraSettings::~CameraSettings()
    {
        disconnect();
        std::cout << "CameraSettings::~CameraSettings()" << std::endl;
    }
    float CameraSettings::GetFieldOfView() const
    {
        return mUi.dblFieldOfView->value();
    }
    float CameraSettings::GetNear() const
    {
        return mUi.dblNear->value();
    }
    float CameraSettings::GetFar() const
    {
        return mUi.dblFar->value();
    }
    void CameraSettings::setFieldOfView ( double aFieldOfView )
    {
        QSettings settings{};
        settings.beginGroup ( "Camera" );
        settings.setValue ( "FieldOfView", static_cast<float> ( aFieldOfView ) );
        settings.endGroup();
        bool blkSignals = mUi.dblFieldOfView->blockSignals ( true );
        mUi.dblFieldOfView->setValue ( aFieldOfView );
        mUi.dblFieldOfView->blockSignals ( blkSignals );
        emit fieldOfViewChanged ( aFieldOfView );
    }
    void CameraSettings::setNear ( double aNear )
    {
        QSettings settings{};
        settings.beginGroup ( "Camera" );
        settings.setValue ( "Near", static_cast<float> ( aNear ) );
        settings.endGroup();
        bool blkSignals = mUi.dblNear->blockSignals ( true );
        mUi.dblNear->setValue ( aNear );
        mUi.dblNear->blockSignals ( blkSignals );
        emit nearChanged ( aNear );
    }
    void CameraSettings::setFar ( double aFar )
    {
        QSettings settings{};
        settings.beginGroup ( "Camera" );
        settings.setValue ( "Far", static_cast<float> ( aFar ) );
        settings.endGroup();
        bool blkSignals = mUi.dblFar->blockSignals ( true );
        mUi.dblFar->setValue ( aFar );
        mUi.dblFar->blockSignals ( blkSignals );
        emit farChanged ( aFar );
    }
    void CameraSettings::clicked ( QAbstractButton* aButton )
    {
        if ( mUi.buttonBox->buttonRole ( aButton ) == QDialogButtonBox::ResetRole )
        {
            setFieldOfView ( 60.0 );
            setNear ( 1.0 );
            setFar ( 1600.0 );
        }
    }
}
