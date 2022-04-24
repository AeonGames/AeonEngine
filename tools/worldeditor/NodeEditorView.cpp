/*
Copyright (C) 2021,2022 Rodrigo Jose Hernandez Cordoba

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

#include <cmath>
#include <QKeyEvent>
#include "NodeEditorView.h"
#include "WorldEditor.h"

namespace AeonGames
{
    NodeEditorView::NodeEditorView ( QWidget *parent ) : QGraphicsView ( parent )
    {
        QGraphicsScene *scene = new QGraphicsScene ( this );
        scene->setItemIndexMethod ( QGraphicsScene::NoIndex );
        scene->setSceneRect ( -200, -200, 400, 400 );
        setScene ( scene );
        setCacheMode ( CacheBackground );
        setViewportUpdateMode ( BoundingRectViewportUpdate );
        setRenderHint ( QPainter::Antialiasing );
        setTransformationAnchor ( AnchorUnderMouse );
        scale ( qreal ( 0.8 ), qreal ( 0.8 ) );
        setMinimumSize ( 400, 400 );
    }

    void NodeEditorView::keyPressEvent ( QKeyEvent *event )
    {
        switch ( event->key() )
        {
        case Qt::Key_Plus:
            zoomIn();
            break;
        case Qt::Key_Minus:
            zoomOut();
            break;
        default:
            QGraphicsView::keyPressEvent ( event );
        }
    }

#if QT_CONFIG(wheelevent)
    void NodeEditorView::wheelEvent ( QWheelEvent *event )
    {
        scaleView ( std::pow ( 2., -event->angleDelta().y() / 240.0 ) );
    }
#endif

    void NodeEditorView::drawBackground ( QPainter *painter, const QRectF &rect )
    {
        QSettings settings{};
        settings.beginGroup ( "Workspace" );
        QColor backgroundColor = settings.value ( "BackgroundColor", QColor ( 127, 127, 127 ) ).value<QColor>();
        settings.endGroup();
        QRectF sceneRect = this->sceneRect();
        painter->fillRect ( rect.intersected ( sceneRect ), backgroundColor );
        painter->setBrush ( Qt::NoBrush );
        painter->setPen ( QPen ( Qt::black, 0 ) );
        painter->drawRect ( sceneRect );
    }

    void NodeEditorView::scaleView ( qreal scaleFactor )
    {
        qreal factor = transform().scale ( scaleFactor, scaleFactor ).mapRect ( QRectF ( 0, 0, 1, 1 ) ).width();
        if ( factor < 0.07 || factor > 100 )
        {
            return;
        }
        scale ( scaleFactor, scaleFactor );
    }

    void NodeEditorView::zoomIn()
    {
        scaleView ( qreal ( 1.2 ) );
    }

    void NodeEditorView::zoomOut()
    {
        scaleView ( 1 / qreal ( 1.2 ) );
    }
}
