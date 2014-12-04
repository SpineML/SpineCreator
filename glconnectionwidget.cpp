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

#include "glconnectionwidget.h"
#include "connectionmodel.h"
#include "systemmodel.h"
#ifdef Q_OS_MAC
#include "glu.h"
#else
#include "GL/glu.h"
#endif
#include "generate_dialog.h"
#include "mainwindow.h"
#if QT_VERSION > QT_VERSION_CHECK(5, 0, 0)
#include <QOpenGLFramebufferObject>
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

glConnectionWidget::glConnectionWidget(rootData * data, QWidget *parent) : QGLWidget(QGLFormat(QGL::SampleBuffers), parent)
{

    model = (QAbstractTableModel *)0;
    pos = QPointF(0,0);
    zoomFactor = 1.0;
    this->data = data;
    setAutoFillBackground(false);
    popIndicesShown = false;
    clickedPopulation = -1;
    clickedNeuron = -1;
    selectedIndex = -1;
    selectedType = 1;
    connGenerationMutex = new QMutex;
    imageSaveMode = false;

    connect(&timer, SIGNAL(timeout()), this, SLOT(updateLogData()));

    timer.start(50);

    newLogTime = 0;
    currentLogTime = 0;

    orthoView = false;

    repaintAllowed = true;
}

void glConnectionWidget::initializeGL()
{

    createPopulationsDL();
    createConnectionsDL();

    glEnable(GL_MULTISAMPLE);
    glEnable(GL_MAP1_VERTEX_3);


}

void glConnectionWidget::toggleOrthoView(bool toggle) {
    orthoView = toggle;
    this->repaint();
}

void glConnectionWidget::clear() {

    selectedObject.clear();
    selectedPops.clear();
    popColours.clear();
    popLogs.clear();
    selectedConns.clear();
    connections.clear();
    selectedIndex = -1;
    selectedType = 1;
    model = (QAbstractTableModel *)0;

}

/*!
 * \brief glConnectionWidget::addLogs
 * \param logs
 *
 * This method is passed a list of logs and checks to see if
 * anny of the logs in the QVector match the ports found in the
 * displayed system. Any that do are linked to the selected pop/
 * conn and can be used to display data from simulations.
 *
 */
void glConnectionWidget::addLogs(QVector < logData * > * logs) {

    // for each population
    for (int i = 0; i < selectedPops.size(); ++i) {

        QSharedPointer <population> pop = selectedPops[i];

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
                    if ((*logs)[k]->logName == possibleLogName)
                        popLogs[i] = (*logs)[k];
                }
            }

        }
    }

    // for each connection...
    for (int i = 0; i < this->selectedConns.size();++i) {

        // get the source Population
        QSharedPointer < population > pop;

        if (qSharedPointerDynamicCast <synapse> (this->selectedConns[i])) {
            QSharedPointer < synapse > s = qSharedPointerDynamicCast <synapse> (this->selectedConns[i]);
            pop = qSharedPointerDynamicCast <population> (s->proj->source);
        }
        else if (qSharedPointerDynamicCast <genericInput> (this->selectedConns[i])) {
            QSharedPointer < genericInput > in = qSharedPointerDynamicCast <genericInput> (this->selectedConns[i]);
            if (in->src->owner->type == populationObject) {
                pop = qSharedPointerCast <population> (in->src->owner);
            }
        }
        else {
            // we shouldn't get here...
            qDebug() << "Error: " << __FILE__ << ", " << __LINE__;
            exit(-1);
        }

        CHECK_CAST(pop);

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
                        connLogs[i] = (*logs)[k];
                    }
                }
            }

        }

        // for each event output port
        for (int j = 0; j < pop->neuronType->component->EventPortList.size(); ++j) {

            EventPort * port = pop->neuronType->component->EventPortList[j];
            // if send port
            if (port->mode == EventSendPort) {

                // construct log name! This should be replaced by XML data from the log
                QString possibleLogName = pop->name + "_" + port->name + "_log.csv";
                possibleLogName.replace(" ", "_");

                // check each log in turn
                for (int k = 0; k < logs->size(); ++k) {
                    if ((*logs)[k]->logName == possibleLogName) {
                        connLogs[i] = (*logs)[k];
                        qDebug() << "Added log for " << possibleLogName;
                    }
                }
            }

        }

    }

}

/*!
 * \brief glConnectionWidget::updateLogDataTime
 * \param index
 *
 * Update the current time to read from the logs. Note that
 * this does not force a redraw as that would lead to excessive redraws,
 * but instead updates the time only.
 */
void glConnectionWidget::updateLogDataTime(int index) {

    newLogTime = index;

}

/*!
 * \brief glConnectionWidget::updateLogData
 * Update the displayed data from the logs. First checks to
 * see if the time has changed, and only updates the data
 * if this is true.
 */
void glConnectionWidget::updateLogData() {

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

    for (int i = 0; i < connLogs.size(); ++i) {

        // skip where there is no log
        if (connLogs[i] == NULL)
            continue;

        // remove old values
        this->connLogVals[i].clear();

        // get a row
        QVector < double > logValues = connLogs[i]->getRow(currentLogTime);

        // data not usable
        if (logValues.size() == 0)
            continue;
        if ((int) logValues.size() > selectedPops[i]->numNeurons)
            continue;

        if (connLogs[i]->dataClass == EVENTDATA) {
            this->connLogVals[i] = logValues;
        } else {
            // normalise the values
            for (int j = 0; j < logValues.size(); ++j) {
                connLogVals[i].push_back(((logValues[j]-popLogs[i]->getMin()))/(popLogs[i]->getMax()-popLogs[i]->getMin()));
            }
        }
    }



    //createPopulationsDL(true);

    // redraw!
    this->repaint();

}

/*!
 * \brief glConnectionWidget::resizeGL
 * Triggered when this GLWidget is resized
 */
void glConnectionWidget::resizeGL(int, int)
{

    // setup the view
    this->repaint();


}

/*!
 * \brief glConnectionWidget::redraw
 * Used to trigger a reconstruction of the display lists
 * and then redraw the GL view.
 */
void glConnectionWidget::redraw() {
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

    createPopulationsDL();
    createConnectionsDL();

    this->repaint();
}

/*!
 * \brief glConnectionWidget::redraw
 *
 * Used for instant feedback when the 3D location spinboxes are changed, but
 * we don't yet want to commit the changes to the undo history.
 */
void glConnectionWidget::redraw(int)
{

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

    createPopulationsDL();
    createConnectionsDL();

    this->repaint();

}

/*!
 * \brief glConnectionWidget::updateConnections
 * Recalculate only the connection display lists, then
 * redraw the GL view.
 */
void glConnectionWidget::updateConnections()
{
    createConnectionsDL();

    this->repaint();

}

/*!
 * \brief glConnectionWidget::allowRepaint
 * On Mac OSX we have an issue where scrolling calls too many repaint events,
 * leading to slow behaviour. To prevent this we have a QTimer which triggers
 * the repaintAllowed bool to true. This means that repaints are only triggered
 * when we need, and if they are triggered too fast the bool skips the repaint.
 */
void glConnectionWidget::allowRepaint() {
    this->repaintAllowed = true;
}

/*!
 * \brief glConnectionWidget::paintEvent
 * Repaint the GL view.
 */
void glConnectionWidget::paintEvent(QPaintEvent * /*event*/ )
{

    // avoid repainting too fast
    if (this->repaintAllowed == false) {
        return;
    } else {
        this->repaintAllowed = false;
        QTimer * timer = new QTimer(this);
        timer->setSingleShot(true);
        connect(timer, SIGNAL(timeout()), this, SLOT(allowRepaint()));
        timer->start(5);
    }
    ++count;
    count = count % 20;

    // don't try and repaint a hidden widget!
    if (!this->isVisible())
        return;

    // get rid of old stuff
    if (imageSaveMode) {
        QColor qtCol = QColor::fromRgbF(1.0,1.0,1.0,0.0);
        qglClearColor(qtCol);
    } else {
        QColor qtCol = QColor::fromRgbF(0.1, 0.1, 0.1, 0.0);
        qglClearColor(qtCol);
    }
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //glClearColor(0.0,0.0,0.0,1.0);

    // setup
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_POLYGON_SMOOTH);
    glEnable(GL_LINE_SMOOTH);

    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_LIGHTING);
    glEnable(GL_COLOR_MATERIAL);

    glEnable(GL_LIGHT0);
    GLfloat pos0[4] = {-1,-1, 10,0};
    glLightfv(GL_LIGHT0, GL_POSITION, pos0);

    /*glEnable(GL_LIGHT1);
    GLfloat pos1[4] = {0,0,-1,0};
    glLightfv(GL_LIGHT1, GL_POSITION, pos1);*/

    // setup the view
    this->setupView();

    glLoadIdentity();

    // work out scaling for line widths:
    float lineScaleFactor;
    if (imageSaveMode) {
        float maxLen;
        maxLen = imageSaveHeight > imageSaveWidth ? imageSaveHeight : imageSaveWidth;
        lineScaleFactor = (1.0/1000.0*maxLen);
    } else
        lineScaleFactor = 1.0;


    // add some neurons!

    // fetch quality setting
    QSettings settings;
    int quality = settings.value("glOptions/detail", 5).toInt();

    // draw with a level of detail dependant on the number on neurons we must draw
    // sum neurons across all pops we'll draw
    int totalNeurons = 0;
    for (int locNum = 0; locNum < selectedPops.size(); ++locNum) {
    totalNeurons += selectedPops[locNum]->layoutType->locations.size();
    }
    int LoD = round(250.0f/float(totalNeurons)*pow(2,float(quality)));

    glPushMatrix();
    glTranslatef(0,0,-5.0);

    // if previewing a layout then override normal drawing
    if (locations.size() > 0) {
        for (int i = 0; i < locations[0].size(); ++i) {
            glPushMatrix();

            glTranslatef(locations[0][i].x, locations[0][i].y, locations[0][i].z);

            this->drawNeuron(0.5, LoD, LoD, QColor(100,100,100,255));

            glPopMatrix();
        }

        glPopMatrix();
        // need this as no painter!
        swapBuffers();
        return;
    }

    // normal drawing
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

            // check we haven't broken stuff
            if (popColours[locNum].size() > currPop->layoutType->locations.size()) {

                popColours[locNum].clear();
                popLogs[locNum] = NULL;

            }

            if (popColours[locNum].size() > 0) {
                this->drawNeuron(0.5, LoD, LoD, popColours[locNum][i]);
            } else
                this->drawNeuron(0.5, LoD, LoD, QColor(100 + 0.5*currPop->colour.red(),100 + 0.5*currPop->colour.green(),100 + 0.5*currPop->colour.blue(),255));

            glPopMatrix();

        }
    }

    // draw synapses
    for (int targNum = 0; targNum < this->selectedConns.size(); ++targNum) {

        // draw the connections:
        glDisable(GL_LIGHTING);
        glEnable(GL_DEPTH_TEST);

        //qDebug() << "Doing conns!";

        //Synapse * currTarg = selectedConns[targNum];

        QSharedPointer <population> src;
        QSharedPointer <population> dst;
        connection * conn;

        if (selectedConns[targNum]->type == synapseObject) {
            QSharedPointer <synapse> currTarg = qSharedPointerDynamicCast <synapse> (selectedConns[targNum]);
            CHECK_CAST(currTarg)
            conn = currTarg->connectionType;
            src = currTarg->proj->source;
            dst = currTarg->proj->destination;
        } else {
            QSharedPointer<genericInput> currIn = qSharedPointerDynamicCast<genericInput> (selectedConns[targNum]);
            CHECK_CAST(currIn)
            conn = currIn->connectionType;
            src = qSharedPointerDynamicCast <population> (currIn->source);
            CHECK_CAST(src)
            dst = qSharedPointerDynamicCast <population> (currIn->destination);
            CHECK_CAST(dst)
        }

        float srcX;
        float srcY;
        float srcZ;

        // big offsets for src
        if (src == selectedObject) {
            srcX = this->loc3Offset.x;
            srcY = this->loc3Offset.y;
            srcZ = this->loc3Offset.z;
        } else {
            srcX = src->loc3.x;
            srcY = src->loc3.y;
            srcZ = src->loc3.z;
        }

        float dstX;
        float dstY;
        float dstZ;

        // big offsets for dst
        if (dst == selectedObject) {
            dstX = this->loc3Offset.x;
            dstY = this->loc3Offset.y;
            dstZ = this->loc3Offset.z;
        } else {
            dstX = dst->loc3.x;
            dstY = dst->loc3.y;
            dstZ = dst->loc3.z;
        }

        // check we have the current version of the connectivity
        if (conn->type == CSV) {
            csv_connection * csv_conn = dynamic_cast<csv_connection *> (conn);
            CHECK_CAST(csv_conn)
            if (csv_conn->generator) {
                pythonscript_connection * pyConn = dynamic_cast<pythonscript_connection *> (csv_conn->generator);
                CHECK_CAST(pyConn)
                if (pyConn->changed()) {
                    pyConn->regenerateConnections();
                    // fetch connections back here:
                    connections[targNum].clear();
                    csv_conn->getAllData(connections[targNum]);
                }
            }
        }

        // fetch the values that will be used to draw curved lines
        int aux_strength;
        GLfloat * center;
        if (selectedConns[targNum]->type == synapseObject) {

            QSharedPointer <synapse> currObj = qSharedPointerDynamicCast <synapse> (selectedConns[targNum]);
            CHECK_CAST(currObj);
            glCallList(currObj->dlIndex);
            aux_strength = currObj->strength;
            center = currObj->center;
        }
        else {
            QSharedPointer <genericInput> currObj = qSharedPointerDynamicCast <genericInput> (selectedConns[targNum]);
            if (!currObj) exit(-1);
            glCallList(currObj->dlIndex);
            aux_strength = currObj->strength;
            center = currObj->center;
        }

        if (conn->type == CSV || conn->type == Kernel || conn->type == Python) {

            if (!src->isVisualised && !dst->isVisualised) {
                glEnable(GL_DEPTH_TEST);
                glEnable(GL_LIGHTING);
                continue;
            }

            connGenerationMutex->lock();

            // draw selected connections on top, or logs
            glDisable(GL_DEPTH_TEST);
            if (selectedConns[targNum] == selectedObject || connLogs[targNum]) {
                for (int i = 0; i < connections[targNum].size(); ++i) {

                    if (connections[targNum][i].src < src->layoutType->locations.size() && connections[targNum][i].dst < dst->layoutType->locations.size()) {
                        glLineWidth(1.0*lineScaleFactor);

                        glColor4f(0.0, 0.0, 0.0, 0.1);

                        // find if selected
                        bool isSelected = false;

                        for (int j = 0; j < (int) selection.count(); ++j) {
                            if (i == (int) selection[j].row())
                            {
                                glLineWidth(2.0*lineScaleFactor);
                                glColor4f(1.0, 0.0, 0.0, 1.0);
                                isSelected = true;
                                break;
                            }
                            if (connections[targNum][i].src == connections[targNum][selection[j].row()].src && selection[j].column() == 0)
                            {
                                glLineWidth(1.5*lineScaleFactor);
                                glColor4f(0.0, 1.0, 0.0, 0.8);
                                isSelected = true;
                            }
                            if (connections[targNum][i].dst == connections[targNum][selection[j].row()].dst && selection[j].column() == 1)
                            {
                                glLineWidth(1.5*lineScaleFactor);
                                glColor4f(0.0, 1.0, 0.0, 0.8);
                                isSelected = true;
                            }
                        }

                        // also check if we have logData
                        if (connLogs[targNum]) {
                            if (connLogs[targNum]->dataClass == ANALOGDATA && connLogVals[targNum].size() > 0) {
                                // colour and size the connections based on input
                                double val = connLogVals[targNum][connections[targNum][i].src];
                                glLineWidth(4.0*val);
                                glColor4f(val, val, val, 1.0);
                                isSelected = true;
                            } else if (connLogs[targNum]->dataClass == EVENTDATA)  {
                                // mark up each event
                                for (int e = 0; e < connLogVals[targNum].size(); ++e) {
                                    if ((int)connLogVals[targNum][e] == connections[targNum][i].src) {
                                        glLineWidth(1.5*lineScaleFactor);
                                        glColor4f(1.0, 0.0, 0.0, 1.0);
                                        if (connections[targNum][i].src >= connDecay[targNum].size()) this->connDecay[targNum].resize(connections[targNum][i].src+1);
                                        this->connDecay[targNum][connections[targNum][i].src] = 1;
                                        isSelected = true;
                                    }
                                }
                            }
                        }

                        // do event decay
                        if (connections[targNum][i].src < connDecay[targNum].size() && connDecay[targNum][connections[targNum][i].src] > 0) {
                            ++connDecay[targNum][connections[targNum][i].src];
                            if (connDecay[targNum][connections[targNum][i].src] > 10) {
                                connDecay[targNum][connections[targNum][i].src]= 0;
                            }
                            glLineWidth(connDecay[targNum][connections[targNum][i].src]*lineScaleFactor);
                            glColor4f(1.0, 1.0, 1.0, 1.0/(connDecay[targNum][connections[targNum][i].src]));
                            isSelected = true;
                        }


                        if (((int) connections[targNum][i].src == selectedIndex && selectedType == 1) \
                                || ((int) connections[targNum][i].dst == selectedIndex && selectedType == 2))
                        {
                            glLineWidth(2.5*lineScaleFactor*count);
                            glColor4f(0.9, 0.9, 1.0, 1.0*(1/float(count)));
                            isSelected = true;
                        }

                        if (isSelected) {
                           // draw in

                           // Decide the control points
                           GLfloat ctrlpoints[aux_strength+2][3];
                           for (int strenghtIndex = 1; strenghtIndex <= aux_strength; strenghtIndex++) {
                               ctrlpoints[strenghtIndex][0] = center[0];
                               ctrlpoints[strenghtIndex][1] = center[1];
                               ctrlpoints[strenghtIndex][2] = center[2];
                           }

                           if (src->isVisualised && dst->isVisualised) {
                               ctrlpoints[0][0] = src->layoutType->locations[connections[targNum][i].src].x+srcX;
                               ctrlpoints[0][1] = src->layoutType->locations[connections[targNum][i].src].y+srcY;
                               ctrlpoints[0][2] = src->layoutType->locations[connections[targNum][i].src].z+srcZ;
                               ctrlpoints[aux_strength+1][0] = dst->layoutType->locations[connections[targNum][i].dst].x+dstX;
                               ctrlpoints[aux_strength+1][1] = dst->layoutType->locations[connections[targNum][i].dst].y+dstY;
                               ctrlpoints[aux_strength+1][2] = dst->layoutType->locations[connections[targNum][i].dst].z+dstZ;
                           }
                           if (src->isVisualised && !dst->isVisualised) {
                               ctrlpoints[0][0] = src->layoutType->locations[connections[targNum][i].src].x;
                               ctrlpoints[0][1] = src->layoutType->locations[connections[targNum][i].src].y;
                               ctrlpoints[0][2] = src->layoutType->locations[connections[targNum][i].src].z;
                               ctrlpoints[aux_strength+1][0] = dstX;
                               ctrlpoints[aux_strength+1][1] = dstY;
                               ctrlpoints[aux_strength+1][2] = dstZ;
                           }
                           if (!src->isVisualised && dst->isVisualised) {
                               ctrlpoints[0][0] = src->loc3.x;
                               ctrlpoints[0][1] = src->loc3.y;
                               ctrlpoints[0][2] = src->loc3.z;
                               ctrlpoints[aux_strength+1][0] = dst->layoutType->locations[connections[targNum][i].dst].x;
                               ctrlpoints[aux_strength+1][1] = dst->layoutType->locations[connections[targNum][i].dst].y;
                               ctrlpoints[aux_strength+1][2] = dst->layoutType->locations[connections[targNum][i].dst].z;
                           }


                           glMap1f(GL_MAP1_VERTEX_3, 0.0, 1.0, 3, aux_strength+2, &ctrlpoints[0][0]);
                           glEnable(GL_MAP1_VERTEX_3);

                           // Draw the line between the neurons
                           glBegin(GL_LINE_STRIP);

                           for (int k = 0; k <= 30; k++)
                               glEvalCoord1f((GLfloat) k/30.0);

                           glEnd();
                       }
                    } else {
                        // ERR - CONNECTION INDEX OUT OF RANGE
                    }
                }
            }
            glEnable(GL_DEPTH_TEST);
            connGenerationMutex->unlock();

        }


        if (conn->type == OnetoOne) {

            if (src->numNeurons == dst->numNeurons) {

                if (src->layoutType->locations.size() > 0 && dst->layoutType->locations.size() > 0) {

                    for (int i = 0; i < src->layoutType->locations.size(); ++i) {

                        glLineWidth(1.5*lineScaleFactor);
                        glColor4f(0.0, 0.0, 1.0, 0.8);

                         // draw in (old code - straight lines only...)
                        /*glBegin(GL_LINES);
                        glVertex3f(src->layoutType->locations[i].x+srcX, src->layoutType->locations[i].y+srcY, src->layoutType->locations[i].z+srcZ);
                        glVertex3f(dst->layoutType->locations[i].x+dstX, dst->layoutType->locations[i].y+dstY, dst->layoutType->locations[i].z+dstZ);
                        glEnd();*/

                        // also check if we have logData
                        if (connLogs[targNum]) {
                            if (connLogs[targNum]->dataClass == ANALOGDATA && connLogVals[targNum].size() > 0) {
                                // colour and size the connections based on input
                                double val = connLogVals[targNum][i];
                                glLineWidth(4.0*val);
                                glColor4f(val, val, val, 1.0);
                            } else if (connLogs[targNum]->dataClass == EVENTDATA)  {
                                // mark up each event
                                for (int e = 0; e < connLogVals[targNum].size(); ++e) {
                                    if ((int)connLogVals[targNum][e] == i) {
                                        glLineWidth(1.5*lineScaleFactor);
                                        glColor4f(1.0, 0.0, 0.0, 1.0);
                                        if (i >= connDecay[targNum].size()) this->connDecay[targNum].resize(i+1);
                                        this->connDecay[targNum][i] = 1;
                                    }
                                }
                            }
                        }

                        // do event decay
                        if (i < connDecay[targNum].size() && connDecay[targNum][i] > 0) {
                            ++connDecay[targNum][i];
                            if (connDecay[targNum][i] > 10) {
                                connDecay[targNum][i]= 0;
                            }
                            glLineWidth(connDecay[targNum][i]*lineScaleFactor);
                            glColor4f(1.0, 1.0, 1.0, 1.0/(connDecay[targNum][i]));
                        }



                        // Decide the control points
                        GLfloat ctrlpoints[aux_strength+2][3];
                        for (int strenghtIndex = 1; strenghtIndex <= aux_strength; strenghtIndex++) {
                            ctrlpoints[strenghtIndex][0] = center[0];
                            ctrlpoints[strenghtIndex][1] = center[1];
                            ctrlpoints[strenghtIndex][2] = center[2];
                        }

                        ctrlpoints[0][0] = src->layoutType->locations[i].x+srcX;
                        ctrlpoints[0][1] = src->layoutType->locations[i].y+srcY;
                        ctrlpoints[0][2] = src->layoutType->locations[i].z+srcZ;
                        ctrlpoints[aux_strength+1][0] = dst->layoutType->locations[i].x+dstX;
                        ctrlpoints[aux_strength+1][1] = dst->layoutType->locations[i].y+dstY;
                        ctrlpoints[aux_strength+1][2] = dst->layoutType->locations[i].z+dstZ;

                        glMap1f(GL_MAP1_VERTEX_3, 0.0, 1.0, 3, aux_strength+2, &ctrlpoints[0][0]);
                        glEnable(GL_MAP1_VERTEX_3);

                        // Draw the line between the neurons
                        glBegin(GL_LINE_STRIP);

                        for (int k = 0; k <= 30; k++)
                            glEvalCoord1f((GLfloat) k/30.0);

                        glEnd();


                    }
                }

                if (src->layoutType->locations.size() > 0 && dst->layoutType->locations.size() == 0) {

                    for (int i = 0; i < src->layoutType->locations.size(); ++i) {

                        glLineWidth(1.5*lineScaleFactor);
                        glColor4f(0.0, 0.0, 1.0, 0.8);
                        // draw in
                        glBegin(GL_LINES);
                        glVertex3f(src->layoutType->locations[i].x+srcX, src->layoutType->locations[i].y+srcY, src->layoutType->locations[i].z+srcZ);
                        glVertex3f(dstX, dstY, dstZ);
                        glEnd();

                    }
                }

                if (src->layoutType->locations.size() == 0 && dst->layoutType->locations.size() > 0) {

                    for (int i = 0; i < dst->layoutType->locations.size(); ++i) {

                        glLineWidth(1.5*lineScaleFactor);
                        glColor4f(0.0, 0.0, 1.0, 0.8);
                        // draw in
                        glBegin(GL_LINES);
                        glVertex3f(srcX, srcY, srcZ);
                        glVertex3f(dst->layoutType->locations[i].x+dst->loc3.x, dst->layoutType->locations[i].y+dst->loc3.y, dst->layoutType->locations[i].z+dst->loc3.z);
                        glEnd();

                    }
                }
            }
        }

        if (conn->type == AlltoAll) {

            if (src->layoutType->locations.size() > 0 && dst->layoutType->locations.size() > 0) {

                for (int i = 0; i < src->layoutType->locations.size(); ++i) {
                    for (int j = 0; j <  dst->layoutType->locations.size(); ++j) {

                        glLineWidth(1.5*lineScaleFactor);
                        glColor4f(0.0, 0.0, 1.0, 0.2);
                        // draw in
                        glBegin(GL_LINES);
                        glVertex3f(src->layoutType->locations[i].x+srcX, src->layoutType->locations[i].y+srcY, src->layoutType->locations[i].z+srcZ);
                        glVertex3f(dst->layoutType->locations[j].x+dstX, dst->layoutType->locations[j].y+dstY, dst->layoutType->locations[j].z+dstZ);
                        glEnd();

                    }
                }
            }
            if (src->layoutType->locations.size() > 0 && dst->layoutType->locations.size() == 0) {

                for (int i = 0; i < src->layoutType->locations.size(); ++i) {

                    glLineWidth(1.5*lineScaleFactor);
                    glColor4f(0.0, 0.0, 1.0, 0.2);
                    // draw in
                    glBegin(GL_LINES);
                    glVertex3f(src->layoutType->locations[i].x+srcX, src->layoutType->locations[i].y+srcY, src->layoutType->locations[i].z+srcZ);
                    glVertex3f(dstX, dstY, dstZ);
                    glEnd();
                }
            }
            if (src->layoutType->locations.size() == 0 && dst->layoutType->locations.size() > 0) {

                 for (int j = 0; j <  dst->layoutType->locations.size(); ++j) {

                    glLineWidth(1.5*lineScaleFactor);
                    glColor4f(0.0, 0.0, 1.0, 0.2);
                    // draw in
                    glBegin(GL_LINES);
                    glVertex3f(srcX, srcY, srcZ);
                    glVertex3f(dst->layoutType->locations[j].x+dstX, dst->layoutType->locations[j].y+dstY, dst->layoutType->locations[j].z+dstZ);
                    glEnd();
                }
            }
        }

        if (conn->type == FixedProb) {

            fixedProb_connection * fpConn = dynamic_cast <fixedProb_connection *> (conn);
            CHECK_CAST(fpConn)

            random.setSeed(fpConn->seed);

            prob = fpConn->p;

            // generate a list of projections to highlight
            QVector < loc > redrawLocs;

            for (int i = 0; i < src->layoutType->locations.size(); ++i) {
                for (int j = 0; j <  dst->layoutType->locations.size(); ++j) {
                    if (random.value() < this->prob) {
                        glLineWidth(1.0*lineScaleFactor);
                        glColor4f(0.0, 0.0, 0.0, 0.1);

                        if (((int) i == selectedIndex && selectedType == 1) \
                                || ((int) j == selectedIndex && selectedType == 2))
                        {
                            // store for redraw of selected connections
                            loc pstart;
                            loc pend;

                            if ((src->layoutType->locations.size() > 0 && dst->layoutType->locations.size() > 0) \
                                    || (src->layoutType->locations.size() > 0 && dst->layoutType->locations.size() == 0)) {
                                pstart.x = src->layoutType->locations[i].x+srcX;
                                pstart.y = src->layoutType->locations[i].y+srcY;
                                pstart.z = src->layoutType->locations[i].z+srcZ;
                            }
                            if (src->layoutType->locations.size() == 0 && dst->layoutType->locations.size() > 0) {
                                pstart.x = srcX;
                                pstart.y = srcY;
                                pstart.z = srcZ;
                            }
                            if ((src->layoutType->locations.size() > 0 && dst->layoutType->locations.size() > 0) \
                                    || (src->layoutType->locations.size() == 0 && dst->layoutType->locations.size() > 0)) {
                                pend.x = dst->layoutType->locations[j].x+dstX;
                                pend.y = dst->layoutType->locations[j].y+dstY;
                                pend.z = dst->layoutType->locations[j].z+dstZ;
                            }
                            if (src->layoutType->locations.size() > 0 && dst->layoutType->locations.size() == 0) {
                                pend.x = dstX;
                                pend.y = dstY;
                                pend.z = dstZ;
                            }
                            redrawLocs.push_back(pstart);
                            redrawLocs.push_back(pend);
                        }
                        else
                        {
                            // draw in
                            glBegin(GL_LINES);
                            if (src->layoutType->locations.size() > 0 && dst->layoutType->locations.size() > 0) {
                                glVertex3f(src->layoutType->locations[i].x+srcX, src->layoutType->locations[i].y+srcY, src->layoutType->locations[i].z+srcZ);
                                glVertex3f(dst->layoutType->locations[j].x+dstX, dst->layoutType->locations[j].y+dstY, dst->layoutType->locations[j].z+dstZ);
                            }
                            if (src->layoutType->locations.size() > 0 && dst->layoutType->locations.size() == 0) {
                                glVertex3f(src->layoutType->locations[i].x+srcX, src->layoutType->locations[i].y+srcY, src->layoutType->locations[i].z+srcZ);
                                glVertex3f(dstX, dstY, dstZ);
                            }
                            if (src->layoutType->locations.size() == 0 && dst->layoutType->locations.size() > 0) {
                                glVertex3f(srcX, srcY, srcZ);
                                glVertex3f(dst->layoutType->locations[j].x+dstX, dst->layoutType->locations[j].y+dstY, dst->layoutType->locations[j].z+dstZ);
                            }
                            glEnd();
                        }
                    }
                }
            }
            // redraw selected (over the top of everything else so no depth test):
            glDisable(GL_DEPTH_TEST);

            for (int i=0; i < redrawLocs.size(); i+=2) {

                // redraw
                glLineWidth(1.5*lineScaleFactor);
                glColor4f(0.0, 0.0, 1.0, 0.8);
                glBegin(GL_LINES);
                glVertex3f(redrawLocs[i].x, redrawLocs[i].y,redrawLocs[i].z);
                glVertex3f(redrawLocs[i+1].x, redrawLocs[i+1].y,redrawLocs[i+1].z);
                glEnd();
            }


        }


        glEnable(GL_DEPTH_TEST);
        glEnable(GL_LIGHTING);
    }

    glDisable(GL_BLEND);
    glDisable(GL_POLYGON_SMOOTH);
    glDisable(GL_LINE_SMOOTH);

    glDisable(GL_DEPTH_TEST);
    glShadeModel(GL_FLAT);
    glDisable(GL_LIGHTING);
    glDisable(GL_COLOR_MATERIAL);

    glMatrixMode(GL_MODELVIEW);


    if (popIndicesShown) {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        QPen pen = painter.pen();
        QPen oldPen = pen;
        pen.setColor(QColor(0,0,0,255));
        painter.setPen(pen);

        float zoomVal = zoomFactor;
        if (zoomVal < 0.3)
            zoomVal = 0.3;

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

                winX /= RETINA_SUPPORT;
                winY /= RETINA_SUPPORT;

                if (orthoView) {
                    winX += this->width()/4.0;
                    winY -= this->height()/4.0;
                }

                if (imageSaveMode) {}
                    //painter.drawText(QRect(winX-(1.0-winZ)*220-20,imageSaveHeight-winY-(1.0-winZ)*220-10,40,20),QString::number(float(i)));
                else
                    if (orthoView)
                        painter.drawText(QRect(winX-(1.0-winZ)*220-10.0/zoomVal-10,this->height()-winY-(1.0-winZ)*220-10.0/zoomVal-10,40,20),QString::number(float(i)));
                    else
                        painter.drawText(QRect(winX-(1.0-winZ)*300-10.0/zoomVal,this->height()-winY-(1.0-winZ)*300-10.0/zoomVal,40,20),QString::number(float(i)));
                    //painter.drawText(QRect(winX-(1.0-winZ)*600,this->height()-winY-(1.0-winZ)*600,40,20),QString::number(float(i)));
                    //painter.drawText(QRect((winX-(1.0-winZ)*220-20),this->height()-(winY-(1.0-winZ)*220+50),40,20),QString::number(float(i)));

                glPopMatrix();
            }
        }
        painter.setPen(oldPen);
        painter.end();
    } else {
        // if the painter isn't there this doesn't get called!
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.end();

        // Make sure the clicked population exists and is visible
        if (0 <= clickedPopulation &&  clickedPopulation <= data->populations.size())
        {
            if (data->populations[clickedPopulation]->isVisualised)
            {
                QSharedPointer <population> currPop = qSharedPointerDynamicCast <population> (data->populations[clickedPopulation]);
                CHECK_CAST(currPop);

                if (0 <= clickedNeuron && clickedNeuron <= currPop->layoutType->locations.size())
                {
                    QPainter painter(this);
                    painter.setRenderHint(QPainter::Antialiasing);

                    QPen pen = painter.pen();
                    QPen oldPen = pen;
                    pen.setColor(QColor(0,0,0,255));
                    painter.setPen(pen);

                    float zoomVal = zoomFactor;
                    if (zoomVal < 0.3)
                        zoomVal = 0.3;

                    // draw text

                    glPushMatrix();

                    glTranslatef(currPop->layoutType->locations[clickedNeuron].x, currPop->layoutType->locations[clickedNeuron].y, currPop->layoutType->locations[clickedNeuron].z);

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

                    winX /= RETINA_SUPPORT;
                    winY /= RETINA_SUPPORT;

                    if (orthoView) {
                        winX += this->width()/4.0;
                        winY -= this->height()/4.0;
                    }


                    QString neuronInfo = "";
                    neuronInfo += "Population: " + currPop->getName();
                    neuronInfo += "\n";
                    neuronInfo += "Neuron: " + QString::number(float(clickedNeuron));
                    QRect textRect;

                    if (imageSaveMode) {}
                        //painter.drawText(QRect(winX-(1.0-winZ)*220-20,imageSaveHeight-winY-(1.0-winZ)*220-10,40,20),QString::number(float(i)));
                    else {
                        if (orthoView)
                            textRect = QRect(winX-(1.0-winZ)*220-10.0/zoomVal-10,this->height()-winY-(1.0-winZ)*220-10.0/zoomVal-10,80,30);
                        else
                            textRect = QRect(winX-(1.0-winZ)*300-10.0/zoomVal,this->height()-winY-(1.0-winZ)*300-10.0/zoomVal,80,30);

                        painter.drawText(textRect, Qt::TextDontClip, neuronInfo);
                        painter.setBrush(Qt::NoBrush);
                        QFont myFont;
                        QFontMetrics fm(myFont);
                        QRect border = fm.boundingRect(textRect, Qt::TextDontClip, neuronInfo);
                        //Add some padding to make sure you can read
                        border.adjust (-5, -5, 5, 5);
                        painter.drawRect(border);
                    }
                        //painter.drawText(QRect(winX-(1.0-winZ)*600,this->height()-winY-(1.0-winZ)*600,40,20),QString::number(float(i)));
                        //painter.drawText(QRect((winX-(1.0-winZ)*220-20),this->height()-(winY-(1.0-winZ)*220+50),40,20),QString::number(float(i)));

                    glPopMatrix();

                    painter.setPen(oldPen);
                    painter.end();
                }
            }
        }
    }

    glPopMatrix();


}

void glConnectionWidget::drawNeuron(GLfloat r, int rings, int segments, QColor col) {

    // draw a sphere to represent a neuron
    int i, j;
    for(i = 0; i <= rings; i++) {
        double rings0 = M_PI * (-0.5 + (double) (i - 1) / rings);
        double z0  = sin(rings0);
        double zr0 =  cos(rings0);

        double rings1 = M_PI * (-0.5 + (double) i / rings);
        double z1 = sin(rings1);
        double zr1 = cos(rings1);

        glBegin(GL_QUAD_STRIP);
        for(j = 0; j <= segments; j++) {
            double segment = 2 * M_PI * (double) (j - 1) / segments;
            double x = cos(segment);
            double y = sin(segment);

            glNormal3f(x * zr0, y * zr0, z0);
            glColor4f(col.redF(), col.greenF(), col.blueF(), col.alphaF());
            glVertex3f(x * zr0*r, y * zr0*r, z0*r);
            glNormal3f(x * zr1, y * zr1, z1);
            glColor4f(col.redF(), col.greenF(), col.blueF(), col.alphaF());
            glVertex3f(x * zr1*r, y * zr1*r, z1*r);
        }
        glEnd();
    }

}

void glConnectionWidget::setupView() {

    int width;
    int height;

    if (imageSaveMode) {
        width = imageSaveWidth;
        height = imageSaveHeight;
    } else {
        width = this->width()*RETINA_SUPPORT;
        height = this->height()*RETINA_SUPPORT;
    }

    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // move view
    if (!orthoView)
        gluPerspective(60.0,((GLfloat)width)/((GLfloat)height), 1.0, 100000.0);
    else {
        float scale = zoomFactor*10.0;
        float aspect = ((GLfloat)height)/((GLfloat)width);
        glOrtho(-scale, scale, -scale*aspect, scale*aspect, -100, 100000.0);
    }


    // preview mode
    if (locations.size() > 0) {

        loc maxes;
        loc mins;

        maxes.x = -INFINITY; maxes.y = -INFINITY; maxes.z = -INFINITY;
        mins.x = INFINITY; mins.y = INFINITY; mins.z = INFINITY;

        for (int locNum = 0; locNum < locations[0].size(); ++locNum) {
            if (locations[0][locNum].x > maxes.x) {
                maxes.x = locations[0][locNum].x;
            }
            if (locations[0][locNum].y > maxes.y) {
                maxes.y = locations[0][locNum].y;
            }
            if (locations[0][locNum].z > maxes.z) {
                maxes.z = locations[0][locNum].z;
            }
            if (locations[0][locNum].x < mins.x) {
                mins.x = locations[0][locNum].x;
            }
            if (locations[0][locNum].y < mins.y) {
                mins.y = locations[0][locNum].y;
            }
            if (locations[0][locNum].z < mins.z) {
                mins.z = locations[0][locNum].z;
            }
        }


        // now we have max and min for each direction, calculate a view that takes everything in...
        // find the max length of an edge:
        loc lengths;
        lengths.x = maxes.x - mins.x;
        lengths.y = maxes.y - mins.y;
        lengths.z = maxes.z - mins.z;

        //int maxDir = 0;
        float maxLen = lengths.x;
        if (lengths.y > maxLen) {maxLen = lengths.y; /*maxDir = 1;*/}
        if (lengths.z > maxLen) {maxLen = lengths.z; /*maxDir = 2;*/}

        // scale based on max length
        glTranslatef(0,0,-maxLen*1.3);
        //glTranslatef(pos.x(),pos.y(),0);

        // rotate
        glRotatef(-45.0f, 1.0f,0.0f,0.0f);
        glTranslatef(0.0f,2.5f,2.5f);
        glRotatef(45.0f, 0.0f,0.0f,1.0f);


        // translate to centre
        loc centres;
        centres.x = (maxes.x + mins.x) / 2.0f;
        centres.y = (maxes.y + mins.y) / 2.0f;
        centres.z = (maxes.z + mins.z) / 2.0f;

        glTranslatef(-centres.x,-centres.y,-centres.z);


        glMatrixMode(GL_MODELVIEW);

        return;
    }
    else if (selectedPops.size() > 0) {

        // default to 1st pop
        int selIndex = 0;

        // find selected object
        if (selectedObject == NULL) {
            selIndex = 0;
        }
        else {
            for (int i = 0; i < selectedPops.size(); ++i) {
                if (selectedObject == selectedPops[i])
                    selIndex = i;
            }
        }

        QSharedPointer <population> currPop = selectedPops[selIndex];

        // work out extent of view:
        // find max and min vals in each direction:
        loc maxes;
        loc mins;

        maxes.x = -INFINITY; maxes.y = -INFINITY; maxes.z = -INFINITY;
        mins.x = INFINITY; mins.y = INFINITY; mins.z = INFINITY;

        for (int locNum = 0; locNum < currPop->layoutType->locations.size(); ++locNum) {
            if (currPop->layoutType->locations[locNum].x > maxes.x) {
                maxes.x = currPop->layoutType->locations[locNum].x;
            }
            if (currPop->layoutType->locations[locNum].y > maxes.y) {
                maxes.y = currPop->layoutType->locations[locNum].y;
            }
            if (currPop->layoutType->locations[locNum].z > maxes.z) {
                maxes.z = currPop->layoutType->locations[locNum].z;
            }
            if (currPop->layoutType->locations[locNum].x < mins.x) {
                mins.x = currPop->layoutType->locations[locNum].x;
            }
            if (currPop->layoutType->locations[locNum].y < mins.y) {
                mins.y = currPop->layoutType->locations[locNum].y;
            }
            if (currPop->layoutType->locations[locNum].z < mins.z) {
                mins.z = currPop->layoutType->locations[locNum].z;
            }
        }


        // now we have max and min for each direction, calculate a view that takes everything in...
        // find the max length of an edge:
        loc lengths;
        lengths.x = maxes.x - mins.x;
        lengths.y = maxes.y - mins.y;
        lengths.z = maxes.z - mins.z;

        //int maxDir = 0;
        float maxLen = lengths.x;
        if (lengths.y > maxLen) {maxLen = lengths.y; /*maxDir = 1;*/}
        if (lengths.z > maxLen) {maxLen = lengths.z; /*maxDir = 2;*/}

        // scale based on max length
        glTranslatef(0,0,-maxLen*1.2*zoomFactor);
        glTranslatef(pos.x(),pos.y(),0);

        // rotate
        glRotatef(-45.0f+rot.y(), 1.0f,0.0f,0.0f);
        glTranslatef(0.0f,2.5f,2.5f);
        glRotatef(45.0f+rot.x(), 0.0f,0.0f,1.0f);


        // translate to centre
        loc centres;
        centres.x = (maxes.x + mins.x) / 2.0f;
        centres.y = (maxes.y + mins.y) / 2.0f;
        centres.z = (maxes.z + mins.z) / 2.0f;

        glTranslatef(-centres.x,-centres.y,-centres.z);


        glMatrixMode(GL_MODELVIEW);

        return;


    }

    glTranslatef(0.0f,0.0f,-10.0f*zoomFactor);
    glTranslatef(pos.x(),0.0f,0.0f);
    glTranslatef(0.0f,pos.y(),0.0f);
    glRotatef(-45.0f, 1.0f,0.0f,0.0f);
    glRotatef(45.0f, 0.0f,0.0f,1.0f);

    glMatrixMode(GL_MODELVIEW);


}

void glConnectionWidget::selectionChanged(QItemSelection top, QItemSelection) {

    // reset nrn index etc...
    selectedType = 1;
    selectedIndex = -1;

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

            QSharedPointer <projection> currProj = qSharedPointerDynamicCast<projection> (currPop->projections[j]);
            CHECK_CAST(currProj)

            if (currProj->getName() == item->name)
                selectedObject = currProj;

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
    //qDebug() << "selected = " << selectedObject->getName();
    // redraw
    this->repaint();

}

void glConnectionWidget::typeChanged(int) {

    QString type = sender()->property("type").toString();

    if (type == "layout") {

        if (selectedObject->type != populationObject)
            qDebug() << "WARNING - bug found - report code 1";

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



    // redraw
    this->repaint();

}

void glConnectionWidget::parsChangedPopulation(int value) {

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

    // redraw
    this->repaint();

}

void glConnectionWidget::parsChangedPopulation(double) {

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

    createPopulationsDL();
    createConnectionsDL();

    // redraw
    this->repaint();

}

void glConnectionWidget::parsChangedPopulation() {


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

    createPopulationsDL();
    createConnectionsDL();

    // redraw
    this->repaint();

}

void glConnectionWidget::parsChangedProjections() {

    this->refreshAll();

    for (int i = 0; i < selectedConns.size(); ++i) {

        connection * conn;

        if (selectedConns[i]->type == synapseObject) {
            QSharedPointer <synapse> currTarg = qSharedPointerDynamicCast <synapse> (selectedConns[i]);
            conn = currTarg->connectionType;
        } else {
            QSharedPointer<genericInput> currIn = qSharedPointerDynamicCast<genericInput> (selectedConns[i]);
            conn = currIn->connectionType;
        }


        // regrab data for kernel based
        if (conn->type == Kernel) {

            // refresh the connections
            if (((kernel_connection *) conn)->changed()) {
                connections[i].clear();
                // launch version increment dialog box:
                generate_dialog generate(((kernel_connection *) conn), ((kernel_connection *) conn)->src, ((kernel_connection *) conn)->dst, connections[i], connGenerationMutex, this);
                bool retVal = generate.exec();
                if (!retVal) {
                    return;
                }
                ((kernel_connection *) conn)->connections = connections[i];
                ((kernel_connection *) conn)->setUnchanged(true);
                connDecay[i].resize(connections[i].size());
            }
        }

        // regrab data for kernel based
        if (conn->type == Python) {

            // refresh the connections
            if (((pythonscript_connection *) conn)->changed()) {
                connections[i].clear();
                // launch version increment dialog box:
                generate_dialog generate(((pythonscript_connection *) conn), ((pythonscript_connection *) conn)->src, ((pythonscript_connection *) conn)->dst, connections[i], connGenerationMutex, this);
                bool retVal = generate.exec();
                if (!retVal) {
                    return;
                }
                ((pythonscript_connection *) conn)->connections = connections[i];
                ((pythonscript_connection *) conn)->setUnchanged(true);
                connDecay[i].resize(connections[i].size());
            }
        }


    }

    createPopulationsDL();
    createConnectionsDL();

    // redraw:
    repaint();

}

void glConnectionWidget::parsChangedProjection() {

    /*if (!data->isValidPointer(selectedObject))
        return;*/ //NOT NEEDED ANYMORE

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
            conn = currIn->connectionType;
            src = qSharedPointerDynamicCast <population> (currIn->source); // would not be here if was not true
            dst = qSharedPointerDynamicCast <population> (currIn->destination);
        }

        if (conn->type == CSV) {

            // generated connection
            this->getConnections();

        }

        // regrab data for kernel based
        if (conn->type == Kernel) {

            // update the projection:


            // find selected object
            for (int i = 0; i < this->selectedConns.size(); ++i) {

                if (selectedObject == selectedConns[i]) {

                    // refresh the connections
                    if (((kernel_connection *) conn)->changed()) {
                        connections[i].clear();
                        // launch version increment dialog box:
                        generate_dialog generate(((kernel_connection *) conn), src, dst, connections[i], connGenerationMutex, this);
                        bool retVal = generate.exec();
                        if (!retVal) {
                            return;
                        }
                        ((kernel_connection *) conn)->connections = connections[i];
                        ((kernel_connection *) conn)->setUnchanged(true);
                        connDecay[i].resize(connections[i].size());
                    }
                }

            }

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
                        connDecay[i].resize(connections[i].size());
                    }
                }

            }

        }


        // redraw:
        repaint();

    }

}

// after switching views, check we haven't broken anything!
void glConnectionWidget::refreshAll() {

    // check on populations
    for (int i = 0; i < selectedPops.size(); ++i) {

        // check pointer is valid - NOT NEEDED ANYMORE!
        /*if (!data->isValidPointer(selectedPops[i])) {
            // remove
            selectedPops.erase(selectedPops.begin()+i);
            popLogs.erase(popLogs.begin()+i);
            popColours.erase(popColours.begin()+i);
            --i;
            continue;
        }*/

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

        // check pointer is valid
        /*if (!data->isValidPointer(selectedConns[i])) {
            // remove
            selectedConns.erase(selectedConns.begin()+i);
            connections.erase(connections.begin()+i);
            --i;
            continue;
        } NOT NEEDED ANYMORE */

        // check it isn't deleted
        if (selectedConns[i]->isDeleted) {
            // remove
            selectedConns.erase(selectedConns.begin()+i);
            connections.erase(connections.begin()+i);
            connDecay.erase(connDecay.begin()+1);
            this->connLogs.erase(connLogs.begin()+1);
            this->connLogVals.erase(connLogVals.begin()+1);
            --i;
            continue;
        }


    }

    createPopulationsDL();
    createConnectionsDL();

}


void glConnectionWidget::setConnType(connectionType cType) {

    this->currProjectionType = cType;

}

void glConnectionWidget::drawLocations(QVector <loc> locs) {

    // redraw based on a set of location passed in (used for layout previews)
    for (int i = 0; i < locations.size(); ++i) {
        locations[i].clear();
    }
    locations.clear();

    this->locations.push_back(locs);

    // redraw
    this->repaint();

}

void glConnectionWidget::clearLocations() {

    // clear the set of location passed in (used for layout previews)
    for (int i = 0; i < locations.size(); ++i) {
        locations[i].clear();
    }
    locations.clear();
}

void glConnectionWidget::setConnectionsModel(QAbstractTableModel * modelIn) {
    model = modelIn;
}

QAbstractTableModel * glConnectionWidget::getConnectionsModel() {
    return model;
}

void glConnectionWidget::getConnections() {

    if (selectedObject != NULL) {
        if (selectedObject->type == synapseObject) {

            QSharedPointer <synapse> currTarg = qSharedPointerDynamicCast <synapse> (selectedObject);

            for (int i = 0; i < this->selectedConns.size(); ++i) {

                if (selectedObject == selectedConns[i] && currTarg->connectionType->type == CSV) {

                    // refresh the connections
                    connections[i].clear();
                    ((csv_connection *) currTarg->connectionType)->getAllData(connections[i]);
                    this->connDecay[i].resize(connections[i].size());
                    //qDebug() << "FETCHING CONNS";

                }
            }
        }
    }

    // force a redraw
    repaint();
}

void glConnectionWidget::sysSelectionChanged(QModelIndex, QModelIndex) {

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
                            connDecay.resize(connections.size()+1);
                            connLogs.push_back(NULL);
                            connLogVals.resize(connLogVals.size()+1);

                            if (currIn->connectionType->type == CSV) {
                                // load in the connections
                                ((csv_connection *) currIn->connectionType)->getAllData(connections.back());
                            } else if (currIn->connectionType->type == Kernel) {
                                if (((kernel_connection *) currIn->connectionType)->connections.size() > 0 && !((kernel_connection *) currIn->connectionType)->changed()) {
                                    connections.back() = ((kernel_connection *) currIn->connectionType)->connections;
                                } else {
                                    // generate
                                    // launch version increment dialog box:
                                    QSharedPointer <population> popSrc = qSharedPointerDynamicCast <population> (currIn->source);
                                    QSharedPointer <population> popDst = qSharedPointerDynamicCast <population> (currIn->destination);
                                    generate_dialog generate(((kernel_connection *) currIn->connectionType), popSrc, popDst, connections.back(), connGenerationMutex, this);
                                    bool retVal = generate.exec();
                                    if (!retVal) {
                                        continue;
                                    }
                                    ((kernel_connection *) currIn->connectionType)->connections = connections.back();
                                    ((kernel_connection *) currIn->connectionType)->setUnchanged(true);
                                }
                            } else if (currIn->connectionType->type == Python) {
                                if (((pythonscript_connection *) currIn->connectionType)->connections.size() > 0 && !((pythonscript_connection *) currIn->connectionType)->changed()) {
                                    connections.back() = ((pythonscript_connection *) currIn->connectionType)->connections;
                                } else {
                                    // generate
                                    // launch version increment dialog box:
                                    QSharedPointer <population> popSrc = qSharedPointerDynamicCast <population> (currIn->source);
                                    QSharedPointer <population> popDst = qSharedPointerDynamicCast <population> (currIn->destination);
                                    generate_dialog generate(((pythonscript_connection *) currIn->connectionType), popSrc, popDst, connections.back(), connGenerationMutex, this);
                                    bool retVal = generate.exec();
                                    if (!retVal) {
                                        continue;
                                    }
                                    ((pythonscript_connection *) currIn->connectionType)->connections = connections.back();
                                    ((pythonscript_connection *) currIn->connectionType)->setUnchanged(true);
                                }
                            }

                            //An element was added to the list, the display list needs to be recreated
                            createConnectionsDL();

                        }

                    } else {
                        // remove from list if there
                        for (int p = 0; p < this->selectedConns.size(); ++p) {
                            if (selectedConns[p] == currIn) {
                                selectedConns.erase(selectedConns.begin()+p);
                                connections.erase(connections.begin()+p);
                                connDecay.erase(connDecay.begin()+p);
                                connLogs.erase(connLogs.begin()+p);
                                connLogVals.erase(connLogVals.begin()+p);
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
                        connDecay.resize(connDecay.size()+1);
                        connLogs.push_back(NULL);
                        connLogVals.resize(connLogVals.size()+1);

                        if (currTarg->connectionType->type == CSV) {
                            // load in the connections
                            ((csv_connection *) currTarg->connectionType)->getAllData(connections.back());
                        } else if (currTarg->connectionType->type == Kernel) {
                            if (((kernel_connection *) currTarg->connectionType)->connections.size() > 0 && !((kernel_connection *) currTarg->connectionType)->changed()) {
                                connections.back() = ((kernel_connection *) currTarg->connectionType)->connections;
                            } else {
                                // generate
                                // launch version increment dialog box:
                                generate_dialog generate(((kernel_connection *) currTarg->connectionType), currTarg->proj->source, currTarg->proj->destination, connections.back(), connGenerationMutex, this);
                                bool retVal = generate.exec();
                                if (!retVal) {
                                    return;
                                }
                                ((kernel_connection *) currTarg->connectionType)->connections = connections.back();
                                ((kernel_connection *) currTarg->connectionType)->setUnchanged(true);
                            }
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

                        //An element was added to the list, the display list needs to be recreated
                        createConnectionsDL();

                    }
                } else {
                    // if in list then remove from list
                    for (int p = 0; p < this->selectedConns.size(); ++p) {
                        if (selectedConns[p] == currTarg) {
                            selectedConns.erase(selectedConns.begin()+p);
                            connections.erase(connections.begin()+p);
                            connDecay.erase(connDecay.begin()+p);
                            connLogs.erase(connLogs.begin()+p);
                            connLogVals.erase(connLogVals.begin()+p);
                        }
                    }
                }

            }

        }

    }
    // check for logs:
    addLogs(&data->main->viewGV.properties->logs);

    // force redraw!
    this->repaint();

}

void glConnectionWidget::connectionDataChanged(QModelIndex, QModelIndex) {

    // refetch the connections:
    getConnections();

}

void glConnectionWidget::connectionSelectionChanged(QItemSelection, QItemSelection) {

    this->selectedIndex = -1;
    this->selectedType = 1;

    this->selection = ((QItemSelectionModel *) sender())->selectedIndexes();
    // force redraw
    repaint();

}

void glConnectionWidget::mousePressEvent(QMouseEvent *event)
{

    setCursor(Qt::ClosedHandCursor);
    button = event->button();
    origPos = event->globalPos();
    origPos.setX(origPos.x() - pos.x()*100/zoomFactor);
    origPos.setY(origPos.y() + pos.y()*100/zoomFactor);
    origRot = event->globalPos();
    origRot.setX(origRot.x() - rot.x()*2);
    origRot.setY(origRot.y() - rot.y()*2);


    // Alg that detectes clicked object ::::ALEX: NOT RETINA SAFE?
    if (button == Qt::LeftButton)
    {
        glDisable(GL_LIGHTING);

        glPushMatrix();
        glLoadIdentity();
        glTranslatef(0,0,-5.0);
        for (int i = 0; i < data->populations.size(); i++) {
            if (data->populations[i]->isVisualised) {
                glPushMatrix();
                // if currently selected
                if (data->populations[i] == selectedObject) {
                    // move to pop location denoted by the spinboxes for x, y, z
                    glTranslatef(loc3Offset.x, loc3Offset.y,loc3Offset.z);
                } else {
                    glTranslatef(data->populations[i]->loc3.x, data->populations[i]->loc3.y,data->populations[i]->loc3.z);
                }

                glCallList(data->populations[i]->dlIndexCol);
                glPopMatrix();
            }
        }
        glPopMatrix();

        // Color picking to detect selected neuron
        unsigned char detectedColor[3];
        GLint viewport[4];
        // The the pixel from the GPU
        glGetIntegerv(GL_VIEWPORT, viewport);
        glReadPixels(event->x()*RETINA_SUPPORT, this->height() - event->y()*RETINA_SUPPORT,1,1, GL_RGB, GL_UNSIGNED_BYTE, detectedColor);

        // Make sure the population exists and is visible
        if (detectedColor[0] <= data->populations.size()) {
            if (data->populations[detectedColor[0]]->isVisualised)
            {
                int selectedNeuron = (detectedColor[2] << 8) + detectedColor[1];

                QSharedPointer < population > currPop = data->populations[detectedColor[0]];

                if (0 <= selectedNeuron && selectedNeuron <= currPop->layoutType->locations.size())
                {
                    clickedPopulation = detectedColor[0];
                    clickedNeuron = selectedNeuron;
                    this->repaint();
                }
                else
                {
                    clickedNeuron = -1;
                }
            }
            else
            {
                clickedPopulation = -1;
            }
        }
        else
        {
            clickedPopulation = -1;
        }
    }

}

void glConnectionWidget::mouseReleaseEvent(QMouseEvent *){
    setCursor(Qt::ArrowCursor);
}

void glConnectionWidget::mouseMoveEvent(QMouseEvent *event){

    if (button == Qt::LeftButton) {
        pos.setX(-(origPos.x() - event->globalPos().x())*0.01*zoomFactor);
        pos.setY((origPos.y() - event->globalPos().y())*0.01*zoomFactor);
    }
    if (button == Qt::RightButton) {
        rot.setX(-(origRot.x() - event->globalPos().x())*0.5);
        rot.setY(-(origRot.y() - event->globalPos().y())*0.5);
    }

    repaint();

}

void glConnectionWidget::wheelEvent(QWheelEvent *event){

    float val = float(event->delta()) / 320.0;
    val = pow(2.0f,val);
    if (event->orientation() == Qt::Vertical) {
        this->zoomFactor *= (val);
        if (this->zoomFactor < 0.00001)
            zoomFactor = 1;
        repaint();
    }

}

void glConnectionWidget::setPopIndicesShown(bool checkState){
    popIndicesShown = checkState;
    repaint();
}

void glConnectionWidget::selectedNrnChanged(int index) {

    QString type = sender()->property("type").toString();

    if (type == "index")
        selectedIndex = index;
    else if (type == "from") {
        selectedType = index+1;
    }

    repaint();

}

/*!
 * \brief glConnectionWidget::createPopulationsDL
 * Murilo's work - create display lists for the Populations
 */
void glConnectionWidget::createPopulationsDL(bool onlyColour)
{
    if (data)
    {
        // fetch quality setting
        QSettings settings;
        int quality = settings.value("glOptions/detail", 5).toInt();

        // draw with a level of detail dependant on the number on neurons we must draw
        // sum neurons across all pops we'll draw
        int totalNeurons = 0;
        for (int locNum = 0; locNum < data->populations.size(); ++locNum) {
            totalNeurons += data->populations[locNum]->numNeurons;
        }
        int LoD = round(250.0f/float(totalNeurons)*pow(2,float(quality)));

        // draw with a level of detail dependant on the number on neurons we must draw
        // put some bounds on
        if (LoD < 4) LoD = 4; if (LoD > 32) LoD = 32;
        if (imageSaveMode)
            LoD = 64;

        //popColours.resize(data->populations.size());
        for(int locNum = 0; locNum < data->populations.size(); locNum++) {
            QSharedPointer <population> currPop = qSharedPointerDynamicCast <population> (data->populations[locNum]);
            CHECK_CAST(currPop);

            // find if the currPop is in the selectedList:
            int selInd = -1;
            for (int i = 0; i < this->selectedPops.size(); ++i) {
                if (this->selectedPops[i] == currPop) {
                    selInd = i;
                }
            }

            // add some neurons!

            // generate data:
            if (currPop->layoutType->locations.size() == 0) {
                QString errs;
                currPop->layoutType->generateLayout(currPop->numNeurons,&currPop->layoutType->locations,errs);
                // display all errors
                if (!errs.isEmpty()) {
                    //this->data->statusBarUpdate(errs,2000);
                }
            }

            if (currPop->dlIndex > 0) glDeleteLists(currPop->dlIndex,1);
            if (currPop->dlIndexCol > 0) glDeleteLists(currPop->dlIndexCol,1);

            // Start the dl to display info
            // create the index with the display lists
            currPop->dlIndex = glGenLists(1);

            // start the display list
            glNewList(currPop->dlIndex, GL_COMPILE);

            for (int i = 0; i < currPop->layoutType->locations.size(); ++i) {

                glPushMatrix();

                glTranslatef(currPop->layoutType->locations[i].x, currPop->layoutType->locations[i].y, currPop->layoutType->locations[i].z);

                // check we haven't broken stuff
                if (selInd > -1 && popColours[selInd].size() > currPop->layoutType->locations.size()) {

                    popColours[selInd].clear();
                    popLogs[selInd] = NULL;
                }

                if (selInd > -1 && popColours[selInd].size() > 0) {
                    this->drawNeuron(0.5, LoD, LoD, popColours[selInd][i]);
                } else if (!onlyColour) {
                    this->drawNeuron(0.5, LoD, LoD, QColor(100 + 0.5*currPop->colour.red(),100 + 0.5*currPop->colour.green(),100 + 0.5*currPop->colour.blue(),255));
                }

                glPopMatrix();
            }

            glEndList();

            //Start the dl to do collision
            // create the index with the display lists
            currPop->dlIndexCol = glGenLists(1);

            // start the display list
            glNewList(currPop->dlIndexCol, GL_COMPILE);

            for (int i = 0; i < currPop->layoutType->locations.size(); ++i) {

                glPushMatrix();

                glTranslatef(currPop->layoutType->locations[i].x, currPop->layoutType->locations[i].y, currPop->layoutType->locations[i].z);

                //ID code from
                //http://www.opengl-tutorial.org/miscellaneous/clicking-on-objects/picking-with-an-opengl-hack/#Giving_an_ID_to_every_object

                // Creating the IDs as colors.
                this->drawNeuron(0.5, LoD, LoD, QColor(locNum, i & 0x000000FF, (i & 0x0000FF00) >> 8));

                glPopMatrix();
            }

            glEndList();
        }
    }
}

/*!
 * \brief glConnectionWidget::createConnectionsDL
 * Murilo's work - create display lists for the Projections
 */
void glConnectionWidget::createConnectionsDL()
{
    //qDebug() << "Start creating the display lists for connections";

    // work out scaling for line widths:
    float lineScaleFactor;
    if (imageSaveMode) {
        float maxLen;
        maxLen = imageSaveHeight > imageSaveWidth ? imageSaveHeight : imageSaveWidth;
        lineScaleFactor = (1.0/1000.0*maxLen);
    } else
       lineScaleFactor = 1.0;


    for (int targNum = 0; targNum < this->selectedConns.size(); ++targNum) {

        // draw the connections:
        glDisable(GL_LIGHTING);
        glEnable(GL_DEPTH_TEST);

        QSharedPointer <population> src;
        QSharedPointer <population> dst;
        connection * conn;

        if (this->selectedConns[targNum]->type == synapseObject) {
            QSharedPointer <synapse> currTarg = qSharedPointerDynamicCast <synapse> (this->selectedConns[targNum]);
            CHECK_CAST(currTarg);
            conn = currTarg->connectionType;
            src = currTarg->proj->source;
            dst = currTarg->proj->destination;
        } else {
            QSharedPointer <genericInput> currIn = qSharedPointerDynamicCast <genericInput> (this->selectedConns[targNum]);
            CHECK_CAST(currIn);
            conn = currIn->connectionType;
            src = qSharedPointerDynamicCast <population> (currIn->source); // would not be here if this was not true
            dst = qSharedPointerDynamicCast <population> (currIn->destination);
            CHECK_CAST(src);
            CHECK_CAST(dst);
        }

        float srcX;
        float srcY;
        float srcZ;

        // big offsets for src
        if (src == selectedObject) {
            srcX = this->loc3Offset.x;
            srcY = this->loc3Offset.y;
            srcZ = this->loc3Offset.z;
        } else {
            srcX = src->loc3.x;
            srcY = src->loc3.y;
            srcZ = src->loc3.z;
        }


        float dstX;
        float dstY;
        float dstZ;

        // big offsets for dst
        if (dst == selectedObject) {
            dstX = this->loc3Offset.x;
            dstY = this->loc3Offset.y;
            dstZ = this->loc3Offset.z;
        } else {
            dstX = dst->loc3.x;
            dstY = dst->loc3.y;
            dstZ = dst->loc3.z;
        }

        // check we have the current version of the connectivity
        if (conn->type == CSV) {
            csv_connection * csv_conn = (csv_connection *) conn;
            if (csv_conn->generator) {
                if (((pythonscript_connection *) csv_conn->generator)->changed()) {
                    ((pythonscript_connection *) csv_conn->generator)->regenerateConnections();
                    // fetch connections back here:
                    connections[targNum].clear();
                    csv_conn->getAllData(connections[targNum]);
                }
            }
        }

        if (conn->type == CSV || conn->type == Kernel || conn->type == Python) {

            if (!src->isVisualised && !dst->isVisualised) {
                glEnable(GL_DEPTH_TEST);
                glEnable(GL_LIGHTING);
                continue;
            }

            connGenerationMutex->lock();

            int aux_strength = 0;
            //If aux_strength is zero, center is not used.
            GLfloat * center;
            bool colourScheme = false;

            // create the index with the display lists
            // start the display list
            if (this->selectedConns[targNum]->type == synapseObject) {
                QSharedPointer <synapse> currObj = qSharedPointerDynamicCast <synapse> (this->selectedConns[targNum]);
                CHECK_CAST(currObj);

                glDeleteLists(currObj->dlIndex,1);
                currObj->dlIndex = glGenLists(1);
                glNewList(currObj->dlIndex, GL_COMPILE);

                aux_strength = currObj->strength;
                center = currObj->center;
                colourScheme = currObj->colorScheme;

            } else {
                QSharedPointer <genericInput> currObj = qSharedPointerDynamicCast <genericInput> (this->selectedConns[targNum]);
                CHECK_CAST(currObj);

                glDeleteLists(currObj->dlIndex,1);
                currObj->dlIndex = glGenLists(1);
                glNewList(currObj->dlIndex, GL_COMPILE);

                aux_strength = currObj->strength;
                center = currObj->center;
                colourScheme = currObj->colorScheme;
            }

            float colourStep;
            float minColourValue = 0;
            float maxColourValue = 0;


            if (colourScheme) {
                if (connections[targNum].size() > 0) {
                    minColourValue = connections[targNum][0].metric;
                    maxColourValue = connections[targNum][0].metric;

                    for (int i = 1; i < connections[targNum].size(); ++i) {
                        if (minColourValue > connections[targNum][i].metric)
                            minColourValue = connections[targNum][i].metric;
                        if (maxColourValue < connections[targNum][i].metric)
                            maxColourValue = connections[targNum][i].metric;
                    }
                }
            }

            colourStep = 1/(maxColourValue-minColourValue);

            for (int i = 0; i < connections[targNum].size(); ++i) {

                if (connections[targNum][i].src < src->layoutType->locations.size() && connections[targNum][i].dst < dst->layoutType->locations.size()) {
                    glLineWidth(2.0 * lineScaleFactor);
                    if (colourScheme > 0)
                        glColor4f(0.0, (connections[targNum][i].metric) * colourStep, (connections[targNum][i].metric) * colourStep, 1);
                    else
                        glColor4f(0.0, 0.0, 0.0, 0.1);

                    // draw in

                    // Decide the control points
                    GLfloat ctrlpoints[aux_strength+2][3];
                    for (int strenghtIndex = 1; strenghtIndex <= aux_strength; strenghtIndex++) {
                        ctrlpoints[strenghtIndex][0] = center[0];
                        ctrlpoints[strenghtIndex][1] = center[1];
                        ctrlpoints[strenghtIndex][2] = center[2];
                    }

                    if (src->isVisualised && dst->isVisualised) {
                        ctrlpoints[0][0] = src->layoutType->locations[connections[targNum][i].src].x+srcX;
                        ctrlpoints[0][1] = src->layoutType->locations[connections[targNum][i].src].y+srcY;
                        ctrlpoints[0][2] = src->layoutType->locations[connections[targNum][i].src].z+srcZ;
                        ctrlpoints[aux_strength+1][0] = dst->layoutType->locations[connections[targNum][i].dst].x+dstX;
                        ctrlpoints[aux_strength+1][1] = dst->layoutType->locations[connections[targNum][i].dst].y+dstY;
                        ctrlpoints[aux_strength+1][2] = dst->layoutType->locations[connections[targNum][i].dst].z+dstZ;
                    }
                    if (src->isVisualised && !dst->isVisualised) {
                        ctrlpoints[0][0] = src->layoutType->locations[connections[targNum][i].src].x;
                        ctrlpoints[0][1] = src->layoutType->locations[connections[targNum][i].src].y;
                        ctrlpoints[0][2] = src->layoutType->locations[connections[targNum][i].src].z;
                        ctrlpoints[aux_strength+1][0] = dstX;
                        ctrlpoints[aux_strength+1][1] = dstY;
                        ctrlpoints[aux_strength+1][2] = dstZ;
                    }
                    if (!src->isVisualised && dst->isVisualised) {
                        ctrlpoints[0][0] = src->loc3.x;
                        ctrlpoints[0][1] = src->loc3.y;
                        ctrlpoints[0][2] = src->loc3.z;
                        ctrlpoints[aux_strength+1][0] = dst->layoutType->locations[connections[targNum][i].dst].x;
                        ctrlpoints[aux_strength+1][1] = dst->layoutType->locations[connections[targNum][i].dst].y;
                        ctrlpoints[aux_strength+1][2] = dst->layoutType->locations[connections[targNum][i].dst].z;
                    }


                    glMap1f(GL_MAP1_VERTEX_3, 0.0, 1.0, 3, aux_strength+2, &ctrlpoints[0][0]);


                    // Draw the line between the neurons
                    glBegin(GL_LINE_STRIP);


                    for (int j = 0; j <= 30; j++)
                        glEvalCoord1f((GLfloat) j/30.0);

                    glEnd();

                } else {
                    // ERR - CONNECTION INDEX OUT OF RANGE
                }


            }

            glEndList();
            connGenerationMutex->unlock();


        }
    }

    //qDebug() << "Finish creating the display lists for connections";
}




#if QT_VERSION > QT_VERSION_CHECK(5, 0, 0)
QImage glConnectionWidget:: renderQImage(int w, int h)
{
    // Set the rendering engine to the size of the image to render
    // Also set the format so that the depth buffer will work
    QOpenGLFramebufferObjectFormat format;
    format.setAttachment(QOpenGLFramebufferObject::Depth);
    QOpenGLFramebufferObject qfb(w,h,format);
    qfb.bind();
    // If the frame buffer does not work then return an empty image
    if(!qfb.isValid()) return(QImage());
    resizeGL(w,h);
    // Draw the scene to the buffer
    glEnable(GL_MULTISAMPLE);
    this->repaint();
    qfb.release();
    resizeGL(width(),height());
    return(qfb.toImage());
}
#endif

QPixmap glConnectionWidget::renderImage(int width, int height) {

    QPixmap pix;
    imageSaveHeight = height;
    imageSaveWidth = width;
    imageSaveMode = true;

// renderPixmap is broken in Qt > 5.0 - this fix doesn't currently work correctly, but is better than nothing
#if QT_VERSION > QT_VERSION_CHECK(5, 0, 0)
    QImage img = renderQImage(width, height);

    if (img.isNull()) {
        qDebug() << "renderQimage returned a dud";
    }

    pix = QPixmap::fromImage(img);
#else

   pix = this->renderPixmap(width, height);
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

    imageSaveMode = false;
    //QImage image = pix.toImage();
    return pix;

}
