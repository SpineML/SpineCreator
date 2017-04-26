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

#include "SC_viewELexptpanelhandler.h"
#include "SC_connectionmodel.h"
#include "SC_network_3d_visualiser_panel.h"
#include "SC_network_layer_rootdata.h"
#include "SC_network_layer_rootlayout.h"
#include "SC_layout_aliaseditdialog.h"
#include "SC_layout_editpreviewdialog.h"
#include "mainwindow.h"
#include "CL_classes.h"
#include "EL_experiment.h"
#include "SC_projectobject.h"
#include "SC_undocommands.h"
#include "qmessageboxresizable.h"
#include <QTimer>


#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
  #define RETINA_SUPPORT 1.0
#else
  #ifdef WIN_HIDPI_FIX
  #define RETINA_SUPPORT WIN_DPI_SCALING
  #else
  #define RETINA_SUPPORT 1.0
  #endif
#endif

#define NEW_EXPERIMENT_VIEW11

viewELExptPanelHandler::viewELExptPanelHandler(QObject *parent) :
    QObject(parent)
{
}

viewELExptPanelHandler::viewELExptPanelHandler(viewELstruct * viewEL, nl_rootdata * data, QObject *parent) :
    QObject(parent)
{

    this->data = data;
    this->viewEL = viewEL;
    this->exptSetup = new QVBoxLayout;
    this->exptInputs = new QVBoxLayout;
    this->exptOutputs = new QVBoxLayout;
    this->exptChanges = new QVBoxLayout;
    this->cursor = QPointF(0,0);

    // visual experiments test code - all looks good but not right now...
#ifdef NEW_EXPERIMENT_VIEW
    gl = new GLWidget;
    connect(gl, SIGNAL(reDraw(QPainter*,float,float,float,int,int,drawStyle)),this,SLOT(reDrawModel(QPainter*,float,float,float,int,int,drawStyle)));
    connect(gl, SIGNAL(mouseMove(float,float)), this, SLOT(mouseMove(float,float)));
    connect(gl, SIGNAL(onLeftMouseDown(float,float,float, bool)), this, SLOT(selectByMouseDown(float,float,float)));

    ((QHBoxLayout *) this->viewEL->expt->layout())->addWidget(gl);

    ((QHBoxLayout *) this->viewEL->expt->layout())->setContentsMargins(0,0,0,0);

    // for animation
    QTimer *timer = new QTimer( this );
    // this creates a Qt timer event
    connect( timer, SIGNAL(timeout()), this->gl, SLOT(animate()) );
    // launch the timer
    timer->start(16);
#endif

    this->exptSetup->setContentsMargins(14*RETINA_SUPPORT,14*RETINA_SUPPORT,14*RETINA_SUPPORT,14*RETINA_SUPPORT);
    this->exptInputs->setContentsMargins(4*RETINA_SUPPORT,4*RETINA_SUPPORT,4*RETINA_SUPPORT,4*RETINA_SUPPORT);
    this->exptOutputs->setContentsMargins(4*RETINA_SUPPORT,4*RETINA_SUPPORT,4*RETINA_SUPPORT,4*RETINA_SUPPORT);
    this->exptChanges->setContentsMargins(4*RETINA_SUPPORT,4*RETINA_SUPPORT,4*RETINA_SUPPORT,4*RETINA_SUPPORT);

    // add panel to expts
    ((QHBoxLayout *) this->viewEL->expt->layout())->addLayout(this->exptSetup);
#ifndef NEW_EXPERIMENT_VIEW
    // add divider
    QFrame* lineA = new QFrame();
    lineA->setMaximumWidth(1*RETINA_SUPPORT);
    lineA->setFrameShape(QFrame::VLine);
    lineA->setFrameShadow(QFrame::Plain);
    ((QHBoxLayout *) this->viewEL->expt->layout())->addWidget(lineA);

    // add panel to expts
    ((QHBoxLayout *) this->viewEL->expt->layout())->addLayout(this->exptInputs);

    // add divider
    QFrame* lineB = new QFrame();
    lineB->setMaximumWidth(1*RETINA_SUPPORT);
    lineB->setFrameShape(QFrame::VLine);
    lineB->setFrameShadow(QFrame::Plain);
    ((QHBoxLayout *) this->viewEL->expt->layout())->addWidget(lineB);

    // add panel to expts
    ((QHBoxLayout *) this->viewEL->expt->layout())->addLayout(this->exptOutputs);

    // add divider
    QFrame* lineC = new QFrame();
    lineC->setMaximumWidth(1*RETINA_SUPPORT);
    lineC->setFrameShape(QFrame::VLine);
    lineC->setFrameShadow(QFrame::Plain);
    ((QHBoxLayout *) this->viewEL->expt->layout())->addWidget(lineC);

    // add panel to expts
    ((QHBoxLayout *) this->viewEL->expt->layout())->addLayout(this->exptChanges);

#endif
    // panel is part of struct viewELstruct and is a QWidget:
    this->viewEL->panel->setStyleSheet("QWidget { background-color: white; }");

    redrawPanel();
    redrawExpt();

}

void viewELExptPanelHandler::recursiveDeleteLoop(QLayout * parentLayout)
{
    QLayoutItem * item;
    while ((item = parentLayout->takeAt(0))) {
        if (item->widget()) {
            if (item->widget()->property("noDelete")!=true) {
                item->widget()->disconnect((QObject *)0);
                item->widget()->hide();
                connect(this, SIGNAL(deleteWidgets()),item->widget(), SLOT(deleteLater()));
            }
        }
        if (item->layout())
            recursiveDeleteLoop(item->layout());
        delete item;
    }
    connect(this, SIGNAL(deleteWidgets()),parentLayout, SLOT(deleteLater()));
}

void viewELExptPanelHandler::recursiveDelete(QLayout * parentLayout)
{
    QLayoutItem * item;
    while ((item = parentLayout->takeAt(2))) {
        if (item->widget()) {
            if (item->widget()->property("noDelete")!=true) {
                item->widget()->disconnect((QObject *)0);
                item->widget()->hide();
                connect(this, SIGNAL(deleteWidgets()),item->widget(), SLOT(deleteLater()));
            }
        }
        if (item->layout())
            recursiveDeleteLoop(item->layout());
        delete item;
    }
}


void viewELExptPanelHandler::recursiveDeleteExpt(QLayout * parentLayout)
{
    QLayoutItem * item;
    while ((item = parentLayout->takeAt(0))) { // invalid index 0
        if (item->widget()) {
            if (item->widget()->property("noDelete")!=true) {
                item->widget()->disconnect((QObject *)0);
                item->widget()->hide();
                connect(this, SIGNAL(deleteWidgets()),item->widget(), SLOT(deleteLater()));
            }
        }
        if (item->layout()) {
            recursiveDeleteLoop(item->layout());
        }
        delete item;
    }
}

void viewELExptPanelHandler::redraw()
{
    redrawPanel();
    redrawExpt();
    // update title in case of undo / redo so the * accurately reflects unsaved changes
    this->data->main->updateTitle();
}

void viewELExptPanelHandler::redraw(int)
{
    redrawPanel();
    redrawExpt();
    // update title in case of undo / redo so the * accurately reflects unsaved changes
    this->data->main->updateTitle();
}

void viewELExptPanelHandler::redraw(double)
{
    redrawPanel();
    redrawExpt();
    // update title in case of undo / redo so the * accurately reflects unsaved changes
    this->data->main->updateTitle();
}

void viewELExptPanelHandler::redrawSimulatorParams(experiment * currentExperiment)
{
    QFont titleFont("Helvetica [Cronyx]", 16);

    QLabel * title;
    title = new QLabel(tr("Setup Simulator"));
    title->setFont(titleFont);
    exptSetup->addWidget(title);

    // redraw SIMULATOR SETUP

    QFormLayout * formSim = new QFormLayout;
    exptSetup->addLayout(formSim);

    //Simulator
    QComboBox * engine = new QComboBox;

    // load existing simulators:
    QSettings settings;

    settings.beginGroup("simulators");
    QStringList sims = settings.childGroups();
    for (int i = 0; i < sims.size(); ++i) {
        engine->addItem(sims[i]);
    }
    settings.endGroup();

    for (int i = 0; i < engine->count(); ++i) {
        engine->setCurrentIndex(i);
        if (engine->currentText() == currentExperiment->setup.simType)
            break;
    }
    connect(engine, SIGNAL(currentIndexChanged(QString)), this, SLOT(changedEngine(QString)));
    formSim->addRow("Engine:",engine);

    //duration
    QDoubleSpinBox * dur = new QDoubleSpinBox;
    dur->setSuffix(" s");
    dur->setMaximum(100000);
    dur->setMinimum(0.01);
    dur->setValue(currentExperiment->setup.duration);
    connect(dur, SIGNAL(editingFinished()), this, SLOT(changedDuration()));
    formSim->addRow("Duration:",dur);

    //timestep
    QDoubleSpinBox * dt = new QDoubleSpinBox;
    dt->setValue(currentExperiment->setup.dt);
    dt->setSuffix(" ms");
    connect(dt, SIGNAL(editingFinished()), this, SLOT(changedDt()));
    formSim->addRow("Time-step:",dt);

    //solver
    QComboBox * solver = new QComboBox;
    solver->addItem("Forward Euler",(int) ForwardEuler);
    //solver->addItem("Runge Kutta",(int) RungeKutta);
    //solver->addItem("DAMSON");
    for (int i = 0; i < solver->count(); ++i) {
        solver->setCurrentIndex(i);
        if (((solverType) solver->itemData(i).toInt()) == currentExperiment->setup.solver)
            break;
    }
    connect(solver, SIGNAL(currentIndexChanged(int)), this, SLOT(changedSolver(int)));
    formSim->addRow("Solver:",solver);

    //solver order
    if (currentExperiment->setup.solver == RungeKutta) {
        QSpinBox * solverOrd = new QSpinBox;
        solverOrd->setMinimum(2);
        solverOrd->setMaximum(4);
        solverOrd->setValue(currentExperiment->setup.solverOrder);
        connect(solverOrd, SIGNAL(valueChanged(int)), this, SLOT(changedSolverOrder(int)));
        formSim->addRow("Solver order:",solverOrd);
    }

}

void viewELExptPanelHandler::redrawExpt()
{
    int vert_scroll = 0;
    int horiz_scroll = 0;

    QScrollArea * scroll = this->viewEL->propertiesScrollArea;

    if (scroll) {
        // store the scroll location so we can replace it later
        horiz_scroll = scroll->horizontalScrollBar()->value();
        vert_scroll = scroll->verticalScrollBar()->value();
        //qDebug() << horiz_scroll << vert_scroll;
    }

    emit this->deleteWidgets();

    recursiveDeleteExpt(exptSetup);
    recursiveDeleteExpt(exptInputs);
    recursiveDeleteExpt(exptOutputs);
    recursiveDeleteExpt(exptChanges);

    experiment * currentExperiment = NULL;

    // find currentExperiment
    for (int i = 0; i < data->experiments.size(); ++i) {
        if (data->experiments[i]->selected) {currentExperiment = data->experiments[i]; break;}
    }

    if (currentExperiment == NULL) return;

    if (currentExperiment->editing)
        return;

    // check if we have any edits going on - if we do then disable the Run Simulator button
    bool edit_going_on = false;
    for (int i = 0; i < currentExperiment->ins.size(); ++i) {
        if (currentExperiment->ins[i]->edit == true) edit_going_on = true;
    }
    for (int i = 0; i < currentExperiment->outs.size(); ++i) {
        if (currentExperiment->outs[i]->edit == true) edit_going_on = true;
    }
    for (int i = 0; i < currentExperiment->lesions.size(); ++i) {
        if (currentExperiment->lesions[i]->edit == true) edit_going_on = true;
    }
    for (int i = 0; i < currentExperiment->gilesions.size(); ++i) {
        if (currentExperiment->gilesions[i]->edit == true) {
            edit_going_on = true;
        }
    }
    for (int i = 0; i < currentExperiment->changes.size(); ++i) {
        if (currentExperiment->changes[i]->edit == true) edit_going_on = true;
    }
    if (edit_going_on) {
        currentExperiment->subEdit = true;
    } else {
        currentExperiment->subEdit = false;
    }
    this->redrawPanel();


#ifdef NEW_EXPERIMENT_VIEW
    // if pointer is not valid it is either NULL or the selected object was deleted
    if (this->data->isValidPointer(this->currSystemObject) == false) {
        // no selection, so draw up the simulator parameters to the panel
        this->redrawSimulatorParams(currentExperiment);
    }

    // we are referencing a valid and not deleted object
    if (this->data->isValidPointer(this->currSystemObject) == true) {
        // check if we have a POPULATION
        if (this->currSystemObject->type == populationObject) {
            // ok, this is a population - draw up the population panel
            // cast to a population
            QSharedPointer <population> pop = (QSharedPointer <population>) this->currSystemObject;
            // set up some fonts to use
            QFont titleFont("Helvetica [Cronyx]", 16);
            QFont sub1Font("Helvetica [Cronyx]", 14);
            QFont sub2Font("Helvetica [Cronyx]", 12);
            // write out some details about this population
            // the name
            QLabel * name;
            name = new QLabel("<b>" + pop->name + "</b>");
            name->setFont(titleFont);
            exptSetup->addWidget(name);
            // the size
            QLabel * size;
            size = new QLabel("Size = " + QString::number(pop->size));
            size->setFont(sub1Font);
            exptSetup->addWidget(size);
            // check if we have a Component on the population
            if (pop->neuronType->component->name != "none") {
                // the component
                QLabel * comp;
                comp = new QLabel("Component is '" + pop->neuronType->component->name + "'");
                comp->setFont(sub1Font);
                exptSetup->addWidget(comp);
                // First put up the input ports that we can send inputs to.
                // Then put up the loggable output ports.
                // Then allow variable overrides.
            } else {
                // no Component, so say that is the issue
                QLabel * comp;
                comp = new QLabel(tr("No component selected"));
                comp->setFont(sub1Font);
                exptSetup->addWidget(comp);
            }
        }
    }

    // add a stretch to force the content to the top
    exptSetup->addStretch();

#else

    // redraw SIMULATOR PARAMETERS
    redrawSimulatorParams(currentExperiment);

    // redraw MODEL INPUTS

    QVBoxLayout * formIn = new QVBoxLayout;
    exptInputs->addLayout(formIn);

    // existing
    for (int i = 0; i < currentExperiment->ins.size(); ++i) {
        exptInput * in = currentExperiment->ins[i];
        formIn->addLayout(in->drawInput(this->data, this));
    }

    //add new input
    QPushButton * addIn = new QPushButton("Add Input");
    addIn->setIcon(QIcon(":/icons/toolbar/addShad.png"));
    addIn->setFlat(true);
    addIn->setFocusPolicy(Qt::NoFocus);
    addIn->setToolTip("Add a new experimental input to the current model");
    addIn->setFont(addFont);
    // grey out if editing:
    for (int i=0; i < currentExperiment->ins.size(); ++i)
        if (currentExperiment->ins[i]->edit)
            addIn->setDisabled(true);

    connect(addIn, SIGNAL(clicked()), this, SLOT(addInput()));

    formIn->addWidget(addIn);



    // redraw MODEL OUTPUTS

    QVBoxLayout * formOut = new QVBoxLayout;
    exptOutputs->addLayout(formOut);

    // existing
    for (int i = 0; i < currentExperiment->outs.size(); ++i) {
        exptOutput * out = currentExperiment->outs[i];
        formOut->addLayout(out->drawOutput(this->data, this));
    }

    // add new output
    QPushButton * addOut = new QPushButton("Add Output");
    addOut->setIcon(QIcon(":/icons/toolbar/addShad.png"));
    addOut->setFlat(true);
    addOut->setFocusPolicy(Qt::NoFocus);
    addOut->setToolTip("Add a new experimental output to the current model");
    addOut->setFont(addFont);
    // grey out if editing:
    for (int i=0; i < currentExperiment->outs.size(); ++i)
        if (currentExperiment->outs[i]->edit)
            addOut->setDisabled(true);

    connect(addOut, SIGNAL(clicked()), this, SLOT(addOutput()));

    formOut->addWidget(addOut);

    // redraw CHANGES

    // Lesions:
    QVBoxLayout * formLesion = new QVBoxLayout;
    exptChanges->addLayout(formLesion);
    formLesion->addWidget(new QLabel("Lesions"));
    // existing lesions
    for (int i = 0; i < currentExperiment->lesions.size(); ++i) {
        exptLesion * out = currentExperiment->lesions[i];
        formLesion->addLayout(out->drawLesion(this->data, this));
    }
    // add new lesion button
    QPushButton * addLesion = new QPushButton("Add Lesion");
    addLesion->setIcon(QIcon(":/icons/toolbar/addShad.png"));
    addLesion->setFlat(true);
    addLesion->setFocusPolicy(Qt::NoFocus);
    addLesion->setToolTip("Add a new lesion to the current model");
    addLesion->setFont(addFont);
    // grey out if editing:
    for (int i=0; i < currentExperiment->lesions.size(); ++i) {
        if (currentExperiment->lesions[i]->edit) {
            addLesion->setDisabled(true);
        }
    }
    connect(addLesion, SIGNAL(clicked()), this, SLOT(addLesion()));
    formLesion->addWidget(addLesion);

    // Generic Input Lesions (This provides a separate UI for GI
    // lesions, though it COULD be part of the same UI).
    QVBoxLayout * formGILesion = new QVBoxLayout;
    exptChanges->addLayout(formGILesion);
    formGILesion->addWidget(new QLabel("Generic Input Lesions"));
    // existing gi lesions
    for (int i = 0; i < currentExperiment->gilesions.size(); ++i) {
        exptGenericInputLesion * out = currentExperiment->gilesions[i];
        formGILesion->addLayout(out->drawLesion(this->data, this));
    }
    // add new gi lesion button
    QPushButton * addGILesion = new QPushButton("Add Generic Input Lesion");
    addGILesion->setIcon(QIcon(":/icons/toolbar/addShad.png"));
    addGILesion->setFlat(true);
    addGILesion->setFocusPolicy(Qt::NoFocus);
    addGILesion->setToolTip("Add a new generic input lesion to the current model");
    addGILesion->setFont(addFont);
    // grey out if editing:
    for (int i=0; i < currentExperiment->gilesions.size(); ++i) {
        if (currentExperiment->gilesions[i]->edit) {
            addGILesion->setDisabled(true);
        }
    }
    connect(addGILesion, SIGNAL(clicked()), this, SLOT(addGILesion()));
    formGILesion->addWidget(addGILesion);

    // Property changes:
    QVBoxLayout * formPropChanges = new QVBoxLayout;
    exptChanges->addLayout(formPropChanges);
    formPropChanges->addWidget(new QLabel("Changed properties"));
    // existing
    for (int i = 0; i < currentExperiment->changes.size(); ++i) {
        exptChangeProp * out = currentExperiment->changes[i];
        formPropChanges->addLayout(out->drawChangeProp(this->data, this));
    }
    // add new par change
    QPushButton * addChange = new QPushButton("Add Property Change");
    addChange->setIcon(QIcon(":/icons/toolbar/addShad.png"));
    addChange->setFlat(true);
    addChange->setFocusPolicy(Qt::NoFocus);
    addChange->setToolTip("Add a changed property to the current model");
    addChange->setFont(addFont);
    // grey out if editing:
    for (int i=0; i < currentExperiment->changes.size(); ++i) {
        if (currentExperiment->changes[i]->edit) {
            addChange->setDisabled(true);
        }
    }
    connect(addChange, SIGNAL(clicked()), this, SLOT(addChangedProp()));
    formPropChanges->addWidget(addChange);

    exptSetup->addStretch();
    exptInputs->addStretch();
    exptOutputs->addStretch();
    exptChanges->addStretch();

    if (scroll) {
        // replace scroll bar location (currently does not work)
        scroll->horizontalScrollBar()->setValue(horiz_scroll);
        scroll->verticalScrollBar()->setValue(vert_scroll);
    }
#endif
}

void viewELExptPanelHandler::redrawPanel()
{
    QVBoxLayout * panel = ((QVBoxLayout *) viewEL->panel->layout());

    // clear panel except toolbar
    emit this->deleteWidgets();
    recursiveDelete(panel);

    // add strech to avoid layout spacing out items
    panel->addStretch();

    // add button
    QPushButton * add = new QPushButton("Add experiment");
    add->setIcon(QIcon(":/icons/toolbar/addShad.png"));
    add->setFlat(true);
    add->setFocusPolicy(Qt::NoFocus);
    add->setToolTip("Add a new experiment to the current model");
    add->setFont(addFont);
    // grey out if editing:
    for (int i=0; i < data->experiments.size(); ++i)
        if (data->experiments[i]->editing)
            add->setDisabled(true);

    connect(add, SIGNAL(clicked()), this, SLOT(addExperiment()));

    panel->insertWidget(2, add);

    // add experiments
    for (int i = data->experiments.size()-1; i >= 0; --i) {
        exptBox * box = data->experiments[i]->getBox(this);
        box->setProperty("index", i);
        connect(box, SIGNAL(clicked()), this, SLOT(changeSelection()));
        panel->insertWidget(2, box);
    }
}

void viewELExptPanelHandler::addExperiment()
{
    QVBoxLayout * panel = ((QVBoxLayout *) viewEL->panel->layout());

    // add experiment to list
    data->experiments.push_back(new experiment);

    // select the new experiment
    exptBox * layout = data->experiments.back()->getBox(this);
    panel->insertWidget(3, layout);
    data->experiments.back()->select(&(data->experiments));

    data->experiments.back()->editing = true;

    // disable run button
    emit enableRun(false);

    // redraw to show the new experiment
    redrawPanel();
    redrawExpt();
}

void viewELExptPanelHandler::delExperiment()
{
    int index = sender()->property("index").toUInt();

    // check we aren't running - don't delete if we are
    if (data->experiments[index] == this->runExpt) {
        return;
    }

    data->experiments.erase(data->experiments.begin() + index);

    // if we delete the last experiment then disable the run button
    if (data->experiments.size() == 0) {
        emit enableRun(false);
    }

    // redraw to updata the selection
    redrawPanel();
    redrawExpt();
}

void viewELExptPanelHandler::moveExperiment()
{
    int index = sender()->property("index").toUInt();
    int type = sender()->property("type").toUInt();

    experiment * tempExperiment;

    switch (type)
    {
    case 0: // up

        tempExperiment = data->experiments[index];
        data->experiments.erase(data->experiments.begin() + index);
        data->experiments.insert(data->experiments.begin() + index+1, tempExperiment);
        break;

    case 1: // down

        tempExperiment = data->experiments[index];
        data->experiments.erase(data->experiments.begin() + index);
        data->experiments.insert(data->experiments.begin() + index-1, tempExperiment);
        break;
    }

    // redraw to updata the selection
    redrawPanel();
}

void viewELExptPanelHandler::editExperiment()
{
    int index = sender()->property("index").toUInt();

    data->experiments[index]->editing = true;

    // enable to run button
    emit enableRun(false);

    // redraw to update the editBox
    redrawPanel();
    redrawExpt();
}

void viewELExptPanelHandler::doneEditExperiment()
{
    int index = sender()->property("index").toUInt();

    // update the experiment
    QLineEdit * titleEdit = (QLineEdit *) sender()->property("title").value<void *>();
    data->experiments[index]->name = titleEdit->text();

    QTextEdit * descEdit = (QTextEdit *) sender()->property("desc").value<void *>();
    data->experiments[index]->description = descEdit->toPlainText();

    data->experiments[index]->editing = false;

    // enable to run button
    emit enableRun(true);

    // redraw to update the editBox
    redrawPanel();
    redrawExpt();
}

void viewELExptPanelHandler::cancelEditExperiment()
{
    int index = sender()->property("index").toUInt();

    data->experiments[index]->editing = false;

    // enable to run button
    emit enableRun(true);

    // redraw to update the editBox
    redrawPanel();
    redrawExpt();
}

void viewELExptPanelHandler::changeSelection()
{
    int index = sender()->property("index").toUInt();

    // also make sure that the run button is enabled
    emit enableRun(true);

    // check if this is the selected box - if it is then do nothing
    if (data->experiments[index]->selected) {
        return;
    }

    // otherwise select it
    data->experiments[index]->select(&(data->experiments));

    // Initialise the viewGV for the experiment if necessary
    this->main->initViewGV (data->experiments[index]);

    // Update the experiment menu
    this->main->setExperimentMenu();

    redrawPanel();
    redrawExpt();
}


void viewELExptPanelHandler::changedEngine(QString sim)
{
    experiment * currentExperiment = NULL;

    // find currentExperiment
    for (int i = 0; i < data->experiments.size(); ++i) {
        if (data->experiments[i]->selected) {currentExperiment = data->experiments[i]; break;}
    }

    if (currentExperiment == NULL) return;

    currentExperiment->setup.simType = sim;

    // Log file location will have changed if the engine changed.
    this->main->updateDatas();

    redrawExpt();
}

void viewELExptPanelHandler::changedDt()
{
    experiment * currentExperiment = NULL;

    // find currentExperiment
    for (int i = 0; i < data->experiments.size(); ++i) {
        if (data->experiments[i]->selected) {currentExperiment = data->experiments[i]; break;}
    }

    if (currentExperiment == NULL) return;

    currentExperiment->setup.dt = ((QDoubleSpinBox *) sender())->value();;
}

void viewELExptPanelHandler::changedDuration()
{
    experiment * currentExperiment = NULL;

    // find currentExperiment
    for (int i = 0; i < data->experiments.size(); ++i) {
        if (data->experiments[i]->selected) {currentExperiment = data->experiments[i]; break;}
    }

    if (currentExperiment == NULL) return;

    currentExperiment->setup.duration = ((QDoubleSpinBox *) sender())->value();
}

void viewELExptPanelHandler::changedSolver(int index)
{
    experiment * currentExperiment = NULL;

    // find currentExperiment
    for (int i = 0; i < data->experiments.size(); ++i) {
        if (data->experiments[i]->selected) {currentExperiment = data->experiments[i]; break;}
    }

    if (currentExperiment == NULL) return;

    solverType type = (solverType) ((QComboBox *) sender())->itemData(index).toInt();

    currentExperiment->setup.solver = type;

    redrawExpt();
}

void viewELExptPanelHandler::changedSolverOrder(int val)
{
    experiment * currentExperiment = NULL;

    // find currentExperiment
    for (int i = 0; i < data->experiments.size(); ++i) {
        if (data->experiments[i]->selected) {currentExperiment = data->experiments[i]; break;}
    }

    if (currentExperiment == NULL) return;

    currentExperiment->setup.solverOrder = val;

    redrawExpt();
}

void viewELExptPanelHandler::addInput()
{
    experiment * currentExperiment = NULL;

    // find currentExperiment
    for (int i = 0; i < data->experiments.size(); ++i) {
        if (data->experiments[i]->selected) {currentExperiment = data->experiments[i]; break;}
    }

    if (currentExperiment == NULL) return;

    currentExperiment->ins.push_back(new exptInput);

    redrawExpt();
}

void viewELExptPanelHandler::setInputName()
{
    exptInput * in = (exptInput *) sender()->property("ptr").value<void *>();
    in->name = ((QLineEdit *) sender())->text();
}

void viewELExptPanelHandler::setInputComponent()
{
    // input text
    QString text = ((QLineEdit *) sender())->text();
    QSharedPointer <ComponentInstance> src = (QSharedPointer <ComponentInstance>)0;

    // find source:
    for (int i = 0; i < data->populations.size(); ++i) {
        if (data->populations[i]->neuronType->getXMLName() == text) {
            src = data->populations[i]->neuronType;
        }
        for (int j = 0; j < data->populations[i]->projections.size(); ++j) {
            for (int k = 0; k < data->populations[i]->projections[j]->synapses.size(); ++k) {
                if (data->populations[i]->projections[j]->synapses[k]->weightUpdateCmpt->getXMLName() == text) {
                    src = data->populations[i]->projections[j]->synapses[k]->weightUpdateCmpt;
                }
                if (data->populations[i]->projections[j]->synapses[k]->postSynapseCmpt->getXMLName() == text) {
                    src = data->populations[i]->projections[j]->synapses[k]->postSynapseCmpt;
                }
            }
        }
    }

    exptInput * in = (exptInput *) sender()->property("ptr").value<void *>();

    if (src != (QSharedPointer <ComponentInstance>)0) {
        // if no change...
        if (in->target == src)
            return;
        // else
        in->target = src;
        // old port is no longer valid
        in->portName = "";
        in->set = true;
        QPalette p = ((QLineEdit *) sender())->palette();
        p.setColor( QPalette::Normal, QPalette::Base, QColor(200, 255, 200) );
        ((QLineEdit *) sender())->setPalette(p);
        // UI changes required so hit up a redraw
        // disconnect sender so it doesn't flip out when it loses focus
        sender()->disconnect((QObject *)0);

        redrawExpt();
    } else {
        in->set = false;
        QPalette p = ((QLineEdit *) sender())->palette();
        p.setColor( QPalette::Normal, QPalette::Base, QColor(255, 200, 200) );
        ((QLineEdit *) sender())->setPalette(p);
    }
}

void viewELExptPanelHandler::setInputPort(int index)
{
    exptInput * in = (exptInput *) sender()->property("ptr").value<void *>();

    // assign the port stored with the currentIndex onto the input
    Port * port = (Port *) ((QComboBox *) sender())->itemData(index).value<void *>();
    in->portName = port->name;
    in->portIsAnalog = port->isAnalog();

    // disconnect sender so it doesn't flip out when it loses focus
    sender()->disconnect((QObject *)0);

    redrawExpt();
}

void viewELExptPanelHandler::setInputType(int index)
{
    exptInput * in = (exptInput *) sender()->property("ptr").value<void *>();

    // assign the port stored with the currentIndex onto the input
    in->inType = (exptInType) ((QComboBox *) sender())->itemData(index).toInt();

    in->params.clear();

    // disconnect sender so it doesn't flip out when it loses focus
    sender()->disconnect((QObject *)0);

    redrawExpt();
}

void viewELExptPanelHandler::setInputRateDistributionType(int index)
{
    exptInput * in = (exptInput *) sender()->property("ptr").value<void *>();

    if (index == 0)
        in->rateDistribution = Regular;
    else if (index == 1)
        in->rateDistribution = Poisson;

    redrawExpt();
}

void viewELExptPanelHandler::setInputRateSeed(int theseed)
{
    exptInput * in = (exptInput *) sender()->property("ptr").value<void *>();
    in->rateSeed = theseed;
    // Don't redraw the expt on this signal, as that would take focus
    // from the widget whilst the user may be entering text. Instead,
    // call redrawExpt on the editingFinished signal.
}

void viewELExptPanelHandler::editingFinishedRateSeed(void)
{
    // Editing finished, so now redraw the experiment. Need this
    // wrapper function as it is a public slot, and redrawExpt is
    // private.
    redrawExpt();
}

void viewELExptPanelHandler::reorderParams (QVector <float>& params)
{
    QVector <float> tempVec;
    while (params.size() > 1) {
        int min = 100000000;
        int minIndex = 0;
        for (int i = 0; i < params.size(); i+=2) {
            if (params[i] < min) {min = params[i]; minIndex = i;}
        }
        for (int i = 0; i < 2; ++i) {
            tempVec.push_back(params[minIndex]);
            params.erase(params.begin()+minIndex);
        }
    }
    params.swap(tempVec);
}

void viewELExptPanelHandler::acceptInput()
{
    exptInput * in = (exptInput *) sender()->property("ptr").value<void *>();

    // Auto re-order params at this point.
    if (in->inType == timevarying) {

        // reorder in->params, which is a vector of floats.
        this->reorderParams (in->params);

    } else if (in->inType == arrayTimevarying) {

        QVector <float> curr; // a single table from in->params goes in here to be sorted
        QVector <float> temp; // a sorted copy of in->params is built in here
        curr.clear();
        temp.clear();

        int index = 0;
        int i = 0;
        while (i < in->params.size()) {
            if (in->params[i] == -1) { // it should be
                index = in->params[++i]; // get the table index from the next entry
                ++i; // and then move on to the first table entry
                while (i < in->params.size() && in->params[i] != -1) { // read the current table
                    curr.push_back(in->params[i++]);
                }
                if (!curr.empty()) {
                    this->reorderParams (curr); // sort
                    temp.push_back(-1);
                    temp.push_back(index);
                    //temp.insert(temp.end(), curr.begin(), curr.end());  // append
                    temp += curr;
                    curr.clear(); // clear ready for the next table read
                }
            } // else corrupt data somewhere? This depends on time never being -1. Negative time not allowed.
        }

        // Swap the (now sorted) contents of temp into in->params
        in->params.swap (temp);
    }

    // Mark that we're no longer editing this input
    in->edit = false;

    redrawExpt();
}

void viewELExptPanelHandler::editInput()
{
    exptInput * in = (exptInput *) sender()->property("ptr").value<void *>();
    in->edit = true;

    redrawExpt();
}

void viewELExptPanelHandler::delInput()
{
    experiment * currentExperiment = NULL;

    // find currentExperiment
    for (int i = 0; i < data->experiments.size(); ++i) {
        if (data->experiments[i]->selected) {currentExperiment = data->experiments[i]; break;}
    }

    if (currentExperiment == NULL) return;

    exptInput * in = (exptInput *) sender()->property("ptr").value<void *>();

    // push the command onto the undo stack
    this->data->currProject->undoStack->push(new deleteInputUndo(this->data, currentExperiment, in));

    redrawExpt();
}

void viewELExptPanelHandler::setInputTypeData(double value)
{
    exptInput * in = (exptInput *) sender()->property("ptr").value<void *>();
    if (in->params.size() != 1) {
        in->params.resize(1);
    }
    in->params[0] = value;
}

void viewELExptPanelHandler::setInputExternalData()
{
    QString type = sender()->property("type").toString();
    exptInput * in = (exptInput *) sender()->property("ptr").value<void *>();

    if (type == "command") {
        in->externalInput.commandline = ((QLineEdit *) sender())->text();
    } else if (type == "port") {
        in->externalInput.port = ((QSpinBox *) sender())->value();
    } else if (type == "size") {
        in->externalInput.size = ((QSpinBox *) sender())->value();
    } else if (type == "timestep") {
        in->externalInput.timestep = ((QDoubleSpinBox *) sender())->value();
    } else if (type == "host") {
        in->externalInput.host = ((QLineEdit *) sender())->text();
    }
}

void viewELExptPanelHandler::setInputParams(int row, int col)
{
    exptInput * in = (exptInput *) sender()->property("ptr").value<void *>();
    QTableWidget * table = (QTableWidget *) sender();

    // find the edited value and alter the back end
    double value = table->currentItem()->data(Qt::DisplayRole).toDouble();

    if (in->inType == timevarying) {

        // construct index and set value
        int index = row*2+col;
        if (index > (int) in->params.size()-1) {
            in->params.resize(qCeil(index/2)*2);
        }

        if (index%2 == 0) {
            // even index; value (time) should be >=0
            if (value < 0) {
                // Error - positive time values required. Force to 0.
                value = 0;
                table->currentItem()->setText("0");
                return;
            }
        }
        in->params[index] = value;

    } else if (in->inType == arrayConstant) {

        in->params[row] = value;

    } else if (in->inType == arrayTimevarying) {

        bool readpast = false;
        QVector <float>::iterator params_iter = in->params.begin();
        QVector <float>::iterator curr_start = in->params.end();
        QVector <float>::iterator curr_end = in->params.end();
        while (params_iter != in->params.end()) {
            if (*params_iter == -1 && readpast) {
                curr_end = params_iter; // Points to one past the last element of the current table.
                readpast = false;
                break;
            }
            if (readpast) { // Expect to skip past a time and a value:
                if (++params_iter == in->params.end()) { break; }
                if (++params_iter == in->params.end()) { continue; }
            }
            if (params_iter != in->params.end() && *params_iter == -1) {
                if (*++params_iter == in->currentIndex) {
                    readpast = true;
                    curr_start = params_iter;
                    curr_start++; // Now at the start of the current index table data (just past the -1 and the index).
                } else {
                    params_iter++; // step past that index, and on to the next.
                }
            } else {
                if (++params_iter == in->params.end()) { break; }
                if (++params_iter == in->params.end()) { continue; }
            }
        }
        if (readpast) {
            // We were "reading" the current table, but didn't get to another table, so need to set end iter
            curr_end = params_iter;
        }

        // construct index and set value in curr.
        int index = row*2+col;
        QVector <float>::iterator index_iter = curr_start;
        index_iter += index;

        // Every other value has to be checked to ensure it's non-negative
        if (index%2 == 0) {
            // even index; value (time) should be >=0
            if (value < 0) {
                // Error - positive time values please! Force to 0.
                value = 0;
                // Trigger a new value in the currentItem, which will re-call this function.
                table->currentItem()->setText("0");
                // Then return.
                return;
            }
        }

        if (index_iter >= curr_end) {
            // need to insert into in->params. This doesn't seem to occur in practice.
            if (col == 1) {
                curr_end = in->params.insert (curr_end, 0.0);
                in->params.insert (curr_end, static_cast<float>(value));
            } else if (col == 0) {
                curr_end = in->params.insert (curr_end, static_cast<float>(value));
                in->params.insert (curr_end, 0.0);
            } else {
                // error
            }
        } else {
            // Can over-write in->params
            *index_iter = static_cast<float>(value);
        }
    }

    redrawExpt();
}

void viewELExptPanelHandler::fillInputParams()
{
    exptInput * in = (exptInput *) sender()->property("ptr").value<void *>();
    QDoubleSpinBox * fillSpin = (QDoubleSpinBox *) sender()->property("ptrSpin").value<void *>();

    if (in->inType == arrayConstant) {
        for (int i = 0; i < in->params.size(); ++i) {
            in->params[i] = fillSpin->value();
        }
    }

    redrawExpt();
}

void viewELExptPanelHandler::changeInputIndex(int index)
{
    exptInput * in = (exptInput *) sender()->property("ptr").value<void *>();
    in->currentIndex = index;
    redrawExpt();
}

void viewELExptPanelHandler::setInputAddTVRow()
{
    exptInput * in = (exptInput *) sender()->property("ptr").value<void *>();

    if (in->inType == timevarying) {
        in->params.resize(qCeil(in->params.size()/2)*2+2);
    }
    if (in->inType == arrayTimevarying) {
        bool found = false;
        int indexIndex = -1;

        // find where to operate on
        for (int i = 0; i < in->params.size(); i+=2) {
            if (in->params[i] == -1 && found) {indexIndex = i; break;}
            if (in->params[i] == -1 && in->params[i+1] == in->currentIndex) {found = true;}
        }

        // insert the row
        if (indexIndex == -1) {
            in->params.push_back(0);
            in->params.push_back(0);
        } else {
            in->params.insert(in->params.begin()+indexIndex-1,2,0);
        }
    }

    redrawExpt();
}

void viewELExptPanelHandler::setInputDelTVRow()
{
    exptInput * in = (exptInput *) sender()->property("ptr").value<void *>();

    if (in->inType == timevarying) {
        in->params.resize(qCeil(in->params.size()/2)*2-2);
    }
    if (in->inType == arrayTimevarying) {
        bool found = false;
        int indexIndex = -1;

        // find where to operate on
        for (int i = 0; i < in->params.size(); i+=2) {
            if (in->params[i] == -1 && found) {indexIndex = i; break;}
            if (in->params[i] == -1 && in->params[i+1] == in->currentIndex) {found = true;}
        }

        // remove the row
        if (indexIndex == -1) {
            in->params.pop_back();
            in->params.pop_back();
        } else {
            in->params.erase(in->params.begin()+indexIndex,in->params.begin()+indexIndex+2);
        }
    }

    redrawExpt();
}

void viewELExptPanelHandler::addOutput()
{
    experiment * currentExperiment = NULL;

    // find currentExperiment
    for (int i = 0; i < data->experiments.size(); ++i) {
        if (data->experiments[i]->selected) {currentExperiment = data->experiments[i]; break;}
    }

    if (currentExperiment == NULL) return;

    currentExperiment->outs.push_back(new exptOutput);

    redrawExpt();
}

void viewELExptPanelHandler::setOutputName()
{
    exptOutput * out = (exptOutput *) sender()->property("ptr").value<void *>();
    out->name = ((QLineEdit *) sender())->text();
}

void viewELExptPanelHandler::setOutputComponent()
{
    // input text
    QString text = ((QLineEdit *) sender())->text();
    QSharedPointer <ComponentInstance> src = (QSharedPointer <ComponentInstance>)0;

    // find source:
    for (int i = 0; i < data->populations.size(); ++i) {
        if (data->populations[i]->neuronType->getXMLName() == text) {
            src = data->populations[i]->neuronType;
        }
        for (int j = 0; j < data->populations[i]->projections.size(); ++j) {
            for (int k = 0; k < data->populations[i]->projections[j]->synapses.size(); ++k) {
                if (data->populations[i]->projections[j]->synapses[k]->weightUpdateCmpt->getXMLName() == text) {
                    src = data->populations[i]->projections[j]->synapses[k]->weightUpdateCmpt;
                }
                if (data->populations[i]->projections[j]->synapses[k]->postSynapseCmpt->getXMLName() == text) {
                    src = data->populations[i]->projections[j]->synapses[k]->postSynapseCmpt;
                }
            }
        }
    }

    exptOutput * out = (exptOutput *) sender()->property("ptr").value<void *>();

    if (src != (QSharedPointer <ComponentInstance>)0) {
        // if no change
        if (out->source == src) {
            return;
        }
        // else...
        out->source = src;
        // old port is no longer valid
        out->portName = "";
        out->set = true;
        QPalette p = ((QLineEdit *) sender())->palette();
        p.setColor( QPalette::Normal, QPalette::Base, QColor(200, 255, 200) );
        ((QLineEdit *) sender())->setPalette(p);
        // UI change so hit up a redraw
        // disconnect sender so it doesn't flip out when it loses focus
        sender()->disconnect((QObject *)0);

        redrawExpt();
    } else {
        qDebug() << "moo2";
        out->set = false;
        QPalette p = ((QLineEdit *) sender())->palette();
        p.setColor( QPalette::Normal, QPalette::Base, QColor(255, 200, 200) );
        ((QLineEdit *) sender())->setPalette(p);
    }
}

void viewELExptPanelHandler::setOutputPort(int index)
{
    exptOutput * out = (exptOutput *) sender()->property("ptr").value<void *>();

    // assign the port stored with the currentIndex onto the input
    Port * port = (Port *) ((QComboBox *) sender())->itemData(index).value<void *>();
    out->portName = port->name;
    out->portIsAnalog = port->isAnalog();

    redrawExpt();
}

void viewELExptPanelHandler::setOutputType()
{
    redrawExpt();
}

void viewELExptPanelHandler::setOutputIndices()
{
    exptOutput * out = (exptOutput *) sender()->property("ptr").value<void *>();
    // sanity check
    QString text = ((QLineEdit *) sender())->text();

    // blank is invalid
    if (text.isEmpty()) {
        ((QLineEdit *) sender())->setText(out->indices);
        return;
    }
    if (text != "all") {
        QString validCharacters("1234567890,");
        for (int i = 0; i < text.size(); ++i) {
            if (!validCharacters.contains(text[i])) {
                // invalid character used
                ((QLineEdit *) sender())->setText(out->indices);
                return;
            }
        }
        QStringList numbers = text.split(',');
        for (int i = 0; i < numbers.size(); ++i) {
            if (numbers[i].isEmpty()) {
                // invalid arrangement used
                ((QLineEdit *) sender())->setText(out->indices);
                return;
            }
        }
    }
    out->indices = text;
}

void viewELExptPanelHandler::setOutputStartT(double t)
{
    exptOutput * out = (exptOutput *) sender()->property("ptr").value<void *>();
    out->startTime = t;
}

void viewELExptPanelHandler::setOutputEndT(double t)
{
    exptOutput * out = (exptOutput *) sender()->property("ptr").value<void *>();
    out->endTime = t;
}

void viewELExptPanelHandler::acceptOutput()
{
    exptOutput * out = (exptOutput *) sender()->property("ptr").value<void *>();
    out->edit = false;

    redrawExpt();
}

void viewELExptPanelHandler::editOutput()
{
    exptOutput * out = (exptOutput *) sender()->property("ptr").value<void *>();
    out->edit = true;

    redrawExpt();
}

void viewELExptPanelHandler::delOutput()
{
    experiment * currentExperiment = NULL;

    // find currentExperiment
    for (int i = 0; i < data->experiments.size(); ++i) {
        if (data->experiments[i]->selected) {currentExperiment = data->experiments[i]; break;}
    }

    if (currentExperiment == NULL) return;

    exptOutput * out = (exptOutput *) sender()->property("ptr").value<void *>();

    // push the command onto the undo stack
    this->data->currProject->undoStack->push(new deleteOutputUndo(this->data, currentExperiment, out));

    redrawExpt();
}

void viewELExptPanelHandler::toggleExternalOutput(bool check)
{
    exptOutput * out = (exptOutput *) sender()->property("ptr").value<void *>();
    out->isExternal = check;
}

void viewELExptPanelHandler::setOutputExternalData()
{
    QString type = sender()->property("type").toString();
    exptOutput * out = (exptOutput *) sender()->property("ptr").value<void *>();

    if (type == "command") {
        out->externalOutput.commandline = ((QLineEdit *) sender())->text();
    } else if (type == "port") {
        out->externalOutput.port = ((QSpinBox *) sender())->value();
    } else if (type == "size") {
        out->externalOutput.size = ((QSpinBox *) sender())->value();
    } else if (type == "timestep") {
        out->externalOutput.timestep = ((QDoubleSpinBox *) sender())->value();
    } else if (type == "host") {
        out->externalOutput.host = ((QLineEdit *) sender())->text();
    }
}

void viewELExptPanelHandler::addLesion()
{
    experiment * currentExperiment = NULL;

    // find currentExperiment
    for (int i = 0; i < data->experiments.size(); ++i) {
        if (data->experiments[i]->selected) {
            currentExperiment = data->experiments[i];
            break;
        }
    }

    if (currentExperiment == NULL) {
        return;
    }

    currentExperiment->lesions.push_back(new exptLesion(currentExperiment));

    redrawExpt();
}

void viewELExptPanelHandler::addGILesion()
{
    // find currentExperiment and add a new generic input lesion
    for (int i = 0; i < data->experiments.size(); ++i) {
        if (data->experiments[i]->selected) {
            data->experiments[i]->gilesions.push_back(new exptGenericInputLesion);
            redrawExpt();
            break;
        }
    }
}

#define NO_GILESION lesionedGI == (QSharedPointer<genericInput>)0
void viewELExptPanelHandler::setGILesion (void)
{
    // input text
    QString text = ((QLineEdit *) sender())->text();
    DBG() << "Input text is: " << text;

    QSharedPointer<genericInput> lesionedGI = (QSharedPointer<genericInput>)0;

    // find the selected genericInput by matching up with the text from the UI
    for (int i = 0; i < data->populations.size() && NO_GILESION; ++i) {
        // First extract dsts from populations themselves:
        QSharedPointer<population> pop = data->populations[i];
        for (int j = 0; j < pop->neuronType->inputs.size(); ++j) { // or ->outputs
            if (pop->neuronType->inputs[j]->getName() == text) {
                lesionedGI = pop->neuronType->inputs[j];
            }
        }

        // Then go through the projections contained within:
        for (int j = 0; j < pop->projections.size() && NO_GILESION; ++j) {
            // For each synapse in projections, test for weightupdate and postsynapse containing inputs
            for (int k = 0; k < pop->projections[j]->synapses.size() && NO_GILESION; ++k) {
                // Find inputs from the weight update and postsynapses on this synapse:
                for (int l = 0; l < pop->projections[j]->synapses[k]->weightUpdateCmpt->inputs.size() && NO_GILESION; ++l) {
                    if (pop->projections[j]->synapses[k]->weightUpdateCmpt->inputs[l]->getName() == text) {
                        lesionedGI = pop->projections[j]->synapses[k]->weightUpdateCmpt->inputs[l];
                    }
                }
                for (int l = 0; l < pop->projections[j]->synapses[k]->postSynapseCmpt->inputs.size() && NO_GILESION; ++l) {
                    if (pop->projections[j]->synapses[k]->postSynapseCmpt->inputs[l]->getName() == text) {
                        lesionedGI = pop->projections[j]->synapses[k]->postSynapseCmpt->inputs[l];
                    }
                }
            }
        }
    }

    exptGenericInputLesion* gilesion = (exptGenericInputLesion*)sender()->property("ptr").value<void *>();
    if (lesionedGI != (QSharedPointer<genericInput>)0) {
        DBG() << "Found GI to lesion which is " << lesionedGI->getName();
        gilesion->gi = lesionedGI;
        gilesion->set = true;
        QPalette p = ((QLineEdit*) sender())->palette();
        p.setColor( QPalette::Normal, QPalette::Base, QColor(200, 255, 200) );
        ((QLineEdit *) sender())->setPalette(p);
    } else {
        gilesion->set = false;
        QPalette p = ((QLineEdit*) sender())->palette();
        p.setColor( QPalette::Normal, QPalette::Base, QColor(255, 200, 200) );
        ((QLineEdit*) sender())->setPalette(p);
    }

    redrawExpt();
}

void viewELExptPanelHandler::setLesionProjection()
{
    // input text
    QString text = ((QLineEdit *) sender())->text();
    QSharedPointer <projection> lesionedProj = (QSharedPointer <projection>)0;

    // find source:
    for (int i = 0; i < data->populations.size(); ++i) {
        for (int j = 0; j < data->populations[i]->projections.size(); ++j) {
            if (data->populations[i]->projections[j]->getName() == text)
                lesionedProj = data->populations[i]->projections[j];
        }
    }

    exptLesion * lesion = (exptLesion *) sender()->property("ptr").value<void *>();

    if (lesionedProj != (QSharedPointer <projection>)0) {
        lesion->proj = lesionedProj;

        lesion->set = true;

        // Here find out if there are any associated genericinput
        // lesions that need to be added to the experiment layer. Find
        // generic inputs which have as their input or output this
        // projection's postsynapse or weightupdate.
        lesion->setAssocGenericInputs();

        QPalette p = ((QLineEdit *) sender())->palette();
        p.setColor( QPalette::Normal, QPalette::Base, QColor(200, 255, 200) );
        ((QLineEdit *) sender())->setPalette(p);
    } else {
        lesion->set = false;
        QPalette p = ((QLineEdit *) sender())->palette();
        p.setColor( QPalette::Normal, QPalette::Base, QColor(255, 200, 200) );
        ((QLineEdit *) sender())->setPalette(p);
    }

    redrawExpt();
}

void viewELExptPanelHandler::acceptLesion()
{
    exptLesion * lesion = (exptLesion *) sender()->property("ptr").value<void *>();
    lesion->edit = false;
    redrawExpt();
}

void viewELExptPanelHandler::editLesion()
{
    exptLesion * lesion = (exptLesion *) sender()->property("ptr").value<void *>();
    lesion->edit = true;
    redrawExpt();
}

void viewELExptPanelHandler::delLesion()
{
    experiment * currentExperiment = NULL;

    // find currentExperiment
    for (int i = 0; i < data->experiments.size(); ++i) {
        if (data->experiments[i]->selected) {currentExperiment = data->experiments[i]; break;}
    }

    if (currentExperiment == NULL) return;

    exptLesion * lesion = (exptLesion *) sender()->property("ptr").value<void *>();

    // push the command onto the undo stack
    this->data->currProject->undoStack->push(new deleteLesionUndo(this->data, currentExperiment, lesion));
    redrawExpt();
}

void viewELExptPanelHandler::acceptGILesion()
{
    exptGenericInputLesion* gilesion = (exptGenericInputLesion*)sender()->property("ptr").value<void*>();
    gilesion->edit = false;
    redrawExpt();
}

void viewELExptPanelHandler::editGILesion()
{
    exptGenericInputLesion* gilesion = (exptGenericInputLesion*)sender()->property("ptr").value<void*>();
    gilesion->edit = true;
    redrawExpt();
}

void viewELExptPanelHandler::delGILesion()
{
    experiment* currentExperiment = NULL;

    // find currentExperiment
    for (int i = 0; i < data->experiments.size(); ++i) {
        if (data->experiments[i]->selected) {
            currentExperiment = data->experiments[i];
            break;
        }
    }

    if (currentExperiment == NULL) {
        return;
    }

    exptGenericInputLesion* gilesion = (exptGenericInputLesion*)sender()->property("ptr").value<void*>();

    // push the command onto the undo stack
    this->data->currProject->undoStack->push(new deleteGILesionUndo(this->data, currentExperiment, gilesion));
    redrawExpt();
}

void viewELExptPanelHandler::setChangeParComponent()
{
    // input text
    QString text = ((QLineEdit *) sender())->text();
    QSharedPointer <ComponentInstance> src = (QSharedPointer <ComponentInstance>)0;

    // find source:
    for (int i = 0; i < data->populations.size(); ++i) {
        if (data->populations[i]->neuronType->getXMLName() == text) {
            src = data->populations[i]->neuronType;
        }
        for (int j = 0; j < data->populations[i]->projections.size(); ++j) {
            for (int k = 0; k < data->populations[i]->projections[j]->synapses.size(); ++k) {
                if (data->populations[i]->projections[j]->synapses[k]->weightUpdateCmpt->getXMLName() == text) {
                    src = data->populations[i]->projections[j]->synapses[k]->weightUpdateCmpt;
                }
                if (data->populations[i]->projections[j]->synapses[k]->postSynapseCmpt->getXMLName() == text) {
                    src = data->populations[i]->projections[j]->synapses[k]->postSynapseCmpt;
                }
            }
        }
    }

    exptChangeProp * changeProp = (exptChangeProp *) sender()->property("ptr").value<void *>();

    if (src != (QSharedPointer <ComponentInstance>)0) {
        changeProp->component = src;
        if (src->ParameterList.size() > 0) {
            changeProp->par = new ParameterInstance(src->ParameterList.front());
        } else if (src->StateVariableList.size() > 0) {
            changeProp->par = new StateVariableInstance(src->StateVariableList.front());
        } else {
            // no pars or statevars, return
            return;
        }
        changeProp->set = true;
        QPalette p = ((QLineEdit *) sender())->palette();
        p.setColor( QPalette::Normal, QPalette::Base, QColor(200, 255, 200) );
        ((QLineEdit *) sender())->setPalette(p);
    } else {
        changeProp->set = false;
        QPalette p = ((QLineEdit *) sender())->palette();
        p.setColor( QPalette::Normal, QPalette::Base, QColor(255, 200, 200) );
        ((QLineEdit *) sender())->setPalette(p);
    }

    // disconnect sender so it doesn't flip out when it loses focus
    sender()->disconnect((QObject *)0);

    redrawExpt();
}

void viewELExptPanelHandler::setChangeParName() {

    exptChangeProp * changeProp = (exptChangeProp *) sender()->property("ptr").value<void *>();

    QLineEdit * name = qobject_cast < QLineEdit * > (sender());

    // this should not happen!
    if (name ==  NULL) return;

    changeProp->name = name->text();

}

void viewELExptPanelHandler::setChangeProp(QString name)
{
    exptChangeProp * changeProp = (exptChangeProp *) sender()->property("ptr").value<void *>();

    for (int i = 0; i < changeProp->component->ParameterList.size(); ++i) {
        if (name == changeProp->component->ParameterList[i]->name) {
            delete changeProp->par;
            changeProp->par = new ParameterInstance(changeProp->component->ParameterList[i]);
        }
    }
    for (int i = 0; i < changeProp->component->StateVariableList.size(); ++i) {
        if (name == changeProp->component->StateVariableList[i]->name) {
            delete changeProp->par;
            changeProp->par = new StateVariableInstance(changeProp->component->StateVariableList[i]);
        }
    }
    redrawExpt();
}

void viewELExptPanelHandler::addChangedProp()
{
    experiment * currentExperiment = NULL;

    // find currentExperiment
    for (int i = 0; i < data->experiments.size(); ++i) {
        if (data->experiments[i]->selected) {currentExperiment = data->experiments[i]; break;}
    }

    if (currentExperiment == NULL) return;

    currentExperiment->changes.push_back(new exptChangeProp);

    redrawExpt();
}

void viewELExptPanelHandler::acceptChangedProp()
{
    exptChangeProp * prop = (exptChangeProp *) sender()->property("ptr").value<void *>();
    prop->edit = false;
    redrawExpt();
}

void viewELExptPanelHandler::editChangedProp()
{
    exptChangeProp * prop = (exptChangeProp *) sender()->property("ptr").value<void *>();
    prop->edit = true;
    redrawExpt();
}

void viewELExptPanelHandler::delChangedProp()
{
    experiment * currentExperiment = NULL;

    // find currentExperiment
    for (int i = 0; i < data->experiments.size(); ++i) {
        if (data->experiments[i]->selected) {currentExperiment = data->experiments[i]; break;}
    }

    if (currentExperiment == NULL) return;

    exptChangeProp * prop = (exptChangeProp *) sender()->property("ptr").value<void *>();

    // push the command onto the undo stack
    this->data->currProject->undoStack->push(new deleteChangePropUndo(this->data, currentExperiment, prop));

    redrawExpt();
}

void viewELExptPanelHandler::run()
{
    QSettings settings;
    settings.setProperty("MERR", QString("False"));

    QToolButton * runButton = qobject_cast < QToolButton * > (sender());
    if (runButton) {
        runButton->disconnect(this);
        runButton->setText("Run experiment");
        QCommonStyle style;
        runButton->setIcon(style.standardIcon(QStyle::SP_MediaStop));
        connect(runButton, SIGNAL(clicked()), this, SLOT(cancelRun()));
    }

    simulatorStdOutText = "";
    simulatorStdErrText = "";

    // fetch current experiment sim engine
    experiment * currentExperiment = NULL;
    int currentExptNum = -1;

    // find currentExperiment
    for (int i = 0; i < data->experiments.size(); ++i) {
        if (data->experiments[i]->selected) {
            currentExperiment = data->experiments[i];
            currentExptNum = i;
            break;
        }
    }

    if (currentExperiment == NULL) {
        this->cleanUpPostRun("", "");
        return;
    }

    this->runExpt = currentExperiment;
    this->runExpt->running = true;

    QString simName = currentExperiment->setup.simType;

    // load path
    settings.beginGroup("simulators/" + simName);
    qDebug() << simName;
    QString path = settings.value("path").toString();
    // Check that path exists and is executable.
    QFile the_script(path);
#ifndef Q_OS_WIN
    if (!the_script.exists()) {
        // Error - convert_script file doesn't exist
        this->cleanUpPostRun("Simulator Error", "The simulator '" + path + "' does not exist.");
        return;
    } else {
        // Path exists, check it's a file and is executable
        // NB: In QT 5.x this would be QFileDevice::ExeOwner etc
        if (the_script.permissions() & (QFile::ExeOwner|QFile::ExeGroup|QFile::ExeOther)) {
            // Probably Ok - we have execute permission of some kind.
        } else {
            // Error - no execute permission on script
            this->cleanUpPostRun("Simulator Error", "The simulator '" + path + "' is not executable.");
            return;
        }
    }
#endif

    // The convert_script takes the working directory as a script argument
    QString wk_dir_string = settings.value("working_dir").toString();
    settings.endGroup();
    wk_dir_string = QDir::toNativeSeparators(wk_dir_string);
#ifdef Q_OS_WIN
    // on windows using Ubuntu BASH we must convert the path
    wk_dir_string = wk_dir_string.replace("\\","/");
    wk_dir_string = wk_dir_string.replace("C:","/mnt/c");
    wk_dir_string = wk_dir_string.replace("D:","/mnt/d");
#endif
    QDir wk_dir(wk_dir_string);

    // clear error message lookup
    this->errorMessages.clear();
    this->errorStrings.clear();

    // try and find an error message lookup
    // FIXME: Need to get this from installed version first, then look in working directory.
    QFile errorMsgLookup(wk_dir.absoluteFilePath("errorMsgLookup.txt"));
    if (errorMsgLookup.open(QIODevice::ReadOnly)) {
        // file exists! start parsing line by line
        QByteArray line = errorMsgLookup.readLine();
        while (!line.isEmpty()) {
            // convert line to string
            QString lineStr(line);
            // separate line by token
            QStringList parts = lineStr.split("#->#");
            // if we have two sides then the line is correctly formatted
            if (parts.size() == 2) {
                // formatted correctly - add to lists
                this->errorStrings.push_back(parts[0]);
                this->errorMessages.push_back(parts[1]);
            }
            // read next line
            line = errorMsgLookup.readLine();
        }
    }

    // set up the environment for the spawned processes
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();

    settings.beginGroup("simulators/" + simName + "/envVar");

    // load environment variable values for the simulator.
    QStringList keysTemp = settings.childKeys();

    for (int i = 0; i < keysTemp.size(); ++i) {
        env.insert(keysTemp[i], settings.value(keysTemp[i]).toString());
    }
    settings.endGroup();

    // add the working dir to the path - this allows Mac users to have the preflight in
    // SpineML_2_BRAHMS
    env.insert("PATH", env.value("PATH", "") + ":" + wk_dir.absolutePath());

    // If the model has changed compared with the one currently saved,
    // then we must write out the current in-memory model to a
    // temporary location and execute that model.
    QString previousFilePath = this->data->currProject->filePath;
    QString tFilePath = this->data->currProject->filePath;

    // In an ideal world, this->data->currProject->isChanged() would
    // reliably give us whether or not a change had been made in the
    // project. Instead, don't rely on that; save a fresh copy of the
    // project out every time.
#ifdef CURRPROJECT_ISCHANGED_WAS_RELIABLE
    if (this->data->currProject->isChanged (this->data)) {
#endif

        #if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
        #else
            this->tdir.setPath(QDir::home.absolutePath());
            this-tdir.mkdir("sctmp");
            this->tdir.cd("sctmp");
        #endif

        // Check the temporary directory is valid for use:

        #if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
                if (!this->tdir.isValid()) {
        #else
                if (!this->tdir.isReadable()) {
        #endif
            qDebug() << "Can't use temporary simulator directory!";
            runButton->setEnabled(true);
            return;
        }

        // clear directory
        QDir model_dir(this->tdir.path());
        QStringList files = model_dir.entryList(QDir::Files);
        for (int i = 0; i < files.size(); ++i) {
            model_dir.remove(files[i]);
        }

        // Write the model into the temporary dir
        tFilePath = this->tdir.path()+ QDir::separator() + "temp.proj";
        settings.setValue("files/currentFileName", tFilePath);
        qDebug() << "Saving project temporarily to: " << tFilePath;
        // save_project changes the current project's filepath.
        if (!this->data->currProject->save_project(tFilePath, this->data)) {
            qDebug() << "Failed to save the model into the temporary model directory";
            this->cleanUpPostRun("Model save error", "The simulation could not be started");
            // Revert currProject->filePath here
            this->data->currProject->filePath = previousFilePath;
            return;
        }
        // Revert currProject->filePath here
        this->data->currProject->filePath = previousFilePath;
#ifdef CURRPROJECT_ISCHANGED_WAS_RELIABLE
    }
#endif

    QProcess * simulator = new QProcess;
    if (!simulator) {
        // Really bad error - memory allocation error. The following will probably fail:
        this->cleanUpPostRun("Memory Allocation Error", "The simulator failed to start - you're out of RAM.");
        return;
    }

    simulator->setWorkingDirectory(wk_dir.absolutePath());
    simulator->setProcessEnvironment(env);

    // This is a project-specific* output directory, stored in a
    // folder in the working directory.
    //
    // *: Actually, it's experiment specific, because if another
    // experiment in the same project has a different simulator, then
    // this path will differ. However, I don't add a _exptN
    // suffix. That said, true experiment-specific data storage would
    // enable easy comparison of two experiments and would be great.
    QString out_dir_name = wk_dir.absolutePath() + QDir::separator() + "temp" + QDir::separator() + data->currProject->getFilenameFriendlyName() + "_e" + QString::number(currentExptNum);

    simulator->setProperty("logpath", out_dir_name + QDir::separator() + "log");

    QFileInfo projFileInfo(tFilePath); // tFilePath contains the path
                                       // to the model being executed,
                                       // either in the original location
                                       // or in the temporary directory.
    QString modelpath(projFileInfo.dir().path());
    {
        QStringList al;
#ifdef Q_OS_WIN
        out_dir_name = wk_dir.absolutePath() + "/temp";
        // on windows using Ubuntu BASH we must convert the path
        modelpath = modelpath.replace("\\","/");
        modelpath = modelpath.replace("C:","/mnt/c");
        modelpath = modelpath.replace("D:","/mnt/d");
        path = QString("cmd.exe /R ") + QString('"') + QString("c:\\WINDOWS\\sysnative\\bash.exe -c '") + path + \
                QString(" -m ") + modelpath + \
                QString(" -w ") + wk_dir.absolutePath() + \
                QString(" -o ") + out_dir_name +\
                QString(" -e ") + QString("%1").arg(currentExptNum) + \
                QString("'") + QString('"')\
                ;
#else
        al << "-m" << modelpath                          // path to input model
           << "-w" << wk_dir.absolutePath()              // path to SpineML_2_BRAHMS dir
           << "-o" << out_dir_name//wk_dir.absolutePath() + QDir::separator() + "temp" // Output dir
           << "-e" << QString("%1").arg(currentExptNum); // The experiment to execute
#endif

        // In settings we set REBUILD - apparently to be an
        // environment variable. However, the way to set
        // convert_script_s2b to rebuild is to pass a -r option.
        settings.beginGroup("simulators/" + simName);
        QString rebuild = settings.value("envVar/REBUILD").toString();
        settings.endGroup();
        if (rebuild == "true") {
            al << "-r"; // Add the -r option for rebuilding components
        } // else don't rebuild

        //path = "C:\\windows\\sysnative\\bash.exe";

       //qDebug() << QProcess::execute(path);

       //path = "notepad.exe";

        QProcess * simulator = new QProcess;
        simulator->setProcessEnvironment(env);

#ifndef Q_OS_WIN
        simulator->start(path,al);
#else
        //path = "cmd.exe";
        qDebug() << path;
        simulator->start(path);

#endif
    }

    // Wait a couple of seconds for the process to start
#ifndef Q_OS_WIN
    if (!simulator->waitForStarted(1000)) {
        // Error - simulator failed to start
        this->cleanUpPostRun("Simulator Error", "The simulator '" + path + "' failed to start.");
        //delete simulator; // Alex: this appears to be dangerous - we can have the wait for started fail, without the simulator crashing
                            // - in which case deleting the simulator causes a crash later on... for this reason I have left it as a memory leak
        return;
    }
#endif

#ifdef Q_OS_WIN
    settings.beginGroup("simulators/" + simName);
    out_dir_name  = settings.value("working_dir").toString() + QDir::separator() + "temp";
    settings.endGroup();
    this->logpath = out_dir_name + QDir::separator() + "log";
#endif

    connect(simulator, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(simulatorFinished(int, QProcess::ExitStatus)));
    connect(simulator, SIGNAL(readyReadStandardOutput()), this, SLOT(simulatorStandardOutput()));
    connect(simulator, SIGNAL(readyReadStandardError()), this, SLOT(simulatorStandardError()));

    // now start a timer to check on the simulation progress
    connect(&simTimeChecker, SIGNAL(timeout()), this, SLOT(checkForSimTime()));
    this->simTimeMax = currentExperiment->setup.duration;

    this->simTimeFileName = QDir::toNativeSeparators(out_dir_name + QDir::separator() + "model" + QDir::separator() + "time.txt");
    QFile::remove(simTimeFileName);
    this->simCancelFileName = QDir::toNativeSeparators(out_dir_name + QDir::separator() + "model" + QDir::separator() + "stop.txt");
    simTimeChecker.start(17);

}

/*!
 * \brief viewELExptPanelHandler::cleanUpPostRun
 * \param msg
 * \param msgDetail
 * If a run ends due to being aborted (user or code) or successfully we have to do some clean up - this
 * was duplicated code so it is now merged into this function. Can pass an optional error message and
 * message detail.
 */
void viewELExptPanelHandler::cleanUpPostRun(QString msg, QString msgDetail) {
    if (!msg.isEmpty()) {
        QMessageBox msgBox;
        msgBox.setWindowTitle(msg);
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setText(msgDetail);
        msgBox.exec();
    }
    if (this->runExpt) {
        if (this->runExpt->runButton) {
            this->runExpt->runButton->disconnect(this);
            this->runExpt->runButton->setText("Run experiment");
            QCommonStyle style;
            this->runExpt->runButton->setIcon(style.standardIcon(QStyle::SP_MediaPlay));
            connect(this->runExpt->runButton, SIGNAL(clicked()), this, SLOT(run()));
        }
        this->runExpt->running = false;
        this->runExpt = NULL;
    }
}

/*!
 * \brief viewELExptPanelHandler::cancelRun
 * This function writes a file that can then be picked up by compatible simulators.
 * The presence of the file causes the simulator to stop running and save logs.
 */
void viewELExptPanelHandler::cancelRun() {

    if (!runExpt) return;

    QFile simCancelFile(simCancelFileName);

    simCancelFile.open(QFile::WriteOnly);
    simCancelFile.close();

#ifdef Q_OS_WIN
    Sleep(2000);
    this->simulatorFinished(0,QProcess::NormalExit);
#endif

}

/*!
 * \brief viewELExptPanelHandler::checkForSimTime
 * This function is used to pick up the infromation left by a simulator, and
 * use it to provide progress feedback to the user. This is done via access to a
 * file called 'time.txt' which is currently located in the simulation model folder.
 *
 */
void viewELExptPanelHandler::checkForSimTime() {

    if (!runExpt) return;

    QFile simTimeFile(simTimeFileName);

    simTimeFile.open(QFile::ReadOnly);

    if (simTimeFile.isOpen()) {
        float simTimeCurr = 0;
        QTextStream tStream(&simTimeFile);
        QString line = tStream.readLine();
        if (line.contains("*")) {
            if (runExpt->runButton) {
                runExpt->runButton->setText(line);
            }
        } else {
            simTimeCurr = line.toFloat();
            // update the UI progress bar
            if (runExpt->runButton) {
                runExpt->runButton->setText("Running: " + QString::number(simTimeCurr) + "ms");
                float proportion = simTimeCurr / (this->simTimeMax*1000);
                runExpt->progressBar->setStyleSheet(QString("QLabel {background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0 rgba(150, 255, 150, 255), ") \
                                       + QString("stop:") + QString::number(proportion) + QString(" rgba(150, 255, 150, 255), stop:")  + QString::number(proportion+0.01) + QString(" rgba(150, 255, 150, 0), stop:1 rgba(255, 255, 255, 0))}"));

            }
        }
#ifdef Q_OS_WIN
    // check if we have finished...
    if (simTimeCurr > this->simTimeMax*1000-0.2) {
        Sleep(2000);
        this->simulatorFinished(0,QProcess::NormalExit);
    }
#endif
        simTimeFile.close();
    }
}

void viewELExptPanelHandler::simulatorFinished(int, QProcess::ExitStatus status)
{
    // stop updating the bar
    simTimeChecker.disconnect();
    simTimeChecker.stop();
    QFile::remove(simCancelFileName);

    // find currentExperiment (could make use of MainWindow::getCurrentExpt)
    experiment* currentExperiment = (experiment*)0;
    for (int i = 0; i < data->experiments.size(); ++i) {
        if (data->experiments[i]->selected) {currentExperiment = data->experiments[i]; break;}
    }
    if (currentExperiment == (experiment*)0) {
        this->cleanUpPostRun("", "");
        return;
    }

    float proportion = 0;
    if (currentExperiment->progressBar) {
        currentExperiment->progressBar->setStyleSheet(QString("QLabel {background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0 rgba(150, 255, 150, 0), ") \
                               + QString("stop:") + QString::number(proportion) + QString(" rgba(150, 255, 150, 0), stop:")  + QString::number(proportion+0.01) + QString(" rgba(150, 255, 150, 0), stop:1 rgba(255, 255, 255, 0))}"));
    }

    // check for errors we can present
    for (int i = 0; i < (int) errorStrings.size(); ++i) {

        // check if error there
        if (simulatorStdOutText.contains(errorStrings[i])) {
            // error found
            QMessageBoxResizable msgBox;
            msgBox.setWindowTitle("Simulator Error Report");
            msgBox.setIcon(QMessageBox::Critical);
            msgBox.setText(errorMessages[i]);
            msgBox.setDetailedText(simulatorStdOutText);
            msgBox.addButton(QMessageBox::Ok);
            msgBox.setDefaultButton(QMessageBox::Ok);
            msgBox.exec();
            this->cleanUpPostRun("", "");
            return;
        }
    }

    // collect logs
#ifndef Q_OS_WIN
    // Update the view of the logfiles
    QDir logs(sender()->property("logpath").toString());
#else
    QDir logs(this->logpath);
#endif

    QStringList filter;
    filter << "*.xml";
    logs.setNameFilters(filter);

    // add logs to graphs
    // First ensure viewGV[currentExperiment] exists. if not, do nothing?
    if (main->existsViewGV(currentExperiment)) {
        DBG() << "viewGV exists for currentExperiment; updating logdata etc.";
        data->main->viewGV[currentExperiment]->properties->populateVLogData (logs.entryList(), &logs);

        // and insert logs into visualiser
        if (data->main->viewVZ.OpenGLWidget != NULL) {
            data->main->viewVZ.OpenGLWidget->addLogs(&data->main->viewGV[currentExperiment]->properties->vLogData);
        }
    } else {
        DBG() << "viewGV didn't exist for currentExperiment, so not updating logdata etc.";
    }

    // get status
    if (status == QProcess::CrashExit) {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Simulator Crash");
        msgBox.setText(simulatorStdErrText);
        msgBox.addButton(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.exec();
        this->cleanUpPostRun("", "");
        return;
    }

    if (status == QProcess::NormalExit) {
        // check if we are running in batch mode (i.e. run button is not set)
        if (this->runExpt->runButton) {
            QMessageBoxResizable msgBox;
            msgBox.setWindowTitle("Simulator Complete");
            msgBox.setIcon(QMessageBox::Information);
            msgBox.setText("Simulator has finished. See below for more details.");
            msgBox.setDetailedText(simulatorStdOutText);
            msgBox.addButton(QMessageBox::Ok);
            msgBox.setDefaultButton(QMessageBox::Ok);
            msgBox.exec();
        }
        // signal others
        this->cleanUpPostRun("", "");
        emit simulationDone();
        return;
    }

    this->cleanUpPostRun("", "");
}

void viewELExptPanelHandler::simulatorStandardOutput()
{
    QByteArray data = ((QProcess *) sender())->readAllStandardOutput();
    simulatorStdOutText = simulatorStdOutText + QString().fromUtf8(data);
}

void viewELExptPanelHandler::simulatorStandardError()
{
    QByteArray data = ((QProcess *) sender())->readAllStandardError();
    simulatorStdOutText = simulatorStdOutText + QString().fromUtf8(data);
}

void viewELExptPanelHandler::mouseMove(float xGL, float yGL)
{
    // move viewpoint
    // first get a pointer the the GLWidget
    GLWidget * source = (GLWidget *) sender();
    // now update the widget location to the new offset
    source->move(xGL+source->viewX-cursor.x(),yGL-source->viewY-cursor.y());
}

void viewELExptPanelHandler::reDrawModel(QPainter* painter,float GLscale, float viewX, float viewY, int width, int height, drawStyle style)
{
    // draw the populations
    for (int i = 0; i < this->data->populations.size(); ++i) {
        this->data->populations[i]->draw(painter, GLscale, viewX, viewY, width, height, this->data->popImage, style);
    }

    // draw the synapses
    for (int i = 0; i < this->data->populations.size(); ++i) {
        this->data->populations[i]->drawSynapses(painter, GLscale, viewX, viewY, width, height, style);
    }

    // draw the generic inputs
    for (int i = 0; i < this->data->populations.size(); ++i) {
        QPen pen(QColor(100,0,0,100));
        pen.setWidthF(float(1));
        painter->setPen(pen);
        this->data->populations[i]->drawInputs(painter, GLscale, viewX, viewY, width, height, style);
    }

    // redraw selected object to highlight, if it is not deleted pointer:
    if (this->currSystemObject->type == populationObject) {
        QSharedPointer <population> pop = qSharedPointerDynamicCast <population> (this->currSystemObject);

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
                painter->drawRect(rectangle);
            }
        }
    }

    if (this->currSystemObject->type == projectionObject) {
        QSharedPointer <projection> col = qSharedPointerDynamicCast <projection> (this->currSystemObject);
        for (int i = 5; i > 1; --i) {
            QPen pen(QColor(0,0,0,30/i));
            pen.setWidthF(float(i*2));
            painter->setPen(pen);
            col->draw(painter, GLscale, viewX, viewY, width, height, this->data->popImage, standardDrawStyle);
        }
    }

}

void viewELExptPanelHandler::selectByMouseDown(float xGL, float yGL, float GLScale)
{
    // store the location selected for use when dragging around the viewport
    this->cursor = QPointF(xGL,yGL);

    // A list of things which have been selected with this left mouse
    // down click. Will be added to this->selList after the logic in
    // this method.
    QVector <QSharedPointer<systemObject> > newlySelectedList;
    this->data->findSelection (xGL, yGL, GLScale, newlySelectedList);

    // if we have an object selected
    if (newlySelectedList.size() == 1) {
        // we have selected a new object - set the current selection to this object if it is a Population or a Projection
        if (newlySelectedList[0]->type == populationObject || newlySelectedList[0]->type == projectionObject) {
            this->currSystemObject = newlySelectedList[0];
            // update the UI
            gl->redrawGLview();
            this->redraw();
        }
    } else {
        this->currSystemObject.clear();
        // update the UI
        gl->redrawGLview();
        this->redraw();
    }
}
