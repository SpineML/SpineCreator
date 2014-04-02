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

#include "viewELexptpanelhandler.h"
#include "connectionmodel.h"
#include "glconnectionwidget.h"
#include "rootdata.h"
#include "rootlayout.h"
#include "layoutaliaseditdialog.h"
#include "layouteditpreviewdialog.h"
#include "mainwindow.h"
#include "nineML_classes.h"
#include "experiment.h"
#include "projectobject.h"
#include "undocommands.h"
//#include "stringify.h"

#define NEW_EXPERIMENT_VIEW11

viewELExptPanelHandler::viewELExptPanelHandler(QObject *parent) :
    QObject(parent)
{
}

viewELExptPanelHandler::viewELExptPanelHandler(viewELstruct * viewEL, rootData * data, QObject *parent) :
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

    this->exptSetup->setContentsMargins(14,14,14,14);
    this->exptInputs->setContentsMargins(4,4,4,4);
    this->exptOutputs->setContentsMargins(4,4,4,4);
    this->exptChanges->setContentsMargins(4,4,4,4);

    // add panel to expts
    ((QHBoxLayout *) this->viewEL->expt->layout())->addLayout(this->exptSetup);
#ifndef NEW_EXPERIMENT_VIEW
    // add divider
    QFrame* lineA = new QFrame();
    lineA->setMaximumWidth(1);
    lineA->setFrameShape(QFrame::VLine);
    lineA->setFrameShadow(QFrame::Plain);
    ((QHBoxLayout *) this->viewEL->expt->layout())->addWidget(lineA);

    // add panel to expts
    ((QHBoxLayout *) this->viewEL->expt->layout())->addLayout(this->exptInputs);

    // add divider
    QFrame* lineB = new QFrame();
    lineB->setMaximumWidth(1);
    lineB->setFrameShape(QFrame::VLine);
    lineB->setFrameShadow(QFrame::Plain);
    ((QHBoxLayout *) this->viewEL->expt->layout())->addWidget(lineB);

    // add panel to expts
    ((QHBoxLayout *) this->viewEL->expt->layout())->addLayout(this->exptOutputs);

    // add divider
    QFrame* lineC = new QFrame();
    lineC->setMaximumWidth(1);
    lineC->setFrameShape(QFrame::VLine);
    lineC->setFrameShadow(QFrame::Plain);
    ((QHBoxLayout *) this->viewEL->expt->layout())->addWidget(lineC);

    // add panel to expts
    ((QHBoxLayout *) this->viewEL->expt->layout())->addLayout(this->exptChanges);

#endif
    this->viewEL->panel->setStyleSheet("background-color :white");

    redrawPanel();
    redrawExpt();

}

void viewELExptPanelHandler::recursiveDeleteLoop(QLayout * parentLayout)
{
    QLayoutItem * item;
    while ((item = parentLayout->takeAt(0))) {
        if (item->widget()) {
            item->widget()->disconnect((QObject *)0);
            item->widget()->hide();
            forDeleting.push_back(item->widget());
        }
        if (item->layout())
            recursiveDeleteLoop(item->layout());
        delete item;
    }
    parentLayout->deleteLater();
}

void viewELExptPanelHandler::recursiveDelete(QLayout * parentLayout)
{
    QLayoutItem * item;
    while ((item = parentLayout->takeAt(2))) {
        if (item->widget()) {
            item->widget()->disconnect((QObject *)0);
            item->widget()->hide();
            forDeleting.push_back(item->widget());
        }
        if (item->layout())
            recursiveDeleteLoop(item->layout());
        delete item;
    }
}


void viewELExptPanelHandler::recursiveDeleteExpt(QLayout * parentLayout)
{
    QLayoutItem * item;
    while ((item = parentLayout->takeAt(0))) {
        if (item->widget()) {
            item->widget()->disconnect((QObject *)0);
            item->widget()->hide();
            forDeleting.push_back(item->widget());
        }
        if (item->layout())
            recursiveDeleteLoop(item->layout());
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
        qDebug() << horiz_scroll << vert_scroll;
    }

    // clear old stuff
    while(forDeleting.size() > 0) {
        forDeleting[0]->deleteLater();
        forDeleting.erase(forDeleting.begin());
    }

    recursiveDeleteExpt(exptSetup);
    recursiveDeleteExpt(exptInputs);
    recursiveDeleteExpt(exptOutputs);
    recursiveDeleteExpt(exptChanges);

    experiment * currentExperiment = NULL;

    // find currentExperiment
    for (uint i = 0; i < data->experiments.size(); ++i) {
        if (data->experiments[i]->selected) {currentExperiment = data->experiments[i]; break;}
    }

    if (currentExperiment == NULL) return;

    if (currentExperiment->editing)
        return;

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
            population * pop = (population *) this->currSystemObject;
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
                // first put up the input ports that we can send inputs to

                // then put up the loggable output ports

                // then allow variable overrides

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
    for (uint i = 0; i < currentExperiment->ins.size(); ++i) {
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
    for (uint i=0; i < currentExperiment->ins.size(); ++i)
        if (currentExperiment->ins[i]->edit)
            addIn->setDisabled(true);

    connect(addIn, SIGNAL(clicked()), this, SLOT(addInput()));

    formIn->addWidget(addIn);



    // redraw MODEL OUTPUTS

    QVBoxLayout * formOut = new QVBoxLayout;
    exptOutputs->addLayout(formOut);

    // existing
    for (uint i = 0; i < currentExperiment->outs.size(); ++i) {
        exptOutput * out = currentExperiment->outs[i];
        formOut->addLayout(out->drawOutput(this->data, this));
    }

    //add new output
    QPushButton * addOut = new QPushButton("Add Output");
    addOut->setIcon(QIcon(":/icons/toolbar/addShad.png"));
    addOut->setFlat(true);
    addOut->setFocusPolicy(Qt::NoFocus);
    addOut->setToolTip("Add a new experimental output to the current model");
    addOut->setFont(addFont);
    // grey out if editing:
    for (uint i=0; i < currentExperiment->outs.size(); ++i)
        if (currentExperiment->outs[i]->edit)
            addOut->setDisabled(true);

    connect(addOut, SIGNAL(clicked()), this, SLOT(addOutput()));

    formOut->addWidget(addOut);

    // redraw CHANGES

    QVBoxLayout * formLesion = new QVBoxLayout;
    exptChanges->addLayout(formLesion);

    formLesion->addWidget(new QLabel("Lesions"));

    // existing
    for (uint i = 0; i < currentExperiment->lesions.size(); ++i) {
        exptLesion * out = currentExperiment->lesions[i];
        formLesion->addLayout(out->drawLesion(this->data, this));
    }

    //add new lesion
    QPushButton * addLesion = new QPushButton("Add Lesion");
    addLesion->setIcon(QIcon(":/icons/toolbar/addShad.png"));
    addLesion->setFlat(true);
    addLesion->setFocusPolicy(Qt::NoFocus);
    addLesion->setToolTip("Add a new lesion to the current model");
    addLesion->setFont(addFont);
    // grey out if editing:
    for (uint i=0; i < currentExperiment->lesions.size(); ++i) {
        if (currentExperiment->lesions[i]->edit) {
            addLesion->setDisabled(true);
        }
    }

    connect(addLesion, SIGNAL(clicked()), this, SLOT(addLesion()));

    formLesion->addWidget(addLesion);



    QVBoxLayout * formPropChanges = new QVBoxLayout;
    exptChanges->addLayout(formPropChanges);

    formPropChanges->addWidget(new QLabel("Changed properties"));

    // existing
    for (uint i = 0; i < currentExperiment->changes.size(); ++i) {
        exptChangeProp * out = currentExperiment->changes[i];
        formPropChanges->addLayout(out->drawChangeProp(this->data, this));
    }

    //add new par change
    QPushButton * addChange = new QPushButton("Add Property Change");
    addChange->setIcon(QIcon(":/icons/toolbar/addShad.png"));
    addChange->setFlat(true);
    addChange->setFocusPolicy(Qt::NoFocus);
    addChange->setToolTip("Add a changed property to the current model");
    addChange->setFont(addFont);
    // grey out if editing:
    for (uint i=0; i < currentExperiment->changes.size(); ++i) {
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
    for (uint i=0; i < data->experiments.size(); ++i)
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
    uint index = sender()->property("index").toUInt();

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
    uint index = sender()->property("index").toUInt();
    uint type = sender()->property("type").toUInt();

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
    uint index = sender()->property("index").toUInt();

    data->experiments[index]->editing = true;

    // enable to run button
    emit enableRun(false);

    // redraw to update the editBox
    redrawPanel();
    redrawExpt();
}

void viewELExptPanelHandler::doneEditExperiment()
{
    uint index = sender()->property("index").toUInt();

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
    uint index = sender()->property("index").toUInt();

    data->experiments[index]->editing = false;

    // enable to run button
    emit enableRun(true);

    // redraw to update the editBox
    redrawPanel();
    redrawExpt();
}

void viewELExptPanelHandler::changeSelection()
{
    uint index = sender()->property("index").toUInt();

    // also make sure that the run button is enabled
    emit enableRun(true);

    // check if this is the selected box - if it is then do nothing
    if (data->experiments[index]->selected)
        return;

    // otherwise select it
    data->experiments[index]->select(&(data->experiments));

    // redraw to update the selection
    redrawPanel();
    redrawExpt();
}


void viewELExptPanelHandler::changedEngine(QString sim)
{
    experiment * currentExperiment = NULL;

    // find currentExperiment
    for (uint i = 0; i < data->experiments.size(); ++i) {
        if (data->experiments[i]->selected) {currentExperiment = data->experiments[i]; break;}
    }

    if (currentExperiment == NULL) return;

    currentExperiment->setup.simType = sim;

    // redraw to update the selection
    redrawExpt();
}

void viewELExptPanelHandler::changedDt()
{
    experiment * currentExperiment = NULL;

    // find currentExperiment
    for (uint i = 0; i < data->experiments.size(); ++i) {
        if (data->experiments[i]->selected) {currentExperiment = data->experiments[i]; break;}
    }

    if (currentExperiment == NULL) return;

    currentExperiment->setup.dt = ((QDoubleSpinBox *) sender())->value();;
}

void viewELExptPanelHandler::changedDuration()
{
    experiment * currentExperiment = NULL;

    // find currentExperiment
    for (uint i = 0; i < data->experiments.size(); ++i) {
        if (data->experiments[i]->selected) {currentExperiment = data->experiments[i]; break;}
    }

    if (currentExperiment == NULL) return;

    currentExperiment->setup.duration = ((QDoubleSpinBox *) sender())->value();
}

void viewELExptPanelHandler::changedSolver(int index)
{
    experiment * currentExperiment = NULL;

    // find currentExperiment
    for (uint i = 0; i < data->experiments.size(); ++i) {
        if (data->experiments[i]->selected) {currentExperiment = data->experiments[i]; break;}
    }

    if (currentExperiment == NULL) return;

    solverType type = (solverType) ((QComboBox *) sender())->itemData(index).toInt();

    currentExperiment->setup.solver = type;

    // redraw to update the selection
    redrawExpt();
}

void viewELExptPanelHandler::changedSolverOrder(int val)
{
    experiment * currentExperiment = NULL;

    // find currentExperiment
    for (uint i = 0; i < data->experiments.size(); ++i) {
        if (data->experiments[i]->selected) {currentExperiment = data->experiments[i]; break;}
    }

    if (currentExperiment == NULL) return;

    currentExperiment->setup.solverOrder = val;

    // redraw to update the selection
    redrawExpt();
}

void viewELExptPanelHandler::addInput()
{
    experiment * currentExperiment = NULL;

    // find currentExperiment
    for (uint i = 0; i < data->experiments.size(); ++i) {
        if (data->experiments[i]->selected) {currentExperiment = data->experiments[i]; break;}
    }

    if (currentExperiment == NULL) return;

    currentExperiment->ins.push_back(new exptInput);

    // redraw to update the selection
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
    NineMLComponentData * src = (NineMLComponentData *)0;

    // find source:
    for (uint i = 0; i < data->populations.size(); ++i) {
        if (data->populations[i]->neuronType->getXMLName() == text) {
            src = data->populations[i]->neuronType;
        }
        for (uint j = 0; j < data->populations[i]->projections.size(); ++j) {
            for (uint k = 0; k < data->populations[i]->projections[j]->synapses.size(); ++k) {
                if (data->populations[i]->projections[j]->synapses[k]->weightUpdateType->getXMLName() == text) {
                    src = data->populations[i]->projections[j]->synapses[k]->weightUpdateType;
                }
                if (data->populations[i]->projections[j]->synapses[k]->postsynapseType->getXMLName() == text) {
                    src = data->populations[i]->projections[j]->synapses[k]->postsynapseType;
                }
            }
        }
    }

    exptInput * in = (exptInput *) sender()->property("ptr").value<void *>();

    if (src != (NineMLComponentData *)0) {
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
        // redraw to update the selection
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

    // redraw to update the selection
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

    // redraw to update the selection
    redrawExpt();
}

void viewELExptPanelHandler::setInputRateDistributionType(int index)
{
    exptInput * in = (exptInput *) sender()->property("ptr").value<void *>();

    if (index == 0)
        in->rateDistribution = Regular;
    else if (index == 1)
        in->rateDistribution = Poisson;

    // redraw to update the selection
    redrawExpt();
}

void viewELExptPanelHandler::acceptInput()
{
    exptInput * in = (exptInput *) sender()->property("ptr").value<void *>();
    in->edit = false;

    // redraw to update the selection
    redrawExpt();
}

void viewELExptPanelHandler::editInput()
{
    exptInput * in = (exptInput *) sender()->property("ptr").value<void *>();
    in->edit = true;

    // redraw to update the selection
    redrawExpt();
}

void viewELExptPanelHandler::delInput()
{
    experiment * currentExperiment = NULL;

    // find currentExperiment
    for (uint i = 0; i < data->experiments.size(); ++i) {
        if (data->experiments[i]->selected) {currentExperiment = data->experiments[i]; break;}
    }

    if (currentExperiment == NULL) return;

    exptInput * in = (exptInput *) sender()->property("ptr").value<void *>();

    // push the command onto the undo stack
    this->data->currProject->undoStack->push(new deleteInputUndo(this->data, currentExperiment, in));

    // redraw to update the selection
    redrawExpt();
}

void viewELExptPanelHandler::setInputTypeData(double value)
{
    exptInput * in = (exptInput *) sender()->property("ptr").value<void *>();
    if (in->params.size() != 1) {
        in->params.resize(1,0);
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
        if (index > (int) in->params.size()-1)
            in->params.resize(qCeil(index/2)*2, 0);

        in->params[index] = value;

        // reorder:
        vector <float> tempVec;

        while (in->params.size() > 1) {
            int min = 100000000;
            int minIndex = 0;
            for (uint i = 0; i < in->params.size(); i+=2) {
                if (in->params[i] < min) {min = in->params[i]; minIndex = i;}
            }
            for (uint i = 0; i < 2; ++i) {
                tempVec.push_back(in->params[minIndex]);
                in->params.erase(in->params.begin()+minIndex);
            }
        }

        in->params.swap(tempVec);

    } else if (in->inType == arrayConstant) {

        in->params[row] = value;

    } else if (in->inType == arrayTimevarying) {

        vector < float > curr;

        bool copy = false;
        // move current index to new vector
        for (int i = 0; i < (int) in->params.size(); i+=2) {
            if (in->params[i] == -1 && copy) break;
            if (copy)
            {
                curr.push_back(in->params[i]);
                curr.push_back(in->params[i+1]);
                in->params.erase(in->params.begin()+i, in->params.begin()+i+2);
                i -= 2;
            }
            if (in->params[i] == -1 && in->params[i+1] == in->currentIndex) {
                copy = true;
                in->params.erase(in->params.begin()+i, in->params.begin()+i+2);
                i -= 2;
            }
        }

        // construct index and set value
        int index = row*2+col;
        if (index > (int) curr.size()-1)
            curr.resize(qCeil(index/2)*2, 0);

        curr[index] = value;

        // reorder:
        vector <float> tempVec;

        while (curr.size() > 1) {
            int min = 100000000;
            int minIndex = 0;
            for (uint i = 0; i < curr.size(); i+=2) {
                if (curr[i] < min) {min = curr[i]; minIndex = i;}
            }
            for (uint i = 0; i < 2; ++i) {
                tempVec.push_back(curr[minIndex]);
                curr.erase(curr.begin()+minIndex);
            }
        }

        // put back the index
        in->params.push_back(-1);
        in->params.push_back(in->currentIndex);
        for (uint i = 0; i < tempVec.size(); ++i) {
            in->params.push_back(tempVec[i]);
        }
    }

    // redraw to update the selection
    redrawExpt();
}

void viewELExptPanelHandler::fillInputParams()
{
    exptInput * in = (exptInput *) sender()->property("ptr").value<void *>();
    QDoubleSpinBox * fillSpin = (QDoubleSpinBox *) sender()->property("ptrSpin").value<void *>();

    if (in->inType == arrayConstant) {
        for (uint i = 0; i < in->params.size(); ++i) {
            in->params[i] = fillSpin->value();
        }
    }

    // redraw to update the selection
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
        in->params.resize(qCeil(in->params.size()/2)*2+2,0);
    }
    if (in->inType == arrayTimevarying) {
        bool found = false;
        int indexIndex = -1;

        // find where to operate on
        for (uint i = 0; i < in->params.size(); i+=2) {
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

    // redraw to update the selection
    redrawExpt();
}

void viewELExptPanelHandler::setInputDelTVRow()
{
    exptInput * in = (exptInput *) sender()->property("ptr").value<void *>();

    if (in->inType == timevarying) {
        in->params.resize(qCeil(in->params.size()/2)*2-2,0);
    }
    if (in->inType == arrayTimevarying) {
        bool found = false;
        int indexIndex = -1;

        // find where to operate on
        for (uint i = 0; i < in->params.size(); i+=2) {
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

    // redraw to update the selection
    redrawExpt();
}

void viewELExptPanelHandler::addOutput()
{
    experiment * currentExperiment = NULL;

    // find currentExperiment
    for (uint i = 0; i < data->experiments.size(); ++i) {
        if (data->experiments[i]->selected) {currentExperiment = data->experiments[i]; break;}
    }

    if (currentExperiment == NULL) return;

    currentExperiment->outs.push_back(new exptOutput);

    // redraw to update the selection
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
    NineMLComponentData * src = (NineMLComponentData *)0;

    // find source:
    for (uint i = 0; i < data->populations.size(); ++i) {
        if (data->populations[i]->neuronType->getXMLName() == text) {
            src = data->populations[i]->neuronType;
        }
        for (uint j = 0; j < data->populations[i]->projections.size(); ++j) {
            for (uint k = 0; k < data->populations[i]->projections[j]->synapses.size(); ++k) {
                if (data->populations[i]->projections[j]->synapses[k]->weightUpdateType->getXMLName() == text) {
                    src = data->populations[i]->projections[j]->synapses[k]->weightUpdateType;
                }
                if (data->populations[i]->projections[j]->synapses[k]->postsynapseType->getXMLName() == text) {
                    src = data->populations[i]->projections[j]->synapses[k]->postsynapseType;
                }
            }
        }
    }

    exptOutput * out = (exptOutput *) sender()->property("ptr").value<void *>();

    if (src != (NineMLComponentData *)0) {
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
        // redraw to update the selection
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

    // redraw to update the selection
    redrawExpt();
}

void viewELExptPanelHandler::setOutputType()
{
    // redraw to update the selection
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

void viewELExptPanelHandler::acceptOutput()
{
    exptOutput * out = (exptOutput *) sender()->property("ptr").value<void *>();
    out->edit = false;

    // redraw to update the selection
    redrawExpt();
}

void viewELExptPanelHandler::editOutput()
{
    exptOutput * out = (exptOutput *) sender()->property("ptr").value<void *>();
    out->edit = true;

    // redraw to update the selection
    redrawExpt();
}

void viewELExptPanelHandler::delOutput()
{
    experiment * currentExperiment = NULL;

    // find currentExperiment
    for (uint i = 0; i < data->experiments.size(); ++i) {
        if (data->experiments[i]->selected) {currentExperiment = data->experiments[i]; break;}
    }

    if (currentExperiment == NULL) return;

    exptOutput * out = (exptOutput *) sender()->property("ptr").value<void *>();

    // push the command onto the undo stack
    this->data->currProject->undoStack->push(new deleteOutputUndo(this->data, currentExperiment, out));

    // redraw to update the selection
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
    for (uint i = 0; i < data->experiments.size(); ++i) {
        if (data->experiments[i]->selected) {currentExperiment = data->experiments[i]; break;}
    }

    if (currentExperiment == NULL) return;

    currentExperiment->lesions.push_back(new exptLesion);

    // redraw to update the selection
    redrawExpt();
}

void viewELExptPanelHandler::setLesionProjection()
{
    // input text
    QString text = ((QLineEdit *) sender())->text();
    projection * lesionedProj = (projection *)0;

    // find source:
    for (uint i = 0; i < data->populations.size(); ++i) {
        for (uint j = 0; j < data->populations[i]->projections.size(); ++j) {
            if (data->populations[i]->projections[j]->getName() == text)
                lesionedProj = data->populations[i]->projections[j];
        }
    }

    exptLesion * lesion = (exptLesion *) sender()->property("ptr").value<void *>();

    if (lesionedProj != (projection *)0) {
        lesion->proj = lesionedProj;
        lesion->set = true;
        QPalette p = ((QLineEdit *) sender())->palette();
        p.setColor( QPalette::Normal, QPalette::Base, QColor(200, 255, 200) );
        ((QLineEdit *) sender())->setPalette(p);
    } else {
        lesion->set = false;
        QPalette p = ((QLineEdit *) sender())->palette();
        p.setColor( QPalette::Normal, QPalette::Base, QColor(255, 200, 200) );
        ((QLineEdit *) sender())->setPalette(p);
    }

    // redraw to update the selection
    redrawExpt();
}

void viewELExptPanelHandler::acceptLesion()
{
    exptLesion * lesion = (exptLesion *) sender()->property("ptr").value<void *>();
    lesion->edit = false;
    // redraw to update the selection
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
    for (uint i = 0; i < data->experiments.size(); ++i) {
        if (data->experiments[i]->selected) {currentExperiment = data->experiments[i]; break;}
    }

    if (currentExperiment == NULL) return;

    exptLesion * lesion = (exptLesion *) sender()->property("ptr").value<void *>();

    // push the command onto the undo stack
    this->data->currProject->undoStack->push(new deleteLesionUndo(this->data, currentExperiment, lesion));

    redrawExpt();
}

void viewELExptPanelHandler::setChangeParComponent()
{
    // input text
    QString text = ((QLineEdit *) sender())->text();
    NineMLComponentData * src = (NineMLComponentData *)0;

    // find source:
    for (uint i = 0; i < data->populations.size(); ++i) {
        if (data->populations[i]->neuronType->getXMLName() == text) {
            src = data->populations[i]->neuronType;
        }
        for (uint j = 0; j < data->populations[i]->projections.size(); ++j) {
            for (uint k = 0; k < data->populations[i]->projections[j]->synapses.size(); ++k) {
                if (data->populations[i]->projections[j]->synapses[k]->weightUpdateType->getXMLName() == text) {
                    src = data->populations[i]->projections[j]->synapses[k]->weightUpdateType;
                }
                if (data->populations[i]->projections[j]->synapses[k]->postsynapseType->getXMLName() == text) {
                    src = data->populations[i]->projections[j]->synapses[k]->postsynapseType;
                }
            }
        }
    }

    exptChangeProp * changeProp = (exptChangeProp *) sender()->property("ptr").value<void *>();

    if (src != (NineMLComponentData *)0) {
        changeProp->component = src;
        if (src->ParameterList.size() > 0) {
            changeProp->par = new ParameterData(src->ParameterList.front());
        } else if (src->StateVariableList.size() > 0) {
            changeProp->par = new StateVariableData(src->StateVariableList.front());
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

    // redraw to update the selection
    redrawExpt();
}

void viewELExptPanelHandler::setChangeProp(QString name)
{
    exptChangeProp * changeProp = (exptChangeProp *) sender()->property("ptr").value<void *>();

    for (uint i = 0; i < changeProp->component->ParameterList.size(); ++i) {
        if (name == changeProp->component->ParameterList[i]->name) {
            delete changeProp->par;
            changeProp->par = new ParameterData(changeProp->component->ParameterList[i]);
        }
    }
    for (uint i = 0; i < changeProp->component->StateVariableList.size(); ++i) {
        if (name == changeProp->component->StateVariableList[i]->name) {
            delete changeProp->par;
            changeProp->par = new StateVariableData(changeProp->component->StateVariableList[i]);
        }
    }
    redrawExpt();
}

void viewELExptPanelHandler::addChangedProp()
{
    experiment * currentExperiment = NULL;

    // find currentExperiment
    for (uint i = 0; i < data->experiments.size(); ++i) {
        if (data->experiments[i]->selected) {currentExperiment = data->experiments[i]; break;}
    }

    if (currentExperiment == NULL) return;

    currentExperiment->changes.push_back(new exptChangeProp);

    // redraw to update the selection
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
    for (uint i = 0; i < data->experiments.size(); ++i) {
        if (data->experiments[i]->selected) {currentExperiment = data->experiments[i]; break;}
    }

    if (currentExperiment == NULL) return;

    exptChangeProp * prop = (exptChangeProp *) sender()->property("ptr").value<void *>();

    // push the command onto the undo stack
    this->data->currProject->undoStack->push(new deleteChangePropUndo(this->data, currentExperiment, prop));

    // redraw to update the selection
    redrawExpt();
}

void viewELExptPanelHandler::run()
{
    QSettings settings;

    runButton = (QPushButton *) sender();
    runButton->setEnabled(false);

    simulatorStdOutText = "";
    simulatorStdErrText = "";

    // fetch current experiment sim engine
    experiment * currentExperiment = NULL;

    // find currentExperiment
    for (uint i = 0; i < data->experiments.size(); ++i) {
        if (data->experiments[i]->selected) {currentExperiment = data->experiments[i]; break;}
    }

    if (currentExperiment == NULL) {
        runButton->setEnabled(true);
        return;
    }

    QString simName = currentExperiment->setup.simType;

    // load path
    settings.beginGroup("simulators/" + simName);
    QString path = settings.value("path").toString();
    // Check that path exists and is executable.
    QFile the_script(path);
    if (!the_script.exists()) {
        // Error - convert_script file doesn't exist
        QMessageBox msgBox;
        msgBox.setWindowTitle("Simulator Error");
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setText("The simulator '" + path + "' does not exist.");
        msgBox.exec();
        runButton->setEnabled(true);
        return;
    } else {
        // Path exists, check it's a file and is executable
        // NB: In QT 5.x this would be QFileDevice::ExeOwner etc
        if (the_script.permissions() & (QFile::ExeOwner|QFile::ExeGroup|QFile::ExeOther)) {
            // Probably Ok - we have execute permission of some kind.
        } else {
            // Error - no execute permission on script
            QMessageBox msgBox;
            msgBox.setWindowTitle("Simulator Error");
            msgBox.setIcon(QMessageBox::Critical);
            msgBox.setText("The simulator '" + path + "' is not executable.");
            msgBox.exec();
            runButton->setEnabled(true);
            return;
        }
    }

    // The convert_script takes the working directory as a script argument
    QString wk_dir_string = settings.value("working_dir").toString();
    settings.endGroup();
    wk_dir_string = QDir::toNativeSeparators(wk_dir_string);
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

    settings.setValue("simulator_export_path",QDir::toNativeSeparators(wk_dir_string + "/model/"));
    settings.setValue("export_binary",settings.value("simulators/" + simName + "/binary").toBool());

    // clear directory
    QDir model_dir(QDir::toNativeSeparators(wk_dir_string + "/model/"));
    QStringList files = model_dir.entryList(QDir::Files);
    for (int i = 0; i < files.size(); ++i) {
        model_dir.remove(files[i]);
    }

    // write out model
    if (!this->data->currProject->export_for_simulator(QDir::toNativeSeparators(wk_dir_string + "/model/"), data)) {
        settings.remove("simulator_export_path");
        settings.remove("export_binary");
        runButton->setEnabled(true);
        return;
    }

    settings.remove("simulator_export_path");
    settings.remove("export_binary");

    QProcess * simulator = new QProcess;
    if (!simulator) {
        // Really bad error - memory allocation error. The following will probably fail:
        QMessageBox msgBox;
        msgBox.setWindowTitle("Memory Allocation Error");
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setText("The simulator failed to start - you're out of RAM.");
        msgBox.exec();
        runButton->setEnabled(true);
        return;
    }

    simulator->setWorkingDirectory(wk_dir.absolutePath());
    simulator->setProcessEnvironment(env);

    simulator->setProperty("logpath", wk_dir_string + QDir::separator() + "temp");

    simulator->start(path, QStringList() << "-w" << wk_dir.absolutePath());

    // Wait a couple of seconds for the process to start
    if (!simulator->waitForStarted(5000)) {
        // Error - simulator failed to start
        QMessageBox msgBox;
        msgBox.setWindowTitle("Simulator Error");
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setText("The simulator '" + path + "' failed to start.");
        msgBox.exec();
        runButton->setEnabled(true);
        delete simulator;
        return;
    }

    connect(simulator, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(simulatorFinished(int, QProcess::ExitStatus)));
    connect(simulator, SIGNAL(readyReadStandardOutput()), this, SLOT(simulatorStandardOutput()));
    connect(simulator, SIGNAL(readyReadStandardError()), this, SLOT(simulatorStandardError()));
}

void viewELExptPanelHandler::simulatorFinished(int, QProcess::ExitStatus status)
{
    // update run button
    runButton->setEnabled(true);

    // check for errors we can present
    for (uint i = 0; i < (uint) errorStrings.size(); ++i) {

        // check if error there
        if (simulatorStdOutText.contains(errorStrings[i])) {
            // error found
            QMessageBox msgBox;
            msgBox.setWindowTitle("Simulator Error");
            msgBox.setIcon(QMessageBox::Critical);
            msgBox.setText(errorMessages[i]);
            msgBox.setDetailedText(simulatorStdOutText);
            msgBox.exec();
            return;
        }
    }

    // collect logs
    QDir logs(sender()->property("logpath").toString());

    QStringList filter;
    filter << "*.xml";
    logs.setNameFilters(filter);

    // add logs to graphs
    data->main->viewGV.properties->loadDataFiles(logs.entryList(), &logs);

    // and insert logs into visualiser
    if (data->main->viewVZ.OpenGLWidget != NULL) {
        data->main->viewVZ.OpenGLWidget->addLogs(&data->main->viewGV.properties->logs);
    }

    // get status
    if (status == QProcess::CrashExit) {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Simulator Error");
        msgBox.setText(simulatorStdErrText);
        msgBox.exec();
        return;
    }

    if (status == QProcess::NormalExit) {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Simulator Complete");
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setText("Simulator has finished. See below for more details.");
        msgBox.setDetailedText(simulatorStdOutText);
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.exec();
        return;
    }
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
    for (unsigned int i = 0; i < this->data->populations.size(); ++i) {
        this->data->populations[i]->draw(painter, GLscale, viewX, viewY, width, height, this->data->popImage, style);
    }

    // draw the synapses
    for (unsigned int i = 0; i < this->data->populations.size(); ++i) {
        this->data->populations[i]->drawSynapses(painter, GLscale, viewX, viewY, width, height, style);
    }

    // draw the generic inputs
    for (unsigned int i = 0; i < this->data->populations.size(); ++i) {
        QPen pen(QColor(100,0,0,100));
        pen.setWidthF(float(1));
        painter->setPen(pen);
        this->data->populations[i]->drawInputs(painter, GLscale, viewX, viewY, width, height, style);
    }

    // redraw selected object to highlight, if it is not deleted pointer:
    if (data->isValidPointer(this->currSystemObject) == true) {
        if (this->currSystemObject->type == populationObject) {
            population * pop = (population *) this->currSystemObject;

            float left = ((pop->getLeft()+viewX)*GLscale+float(width))/2;
            float right = ((pop->getRight()+viewX)*GLscale+float(width))/2;
            float top = ((-pop->getTop()+viewY)*GLscale+float(height))/2;
            float bottom = ((-pop->getBottom()+viewY)*GLscale+float(height))/2;

            if (pop->isSpikeSource) {
                for (unsigned int i = 5; i > 1; --i) {
                    QPen pen(QColor(0,0,0,50/i));
                    pen.setWidthF(float(i*2));
                    painter->setPen(pen);
                    painter->drawEllipse(QPointF((right+left)/2.0, (top+bottom)/2.0),0.5*GLscale/2.0,0.5*GLscale/2.0);
                }

            } else {

                for (unsigned int i = 5; i > 1; --i) {
                    QPen pen(QColor(0,0,0,50/i));
                    pen.setWidthF(float(i*2));
                    painter->setPen(pen);
                    QRectF rectangle(left, top, right-left, bottom-top);
                    painter->drawRect(rectangle);
                }
            }
        }

        if (this->currSystemObject->type == projectionObject) {
            projection * col = (projection *) this->currSystemObject;
            for (unsigned int i = 5; i > 1; --i) {
                QPen pen(QColor(0,0,0,30/i));
                pen.setWidthF(float(i*2));
                painter->setPen(pen);
                col->draw(painter, GLscale, viewX, viewY, width, height, this->data->popImage, standardDrawStyle);
            }
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
    vector<systemObject*> newlySelectedList;
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
        this->currSystemObject = NULL;
        // update the UI
        gl->redrawGLview();
        this->redraw();
    }
}
