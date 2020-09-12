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

#include "SC_glConnectionWidget.h"
#include "SC_connectionmodel.h"
#include "SC_systemmodel.h"
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

glConnectionWidget::glConnectionWidget (nl_rootdata* _data, QWidget* parent)
    : QOpenGLWidget(parent)
{
    model = (QAbstractTableModel *)0;
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
    orthoView = false; // Currently unused anyway.
}

glConnectionWidget::~glConnectionWidget()
{
    if (this->shaderProg != (QOpenGLShaderProgram*)0) { delete this->shaderProg; }
    if (this->nscene != (NeuronScene*)0) { delete this->nscene; }
}

void
glConnectionWidget::initializeGL()
{
    QOpenGLFunctions *f = this->context()->functions();
    f->glClearColor (0.8f, 0.7f, 0.8f, 1.0f);

    // initialize shaders
    this->shaderProg = new QOpenGLShaderProgram(this);

    // Add shaders from files, making it easier to read/modify the shader code
    if (!this->shaderProg->addShaderFromSourceFile (QOpenGLShader::Vertex,
                                                    "/home/seb/src/SpineCreator/vshader.glsl")) {
        DBG() << "Failed to open vertex shader glsl";
        close();
    }
    if (!this->shaderProg->addShaderFromSourceFile (QOpenGLShader::Fragment,
                                                    "/home/seb/src/SpineCreator/fshader.glsl")) {
        DBG() << "Failed to open fragment shader glsl";
        close();
    }

    // link and bind Shader (Do not release until VAO is created)
    if (!this->shaderProg->link()) { close(); }
    if (!this->shaderProg->bind()) { close(); }

    // Enable depth buffer
    f->glEnable(GL_DEPTH_TEST);

    // Do we create the scene here? Probably not, as we may not actually HAVE a scene yet
    // Create the scene. This creates several spherelayers, each containing VAO and VBOs
    this->nscene = new NeuronScene (this->shaderProg, f);

    // No point setting up model in initializeGL, but mark that we will need to compute our lines and spheres
    this->setupModelRequired = true;

    // Now VAOs were created in scene objects, release shaderProg
    this->shaderProg->release();

    // Set the perspective. Might need retina factor here.
    this->setPerspective (this->width(), this->height());
}

void
glConnectionWidget::resizeGL (int w, int h)
{
    this->setPerspective (w, h);
}

void
glConnectionWidget::paintGL()
{
    if (this->setupModelRequired == true) {
        DBG() << "Calling setupModel in context " << QOpenGLContext::currentContext();
        this->setupModel();
    } // no need to set up model

    QOpenGLFunctions *f = this->context()->functions();
    f->glViewport (0, 0, this->width(), this->height());
    this->setPerspective (this->width(), this->height());
    // Calculate model view transformation - transforming from "model space" to "worldspace".
    QMatrix4x4 sceneview;
    // This line translates from model space to world space.
    DBG() << "Translating by " << this->scenetrans;
    sceneview.translate (this->scenetrans); // send backwards into distance
    // And this rotation completes the transition from model to world
    sceneview.rotate (this->rotation);
    // Bind shader program...
    this->shaderProg->bind();
    // Clear color buffer and **also depth buffer**
    f->glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // Set modelview-projection matrix. Don't have a viewmatrix to postmultiply (projMatrix * sceneview).
    //this->shaderProg->setUniformValue ("mvp_matrix", this->projMatrix * sceneview);
    // Call the NeuronScene's render method
    this->nscene->render(this->projMatrix * sceneview);
    // Finally, release the shaderProg
    this->shaderProg->release();
}

void
glConnectionWidget::toggleOrthoView (bool toggle)
{
    DBG() << "INFO: No orthographic projection is available.";
#if 0
    this->orthoView = toggle;
    this->update();
#endif
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
    // Maybe also
    this->nscene->reset();
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
glConnectionWidget::updateLogData()
{
    if (newLogTime == currentLogTime) { return; }

    currentLogTime = newLogTime;

    // fetch data from logs
    for (int i = 0; i < popLogs.size(); ++i) {

        // skip where there is no log
        if (popLogs[i] == NULL) { continue; }

        // get a row
        QVector < double > logValues = popLogs[i]->getRow(currentLogTime);

        // data not usable
        if (logValues.size() == 0) { continue; }
        if ((int) logValues.size() > selectedPops[i]->numNeurons) { continue; }

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

    this->update();
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
    this->update();
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

    // Use loc3Offset to update the current population's location, then update
    qSharedPointerDynamicCast<population>(selectedObject)->loc3 = loc3Offset;
    // Then that would be reflected during render

    this->update();
}

void
glConnectionWidget::updateModel (void)
{
    // Create spheres in a SphereLayer for each population & add to NeuronScene.
    for (int locNum = 0; locNum < this->selectedPops.size(); ++locNum) {
        QSharedPointer<population> currPop = selectedPops[locNum];
        SphereLayer* sl = this->nscene->createSphereLayer();
        loc mean_location(0.0f, 0.0f, 0.0f);
        for (int i = 0; i < currPop->layoutType->locations.size(); ++i) {
            mean_location += currPop->layoutType->locations[i];
        }
        mean_location /= currPop->layoutType->locations.size();
        mean_location.z += locNum;
        sl->setOffset (mean_location + currPop->loc3);
    }
}

// Only call via paintGL, to ensure QOpenGLContext is correct.
void
glConnectionWidget::setupModel (void)
{
    DBG() << "called " << QOpenGLContext::currentContext();

    // This modifies this->nscene; a NeuronScene object which contains
    // some number of SphereLayer objects and then the lines
    // between them. Currently, these are LinesLayer and SphereLayer
    // objects. These are very closely linked to conn and population
    // objects and could potentially be members of those classes -
    // they need to obtain information from those classes in order to
    // render the locations of the spheres and their colours.

    if (this->locations.size() > 0) {
        DBG() << "FIXME FIXME: Do something about this->locations";
    }

    // I'll do a complete reset and rebuild whenever anything changes.
    DBG() << "Before nscene reset " << QOpenGLContext::currentContext();
    this->nscene->reset();
    DBG() << "AFTER nscene reset " << QOpenGLContext::currentContext();

    // Create spheres in a SphereLayer for each population & add to NeuronScene.
    for (int locNum = 0; locNum < this->selectedPops.size(); ++locNum) {
        QSharedPointer<population> currPop = selectedPops[locNum];
        SphereLayer* sl = this->nscene->createSphereLayer();
        DBG() << "Adding spheres for population: " << currPop->name;
        loc mean_location(0.0f, 0.0f, 0.0f);

        vector<float> pcolour (3, 0.0f);
        pcolour[0] = currPop->colour.redF();
        pcolour[1] = currPop->colour.blueF();
        pcolour[2] = currPop->colour.greenF();

        for (int i = 0; i < currPop->layoutType->locations.size(); ++i) {
            if (!i) { DBG() << "Adding at least one sphere!"; }
            mean_location += currPop->layoutType->locations[i];
            sl->addSphere({currPop->layoutType->locations[i].x,
                           currPop->layoutType->locations[i].y,
                           currPop->layoutType->locations[i].z},
                           0.25f,
                           pcolour);
        }
        // Set x/y offsets for the layer. Start with the mean of x and y
        mean_location /= currPop->layoutType->locations.size();// prob. don't want to compute mean_location here.
        // Actually, much better; then sphere layer can access currPop's loc3 and colour.
        sl->pop = currPop;

        sl->postInit();
    }
    this->setupModelRequired = false;
}

void
glConnectionWidget::setPerspective (int w, int h)
{
    //DBG() << "Called. imageSaveMode: " << imageSaveMode << ", orthoView: " << orthoView;

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

    const float zNear = 1.0, zFar = 100000.0, fov = 30.0;
    if (!this->orthoView) {
        // Set perspective projection
        float aspect = float(w) / float(h ? h : 1);
        this->projMatrix.perspective (fov, aspect, zNear, zFar);
    } else {
        DBG() << "FIXME FIXME: ORTHOVIEW WON'T WORK!";
        // or orthographic like this:
        // FIXME: this needs re-thinking, because zoomFactor is no longer used
        float scale = 10.0f; // this->zoomFactor * 10.0f;
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

    DBG() << "Maybe setupModel? 10";
    this->update();
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

    DBG() << "Maybe setupModel? 9";
    this->update();
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

//    this->setupModel();
    this->update();
}

void
glConnectionWidget::parsChangedPopulation(double)
{
    this->parsChangedPopulation();
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
    DBG() << "Maybe setupModel? 7";
    this->update();
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

//    this->setupModel();
    this->update();
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

        DBG() << "Maybe setupModel? 5";

        this->update();
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
    // redraw based on a set of location passed in (used for layout previews). Called
    // from a slot
    this->clearLocations();
    this->locations.push_back(locs);
    this->update();
}

void
glConnectionWidget::clearLocations()
{
    // clear the set of location passed in (used for layout previews)
    for (int i = 0; i < locations.size(); ++i) { locations[i].clear(); }
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
    DBG() << "Called";
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

    DBG() << "Maybe setupModel? 4";
    this->update();
}

void
glConnectionWidget::sysSelectionChanged(QModelIndex, QModelIndex)
{
    DBG() << "Called. " << QOpenGLContext::currentContext();
    // this is fired when an item is checked or unchecked.

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

    this->setupModelRequired = true;
    this->update();
    DBG() << "update() done, now return. " << QOpenGLContext::currentContext();
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
    DBG() << "Maybe setupModel? 2";
    this->update();
}

void
glConnectionWidget::mousePressEvent (QMouseEvent* event)
{
    setCursor (Qt::ClosedHandCursor);
    // Can access modifiers like this:
    // if ((QApplication::keyboardModifiers() & Qt::ShiftModifier)) {
    mousePressPosition = QVector2D(event->localPos()); // localPos is pixels within the window
    this->mousePressPosition = QVector2D(event->localPos());
    // Save the rotation at the start of the mouse movement
    this->savedRotation = this->rotation;
    DBG() << " initial rotation: " << this->savedRotation;
    // Get the scene's rotation at the start of the mouse movement:
    this->sceneMatrix.setToIdentity();
    this->sceneMatrix.rotate (this->savedRotation);
    this->invscene = this->sceneMatrix.inverted();

    Qt::MouseButton button = event->button();
    if (button == Qt::LeftButton) {
        // If the left button is down, then rotate the scene
        this->rotateMode = true;
    } else if (button == Qt::RightButton) {
        // If the left button is down, then translate the scene in x/y
        this->translateMode = true;
    }
}

void
glConnectionWidget::mouseReleaseEvent (QMouseEvent* event)
{
    setCursor (Qt::ArrowCursor);
    Qt::MouseButton button = event->button();
    if (button == Qt::LeftButton) {
        this->rotateMode = false;
    } else if (button == Qt::RightButton) {
        this->translateMode = false;
    }
}

void
glConnectionWidget::mouseMoveEvent (QMouseEvent* event)
{
    // Mouse release position - mouse press position - as from example opengl
    this->cursorpos = QVector2D(event->localPos());
    DBG() << "cursorpos = " << this->cursorpos;

    QVector3D mouseMoveWorld = { 0.0f, 0.0f, 0.0f };

    // This is "rotate the scene" model. Will need "rotate one visual" mode.
    if (this->rotateMode) {
        // Convert mousepress/cursor positions (in pixels) to the range -1 -> 1:
        QVector2D p0_coord = this->mousePressPosition;
        p0_coord[0] -= this->width()/2.0;
        p0_coord[0] /= this->width()/2.0;
        p0_coord[1] -= this->height()/2.0;
        p0_coord[1] /= this->height()/2.0;
        QVector2D p1_coord = this->cursorpos;
        p1_coord[0] -= this->width()/2.0;
        p1_coord[0] /= this->width()/2.0;
        p1_coord[1] -= this->height()/2.0;
        p1_coord[1] /= this->height()/2.0;
        DBG() << "Rotating based on mouse move from " << p0_coord << " to " << p1_coord;
        // NB: DON'T update this->mousePressPosition until user releases button (see mouseReleaseButton())

        // Add the depth at which the object lies.  Use forward projection to determine
        // the correct z coordinate for the inverse projection. This assumes only one object.
        QVector4D point =  { 0.0f, 0.0f, this->scenetrans.z(), 1.0f };
        QVector4D pp = this->projMatrix.map(point);
        float coord_z = pp[2]/pp[3]; // divide by pp[3] is divide by/normalise by 'w'.
        DBG() << "(Rotate) scenetrans.z: " << this->scenetrans.z()
              << "; z position of object, coord_z: " << coord_z;

        // Construct two points for the start and end of the mouse movement
        QVector4D p0 = { p0_coord[0], p0_coord[1], coord_z, 1.0f };
        QVector4D p1 = { p1_coord[0], p1_coord[1], coord_z, 1.0f };
        // Apply inverse projection to get 2 points (in the world frame) for the mouse movement
        QVector4D v0 = this->invproj * p0;
        QVector4D v1 = this->invproj * p1;
        // This computes the difference betwen v0 and v1, the 2 mouse positions in the
        // world space. Note the swap between x and y
        if ((QApplication::keyboardModifiers() & Qt::ShiftModifier)) {
            // Sort of "rotate the page" mode.
            mouseMoveWorld[2] = -((v1[1]/v1[3]) - (v0[1]/v0[3])) + ((v1[0]/v1[3]) - (v0[0]/v0[3]));
        } else {
            mouseMoveWorld[1] = +((v1[0]/v1[3]) - (v0[0]/v0[3])); // Odd, to get sensible-ish behaviour, had to make this +() not -().
            mouseMoveWorld[0] = +((v1[1]/v1[3]) - (v0[1]/v0[3])); // had to make this +() not -(), as well. What's different? Sign of z probably.
        }
        // Rotation axis is perpendicular to the mouse position difference vector
        // BUT we have to project into the model frame to determine how to rotate the model!
        float rotamount = mouseMoveWorld.length() * 100.0 * std::abs(coord_z); // Bit arbitrary. Should change with depth?
        // Calculate new rotation axis as weighted sum
        this->rotationAxis = (mouseMoveWorld * rotamount);
        this->rotationAxis.normalize();
        // Now inverse apply the rotation of the scene to the rotation axis
        // (Vector<float,3>), so that we rotate the model the right way.
        //QVector4D tmp_4D = this->invscene * this->rotationAxis; // or:
        QVector4D tmp_4D = this->invscene.map (this->rotationAxis);
        this->rotationAxis = tmp_4D.toVector3D();
        // Update rotation starting from the saved position.
        this->rotation = this->savedRotation;
        this->rotation *= QQuaternion::fromAxisAndAngle(this->rotationAxis, rotamount);

        this->update();

    } else if (this->translateMode) { // allow only rotate OR translate for a single mouse movement

        // Convert mousepress/cursor positions (in pixels) to the range -1 -> 1:
        QVector2D p0_coord = this->mousePressPosition;
        p0_coord[0] -= this->width()/2.0;
        p0_coord[0] /= this->width()/2.0;
        p0_coord[1] -= this->height()/2.0;
        p0_coord[1] /= this->height()/2.0;
        QVector2D p1_coord = this->cursorpos;
        p1_coord[0] -= this->width()/2.0;
        p1_coord[0] /= this->width()/2.0;
        p1_coord[1] -= this->height()/2.0;
        p1_coord[1] /= this->height()/2.0;

        this->mousePressPosition = this->cursorpos;

        // Add the depth at which the object lies.  Use forward projection to determine
        // the correct z coordinate for the inverse projection. This assumes only one
        // object.
        QVector4D point =  { 0.0f, 0.0f, this->scenetrans.z(), 1.0f };
        QVector4D pp = this->projMatrix.map (point);
        float coord_z = pp[2]/pp[3]; // divide by pp[3] is divide by/normalise by 'w'.
        DBG() << "(Translate) scenetrans.z: " << this->scenetrans.z()
              << "; z position of object, coord_z: " << coord_z;

        // Construct two points for the start and end of the mouse movement
        QVector4D p0 = { p0_coord[0], p0_coord[1], coord_z, 1.0f };
        QVector4D p1 = { p1_coord[0], p1_coord[1], coord_z, 1.0f };
        // Apply the inverse projection to get two points in the world frame of reference:
        QVector4D v0 = this->invproj.map (p0); // QMatrix.map (p0) seems to be equivalent to QMatrix * p0
        QVector4D v1 = this->invproj * p1;
        // This computes the difference betwen v0 and v1, the 2 mouse positions in the world
        mouseMoveWorld[0] = (v1[0]/v1[3]) - (v0[0]/v0[3]);
        mouseMoveWorld[1] = (v1[1]/v1[3]) - (v0[1]/v0[3]);
        //mouseMoveWorld.z = (v1[2]/v1[3]) - (v0[2]/v0[3]);// unmodified

#define APPLY_PERSPECTIVE_FACTOR 1
#if defined TRANSFORMTOMOUSEMOVESCENE
        // Now need "mouseMoveAtModel" to translate the scene by the right amount. I
        // thought coord_z was supposed to do this...
        QVector3D mouseMoveScene = mouseMoveWorld * (std::abs(coord_z));
        this->scenetrans[0] += mouseMoveScene[0];
        this->scenetrans[1] -= mouseMoveScene[1];
#elif defined APPLY_PERSPECTIVE_FACTOR
        // This seems a bit cludgy, but it works for now
        float factor = 4.0f;
        float perspFactor = (std::abs(scenetrans.z()) - factor)/factor; // FIXME: Make this->zNear. Nope.
        this->scenetrans[0] += mouseMoveWorld[0] * perspFactor;
        this->scenetrans[1] -= mouseMoveWorld[1] * perspFactor;
#else
        // Original
        this->scenetrans[0] += mouseMoveWorld[0];
        this->scenetrans[1] -= mouseMoveWorld[1];
#endif
        this->update(); // Shouldn't need this if we schedule repaints/renders on a timed basis
    }
}

void
glConnectionWidget::wheelEvent(QWheelEvent *event)
{
    float val = float(event->delta()) / 32.0;
    DBG() << "val= " << val;
    if (event->orientation() == Qt::Vertical) {
        // Change sceneTrans to zoom in & out
        float distFactor = this->scenetrans[2]*this->scenetrans[2];
        distFactor = (distFactor > 1e5 ? 1e5 : distFactor);
        distFactor = (distFactor < 1 ? 1 : distFactor);
        DBG() << "distFactor: " << distFactor;
        this->scenetrans[2] += val * this->scenetrans_stepsize * distFactor;
        this->scenetrans[2] = (this->scenetrans[2] > -1.0f ? -1.0f : this->scenetrans[2]);
        this->update();
    }
}

void
glConnectionWidget::setPopIndicesShown(bool checkState)
{
    popIndicesShown = checkState;
    this->update();
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

    DBG() << "Maybe setupModel? 3";

    this->update();
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
    this->setPerspective (w_img, h_img);
    glEnable (GL_MULTISAMPLE);
    this->update();
    qfb.release();
    this->setPerspective (this->width(), this->height());
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
