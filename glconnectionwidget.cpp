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
    selectedObject = NULL;
    setAutoFillBackground(false);
    popIndicesShown = false;
    selectedIndex = 0;
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

    glEnable(GL_MULTISAMPLE);


}

void glConnectionWidget::toggleOrthoView(bool toggle) {
    orthoView = toggle;
    this->repaint();
}

void glConnectionWidget::clear() {

    selectedObject = NULL;
    selectedPops.clear();
    popColours.clear();
    popLogs.clear();
    selectedConns.clear();
    connections.clear();
    selectedIndex = 0;
    selectedType = 1;
    model = (QAbstractTableModel *)0;

}

void glConnectionWidget::addLogs(QVector < logData * > * logs) {

    // for each population
    for (uint i = 0; i < selectedPops.size(); ++i) {

        population * pop = selectedPops[i];

        // for each analog output port
        for (uint j = 0; j < pop->neuronType->component->AnalogPortList.size(); ++j) {

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

}

void glConnectionWidget::updateLogDataTime(int index) {

    newLogTime = index;

}

void glConnectionWidget::updateLogData() {

    if (newLogTime == currentLogTime)
        return;

    currentLogTime = newLogTime;

    // fetch data from logs
    for (uint i = 0; i < popLogs.size(); ++i) {

        // skip where there is no log
        if (popLogs[i] == NULL)
            continue;

        // get a row
        vector < double > logValues = popLogs[i]->getRow(currentLogTime);

        // data not usable
        if (logValues.size() == 0)
            continue;
        if ((int) logValues.size() > selectedPops[i]->numNeurons)
            continue;

        // resize container
        QColor col(0,0,0,255);
        popColours[i].resize(selectedPops[i]->numNeurons, col);

        // remap data
        for (uint j = 0; j < logValues.size(); ++j) {
            if (logValues[j] < Q_INFINITY && (popLogs[i]->getMax()-popLogs[i]->getMin()) != 0) {
                int val = ((logValues[j]-popLogs[i]->getMin())*255.0)/(popLogs[i]->getMax()-popLogs[i]->getMin());
                val *= 3;
                // complete the remap in just 4 ternarys 
                int val3 = val > 511 ? val-512 : 0;
                int val2 = val3 > 0 ? 255 : val;
                val2 = val2 > 255 ? val2 - 255 : 0;
                int val1 = val2 > 0 ? 255 : val;
                popColours[i][j] = QColor(val1,val2, val3, 255);
            }
        }
    }

    // redraw!
    this->repaint();

}


void glConnectionWidget::resizeGL(int, int)
{

    // setup the view
    this->repaint();


}

void glConnectionWidget::redraw() {
    // refetch layout of current selection
    if (selectedObject != NULL) {
        if (selectedObject->type == populationObject) {
            QString errs;
            population * currPop = ((population *) selectedObject);
            currPop->layoutType->locations.clear();
            currPop->layoutType->generateLayout(currPop->numNeurons,&currPop->layoutType->locations,errs);
        }
    }
    this->repaint();
}

void glConnectionWidget::redraw(int)
{

    // we haven't updated the underlying data yet - but we want to show spinbox changes
    // get spinbox ptrs:
    QSpinBox * xSpin = (QSpinBox *) sender()->property("xptr").value<void *>();
    QSpinBox * ySpin = (QSpinBox *) sender()->property("yptr").value<void *>();
    QSpinBox * zSpin = (QSpinBox *) sender()->property("zptr").value<void *>();

    loc3Offset.x = xSpin->value();
    loc3Offset.y = ySpin->value();
    loc3Offset.z = zSpin->value();

    this->repaint();

}

void glConnectionWidget::allowRepaint() {
    this->repaintAllowed = true;
}

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

    // don't try and repaint a hidden widget!
    if (!this->isVisible())
        return;

    // get rid of old stuff
    if (imageSaveMode) {
        QColor qtCol = QColor::fromRgbF(1.0,1.0,1.0,0.0);
        qglClearColor(qtCol);
    } else {
        QColor qtCol = QColor::fromCmykF(0.5, 0.5, 0.5, 0.0);
        qglClearColor(qtCol.light());
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

    glPushMatrix();
    glTranslatef(0,0,-5.0);

    // if previewing a layout then override normal drawing
    if (locations.size() > 0) {
        for (unsigned int i = 0; i < locations[0].size(); ++i) {
            glPushMatrix();

            glTranslatef(locations[0][i].x, locations[0][i].y, locations[0][i].z);

            // draw with a level of detail dependant on the number on neurons we must draw
            int LoD = round(250.0f/float(locations[0].size())*pow(2,float(quality)));
            // put some bounds on
            if (LoD < 4) LoD = 4; if (LoD > 32) LoD = 32;
            this->drawNeuron(0.5, LoD, LoD, QColor(100,100,100,255));

            glPopMatrix();
        }

        glPopMatrix();
        // need this as no painter!
        swapBuffers();
        return;
    }

    // draw with a level of detail dependant on the number on neurons we must draw
    // sum neurons across all pops we'll draw
    int totalNeurons = 0;
    for (uint locNum = 0; locNum < selectedPops.size(); ++locNum) {
        totalNeurons += selectedPops[locNum]->layoutType->locations.size();
    }
    int LoD = round(250.0f/float(totalNeurons)*pow(2,float(quality)));

    // normal drawing
    for (uint locNum = 0; locNum < selectedPops.size(); ++locNum) {
        population * currPop = selectedPops[locNum];
        for (unsigned int i = 0; i < currPop->layoutType->locations.size(); ++i) {
            glPushMatrix();

            glTranslatef(currPop->layoutType->locations[i].x, currPop->layoutType->locations[i].y, currPop->layoutType->locations[i].z);

            // if currently selected
            if (currPop == selectedObject) {
                // move to pop location denoted by the spinboxes for x, y, z
                glTranslatef(loc3Offset.x, loc3Offset.y,loc3Offset.z);
            } else {
                glTranslatef(currPop->loc3.x, currPop->loc3.y,currPop->loc3.z);
            }

            // draw with a level of detail dependant on the number on neurons we must draw
            // put some bounds on
            if (LoD < 4) LoD = 4; if (LoD > 32) LoD = 32;
            if (imageSaveMode)
                LoD = 64;

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
    for (uint targNum = 0; targNum < this->selectedConns.size(); ++targNum) {

        // draw the connections:
        glDisable(GL_LIGHTING);
        glEnable(GL_DEPTH_TEST);

        //qDebug() << "Doing conns!";

        //Synapse * currTarg = selectedConns[targNum];

        population * src;
        population * dst;
        connection * conn;

        if (selectedConns[targNum]->type == synapseObject) {
            synapse * currTarg = (synapse *) selectedConns[targNum];
            conn = currTarg->connectionType;
            src = currTarg->proj->source;
            dst = currTarg->proj->destination;
        } else {
            genericInput * currIn = (genericInput *) selectedConns[targNum];
            conn = currIn->connectionType;
            src = (population *) currIn->source; // would not be here if this was not true
            dst = (population *) currIn->destination;
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

        if (conn->type == CSV || conn->type == DistanceBased || conn->type == Kernel || conn->type == Python) {

            if (!src->isVisualised && !dst->isVisualised) {
                glEnable(GL_DEPTH_TEST);
                glEnable(GL_LIGHTING);
                continue;
            }

            connGenerationMutex->lock();
            for (uint i = 0; i < connections[targNum].size(); ++i) {

                if (connections[targNum][i].src < src->layoutType->locations.size() && connections[targNum][i].dst < dst->layoutType->locations.size()) {
                    glLineWidth(1.0*lineScaleFactor);

                    glColor4f(0.0, 0.0, 0.0, 0.1);

                    // draw in
                    glBegin(GL_LINES);
                    if (src->isVisualised && dst->isVisualised) {
                        glVertex3f(src->layoutType->locations[connections[targNum][i].src].x+srcX, src->layoutType->locations[connections[targNum][i].src].y+srcY, src->layoutType->locations[connections[targNum][i].src].z+srcZ);
                        glVertex3f(dst->layoutType->locations[connections[targNum][i].dst].x+dstX, dst->layoutType->locations[connections[targNum][i].dst].y+dstY, dst->layoutType->locations[connections[targNum][i].dst].z+dstZ);
                    }
                    if (src->isVisualised && !dst->isVisualised) {
                        glVertex3f(src->layoutType->locations[connections[targNum][i].src].x, src->layoutType->locations[connections[targNum][i].src].y, src->layoutType->locations[connections[targNum][i].src].z);
                        glVertex3f(dstX, dstY, dstZ);
                    }
                    if (!src->isVisualised && dst->isVisualised) {
                        glVertex3f(src->loc3.x, src->loc3.y, src->loc3.z);
                        glVertex3f(dst->layoutType->locations[connections[targNum][i].dst].x, dst->layoutType->locations[connections[targNum][i].dst].y, dst->layoutType->locations[connections[targNum][i].dst].z);
                    }
                    glEnd();



                } else {
                    // ERR - CONNECTION INDEX OUT OF RANGE
                }

            }

            // draw selected connections on top
            glDisable(GL_DEPTH_TEST);
            if (selectedConns[targNum] == selectedObject) {
                for (uint i = 0; i < connections[targNum].size(); ++i) {

                    if (connections[targNum][i].src < src->layoutType->locations.size() && connections[targNum][i].dst < dst->layoutType->locations.size()) {
                        glLineWidth(1.0*lineScaleFactor);

                        glColor4f(0.0, 0.0, 0.0, 0.1);

                        // find if selected
                        bool isSelected = false;

                        for (uint j = 0; j < (uint) selection.count(); ++j) {
                            if (i == (uint) selection[j].row())
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

                        if (((int) connections[targNum][i].src == selectedIndex && selectedType == 1) \
                                || ((int) connections[targNum][i].dst == selectedIndex && selectedType == 2))
                        {
                            glLineWidth(1.5*lineScaleFactor);
                            glColor4f(0.0, 1.0, 0.0, 1.0);
                            isSelected = true;
                        }

                        if (isSelected) {
                            // draw in
                            glBegin(GL_LINES);
                            if (src->isVisualised && dst->isVisualised) {
                                glVertex3f(src->layoutType->locations[connections[targNum][i].src].x+srcX, src->layoutType->locations[connections[targNum][i].src].y+srcY, src->layoutType->locations[connections[targNum][i].src].z+srcZ);
                                glVertex3f(dst->layoutType->locations[connections[targNum][i].dst].x+dstX, dst->layoutType->locations[connections[targNum][i].dst].y+dstY, dst->layoutType->locations[connections[targNum][i].dst].z+dstZ);
                            }
                            if (src->isVisualised && !dst->isVisualised) {
                                glVertex3f(src->layoutType->locations[connections[targNum][i].src].x, src->layoutType->locations[connections[targNum][i].src].y, src->layoutType->locations[connections[targNum][i].src].z);
                                glVertex3f(dstX, dstY, dstZ);
                            }
                            if (!src->isVisualised && dst->isVisualised) {
                                glVertex3f(src->loc3.x, src->loc3.y, src->loc3.z);
                                glVertex3f(dst->layoutType->locations[connections[targNum][i].dst].x, dst->layoutType->locations[connections[targNum][i].dst].y, dst->layoutType->locations[connections[targNum][i].dst].z);
                            }
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

                    for (uint i = 0; i < src->layoutType->locations.size(); ++i) {

                        glLineWidth(1.5*lineScaleFactor);
                        glColor4f(0.0, 0.0, 1.0, 0.8);

                         // draw in
                        glBegin(GL_LINES);
                        glVertex3f(src->layoutType->locations[i].x+srcX, src->layoutType->locations[i].y+srcY, src->layoutType->locations[i].z+srcZ);
                        glVertex3f(dst->layoutType->locations[i].x+dstX, dst->layoutType->locations[i].y+dstY, dst->layoutType->locations[i].z+dstZ);
                        glEnd();

                    }
                }

                if (src->layoutType->locations.size() > 0 && dst->layoutType->locations.size() == 0) {

                    for (uint i = 0; i < src->layoutType->locations.size(); ++i) {

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

                    for (uint i = 0; i < dst->layoutType->locations.size(); ++i) {

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

                for (uint i = 0; i < src->layoutType->locations.size(); ++i) {
                    for (uint j = 0; j <  dst->layoutType->locations.size(); ++j) {

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

                for (uint i = 0; i < src->layoutType->locations.size(); ++i) {

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

                 for (uint j = 0; j <  dst->layoutType->locations.size(); ++j) {

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

            random.setSeed(((fixedProb_connection *) conn)->seed);

            prob = ((fixedProb_connection *) conn)->p;

            // generate a list of projections to highlight
            vector < loc > redrawLocs;

            for (uint i = 0; i < src->layoutType->locations.size(); ++i) {
                for (uint j = 0; j <  dst->layoutType->locations.size(); ++j) {
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

            for (uint i=0; i < redrawLocs.size(); i+=2) {

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
        for (uint locNum = 0; locNum < selectedPops.size(); ++locNum) {
            population * currPop = selectedPops[locNum];
            for (unsigned int i = 0; i < currPop->layoutType->locations.size(); ++i) {
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
        swapBuffers();
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

        for (uint locNum = 0; locNum < locations[0].size(); ++locNum) {
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
            for (uint i = 0; i < selectedPops.size(); ++i) {
                if (selectedObject == selectedPops[i])
                    selIndex = i;
            }
        }

        population * currPop = selectedPops[selIndex];

        // work out extent of view:
        // find max and min vals in each direction:
        loc maxes;
        loc mins;

        maxes.x = -INFINITY; maxes.y = -INFINITY; maxes.z = -INFINITY;
        mins.x = INFINITY; mins.y = INFINITY; mins.z = INFINITY;

        for (uint locNum = 0; locNum < currPop->layoutType->locations.size(); ++locNum) {
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
    selectedIndex = 0;

    // cancel current selection
    selectedObject = NULL;

    // look up the selected item
    QModelIndexList indices = top.indexes();
    TreeItem *item = static_cast<TreeItem*>(indices[0].internalPointer());

    for (uint i = 0; i < data->populations.size(); ++i) {

        population * currPop = (population *) data->populations[i];

        // populations
        if (currPop->getName() == item->name) {
            this->selectedObject = currPop;
            // reset currPop offset
            this->loc3Offset.x = currPop->loc3.x;
            this->loc3Offset.y = currPop->loc3.y;
            this->loc3Offset.z = currPop->loc3.z;
        }

        // population inputs
        for (uint output = 0; output < data->populations[i]->neuronType->outputs.size(); ++output) {

            genericInput * currOutput = data->populations[i]->neuronType->outputs[output];

            // add Synapse
            if (!currOutput->projInput)
            {
                if("Output from " + currOutput->source->getName() + " to " + currOutput->destination->getName() + " port " + currOutput->dstPort + " " + QString::number(output) == item->name) {
                    selectedObject = currOutput;
                }
            }

        }

        // projections
        for (uint j = 0; j < currPop->projections.size(); ++j) {

            projection * currProj = (projection *) currPop->projections[j];

            if (currProj->getName() == item->name)
                selectedObject = currProj;

            // synapses
            for (uint k = 0; k < currProj->synapses.size(); ++k) {

                synapse * currTarg = (synapse *) currProj->synapses[k];

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
        population * currPop = ((population *) selectedObject);
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
    for (uint i = 0; i < selectedPops.size(); ++i) {
        if (selectedObject == selectedPops[i]) {

            population * currPop = (population *) selectedObject;
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
    for (uint i = 0; i < selectedPops.size(); ++i) {
        if (selectedObject == selectedPops[i]) {

            population * currPop = (population *) selectedObject;
            QString errs;
            currPop->layoutType->locations.clear();
            currPop->layoutType->generateLayout(currPop->numNeurons,&currPop->layoutType->locations,errs);
            // display all errors
            if (!errs.isEmpty()) {
                //this->data->statusBarUpdate(errs,2000);
            }
        }
    }
    // redraw
    this->repaint();

}

void glConnectionWidget::parsChangedPopulation() {


    // if the selected population is in the list (i.e. checked) then refetch locations
    for (uint i = 0; i < selectedPops.size(); ++i) {
        if (selectedObject == selectedPops[i]) {

            population * currPop = (population *) selectedObject;
            QString errs;
            currPop->layoutType->locations.clear();
            currPop->layoutType->generateLayout(currPop->numNeurons,&currPop->layoutType->locations,errs);
            // display all errors
            if (!errs.isEmpty()) {
                //this->data->statusBarUpdate(errs,2000);
            }
        }
    }

    // redraw
    this->repaint();

}

void glConnectionWidget::parsChangedProjections() {

    this->refreshAll();

    for (uint i = 0; i < selectedConns.size(); ++i) {

        connection * conn;

        if (selectedConns[i]->type == synapseObject) {
            synapse * currTarg = (synapse *) selectedConns[i];
            conn = currTarg->connectionType;
        } else {
            genericInput * currIn = (genericInput *) selectedConns[i];
            conn = currIn->connectionType;
        }

        // regrab data for distance based
        if (conn->type == DistanceBased) {

            // refresh the connections
            if (((distanceBased_connection *) conn)->changed()) {
                connections[i].clear();
                // launch version increment dialog box:
                generate_dialog generate(((distanceBased_connection *) conn), ((distanceBased_connection *) conn)->src, ((distanceBased_connection *) conn)->dst, connections[i], connGenerationMutex, this);
                bool retVal = generate.exec();
                if (!retVal) {
                    return;
                }
                ((distanceBased_connection *) conn)->connections = connections[i];
                ((distanceBased_connection *) conn)->setUnchanged(true);
            }

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
            }
        }


    }


    // redraw:
    repaint();

}

void glConnectionWidget::parsChangedProjection() {

    if (!data->isValidPointer(selectedObject))
        return;

    // can only be the current selection
    if (selectedObject->type == synapseObject || selectedObject->type == inputObject) {

        connection * conn;
        population * src;
        population * dst;

        if (selectedObject->type == synapseObject) {
            synapse * currTarg = (synapse *) selectedObject;
            conn = currTarg->connectionType;
            src = currTarg->proj->source;
            dst = currTarg->proj->destination;
        } else {
            genericInput * currIn = (genericInput *) selectedObject;
            conn = currIn->connectionType;
            src = (population *) currIn->source; // would not be here if was not true
            dst = (population *) currIn->destination;
        }

        // regrab data for distance based
        if (conn->type == DistanceBased) {

            QString newEquation;

            QLineEdit * senderPtr = NULL;

            if (sender()) {
                newEquation = ((QLineEdit *) sender())->text();
                senderPtr = (QLineEdit *) sender();
                sender()->disconnect(this);
            }

            // find selected object
            for (uint i = 0; i < this->selectedConns.size(); ++i) {

                if (selectedObject == selectedConns[i]) {

                    // refresh the connections
                    if (((distanceBased_connection *) conn)->changed(newEquation)) {
                        connections[i].clear();
                        // launch version increment dialog box:
                        generate_dialog generate(((distanceBased_connection *) conn), src, dst, connections[i], connGenerationMutex, this);
                        bool retVal = generate.exec();
                        if (!retVal) {
                            qDebug() << "glConnectionWidget::parsChangedProjection: generate.exec() returned 0, but continuing in this case...";
                        }
                        ((distanceBased_connection *) conn)->connections = connections[i];
                        ((distanceBased_connection *) conn)->setUnchanged(true);
                    }
                }
            }

            // reconnect
            if (senderPtr) {
                connect(senderPtr, SIGNAL(editingFinished()), this, SLOT (parsChangedProjection()));
            }

        }
        // regrab data for kernel based
        if (conn->type == Kernel) {

            // update the projection:


            // find selected object
            for (uint i = 0; i < this->selectedConns.size(); ++i) {

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
                    }
                }

            }

        }
        // regrab data for python script based
        if (conn->type == Python) {

            // update the projection:


            // find selected object
            for (uint i = 0; i < this->selectedConns.size(); ++i) {

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


        // redraw:
        repaint();

    }

}

// after switching views, check we haven't broken anything!
void glConnectionWidget::refreshAll() {

    // check on populations
    for (uint i = 0; i < selectedPops.size(); ++i) {

        // check pointer is valid
        if (!data->isValidPointer(selectedPops[i])) {
            // remove
            selectedPops.erase(selectedPops.begin()+i);
            popLogs.erase(popLogs.begin()+i);
            popColours.erase(popColours.begin()+i);
            --i;
            continue;
        }

        population * currPop = selectedPops[i];

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
    for (uint i = 0; i < selectedConns.size(); ++i) {

        // check pointer is valid
        if (!data->isValidPointer(selectedConns[i])) {
            // remove
            selectedConns.erase(selectedConns.begin()+i);
            connections.erase(connections.begin()+i);
            --i;
            continue;
        }

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


void glConnectionWidget::setConnType(connectionType cType) {

    this->currProjectionType = cType;

}

void glConnectionWidget::drawLocations(vector <loc> locs) {

    // redraw based on a set of location passed in (used for layout previews)
    for (uint i = 0; i < locations.size(); ++i) {
        locations[i].clear();
    }
    locations.clear();

    this->locations.push_back(locs);

    // redraw
    this->repaint();

}

void glConnectionWidget::clearLocations() {

    // clear the set of location passed in (used for layout previews)
    for (uint i = 0; i < locations.size(); ++i) {
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

            synapse * currTarg = (synapse *) selectedObject;

            for (uint i = 0; i < this->selectedConns.size(); ++i) {

                if (selectedObject == selectedConns[i] && currTarg->connectionType->type == CSV) {

                    // refresh the connections
                    connections[i].clear();
                    ((csv_connection *) currTarg->connectionType)->getAllData(connections[i]);
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

    for (uint i = 0; i < data->populations.size(); ++i) {

        population * currPop = (population *) data->populations[i];

        // populations
        if (currPop->isVisualised) {
            // if not in list then add to list
            bool inList = false;
            for (uint p = 0; p < this->selectedPops.size(); ++p) {
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
            for (uint p = 0; p < this->selectedPops.size(); ++p) {
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
        for (uint j = 0; j < currPop->neuronType->inputs.size(); ++j) {
            genericInput * currIn = currPop->neuronType->inputs[j];

            // only for pop -> pop inputs
            if (!currIn->projInput) {
                if (currIn->source->type == populationObject && currIn->destination->type == populationObject) {
                    if (currIn->isVisualised) {
                        // add to list if not in list
                        bool inList = false;
                        for (uint p = 0; p < this->selectedConns.size(); ++p) {
                            if (selectedConns[p] == currIn)
                                inList = true;
                        }
                        if (!inList) {
                            selectedConns.push_back(currIn);
                            connections.resize(connections.size()+1);

                            if (currIn->connectionType->type == CSV) {
                                // load in the connections
                                ((csv_connection *) currIn->connectionType)->getAllData(connections.back());
                            } else if (currIn->connectionType->type == Kernel) {
                                if (((kernel_connection *) currIn->connectionType)->connections.size() > 0 && !((kernel_connection *) currIn->connectionType)->changed()) {
                                    connections.back() = ((kernel_connection *) currIn->connectionType)->connections;
                                } else {
                                    // generate
                                    // launch version increment dialog box:
                                    generate_dialog generate(((kernel_connection *) currIn->connectionType), (population *) currIn->source, (population *) currIn->destination, connections.back(), connGenerationMutex, this);
                                    bool retVal = generate.exec();
                                    if (!retVal) {
                                        return;
                                    }
                                    ((kernel_connection *) currIn->connectionType)->connections = connections.back();
                                    ((kernel_connection *) currIn->connectionType)->setUnchanged(true);
                                }
                            } else if (currIn->connectionType->type == DistanceBased) {
                                if (((distanceBased_connection *) currIn->connectionType)->connections.size() > 0 && !((distanceBased_connection *) currIn->connectionType)->changed()) {
                                    connections.back() = ((distanceBased_connection *) currIn->connectionType)->connections;
                                } else {
                                    // generate
                                    // launch version increment dialog box:
                                    generate_dialog generate(((distanceBased_connection *) currIn->connectionType), (population *) currIn->source, (population *) currIn->destination, connections.back(), connGenerationMutex, this);
                                    bool retVal = generate.exec();
                                    if (!retVal) {
                                        return;
                                    }
                                    ((distanceBased_connection *) currIn->connectionType)->connections = connections.back();
                                    ((distanceBased_connection *) currIn->connectionType)->setUnchanged(true);
                                }
                            } else if (currIn->connectionType->type == Python) {
                                if (((pythonscript_connection *) currIn->connectionType)->connections.size() > 0 && !((pythonscript_connection *) currIn->connectionType)->changed()) {
                                    connections.back() = ((pythonscript_connection *) currIn->connectionType)->connections;
                                } else {
                                    // generate
                                    // launch version increment dialog box:
                                    generate_dialog generate(((pythonscript_connection *) currIn->connectionType), (population *) currIn->source, (population *) currIn->destination, connections.back(), connGenerationMutex, this);
                                    bool retVal = generate.exec();
                                    if (!retVal) {
                                        return;
                                    }
                                    ((pythonscript_connection *) currIn->connectionType)->connections = connections.back();
                                    ((pythonscript_connection *) currIn->connectionType)->setUnchanged(true);
                                }
                            }

                        }

                    } else {
                        // remove from list if there
                        for (uint p = 0; p < this->selectedConns.size(); ++p) {
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
        for (uint j = 0; j < currPop->projections.size(); ++j) {

            projection * currProj = (projection *) currPop->projections[j];

            // synapses
            for (uint k = 0; k < currProj->synapses.size(); ++k) {

                synapse * currTarg = (synapse *) currProj->synapses[k];

                if (currTarg->isVisualised) {
                    // if not in list then add to list
                    bool inList = false;
                    for (uint p = 0; p < this->selectedConns.size(); ++p) {
                        if (selectedConns[p] == currTarg)
                            inList = true;
                    }
                    if (!inList) {
                        selectedConns.push_back(currTarg);
                        connections.resize(connections.size()+1);

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
                        } else if (currTarg->connectionType->type == DistanceBased) {
                            if (((distanceBased_connection *) currTarg->connectionType)->connections.size() > 0 && !((distanceBased_connection *) currTarg->connectionType)->changed()) {
                                connections.back() = ((distanceBased_connection *) currTarg->connectionType)->connections;
                            } else {
                                // generate
                                // launch version increment dialog box:
                                generate_dialog generate(((distanceBased_connection *) currTarg->connectionType), currTarg->proj->source, currTarg->proj->destination, connections.back(), connGenerationMutex, this);
                                bool retVal = generate.exec();
                                if (!retVal) {
                                    return;
                                }
                                ((distanceBased_connection *) currTarg->connectionType)->connections = connections.back();
                                ((distanceBased_connection *) currTarg->connectionType)->setUnchanged(true);
                            }
                        } else if (currTarg->connectionType->type == Python) {
                            if (((pythonscript_connection *) currTarg->connectionType)->connections.size() > 0 && !((pythonscript_connection *) currTarg->connectionType)->changed()) {
                                connections.back() = ((distanceBased_connection *) currTarg->connectionType)->connections;
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
                    for (uint p = 0; p < this->selectedConns.size(); ++p) {
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
    addLogs(&data->main->viewGV.properties->logs);

    // force redraw!
    this->repaint();

}

void glConnectionWidget::connectionDataChanged(QModelIndex, QModelIndex) {

    // refetch the connections:
    getConnections();

}

void glConnectionWidget::connectionSelectionChanged(QItemSelection, QItemSelection) {

    this->selectedIndex = 0;
    this->selectedType = 1;

    this->selection = ((QItemSelectionModel *) sender())->selectedIndexes();
    // force redraw
    repaint();

}

void glConnectionWidget::mousePressEvent(QMouseEvent *event){
    setCursor(Qt::ClosedHandCursor);
    button = event->button();
    origPos = event->globalPos();
    origPos.setX(origPos.x() - pos.x()*100/zoomFactor);
    origPos.setY(origPos.y() + pos.y()*100/zoomFactor);
    origRot = event->globalPos();
    origRot.setX(origRot.x() - rot.x()*2);
    origRot.setY(origRot.y() - rot.y()*2);

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

#if QT_VERSION > QT_VERSION_CHECK(5, 0, 0)
QImage glConnectionWidget:: renderQImage(int w, int h)
{
    // Set the rendering engine to the size of the image to render
    // Also set the format so that the depth buffer will work
    QOpenGLFramebufferObjectFormat format;
    //format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
    QOpenGLFramebufferObject qfb(w,h,format);
    qfb.bind();
    // If the frame buffer does not work then return an empty image
    if(!qfb.isValid()) return(QImage());
    resizeGL(w,h);
    // Draw the scene to the buffer
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
        for (uint locNum = 0; locNum < selectedPops.size(); ++locNum) {
            population * currPop = selectedPops[locNum];
            for (unsigned int i = 0; i < currPop->layoutType->locations.size(); ++i) {
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
