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

#ifndef NETVIEWWIDGET_H
#define NETVIEWWIDGET_H

#include "globalHeader.h"

#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE 0x809D
#endif

class NetViewWidget : public QWidget
{
    Q_OBJECT

public:
    explicit NetViewWidget(QWidget *parent = 0);
    ~NetViewWidget();
    void move(float, float);

    int changed;
    float viewX;
    float viewY;
    QImage popTex;
    bool gridSelect;
    bool addSelection;
    float gridScale;
    Qt::MouseButton button;

signals:
    void currElement(int type, int index);
    void reDraw(QPainter*, float, float, float, int, int, drawStyle);
    void onLeftMouseDown(float _x, float _y, float _myScale, bool shiftDown);
    void onRightMouseDown(float _x, float _y, float _myScale);
    void mouseMove(float _x, float _y);
    void drawSynapse(float _x, float _y);
    void addBezierOrProjection(float _x, float _y);
    void abortProjection();
#ifdef NEED_MOUSE_UP_LOGIC
    void onLeftMouseUp(float _x, float _y, float _myScale);
#endif
    void itemWasMoved();
    void dragSelect(float _x, float _y);
    void endDragSelect();

public slots:
    void animate();
    void zoomOut();
    void zoomIn();
    void startConnect();
    void finishConnect();
    void redrawNetView();
    void saveImage();

protected:
    void initializeGL();
    void paintEvent(QPaintEvent * event);
    void resizeGL(int, int);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
    void keyPressEvent(QKeyEvent * event);
    void keyReleaseEvent(QKeyEvent * event);

private:
    int currSelType;
    int currSelInd;
    float myScale;
    float targmyScale;
    bool connectMode;
    // on some windows laptops we can't RMB drag, so this allows SHIFT-LMB dragging
    bool LMB_drag;

    /*!
     * Is an item being dragged around the screen? Used in combination
     * with mouseReleaseEvents. Currently, the value of this attribute
     * is only effective when connectMode==false
     */
    bool itemMoving;
};

#endif // NETVIEWWIDGET_H
