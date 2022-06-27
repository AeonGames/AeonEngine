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
#include <QPainter>
#include <iostream>
#include "NodeView.h"
#include "NodeWindow.h"
#include "WorldEditor.h"

namespace AeonGames
{
    NodeView::NodeView ( QWidget *parent ) : QWidget ( parent )
    {
        //setMinimumSize ( QSize ( 400, 400 ) );
        NodeWindow* nw = new NodeWindow ( this );
        nw->show();
    }
    NodeView::~NodeView() = default;

    void NodeView::paintEvent ( QPaintEvent *event )
    {
        QPainter painter ( this );
        for ( auto &i : event->region() )
        {
            painter.fillRect ( i, Qt::gray );
        }
    }

    void NodeView::keyPressEvent ( QKeyEvent *event )
    {
        switch ( event->key() )
        {
        case Qt::Key_Plus:
            break;
        case Qt::Key_Minus:
            break;
        default:
            QWidget::keyPressEvent ( event );
        }
    }

    void NodeView::mousePressEvent ( QMouseEvent *event )
    {
        if ( event->button() == Qt::LeftButton )
        {
            event->accept();
        }
        else
        {
            QWidget::mousePressEvent ( event );
        }
    }

    void NodeView::mouseMoveEvent ( QMouseEvent *event )
    {
        QWidget::mouseMoveEvent ( event );
    }

    void NodeView::mouseReleaseEvent ( QMouseEvent *event )
    {
        QWidget::mouseReleaseEvent ( event );
    }

#if QT_CONFIG(wheelevent)
    void NodeView::wheelEvent ( QWheelEvent *event )
    {
        QWidget::wheelEvent ( event );
    }
#endif
}
