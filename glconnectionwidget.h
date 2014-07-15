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
**           Author: Alex Cope                                            **
**  Website/Contact: http://bimpa.group.shef.ac.uk/                       **
****************************************************************************/

#ifndef GLCONNECTIONWIDGET_H
#define GLCONNECTIONWIDGET_H

#include "globalHeader.h"
#include "logdata.h"

class RNG
{
public:
    RNG() {a_RNG = 1103515245; c_RNG = 12345;}
    void setSeed(int seedIn) {seed = seedIn;}
    float value() {
        seed = abs(seed*a_RNG+c_RNG);
        float seed2 = seed/2147483648.0;
        return seed2;
    }

private:
    int seed;
    int a_RNG;
    int c_RNG;

};

struct popLocs {

    vector < loc > locations;
    QString name;
    float x;
    float y;
    float z;

};

struct loc3f {
    float x;
    float y;
    float z;
};

class glConnectionWidget : public QGLWidget
{
    Q_OBJECT
public:
    explicit glConnectionWidget(rootData * data, QWidget *parent = 0);
    vector <QSharedPointer <population> > selectedPops;
    vector < popLocs> pops;
    vector <QSharedPointer<systemObject> > selectedConns;
    vector < vector < loc > > locations; // temp readded
    vector < QColor > cols;
    vector < vector < conn > > connections;
    void setConnectionsModel(QAbstractTableModel *);
    QAbstractTableModel * getConnectionsModel();
    void getConnections();
    void setConnType(connectionType cType);
    float prob;
    int seed;
    bool popIndicesShown;
    void clear();
    QPixmap renderImage(int, int);
    void addLogs(QVector<logData *> *logs);
    void refreshAll();

private:
    void drawNeuron(GLfloat, int, int, QColor);
    void setupView();
    QString currentObjectName;
    QAbstractTableModel * model;
    QAbstractItemModel * sysModel;
    QModelIndexList selection;
    float zoomFactor;
    QPointF pos;
    QPointF rot;
    QPointF origPos;
    QPointF origRot;
    Qt::MouseButton button;
    connectionType currProjectionType;
    RNG random;
    rootData * data;
    loc3f loc3Offset;
    QSharedPointer<systemObject> selectedObject;
    int selectedIndex;
    int selectedType;
    QString last_distance_based_equation;
    QMutex * connGenerationMutex;
    bool imageSaveMode;
    int imageSaveWidth;
    int imageSaveHeight;
    vector < vector < QColor > > popColours;
    vector < logData * > popLogs;
    int currentLogTime;
    int newLogTime;
    QTimer timer;
    bool orthoView;
    bool repaintAllowed;
#if QT_VERSION > QT_VERSION_CHECK(5, 0, 0)
    QImage renderQImage(int w, int h);
#endif

signals:
    void currElement(int type, int index);
    //void reDraw(QPainter*, float, float, float, int, int);
    void getNeuronLocationsSrc(vector < vector <loc> > *, vector < QColor > *, QString name = "");
    void updatePanel(QString);
    void setSelectionbyName(QString);
    
public slots:
    void redraw();
    void redraw(int);
    void selectionChanged(QItemSelection top, QItemSelection);
    void parsChangedPopulation(double);
    void parsChangedPopulation(int);
    void parsChangedPopulation();
    void parsChangedProjection();
    void parsChangedProjections();
    void typeChanged(int);
    void drawLocations(vector <loc> locs);
    void clearLocations();
    void connectionDataChanged(QModelIndex, QModelIndex);
    void connectionSelectionChanged(QItemSelection,QItemSelection);
    void sysSelectionChanged(QModelIndex, QModelIndex);
    void setPopIndicesShown(bool);
    void selectedNrnChanged(int);
    void updateLogDataTime(int index);
    void updateLogData();
    void toggleOrthoView(bool);
    void allowRepaint();

protected:
    void initializeGL();
    void paintEvent(QPaintEvent *);
    void resizeGL(int width, int height);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);

};

#endif // GLCONNECTIONWIDGET_H
