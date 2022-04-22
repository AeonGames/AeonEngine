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
#ifndef AEONGAMES_RENDERERSELECT_DIALOG_H
#define AEONGAMES_RENDERERSELECT_DIALOG_H

#include "ui_RendererSelectDialog.h"

namespace AeonGames
{
    class RendererSelectDialog : public QDialog
    {
        Q_OBJECT
    public:
        RendererSelectDialog ( QWidget *parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags() );
        ~RendererSelectDialog();
        void SetRenderers ( const QStringList& renderers );
        const QString GetSelected() const;
    private slots:
    private:
        Ui::RendererSelectDialog mUi;
    };
}
#endif
