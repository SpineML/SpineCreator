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

#include "rootdata.h"
#include "connectionlistdialog.h"
#include "experiment.h"
#include "undocommands.h"
#include "savenetworkimage_dialog.h"
#include "mainwindow.h"
#include "projectobject.h"
#include "systemmodel.h"
#include "nineml_rootcomponentitem.h"

/*
 Alex Cope 2012
 The rootData class stores the 9ML description of the system -
 what populations there are, what projections, and the parameters of all these.
 It is a central target for all changes to the system, and any changes made will
 propagate through here.
*/


rootData::rootData(QObject *parent) :
    QObject(parent)
{
    QSettings settings;
    //QString localName = settings.value("versioning/localName", QHostInfo::localHostName()).toString();
    //version.setVersion(0,0,1,localName);

    this->cursor.x = 0;
    this->cursor.y = 0;
    this->largestIndex = 0;
    this->catalogUnsorted.push_back(QSharedPointer<NineMLComponent>(new NineMLComponent()));
    this->catalogUnsorted[0]->name = "none";
    this->catalogUnsorted[0]->type = "moo";
    this->catalogNrn.push_back(QSharedPointer<NineMLComponent>(new NineMLComponent()));
    this->catalogNrn[0]->name = "none";
    this->catalogNrn[0]->type = "neuron_body";
    this->catalogWU.push_back(QSharedPointer<NineMLComponent>(new NineMLComponent()));
    this->catalogWU[0]->name = "none";
    this->catalogWU[0]->type = "weight_update";
    this->catalogPS.push_back(QSharedPointer<NineMLComponent>(new NineMLComponent()));
    this->catalogPS[0]->name = "none";
    this->catalogPS[0]->type = "postsynapse";
    this->catalogLayout.push_back(QSharedPointer<NineMLLayout>(new NineMLLayout()));
    this->catalogLayout[0]->name = "none";
    this->selectionMoved = false;

    this->selChange = false;

    if (!popImage.load( ":/icons/objects/icons/nrn.png" )) std::cerr << "warn" << endl;

    // update version and name
    setCaption("");

    clipboardCData.clear();
    projectActions = NULL;

    catalogConn.push_back("1");
    catalogConn.push_back("2");
}

void rootData::redrawViews()
{
    // redraw the network
    reDrawAll();
    redrawGLview();

    // redraw the component file list
    main->viewCL.fileList->disconnect();
    main->addComponentsToFileList();
    connect(main->viewCL.fileList, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), main, SLOT(fileListItemChanged(QListWidgetItem*,QListWidgetItem*)));
    // clear edited component
    if (main->viewCL.root != NULL) {
        delete main->viewCL.root;
        main->viewCL.root = NULL;
    }

    // redraw experiments
    main->viewELhandler->redraw();

    if ( main->viewVZ.OpenGLWidget != NULL) {
        // redraw viz
        main->viewVZ.OpenGLWidget->refreshAll();
        // clear away old stuff
        main->viewVZ.currObject = (QSharedPointer<systemObject>)0;
        main->viewVZhandler->clearAll();
        main->viewVZ.OpenGLWidget->clear();
        // configure TreeView
        if (!(main->viewVZ.sysModel == NULL)) {
            delete main->viewVZ.sysModel;
        }
        main->viewVZ.sysModel = new systemmodel(this);
        main->viewVZ.treeView->setModel(main->viewVZ.sysModel);
        // connect for function
        connect(main->viewVZ.treeView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), main->viewVZhandler, SLOT(selectionChanged(QItemSelection,QItemSelection)));
        connect(main->viewVZ.treeView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), main->viewVZ.OpenGLWidget, SLOT(selectionChanged(QItemSelection,QItemSelection)));
        connect(main->viewVZ.sysModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), main->viewVZ.OpenGLWidget, SLOT(sysSelectionChanged(QModelIndex,QModelIndex)));
        main->viewVZhandler->redrawHeaders();
        main->viewVZ.OpenGLWidget->sysSelectionChanged(QModelIndex(), QModelIndex());
    }

}

void rootData::selectProject(QAction * action)
{
    // deselect the current project
    currProject->deselect_project(this);
    // select the new project
    projects[action->property("number").toInt()]->select_project(this);
    redrawViews();
    main->updateTitle();
}

void rootData::replaceComponent(QSharedPointer<NineMLComponent> oldComp, QSharedPointer<NineMLComponent> newComp)
{
    for (int p = 0; p < this->populations.size(); ++p) {

        QSharedPointer <population> pop = this->populations[p];

        // replace references
        if (pop->neuronType->component == oldComp) {
            // has the type changed?
            if (newComp->type != "neuron_body") {
                pop->neuronType->migrateComponent(catalogNrn[0]);
                for (int i = 0; i < experiments.size(); ++i) {
                    experiment * currExpt = experiments[i];
                    currExpt->updateChanges(pop->neuronType);
                }
            } else {
                pop->neuronType->migrateComponent(newComp);
                for (int i = 0; i < experiments.size(); ++i) {
                    experiment * currExpt = experiments[i];
                    currExpt->updateChanges(pop->neuronType);
                }
            }
        }

        for (int pr = 0; pr < pop->projections.size(); ++pr) {

            QSharedPointer <projection> proj = pop->projections[pr];

            for (int sy = 0; sy < proj->synapses.size(); ++sy) {

                QSharedPointer <synapse> syn = proj->synapses[sy];

                // replace references
                if (syn->weightUpdateType->component == oldComp) {
                    // has the type changed?
                    if (newComp->type != "weight_update") {
                        syn->weightUpdateType->migrateComponent(catalogWU[0]);
                        for (int i = 0; i < experiments.size(); ++i) {
                            experiment * currExpt = experiments[i];
                            currExpt->updateChanges(syn->weightUpdateType);
                        }
                    } else {
                        syn->weightUpdateType->migrateComponent(newComp);
                        for (int i = 0; i < experiments.size(); ++i) {
                            experiment * currExpt = experiments[i];
                            currExpt->updateChanges(syn->weightUpdateType);
                        }
                    }
                }

                // replace references
                if (syn->postsynapseType->component == oldComp) {
                    // has the type changed?
                    if (newComp->type != "postsynapse") {
                        syn->postsynapseType->migrateComponent(catalogPS[0]);
                        for (int i = 0; i < experiments.size(); ++i) {
                            experiment * currExpt = experiments[i];
                            currExpt->updateChanges(syn->postsynapseType);
                        }
                    } else {
                        syn->postsynapseType->migrateComponent(newComp);
                        for (int i = 0; i < experiments.size(); ++i) {
                            experiment * currExpt = experiments[i];
                            currExpt->updateChanges(syn->postsynapseType);
                        }
                    }
                }
            }
        }

        // also fix experiments with bad pointers to PORTS and PARS & COMPONENTS
        for (int i = 0; i < experiments.size(); ++i) {
            experiment * currExpt = experiments[i];
            currExpt->purgeBadPointer(oldComp, newComp);
        }

    }

    // clear undo
    this->currProject->undoStack->clear();
}

// centralised function for finding if a component is in the model
bool rootData::isComponentInUse(QSharedPointer<NineMLComponent> oldComp)
{
    for (int p = 0; p < populations.size(); ++p) {

        QSharedPointer <population> pop = populations[p];

        // replace references
        if (pop->neuronType->component == oldComp) {
            return true;
        }

        for (int pr = 0; pr < pop->projections.size(); ++pr) {

            QSharedPointer <projection> proj = pop->projections[pr];

            for (int sy = 0; sy < proj->synapses.size(); ++sy) {

                QSharedPointer <synapse> syn = proj->synapses[sy];

                // replace references
                if (syn->weightUpdateType->component == oldComp) {
                    return true;
                }

                // replace references
                if (syn->postsynapseType->component == oldComp) {
                    return true;
                }
            }
        }
        // also fix experiments with bad pointers:
    }

    // component not found
    return false;
}

// centralised function for finding if a component is in the model
bool rootData::removeComponent(QSharedPointer<NineMLComponent> oldComp)
{
    if (isComponentInUse(oldComp)) {
        return false;
    }

    if (oldComp == NULL) {
        return true;
    }

    QVector < QSharedPointer<NineMLComponent> > * curr_lib;
    if (oldComp->type == "neuron_body") {
        curr_lib = &this->catalogNrn;
    }
    if (oldComp->type == "weight_update") {
        curr_lib = &this->catalogWU;
    }
    if (oldComp->type == "postsynapse") {
        curr_lib = &this->catalogPS;
    }

    for (int i = 0; i < curr_lib->size(); ++i) {
        if ((*curr_lib)[i] == oldComp) {
            curr_lib->erase(curr_lib->begin()+i);
            oldComp.clear();
            return true;
        }
    }

    // component not found
    return false;
}

void rootData::updateStatusBar(QString in_string, int time)
{
    emit statusBarUpdate(in_string, time);
}

void rootData::reDrawPanel()
{
    emit updatePanel(this);
}

void rootData::callRedrawGLview()
{
    emit redrawGLview();
}

void rootData::saveImage(QString fileName)
{
    if (!fileName.isEmpty()) {
        saveNetworkImageDialog svImDiag(this, fileName);
        svImDiag.exec();
    }
}

void rootData::reDrawAll()
{
    // update panel - we don't always want to do this as it loses focus from widgets
    emit updatePanel(this);
}

void rootData::reDrawAll(QPainter *painter, float GLscale, float viewX, float viewY, int width, int height, drawStyle style)
{
    if (style == standardDrawStyle) {
        for (int i = 0; i < selList.size(); ++i) {
            if (selList[i]->type == populationObject) {
                QSharedPointer <population> pop = qSharedPointerDynamicCast <population> (selList[i]);

                float left = ((pop->getLeft()+viewX)*GLscale+float(width))/2;
                float right = ((pop->getRight()+viewX)*GLscale+float(width))/2;
                float top = ((-pop->getTop()+viewY)*GLscale+float(height))/2;
                float bottom = ((-pop->getBottom()+viewY)*GLscale+float(height))/2;

                if (pop->isSpikeSource) {
                    for (int i = 5; i > 1; --i) {
                        QPen pen(QColor(0,0,0,50/i));
                        pen.setWidthF(float(i*2));
                        painter->setPen(pen);
                        painter->drawEllipse(QPointF((right+left)/2.0, (top+bottom)/2.0),0.5*GLscale/2.0,0.5*GLscale/2.0);
                    }

                } else {

                    for (int i = 5; i > 1; --i) {
                        QPen pen(QColor(0,0,0,50/i));
                        pen.setWidthF(float(i*2));
                        painter->setPen(pen);
                        QRectF rectangle(left, top, right-left, bottom-top);
                        painter->drawRoundedRect(rectangle,0.05*GLscale,0.05*GLscale);
                        //painter->drawRect(rectangle);
                    }
                }
            }
        }
    }

    // populations
    for (int i = 0; i < this->populations.size(); ++i) {
        this->populations[i]->draw(painter, GLscale, viewX, viewY, width, height, this->popImage, style);
    }
    for (int i = 0; i < this->populations.size(); ++i) {
        this->populations[i]->drawSynapses(painter, GLscale, viewX, viewY, width, height, style);
    }
    for (int i = 0; i < this->populations.size(); ++i) {
        QPen pen(QColor(100,0,0,100));

        QSettings settings;
        float dpi = settings.value("dpi", "1").toFloat();
        pen.setWidthF(float(1)/dpi);

        painter->setPen(pen);
        this->populations[i]->drawInputs(painter, GLscale, viewX, viewY, width, height, style);
    }

    // selected object
    if (style == standardDrawStyle) {
        for (int i = 0; i < selList.size(); ++i) {

            if (selList[i]->type == projectionObject) {

                QSharedPointer <projection> col = qSharedPointerDynamicCast <projection> (selList[i]);

                for (int i = 5; i > 1; --i) {
                    QPen pen(QColor(0,0,0,30/i));
                    pen.setWidthF(float(i*2));
                    painter->setPen(pen);
                    col->draw(painter, GLscale, viewX, viewY, width, height, this->popImage, standardDrawStyle);
                    // only draw handles if we aren't using multiple selection
                    if (selList.size() == 1) {
                        col->drawHandles(painter, GLscale, viewX, viewY, width, height);
                    }
                }
            }

            if (selList[i]->type == inputObject) {

                QSharedPointer<genericInput> input = qSharedPointerDynamicCast<genericInput> (selList[i]);

                for (int i = 5; i > 1; --i) {
                    QPen pen(QColor(0,0,0,30/i));
                    pen.setWidthF(float(i*2));
                    painter->setPen(pen);
                    input->draw(painter, GLscale, viewX, viewY, width, height, this->popImage, standardDrawStyle);
                    // only draw handles if we aren't using multiple selection
                    if (selList.size() == 1) {
                        input->drawHandles(painter, GLscale, viewX, viewY, width, height);
                    }
                }
            }
        }
    }

    // cursor
    painter->setPen(QColor(0,0,0,255));

    float x = ((this->cursor.x+viewX)*GLscale+float(width))/2;
    float y = ((-this->cursor.y+viewY)*GLscale+float(height))/2;

    QPointF point(x,y);
    painter->drawEllipse(point,10,10);
    painter->drawLine(QLineF(x, y-14.0f, x, y+14.0f));
    painter->drawLine(QLineF(x-14.0f, y, x+14.0f, y));

    // update positions
    for (int i = 0; i < this->populations.size(); ++i) {
        this->populations[i]->animate(this->populations[i]);
    }

    // draw dragselect if present
    if (fabs(this->dragSelection.width()) > 0.001) {
        // convert to screen co-ords
        QRectF dragSelectionScreen;
        // top left conversion
        QPointF point = dragSelection.topLeft();
        point.setX(((point.x()+viewX)*GLscale+width)/2);
        point.setY(((-point.y()+viewY)*GLscale+height)/2);
        dragSelectionScreen.setTopLeft(point);
        // bottom right conversion
        point = dragSelection.bottomRight();
        point.setX(((point.x()+viewX)*GLscale+width)/2);
        point.setY(((-point.y()+viewY)*GLscale+height)/2);
        dragSelectionScreen.setBottomRight(point);
        // make sure the rect is not inverted (i.e. non-negative with & height) so draws correctly
        dragSelectionScreen = dragSelectionScreen.normalized();
        // set draw style
        painter->save();
        painter->setPen(QColor(0,255,0,200));
        painter->setBrush(QColor(0,255,0,50));
        // draw
        painter->drawRect(dragSelectionScreen);
        painter->restore();
    }
}

void destroyDom(QDomNode &node)
{
    QDomNodeList childList = node.childNodes();
    QDomNode child;
    for (int i = 0; i < childList.count(); ++i) {
        child = childList.item(i);
        destroyDom(child);
        child.clear();
    }
}

void rootData::onRightMouseDown(float xGL, float yGL, float GLscale)
{
    // insert new point into projection
    if (selList.size() == 1) {
        if (this->selList[0]->type == projectionObject || this->selList[0]->type == inputObject) {
            // try to delete control point, if that fails then add one!
            QSharedPointer <projection> proj = qSharedPointerDynamicCast <projection> (selList[0]);
            if (!proj->deleteControlPoint(xGL, yGL, GLscale)) {
                proj->insertControlPoint(xGL, yGL, GLscale);
            }
        }
    }
    // log the co-ordinates for drag select
    this->dragListStart = QPointF(xGL, yGL);
}

void rootData::dragSelect(float xGL, float yGL)
{
    bool addSelection = (QApplication::keyboardModifiers() & Qt::ShiftModifier);

    // setup the QRect for the selection
    this->dragSelection = QRectF(this->dragListStart, QPointF(xGL, yGL));

    QVector <QSharedPointer<systemObject> > oldSel = selList;

    // clear slection list if shift not held
    if (!addSelection) {
        selList.clear();
    }

    // add selected objects to list
    for (int i = 0; i < this->populations.size(); ++i) {
        QSharedPointer <population> pop = populations[i];

        if (dragSelection.contains(pop->x, pop->y)) {
            // if not already selected
            bool alreadySelected = false;
            for (int s = 0; s < this->selList.size(); ++s) {
                if (selList[s] == pop) {
                    alreadySelected = true;
                }
            }
            if (!alreadySelected) {
                selList.push_back(pop);
            }
        }

        for (int j = 0; j < pop->neuronType->inputs.size(); ++j) {
            QSharedPointer<genericInput> in = pop->neuronType->inputs[j];

            if (in->curves.size() > 0) {
                if (dragSelection.contains(in->start) && dragSelection.contains(in->curves.back().end)) {
                    // if not already selected
                    bool alreadySelected = false;
                    for (int s = 0; s < this->selList.size(); ++s) {
                        if (selList[s] == in) {
                            alreadySelected = true;
                        }
                    }
                    if (!alreadySelected) {
                        selList.push_back(in);
                    }
                }
            }
        }

        for (int j = 0; j < pop->projections.size(); ++j) {
            QSharedPointer <projection> proj = pop->projections[j];

            if (proj->curves.size() > 0) {
                if (dragSelection.contains(proj->start) && dragSelection.contains(proj->curves.back().end)) {
                    // if not already selected
                    bool alreadySelected = false;
                    for (int s = 0; s < this->selList.size(); ++s) {
                        if (selList[s] == proj) {
                            alreadySelected = true;
                        }
                    }
                    if (!alreadySelected) {
                        selList.push_back(proj);
                    }
                }
            }

            for (int pt = 0; pt < proj->synapses.size(); ++pt) {

                QSharedPointer <synapse> projT = proj->synapses[pt];


                // select generic inputs for weightupdate
                for (int c = 0; c < projT->weightUpdateType->inputs.size(); ++c) {

                    QSharedPointer<genericInput> in = projT->weightUpdateType->inputs[c];

                    if (in->curves.size() > 0) {
                        if (dragSelection.contains(in->start) && dragSelection.contains(in->curves.back().end)) {
                            // if not already selected
                            bool alreadySelected = false;
                            for (int s = 0; s < this->selList.size(); ++s) {
                                if (selList[s] == in) {
                                    alreadySelected = true;
                                }
                            }
                            if (!alreadySelected) {
                                selList.push_back(in);
                            }
                        }
                    }
                }

                // select generic inputs for postsynapse
                for (int c = 0; c < projT->postsynapseType->inputs.size(); ++c) {

                    QSharedPointer<genericInput> in = projT->postsynapseType->inputs[c];

                    if (in->curves.size() > 0) {
                        if (dragSelection.contains(in->start) && dragSelection.contains(in->curves.back().end)) {
                            // if not already selected
                            bool alreadySelected = false;
                            for (int s = 0; s < this->selList.size(); ++s) {
                                if (selList[s] == in) {
                                    alreadySelected = true;
                                }
                            }
                            if (!alreadySelected) {
                                selList.push_back(in);
                            }
                        }
                    }
                }
            }
        }
    }

    // check if the selection has changed to prevent unecessary redrawing
    bool selectionChanged = false;
    if (oldSel.size() == selList.size()) {
        for (int i = 0; i < oldSel.size(); ++i) {
            if (oldSel[i] != selList[i]) {
                selectionChanged = true;
            }
        }
    } else {
        selectionChanged = true;
    }

    if (selectionChanged) {
        reDrawAll();
    }
}

void rootData::endDragSelection()
{
    this->dragSelection = QRect(-1,-1,0,0);
}

#ifdef NEED_MOUSE_UP_LOGIC
void rootData::onLeftMouseUp (float xGL, float yGL, float GLscale)
{
    // All selection logic is in onLeftMouseDown(), so currently, this
    // method is a no-op.
}
#endif

void rootData::itemWasMoved()
{
    if (!this->selList.empty()) {
        // We have a pointer(s) to the moved item(s). Check types to
        // see what to do with it/them.  If ANY object in selList is a
        // population, then call populationMoved.
        QVector <QSharedPointer<population> > pops = this->currSelPopulations();
        if (!pops.empty()) {
            this->populationMoved (pops);
        } // else do nothing

        if (this->selList.size() == 1
            && (this->selList[0]->type == projectionObject
                || this->selList[0]->type == inputObject)) {
            // The selected thing was moved.
            this->projectionHandleMoved();
        }
    }
}

void rootData::projectionHandleMoved()
{
    // Already checked before call, but lets be safe
    if (this->selList.size() == 1
        && (this->selList[0]->type == projectionObject
            || this->selList[0]->type == inputObject)) {
        // New position of the handle
        QSharedPointer<projection> projptr = qSharedPointerDynamicCast<projection>(this->selList[0]);
        QPointF newPos = projptr->selectedControlPointLocation();
        currProject->undoStack->push(new moveProjectionHandle(this, projptr, lastLeftMouseDownPos, newPos));
    }
}

void rootData::populationMoved(const QVector <QSharedPointer<population> >& pops)
{
    if (pops.empty()) {
        return;
    }

    // We don't need to record that the population moved here - the
    // mouseMove events already handle that (allowing the population
    // widget to be re-drawn as it's moved. However, what we DO need
    // to do is to record that the population moved in the undo stack.

    QVector <QSharedPointer<population> >::const_iterator popsi = pops.begin();

    // A parent undocommand which will group together potentially multiple undos.
    QUndoCommand* parentCmd = new QUndoCommand();

    while (popsi != pops.end()) {
        // New position of the population, which it already has a record of.
        QPointF newPos((*popsi)->targx, (*popsi)->targy);

        // The last position of the population is the
        // lastSelectionPosition (which is the *mouse* position) offset by
        // the current offset in the population (as the population moves,
        // the mouse remains in the same location on the object).
        QPointF lastPopulationPosition = lastLeftMouseDownPos + (*popsi)->getLocationOffset();
        new movePopulation(this, (*popsi), lastPopulationPosition, newPos, parentCmd);
        ++popsi;
    }

    if (pops.size() > 1) {
        parentCmd->setText ("move populations");
    } else {
        parentCmd->setText ("move population");
    }

    currProject->undoStack->push(parentCmd);
}

void rootData::onNewSelections (float xGL, float yGL)
{
    qDebug() << __FUNCTION__ << " emitting updatePanel";
    emit updatePanel(this);
    for (int i = 0; i < this->selList.size(); ++i) {
        // register locations relative to cursor:
        qDebug() << "Setting a location offset...";
        this->selList[i]->setLocationOffsetRelTo(xGL, yGL);
    }
}

bool rootData::selListContains (const QVector <QSharedPointer<systemObject> >& objectList)
{
    QVector <QSharedPointer<systemObject> >::const_iterator i = objectList.begin();
    while (i != objectList.end()) {
        if (std::find (this->selList.begin(), this->selList.end(), *i) != this->selList.end()) {
            return true;
        }
        ++i;
    }
    return false;
}

void rootData::deleteFromSelList (const QVector <QSharedPointer<systemObject> >& objectList)
{
    QVector <QSharedPointer<systemObject> >::const_iterator i = objectList.begin();
    while (i != objectList.end()) {
        QVector <QSharedPointer<systemObject> >::iterator j = this->selList.begin();
        while (j != this->selList.end()) {
            if (*i == *j) { // That is, selList contains a member of objectList
                j = this->selList.erase (j);
            } else {
                ++j;
            }
        }
        ++i;
    }
}

// When the "left" mouse goes down, select what's underneath, if anything.
void rootData::onLeftMouseDown(float xGL, float yGL, float GLscale, bool shiftDown)
{
    qDebug() << __FUNCTION__ << " called, shift is " << (shiftDown ? "Down" : "Up");

    // Record the position of the selection.
    this->lastLeftMouseDownPos.setX(xGL);
    this->lastLeftMouseDownPos.setY(yGL);

    // Before finding selections, see if user has an already selected
    // projection/input and if so, if user has clicked on a handle.
    if (!this->selList.empty() && this->selList.size() == 1) {
        if (this->selList[0]->type == projectionObject || this->selList[0]->type == inputObject) {
            QSharedPointer <projection> proj = qSharedPointerDynamicCast <projection> (selList[0]);
            if ( proj->selectControlPoint(xGL, yGL, GLscale) ) {
                return;
            } // else no handle on the inputObject/projectionObject was selected
        } // else the previously selected object was not an input or projection
    }

    // A list of things which have been selected with this left mouse
    // down click. Will be added to this->selList after the logic in
    // this method.
    QVector <QSharedPointer<systemObject> > newlySelectedList;
    this->findSelection (xGL, yGL, GLscale, newlySelectedList);

    // Possibilities:
    // 1. Nothing previously selected, user clicked on new object, shift OR no shift -> new selection
    // 3. Nothing previously selected, user unselected, shift OR no shift -> do nothing
    //
    // 4. Something previously selected, user clicked on new object, no shift -> switch selection
    // 5. Something previously selected, user clicked on new object, WITH shift -> add to selection
    // 6. Something previously selected, user unselected, WITH shift -> do nothing
    // 7. Something previously selected, user unselected, no shift -> clear selection
    //
    // 8. Something(s) were previously selected, user clicked on one of them -> User is moving selection.

    if (this->selList.empty()) { // Nothing previously selected.
        qDebug() << "Nothing prev. selected...";
        if (newlySelectedList.empty()) {
            // Nothing selected now, do nothing but show the cursor.
            qDebug() << "Nothing selected now, so show cursor.";
            cursor.x = xGL;
            cursor.y = yGL;
            emit updatePanel(this);
        } else {
            // Have new selection, selList is empty, swap the contents
            // of newlySelectedList into selList.
            qDebug() << "New selection, swap new selection into selList.";
            this->selList.swap (newlySelectedList);
            this->onNewSelections(xGL, yGL);
        }

    } else { // We have a previous selection.
        qDebug() << "We have a previous selection...";
        if (newlySelectedList.empty()) {
            // Nothing selected now.
            qDebug() << "Nothing is newly selected...";
            if (shiftDown) {
                // User still has shift down; do nothing. Show cursor?
                qDebug() << "User has shift down. Show cursor.";
                cursor.x = xGL;
                cursor.y = yGL;
                // If we selected nothing, then just this:
                emit updatePanel(this);
            } else {
                // Clear selection and show cursor:
                qDebug() << "User doesn't have shift down, so clear selection and show cursor.";
                this->cursor.x = xGL;
                this->cursor.y = yGL;
                this->selList.clear();
                this->onNewSelections(xGL, yGL);
            }
        } else {
            // Have new selection
            qDebug() << "We have a new selection...";
            if (shiftDown) {
                // User still has shift down; append, leaving cursor unchanged
                if (!this->selListContains (newlySelectedList)) {
                    qDebug() << "User has shift down, (some of) newlySelected is not in selList, so append newlySelected onto selList";
                    //this->selList.insert (this->selList.end(), newlySelectedList.begin(), newlySelectedList.end());
                    // since we have moved to QVectors this should have the same effect as the above - Alex 17 July 2014
                    this->selList += newlySelectedList;
                } else {
                    // user has shift down,. but newlySelected is already in selList, so in this case REMOVE it!
                    qDebug() << "user has shift down clicking on existing object, so delete";
                    this->deleteFromSelList (newlySelectedList);
                }
            } else {
                // Shift not down, user wishes to switch selection OR move several selected items
                qDebug() << "Shift is not down, so user wishes to switch selection or move selected items. Swap newly selected into selList";
                if (this->selListContains (newlySelectedList)) {
                    qDebug() << "selList contains newly selected; user wishes to MOVE selected items.";
                    // Nothing further to do here?
                } else {
                    // Swap selection, leave cursor unchanged.
                    qDebug() << "newly selected not in selList. user wishes to switch selection. Swap newly selected into selList";
                    this->selList.swap (newlySelectedList);
                }
#if 0
                // Or maybe:?
                for (int i = 0; i < selList.size(); ++i) {
                    // register locations relative to cursor:
                    this->selList[i]->setLocationOffsetRelTo(xGL, yGL);
                }
                this->selectionMoved = false;
#endif
            }
            // As we selected something do this:
            this->onNewSelections(xGL, yGL);
        }
    }
}

void rootData::findSelection (float xGL, float yGL, float GLscale, QVector <QSharedPointer<systemObject> >& newlySelectedList)
{
    // Now look at populations and their inputs.
    for (int i = 0; i < this->populations.size(); ++i) {

        // check population inputs:
        for (int j = 0; j < this->populations[i]->neuronType->inputs.size(); ++j) {
            // find if an edge of the input is hit
            if (this->populations[i]->neuronType->inputs[j]->is_clicked(xGL, yGL, GLscale)) {
                // add the input to the selection list
                newlySelectedList.push_back(this->populations[i]->neuronType->inputs[j]);
                // selection complete, move on
                return;
            }
        } // for loop checking population inputs

        // check projections
        for (int j = 0; j < this->populations[i]->projections.size(); ++j) {

            // find if an edge of the projection is hit
            if (this->populations[i]->projections[j]->is_clicked(xGL, yGL, GLscale)) {
                // add to selection list
                newlySelectedList.push_back(this->populations[i]->projections[j]);
                // selection complete, move on
                return;
            }

            // check proj inputs:
            for (int k = 0; k < this->populations[i]->projections[j]->synapses.size(); ++k) {

                QSharedPointer <synapse> col = this->populations[i]->projections[j]->synapses[k];

                for (int l = 0; l < col->weightUpdateType->inputs.size(); ++l) {

                    // find if an edge of the input is hit
                    if (col->weightUpdateType->inputs[l]->is_clicked(xGL, yGL, GLscale)) {
                        //  add to selection list
                        newlySelectedList.push_back(col->weightUpdateType->inputs[l]);
                        // selection complete, move on
                        return;
                    }
                }

                for (int l = 0; l < col->postsynapseType->inputs.size(); ++l) {
                    // find if an edge of the input is hit
                    if (col->postsynapseType->inputs[l]->is_clicked(xGL, yGL, GLscale)) {
                        //  add to selection list
                        newlySelectedList.push_back(col->postsynapseType->inputs[l]);
                        // selection complete, move on
                        return;
                    }
                }
            } // for loop checking projection inputs.
        } // for-loop over projections

        // select if under the cursor - no two objects should overlap!
        if (this->populations[i]->is_clicked(xGL, yGL, GLscale)) {
            //  add to selection list
            newlySelectedList.push_back(this->populations[i]);
            // selection complete, move on
            return;
        }
    } // for-loop over populations
}

QColor rootData::getColor(QColor initCol)
{
    // launch the QT colorpicker, and get a colour - if cancel is
    // pressed then the output fails .isValid(), and we return the
    // input instead
    QColor out = QColorDialog::getColor(initCol);
    if (out.isValid()) {
        return out;
    } else {
        return initCol;
    }
}

void rootData::deleteCurrentSelection()
{
    if (selList.size() > 1) {
        currProject->undoStack->push(new delSelection(this, selList));
    } else if (selList.size() == 1) {
        // separate out:
        if (selList[0]->type == populationObject) {
            QSharedPointer <population> pop = qSharedPointerDynamicCast <population> (selList[0]);
            currProject->undoStack->push(new delPopulation(this,pop));
        }
        else if (selList[0]->type == projectionObject) {
            QSharedPointer <projection> proj = qSharedPointerDynamicCast <projection> (selList[0]);
            currProject->undoStack->push(new delProjection(this, proj));
        }
        else if (selList[0]->type == inputObject) {
            QSharedPointer <genericInput> in = qSharedPointerDynamicCast <genericInput> (selList[0]);
            currProject->undoStack->push(new delInput(this,in));
        }
    }
    this->undoOrRedoPerformed(0);
}

void rootData::addPopulation()
{
    if (this->cursor.x != -100000) {
        QString popName = "Population";
        // go and get a decent number:
        popName = getUniquePopName(popName);

        QSharedPointer <population> pop = QSharedPointer<population> (new population(cursor.x, cursor.y, 1.0f,  5.0/3.0f, popName));
        pop->tag = this->getIndex();
        pop->layoutType = QSharedPointer<NineMLLayoutData> (new NineMLLayoutData(this->catalogLayout[0]));
        pop->neuronType = QSharedPointer <NineMLComponentData> (new NineMLComponentData(this->catalogNrn[0]));
        pop->neuronType->owner = pop;

        currProject->undoStack->push(new addPopulationCmd(this, pop));

        emit updatePanel(this);
        emit redrawGLview();
    }
}

void rootData::addSpikeSource()
{
    if (this->cursor.x != -100000) {
        QString popName = "Spike Source";
        // go and get a decent number:
        popName = getUniquePopName(popName);

        QSharedPointer <population> pop = QSharedPointer <population> (new population(cursor.x, cursor.y, 1.0f,  5.0/3.0f, popName));
        pop->tag = this->getIndex();
        pop->layoutType = QSharedPointer<NineMLLayoutData> (new NineMLLayoutData(this->catalogLayout[0]));

        // make a spikes source
        pop->makeSpikeSource(pop);

        currProject->undoStack->push(new addPopulationCmd(this, pop));

        emit updatePanel(this);
        emit redrawGLview();
    }
}

void rootData::addBezierOrProjection(float xGL, float yGL)
{
    if (this->selList.size() == 1) {
        if (this->selList[0]->type == projectionObject) {

            // shortcuts to the projection and population
            QSharedPointer <projection> proj = qSharedPointerDynamicCast <projection> (this->selList[0]);
            QSharedPointer <population> pop = proj->source;

            // find if we have hit a population:
            for (int i = 0; i < this->populations.size(); ++i) {

                // ignore spike sources
                if (this->populations[i]->isSpikeSource) continue;

                QPainterPath box;
                if (this->populations[i]->addToPath(&box)->contains(QPointF(xGL,yGL))) {

                    // we have a collision, so fix up the connection and return:
                    QSharedPointer <population> dest = this->populations[i];

                    // first check for an existing connection...
                    if (dest->connectsTo(pop)) {
                        // EXISTING CONNECTION - NOTIFY USER
                        emit statusBarUpdate(tr("Connection exists already!"), 2000);
                    } else {
                        // new connection! finalise the connection...
                        dest->reverseProjections.push_back(proj);
                        proj->destination = dest;
                        // add to undo stack
                        currProject->undoStack->push(new addProjection(this, proj));
                        // redraw the parameters panel
                        emit updatePanel(this);
                        // tell gl viewport to stop tracking the mouse
                        emit finishDrawingSynapse();
                    }
                    // In both cases above, we now want to return:
                    return;
                }
            }

            // no population hit, so add a curve:
            emit statusBarUpdate(tr("Adding new point"), 2000);
            // create new bezierCurve
            bezierCurve newCurve;
            proj->curves.push_back(newCurve);
        }
    }
}

void rootData::startAddBezier(float xGL, float yGL)
{
    if (this->selList.size() == 1) {
        if (this->selList[0]->type == populationObject) {

            QSharedPointer <population> pop = qSharedPointerDynamicCast <population> (selList[0]);

            emit statusBarUpdate(tr("Adding new connection"), 2000);
            // create new bezierCurve
            bezierCurve newCurve;

            // add the new projection (for now)
            pop->projections.push_back(QSharedPointer<projection> (new projection()));
            pop->projections.back()->tag = getIndex();
            pop->projections.back()->source = pop;

            // select the new projection
            this->selList.clear();
            this->selList.push_back(pop->projections.back());

            pop->projections.back()->curves.push_back(newCurve);
        }

        if (this->selList[0]->type == projectionObject) {

            //shortcut to projection:
            QSharedPointer <projection> col = qSharedPointerDynamicCast <projection> (selList[0]);
            // and source pop
            QSharedPointer <population> pop = col->source;

            // setup the curve
            // add start where the line from pop to cursor hits the bounding box of the pop

            // if we are a self selection
            QPainterPath * tempPP = new QPainterPath;
            if (pop->addToPath(tempPP)->contains(QPointF(xGL, yGL))) {

                // self selection
                col->start = QPointF(pop->getRight(), pop->getBottom() + 0.3*pop->size);
                col->curves.back().end = QPointF(pop->getRight()- 0.3*pop->size, pop->getBottom());
                col->curves.back().C1 = QPointF(pop->getRight() + 1.0, pop->getBottom() + 0.3*pop->size);
                col->curves.back().C2 = QPointF(pop->getRight()- 0.3*pop->size, pop->getBottom() - 1.0);

            } else {

                // see if any of the other populations are under the mouse cursor
                int selPop = -1;
                for (int i = 0; i < this->populations.size(); ++i) {
                    // ignore spike sources
                    if (this->populations[i]->isSpikeSource) continue;
                    QPainterPath * tempPP = new QPainterPath;
                    if (this->populations[i]->addToPath(tempPP)->contains(QPointF(xGL, yGL))) {
                        selPop = i;
                    }
                    delete tempPP;
                }

                if (selPop != -1) {

                    QSharedPointer <population> dest = this->populations[selPop];

                    if (col->curves.size() == 1) {
                        // if first curve then setup start point
                        // find where on the population box we should start the col
                        QPointF boxEdge = col->findBoxEdge(pop, xGL, yGL);
                        col->start = boxEdge;

                        // add end point
                        // find where on the population box we should start the col
                        boxEdge = col->findBoxEdge(dest, boxEdge.x(), boxEdge.y());
                        col->curves.back().end = boxEdge;
                    } else {
                        // add end point
                        // find where on the population box we should start the col
                        QPointF boxEdge = col->findBoxEdge(dest, col->curves[col->curves.size()-2].end.x(), col->curves[col->curves.size()-2].end.y());
                        col->curves.back().end = boxEdge;
                    }

                    // set up handles
                    col->setAutoHandles(pop, dest, QPointF(xGL,yGL));

                } else {

                    if (col->curves.size() == 1) {
                        // if first curve then setup start point
                        // find where on the population box we should start the col
                        QPointF boxEdge = col->findBoxEdge(pop, xGL, yGL);
                        col->start = boxEdge;
                    }
                    // setup end point
                    col->curves.back().end = QPointF(xGL, yGL);
                    // set up handles
                    col->setAutoHandles(pop, pop, QPointF(xGL,yGL));
                }

            }
            delete tempPP;
        }
    }
}

void rootData::abortProjection()
{
    // the new projection should be the only one in the selList, but may not be...
    if (selList[0]->type == projectionObject) {
        QSharedPointer <projection> proj = qSharedPointerDynamicCast <projection> (this->selList[0]);
        // remove from the selList so we don't break when it is deleted
        selList.clear();
        // remove references from other components
        proj->disconnect();
        // remove from the system
        proj->remove(this);
    }
}

void rootData::mouseMoveGL(float xGL, float yGL)
{
    selectionMoved = true;
    //qDebug() << "pos = " << xGL << " " << yGL;

    if (this->selList.empty()) {
        // move viewpoint only, then return.
        CHECK_CAST(dynamic_cast<GLWidget *>(sender()))
        GLWidget * source = (GLWidget *) sender();
        source->move(xGL+source->viewX-cursor.x,yGL-source->viewY-cursor.y);
        return;
    }

    // revised move code for multiple objects

    // if grid is on, snap to grid
    CHECK_CAST(dynamic_cast<GLWidget *>(sender()))
    GLWidget * source = (GLWidget *) sender();
    if (source->gridSelect) {
        xGL = round(xGL/source->gridScale)*source->gridScale;
        yGL = round(yGL/source->gridScale)*source->gridScale;
    }

    if (selList.size() > 1) {
        for (int i = 0; i < selList.size(); ++i) {
            selList[i]->move(xGL, yGL);
        }
    } else {
        // if only one thing

        if (this->selList[0]->type == populationObject) {
            bool collision = false;
            QSharedPointer <population> pop = qSharedPointerDynamicCast <population> (selList[0]);

            // avoid collisions
            for (int i = 0; i < populations.size(); ++i) {
                if (populations[i]->getName() != pop->getName()) {
                    if (populations[i]->within_bounds(pop->leftBound(xGL)+0.01, pop->topBound(yGL)-0.01)) collision = true;
                    if (populations[i]->within_bounds(pop->rightBound(xGL)-0.01, pop->topBound(yGL)-0.01)) collision = true;
                    if (populations[i]->within_bounds(pop->leftBound(xGL)+0.01, pop->bottomBound(yGL)+0.01)) collision = true;
                    if (populations[i]->within_bounds(pop->rightBound(xGL)-0.01, pop->bottomBound(yGL)+0.01)) collision = true;
                }
            }

            // snap to grid:
            if (source->gridSelect) {
                selList[0]->setLocationOffset (QPointF(0,0));
            }

            if (!collision) {
                selList[0]->move(xGL, yGL);
            }

        } else if (this->selList[0]->type == projectionObject) {
            // not a population in this case...
            QSharedPointer<projection> proj = qSharedPointerDynamicCast <projection> (selList[0]);
            CHECK_CAST(proj)
            proj->moveSelectedControlPoint(xGL, yGL);

        } else if (this->selList[0]->type == inputObject) {
            // not a population in this case either
            QSharedPointer<genericInput> in = qSharedPointerDynamicCast <genericInput> (selList[0]);
            CHECK_CAST(in)
            in->moveSelectedControlPoint(xGL, yGL);
        }
    }
}

void rootData::updatePortMap(QString var)
{
    // update the variables to connect up the stages of the currently selected projection
    genericInput * ptr = (genericInput *) sender()->property("ptr").value<void *>();

    if (!this->isValidPointer(ptr)) {
        qDebug() << "Found a bad pointer in updatePortMap";
        exit(0);
    }

    QStringList ports = var.split("->");
    // for safety
    if (ports.size()>1) {
        ptr->srcPort = ports.at(0);
        ptr->dstPort = ports.at(1);
    }
}

void rootData::updateComponentType(int index)
{
    // update the components of the currently selected object
    QSharedPointer <population> currSel;
    QSharedPointer <synapse> targSel;
    QSharedPointer<genericInput> inSel;

    QSharedPointer<systemObject> ptr;
    // get ptr
    if (selList.size() ==1) {
        ptr = selList[0];
    }

    if (ptr.isNull()) {
        return;
    }

    // if projection we need the current synapse
    if (ptr->type == projectionObject) {
        QSharedPointer <projection> proj = qSharedPointerDynamicCast <projection> (ptr);
        ptr = proj->synapses[proj->currTarg];
    }

    // get type of change
    QString type = sender()->property("type").toString();

    // event says we need to update the selected object accordingly, so we'll do that first:
    switch (ptr->type) {
        case populationObject:
            // if there is a mis-match between the type and the selected type then update
            currSel = qSharedPointerDynamicCast <population> (ptr);

            if (type == "layout") {
                if (index >= 0) {
                    if (currSel->layoutType->component->name.compare(this->catalogLayout[index]->name) != 0) {
                        currSel->layoutType.clear();
                        currSel->layoutType = QSharedPointer<NineMLLayoutData> (new NineMLLayoutData(this->catalogLayout[index]));
                        this->reDrawAll();
                    }
                }
            }
            if (type == "neuron") {
                if (index >= 0) {
                    if (currSel->neuronType->component->name != this->catalogNrn[index]->name || \
                        currSel->neuronType->component->path != this->catalogNrn[index]->path) {
                        currProject->undoStack->push(new updateComponentTypeUndo(this, currSel->neuronType, this->catalogNrn[index]));
                    }
                }
            }
            break;
        case synapseObject:
            targSel = qSharedPointerDynamicCast <synapse> (ptr);
            if (type == "weight_update") {
                if (index >= 0) {
                    if (targSel->weightUpdateType->component->name != this->catalogWU[index]->name || \
                        targSel->weightUpdateType->component->path != this->catalogWU[index]->path) {
                        currProject->undoStack->push(new updateComponentTypeUndo(this, targSel->weightUpdateType, this->catalogWU[index]));
                    }
                }
            }
            if (type == "postsynapse") {
                if (index >= 0) {
                    if (targSel->postsynapseType->component->name != this->catalogPS[index]->name || \
                        targSel->postsynapseType->component->path != this->catalogPS[index]->path) {
                        currProject->undoStack->push(new updateComponentTypeUndo(this, targSel->postsynapseType, this->catalogPS[index]));
                    }
                }
            }
            if (type == "conn") {
                if (index >= 0) {
                    if (targSel->connectionType->getIndex() != index) {
                        currProject->undoStack->push(new changeConnection(this, ptr, index));
                    }
                }
            }
            break;
        case inputObject:
            inSel = qSharedPointerDynamicCast<genericInput> (ptr);
            if (type == "input") {
                if (index >= 0) {
                    if (inSel->connectionType->getIndex() != index) {
                        currProject->undoStack->push(new changeConnection(this, ptr, index));
                    }
                }
            }
            break;
        default:
            cerr << "Something has gone badly wrong!";
            exit(-1);
            break;
    }

    // redraw GL view
    emit redrawGLview();

    emit updatePanelView2("comboboxOSXfix");
}

void rootData::updatePanelView2Accessor()
{
    emit updatePanelView2("");
}

void rootData::updatePar()
{
    QString action = sender()->property("action").toString();

    // update the type of parameter
    if (action == "updateType") {
        ParameterData * par = (ParameterData *) sender()->property("ptr").value<void *>();
        CHECK_CAST(dynamic_cast<ParameterData *>(par))
        QString newType = sender()->property("newType").toString();
        currProject->undoStack->push(new updateParType(this, par, newType));
    }

    // launch the list editor dialog
    if (action == "editList") {
        ParameterData * par = (ParameterData *) sender()->property("ptr").value<void *>();
        CHECK_CAST(dynamic_cast<ParameterData *>(par))
        valueListDialog * dialog  = new valueListDialog(par);
        dialog->show();
    }

    if (action == "changeVal") {
        // Update the parameter value
        ParameterData * par = (ParameterData *) sender()->property("ptr").value<void *>();
        CHECK_CAST(dynamic_cast<ParameterData *>(par))
        int index = sender()->property("valToChange").toInt();
        float value = ((QDoubleSpinBox *) sender())->value();
        // only add undo if value has changed
        if (value != par->value[index]) {
            currProject->undoStack->push(new updateParUndo(this, par, index, value));
        }
    }

    if (action == "changeConnProb") {
        // Update the parameter value
        fixedProb_connection * conn = (fixedProb_connection *) sender()->property("ptr").value<void *>();
        CHECK_CAST(dynamic_cast<fixedProb_connection *>(conn))
        float value = ((QDoubleSpinBox *) sender())->value();
        // only add undo if value has changed
        if (value != conn->p) {
            currProject->undoStack->push(new updateConnProb(this, conn, value));
        }
    }

    if (action == "changeConnKerSize") {
        // Update the parameter value
        kernel_connection * conn = (kernel_connection *) sender()->property("ptr").value<void *>();
        CHECK_CAST(dynamic_cast<kernel_connection *>(conn))
        int kernel_size = ((QComboBox *) sender())->currentIndex() * 2 + 3;
        // only add undo if value has changed
        conn->setKernelSize(kernel_size);
        emit updatePanelView2("");
    }

    if (action == "changeConnKerScale") {
        // Update the parameter value
        kernel_connection * conn = (kernel_connection *) sender()->property("ptr").value<void *>();
        CHECK_CAST(dynamic_cast<kernel_connection *>(conn))
        float kernel_scale = ((QDoubleSpinBox *) sender())->value();
        CHECK_CAST(dynamic_cast<QDoubleSpinBox *>(sender()))
        // only add undo if value has changed
        conn->setKernelScale(kernel_scale);
    }

    if (action == "changeConnKernel") {
        // Update the parameter value
        kernel_connection * conn = (kernel_connection *) sender()->property("ptr").value<void *>();
        CHECK_CAST(dynamic_cast<kernel_connection *>(conn))
        float kernel_value = ((QDoubleSpinBox *) sender())->value();
        CHECK_CAST(dynamic_cast<QDoubleSpinBox *>(sender()))
        int i = sender()->property("i").toInt();
        int j = sender()->property("j").toInt();
        // only add undo if value has changed
        conn->setKernel(i,j,kernel_value);
    }

    if (action == "changePythonScriptPar") {
        // Update the parameter value
        pythonscript_connection * conn = (pythonscript_connection *) sender()->property("ptr").value<void *>();
        CHECK_CAST(dynamic_cast<pythonscript_connection *>(conn))
        float par_value = ((QDoubleSpinBox *) sender())->value();
        QString par_name = ((QDoubleSpinBox *) sender())->property("par_name").toString();
        CHECK_CAST(dynamic_cast<QDoubleSpinBox *>(sender()))
        // only add undo if value has changed
        currProject->undoStack->push(new undoUpdatePythonConnectionScriptPar(this, conn, par_value, par_name));
    }

    if (action == "changePythonScriptProp") {
        // Update the property affected by the connection
        pythonscript_connection * conn = (pythonscript_connection *) sender()->property("ptr").value<void *>();
        CHECK_CAST(dynamic_cast<pythonscript_connection *>(conn))
        QString par_name = ((QComboBox *) sender())->currentText();
        CHECK_CAST(dynamic_cast<QComboBox *>(sender()))
        // only add undo if value has changed
        currProject->undoStack->push(new undoUpdatePythonConnectionScriptProp(this, conn, par_name));
    }
}

void rootData::updatePar(int value)
{
    // Update the parameter value
    ParameterData * par = (ParameterData *) sender()->property("ptr").value<void *>();
    CHECK_CAST(dynamic_cast<ParameterData *>(par))
    par->value[0] = value;

    switch (value) {
    case 0:
        par->value.resize(4);
        break;
    case 1:
        par->value.resize(4);
        par->value[2] = 1;
        par->value[3] = 123;
        break;
    case 2:
        par->value.resize(4);
        par->value[2] = 1;
        par->value[3] = 123;
        break;
    }

    // update panel
    updatePanel(this);
}

void rootData::updateDrawStyle() {

    drawStyle style = (drawStyle) sender()->property("style").toUInt();

    if (this->selList.size() == 1) {
        // have we only got a proj selected (this should always be the case)
        if (this->selList[0]->type == projectionObject) {
            // cast to a proj
            QSharedPointer <projection> proj = qSharedPointerDynamicCast <projection> (selList[0]);
            // test if the cast succeeded
            if (!proj.isNull()) {
                currProject->undoStack->push(new updateProjDrawStyle(proj, style, proj->style()));
            }
        }
    }
    emit updatePanel(this);
}

QVector <QSharedPointer<population> > rootData::currSelPopulations()
{
    // get the currently selected populations (ALL of them)
    QVector <QSharedPointer<population> > currSel;
    QVector <QSharedPointer<systemObject> >::const_iterator i = this->selList.begin();
    while (i != this->selList.end()) {
        if ((*i)->type == populationObject) {
            currSel.push_back (qSharedPointerDynamicCast<population> (*i));
        }
        ++i;
    }
    return currSel;
}

QSharedPointer<population> rootData::currSelPopulation()
{
    // get the currently selected population
    QSharedPointer<population> currSel;
    if (this->selList.size() == 1) {
        if (this->selList[0]->type == populationObject) {
            currSel = qSharedPointerDynamicCast<population> (this->selList[0]);
        }
    }
    return currSel;
}

void rootData::updateLayoutPar()
{
    // get the currently selected population
    QSharedPointer <population> currSel = this->currSelPopulation();

    if (currSel.isNull()) {
        return;
    }

    int type = sender()->property("type").toInt();
    switch (type) {
    case 0:
    {
        QDoubleSpinBox * source = (QDoubleSpinBox *) sender();
        CHECK_CAST(dynamic_cast<QDoubleSpinBox *> (sender()))
        QSharedPointer<NineMLLayoutData> layout = currSel->layoutType;
        if (layout->minimumDistance != source->value())
            currProject->undoStack->push(new updateLayoutMinDist(this,layout,source->value()));
        break;
    }
    case 1:
    {
        QSpinBox * source = (QSpinBox *) sender();
        CHECK_CAST(dynamic_cast<QSpinBox *> (sender()))
        QSharedPointer<NineMLLayoutData> layout = currSel->layoutType;
        if (layout->seed != source->value())
            currProject->undoStack->push(new updateLayoutSeed(this,layout,source->value()));
        break;
    }
    }
}

void rootData::setSize()
{
    // get the currently selected population
    QSharedPointer <population> currSel = this->currSelPopulation();

    if (currSel == NULL) {
        return;
    }

    // get value
    int value = ((QSpinBox *) sender())->value();
    CHECK_CAST(dynamic_cast<QSpinBox *>(sender()))

    // only update if we have a change
    if (value != currSel->numNeurons) {
        currProject->undoStack->push(new setSizeUndo(this, currSel, value));
    }
}

void rootData::setLoc3()
{
    // get the currently selected population
    QSharedPointer <population> currSel = this->currSelPopulation();

    if (currSel == NULL) {
        return;
    }

    int index = sender()->property("type").toInt();
    int value = ((QSpinBox *) sender())->value();
    CHECK_CAST(dynamic_cast<QSpinBox *>(sender()))

    currProject->undoStack->push(new setLoc3Undo(this, currSel, index, value));
}

void rootData::renamePopulation()
{
    // get the currently selected population
    QSharedPointer <population> currSel = this->currSelPopulation();

    if (currSel == NULL) {
        return;
    }

    // get the title label so we can update it with the new name
    QLabel * titleLabel = (QLabel *) sender()->property("ptrTitle").value<void *>();
    CHECK_CAST(dynamic_cast<QLabel *>(titleLabel))

    // get the rename box so we can get the new title (it isn't necessarily the sender)
    QLineEdit * renameBox = (QLineEdit *) sender()->property("ptrRename").value<void *>();
    CHECK_CAST(dynamic_cast<QLineEdit *>(renameBox))

    QString newName = renameBox->text();
    // check name is unique
    QString finalName = this->getUniquePopName(newName);
    if (finalName.compare(newName) != 0) {
            QMessageBox msgBox;
            msgBox.setText("A population exists with that name, adding a number");
            msgBox.exec();
    }

    // update name (undo-able)
    if (finalName != currSel->name) {
        titleLabel->setText("<u><b>" + finalName + "</b></u>");
        currProject->undoStack->push(new updateTitle(currSel, finalName, currSel->name));
    }

    // redraw view
    emit redrawGLview();
}


QString rootData::getUniquePopName(QString newName)
{
    // are we the selected pop, or not?
    int ind = 0;
    if (selList.size() == 0) {
        ind = -1;
    } else if (selList.size() == 1) {
        for (int i = 0; i < populations.size(); ++i) {
            if (populations[i]->getName() == selList[0]->getName()) ind = i;
        }
    }

    // is the root name used? If not we can just pass it back
    for (int i = 0; i < this->populations.size(); ++i) {
        if (this->populations[i]->name.compare(newName) == 0 && (int) i != ind) {

            // name in use - find the lowest number we can add to make a unique name
            bool nameGood = false;
            int j = 0;
            while (!nameGood) {
                // assume name is good
                nameGood = true;
                ++j;
                QString testName = newName;
                testName.append(" " + QString::number(j));

                // check name against populations
                for (int k = 0; k < this->populations.size(); ++k) {
                    if (ind == -1) {
                        if (this->populations[k]->name.compare(testName) == 0) {
                            nameGood = false;
                        }
                    } else {
                        if (this->populations[k]->name.compare(testName) == 0 && populations[i]->getName() != selList[0]->getName()) {
                            nameGood = false;
                        }
                    }
                }
            }
            // we have the name - we can exit
            newName.append(" " + QString::number(j));
        }
    }
    return newName;
}



int rootData::getIndex()
{
    ++(this->largestIndex);
    return this->largestIndex;
}

void rootData::changeSynapse()
{
    QPushButton * source = (QPushButton *) sender();
    CHECK_CAST(dynamic_cast<QPushButton *>(sender()))
    QString dir = source->property("direction").toString();

    // get the currently selected projection
    QSharedPointer <projection> currSel;
    if (this->selList.size() == 1) {
        if (this->selList[0]->type == projectionObject) {
            currSel = qSharedPointerDynamicCast<projection> (this->selList[0]);
        }
    }

    if (currSel.isNull()) {
        return;
    }

    // change the current Synapse of the projection accordingly

    int currTarg = currSel->currTarg;
    if (dir.compare("left") == 0) {
        if (currTarg == 0) {
            currTarg = currSel->synapses.size() - 1;
        } else {
            --currTarg;
        }

    } else if (dir.compare("right") == 0) {
        if (currTarg == (int) currSel->synapses.size() - 1) {
            currTarg = 0;
        } else {
            ++currTarg;
        }

    } else if (dir.compare("add") == 0) {
        // add the Synapse
        currProject->undoStack->push(new addSynapse(this, currSel));

    } else if (dir.compare("rem") == 0) {
        // remove the Synapse
        currProject->undoStack->push(new delSynapse(this, currSel, currSel->synapses[currTarg]));
    }
    currSel->currTarg = currTarg;

    // force complete redraw of the parameters panels;
    emit updatePanel(this);
}

void rootData::selectColour()
{
    // get the currently selected population
    QSharedPointer <population> currSel = this->currSelPopulation();

    if (currSel == NULL) {
        return;
    }

    // change the current Synapse of the projection accordingly
    currSel->colour = this->getColor(currSel->colour);

    // redraw GL
    emit redrawGLview();
}

void rootData::getNeuronLocationsSrc(QVector <QVector <loc> > *locations,QVector <QColor> * cols, QString name)
{
    QVector <loc> tempLoc;
    cols->clear();

    if (name == "") {

        if (this->selList.size() == 1) {

            if (this->selList[0]->type == populationObject) {

                QSharedPointer <population> pop = qSharedPointerDynamicCast<population> (selList[0]);

                if (pop->layoutType->component->name == "none") {

                    for (int i = 0; i < locations->size(); ++i) {
                        locations[i].clear();
                    }
                    locations->clear();

                    // linear layout by default:
                    for (int i = 0; i < (int) pop->numNeurons; ++i) {

                        loc newLoc;
                        //do a square:
                        newLoc.x = i%10;
                        newLoc.y = floor(float(i) / 10.0);
                        newLoc.z = 0;
                        tempLoc.push_back(newLoc);
                    }

                    locations->push_back(tempLoc);
                    cols->push_back(pop->colour);

                } else {

                    QSharedPointer <population> pop = qSharedPointerDynamicCast<population> (selList[0]);

                    locations->clear();
                    cols->push_back(pop->colour);

                    // generate the locations!
                    QString err = "";
                    locations->resize(1);
                    pop->layoutType->generateLayout(pop->numNeurons, &((*locations)[0]), err);
                    if (err != "") {
                        emit statusBarUpdate(err, 2000);
                    }
                    return;
                }
            }
        }

    } else {

        // find what has that name, and send back the details
        for (int ind = 0; ind < this->populations.size(); ++ind) {
            if (this->populations[ind]->name == name) {

                if (this->populations[ind]->layoutType->component->name == "none") {

                    for (int i = 0; i < locations->size(); ++i) {
                        locations[i].clear();
                    }
                    locations->clear();

                    // linear layout by default:
                    for (int i = 0; i < (int) this->populations[ind]->numNeurons; ++i) {
                        loc newLoc;
                        //do a square:
                        newLoc.x = i%10;
                        newLoc.y = floor(float(i) / 10.0);
                        newLoc.z = 0;
                        tempLoc.push_back(newLoc);
                    }
                    locations->push_back(tempLoc);
                    cols->push_back(this->populations[ind]->colour);
                    return;

                } else {

                    for (int i = 0; i < locations->size(); ++i) {
                        locations[i].clear();
                    }
                    locations->clear();
                    cols->push_back(this->populations[ind]->colour);
                    // generate the locations!
                    QString err = "";
                    locations->resize(1);
                    this->populations[ind]->layoutType->generateLayout(this->populations[ind]->numNeurons, &((*locations)[0]), err);
                    if (err != "") {
                        emit statusBarUpdate(err, 2000);
                    }
                    return;
                }
            }

            for (int cInd = 0; cInd < this->populations[ind]->projections.size(); ++cInd) {

                if (this->populations[ind]->projections[cInd]->getName() == name) {

                    for (int i = 0; i < locations->size(); ++i) {
                        locations[i].clear();
                    }
                    locations->clear();

                    // generate src and dst locations:
                    // SOURCE
                    QString err = "";
                    locations->resize(2);
                    if (this->populations[ind]->projections[cInd]->source->layoutType->component->name != "none") {
                        this->populations[ind]->projections[cInd]->source->layoutType->generateLayout(this->populations[ind]->projections[cInd]->source->numNeurons, &((*locations)[0]), err);
                        cols->push_back(this->populations[ind]->projections[cInd]->source->colour);
                    } else {
                        // linear layout by default:
                        tempLoc.clear();
                        for (int i = 0; i < (int) this->populations[ind]->projections[cInd]->source->numNeurons; ++i) {
                            loc newLoc;
                            //do a square:
                            newLoc.x = i%10;
                            newLoc.y = floor(float(i) / 10.0);
                            newLoc.z = 0;
                            tempLoc.push_back(newLoc);
                        }
                        (*locations)[0] = tempLoc;
                        cols->push_back(this->populations[ind]->projections[cInd]->source->colour);
                    }
                    if (err != "") {
                        emit statusBarUpdate(err, 2000);
                    }

                    // DESTINATION
                    if (this->populations[ind]->projections[cInd]->destination->layoutType->component->name != "none") {
                        this->populations[ind]->projections[cInd]->destination->layoutType->generateLayout(this->populations[ind]->projections[cInd]->destination->numNeurons, &((*locations)[1]), err);
                        cols->push_back(this->populations[ind]->projections[cInd]->destination->colour);
                    } else {
                        // linear layout by default:
                        tempLoc.clear();
                        for (int i = 0; i < (int) this->populations[ind]->projections[cInd]->destination->numNeurons; ++i) {

                            loc newLoc;

                            //do a square:
                            newLoc.x = i%10;
                            newLoc.y = floor(float(i) / 10.0);
                            newLoc.z = 0;
                            tempLoc.push_back(newLoc);

                        }
                        (*locations)[1] = tempLoc;
                        cols->push_back(this->populations[ind]->projections[cInd]->destination->colour);
                    }
                    if (err != "") {
                        emit statusBarUpdate(err, 2000);
                    }
                    return;
                }
            }
        }
    }
}

QSharedPointer<systemObject> rootData::getObjectFromName(QString name)
{
    QSharedPointer<systemObject> currObject = (QSharedPointer<systemObject>)0;

    // find the pop / projection that is being displayed
    for (int i = 0; i < this->populations.size(); ++i) {
        if (this->populations[i]->getName() == name) {
            currObject = this->populations[i];
            break;
        }
        for (int j = 0; j < this->populations[i]->projections.size(); ++j) {
            if (this->populations[i]->projections[j]->getName() == name) {
                currObject = this->populations[i]->projections[j];
                break;
            }
        }
    }

    return currObject;
}


// allow safe usage of systemObject pointers
/*bool rootData::isValidPointer(systemObject * ptr)
{
    // find the pop / projection / input reference
    for (int i = 0; i < this->populations.size(); ++i) {

        if (this->populations[i].data() == ptr) {
            return true;
        }

        for (int j = 0; j < this->populations[i]->neuronType->inputs.size(); ++j)
            if (this->populations[i]->neuronType->inputs[j].data() == ptr) {
                return true;
            }

        for (int j = 0; j < this->populations[i]->projections.size(); ++j) {

            if (this->populations[i]->projections[j].data() == ptr) {
                return true;
            }

            for (int k = 0; k < this->populations[i]->projections[j]->synapses.size(); ++k) {
                if (this->populations[i]->projections[j]->synapses[k].data() == ptr) {
                    return true;
                }
                for (int l = 0; l < this->populations[i]->projections[j]->synapses[k]->weightUpdateType->inputs.size(); ++l) {
                    if (this->populations[i]->projections[j]->synapses[k]->weightUpdateType->inputs[l].data() == ptr) {
                        return true;
                    }
                }

                for (int l = 0; l < this->populations[i]->projections[j]->synapses[k]->postsynapseType->inputs.size(); ++l) {
                    if (this->populations[i]->projections[j]->synapses[k]->postsynapseType->inputs[l].data() == ptr) {
                        return true;
                    }
                }
            }
        }
    }

    // not found
    return false;
}*/

QSharedPointer<systemObject> rootData::isValidPointer(systemObject * ptr)
{
    // find the pop / projection / input reference
    for (int i = 0; i < this->populations.size(); ++i) {

        if (this->populations[i].data() == ptr) {
            return this->populations[i];
        }

        for (int j = 0; j < this->populations[i]->neuronType->inputs.size(); ++j)
            if (this->populations[i]->neuronType->inputs[j].data() == ptr) {
                return this->populations[i]->neuronType->inputs[j];
            }

        for (int j = 0; j < this->populations[i]->projections.size(); ++j) {

            if (this->populations[i]->projections[j].data() == ptr) {
                return this->populations[i]->projections[j];
            }

            for (int k = 0; k < this->populations[i]->projections[j]->synapses.size(); ++k) {
                if (this->populations[i]->projections[j]->synapses[k].data() == ptr) {
                    return this->populations[i]->projections[j]->synapses[k];
                }
                for (int l = 0; l < this->populations[i]->projections[j]->synapses[k]->weightUpdateType->inputs.size(); ++l) {
                    if (this->populations[i]->projections[j]->synapses[k]->weightUpdateType->inputs[l].data() == ptr) {
                        return this->populations[i]->projections[j]->synapses[k]->weightUpdateType->inputs[l];
                    }
                }

                for (int l = 0; l < this->populations[i]->projections[j]->synapses[k]->postsynapseType->inputs.size(); ++l) {
                    if (this->populations[i]->projections[j]->synapses[k]->postsynapseType->inputs[l].data() == ptr) {
                        return this->populations[i]->projections[j]->synapses[k]->postsynapseType->inputs[l];
                    }
                }
            }
        }
    }

    // not found
    QSharedPointer<systemObject> null;
    return null;
}

// allow safe usage of NineMLComponentData pointers
QSharedPointer<NineMLComponentData> rootData::isValidPointer(NineMLComponentData * ptr)
{
    // find the reference
    for (int i = 0; i < this->populations.size(); ++i) {

        if (this->populations[i]->neuronType.data() == ptr) {
            return this->populations[i]->neuronType;
        }

        for (int j = 0; j < this->populations[i]->projections.size(); ++j) {
            for (int k = 0; k < this->populations[i]->projections[j]->synapses.size(); ++k) {
                if (this->populations[i]->projections[j]->synapses[k]->weightUpdateType.data() == ptr) {
                    return this->populations[i]->projections[j]->synapses[k]->weightUpdateType;
                }
                if (this->populations[i]->projections[j]->synapses[k]->postsynapseType.data() == ptr) {
                    return this->populations[i]->projections[j]->synapses[k]->postsynapseType;
                }
            }
        }
    }

    // not found
    QSharedPointer<NineMLComponentData> null;
    return null;
}

// allow safe usage of NineMLComponent pointers
QSharedPointer<NineMLComponent> rootData::isValidPointer(NineMLComponent * ptr)
{
    qDebug() << ptr;
    qDebug() << "/////";

    for (int i = 0; i < this->catalogNrn.size(); ++i) {
        qDebug() << catalogNrn[i].data();
        if (catalogNrn[i] == ptr) {
            return catalogNrn[i];
        }
    }
    for (int i = 0; i < this->catalogPS.size(); ++i) {
        qDebug() << catalogPS[i].data();
        if (catalogPS[i]  == ptr) {
            return catalogPS[i];
        }
    }
    for (int i = 0; i < this->catalogUnsorted.size(); ++i) {
        qDebug() << catalogUnsorted[i].data();
        if (catalogUnsorted[i]  == ptr) {
            return catalogUnsorted[i];
        }
    }
    for (int i = 0; i < this->catalogWU.size(); ++i) {
        qDebug() << catalogWU[i].data();
        if (catalogWU[i] == ptr) {
            return catalogWU[i];
        }
    }

    // not found
    QSharedPointer<NineMLComponent> null;
    return null;
}

void rootData::setSelectionbyName(QString name)
{
    QSharedPointer<systemObject> currObject = (QSharedPointer<systemObject>)0;

    // find the pop / projection that is being displayed
    for (int i = 0; i < this->populations.size(); ++i) {

        if (this->populations[i]->getName() == name) {
            currObject = this->populations[i];
            break;
        }

        for (int j = 0; j < this->populations[i]->projections.size(); ++j) {
            if (this->populations[i]->projections[j]->getName() == name) {
                currObject = this->populations[i]->projections[j];
                break;
            }
        }
    }

    this->selList.clear();
    this->selList.push_back(currObject);
}

void rootData::addgenericInput()
{
    // input text
    QString text = ((QLineEdit *) sender())->text();
    CHECK_CAST(dynamic_cast<QLineEdit *>(sender()))
    QSharedPointer <NineMLComponentData> src;

    // find source:
    for (int i = 0; i < this->populations.size(); ++i) {
        if (this->populations[i]->neuronType->getXMLName() == text) {
            src = this->populations[i]->neuronType;
        }
        for (int j = 0; j < this->populations[i]->projections.size(); ++j) {
            for (int k = 0; k < this->populations[i]->projections[j]->synapses.size(); ++k) {
                if (this->populations[i]->projections[j]->synapses[k]->weightUpdateType->getXMLName() == text) {
                    src = this->populations[i]->projections[j]->synapses[k]->weightUpdateType;
                }
                if (this->populations[i]->projections[j]->synapses[k]->postsynapseType->getXMLName() == text) {
                    src = this->populations[i]->projections[j]->synapses[k]->postsynapseType;
                }
            }
        }
    }

    if (!src.isNull()) {

        NineMLComponentData * dst = (NineMLComponentData *) sender()->property("ptr").value<void *>();

        // check it is valid
        QSharedPointer<NineMLComponentData> dstShr = isValidPointer(dst);

        if (dstShr.isNull()) {
            qDebug() << "Found a bad pointer in addGenericInput";
            exit(0);
        }

        // disconnect so we don't get multiples
        sender()->disconnect((QObject *) 0);

        // add the genericInput
        currProject->undoStack->push(new addInput(this, src, dstShr));

        // redraw panel
        emit updatePanel(this);
        emit redrawGLview();

    } else {
        // src not found - set the LineEdit background red-ish
        QPalette p = ((QLineEdit *) sender())->palette();
        CHECK_CAST(dynamic_cast<QLineEdit *>(sender()))
        p.setColor( QPalette::Normal, QPalette::Base, QColor(255, 200, 200) );
        ((QLineEdit *) sender())->setPalette(p);
    }
}

void rootData::delgenericInput()
{
    genericInput * ptr = (genericInput *) sender()->property("ptr").value<void *>();

    QSharedPointer <genericInput> ptrShr = qSharedPointerDynamicCast<genericInput> (this->isValidPointer(ptr));

    if (ptrShr.isNull()) {
        qDebug() << "Found a bad pointer in delGenericInput";
        exit(0);
    }

    // delete the genericInput
    currProject->undoStack->push(new delInput(this, ptrShr));
    emit updatePanel(this);
}

void rootData::editConnections()
{
    // launch the list editor dialog
    csv_connection * conn = (csv_connection *) sender()->property("ptr").value<void *>();
    CHECK_CAST(dynamic_cast<csv_connection *>(conn))
    connectionListDialog * dialog  = new connectionListDialog(conn);
    connect(dialog,SIGNAL(completed()), this, SLOT(reDrawAll()));
    dialog->show();
}

void rootData::setTitle()
{
    emit setWindowTitle();
}

void rootData::setModelTitle(QString model_name)
{
    currProject->undoStack->push(new updateModelTitle(this, model_name, currProject));
    setCaptionOut(model_name);
}

void rootData::setCaptionOut(QString model_name)
{
    // update version and name caption
    emit setCaption(model_name + " <i>" + "</i>");
}

void rootData::undoOrRedoPerformed(int)
{
    emit redrawGLview();
    setCaptionOut(this->currProject->name);
    // update file list for components
    emit setWindowTitle();
    emit updatePanel(this);
    qDebug() << "Here";
}

void rootData::copyParsToClipboard()
{
    // safety
    if (selList.size() == 1) {

        // if population
        if (selList[0]->type == populationObject) {
            if (sender()->property("source").toString() == "tab1") {
                if (clipboardCData.isNull()) {
                    clipboardCData.clear();
                }
                QSharedPointer <population> pop = qSharedPointerDynamicCast<population> (selList[0]);
                clipboardCData = QSharedPointer<NineMLComponentData> (new NineMLComponentData(pop->neuronType));
            }
        }

        // if projection
        if (selList[0]->type == projectionObject) {
            if (sender()->property("source").toString() == "tab1") {
                if (clipboardCData.isNull()) {
                    clipboardCData.clear();
                }
                QSharedPointer <projection> proj = qSharedPointerDynamicCast<projection> (selList[0]);
                clipboardCData = QSharedPointer<NineMLComponentData> (new NineMLComponentData(proj->synapses[proj->currTarg]->weightUpdateType));
            }
            if (sender()->property("source").toString() == "tab2") {
                if (clipboardCData.isNull()) {
                    clipboardCData.clear();
                }
                QSharedPointer <projection> proj = qSharedPointerDynamicCast<projection> (selList[0]);
                clipboardCData = QSharedPointer<NineMLComponentData> (new NineMLComponentData(proj->synapses[proj->currTarg]->postsynapseType));
            }
        }
    }
    this->reDrawAll();
}

void rootData::pasteParsFromClipboard()
{
    // safety
    if (selList.size() == 1) {

        // if population
        if (selList[0]->type == populationObject) {
            if (sender()->property("source").toString() == "tab1") {
                QSharedPointer <population> pop = qSharedPointerDynamicCast<population> (selList[0]);
                currProject->undoStack->push(new pastePars(this,clipboardCData,pop->neuronType));
            }
        }

        // if projection
        if (selList[0]->type == projectionObject) {
            QSharedPointer <projection> proj = qSharedPointerDynamicCast<projection> (selList[0]);
            if (sender()->property("source").toString() == "tab1") {
                currProject->undoStack->push(new pastePars(this,clipboardCData,(proj->synapses[proj->currTarg]->weightUpdateType)));
            }
            if (sender()->property("source").toString() == "tab2") {
                currProject->undoStack->push(new pastePars(this,clipboardCData,(proj->synapses[proj->currTarg]->postsynapseType)));
            }
        }
    }
}
