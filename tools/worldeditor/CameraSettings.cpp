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
#include "WorldEditor.h"
#include "CameraSettings.h"
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QDialogButtonBox>

namespace AeonGames
{
    CameraSettings::CameraSettings (
        QWidget *parent, Qt::WindowFlags f ) :
        QDialog ( parent, f ), Ui::CameraSettings()
    {
        setupUi ( this );
        auto& settings = qWorldEditorApp->GetSettings();
        settings.beginGroup ( "Camera" );
        setFieldOfView ( settings.value ( "FieldOfView", 60.0f ).toDouble() );
        setNear ( settings.value ( "Near", 1.0f ).toDouble() );
        setFar ( settings.value ( "Far", 1600.0f ).toDouble() );
        settings.endGroup();
    }

    CameraSettings::~CameraSettings()
        = default;
    float CameraSettings::GetFieldOfView() const
    {
        return dblFieldOfView->value();
    }
    float CameraSettings::GetNear() const
    {
        return dblNear->value();
    }
    float CameraSettings::GetFar() const
    {
        return dblFar->value();
    }
    void CameraSettings::setFieldOfView ( double aFieldOfView )
    {
        auto& settings = qWorldEditorApp->GetSettings();
        settings.beginGroup ( "Camera" );
        settings.setValue ( "FieldOfView", static_cast<float> ( aFieldOfView ) );
        settings.endGroup();
        bool blkSignals = dblFieldOfView->blockSignals ( true );
        dblFieldOfView->setValue ( aFieldOfView );
        dblFieldOfView->blockSignals ( blkSignals );
        emit fieldOfViewChanged ( aFieldOfView );
    }
    void CameraSettings::setNear ( double aNear )
    {
        auto& settings = qWorldEditorApp->GetSettings();
        settings.beginGroup ( "Camera" );
        settings.setValue ( "Near", static_cast<float> ( aNear ) );
        settings.endGroup();
        bool blkSignals = dblNear->blockSignals ( true );
        dblNear->setValue ( aNear );
        dblNear->blockSignals ( blkSignals );
        emit nearChanged ( aNear );
    }
    void CameraSettings::setFar ( double aFar )
    {
        auto& settings = qWorldEditorApp->GetSettings();
        settings.beginGroup ( "Camera" );
        settings.setValue ( "Far", static_cast<float> ( aFar ) );
        settings.endGroup();
        bool blkSignals = dblFar->blockSignals ( true );
        dblFar->setValue ( aFar );
        dblFar->blockSignals ( blkSignals );
        emit farChanged ( aFar );
    }
    void CameraSettings::clicked ( QAbstractButton* aButton )
    {
        if ( buttonBox->buttonRole ( aButton ) == QDialogButtonBox::ResetRole )
        {
            setFieldOfView ( 60.0 );
            setNear ( 1.0 );
            setFar ( 1600.0 );
        }
    }
}
