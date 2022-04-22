/*
Copyright (C) 2022 Rodrigo Jose Hernandez Cordoba

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

#ifndef AEONGAMES_NODE_EDITOR_VIEW_H
#define AEONGAMES_NODE_EDITOR_VIEW_H

#include <QGraphicsView>

namespace AeonGames
{
    class NodeEditorView : public QGraphicsView
    {
        Q_OBJECT
    public:
        NodeEditorView ( QWidget *parent = nullptr );
        void itemMoved();

    public slots:
        void zoomIn();
        void zoomOut();

    protected:
        void keyPressEvent ( QKeyEvent *event ) override;
#if QT_CONFIG(wheelevent)
        void wheelEvent ( QWheelEvent *event ) override;
#endif
        void drawBackground ( QPainter *painter, const QRectF &rect ) override;
        void scaleView ( qreal scaleFactor );
    };
}
#endif
