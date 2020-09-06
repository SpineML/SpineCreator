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

#include "SC_network_3d_visualiser_panel.h"
#include "SC_connectionmodel.h"
#include "SC_systemmodel.h"

#if 0 // Qt OpenGL includes should automatically handle GL includes.
# ifdef Q_OS_MAC
#  include "glu.h"
# else
#  include "GL/glu.h"
# endif
#endif

#include "SC_python_connection_generate_dialog.h"
#include "mainwindow.h"

#if QT_VERSION > QT_VERSION_CHECK(5, 0, 0)
# include <QOpenGLFramebufferObject>
#endif

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
# define RETINA_SUPPORT 1.0
#else
# ifndef Q_OS_GLN
#  define RETINA_SUPPORT this->windowHandle()->devicePixelRatio()
# else
#  define RETINA_SUPPORT 1.0
# endif
#endif

#include <limits>


glConnectionWidget::glConnectionWidget(nl_rootdata* _data, QWidget *parent)
    : QOpenGLWidget(parent)
{
    model = (QAbstractTableModel *)0;
    pos = QPointF(0,0);
    zoomFactor = 1.0;
    this->data = _data;
    setAutoFillBackground (false);
    popIndicesShown = false;
    selectedIndex = 0;
    selectedType = 1;
    connGenerationMutex = new QMutex;
    imageSaveMode = false;

    connect (&timer, SIGNAL(timeout()), this, SLOT(updateLogData()));

    timer.start(50);
    newLogTime = 0;
    currentLogTime = 0;

    orthoView = false;
}

glConnectionWidget::~glConnectionWidget()
{
    if (this->shaderProg != (QOpenGLShaderProgram*)0) {
        delete this->shaderProg;
    }
    if (this->nscene != (NeuronScene*)0) {
        delete this->nscene;
    }
}

void
glConnectionWidget::initializeGL()
{
    if (this->context()->isValid()) {
        qDebug() << "Context is valid.";
    } else {
        qDebug() << "Context is INvalid.";
    }
    QOpenGLFunctions *f = this->context()->functions();
    f->glClearColor (0.8f, 0.7f, 0.8f, 1.0f);

    // initialize shaders
    this->shaderProg = new QOpenGLShaderProgram();

    // Add shaders from files, making it easier to read/modify the shader code
    if (!this->shaderProg->addShaderFromSourceFile (QOpenGLShader::Vertex,
                                                    "/home/seb/src/SpineCreator/vshader.glsl")) {
        close();
    }
    if (!this->shaderProg->addShaderFromSourceFile (QOpenGLShader::Fragment,
                                                    "/home/seb/src/SpineCreator/fshader.glsl")) {
        close();
    }

    // link and bind Shader (Do not release until VAO is created)
    if (!this->shaderProg->link()) { close(); }
    if (!this->shaderProg->bind()) { close(); }

    // Enable depth buffer
    f->glEnable(GL_DEPTH_TEST);

    // Enable back face culling. That means you can't rotate the object with a rotn matrix though.
    //f->glEnable(GL_CULL_FACE);

    // Do we create the scene here? Probably not, as we may not actually HAVE a scene yet
    // Create the scene. This creates several spherelayers, each containing VAO and VBOs
    this->nscene = new NeuronScene (this->shaderProg, f);

    // Now VAOs were created in scene object, release shaderProg
    this->shaderProg->release();

    // Set the perspective
    this->setPerspective (this->width(), this->height());
}

void
glConnectionWidget::toggleOrthoView (bool toggle)
{
    this->orthoView = toggle;
    this->repaint();
}

void
glConnectionWidget::clear()
{
    selectedPops.clear();
    popColours.clear();
    popLogs.clear();
    selectedConns.clear();
    connections.clear();
    selectedIndex = 0;
    selectedType = 1;
    model = (QAbstractTableModel *)0;
}

// This builds a list of possible logs from the populations in the network.
void
glConnectionWidget::addLogs (QVector<logData*>* logs)
{
    // for each population
    for (int i = 0; i < selectedPops.size(); ++i) {

        QSharedPointer<population> pop = selectedPops[i];

        // for each analog output port
        for (int j = 0; j < pop->neuronType->component->AnalogPortList.size(); ++j) {

            AnalogPort * port = pop->neuronType->component->AnalogPortList[j];
            // if send port
            if (port->mode == AnalogSendPort) {

                // construct log name! This should be replaced by XML data from the log
                QString possibleLogName = pop->name + "_" + port->name + "_log.bin";
                possibleLogName.replace(" ", "_");

                // check each log in turn
                for (int k = 0; k < logs->size(); ++k) {
                    if ((*logs)[k]->logName == possibleLogName) {
                        this->popLogs[i] = (*logs)[k];
                    }
                }
            }
        }
    }
}

void
glConnectionWidget::updateLogDataTime(int index)
{
    newLogTime = index;
}

void
glConnectionWidget::updateLogData()
{
    if (newLogTime == currentLogTime)
        return;

    currentLogTime = newLogTime;

    // fetch data from logs
    for (int i = 0; i < popLogs.size(); ++i) {

        // skip where there is no log
        if (popLogs[i] == NULL)
            continue;

        // get a row
        QVector < double > logValues = popLogs[i]->getRow(currentLogTime);

        // data not usable
        if (logValues.size() == 0)
            continue;
        if ((int) logValues.size() > selectedPops[i]->numNeurons)
            continue;

        // resize container
        QColor col(0,0,0,255);
        popColours[i].resize(selectedPops[i]->numNeurons);
        popColours[i].fill(col);

        // remap data
        for (int j = 0; j < logValues.size(); ++j) {
            if (logValues[j] < Q_INFINITY && (popLogs[i]->getMax()-popLogs[i]->getMin()) != 0) {
                int val = ((logValues[j]-popLogs[i]->getMin())*255.0)/(popLogs[i]->getMax()-popLogs[i]->getMin());
                val *= 3;
                // complete the remap in just 4 ternarys
                int val3 = val > 511 ? val-512 : 0;
                int val2 = val3 > 0 ? 511 : val;
                val2 = val2 > 255 ? val2 - 256 : 0;
                int val1 = val < 255 ? val : 255;

                popColours[i][j] = QColor(val1,val2, val3, 255);
            }
        }
    }

    // redraw!
    this->repaint();
}

void
glConnectionWidget::resizeGL (int w, int h)
{
    this->setPerspective (w, h);
}

void
glConnectionWidget::redraw()
{
    DBG() << "Called";
    // refetch layout of current selection
    if (selectedObject != NULL) {
        if (selectedObject->type == populationObject) {
            QString errs;
            QSharedPointer <population> currPop = qSharedPointerDynamicCast <population> (selectedObject);
            CHECK_CAST(currPop)
            currPop->layoutType->locations.clear();
            currPop->layoutType->generateLayout(currPop->numNeurons,&currPop->layoutType->locations,errs);
        }
    }
    DBG() << "Calling repaint(). Probably needs to be something else";
    this->repaint();
}

void
glConnectionWidget::redraw(int)
{
    DBG() << "redraw(int) called";
    // we haven't updated the underlying data yet - but we want to show spinbox changes
    // get spinbox ptrs:
    QObject * temp = (QObject *) sender()->property("xptr").value<void *>();
    QSpinBox * xSpin = qobject_cast<QSpinBox *> (temp);
    CHECK_CAST(xSpin)
    temp = (QObject *) sender()->property("yptr").value<void *>();
    QSpinBox * ySpin = qobject_cast<QSpinBox *> (temp);
    CHECK_CAST(ySpin)
    temp = (QObject *) sender()->property("zptr").value<void *>();
    QSpinBox * zSpin = qobject_cast<QSpinBox *> (temp);
    CHECK_CAST(zSpin)

    loc3Offset.x = xSpin->value();
    loc3Offset.y = ySpin->value();
    loc3Offset.z = zSpin->value();

    this->repaint();
}

void
glConnectionWidget::paintGL()
{
    DBG() << "paintGL called";

    QOpenGLFunctions *f = this->context()->functions();

    // Put the render code in here. Here's what's in shapewindow.cpp
    // in my working example, but I'll probbaly distribute this out
    // amoungst paintGL etc.
    const qreal retinaScale = devicePixelRatio();
    f->glViewport (0, 0, this->width() * retinaScale, this->height() * retinaScale);

    // Set the perspective from the width/height
    this->setPerspective (this->width(), this->height());

    // Calculate model view transformation
    QMatrix4x4 rotmat;
    rotmat.translate (0.0, 0.0, -3.50); // send backwards into distance
    rotmat.rotate (this->rotation);

    // Bind shader program...
    this->shaderProg->bind();

    // Set modelview-projection matrix
    this->shaderProg->setUniformValue ("mvp_matrix", this->projMatrix* rotmat);

    // Clear color buffer and **also depth buffer**
    f->glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    this->nscene->render();

    // ...and release the shaderProg
    this->shaderProg->release();
}


void
glConnectionWidget::setupModel (void)
{
    DBG() << "Called";
    // This modifies this->nscene; a NeuronScene object which contains
    // some number of SphereLayer objects and then I guess the lines
    // between them. Currently, these are LinesLayer and SphereLayer
    // objects. These are very closely linked to conn and population
    // objects and could potentially be members of those classes -
    // they need to obtain information from those classes in order to
    // render the locations of the spheres and their colours.
}

void
glConnectionWidget::setPerspective (int w, int h)
{
    DBG() << "Called. imageSaveMode: " << imageSaveMode << ", orthoView: " << orthoView;

#ifdef NEED_IMAGE_SAVEMODE // Perhaps QOpenGLWidget not ideal for image saving?
    if (this->imageSaveMode) {
        w = this->imageSaveWidth;
        h = this->imageSaveHeight;
    } else {
        // Do nothing, use passed-in w and h
    }
#endif
    // Reset projection
    this->projMatrix.setToIdentity();

    const float zNear = 1.0, zFar = 100000.0, fov = 60.0;
    if (!this->orthoView) {
        // Set perspective projection
        float aspect = float(w) / float(h ? h : 1);
        this->projMatrix.perspective (fov, aspect, zNear, zFar);
    } else {
        // or orthographic like this:
        float scale = this->zoomFactor * 10.0f;
        float aspect = float(h) / float(w ? w : 1);
        this->projMatrix.ortho (-scale, scale, -scale*aspect, scale*aspect, -100, zFar);
    }
}

void
glConnectionWidget::selectionChanged (QItemSelection top, QItemSelection)
{
#define SHOULD_RESET_NRN_INDEX 1 // For now...
#ifdef SHOULD_RESET_NRN_INDEX // Probably need to check if
                              // selectedIndex is off the end of the
                              // population here, but otherwise, keep
                              // the same selected index. Can be
                              // useful when comparing two similar
                              // sized populations
    // reset nrn index etc...
    selectedType = 1;
    selectedIndex = 0;
#endif

    // cancel current selection
    selectedObject.clear();

    // look up the selected item
    QModelIndexList indices = top.indexes();
    TreeItem *item = static_cast<TreeItem*>(indices[0].internalPointer());

    for (int i = 0; i < data->populations.size(); ++i) {

        QSharedPointer <population> currPop = qSharedPointerDynamicCast<population> (data->populations[i]);
        CHECK_CAST(currPop)

        // populations
        if (currPop->getName() == item->name) {
            this->selectedObject = currPop;
            // reset currPop offset
            this->loc3Offset.x = currPop->loc3.x;
            this->loc3Offset.y = currPop->loc3.y;
            this->loc3Offset.z = currPop->loc3.z;
        }

        // population inputs
        for (int output = 0; output < data->populations[i]->neuronType->outputs.size(); ++output) {

            QSharedPointer<genericInput> currOutput = data->populations[i]->neuronType->outputs[output];

            // add Synapse
            if (!currOutput->projInput)
            {
                if("Output from " + currOutput->source->getName() + " to " + currOutput->destination->getName() + " port " + currOutput->dstPort + " " + QString::number(output) == item->name) {
                    selectedObject = currOutput;
                }
            }
        }

        // projections
        for (int j = 0; j < currPop->projections.size(); ++j) {

            QSharedPointer<projection> currProj = qSharedPointerDynamicCast<projection> (currPop->projections[j]);
            CHECK_CAST(currProj)

            if (currProj->getName() == item->name) {
                selectedObject = currProj;
            }

            // synapses
            for (int k = 0; k < currProj->synapses.size(); ++k) {

                QSharedPointer <synapse> currTarg = qSharedPointerDynamicCast<synapse> (currProj->synapses[k]);
                CHECK_CAST(currTarg)

                if (currProj->getName() + ": Synapse " + QString::number(k) == item->name) {
                    selectedObject = currTarg;
                }
            }
        }
    }

    this->repaint();
}

void
glConnectionWidget::typeChanged(int)
{
    QString type = sender()->property("type").toString();

    if (type == "layout") {

        if (selectedObject->type != populationObject) {
            qDebug() << "WARNING - bug found - report code 1";
        }

        // find the selected object and get the locations, then force a redraw
        QString errs;
        QSharedPointer <population> currPop = qSharedPointerDynamicCast <population> (selectedObject);
        currPop->layoutType->locations.clear();
        currPop->layoutType->generateLayout(currPop->numNeurons,&currPop->layoutType->locations,errs);
        if (!errs.isEmpty()) {
            this->data->updateStatusBar(errs,2000);
        }
        //this->currProjectionType = none;
    }
    if (type == "conn") {
        // find the locations of the src and dst:
    }

    this->repaint();
}

void
glConnectionWidget::parsChangedPopulation(int value)
{
    // if the selected population is in the list (i.e. checked) then refetch locations
    for (int i = 0; i < selectedPops.size(); ++i) {
        if (selectedObject == selectedPops[i]) {

            QSharedPointer <population> currPop = qSharedPointerDynamicCast <population> (selectedObject);
            QString errs;
            currPop->layoutType->locations.clear();
            currPop->layoutType->generateLayout(value,&currPop->layoutType->locations,errs);
            // display all errors
            if (!errs.isEmpty()) {
                this->data->updateStatusBar(errs,2000);
            }

            // invalidate logs as size of pop has changed
            popLogs[i] = NULL;
            popColours[i].clear();
        }
    }

    this->repaint();
}

void
glConnectionWidget::parsChangedPopulation(double)
{
    // if the selected population is in the list (i.e. checked) then refetch locations
    for (int i = 0; i < selectedPops.size(); ++i) {
        if (selectedObject == selectedPops[i]) {

            QSharedPointer <population> currPop = qSharedPointerDynamicCast <population> (selectedObject);
            QString errs;
            currPop->layoutType->locations.clear();
            currPop->layoutType->generateLayout(currPop->numNeurons,&currPop->layoutType->locations,errs);
            // display all errors
            if (!errs.isEmpty()) {
                //this->data->statusBarUpdate(errs,2000);
            }
        }
    }

    this->repaint();
}

void
glConnectionWidget::parsChangedPopulation()
{
    // if the selected population is in the list (i.e. checked) then refetch locations
    for (int i = 0; i < selectedPops.size(); ++i) {
        if (selectedObject == selectedPops[i]) {

            QSharedPointer <population> currPop = qSharedPointerDynamicCast <population> (selectedObject);
            QString errs;
            currPop->layoutType->locations.clear();
            currPop->layoutType->generateLayout(currPop->numNeurons,&currPop->layoutType->locations,errs);
            // display all errors
            if (!errs.isEmpty()) {
                //this->data->statusBarUpdate(errs,2000);
            }
        }
    }

    this->repaint();
}

void
glConnectionWidget::parsChangedProjections()
{
    this->refreshAll();

    for (int i = 0; i < selectedConns.size(); ++i) {

        connection * conn;

        if (selectedConns[i]->type == synapseObject) {
            QSharedPointer <synapse> currTarg = qSharedPointerDynamicCast <synapse> (selectedConns[i]);
            conn = currTarg->connectionType;
        } else {
            QSharedPointer<genericInput> currIn = qSharedPointerDynamicCast<genericInput> (selectedConns[i]);
            conn = currIn->conn;
        }

        // regrab data for python based
        if (conn->type == Python) {

            // refresh the connections
            if (((pythonscript_connection *) conn)->changed()) {
                connections[i].clear();
                // launch version increment dialog box:
                generate_dialog generate(((pythonscript_connection *) conn), ((pythonscript_connection *) conn)->srcPop, ((pythonscript_connection *) conn)->dstPop, connections[i], connGenerationMutex, this);
                bool retVal = generate.exec();
                if (!retVal) {
                    return;
                }
                ((pythonscript_connection *) conn)->connections = connections[i];
                ((pythonscript_connection *) conn)->setUnchanged(true);
            }
        }
    }

    repaint();
}

void
glConnectionWidget::parsChangedProjection()
{
    // can only be the current selection
    if (selectedObject->type == synapseObject || selectedObject->type == inputObject) {

        connection * conn;
        QSharedPointer <population> src;
        QSharedPointer <population> dst;

        if (selectedObject->type == synapseObject) {
            QSharedPointer <synapse> currTarg = qSharedPointerDynamicCast <synapse> (selectedObject);
            conn = currTarg->connectionType;
            src = currTarg->proj->source;
            dst = currTarg->proj->destination;
        } else {
            QSharedPointer<genericInput> currIn = qSharedPointerDynamicCast<genericInput> (selectedObject);
            conn = currIn->conn;
            src = qSharedPointerDynamicCast <population> (currIn->source); // would not be here if was not true
            dst = qSharedPointerDynamicCast <population> (currIn->destination);
        }

        if (conn->type == CSV) {
            // generated connection
            this->getConnections();
        }

        // regrab data for python script based
        if (conn->type == Python) {
            // update the projection:
            // find selected object
            for (int i = 0; i < this->selectedConns.size(); ++i) {

                if (selectedObject == selectedConns[i]) {

                    // refresh the connections
                    if (((pythonscript_connection *) conn)->changed()) {
                        connections[i].clear();
                        // launch version increment dialog box:
                        generate_dialog generate(((pythonscript_connection *) conn), src, dst, connections[i], connGenerationMutex, this);
                        bool retVal = generate.exec();
                        if (!retVal) {
                            return;
                        }
                        ((pythonscript_connection *) conn)->connections = connections[i];
                        ((pythonscript_connection *) conn)->setUnchanged(true);
                    }
                }
            }
        }

        repaint();
    }
}

// after switching views, check we haven't broken anything!
void
glConnectionWidget::refreshAll()
{
    DBG() << "Called";
    // check on populations
    for (int i = 0; i < selectedPops.size(); ++i) {

        QSharedPointer <population> currPop = selectedPops[i];

        // check it isn't deleted
        if (currPop->isDeleted) {
            // remove
            selectedPops.erase(selectedPops.begin()+i);
            popLogs.erase(popLogs.begin()+i);
            popColours.erase(popColours.begin()+i);
            --i;
            continue;
        }

        // refresh data if size has changed
        if ((int) currPop->layoutType->locations.size() != currPop->numNeurons) {
            QString errs;
            currPop->layoutType->locations.clear();
            currPop->layoutType->generateLayout(currPop->numNeurons,&currPop->layoutType->locations,errs);
            // display all errors
            if (!errs.isEmpty()) {
                this->data->updateStatusBar(errs,2000);
            }
        }
    }

    // check on projections
    for (int i = 0; i < selectedConns.size(); ++i) {
        // check it isn't deleted
        if (selectedConns[i]->isDeleted) {
            // remove
            selectedConns.erase(selectedConns.begin()+i);
            connections.erase(connections.begin()+i);
            --i;
            continue;
        }
    }
}

void
glConnectionWidget::setConnType(connectionType cType)
{
    this->currProjectionType = cType;
}

void
glConnectionWidget::drawLocations(QVector <loc> locs)
{
    // redraw based on a set of location passed in (used for layout previews)
    for (int i = 0; i < locations.size(); ++i) {
        locations[i].clear();
    }
    locations.clear();
    this->locations.push_back(locs);
    this->repaint();
}

void
glConnectionWidget::clearLocations()
{
    // clear the set of location passed in (used for layout previews)
    for (int i = 0; i < locations.size(); ++i) {
        locations[i].clear();
    }
    locations.clear();
}

void
glConnectionWidget::setConnectionsModel(QAbstractTableModel * modelIn)
{
    model = modelIn;
}

QAbstractTableModel *
glConnectionWidget::getConnectionsModel()
{
    return model;
}

void
glConnectionWidget::getConnections()
{
    if (selectedObject != NULL) {
        if (selectedObject->type == synapseObject) {

            QSharedPointer <synapse> currTarg = qSharedPointerDynamicCast <synapse> (selectedObject);

            for (int i = 0; i < this->selectedConns.size(); ++i) {

                if (selectedObject == selectedConns[i] && currTarg->connectionType->type == CSV) {
                    // refresh the connections
                    connections[i].clear();
                    ((csv_connection *) currTarg->connectionType)->getAllData(connections[i]);
                }
            }
        }
    }

    this->repaint();
}

void
glConnectionWidget::sysSelectionChanged(QModelIndex, QModelIndex)
{
    // this is fired when an item is checked or unchecked

    for (int i = 0; i < data->populations.size(); ++i) {

        QSharedPointer <population> currPop = (QSharedPointer <population>) data->populations[i];

        // populations
        if (currPop->isVisualised) {
            // if not in list then add to list
            bool inList = false;
            for (int p = 0; p < this->selectedPops.size(); ++p) {
                if (selectedPops[p] == currPop)
                    inList = true;
            }
            if (!inList) {
                // generate data:
                if (currPop->layoutType->locations.size() == 0) {
                    QString errs;
                    currPop->layoutType->generateLayout(currPop->numNeurons,&currPop->layoutType->locations,errs);
                    // display all errors
                    if (!errs.isEmpty()) {
                        //this->data->statusBarUpdate(errs,2000);
                    }
                }
                selectedPops.push_back(currPop);
                popLogs.push_back(NULL);
                popColours.resize(popColours.size()+1);
            }
        } else {
            // if in list then remove from list
            for (int p = 0; p < this->selectedPops.size(); ++p) {
                if (selectedPops[p] == currPop) {
                    selectedPops.erase(selectedPops.begin()+p);
                    popLogs.erase(popLogs.begin()+p);
                    popColours.erase(popColours.begin()+p);
                    // clear location data
                    currPop->layoutType->locations.clear();
                }
            }
        }

        // population inputs:
        for (int j = 0; j < currPop->neuronType->inputs.size(); ++j) {
            QSharedPointer<genericInput> currIn = currPop->neuronType->inputs[j];

            // only for pop -> pop inputs
            if (!currIn->projInput) {
                if (currIn->source->type == populationObject && currIn->destination->type == populationObject) {
                    if (currIn->isVisualised) {
                        // add to list if not in list
                        bool inList = false;
                        for (int p = 0; p < this->selectedConns.size(); ++p) {
                            if (selectedConns[p] == currIn)
                                inList = true;
                        }
                        if (!inList) {
                            selectedConns.push_back(currIn);
                            connections.resize(connections.size()+1);

                            if (currIn->conn->type == CSV) {
                                // load in the connections
                                ((csv_connection *) currIn->conn)->getAllData(connections.back());
                            } else if (currIn->conn->type == Python) {
                                if (((pythonscript_connection *) currIn->conn)->connections.size() > 0 && !((pythonscript_connection *) currIn->conn)->changed()) {
                                    connections.back() = ((pythonscript_connection *) currIn->conn)->connections;
                                } else {
                                    // generate
                                    // launch version increment dialog box:
                                    QSharedPointer <population> popSrc = qSharedPointerDynamicCast <population> (currIn->source);
                                    QSharedPointer <population> popDst = qSharedPointerDynamicCast <population> (currIn->destination);
                                    generate_dialog generate(((pythonscript_connection *) currIn->conn), popSrc, popDst, connections.back(), connGenerationMutex, this);
                                    bool retVal = generate.exec();
                                    if (!retVal) {
                                        continue;
                                    }
                                    ((pythonscript_connection *) currIn->conn)->connections = connections.back();
                                    ((pythonscript_connection *) currIn->conn)->setUnchanged(true);
                                }
                            }

                        }

                    } else {
                        // remove from list if there
                        for (int p = 0; p < this->selectedConns.size(); ++p) {
                            if (selectedConns[p] == currIn) {
                                selectedConns.erase(selectedConns.begin()+p);
                                connections.erase(connections.begin()+p);
                            }
                        }
                    }
                }
            }
        }

        // projections
        for (int j = 0; j < currPop->projections.size(); ++j) {

            QSharedPointer <projection> currProj = (QSharedPointer <projection>) currPop->projections[j];

            // synapses
            for (int k = 0; k < currProj->synapses.size(); ++k) {

                QSharedPointer <synapse> currTarg = (QSharedPointer <synapse>) currProj->synapses[k];

                if (currTarg->isVisualised) {
                    // if not in list then add to list
                    bool inList = false;
                    for (int p = 0; p < this->selectedConns.size(); ++p) {
                        if (selectedConns[p] == currTarg)
                            inList = true;
                    }
                    if (!inList) {
                        selectedConns.push_back(currTarg);
                        connections.resize(connections.size()+1);

                        if (currTarg->connectionType->type == CSV) {
                            // load in the connections
                            ((csv_connection *) currTarg->connectionType)->getAllData(connections.back());
                        } else if (currTarg->connectionType->type == Python) {
                            if (((pythonscript_connection *) currTarg->connectionType)->connections.size() > 0 && !((pythonscript_connection *) currTarg->connectionType)->changed()) {
                                connections.back() = ((pythonscript_connection *) currTarg->connectionType)->connections;
                            } else {
                                // generate
                                // launch version increment dialog box:
                                generate_dialog generate(((pythonscript_connection *) currTarg->connectionType), currTarg->proj->source, currTarg->proj->destination, connections.back(), connGenerationMutex, this);
                                bool retVal = generate.exec();
                                if (!retVal) {
                                    return;
                                }
                                ((pythonscript_connection *) currTarg->connectionType)->connections = connections.back();
                                ((pythonscript_connection *) currTarg->connectionType)->setUnchanged(true);
                            }
                        }

                    }
                } else {
                    // if in list then remove from list
                    for (int p = 0; p < this->selectedConns.size(); ++p) {
                        if (selectedConns[p] == currTarg) {
                            selectedConns.erase(selectedConns.begin()+p);
                            connections.erase(connections.begin()+p);
                        }
                    }
                }
            }
        }
    }
    // check for logs:
    experiment* currentExperiment = data->main->getCurrentExpt();
    if (currentExperiment != (experiment*)0) {
        viewGVstruct* vgvs = data->main->viewGV[currentExperiment];
        if (vgvs != (viewGVstruct*)0) {
            addLogs(&vgvs->properties->vLogData);
        }
    }

    // force redraw!
    this->repaint();
}

void
glConnectionWidget::connectionDataChanged(QModelIndex, QModelIndex)
{
    // refetch the connections:
    this->getConnections();
}

void
glConnectionWidget::connectionSelectionChanged(QItemSelection, QItemSelection)
{
    this->selectedIndex = 0;
    this->selectedType = 4;
    this->selection = ((QItemSelectionModel *) sender())->selectedIndexes();
    this->repaint();
}

void
glConnectionWidget::mousePressEvent (QMouseEvent *event)
{
    setCursor (Qt::ClosedHandCursor);
    button = event->button();

    mousePressPosition = QVector2D(event->localPos());

    this->origPos = event->globalPos();
    this->origPos.setX (this->origPos.x() - this->pos.x()*100/this->zoomFactor);
    this->origPos.setY (this->origPos.y() + this->pos.y()*100/this->zoomFactor);

    this->origRot = event->globalPos();
    this->origRot.setX (this->origRot.x() - this->rot.x()*2);
    this->origRot.setY (this->origRot.y() - this->rot.y()*2);
}

void
glConnectionWidget::mouseReleaseEvent (QMouseEvent *)
{
    setCursor (Qt::ArrowCursor);
}

void
glConnectionWidget::mouseMoveEvent (QMouseEvent *event)
{
    QVector3D n;
    // Different translate/rotate
    if (button == Qt::LeftButton) {
        if ((QApplication::keyboardModifiers() & Qt::ShiftModifier)) {
            // Rotate
            DBG() << "keymods";
            this->rot.setX(-(this->origRot.x() - event->globalPos().x())*0.5);
            this->rot.setY(-(this->origRot.y() - event->globalPos().y())*0.5);
            n = QVector3D(this->rot.y(), this->rot.x(), 0.0).normalized();

        } else {
            // Translate?
            DBG() << "Left button";
            pos.setX(-(this->origPos.x() - event->globalPos().x())*0.01*this->zoomFactor);
            pos.setY((this->origPos.y() - event->globalPos().y())*0.01*this->zoomFactor);
            n = QVector3D(this->pos.y(), this->pos.x(), 0.0).normalized();

        }
    }
    if (button == Qt::RightButton)
        DBG() << "Right button";{
        this->rot.setX(-(this->origRot.x() - event->globalPos().x())*0.5);
        this->rot.setY(-(this->origRot.y() - event->globalPos().y())*0.5);
        n = QVector3D(this->rot.y(), this->rot.x(), 0.0).normalized();
    }

    // Mouse release position - mouse press position - as from example opengl
    QVector2D diff = (QVector2D(event->localPos()) - mousePressPosition)/100;

    // Calculate new rotation axis as weighted sum
    this->rotationAxis = (this->rotationAxis + n).normalized();

    qDebug() << "rotationAxis: " << this->rotationAxis;

    // Update rotation - figure out the maths of this. In shapewindow, it's done in a timer.
    float rotangle = diff.length();
    this->rotation = QQuaternion::fromAxisAndAngle(this->rotationAxis, rotangle) * this->rotation;

    this->setPerspective (this->width(), this->height());

    this->repaint(); // Shouldn't need this if we schedule repaints/renders on a timed basis
    // or?
    //this->paintGL();
}

void
glConnectionWidget::wheelEvent(QWheelEvent *event)
{
    float val = float(event->delta()) / 320.0;
    val = pow(2.0f,val);
    if (event->orientation() == Qt::Vertical) {
        this->zoomFactor *= (val);
        if (this->zoomFactor < 0.00001) {
            zoomFactor = 1;
        }
        // FIXME: Set rotation?
        this->setPerspective (this->width(), this->height());
        this->repaint();
    }
}

void
glConnectionWidget::setPopIndicesShown(bool checkState)
{
    popIndicesShown = checkState;
    this->repaint();
}

void
glConnectionWidget::selectedNrnChanged(int index)
{
    QString type = sender()->property("type").toString();

    if (type == "index") {
        // Then the index box must have called this slot
        selectedIndex = index;
    } else if (type == "from") {
        // In this case, the source/destination combo box must be the sender
        selectedType = index+1;
    }

    this->repaint();
}

#if QT_VERSION > QT_VERSION_CHECK(5, 0, 0)
QImage
glConnectionWidget:: renderQImage (int w_img, int h_img)
{
    // Set the rendering engine to the size of the image to render
    // Also set the format so that the depth buffer will work
    QOpenGLFramebufferObjectFormat format;
    format.setAttachment (QOpenGLFramebufferObject::Depth);
    QOpenGLFramebufferObject qfb (w_img, h_img, format);
    qfb.bind();

    // If the frame buffer does not work then return an empty image...
    if(!qfb.isValid()) {
        return(QImage());
    }

    // ...otherwise, draw the scene to the buffer and return image
    this->resizeGL (w_img, h_img);
    glEnable (GL_MULTISAMPLE);
    this->repaint();
    qfb.release();
    this->resizeGL (this->width(), this->height());
    return (qfb.toImage());
}
#endif

QPixmap
glConnectionWidget::renderImage (int width, int height)
{
    QPixmap pix;
    this->imageSaveHeight = height;
    this->imageSaveWidth = width;
    this->imageSaveMode = true;

#if 0 // FIXME if this is for returning PNGs of the screen, needs to be updated to new OpenGL

// renderPixmap is broken in Qt > 5.0 - this fix doesn't currently work correctly, but is better than nothing
#if QT_VERSION > QT_VERSION_CHECK(5, 0, 0)
    QImage img = renderQImage(width, height);

    if (img.isNull()) {
        qDebug() << "renderQimage returned a dud";
    }

    pix = QPixmap::fromImage (img);
#else
    pix = this->renderPixmap (width, height);
#endif

    QPainter painter(&pix);
    painter.setRenderHint(QPainter::Antialiasing);
    QFont font = painter.font();
    font.setPointSizeF(font.pointSizeF()*((float) width)/((float) this->width()));
    painter.setFont(font);
    painter.setPen(QColor(100,100,100));

    setupView();

    glPushMatrix();
    glTranslatef(0,0,-5.0);

    if (popIndicesShown) {
        // draw text
        for (int locNum = 0; locNum < selectedPops.size(); ++locNum) {
            QSharedPointer <population> currPop = selectedPops[locNum];
            for (int i = 0; i < currPop->layoutType->locations.size(); ++i) {
                glPushMatrix();

                glTranslatef(currPop->layoutType->locations[i].x, currPop->layoutType->locations[i].y, currPop->layoutType->locations[i].z);

                // if currently selected
                if (currPop == selectedObject) {
                    // move to pop location denoted by the spinboxes for x, y, z
                    glTranslatef(loc3Offset.x, loc3Offset.y,loc3Offset.z);
                } else {
                    glTranslatef(currPop->loc3.x, currPop->loc3.y,currPop->loc3.z);
                }

                // print up text:
                GLdouble modelviewMatrix[16];
                GLdouble projectionMatrix[16];
                GLint viewPort[4];
                GLdouble winX;
                GLdouble winY;
                GLdouble winZ;
                glGetIntegerv(GL_VIEWPORT, viewPort);
                glGetDoublev(GL_MODELVIEW_MATRIX, modelviewMatrix);
                glGetDoublev(GL_PROJECTION_MATRIX, projectionMatrix);
                gluProject(0, 0, 0, modelviewMatrix, projectionMatrix, viewPort, &winX, &winY, &winZ);

                painter.drawText(QRect(winX-(1.0-winZ)*220-(20.0)*(float(width)/500.0),height-winY-(1.0-winZ)*220-10*(float(width)/500.0),40*(float(width)/500.0),20*(float(width)/500.0)),QString::number(float(i)));

                glPopMatrix();
            }
        }
        painter.end();
    }

    glPopMatrix();
#endif
    this->imageSaveMode = false;
    return pix;
}
