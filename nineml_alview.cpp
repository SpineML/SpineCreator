/***************************************************************************
**                                                                        **
**  This file is part of SpineCreator, an easy to use, GUI for            **
**  describing spiking neural network models.                             **
**  Copyright (C) 2013 Alex Cope, Paul Richmond                           **
**                                                                        **
**  This program is free software: you can redistribute it and/or modify  **
**  it under the terms of the GNU General Public License as published by  **
**  the Free Software Foundation, either version 3 of the License, or     **
**  (at your option) any later version.                                   **
**                                                                        **
**  This program is distributed in the hope that it will be useful,       **
**  but WITHOUT ANY WARRANTY; without even the implied warranty of        **
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         **
**  GNU General Public License for more details.                          **
**                                                                        **
**  You should have received a copy of the GNU General Public License     **
**  along with this program.  If not, see http://www.gnu.org/licenses/.   **
**                                                                        **
****************************************************************************
**           Author: Paul Richmond                                        **
**  Website/Contact: http://bimpa.group.shef.ac.uk/                       **
****************************************************************************/


#include "nineml_alview.h"

ALView::ALView(QWidget *parent) :
    QGraphicsView(parent)
{
    setDragMode(QGraphicsView::NoDrag);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setCursor(Qt::ArrowCursor);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setRenderHints(QPainter::TextAntialiasing | QPainter::HighQualityAntialiasing);
}

void ALView::zoomIn()
{
    scale(ALView::ZOOM_FACTOR(), ALView::ZOOM_FACTOR());
}

void ALView::zoomOut()
{
    scale(1.0 / ALView::ZOOM_FACTOR(), 1.0 / ALView::ZOOM_FACTOR());
}

void ALView::wheelEvent(QWheelEvent *event)
{
        if(event->delta() > 0) {
            //Zoom in
            zoomIn();
        } else {
            //Zooming out
            zoomOut();
        }
        event->accept();
}

void ALView::mousePressEvent(QMouseEvent *event)
{
    setDragMode(QGraphicsView::ScrollHandDrag);
    QGraphicsView::mousePressEvent(event);
}

void ALView::mouseReleaseEvent(QMouseEvent *event)
{
    setDragMode(QGraphicsView::NoDrag);
    QGraphicsView::mouseReleaseEvent(event);
}

qreal ALView::ZOOM_FACTOR()
{
    return (qreal) 1.5;
}

