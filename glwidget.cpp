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

#include "glwidget.h"

#ifdef Q_OS_MAC
GLWidget::GLWidget(QWidget *parent)
    : QGLWidget(QGLFormat(QGL::SampleBuffers), parent)
#else
GLWidget::GLWidget(QWidget *parent):QWidget(parent)
#endif

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
  #define RETINA_SUPPORT 1.0
#else
  #ifdef Q_OS_MAC
  #define RETINA_SUPPORT this->windowHandle()->devicePixelRatio()
  #else
  #define RETINA_SUPPORT 1.0
  #endif
#endif
{
    // variable for making sure we don't redraw the openGL when we don't need to
    changed = 100;

    currSelType = 0;
    currSelInd = 0;
    GLscale = 100.0*RETINA_SUPPORT;
    targGLscale = 100.0*RETINA_SUPPORT;
    viewX = 0.0;
    viewY = 0.0;
    connectMode = false;
    addSelection = false;
    gridSelect = false;
    gridScale = 0.5;
    this->button = Qt::NoButton;

    // accept both tab and click focus
    this->setFocusPolicy(Qt::StrongFocus);

    // Nothing is moving to begin with.
    this->itemMoving = false;
}


GLWidget::~GLWidget()
{
}

void GLWidget::redrawGLview()
{
    changed = 100;
}


void GLWidget::animate()
{
    float animSpeed = 1.0;

    GLscale += (targGLscale - GLscale)*animSpeed;

    // only redraw the openGL when we need to
    if (changed) {
        --changed;
        repaint();
    }
}

void GLWidget::mousePressEvent(QMouseEvent* event)
{
    this->button = event->button();

    // convert the incoming x and y into the openGL coordinates
    float xGL = float((event->x()*RETINA_SUPPORT)-(this->width()*RETINA_SUPPORT)/2)*2.0/(GLscale)-viewX;
    float yGL = -(float((event->y()*RETINA_SUPPORT)-(this->height()*RETINA_SUPPORT)/2)*2.0/(GLscale)-viewY);

    // select under the mouse
    //cout << "coords " << float(xGL) << " " << float(yGL) << endl;
    if (this->connectMode == false) {
        if (event->button() == Qt::LeftButton) {
            setCursor(Qt::ClosedHandCursor);
            // Rather than using the shift modifier, lets use logic in
            // onLeftMouseDown to see if we should select the object or drag the group.
            bool shiftDown = (QApplication::keyboardModifiers() & Qt::ShiftModifier);
            emit onLeftMouseDown(xGL, yGL, this->GLscale, shiftDown);
        }
        if (event->button() == Qt::RightButton) {
            setCursor(Qt::CrossCursor);
            emit onRightMouseDown(xGL, yGL, this->GLscale);
        }
    } else {
        if (event->button() == Qt::LeftButton) {
            // fix the current edge
            emit addBezierOrProjection(xGL, yGL);
        }
    }
    event->setAccepted(true);
}

void GLWidget::mouseReleaseEvent(QMouseEvent* event)
{
    changed = 100;
    this->button = Qt::NoButton;
    setCursor(Qt::ArrowCursor);
    // convert the incoming x and y into the openGL coordinates
    float xGL = float((event->x()*RETINA_SUPPORT)-(this->width()*RETINA_SUPPORT)/2)*2.0/(GLscale)-viewX;
    float yGL = -(float((event->y()*RETINA_SUPPORT)-(this->height()*RETINA_SUPPORT)/2)*2.0/(GLscale)-viewY);

    // select under the mouse
    //qDebug() << "coords: " << float(xGL) << " " << float(yGL) << endl;
    if (this->connectMode == false) {
        if (event->button() == Qt::LeftButton && !this->itemMoving) {
#ifdef NEED_MOUSE_UP_LOGIC
            emit onLeftMouseUp(xGL, yGL, this->GLscale);
#endif
        } else if (event->button() == Qt::LeftButton && this->itemMoving) {
            // Item was released after moving.
            emit itemWasMoved();
            this->itemMoving = false;
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

void GLWidget::wheelEvent(QWheelEvent* event)
{
    changed = 100;
    float val = float(event->delta()) / 320.0;

    val = pow(2.0f,val);
    if (event->orientation() == Qt::Vertical) {
        this->targGLscale *= (val);
#ifndef Q_OS_MAC
        float xGL = float((event->x()*RETINA_SUPPORT)-(this->width()*RETINA_SUPPORT)/2)*2.0/GLscale;
        float yGL = -float((event->y()*RETINA_SUPPORT)-(this->height()*RETINA_SUPPORT)/2)*2.0/GLscale;
        // aim the scrolling at the current cursor location (goes bad on osx)
        double scaleVal = 5.0;
        if (val > 1.0) {
            viewX -= xGL/scaleVal;
            viewY += yGL/scaleVal;
        }
        if (val < 1.0) {
            viewX += xGL/scaleVal;
            viewY -= yGL/scaleVal;
        }
#endif
    }
    event->setAccepted(true);
}

void GLWidget::mouseMoveEvent(QMouseEvent* event)
{
    changed = 100;

    // convert mouse event into openGL coordinates
    float xGL = float((event->x()*RETINA_SUPPORT)-(this->width()*RETINA_SUPPORT)/2)*2.0/(GLscale)-viewX;
    float yGL = -(float((event->y()*RETINA_SUPPORT)-(this->height()*RETINA_SUPPORT)/2)*2.0/(GLscale)-viewY);

    if (this->connectMode == false) {
        if (this->button == Qt::LeftButton) {
            this->itemMoving = true;
            emit mouseMove(xGL, yGL);
        } else {
            this->itemMoving = false;
        }

        if (this->button == Qt::RightButton) {
            emit dragSelect(xGL, yGL);
        }
    }  else {
        emit drawSynapse(xGL, yGL);
    }
    repaint();
    event->setAccepted(true);
}

void GLWidget::keyPressEvent(QKeyEvent * event)
{
    changed = 100;

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

void GLWidget::keyReleaseEvent(QKeyEvent * event)
{
    changed = 100;

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

void GLWidget::initializeGL()
{
    //QColor qtCol = QColor::fromCmykF(0.5, 0.5, 0.5, 0.0);
    //qglClearColor(qtCol.light());
}

void GLWidget::resizeGL(int, int)
{
}

void GLWidget::paintEvent(QPaintEvent * event)
{

#ifdef Q_OS_MAC
    makeCurrent();
    //QPixmap pix((this->width()*RETINA_SUPPORT), (this->height()*RETINA_SUPPORT))
    glViewport (0, 0, (this->width()*RETINA_SUPPORT), (this->height()*RETINA_SUPPORT));
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, (this->width()*RETINA_SUPPORT),0,(this->height()*RETINA_SUPPORT),-1,1);
    glMatrixMode (GL_MODELVIEW);
    QImage workaround_for_slow_osx_drawing((this->width()*RETINA_SUPPORT), (this->height()*RETINA_SUPPORT), QImage::Format_RGB32);
    QPainter painter(&workaround_for_slow_osx_drawing);
    workaround_for_slow_osx_drawing.fill(Qt::red); // debug so we can check all is painted
#else
    QPainter painter(this);
#endif

    //painter.setRenderHint(QPainter::HighQualityAntialiasing, true);
    painter.setRenderHint(QPainter::Antialiasing,true);
    painter.setRenderHint( QPainter::TextAntialiasing,true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform,true);

    QRect fillRectangle(event->rect().x()*RETINA_SUPPORT, event->rect().y()*RETINA_SUPPORT, event->rect().width()*RETINA_SUPPORT, event->rect().height()*RETINA_SUPPORT);
    painter.fillRect(fillRectangle, Qt::white);
    // painter.beginNativePainting();

    // setup the painter
#ifdef Q_OS_MAC
    QFont font("Monospace", GLscale/18.0f*1.3f);
    font.setPointSizeF(GLscale/18.0f*1.3f);
#else
    QFont font("Monospace", GLscale/18.0f);
    font.setPointSizeF(GLscale/18.0f);
#endif
    font.setStyleHint(QFont::TypeWriter);
    painter.setPen(QColor(0,0,0,255));
    painter.setFont(font);

    // if grid then draw it up:
#ifndef Q_OS_MAC
    if (this->gridSelect) {
        painter.save();
        painter.translate((this->width()*RETINA_SUPPORT)/2, (this->height()*RETINA_SUPPORT)/2);
        //painter.scale(,-GLscale/2.0);
        painter.translate(viewX*(GLscale/2.0), viewY*(GLscale/2.0));
        // too fine a grid looks bad and is of no use
        QPen pen = painter.pen();
        pen.setWidth(1.5*RETINA_SUPPORT);
        painter.setPen(pen);
        if (GLscale > 10) {
            for (float i = round(-viewX/this->gridScale)*this->gridScale*GLscale; i < round(-viewX/this->gridScale)*this->gridScale+(this->width()*RETINA_SUPPORT); i +=this->gridScale*GLscale/2.0) {
                for (float j =round(viewY/this->gridScale)*this->gridScale*GLscale; j < round(viewY/this->gridScale)*this->gridScale+(this->height()*RETINA_SUPPORT); j +=this->gridScale*GLscale/2.0) {
                    painter.drawPoint(QPointF(i,j));
                }
            }
            for (float i = round(-viewX/this->gridScale)*this->gridScale*GLscale; i > round(-viewX/this->gridScale)*this->gridScale-(this->width()*RETINA_SUPPORT); i -=this->gridScale*GLscale/2.0) {
                for (float j =round(viewY/this->gridScale)*this->gridScale*GLscale; j > round(viewY/this->gridScale)*this->gridScale-(this->height()*RETINA_SUPPORT); j -=this->gridScale*GLscale/2.0) {
                    painter.drawPoint(QPointF(i,j));
                }
            }
            for (float i = round(-viewX/this->gridScale)*this->gridScale*GLscale; i > round(-viewX/this->gridScale)*this->gridScale-(this->width()*RETINA_SUPPORT); i -=this->gridScale*GLscale/2.0) {
                for (float j =round(viewY/this->gridScale)*this->gridScale*GLscale; j < round(viewY/this->gridScale)*this->gridScale+(this->height()*RETINA_SUPPORT); j +=this->gridScale*GLscale/2.0) {
                    painter.drawPoint(QPointF(i,j));
                }
            }
            for (float i = round(-viewX/this->gridScale)*this->gridScale*GLscale; i < round(-viewX/this->gridScale)*this->gridScale+(this->width()*RETINA_SUPPORT); i +=this->gridScale*GLscale/2.0) {
                for (float j =round(viewY/this->gridScale)*this->gridScale*GLscale; j > round(viewY/this->gridScale)*this->gridScale-(this->height()*RETINA_SUPPORT); j -=this->gridScale*GLscale/2.0) {
                    painter.drawPoint(QPointF(i,j));
                }
            }
        }
        painter.restore();
    }
#endif

    // send the painter, and the transforms needed to paint to the right place
    emit reDraw(&painter, GLscale, viewX, viewY, (this->width()*RETINA_SUPPORT), (this->height()*RETINA_SUPPORT), standardDrawStyle);

    //painter.endNativePainting();
    painter.end();

#ifdef Q_OS_MAC
    QImage gldata = QGLWidget::convertToGLFormat(workaround_for_slow_osx_drawing);
    glDrawPixels((this->width()*RETINA_SUPPORT), (this->height()*RETINA_SUPPORT), GL_RGBA, GL_UNSIGNED_BYTE, gldata.bits());

    glSwapAPPLE();
#endif
}

void GLWidget::move(GLfloat x, GLfloat y)
{
    this->viewX =x;
    this->viewY =-y;
    //emit reDraw();
}

void GLWidget::zoomOut()
{
    this->targGLscale *= 2.0;
}

void GLWidget::zoomIn()
{
    this->targGLscale /= 2.0;
}

void GLWidget::startConnect()
{
    this->connectMode = true;
    this->setMouseTracking(true);
}

void GLWidget::finishConnect()
{
    this->connectMode = false;
    this->setMouseTracking(false);

    changed = 100;
}

void GLWidget::saveImage()
{
    //this->makeCurrent();
    //QPixmap pic = this->renderPixmap((this->width()*RETINA_SUPPORT)*4,(this->height()*RETINA_SUPPORT)*4, false);
    //QImage image = pic.toImage();
    //image.save("/home/mp/test_image.png","png");
}
