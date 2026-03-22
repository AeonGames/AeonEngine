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

#ifndef AEONGAMES_NODE_VIEW_H
#define AEONGAMES_NODE_VIEW_H

#include <QWidget>

namespace AeonGames
{
    /** @brief Custom widget for visual node graph editing. */
    class NodeView : public QWidget
    {
        Q_OBJECT
    public:
        /**
         * @brief Construct the node view widget.
         * @param parent Parent widget.
         */
        NodeView ( QWidget *parent = nullptr );
        /** @brief Destructor. */
        ~NodeView();
    public slots:
    protected:
        /// @brief Handle key press events.
        void keyPressEvent ( QKeyEvent *event ) override;
        /// @brief Handle mouse press events.
        void mousePressEvent ( QMouseEvent *event ) override;
        /// @brief Handle mouse move events.
        void mouseMoveEvent ( QMouseEvent *event ) override;
        /// @brief Handle mouse release events.
        void mouseReleaseEvent ( QMouseEvent *event ) override;
        /// @brief Handle paint events.
        void paintEvent ( QPaintEvent *event ) override;
#if QT_CONFIG(wheelevent)
        void wheelEvent ( QWheelEvent *event ) override;
#endif
    private:
    };
}
#endif
