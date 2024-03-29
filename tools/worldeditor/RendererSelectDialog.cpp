/*
Copyright (C) 2017,2018,2022 Rodrigo Jose Hernandez Cordoba

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
#include "RendererSelectDialog.h"
#include <QComboBox>

namespace AeonGames
{
    RendererSelectDialog::RendererSelectDialog ( QWidget *parent, Qt::WindowFlags f ) :
        QDialog ( parent, f )
    {
        mUi.setupUi ( this );
    }

    RendererSelectDialog::~RendererSelectDialog()
        = default;

    void RendererSelectDialog::SetRenderers ( const QStringList & renderers )
    {
        mUi.mRendererComboBox->clear();
        mUi.mRendererComboBox->addItems ( renderers );
    }
    const QString RendererSelectDialog::GetSelected() const
    {
        return mUi.mRendererComboBox->currentText();
    }
}
