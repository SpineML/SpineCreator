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
**  Modern OpenGL updates, 2019-2020: Seb James                           **
**  Website/Contact: github                                               **
****************************************************************************/

#ifndef GLCONNECTIONWIDGET_H
#define GLCONNECTIONWIDGET_H

#include "globalHeader.h"
#include "SC_logged_data.h"

#include <QOpenGLWidget>
#include <QOpenGLContext>

#include "NeuronScene.h"

#if 0
//! Homemade random number generator
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
#endif

//! Local population locations class, used to help lay out the visualization
struct popLocs
{
    QVector < loc > locations;
    QString name;
    float x;
    float y;
    float z;
};

/*!
 * A visualization widget for neurons and projections
 *
 * glConnectionWidget shows a 3D model of (selected) neuron populations and projections.
 *
 * Originally written in OpenGL 2 code, it has now (Sept. 2020) been rewritten in OpenGL
 * 4.1, with the vertices of the neuron models being computed on the CPU and then GLSL
 * shaders doing the graphics heavy lifting.
 */
class glConnectionWidget : public QOpenGLWidget
{
    Q_OBJECT
public:
    explicit glConnectionWidget (nl_rootdata * data, QWidget *parent = 0);
    ~glConnectionWidget ();
    //! Populations selected for visualization
    QVector<QSharedPointer<population> > selectedPops;
    //! The relative location of each population within the visualization is additional information for the model.
    QVector<popLocs> pops;
    //! Connections selected for visualization
    QVector<QSharedPointer<systemObject> > selectedConns;
    //! 2D sheet of locations. Related to the x/y/z spinboxes in the right hand "panel", I think.
    QVector<QVector<loc> > locations; // temp readded
#if 0
    QVector<QColor> cols;
#endif
    //! A container of connections. This exists here because the connections can be modified by this user interface (I think).
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
    //!
    QString currentObjectName;
    //! This is a 'connection model', I think
    QAbstractTableModel* model;
    //!
    QAbstractItemModel* sysModel;
    //!
    QModelIndexList selection;
    //!
    Qt::MouseButton button;
    //!
    connectionType currProjectionType;
#if 0
    //! Don't think we need this.
    RNG random;
#endif
    //! This points to the SpineML model which is being (partially) visualised.
    nl_rootdata* data;
    //! Another spatial offset...
    loc loc3Offset;
    //! A selected object. Selected by what?
    QSharedPointer<systemObject> selectedObject;
    //! Indexes the currently selected neuron in a population. Probably.
    int selectedIndex;
    //!
    int selectedType;
    //!
    //QString last_distance_based_equation;
    //!
    QMutex * connGenerationMutex;
    //! Are we in 'save the image mode'?
    bool imageSaveMode;
    //! Width, in pixels of the image to save
    int imageSaveWidth;
    //! Height, in pixels of the image to save
    int imageSaveHeight;
    //! Holds the colour information for the different neuron populations. These are the
    //! colours that the user set in the network view of the SpineCreator interface.
    QVector<QVector<QColor> > popColours;
    //! Holds information about the activity of populations whilst the simulation was
    //! running. I think. Allows for animations to be made.
    QVector<logData*> popLogs;
    //! Used to make sure not to collect too much data from the logs
    int currentLogTime;
    //! Used to make sure not to collect too much data from the logs
    int newLogTime;
    //! A timer used to update log information. Every 50 ms (hardcoded) updateLogData() is called
    QTimer timer;
    //! A switch between orthographic and normal projections. May avoid coding this.
    bool orthoView;
#if QT_VERSION > QT_VERSION_CHECK(5, 0, 0)
    QImage renderQImage(int w, int h);
#endif

signals:
    //! Possibly unused
    void currElement(int type, int index);
    //! Get information about the source neurons of the currently selected neuron. At a guess.
    void getNeuronLocationsSrc (QVector<QVector<loc> >* locn,
                                QVector<QColor>* colr,
                                QString name = "");
    //! Possibly unused
    void updatePanel(QString);
    //! Possibly used
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
    //! Called by external code to update the log data time. This occurs when the "time
    //! slider" is moved, requesting a view of the simulation and its actvity at a
    //! specific index
    void updateLogDataTime(int index) { this->newLogTime = index; }
    //! Update information from the Simulator log to show in the "playback" for the visualization.
    void updateLogData();
    //! Self-explanatory
    void toggleOrthoView (bool);

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

    //! Set true if the model should be set up on the next paintGL call. This ensures
    //! that when the CPU-side work of setting the model up is called, the correct GL
    //! context is current.
    bool setupModelRequired = false;

    // UI interaction methods
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);

private:
    //! A Qt wrapper referring to the GLSL shader program
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
    //! representing axonal connections between the neurons. This is the container for
    //! the CPU-side information about the objects that will be rendered by OpenGL.
    NeuronScene* nscene = (NeuronScene*)0;
};

#endif // GLCONNECTIONWIDGET_H
