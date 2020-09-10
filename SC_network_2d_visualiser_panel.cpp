/***************************************************************************
**                                                                        **
**  This file is part of SpineCreator, an easy to use GUI for             **
**  describing spiking neural network models.                             **
**  Copyright (C) 2013-2014 Alex Cope, Paul Richmond, Seb James           **
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
**          Authors: Alex Cope, Seb James                                 **
**  Website/Contact: http://bimpa.group.shef.ac.uk/                       **
****************************************************************************/

#include "SC_network_2d_visualiser_panel.h"

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
  #define RETINA_SUPPORT 1.0
#else
  #ifdef Q_OS_WIN2
  #define RETINA_SUPPORT 1.0
  #define RETINA_SUPPORT2 1.75
  #else
  #define RETINA_SUPPORT 1.0
  #endif
#endif

#define CHANGED_TIME 10

NetViewWidget::NetViewWidget(QWidget *parent):QWidget(parent)
{
    // variable for making sure we don't redraw the widget when we don't need to
    changed = CHANGED_TIME;

    currSelType = 0;
    currSelInd = 0;
    myScale = 100.0*RETINA_SUPPORT;
    targmyScale = 100.0*RETINA_SUPPORT;
    viewX = 0.0;
    viewY = 0.0;
    connectMode = false;
    addSelection = false;
    gridSelect = false;
    gridScale = 0.5;
    this->button = Qt::NoButton;
    this->LMB_drag = false;

    // accept both tab and click focus
    this->setFocusPolicy(Qt::StrongFocus);

    // Nothing is moving to begin with.
    this->itemMoving = false;
}


NetViewWidget::~NetViewWidget()
{
}

void NetViewWidget::redrawNetView()
{
    changed = CHANGED_TIME;
}


void NetViewWidget::animate()
{
    float animSpeed = 1.0;

    myScale += (targmyScale - myScale)*animSpeed;

    // only redraw the openGL when we need to
    if (changed) {
        --changed;
        repaint();
    }
}

void NetViewWidget::mousePressEvent(QMouseEvent* event)
{
    this->button = event->button();

    // convert the incoming x and y into the openGL coordinates
    float _x = float((event->x()*RETINA_SUPPORT)-(this->width()*RETINA_SUPPORT)/2)*2.0/(myScale)-viewX;
    float _y = -(float((event->y()*RETINA_SUPPORT)-(this->height()*RETINA_SUPPORT)/2)*2.0/(myScale)-viewY);

    // select under the mouse
    //cout << "coords " << float(_x) << " " << float(_y) << endl;
    if (this->connectMode == false) {
        if (event->button() == Qt::LeftButton) {
            setCursor(Qt::ClosedHandCursor);
            // Rather than using the shift modifier, lets use logic in
            // onLeftMouseDown to see if we should select the object or drag the group.
            bool shiftDown = (QApplication::keyboardModifiers() & Qt::ShiftModifier);
            emit onLeftMouseDown(_x, _y, this->myScale, shiftDown);
        }
        if (event->button() == Qt::RightButton) {
            setCursor(Qt::CrossCursor);
            emit onRightMouseDown(_x, _y, this->myScale);
        }
    } else {
        if (event->button() == Qt::LeftButton) {
            // fix the current edge
            emit addBezierOrProjection(_x, _y);
        }
    }
    event->setAccepted(true);
    LMB_drag = false;
}

void NetViewWidget::mouseReleaseEvent(QMouseEvent* event)
{
    changed = CHANGED_TIME;
    this->button = Qt::NoButton;
    setCursor(Qt::ArrowCursor);
    // convert the incoming x and y into the openGL coordinates
#ifdef NEED_MOUSE_UP_LOGIC
    float _x = float((event->x()*RETINA_SUPPORT)-(this->width()*RETINA_SUPPORT)/2)*2.0/(myScale)-viewX;
    float _y = -(float((event->y()*RETINA_SUPPORT)-(this->height()*RETINA_SUPPORT)/2)*2.0/(myScale)-viewY);
    //qDebug() << "coords: " << float(_x) << " " << float(_y) << endl;
#endif
    // select under the mouse
    if (this->connectMode == false) {
        if (event->button() == Qt::LeftButton && !this->itemMoving) {
#ifdef NEED_MOUSE_UP_LOGIC
            emit onLeftMouseUp(_x, _y, this->myScale);
#endif
        } else if (event->button() == Qt::LeftButton && this->itemMoving) {
            // Item was released after moving.
            emit itemWasMoved();
            this->itemMoving = false;
        }
        if (event->button() == Qt::LeftButton && this->LMB_drag) {
            emit endDragSelect();
            LMB_drag = false;
        }
        if (event->button() == Qt::RightButton) {
            emit endDragSelect();
        }
    } else {

        if (event->button() == Qt::RightButton) {
            // abort connection
            this->connectMode = false;
            //this->setMouseTracking(false);
            emit abortProjection();
        }
    }
    event->setAccepted(true);
}

void NetViewWidget::wheelEvent(QWheelEvent* event)
{
    changed = CHANGED_TIME;
    float val = float(event->delta()) / 320.0;

    val = pow(2.0f,val);
    if (event->orientation() == Qt::Vertical) {
        this->targmyScale *= (val);
#ifdef Q_OS_LINUX
        float _x = float((event->x()*RETINA_SUPPORT)-(this->width()*RETINA_SUPPORT)/2)*2.0/myScale;
        float _y = -float((event->y()*RETINA_SUPPORT)-(this->height()*RETINA_SUPPORT)/2)*2.0/myScale;
        // aim the scrolling at the current cursor location (goes bad on osx)
        double scaleVal = 5.0;
        if (val > 1.0) {
            viewX -= _x/scaleVal;
            viewY += _y/scaleVal;
        }
        if (val < 1.0) {
            viewX += _x/scaleVal;
            viewY -= _y/scaleVal;
        }
#endif
    }
    event->setAccepted(true);
}

void NetViewWidget::mouseMoveEvent(QMouseEvent* event)
{
    DBG() << "called";
    changed = CHANGED_TIME;

    // convert mouse event into openGL coordinates
    float _x = float((event->x()*RETINA_SUPPORT)-(this->width()*RETINA_SUPPORT)/2)*2.0/(myScale)-viewX;
    float _y = -(float((event->y()*RETINA_SUPPORT)-(this->height()*RETINA_SUPPORT)/2)*2.0/(myScale)-viewY);

    if (this->connectMode == false) {
        if ((QApplication::keyboardModifiers() & Qt::ShiftModifier)) {
            if (this->button == Qt::LeftButton) {

                if (LMB_drag == false) {
                    qDebug() << "Start LMB drag";
                    emit onRightMouseDown(_x, _y, this->myScale);
                } else {
                    emit dragSelect(_x, _y);
                }
                 LMB_drag = true;
            }
        } else {
            if (this->button == Qt::LeftButton) {
                this->itemMoving = true;
                emit mouseMove(_x, _y);
            } else {
                this->itemMoving = false;
            }
        }

        if (this->button == Qt::RightButton) {
            emit dragSelect(_x, _y);
        }
    }  else {
        emit drawSynapse(_x, _y);
    }
    //repaint();
    event->setAccepted(true);
}

void NetViewWidget::keyPressEvent(QKeyEvent * event)
{
    changed = CHANGED_TIME;

    if (event->type() == QEvent::KeyPress) {
        if (event->key() == Qt::Key_Control) {
            this->gridSelect = true;
        }
        /*
        if (event->key() == Qt::Key_Shift) {
            this->addSelection = true;
        }
        */
    }
    event->setAccepted(true);
}

void NetViewWidget::keyReleaseEvent(QKeyEvent * event)
{
    changed = CHANGED_TIME;

    if (event->type() == QEvent::KeyRelease) {
        if (event->key() == Qt::Key_Control) {
            this->gridSelect = false;
        }
        /*
        if (event->key() == Qt::Key_Shift) {
            this->addSelection = false;
        }
        */
    }
    event->setAccepted(true);
}

void NetViewWidget::initializeGL()
{
}

void NetViewWidget::resizeGL(int, int)
{
}

void NetViewWidget::paintEvent(QPaintEvent * event)
{
    QPainter painter(this);

    //painter.setRenderHint(QPainter::HighQualityAntialiasing, true);
    painter.setRenderHint(QPainter::Antialiasing,true);
    painter.setRenderHint( QPainter::TextAntialiasing,true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform,true);

    QRect fillRectangle(event->rect().x()*RETINA_SUPPORT, event->rect().y()*RETINA_SUPPORT, event->rect().width()*RETINA_SUPPORT, event->rect().height()*RETINA_SUPPORT);
    painter.fillRect(fillRectangle, Qt::white);
    // painter.beginNativePainting();

    // setup the painter
    QFont font("Monospace", myScale/18.0f);
    font.setPointSizeF(myScale/18.0f);
    font.setStyleHint(QFont::TypeWriter);
    painter.setPen(QCOL_BLACK);
    painter.setFont(font);

    // if grid then draw it up:
    if (this->gridSelect) {
        painter.save();
        painter.translate((this->width()*RETINA_SUPPORT)/2, (this->height()*RETINA_SUPPORT)/2);
        //painter.scale(,-myScale/2.0);
        painter.translate(viewX*(myScale/2.0), viewY*(myScale/2.0));
        // too fine a grid looks bad and is of no use
        QPen pen = painter.pen();
        pen.setWidth(1.5*RETINA_SUPPORT);
        painter.setPen(pen);
        if (myScale > 10) {
            for (float i = round(-viewX/this->gridScale)*this->gridScale*myScale; i < round(-viewX/this->gridScale)*this->gridScale+(this->width()*RETINA_SUPPORT); i +=this->gridScale*myScale/2.0) {
                for (float j =round(viewY/this->gridScale)*this->gridScale*myScale; j < round(viewY/this->gridScale)*this->gridScale+(this->height()*RETINA_SUPPORT); j +=this->gridScale*myScale/2.0) {
                    painter.drawPoint(QPointF(i,j));
                }
            }
            for (float i = round(-viewX/this->gridScale)*this->gridScale*myScale; i > round(-viewX/this->gridScale)*this->gridScale-(this->width()*RETINA_SUPPORT); i -=this->gridScale*myScale/2.0) {
                for (float j =round(viewY/this->gridScale)*this->gridScale*myScale; j > round(viewY/this->gridScale)*this->gridScale-(this->height()*RETINA_SUPPORT); j -=this->gridScale*myScale/2.0) {
                    painter.drawPoint(QPointF(i,j));
                }
            }
            for (float i = round(-viewX/this->gridScale)*this->gridScale*myScale; i > round(-viewX/this->gridScale)*this->gridScale-(this->width()*RETINA_SUPPORT); i -=this->gridScale*myScale/2.0) {
                for (float j =round(viewY/this->gridScale)*this->gridScale*myScale; j < round(viewY/this->gridScale)*this->gridScale+(this->height()*RETINA_SUPPORT); j +=this->gridScale*myScale/2.0) {
                    painter.drawPoint(QPointF(i,j));
                }
            }
            for (float i = round(-viewX/this->gridScale)*this->gridScale*myScale; i < round(-viewX/this->gridScale)*this->gridScale+(this->width()*RETINA_SUPPORT); i +=this->gridScale*myScale/2.0) {
                for (float j =round(viewY/this->gridScale)*this->gridScale*myScale; j > round(viewY/this->gridScale)*this->gridScale-(this->height()*RETINA_SUPPORT); j -=this->gridScale*myScale/2.0) {
                    painter.drawPoint(QPointF(i,j));
                }
            }
        }
        painter.restore();
    }

    // send the painter, and the transforms needed to paint to the right place
    emit reDraw(&painter, myScale, viewX, viewY, (this->width()*RETINA_SUPPORT), (this->height()*RETINA_SUPPORT), standardDrawStyle);

    //painter.endNativePainting();
    painter.end();
}

void NetViewWidget::move(float x, float y)
{
    this->viewX =x;
    this->viewY =-y;
    //emit reDraw();
}

void NetViewWidget::zoomOut()
{
    this->targmyScale *= 2.0;
}

void NetViewWidget::zoomIn()
{
    this->targmyScale /= 2.0;
}

void NetViewWidget::startConnect()
{
    this->connectMode = true;
    this->setMouseTracking(true);
}

void NetViewWidget::finishConnect()
{
    this->connectMode = false;
    this->setMouseTracking(false);

    changed = CHANGED_TIME;
}

void NetViewWidget::saveImage()
{
    //this->makeCurrent();
    //QPixmap pic = this->renderPixmap((this->width()*RETINA_SUPPORT)*4,(this->height()*RETINA_SUPPORT)*4, false);
    //QImage image = pic.toImage();
    //image.save("/home/mp/test_image.png","png");
}
