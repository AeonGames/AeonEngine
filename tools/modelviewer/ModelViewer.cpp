/*
Copyright 2017 Rodrigo Jose Hernandez Cordoba

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
#include "ModelViewer.h"
#include <QMessageBox>

namespace AeonGames
{
    ModelViewer::ModelViewer ( int &argc, char *argv[] ) : QApplication ( argc, argv )
    {}
    ModelViewer::~ModelViewer()
    {}
    bool ModelViewer::notify ( QObject *receiver, QEvent *event )
    {
        try
        {
            return QApplication::notify ( receiver, event );
        }
        catch ( std::runtime_error& e )
        {
            QMessageBox::critical ( nullptr, applicationName(),
                                    e.what(),
                                    QMessageBox::Ok,
                                    QMessageBox::Ok );
            return false;
        }
    }
}