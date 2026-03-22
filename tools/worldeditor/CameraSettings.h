/*
Copyright (C) 2019,2022,2026 Rodrigo Jose Hernandez Cordoba

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
    /** @brief Dialog for adjusting camera projection settings. */
    class CameraSettings : public QDialog
    {
        Q_OBJECT
    public:
        /**
         * @brief Construct the camera settings dialog.
         * @param parent Parent widget.
         * @param f Window flags.
         */
        CameraSettings ( QWidget *parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags() );
        /** @brief Destructor. */
        ~CameraSettings() override;
        /** @brief Get the field of view angle.
         *  @return Field of view in degrees. */
        float GetFieldOfView() const;
        /** @brief Get the near clipping plane distance.
         *  @return Near plane distance. */
        float GetNear() const;
        /** @brief Get the far clipping plane distance.
         *  @return Far plane distance. */
        float GetFar() const;
    private slots:
        /** @brief Slot called when the field of view value changes.
         *  @param aFieldOfView New field of view value. */
        void setFieldOfView ( double aFieldOfView );
        /** @brief Slot called when the near plane value changes.
         *  @param aNear New near plane value. */
        void setNear ( double aNear );
        /** @brief Slot called when the far plane value changes.
         *  @param aFar New far plane value. */
        void setFar ( double aFar );
        /** @brief Slot called when a dialog button is clicked.
         *  @param aButton The button that was clicked. */
        void clicked ( QAbstractButton* aButton );
    signals:
        /** @brief Emitted when the field of view changes.
         *  @param aFieldOfView New field of view value. */
        void fieldOfViewChanged ( double aFieldOfView );
        /** @brief Emitted when the near plane changes.
         *  @param aNear New near plane value. */
        void nearChanged ( double aNear );
        /** @brief Emitted when the far plane changes.
         *  @param aFar New far plane value. */
        void farChanged ( double aFar );
    private:
        Ui::CameraSettings mUi;
    };
}
#endif
