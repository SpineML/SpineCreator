/***************************************************************************
**                                                                        **
**  This file is part of SpineCreator, an easy to use GUI for             **
**  describing spiking neural network models.                             **
**  Copyright (C) 2013-2019 Alex Cope, Paul Richmond, Seb James           **
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
**  Modern OpenGL updates, 2019: Seb James                                **
**  Website/Contact: http://bimpa.group.shef.ac.uk/                       **
****************************************************************************/

#ifndef GLCONNECTIONWIDGET_H
#define GLCONNECTIONWIDGET_H

#include "globalHeader.h"
#include "SC_logged_data.h"

#include <QOpenGLWidget>
#include <QOpenGLContext>

#include "NeuronScene.h"

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

struct popLocs
{
    QVector < loc > locations;
    QString name;
    float x;
    float y;
    float z;
};

class glConnectionWidget : public QOpenGLWidget
{
    Q_OBJECT
public:
    explicit glConnectionWidget (nl_rootdata * data, QWidget *parent = 0);
    ~glConnectionWidget ();
    //! Populations selected for visualization
    QVector<QSharedPointer<population> > selectedPops;
    QVector<popLocs> pops;
    //! Connections selected for visualization
    QVector<QSharedPointer<systemObject> > selectedConns;

    // 2D sheet of locations
    QVector<QVector<loc> > locations; // temp readded

    QVector<QColor> cols;
    QVector<QVector<conn> > connections;

    void setConnectionsModel(QAbstractTableModel *);
    QAbstractTableModel * getConnectionsModel();
    void getConnections();
    void setConnType(connectionType cType);
    float prob;
    int seed;
    bool popIndicesShown;
    void clear();
    QPixmap renderImage (int, int);
    void addLogs (QVector<logData*>* logs);
    //! after switching views, carry out a number of checks on the populations etc
    void refreshAll();

private:
    QString currentObjectName;
    QAbstractTableModel * model;
    QAbstractItemModel * sysModel;
    QModelIndexList selection;
    Qt::MouseButton button;
    connectionType currProjectionType;
    RNG random;
    nl_rootdata* data;
    loc loc3Offset;
    QSharedPointer<systemObject> selectedObject;
    int selectedIndex;
    int selectedType;
    QString last_distance_based_equation;
    QMutex * connGenerationMutex;
    bool imageSaveMode;
    int imageSaveWidth;
    int imageSaveHeight;
    QVector<QVector<QColor> > popColours;
    QVector<logData*> popLogs;
    int currentLogTime;
    int newLogTime;
    QTimer timer;
    bool orthoView;
#if QT_VERSION > QT_VERSION_CHECK(5, 0, 0)
    QImage renderQImage(int w, int h);
#endif

signals:
    void currElement(int type, int index);
    void getNeuronLocationsSrc (QVector<QVector<loc> >* locn,
                                QVector<QColor>* colr,
                                QString name = "");
    void updatePanel(QString);
    void setSelectionbyName(QString);

public slots:
    //! Called when 3D location spinboxes are changes (for x,y,z positions). Subcalls
    //! redraw(void)
    void redraw (int);
    void redraw();

    //! Called when the populations/projections selected for visualization changed
    void selectionChanged (QItemSelection top, QItemSelection);

    //! Parameters changed for the population. E.g. the number of neurons was modified
    void parsChangedPopulation(double);
    //! Parameters changed for the population. E.g. the number of neurons was modified
    void parsChangedPopulation(int);
    //! Parameters changed for the population. E.g. the number of neurons was modified
    void parsChangedPopulation();

    //! Parameters changed a projection. E.g. as a result of num of neurons in pop. changing
    void parsChangedProjection();
    //! Parameters changed for projections. E.g. as a result of num of neurons in pop. changing
    void parsChangedProjections();

    void typeChanged(int);

    void drawLocations (QVector<loc> locs);
    void clearLocations();
    void connectionDataChanged(QModelIndex, QModelIndex);
    void connectionSelectionChanged(QItemSelection,QItemSelection);
    void sysSelectionChanged(QModelIndex, QModelIndex);
    void setPopIndicesShown(bool);
    void selectedNrnChanged(int);
    void updateLogDataTime(int index);
    void updateLogData();

    void toggleOrthoView(bool);

protected:
    //! GL rendering setup
    void initializeGL() override;
    //! Paint stuff
    void paintGL() override;
    //! Resize override. Just calls setPerspective.
    void resizeGL (int w, int h) override;

    //! Compute the current view of the model - perform the relevant translations and so
    //! on. Updates whenever the user moves the scene using mouse interaction
    //! events. Call this when the window is resized, too.
    void setPerspective (int w, int h);

    //! Set up the "model" where "model" in this context means the vertices that make up
    //! the triangles which the graphics system will render. So in here, we compute
    //! spheres, lines and so on.
    void setupModel (void);

    //! Set true if the model should be set up on the next paintGL call
    bool setupModelRequired = false;

    // UI interaction methods
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);

private:
    //! A Qt version of the shader program
    QOpenGLShaderProgram* shaderProg;
    //! Current rotational state of the neuron view model
    QQuaternion rotation;
    //! A stored rotation
    QQuaternion savedRotation;
    //! The scene *translation*
    QVector3D scenetrans = QVector3D(0,0,-10);
    //! The default scene *translation*
    QVector3D scenetrans_default = QVector3D(0,0,-10);
    //! How big should the steps in scene translation be when scrolling?
    float scenetrans_stepsize = 0.1;
    //! When true, cursor movements induce rotation of scene
    bool rotateMode = false;
    //! When true, cursor movements induce translation of scene
    bool translateMode = false;
    //! Screen coordinates of the position of the last mouse press
    QVector2D mousePressPosition;
    //! Current mouse position. Could be a QPointF.
    QVector2D cursorpos;
    //! The current rotation axis. World frame?
    QVector3D rotationAxis;

    //! Projection matrix for the neuron view model
    QMatrix4x4 projMatrix;
    //! The inverse projection
    QMatrix4x4 invproj;
    //! The scene transformation
    QMatrix4x4 sceneMatrix;
    //! The inverse scene transformation
    QMatrix4x4 invscene;
    //! The "neuron scene" - a scene of spheres representing neurons, and lines
    //! representing axonal connections between the neurons.
    NeuronScene* nscene = (NeuronScene*)0;
};

#endif // GLCONNECTIONWIDGET_H
