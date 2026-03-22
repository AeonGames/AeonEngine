/*
Copyright (C) 2022,2026 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_NODE_WINDOW_H
#define AEONGAMES_NODE_WINDOW_H

#include "ui_NodeWindow.h"

namespace AeonGames
{
    /** @brief Widget for displaying and editing node properties. */
    class NodeWindow : public QWidget
    {
        Q_OBJECT
    public:
        /**
         * @brief Construct the node window widget.
         * @param parent Parent widget.
         * @param f Window flags.
         */
        NodeWindow ( QWidget *parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags() );
        /** @brief Destructor. */
        ~NodeWindow();
    private slots:
    signals:
    private:
        Ui::NodeWindow mUi;
    };
}
#endif
