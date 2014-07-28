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

#include "viewVZlayoutedithandler.h"
#include "systemmodel.h"
#include "experiment.h"
#include "filteroutundoredoevents.h"
#include "projectobject.h"

viewVZLayoutEditHandler::viewVZLayoutEditHandler(rootData * data, viewNLstruct * viewNL, viewVZstruct * viewVZ, QObject *parent) :
    QObject(parent)
{
    this->data = data;
    this->viewVZ = viewVZ;
    this->viewNL = viewNL;
    this->viewVZ->currObject = (systemObject *)0;
    this->viewVZ->treeView = NULL;

    connect(&playBack, SIGNAL(timeout()), this, SLOT(playBackTimeout()));

    initGlobal();
    initPopulation();
    initConnection();

    // add error reporting box and stretch
    QVBoxLayout * panelLayout = (QVBoxLayout *) this->viewVZ->panel->layout();
    panelLayout->addWidget(this->viewVZ->errors);
    panelLayout->addStretch();

}

void clearLayout(QLayout *layout, QString type) {
    QLayoutItem *item;
    int passedItems = 0;
    while((item = layout->itemAt(passedItems))) {
        if (item->layout()) {
            if (item->layout()->property("wtype").toString() == type || type == "") {
                clearLayout(item->layout(), "");
                layout->removeItem(item);
                item->layout()->deleteLater();
                delete item;
            } else
                ++passedItems;
        }
        else if (item->widget()) {
            if (item->widget()->property("wtype").toString() == type || type == "") {
                layout->removeItem(item);

#ifdef Q_OS_MAC
                if (qobject_cast<QComboBox*>(item->widget()) == NULL)
                    item->widget()->deleteLater();
                else
                    item->widget()->hide(); // ok, it's a memory leak - but f-it it isn't a big one and if I deletelater it crashes - Alex 2013
#else
                item->widget()->deleteLater();
#endif

                delete item;
            } else
                ++passedItems;
        }
        else if (item->spacerItem()) {
            ++passedItems;
        }
    }
}

void viewVZLayoutEditHandler::selectAll() {
    setAllSelectState(true);
}

void viewVZLayoutEditHandler::deselectAll() {
    setAllSelectState(false);
}

/*!
 * \brief viewVZLayoutEditHandler::saveTreeState
 * Save the expanded state of the QTreeWidget
 */
void viewVZLayoutEditHandler::saveTreeState(void)
{
    QStringList List;

    // prepare list
    // PS: getPersistentIndexList() function is a simple `return this->persistentIndexList()` from TreeModel model class
    foreach (QModelIndex index, this->viewVZ->sysModel->getPersistentIndexList())
    {
        if (this->viewVZ->treeView->isExpanded(index))
        {
            List << index.data(Qt::DisplayRole).toString();
        }
    }

    // save list
    this->data->currProject->treeWidgetState = List;
}

/*!
 * \brief viewVZLayoutEditHandler::restoreTreeState
 * Restore the expanded state of the QTreeWidget
 */
void viewVZLayoutEditHandler::restoreTreeState(void)
{
    QStringList List = this->data->currProject->treeWidgetState;

    foreach (QString item, List)
    {
        // search `item` text in model
        QModelIndexList Items = this->viewVZ->sysModel->match(this->viewVZ->sysModel->index(0, 0), Qt::DisplayRole, QVariant::fromValue(item));
        if (!Items.isEmpty())
        {
            foreach (QModelIndex index, Items) {
                // first level in QTreeWidget
                this->viewVZ->treeView->setExpanded(index, true);
                // now the second level (which is all we need)
                foreach (QString item2, List)
                {
                    // search `item` text in model
                    QModelIndexList Items2 = this->viewVZ->sysModel->match(this->viewVZ->sysModel->index(0, 0, index), Qt::DisplayRole, QVariant::fromValue(item2));
                    if (!Items2.isEmpty())
                    {
                        // Second level of QTreeWidget
                        this->viewVZ->treeView->setExpanded(Items2.first(), true);
                    }
                }
            }
        }
    }
}

void viewVZLayoutEditHandler::setAllSelectState(bool selectState) {

    for (uint i = 0; i < data->populations.size(); ++i) {

        population * currPop = (population *) data->populations[i];

        // populations
        currPop->isVisualised = selectState;

        // population generic outputs:

        for (uint output = 0; output < data->populations[i]->neuronType->outputs.size(); ++output) {

            genericInput * currOutput = data->populations[i]->neuronType->outputs[output];

            // add output if is not a projection input
            if (!currOutput->projInput) {
                currOutput->isVisualised = selectState;
            }

        }

        // projections
        for (uint j = 0; j < currPop->projections.size(); ++j) {

            projection * currProj = (projection *) currPop->projections[j];

            // synapses
            for (uint k = 0; k < currProj->synapses.size(); ++k) {

                synapse * currTarg = (synapse *) currProj->synapses[k];

                currTarg->isVisualised = selectState;

            }

        }

    }

    // apply changes
    viewVZ->OpenGLWidget->sysSelectionChanged(QModelIndex(), QModelIndex());

    // update treeview
    viewVZ->treeView->reset();

}

void viewVZLayoutEditHandler::initGlobal() {

    // the treeView:
    viewVZ->treeView = new QTreeView;
    viewVZ->treeView->setMaximumHeight(300);
    viewVZ->treeView->setMinimumHeight(300);
    // animation looks ugly on OSX
    viewVZ->treeView->setAnimated(false);

    // connect for hide
    connect(this, SIGNAL(hideTree()), viewVZ->treeView, SLOT(hide()));
    // connect for show
    connect(this, SIGNAL(showAll()), viewVZ->treeView, SLOT(show()));

    ((QVBoxLayout *) this->viewVZ->panel->layout())->addWidget(viewVZ->treeView);

    // add tools for manipulating the treeView ////

    QHBoxLayout * tools = new QHBoxLayout();

    QPushButton * expand = new QPushButton("Expand All");
    tools->addWidget(expand);
    // connect for hide
    connect(this, SIGNAL(hideAll()), expand, SLOT(hide()));
    // connect for show
    connect(this, SIGNAL(showAll()), expand, SLOT(show()));
    // connect for function
    connect(expand, SIGNAL(clicked()), this->viewVZ->treeView, SLOT(expandAll()));

    QPushButton * collapse = new QPushButton("Collapse All");
    tools->addWidget(collapse);
    // connect for hide
    connect(this, SIGNAL(hideAll()), collapse, SLOT(hide()));
    // connect for show
    connect(this, SIGNAL(showAll()), collapse, SLOT(show()));
    // connect for function
    connect(collapse, SIGNAL(clicked()), this->viewVZ->treeView, SLOT(collapseAll()));

    tools->addStretch();

    QPushButton * select = new QPushButton("Select All");
    tools->addWidget(select);
    // connect for hide
    connect(this, SIGNAL(hideAll()), select, SLOT(hide()));
    // connect for show
    connect(this, SIGNAL(showAll()), select, SLOT(show()));
    // connect for function
    connect(select, SIGNAL(clicked()), this, SLOT(selectAll()));

    QPushButton * deselect = new QPushButton("Deselect All");
    tools->addWidget(deselect);
    // connect for hide
    connect(this, SIGNAL(hideAll()), deselect, SLOT(hide()));
    // connect for show
    connect(this, SIGNAL(showAll()), deselect, SLOT(show()));
    // connect for function
    connect(deselect, SIGNAL(clicked()), this, SLOT(deselectAll()));

    ((QVBoxLayout *) this->viewVZ->panel->layout())->addLayout(tools);

    // add timebar //////////////////

    QHBoxLayout * time = new QHBoxLayout();
    time->addWidget(new QLabel("Time"));

    // connect for hide
    connect(this, SIGNAL(hideAll()), time->itemAt(0)->widget(), SLOT(hide()));
    // connect for show
    connect(this, SIGNAL(showAll()), time->itemAt(0)->widget(), SLOT(show()));

    timeSlider = new QSlider(Qt::Horizontal);
    time->addWidget(timeSlider);

    // connect for hide
    connect(this, SIGNAL(hideAll()), timeSlider, SLOT(hide()));
    // connect for show
    connect(this, SIGNAL(showAll()), timeSlider, SLOT(show()));
    // connect for change
    connect(timeSlider, SIGNAL(sliderMoved(int)), viewVZ->OpenGLWidget, SLOT(updateLogDataTime(int)));

    QCommonStyle style;
    playButton = new QPushButton("");
    playButton->setIcon(style.standardIcon(QStyle::SP_MediaPlay));
    playButton->setMaximumWidth(32);
    playButton->setFlat(true);

    time->addWidget(playButton);
    // connect for hide
    connect(this, SIGNAL(hideAll()), playButton, SLOT(hide()));
    // connect for show
    connect(this, SIGNAL(showAll()), playButton, SLOT(show()));
    // connect for change
    connect(playButton, SIGNAL(clicked()), this, SLOT(togglePlay()));

    ((QVBoxLayout *) this->viewVZ->panel->layout())->addLayout(time);

    // view toggle //////////

    QPushButton * popIndices = new QPushButton(QIcon(":/icons/toolbar/images/indices.png"), "", data->main);
    connect(popIndices, SIGNAL(toggled(bool)), this->viewVZ->OpenGLWidget, SLOT(setPopIndicesShown(bool)));
    popIndices->setCheckable("true");
    popIndices->setToolTip("Hide / show the indices on populations");
    popIndices->setMinimumHeight(28);
    popIndices->setMaximumWidth(28);
    popIndices->setFlat(true);
    popIndices->setChecked(false);

    viewVZ->toolbar->layout()->addWidget(popIndices);

    // view mode (ortho or not)

    QPushButton * ortho = new QPushButton(QIcon(":/icons/toolbar/images/ortho.png"), "", data->main);
    connect(ortho, SIGNAL(toggled(bool)), this->viewVZ->OpenGLWidget, SLOT(toggleOrthoView(bool)));
    ortho->setCheckable("true");
    ortho->setToolTip("Switch between orthographic and perspective modes");
    ortho->setMinimumHeight(28);
    ortho->setMaximumWidth(28);
    ortho->setFlat(true);
    ortho->setChecked(false);

    viewVZ->toolbar->layout()->addWidget(ortho);

    ((QHBoxLayout *)viewVZ->toolbar->layout())->addStretch();


}

void viewVZLayoutEditHandler::initPopulation() {

    QVBoxLayout * panelLayout = (QVBoxLayout *) this->viewVZ->panel->layout();

    ////// SIZE

    QHBoxLayout * tempBox = new QHBoxLayout();
    panelLayout->addLayout(tempBox);
    tempBox->addWidget(new QLabel("<b>Population size</b>"));

    // connect for hide
    connect(this, SIGNAL(hideAll()), tempBox->itemAt(0)->widget(), SLOT(hide()));
    // connect for show
    connect(this, SIGNAL(showPopulation()), tempBox->itemAt(0)->widget(), SLOT(show()));

    sizeSpin = new QSpinBox;
    sizeSpin->setRange(1, 2000000);
    sizeSpin->setSingleStep(1);
    sizeSpin->setFocusPolicy(Qt::StrongFocus);
    sizeSpin->installEventFilter(new FilterOutUndoRedoEvents);
    tempBox->addWidget(sizeSpin);

    // connect for hide
    connect(this, SIGNAL(hideAll()), sizeSpin, SLOT(hide()));
    // connect for show
    connect(this, SIGNAL(showPopulation()), sizeSpin, SLOT(show()));
    // connect for set value
    connect(this, SIGNAL(setPopulationSize(int)), sizeSpin, SLOT(setValue(int)));

    connect(sizeSpin, SIGNAL(editingFinished()), this->data, SLOT (setSize()));
    connect(sizeSpin, SIGNAL(valueChanged(int)), this->viewVZ->OpenGLWidget, SLOT (parsChangedPopulation(int)));

    tempBox->addStretch();

    ///// LOCATION

    QLabel * locationLabel = new QLabel("<b>3D location</b>");
    tempBox->addWidget(locationLabel);
    // connect for hide
    connect(this, SIGNAL(hideAll()), locationLabel, SLOT(hide()));
    // connect for show
    connect(this, SIGNAL(showPopulation()), locationLabel, SLOT(show()));

    // spinboxes for x, y, z

    QLabel * xLabel = new QLabel("x");
    // connect for hide
    connect(this, SIGNAL(hideAll()), xLabel, SLOT(hide()));
    // connect for show
    connect(this, SIGNAL(showPopulation()), xLabel, SLOT(show()));
    tempBox->addWidget(xLabel);
    xSpin = new QSpinBox;
    xSpin->setRange(-10000, 10000);
    xSpin->setSingleStep(1);
    xSpin->setMaximumWidth(70);
    xSpin->setProperty("type", 0);
    xSpin->setFocusPolicy(Qt::StrongFocus);
    xSpin->installEventFilter(new FilterOutUndoRedoEvents);
    tempBox->addWidget(xSpin);

    // connect for hide
    connect(this, SIGNAL(hideAll()), xSpin, SLOT(hide()));
    // connect for show
    connect(this, SIGNAL(showPopulation()), xSpin, SLOT(show()));
    // connect for set value
    connect(this, SIGNAL(setPopulationX(int)), xSpin, SLOT(setValue(int)));

    QLabel * yLabel = new QLabel("y");
    // connect for hide
    connect(this, SIGNAL(hideAll()), yLabel, SLOT(hide()));
    // connect for show
    connect(this, SIGNAL(showPopulation()), yLabel, SLOT(show()));
    tempBox->addWidget(yLabel);
    ySpin = new QSpinBox;
    ySpin->setRange(-10000, 10000);
    ySpin->setSingleStep(1);
    ySpin->setMaximumWidth(70);
    ySpin->setProperty("type", 1);
    ySpin->setFocusPolicy(Qt::StrongFocus);
    ySpin->installEventFilter(new FilterOutUndoRedoEvents);
    tempBox->addWidget(ySpin);

    // connect for hide
    connect(this, SIGNAL(hideAll()), ySpin, SLOT(hide()));
    // connect for show
    connect(this, SIGNAL(showPopulation()), ySpin, SLOT(show()));
    // connect for set value
    connect(this, SIGNAL(setPopulationY(int)), ySpin, SLOT(setValue(int)));

    QLabel * zLabel = new QLabel("z");
    // connect for hide
    connect(this, SIGNAL(hideAll()), zLabel, SLOT(hide()));
    // connect for show
    connect(this, SIGNAL(showPopulation()), zLabel, SLOT(show()));
    tempBox->addWidget(zLabel);
    zSpin = new QSpinBox;
    zSpin->setRange(-10000, 10000);
    zSpin->setSingleStep(1);
    zSpin->setMaximumWidth(70);
    zSpin->setProperty("type", 2);
    zSpin->setFocusPolicy(Qt::StrongFocus);
    zSpin->installEventFilter(new FilterOutUndoRedoEvents);
    tempBox->addWidget(zSpin);

    // connect for hide
    connect(this, SIGNAL(hideAll()), zSpin, SLOT(hide()));
    // connect for show
    connect(this, SIGNAL(showPopulation()), zSpin, SLOT(show()));
    // connect for set value
    connect(this, SIGNAL(setPopulationZ(int)), zSpin, SLOT(setValue(int)));

    // connect up
    connect(xSpin, SIGNAL(editingFinished()), this->data, SLOT (setLoc3()));
    connect(xSpin, SIGNAL(valueChanged(int)), this->viewVZ->OpenGLWidget, SLOT (redraw(int)));
    connect(ySpin, SIGNAL(editingFinished()), this->data, SLOT (setLoc3()));
    connect(ySpin, SIGNAL(valueChanged(int)), this->viewVZ->OpenGLWidget, SLOT (redraw(int)));
    connect(zSpin, SIGNAL(editingFinished()), this->data, SLOT (setLoc3()));
    connect(zSpin, SIGNAL(valueChanged(int)), this->viewVZ->OpenGLWidget, SLOT (redraw(int)));

    // ptrs for openGLwidget
    xSpin->setProperty("xptr", qVariantFromValue((void *) xSpin));
    xSpin->setProperty("yptr", qVariantFromValue((void *) ySpin));
    xSpin->setProperty("zptr", qVariantFromValue((void *) zSpin));
    ySpin->setProperty("xptr", qVariantFromValue((void *) xSpin));
    ySpin->setProperty("yptr", qVariantFromValue((void *) ySpin));
    ySpin->setProperty("zptr", qVariantFromValue((void *) zSpin));
    zSpin->setProperty("xptr", qVariantFromValue((void *) xSpin));
    zSpin->setProperty("yptr", qVariantFromValue((void *) ySpin));
    zSpin->setProperty("zptr", qVariantFromValue((void *) zSpin));


    ///// LAYOUT PARAMETERS //////

    // draw up the combobox
    layoutComboBox = this->addDropBox(panelLayout, "Layout", "layout");
    for (unsigned int i = 0; i < this->data->catalogLayout.size(); ++i) {
        layoutComboBox->addItem(this->data->catalogLayout[i]->name);
    }
    // connect for hide
    connect(this, SIGNAL(hideAll()), layoutComboBox, SLOT(hide()));
    // connect for show
    connect(this, SIGNAL(showPopulation()), layoutComboBox, SLOT(show()));
    // connect for function
    connect(layoutComboBox, SIGNAL(activated(int)), data, SLOT(updateComponentType(int)));

    QHBoxLayout * extraBox = new QHBoxLayout();
    panelLayout->addLayout(extraBox);

    QLabel * minDistLabel = new QLabel("min distance");
    // connect for hide
    connect(this, SIGNAL(hideAll()), minDistLabel, SLOT(hide()));
    // connect for show
    connect(this, SIGNAL(showPopulation()), minDistLabel, SLOT(show()));
    extraBox->addWidget(minDistLabel);
    QDoubleSpinBox *minDist = new QDoubleSpinBox;
    minDist->setRange(0, 10000);
    minDist->setSingleStep(1);
    minDist->setProperty("type", 0);
    minDist->setFocusPolicy(Qt::StrongFocus);
    minDist->installEventFilter(new FilterOutUndoRedoEvents);
    extraBox->addWidget(minDist);
    connect(minDist, SIGNAL(editingFinished()), this->data, SLOT (updateLayoutPar()));
    connect(minDist, SIGNAL(editingFinished()), this->viewVZ->OpenGLWidget, SLOT (redraw()));

    // connect for hide
    connect(this, SIGNAL(hideAll()), minDist, SLOT(hide()));
    // connect for show
    connect(this, SIGNAL(showPopulation()), minDist, SLOT(show()));
    // connect for set value
    connect(this, SIGNAL(setMinDistance(double)), minDist, SLOT(setValue(double)));

    QLabel * seedLabel = new QLabel("seed");
    // connect for hide
    connect(this, SIGNAL(hideAll()), seedLabel, SLOT(hide()));
    // connect for show
    connect(this, SIGNAL(showPopulation()), seedLabel, SLOT(show()));
    extraBox->addWidget(seedLabel);
    QSpinBox *seed = new QSpinBox;
    seed->setRange(0, 10000);
    seed->setSingleStep(1);
    seed->setProperty("type", 1);
    seed->setFocusPolicy(Qt::StrongFocus);
    seed->installEventFilter(new FilterOutUndoRedoEvents);
    extraBox->addWidget(seed);
    connect(seed, SIGNAL(editingFinished()), this->data, SLOT (updateLayoutPar()));
    connect(seed, SIGNAL(editingFinished()), this->viewVZ->OpenGLWidget, SLOT (redraw()));

    // connect for hide
    connect(this, SIGNAL(hideAll()), seed, SLOT(hide()));
    // connect for show
    connect(this, SIGNAL(showPopulation()), seed, SLOT(show()));
    // connect for set value
    connect(this, SIGNAL(setSeed(int)), seed, SLOT(setValue(int)));

    extraBox->addStretch();

}

void viewVZLayoutEditHandler::initConnection() {

    // draw up the panel for a connection
    QVBoxLayout * panelLayout = (QVBoxLayout *) this->viewVZ->panel->layout();

    //QSettings settings;
    //bool devMode = settings.value("dev_mode_on", "false").toBool();

    connectionComboBox = this->addDropBox(panelLayout, "Connectivity", "will_be_overriden");
    this->updateConnectionList();
    /*connectionComboBox->addItem("All to All");
    connectionComboBox->addItem("One to One");
    connectionComboBox->addItem("Fixed Probability");
    connectionComboBox->addItem("Explicit List");
    connectionComboBox->addItem("Distance Based Probability");
    connectionComboBox->addItem("Kernel");
    // add python scripts
    QSettings settings;
    settings.beginGroup("pythonscripts");
    QStringList scripts = settings.childKeys();
    connectionComboBox->addItems(scripts);
    settings.endGroup();
    connect(connectionComboBox, SIGNAL(activated(int)), data, SLOT(updateComponentType(int)));*/

    // connect for hide
    connect(this, SIGNAL(hideAll()), connectionComboBox, SLOT(hide()));
    // connect for show
    connect(this, SIGNAL(showConnection()), connectionComboBox, SLOT(show()));


    // SIZE COMBOBOX FOR KERNEL
    kernelComboBox = new QComboBox;
    kernelComboBox->setProperty("conn", "true");
    kernelComboBox->setToolTip("select kernel size");
    kernelComboBox->setProperty("action","changeConnKerSize");
    kernelComboBox->setFocusPolicy(Qt::StrongFocus);
    kernelComboBox->installEventFilter(new FilterOutUndoRedoEvents);
    for (uint i = 0; i < 5; ++i) {
        qint32 ksize = i *2 + 3;
        QString kstring = QString::number(ksize) + " x " + QString::number(ksize);
        kernelComboBox->addItem(kstring);
    }
    connect(kernelComboBox, SIGNAL(currentIndexChanged(int)), data, SLOT (updatePar()));
    connect(this, SIGNAL(hideAll()), kernelComboBox, SLOT(hide()));
}

void viewVZLayoutEditHandler::updateConnectionList() {

    disconnect(connectionComboBox,0,0,0);
    connectionComboBox->clear();
    connectionComboBox->addItem("All to All");
    connectionComboBox->addItem("One to One");
    connectionComboBox->addItem("Fixed Probability");
    connectionComboBox->addItem("Explicit List");
    connectionComboBox->addItem("Kernel");
    //connectionComboBox->addItem("Python Script");
    // add python scripts
    QSettings settings;
    settings.beginGroup("pythonscripts");
    QStringList scripts = settings.childKeys();
    connectionComboBox->addItems(scripts);
    settings.endGroup();
    connect(connectionComboBox, SIGNAL(activated(int)), data, SLOT(updateComponentType(int)));
}

void viewVZLayoutEditHandler::updateLayoutList(rootData * data) {

    // upadte the layout list
    layoutComboBox->clear();
    // we don't want to set stuff while this occurs
    layoutComboBox->disconnect(data);
    for (unsigned int i = 0; i < data->catalogLayout.size(); ++i) {
        layoutComboBox->addItem(data->catalogLayout[i]->name);

        // safe way of updating the layout list index
        if (data->isValidPointer(viewVZ->currObject))
            if (viewVZ->currObject->type == populationObject)
                if (((population *) viewVZ->currObject)->layoutType->component == data->catalogLayout[i])
                    layoutComboBox->setCurrentIndex(i);
    }

    connect(layoutComboBox, SIGNAL(activated(int)), data, SLOT(updateComponentType(int)));

}

QComboBox* viewVZLayoutEditHandler::addDropBox(QVBoxLayout * layout, QString name, QString type) {

    QHBoxLayout * box = new QHBoxLayout();
    layout->addLayout(box);
    box->addWidget(new QLabel("<b>" + name + ":</b>"));
    if (type == "layout") {
        connect(this, SIGNAL(hideAll()), box->itemAt(0)->widget(), SLOT(hide()));
        connect(this, SIGNAL(showPopulation()), box->itemAt(0)->widget(), SLOT(show()));
    } else {
        connect(this, SIGNAL(hideAll()), box->itemAt(0)->widget(), SLOT(hide()));
        connect(this, SIGNAL(showConnection()), box->itemAt(0)->widget(), SLOT(show()));
    }
    QComboBox *select = new QComboBox();
    select->setFocusPolicy(Qt::StrongFocus);
    select->installEventFilter(new FilterOutUndoRedoEvents);
    select->setProperty("type", type);
    box->addWidget(select);
    if (type == "layout") {
        QPushButton * editButton = new QPushButton("Edit");
        editButton->setFixedWidth(60);
        box->addWidget(editButton);
        QObject::connect(editButton, SIGNAL(clicked()), this, SLOT(redrawLayoutEdit()));
        connect(this, SIGNAL(hideAll()), editButton, SLOT(hide()));
        connect(this, SIGNAL(showPopulation()), editButton, SLOT(show()));
    }
    return select;

}

QFrame * viewVZLayoutEditHandler::getDivider() {

    QFrame * div1 = new QFrame();
    div1->setFrameShape(QFrame::HLine);
    connect(this, SIGNAL(deleteProperties()), div1, SLOT(deleteLater()));
    return div1;

}

void viewVZLayoutEditHandler::togglePlay() {

    if (playBack.isActive()) {

        playBack.stop();
        QPushButton * but = (QPushButton *) sender();
        QCommonStyle style;
        but->setIcon(style.standardIcon(QStyle::SP_MediaPlay));

    } else {

        playBack.setInterval(15);
        playBack.start();
        QPushButton * but = (QPushButton *) sender();
        QCommonStyle style;
        but->setIcon(style.standardIcon(QStyle::SP_MediaPause));

    }

}

void viewVZLayoutEditHandler::playBackTimeout() {

    if (timeSlider->value() < timeSlider->maximum()) {
        timeSlider->setValue(timeSlider->value()+50);
        viewVZ->OpenGLWidget->updateLogDataTime(timeSlider->value());
    } else {
        playBack.stop();
        QCommonStyle style;
        playButton->setIcon(style.standardIcon(QStyle::SP_MediaPlay));
    }

}

void viewVZLayoutEditHandler::clearAll() {

    if (!this->viewVZ->panel->layout()) {
        qDebug() << "wtf - search for #14634";
        return;
    }

    // remove existing stuff first
    clearLayout(this->viewVZ->panel->layout(), "header");
    clearLayout(this->viewVZ->panel->layout(), "body");
    clearLayout(this->viewVZ->panel->layout(), "select");
    clearLayout(this->viewVZ->panel->layout(), "par");

}

void viewVZLayoutEditHandler::redrawFromObject(QString name) {


    // incorrect input - unless we just want a refresh...
    if (name != "" && name != "comboboxOSXfix") {

        this->viewVZ->currObject = this->data->getObjectFromName(name);
    }

    if (this->viewVZ->currObject == (systemObject *)0) {
        // oops, this shouldn't happen
        //cerr << "OOPS - looked up a name for an object that wasn't there: " << name.toStdString() << "\n";
        return;
    }

    if (name == "comboboxOSXfix") {
#ifdef Q_OS_OSX
        // don't clear layout
        redrawHeaders();
#else
        // clear layout (note this used to have a "mode" argument, but that's no longer used and hence this does the same as the Q_OS_MAC case above.
        redrawHeaders();
#endif
    } else {
        redrawHeaders();
    }
}

void viewVZLayoutEditHandler::redrawHeaders()
{
    if (this->viewVZ->treeView == NULL) {
        return;
    }

    emit deleteProperties();
    emit hideAll();
    emit showAll();

    // refresh python scripts
    this->updateConnectionList();

    // configure timeSlider
    for (uint i = 0; i < data->experiments.size(); ++i) {
        if (data->experiments[i]->selected) {
            timeSlider->setMinimum(0);
            timeSlider->setMaximum((int) ((data->experiments[i]->setup.duration * 1000) / data->experiments[i]->setup.dt));
        }
    }

    if (this->viewVZ->currObject != (systemObject *)0) {
        // draw the parameters
        redrawProperties();
        drawDeletables();
    }
}


void viewVZLayoutEditHandler::redrawProperties() {

    if(!data->isValidPointer(this->viewVZ->currObject)) {
        this->viewVZ->currObject = (systemObject *)0;
        return;
    }

    if (this->viewVZ->currObject->type == populationObject) {

        // draw up the panel for a population
        NineMLLayoutData * currLayout = ((population *) this->viewVZ->currObject)->layoutType;

        // show widgets
        emit showPopulation();

        // configure widgets (disconnect where they have 'valueChanged' signals

        sizeSpin->disconnect(this->viewVZ->OpenGLWidget);
        xSpin->disconnect(this->viewVZ->OpenGLWidget);
        ySpin->disconnect(this->viewVZ->OpenGLWidget);
        zSpin->disconnect(this->viewVZ->OpenGLWidget);
        emit setPopulationSize(((population *) this->viewVZ->currObject)->numNeurons);
        emit setPopulationX(((population *) this->viewVZ->currObject)->loc3.x);
        emit setPopulationY(((population *) this->viewVZ->currObject)->loc3.y);
        emit setPopulationZ(((population *) this->viewVZ->currObject)->loc3.z);
        connect(sizeSpin, SIGNAL(valueChanged(int)), this->viewVZ->OpenGLWidget, SLOT (parsChangedPopulation(int)));
        connect(xSpin, SIGNAL(valueChanged(int)), this->viewVZ->OpenGLWidget, SLOT (redraw(int)));
        connect(ySpin, SIGNAL(valueChanged(int)), this->viewVZ->OpenGLWidget, SLOT (redraw(int)));
        connect(zSpin, SIGNAL(valueChanged(int)), this->viewVZ->OpenGLWidget, SLOT (redraw(int)));
        emit setMinDistance(currLayout->minimumDistance);
        emit setSeed(currLayout->seed);

        updateLayoutList(data);

    }

    if (this->viewVZ->currObject->type == inputObject) {

        genericInput * input = (genericInput *) this->viewVZ->currObject;

        // show widgets
        emit showConnection();

        connectionComboBox->setProperty("type", "input");

        if (input->source->type != populationObject || input->destination->type != populationObject) {
            qDebug() << "We are visualising an input not between two pops";
            return;
        }

        population * src = (population *) input->source;
        population * dst = (population *) input->destination;

        // disable 1-2-1 for unequal population sizes
        if (src->numNeurons != dst->numNeurons) {
            QModelIndex ind = connectionComboBox->model()->index(1,0);
            connectionComboBox->model()->setData(ind, QVariant(0), Qt::UserRole-1);
        } else {
            QModelIndex ind = connectionComboBox->model()->index(1,0);
            connectionComboBox->model()->setData(ind, QVariant(1), Qt::UserRole-1);
        }

        // set index
        connectionComboBox->disconnect(data);
        connectionComboBox->setCurrentIndex(input->connectionType->getIndex());
        connect(connectionComboBox, SIGNAL(activated(int)), data, SLOT(updateComponentType(int)));

    }

    if (this->viewVZ->currObject->type == synapseObject) {

        // draw up the panel for a projection
        synapse * syn = (synapse *) this->viewVZ->currObject;

        // show widgets
        emit showConnection();

        connectionComboBox->setProperty("type", "conn");

        population * src = (population *) syn->proj->source;
        population * dst = (population *) syn->proj->destination;

        // disable 1-2-1 for unequal population sizes
        if (src->numNeurons != dst->numNeurons) {
            QModelIndex ind = connectionComboBox->model()->index(1,0);
            connectionComboBox->model()->setData(ind, QVariant(0), Qt::UserRole-1);
        } else {
            QModelIndex ind = connectionComboBox->model()->index(1,0);
            connectionComboBox->model()->setData(ind, QVariant(1), Qt::UserRole-1);
        }

        // set index
        connectionComboBox->disconnect(data);
        connectionComboBox->setCurrentIndex(syn->connectionType->getIndex());
        connect(connectionComboBox, SIGNAL(activated(int)), data, SLOT(updateComponentType(int)));

    }

}

void viewVZLayoutEditHandler::selectionChanged(QItemSelection top, QItemSelection) {

    // look up the selected item
    QModelIndexList indices = top.indexes();
    TreeItem *item = static_cast<TreeItem*>(indices[0].internalPointer());

    // sanity check
    bool found = false;

    for (uint i = 0; i < data->populations.size(); ++i) {

        population * currPop = (population *) data->populations[i];

        // populations
        if (currPop->getName() == item->name) {
            viewVZ->currObject = currPop; found = true;
        }

        // population generic outputs:

        for (uint output = 0; output < data->populations[i]->neuronType->outputs.size(); ++output) {

            genericInput * currOutput = data->populations[i]->neuronType->outputs[output];

            // add output if is not a projection input
            if (!currOutput->projInput) {
                if("Output from " + currOutput->source->getName() + " to " + currOutput->destination->getName() + " port " + currOutput->dstPort + " " + QString::number(output) == item->name) {
                    viewVZ->currObject = currOutput; found = true;
                }
            }

        }

        // projections
        for (uint j = 0; j < currPop->projections.size(); ++j) {

            projection * currProj = (projection *) currPop->projections[j];

            if (currProj->getName() == item->name)
                viewVZ->currObject = currProj; found = true;

            // synapses
            for (uint k = 0; k < currProj->synapses.size(); ++k) {

                synapse * currTarg = (synapse *) currProj->synapses[k];

                if (currProj->getName() + ": Synapse " + QString::number(k) == item->name) {
                    viewVZ->currObject = currTarg; found = true;
                }

            }

        }

    }

    // sanity
    if (!found) {
        qDebug() << "Tried to select a Visualiser object in NL and failed: " << item->name;
    }

    // set the selection in data to our new selection
    data->selList.clear();
    data->selList.push_back(viewVZ->currObject);

    this->redrawHeaders();

}

void viewVZLayoutEditHandler::redrawLayoutEdit() {

    // use deleteLater
    emit hideAll();
    emit hideTree();
    emit deleteProperties();

    // clear existing elements:
    clearLayout(this->viewVZ->panel->layout(), "body");
    clearLayout(this->viewVZ->panel->layout(), "select");

    // create copy of current layout
    this->viewVZ->editLayout = new NineMLLayout(((population *) this->viewVZ->currObject)->layoutType->component);

    // check if 'none'
    if (viewVZ->editLayout->name == "none")
        viewVZ->editLayout->name = "New Layout";

    // we must always have a regime
    if (this->viewVZ->editLayout->RegimeList.size() == 0)
        this->viewVZ->editLayout->RegimeList.push_back(new RegimeSpace());

    // check we have x, y, z and numNeurons
    bool isX = false;
    bool isY = false;
    bool isZ = false;
    bool isNumNeurons = false;
    for (uint i = 0; i < this->viewVZ->editLayout->StateVariableList.size(); ++i) {
        if (this->viewVZ->editLayout->StateVariableList[i]->name == "x")
            isX = true;
        if (this->viewVZ->editLayout->StateVariableList[i]->name == "y")
            isY = true;
        if (this->viewVZ->editLayout->StateVariableList[i]->name == "z")
            isZ = true;
    }

    for (uint i = 0; i < this->viewVZ->editLayout->ParameterList.size(); ++i) {
        if (this->viewVZ->editLayout->ParameterList[i]->name == "numNeurons")
            isNumNeurons = true;
    }

    // add x, y, z and numNeurons if they are not present
    if (!isX) {viewVZ->editLayout->StateVariableList.push_back(new StateVariable); viewVZ->editLayout->StateVariableList.back()->name = "x";}
    if (!isY) {viewVZ->editLayout->StateVariableList.push_back(new StateVariable); viewVZ->editLayout->StateVariableList.back()->name = "y";}
    if (!isZ) {viewVZ->editLayout->StateVariableList.push_back(new StateVariable); viewVZ->editLayout->StateVariableList.back()->name = "z";}
    if (!isNumNeurons) {viewVZ->editLayout->ParameterList.push_back(new Parameter); viewVZ->editLayout->ParameterList.back()->name = "numNeurons";}

    // validate it:
    QStringList err;
    err = this->viewVZ->editLayout->validateComponent();

    if (err.size() > 1) {
        for (uint j = 0; j < (uint) err.size(); ++j) {
            cerr << err[j].toStdString() << "\n";
        }
    }

    // draw in the new objects

    QVBoxLayout * panelLayout = (QVBoxLayout *) this->viewVZ->panel->layout();

    QVBoxLayout * editLayout = new QVBoxLayout;
    editLayout->setProperty("wtype", "body");

    editLayout->setContentsMargins(10,0,0,0);
    //editLayout->setSpacing(20);

    // add parameters and statevariables:

    // PARAMETERS
    QHBoxLayout * varLayout = new QHBoxLayout;
    varLayout->setContentsMargins(0,0,0,0);
    varLayout->setSpacing(0);
    editLayout->addLayout(varLayout);

    varLayout->addWidget(new QLabel("<b>Parameters:</b> numNeurons,"));

    QLineEdit * parameters = new QLineEdit;

    QString parsString;

    // add comma separated parameters to parsString
    for (uint i = 0; i < this->viewVZ->editLayout->ParameterList.size(); ++i) {
        if (this->viewVZ->editLayout->ParameterList[i]->name != "numNeurons")
            parsString += this->viewVZ->editLayout->ParameterList[i]->name + ",";
    }

    // take off trailing comma
    if (parsString.size() > 0)
        parsString.chop(1);

    parameters->setText(parsString);

    // join up
    QObject::connect(parameters, SIGNAL(editingFinished()), this, SLOT(updateParameters()));
    varLayout->addWidget(parameters);

    // STATE VARIABLES
    QHBoxLayout * statevarLayout = new QHBoxLayout;
    statevarLayout->setContentsMargins(0,0,0,0);
    statevarLayout->setSpacing(0);
    editLayout->addLayout(statevarLayout);

    statevarLayout->addWidget(new QLabel("<b>Variables:</b> x,y,z,"));

    QLineEdit * stateVars = new QLineEdit;
    stateVars->setProperty("parentLayout", qVariantFromValue((void *) editLayout));

    QString stateVarsString;

    // add comma separated parameters to parsString
    for (uint i = 0; i < this->viewVZ->editLayout->StateVariableList.size(); ++i) {
        if (this->viewVZ->editLayout->StateVariableList[i]->name != "x" && this->viewVZ->editLayout->StateVariableList[i]->name != "y" && this->viewVZ->editLayout->StateVariableList[i]->name != "z")
            stateVarsString += this->viewVZ->editLayout->StateVariableList[i]->name + ",";
    }

    // take off trailing comma
    if (stateVarsString.size() > 0)
        stateVarsString.chop(1);

    stateVars->setText(stateVarsString);

    // join up
    QObject::connect(stateVars, SIGNAL(editingFinished()), this, SLOT(updateStateVariables()));
    statevarLayout->addWidget(stateVars);

    // ALIASES
    QHBoxLayout * aliasLayout = new QHBoxLayout;
    aliasLayout->setContentsMargins(0,0,0,0);
    aliasLayout->setSpacing(0);
    editLayout->addLayout(aliasLayout);

    aliasLayout->addWidget(new QLabel("<b>Aliases:</b>"));

    // make sure all is aligned
    aliasLayout->addStretch();

    QPushButton * aliases = new QPushButton("Edit");
    aliases->setMaximumWidth(70);

    aliasLayout->addWidget(aliases);

    // join up
    QObject::connect(aliases, SIGNAL(clicked()), this, SLOT(editAliases()));


    // ADD REGIMES

    for (uint i = 0; i < this->viewVZ->editLayout->RegimeList.size(); ++i) {

        editLayout->addLayout(this->drawRegime(this->viewVZ->editLayout->RegimeList[i]));

    }

    panelLayout->insertLayout(0,editLayout);

    // add a title:
    QHBoxLayout * topBar = new QHBoxLayout;
    topBar->setProperty("wtype", "body");

    QLabel * title = new QLabel("<b>Editing: " + this->viewVZ->editLayout->name + "</b>");
    topBar->addWidget(title);

    QPushButton * preview = new QPushButton("Preview");
    preview->setMaximumWidth(70);
    // connect up:
    QObject::connect(preview, SIGNAL(clicked()), this, SLOT(previewEditedLayout()));
    topBar->addWidget(preview);

    QPushButton * discard = new QPushButton("Discard");
    discard->setMaximumWidth(70);
    // connect up:
    QObject::connect(discard, SIGNAL(clicked()), this, SLOT(discardEditedLayout()));
    topBar->addWidget(discard);

    QPushButton * save = new QPushButton("Save");
    save->setMaximumWidth(70);
    // connect up:
    QObject::connect(save, SIGNAL(clicked()), this, SLOT(saveEditedLayout()));
    topBar->addWidget(save);

    panelLayout->insertLayout(0,topBar);

    this->viewVZ->errors->show();


}

void viewVZLayoutEditHandler::drawDeletables() {

    if (this->viewVZ->currObject == NULL)
        return;

    // get a handle to the layout for drawing
   QVBoxLayout * panelLayout = (QVBoxLayout *) this->viewVZ->panel->layout();

    if (this->viewVZ->currObject->type == populationObject) {

        // draw up the panel for a population
        NineMLLayoutData * currLayout = ((population *) this->viewVZ->currObject)->layoutType;

        // draw up the parameters:
        for (int j = 0; j < 2; ++j) {

            // configure:
            QString name;
            QString parType;
            QString boxTitle;
            int listSize = -1;
            double value = -1;
            if (j == 0) {
                parType = "Par";
                boxTitle = "<b>Parameters</b>";
                listSize = currLayout->ParameterList.size();
            }
            if (j == 1) {
                parType = "State";
                boxTitle = "<b>State variable initial values</b>";
                listSize = currLayout->StateVariableList.size();
            }

            // now draw based upon the selected type

            if ((j == 0 && listSize > 1) || (j == 1 && listSize > 3) || (j == 2 && listSize > 0)) {

                QLabel * label = new QLabel(boxTitle);
                panelLayout->insertWidget(panelLayout->count() - 2, label);
                connect(this, SIGNAL(deleteProperties()), label, SLOT(deleteLater()));

                QFormLayout * varLayout = new QFormLayout();
                panelLayout->insertLayout(panelLayout->count() - 2, varLayout);
                connect(this, SIGNAL(deleteProperties()), varLayout, SLOT(deleteLater()));


                for (int l = 0; l < listSize; ++l) {
                    ParameterData * currPar;
                    if (j == 0) {
                        name = currLayout->ParameterList[l]->name;
                        if (name == "numNeurons" && currLayout->type == NineMLLayoutType) continue;
                        value = currLayout->ParameterList[l]->value[0];
                        currPar = (ParameterData *) currLayout->ParameterList[l];
                    }
                    if (j == 1) {
                        name = currLayout->StateVariableList[l]->name;
                        value = currLayout->StateVariableList[l]->value[0];
                        if (name == "x" && currLayout->type == NineMLLayoutType) continue;
                        if (name == "y" && currLayout->type == NineMLLayoutType) continue;
                        if (name == "z" && currLayout->type == NineMLLayoutType) continue;
                        currPar = (ParameterData *) currLayout->StateVariableList[l];
                    }

                    QDoubleSpinBox *parSpin = new QDoubleSpinBox;
                    parSpin->setRange(-200000, 200000);
                    parSpin->setSingleStep(0.1);
                    parSpin->setDecimals(4);
                    parSpin->setValue(value);
                    parSpin->setSuffix(" ");
                    parSpin->setFocusPolicy(Qt::StrongFocus);
                    parSpin->installEventFilter(new FilterOutUndoRedoEvents);



                    if (j == 0)
                        parSpin->setProperty("ptr", qVariantFromValue((void *) currPar));
                    if (j == 1)
                        parSpin->setProperty("ptr", qVariantFromValue((void *) currPar));

                    varLayout->addRow(name, parSpin);
                    connect(this, SIGNAL(deleteProperties()), varLayout->itemAt(varLayout->rowCount()-1, QFormLayout::LabelRole)->widget(), SLOT(deleteLater()));
                    connect(this, SIGNAL(deleteProperties()), varLayout->itemAt(varLayout->rowCount()-1, QFormLayout::FieldRole)->widget(), SLOT(deleteLater()));

                    parSpin->setProperty("action", "changeVal");
                    parSpin->setProperty("valToChange", 0);
                    connect(parSpin, SIGNAL(editingFinished()), this->data, SLOT (updatePar()));
                    connect(parSpin, SIGNAL(editingFinished()), this->viewVZ->OpenGLWidget, SLOT (parsChangedPopulation()));
                    parSpin->setProperty("type","layout" + parType);
                    varLayout->itemAt(varLayout->rowCount()-1,QFormLayout::LabelRole)->widget()->setProperty("type","layout" + parType);

                }
            }
        }
    }


    if (this->viewVZ->currObject->type == synapseObject || this->viewVZ->currObject->type == inputObject) {

        // for now we only handle discrete tables, so leave this blank

        // HACK - quick table:
        connection * currConn;
        if (this->viewVZ->currObject->type == synapseObject) {
            synapse * currSyn = (synapse *) this->viewVZ->currObject;
            currConn = currSyn->connectionType;
        } else {
            genericInput * currIn = (genericInput *) this->viewVZ->currObject;
            currConn = currIn->connectionType;
        }

        // change display options based on type of connection
        if (currConn->type != CSV || ((csv_connection *) currConn)->generator != NULL) {

            panelLayout->insertWidget(panelLayout->count() - 2, getDivider(),2);

            // select src & dst to show
            QHBoxLayout * hlay = new QHBoxLayout;
            connect(this, SIGNAL(deleteProperties()), hlay, SLOT(deleteLater()));

            hlay->addWidget(new QLabel("Show connections of index "));
            connect(this, SIGNAL(deleteProperties()), hlay->itemAt(0)->widget(), SLOT(deleteLater()));

            QSpinBox *index = new QSpinBox;
            index->setRange(0, 10000000);
            index->setMaximumWidth(60);
            index->setProperty("type", "index");
            index->setFocusPolicy(Qt::StrongFocus);
            index->installEventFilter(new FilterOutUndoRedoEvents);
            connect(index, SIGNAL(valueChanged(int)), viewVZ->OpenGLWidget, SLOT (selectedNrnChanged(int)));
            connect(this, SIGNAL(deleteProperties()), index, SLOT(deleteLater()));
            hlay->addWidget(index);

            hlay->addWidget(new QLabel(" from population "));
            connect(this, SIGNAL(deleteProperties()), hlay->itemAt(2)->widget(), SLOT(deleteLater()));

            QComboBox *from = new QComboBox;
            from->addItem("source");
            from->addItem("destination");
            from->setCurrentIndex(0);
            from->setMaximumWidth(200);
            from->setProperty("type", "from");
            from->setFocusPolicy(Qt::StrongFocus);
            from->installEventFilter(new FilterOutUndoRedoEvents);
            connect(from, SIGNAL(currentIndexChanged(int)), viewVZ->OpenGLWidget, SLOT (selectedNrnChanged(int)));
            connect(this, SIGNAL(deleteProperties()), from, SLOT(deleteLater()));
            hlay->addWidget(from);

            panelLayout->insertLayout(panelLayout->count() - 2, hlay,2);

            panelLayout->insertWidget(panelLayout->count() - 2, getDivider(),2);

        }

        if (currConn->type == FixedProb || currConn->type == AlltoAll || currConn->type == OnetoOne || currConn->type == CSV) {

            // draw up probability changer
            QLayout * lay = currConn->drawLayout(this->data, this, NULL);
            panelLayout->insertLayout(panelLayout->count() - 2, lay,2);

        }

        if (currConn->type == Kernel) {

            // draw up kernel size and scale
            QHBoxLayout * hlay = new QHBoxLayout;
            connect(this, SIGNAL(deleteProperties()), hlay, SLOT(deleteLater()));

            // SIZE CONFIGURATION
            kernelComboBox->setProperty("ptr", qVariantFromValue((void *) currConn));
            int index = (((kernel_connection *) currConn)->kernel_size - 3)/2;
            kernelComboBox->setCurrentIndex(index);
            kernelComboBox->show();

            // SCALE
            QDoubleSpinBox *scaleWidget = new QDoubleSpinBox;
            scaleWidget->setProperty("conn", "true");
            scaleWidget->setToolTip("select kernel scale");
            scaleWidget->setMinimum(0.1);
            scaleWidget->setMaximum(100.0);
            scaleWidget->setValue(((kernel_connection *) currConn)->kernel_scale);
            scaleWidget->setProperty("ptr", qVariantFromValue((void *) currConn));
            scaleWidget->setProperty("action","changeConnKerScale");
            scaleWidget->setFocusPolicy(Qt::StrongFocus);
            scaleWidget->installEventFilter(new FilterOutUndoRedoEvents);
            connect(scaleWidget, SIGNAL(valueChanged(double)), data, SLOT (updatePar()));
            //

            hlay->addWidget(new QLabel("Kernel size: "));
            connect(this, SIGNAL(deleteProperties()), hlay->itemAt(hlay->count()-1)->widget(), SLOT(deleteLater()));
            hlay->addWidget(kernelComboBox);
            hlay->addWidget(new QLabel("Kernel scale: "));
            connect(this, SIGNAL(deleteProperties()), hlay->itemAt(hlay->count()-1)->widget(), SLOT(deleteLater()));
            hlay->addWidget(scaleWidget);
            connect(this, SIGNAL(deleteProperties()), scaleWidget, SLOT(deleteLater()));

            panelLayout->insertLayout(panelLayout->count() - 2, hlay,2);

            QGridLayout *glay = new QGridLayout;
            connect(this, SIGNAL(deleteProperties()), glay, SLOT(deleteLater()));
            glay->setContentsMargins(0,0,0,0);
            glay->setSpacing(0);
            QLabel * kernBoxLabel = new QLabel("Kernel: ");
            glay->addWidget(kernBoxLabel,0,0,1,1);
            connect(this, SIGNAL(deleteProperties()), kernBoxLabel, SLOT(deleteLater()));
            QDoubleSpinBox * kernel;
            // add kernel
            for (int i = 0; i < ((kernel_connection *) currConn)->kernel_size; ++i) {
                for (int j = 0; j < ((kernel_connection *) currConn)->kernel_size; ++j) {
                    kernel = new QDoubleSpinBox;
                    kernel->setMinimum(0);
                    kernel->setMaximum(1.0);
                    kernel->setProperty("i",i);
                    kernel->setProperty("j",j);
                    kernel->setFocusPolicy(Qt::StrongFocus);
                    kernel->installEventFilter(new FilterOutUndoRedoEvents);
                    kernel->setProperty("ptr", qVariantFromValue((void *) currConn));
                    kernel->setProperty("action","changeConnKernel");
                    kernel->setValue(((kernel_connection *) currConn)->kernel[i][j]);
                    connect(kernel, SIGNAL(valueChanged(double)), data, SLOT (updatePar()));
                    connect(this, SIGNAL(deleteProperties()), kernel, SLOT(deleteLater()));
                    glay->addWidget(kernel,i,j+1,1,1);
                }
            }

            panelLayout->insertLayout(panelLayout->count() - 2, glay,2);

            QCheckBox * convert = new QCheckBox("Output as explicit list");
            connect(this, SIGNAL(deleteProperties()), convert, SLOT(deleteLater()));
            convert->setChecked(((kernel_connection *)currConn)->isList());
            if (this->viewVZ->currObject->type == synapseObject) {
                synapse * currSyn = (synapse *) this->viewVZ->currObject;
                convert->setProperty("ptrSrc", qVariantFromValue((void *) currSyn->proj->source));
                convert->setProperty("ptrDst", qVariantFromValue((void *) currSyn->proj->destination));
            } else {
                genericInput * currIn = (genericInput *) this->viewVZ->currObject;
                convert->setProperty("ptrSrc", qVariantFromValue((void *) currIn->source));
                convert->setProperty("ptrDst", qVariantFromValue((void *) currIn->destination));
            }
            connect(convert, SIGNAL(toggled(bool)), currConn, SLOT (convertToList(bool)));

            panelLayout->insertWidget(panelLayout->count() - 2, convert,2);

            QPushButton * generate = new QPushButton("Generate");
            connect(this, SIGNAL(deleteProperties()), generate, SLOT(deleteLater()));
            generate->setProperty("ptr", qVariantFromValue((void *) currConn));
            connect(generate, SIGNAL(clicked()), viewVZ->OpenGLWidget, SLOT (parsChangedProjection()));

            panelLayout->insertWidget(panelLayout->count() - 2, generate,2);

        }

        if (currConn->type == Python) {


            //}
            // add the text edit to the main layout
            //panelLayout->insertWidget(panelLayout->count() -2, scriptEdit, 2);

        }

        this->viewVZ->OpenGLWidget->setConnType(currConn->type);

    }

}

void viewVZLayoutEditHandler::disableButton() {

    QPushButton * button = (QPushButton *) sender()->property("buttonToDisable").value<void *>();
    button->setEnabled(false);

}

QHBoxLayout * viewVZLayoutEditHandler::drawRegime(RegimeSpace * srcRegime) {

    QHBoxLayout * regimeSurround = new QHBoxLayout();
    regimeSurround->setContentsMargins(0,0,0,0);

    // add a block of colour to differentiate the extent of the regime
    QLabel * block = new QLabel("");
    block->setStyleSheet("QLabel { background: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:1 rgba(200, 200, 200, 255), stop:0 rgba(200,200,200,0)); }");
    block->setFixedWidth(0);
    regimeSurround->addWidget(block);

    QVBoxLayout * regimeLay = new QVBoxLayout();
    regimeSurround->setSpacing(0);
    regimeSurround->addLayout(regimeLay);

    // add another block of colour to differentiate the extent of the regime
    QLabel * blockEnd = new QLabel("");
    blockEnd->setStyleSheet("QLabel { background: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0 rgba(200, 200, 200, 255), stop:1 rgba(200,200,200,0)); }");
    blockEnd->setFixedWidth(0);
    regimeSurround->addWidget(blockEnd);

    // add a bar at the top
    QLabel * bar1 = new QLabel("");
    bar1->setStyleSheet("QLabel { background: rgba(250, 250, 250, 255); }");
    bar1->setMaximumHeight(2);
    regimeLay->addWidget(bar1);

    QHBoxLayout * regTitle = new QHBoxLayout;
    regimeLay->addLayout(regTitle);

    regTitle->addWidget(new QLabel("<b>Transforms</b>"));

    // add a bar under the title:
    QLabel * bar2b = new QLabel("");
    bar2b->setStyleSheet("QLabel { background: rgba(200, 200, 200, 0); }");
    bar2b->setMaximumHeight(6);
    regimeLay->addWidget(bar2b);

    // draw transforms up in order
    vector < int > order;
    for (uint j = 0; j < srcRegime->TransformList.size(); ++j) {
        if (srcRegime->TransformList[j]->order > (int) order.size()) order.resize(srcRegime->TransformList[j]->order);
        order[srcRegime->TransformList[j]->order-1] = j;
    }

    QVBoxLayout * transformsLayout = new QVBoxLayout();
    transformsLayout->setContentsMargins(0,0,0,0);
    transformsLayout->setProperty("container", "regime");
    regimeLay->addLayout(transformsLayout);

    for (uint j = 0; j < order.size(); ++j) {

        // send a pointer to the container as a QVariant
        QVariant transContainer = QVariant(qVariantFromValue((void *) srcRegime));
        transformsLayout->addLayout(this->drawTransform(transContainer, srcRegime->TransformList[order[j]]));

    }

    // add new
    QPushButton * addTr = new QPushButton("Transform");
    addTr->setIcon(QIcon(":/icons/toolbar/addShad.png"));
    addTr->setProperty("ptr", qVariantFromValue((void *) srcRegime));
    addTr->setProperty("parentLayout", qVariantFromValue((void *) transformsLayout));
    addTr->setMaximumWidth(110);
    addTr->setFlat(true);
    addTr->setFocusPolicy(Qt::NoFocus);
    addTr->setToolTip("Add a Transform to update the current location");
    addTr->setFont(addFont);

    // connect up:
    QObject::connect(addTr, SIGNAL(clicked()), this, SLOT(addTransform()));
    transformsLayout->addWidget(addTr, Qt::AlignRight);

    // add a bar under the transforms:
    QLabel * bar3 = new QLabel("");
    bar3->setStyleSheet("QLabel { background: rgba(200, 200, 200, 255); }");
    bar3->setMaximumHeight(6);
    regimeLay->addWidget(bar3);

    // add the OnConditions

    QVBoxLayout * oncondsLayout = new QVBoxLayout();
    oncondsLayout->setContentsMargins(0,10,0,0);
    oncondsLayout->setSpacing(10);
    regimeLay->addLayout(oncondsLayout);


    return regimeSurround;

}

void viewVZLayoutEditHandler::changeTypeOfTransition(QString type) {

    QComboBox * src = (QComboBox *) sender();

    Transform * trans = (Transform *) src->property("ptr").value<void *>();

    NineMLLayout * lay = this->viewVZ->editLayout;

    // find pointer to new variable
    StateVariable * var;
    bool found = false;
    for (uint i = 0; i < lay->StateVariableList.size(); ++i) {
        if (type == lay->StateVariableList[i]->name) {
            var = lay->StateVariableList[i];
            found = true;
        }
    }
    if (!found) return; // OOPS HERE - SORT OUT

    trans->variableName = type;
    trans->variable = var;

}

void viewVZLayoutEditHandler::changeTransitionMaths() {


    QLineEdit * src = (QLineEdit *) sender();

    Transform * trans = (Transform *) src->property("ptr").value<void *>();

    NineMLLayout * lay = this->viewVZ->editLayout;

    QString maths = src->text();

    trans->maths->equation = maths;

    QStringList errs;

    trans->maths->validateMathInLine(lay, &errs);

    viewVZeditDisplayErrors(errs);

    if (errs.size() > 0) {
        QPalette p = src->palette();
        p.setColor( QPalette::Normal, QPalette::Base, QColor(255, 200, 200) );
        src->setPalette(p);
    } else {
        QPalette p = src->palette();
        p.setColor( QPalette::Normal, QPalette::Base, QColor(255, 255, 255) );
        src->setPalette(p);
    }


}

QHBoxLayout * viewVZLayoutEditHandler::drawTransform(QVariant transContainer, Transform * srcTrans) {

    QHBoxLayout * transLay = new QHBoxLayout();
    transLay->setContentsMargins(0,0,0,0);
    transLay->setSpacing(1);

    // add axis list
    QComboBox * trans = new QComboBox;

    trans->setProperty("ptr", qVariantFromValue((void *) srcTrans));
    trans->setProperty("typeCBox", "stateVar");

    // load in the options, and set the current one
    for (uint i = 0; i < this->viewVZ->editLayout->StateVariableList.size(); ++i) {

        trans->addItem(this->viewVZ->editLayout->StateVariableList[i]->name);
        if (this->viewVZ->editLayout->StateVariableList[i]->name == srcTrans->variable->name)
            trans->setCurrentIndex(i);

    }

    trans->setMaximumWidth(70);

    // connect up:
    QObject::connect(trans, SIGNAL(activated(QString)), this, SLOT(changeTypeOfTransition(QString)));

    // add to layout:
    transLay->addWidget(trans);

    // Maths:
    QLineEdit * maths = new QLineEdit;
    maths->setProperty("ptr", qVariantFromValue((void *) srcTrans));
    maths->setText(srcTrans->maths->equation);

    // connect up:
    QObject::connect(maths, SIGNAL(editingFinished()), this, SLOT(changeTransitionMaths()));

    transLay->addWidget(maths);

    // Move order down:
    QPushButton * ordDown = new QPushButton("");
    ordDown->setIcon(QIcon(":/icons/toolbar/up.png"));
    ordDown->setProperty("ptr", qVariantFromValue((void *) srcTrans));
    ordDown->setProperty("regptr", transContainer);
    ordDown->setProperty("parentLayout", qVariantFromValue((void *) transLay));
    ordDown->setFlat(true);
    ordDown->setMaximumWidth(28);
    ordDown->setMaximumHeight(28);
    ordDown->setFocusPolicy(Qt::NoFocus);

    // connect up:
    QObject::connect(ordDown, SIGNAL(clicked()), this, SLOT(transformOrderDown()));
    transLay->addWidget(ordDown);

    // Move order up:
    QPushButton * ordUp = new QPushButton("");
    ordUp->setIcon(QIcon(":/icons/toolbar/down.png"));
    ordUp->setProperty("ptr", qVariantFromValue((void *) srcTrans));
    ordUp->setProperty("regptr", transContainer);
    ordUp->setProperty("parentLayout", qVariantFromValue((void *) transLay));
    ordUp->setFlat(true);
    ordUp->setMaximumWidth(28);
    ordUp->setMaximumHeight(28);
    ordUp->setFocusPolicy(Qt::NoFocus);

    // connect up:
    QObject::connect(ordUp, SIGNAL(clicked()), this, SLOT(transformOrderUp()));
    transLay->addWidget(ordUp);

    // Remove:
    QPushButton * del = new QPushButton;
    del->setIcon(QIcon(":/icons/toolbar/delShad.png"));
    del->setProperty("ptr", qVariantFromValue((void *) srcTrans));
    del->setProperty("regptr", transContainer);
    del->setProperty("parentLayout", qVariantFromValue((void *) transLay));
    del->setFlat(true);
    del->setMaximumWidth(28);
    del->setMaximumHeight(28);
    del->setFocusPolicy(Qt::NoFocus);

    // connect up:
    QObject::connect(del, SIGNAL(clicked()), this, SLOT(deleteTransform()));
    transLay->addWidget(del);

    return transLay;

}



void viewVZLayoutEditHandler::transformOrderDown() {

    QComboBox * src = (QComboBox *) sender();

    Transform * trans = (Transform *) src->property("ptr").value<void *>();

    QHBoxLayout * parentLayout = (QHBoxLayout *) src->property("parentLayout").value<void *>();
    QVBoxLayout * regimeLayout = (QVBoxLayout *) parentLayout->parent();

    if (regimeLayout->property("container") == "regime") {

        RegimeSpace * regime = (RegimeSpace *) src->property("regptr").value<void *>();

        if (trans->order != 1) {

            // move the layout:
            regimeLayout->takeAt(trans->order-1);
            parentLayout->setParent(NULL); // sorts out parent reassignment clash
            regimeLayout->insertLayout(trans->order-2, parentLayout);

            // change the order:
            for (uint i = 0; i < regime->TransformList.size(); ++i){
                if (regime->TransformList[i]->order == trans->order-1)
                    regime->TransformList[i]->order = trans->order;
            }
            --(trans->order);

        }
    }

    if (regimeLayout->property("container") == "oncond") {

        OnConditionSpace * oncond = (OnConditionSpace *) src->property("regptr").value<void *>();

        if (trans->order != 1) {

            // move the layout:
            /*QLayoutItem * temp;
            temp = */regimeLayout->takeAt(trans->order-1);
            parentLayout->setParent(NULL); // sorts out parent reassignment clash
            regimeLayout->insertLayout(trans->order-2, parentLayout);

            // change the order:
            for (uint i = 0; i < oncond->TransformList.size(); ++i){

                if (oncond->TransformList[i]->order == trans->order-1)
                    oncond->TransformList[i]->order = trans->order;
            }
            --(trans->order);

        }
    }

}

void viewVZLayoutEditHandler::transformOrderUp() {

    QComboBox * src = (QComboBox *) sender();

    Transform * trans = (Transform *) src->property("ptr").value<void *>();
    QHBoxLayout * parentLayout = (QHBoxLayout *) src->property("parentLayout").value<void *>();
    QVBoxLayout * regimeLayout = (QVBoxLayout *) parentLayout->parent();

    if (regimeLayout->property("container") == "regime") {

        RegimeSpace * regime = (RegimeSpace *) src->property("regptr").value<void *>();

        if (trans->order != (int) regime->TransformList.size()) {

            // move the layout:
            /*QLayoutItem * temp;
            temp = */regimeLayout->takeAt(trans->order-1);
            parentLayout->setParent(NULL); // sorts out parent reassignment clash
            regimeLayout->insertLayout(trans->order, parentLayout);

            // change the order:
            for (uint i = 0; i < regime->TransformList.size(); ++i){
                if (regime->TransformList[i]->order == trans->order+1)
                    regime->TransformList[i]->order = trans->order;
            }
            ++trans->order;

        }
    }

    if (regimeLayout->property("container") == "oncond") {

        OnConditionSpace * oncond = (OnConditionSpace *) src->property("regptr").value<void *>();

        if (trans->order != (int) oncond->TransformList.size()) {

            // move the layout:
            /*QLayoutItem * temp;
            temp = */regimeLayout->takeAt(trans->order-1);
            parentLayout->setParent(NULL); // sorts out parent reassignment clash
            regimeLayout->insertLayout(trans->order, parentLayout);

            // change the order:
            for (uint i = 0; i < oncond->TransformList.size(); ++i){
                if (oncond->TransformList[i]->order == trans->order+1)
                    oncond->TransformList[i]->order = trans->order;
            }
            ++trans->order;

        }
    }

}

void viewVZLayoutEditHandler::deleteTransform() {

    QComboBox * src = (QComboBox *) sender();

    Transform * trans = (Transform *) src->property("ptr").value<void *>();
    QHBoxLayout * parentLayout = (QHBoxLayout *) src->property("parentLayout").value<void *>();
    QVBoxLayout * regimeLayout = (QVBoxLayout *) parentLayout->parent();

    if (regimeLayout->property("container") == "regime") {

        RegimeSpace * regime = (RegimeSpace *) src->property("regptr").value<void *>();

        int removedOrder = trans->order;
        trans->order = -100;

        // apply to regime
        for (uint i = 0; i < regime->TransformList.size(); ++i) {
            if (regime->TransformList[i]->order == -100)
                regime->TransformList.erase(regime->TransformList.begin()+i, regime->TransformList.begin()+i+1);
        }

        // redo order
        for (uint i = 0; i < regime->TransformList.size(); ++i) {
            if (regime->TransformList[i]->order > removedOrder)
                --(regime->TransformList[i]->order);
        }

    }

    // remove the transform itself:
    delete trans;

    // now remove the widgets and source layout
    QLayoutItem * item;
    while ((item = parentLayout->takeAt(0))) {
        if (item->widget())
            item->widget()->deleteLater();
        if (item->layout())
            item->layout()->deleteLater();
    }
    parentLayout->deleteLater();

}

void viewVZLayoutEditHandler::addTransform() {

    QPushButton * src = (QPushButton *) sender();

    QVBoxLayout * parentLayout = (QVBoxLayout *) src->property("parentLayout").value<void *>();

    if (parentLayout->property("container") == "regime") {

        RegimeSpace * regime = (RegimeSpace *) src->property("ptr").value<void *>();

        NineMLLayout * lay = this->viewVZ->editLayout;

        // add the transform to the transform list for the regime:
        regime->TransformList.resize(regime->TransformList.size()+1, new Transform);
        regime->TransformList.back()->maths = new MathInLine;
        regime->TransformList.back()->maths->equation = "0";
        regime->TransformList.back()->variableName = "x";
        regime->TransformList.back()->type = TRANSLATE;
        regime->TransformList.back()->order = regime->TransformList.size();

        QStringList errs;
        regime->TransformList.back()->validateTransform(lay, &errs);

        this->viewVZeditDisplayErrors(errs);

        // now draw the transform
        // send a pointer to the container as a QVariant
        QVariant transContainer = qVariantFromValue((void *) regime);
        parentLayout->insertLayout(regime->TransformList.size()-1, this->drawTransform(transContainer, regime->TransformList.back()));

    }

}

void recursiveDeleteLater(QLayout * parentLayout) {

    QLayoutItem * item;
    while ((item = parentLayout->takeAt(0))) {
        if (item->widget())
            item->widget()->deleteLater();
        if (item->layout())
            recursiveDeleteLater(item->layout());
        delete item;
    }
    parentLayout->deleteLater();

}

void viewVZLayoutEditHandler::updateStateVariableRefs(QLayout * lay) {

    QLayoutItem * item;

    for (uint i = 0; i < (uint) lay->count(); ++i) {

        item = lay->itemAt(i);

        if (item->layout()) {
            updateStateVariableRefs(item->layout());
        }
        if (item->widget()) {
            // see if the widget is a combobox
            QString name = "QComboBox";
            if (name.contains(item->widget()->metaObject()->className())) {

                // we have a combobox, check the type is a regime holding one, and then update it with new regimes

                if (item->widget()->property("typeCBox").toString() == "stateVar") {

                    // get names and set index
                    QComboBox * tempCBox = (QComboBox *) item->widget();
                    tempCBox->clear();

                    Transform * srcTr = (Transform *) tempCBox->property("ptr").value<void *>();

                    // add options:
                    for (uint j = 0; j < this->viewVZ->editLayout->StateVariableList.size(); ++j) {

                        tempCBox->addItem(this->viewVZ->editLayout->StateVariableList[j]->name);
                        if (this->viewVZ->editLayout->StateVariableList[j]->name == srcTr->variable->name)
                            tempCBox->setCurrentIndex(j);

                    }
                }
            }
        }

    }

}

void viewVZLayoutEditHandler::updateParameters() {

    QLineEdit * src = (QLineEdit *) sender();

    QString text = src->text();

    // clear whitespace
    text.replace(" ", "");

    // tokenize
    QStringList pars = text.split(",");

    // remove empty tokens:
    for (uint i=0; i < (uint) pars.size(); ++i) {
        if (pars[i].size() == 0){
            pars.removeAt(i);
            --i;}
    }

    pars.push_back("numNeurons");

    // un duplicate and warn
    int num = pars.count();
    pars.removeDuplicates();

    QStringList errs;
    if (num != pars.count())
    {

        errs.push_back("Duplicate parameters found, removing...");
        this->viewVZeditDisplayErrors(errs);

    }

    if (errs.size() > 0) {
        QPalette p = src->palette();
        p.setColor( QPalette::Normal, QPalette::Base, QColor(255, 200, 200) );
        src->setPalette(p);
    } else {
        QPalette p = src->palette();
        p.setColor( QPalette::Normal, QPalette::Base, QColor(255, 255, 255) );
        src->setPalette(p);
    }

    // now match and remove all pars
    for (uint i = 0; i < this->viewVZ->editLayout->ParameterList.size(); ++i) {

        bool found = false;

        for (uint j = 0; j < (uint) pars.size(); ++j) {
            if (pars[j] == this->viewVZ->editLayout->ParameterList[i]->name) {
                found = true;
                pars.removeAt(j);
            }
        }

        // if par is not there, add it
        if (!found) {
            this->viewVZ->editLayout->ParameterList.erase(this->viewVZ->editLayout->ParameterList.begin()+i, this->viewVZ->editLayout->ParameterList.begin()+i+1);
            --i;
        }

    }

    // now add left over pars
    for (uint j = 0; j < (uint) pars.size(); ++j) {
        this->viewVZ->editLayout->ParameterList.push_back(new Parameter);
        this->viewVZ->editLayout->ParameterList.back()->name = pars[j];
    }

}

void viewVZLayoutEditHandler::updateStateVariables() {

    QLineEdit * src = (QLineEdit *) sender();

    QString text = src->text();

    // clear whitespace
    text.replace(" ", "");

    // tokenize
    QStringList pars = text.split(",");

    // remove empty tokens:
    for (uint i=0; i < (uint) pars.size(); ++i) {
        if (pars[i].size() == 0){
            pars.removeAt(i);
            --i;}
    }

    pars.push_back("x");
    pars.push_back("y");
    pars.push_back("z");

    // un duplicate and warn
    int num = pars.count();
    pars.removeDuplicates();

    QStringList errs;
    if (num != pars.count())
    {
        errs.push_back("Duplicate parameters found, removing...");
        this->viewVZeditDisplayErrors(errs);
    }

    if (errs.size() > 0) {
        QPalette p = src->palette();
        p.setColor( QPalette::Normal, QPalette::Base, QColor(255, 200, 200) );
        src->setPalette(p);
    } else {
        QPalette p = src->palette();
        p.setColor( QPalette::Normal, QPalette::Base, QColor(255, 255, 255) );
        src->setPalette(p);
    }

    // now match and remove all pars
    for (uint i = 0; i < this->viewVZ->editLayout->StateVariableList.size(); ++i) {

        bool found = false;

        for (uint j = 0; j < (uint) pars.size(); ++j) {
            if (pars[j] == this->viewVZ->editLayout->StateVariableList[i]->name) {
                found = true;
                pars.removeAt(j);
            }
        }

        // if par is not there, add it
        if (!found) {
            this->viewVZ->editLayout->StateVariableList.erase(this->viewVZ->editLayout->StateVariableList.begin()+i, this->viewVZ->editLayout->StateVariableList.begin()+i+1);
            --i;
        }

    }

    // now add left over pars
    for (uint j = 0; j < (uint) pars.size(); ++j) {
        this->viewVZ->editLayout->StateVariableList.push_back(new StateVariable);
        this->viewVZ->editLayout->StateVariableList.back()->name = pars[j];
    }

    // and redo the state vars
    updateStateVariableRefs(this->viewVZ->panel->layout());

}

void viewVZLayoutEditHandler::editAliases() {

    LayoutAliasEditDialog editor(this->viewVZ->editLayout);
    editor.exec();

}

void viewVZLayoutEditHandler::discardEditedLayout() {

    // remove the edited version from memory
    delete this->viewVZ->editLayout;

    // clear existing elements:
    clearLayout(this->viewVZ->panel->layout(), "body");
    clearLayout(this->viewVZ->panel->layout(), "select");

    // go to standard viewVZ:
    redrawFromObject(this->viewVZ->currObject->getName());
    viewVZ->treeView->show();

    this->viewVZ->OpenGLWidget->clearLocations();
    this->viewVZ->OpenGLWidget->redraw();

    // remove error box
    this->viewVZ->errors->hide();

}

void viewVZLayoutEditHandler::saveEditedLayout() {

    // dialog for new name:
    bool ok;
    QString text = QInputDialog::getText((QWidget *)this->parent(), tr("Enter new layout name"),
                                         tr("Layout name:"), QLineEdit::Normal,
                                         this->viewVZ->editLayout->name, &ok);
    if (ok && !text.isEmpty()) {

        this->viewVZ->editLayout->name = text;
        bool saved = false;
        // overwrite
        for (uint i = 0; i < this->data->catalogLayout.size(); ++i) {
            if (this->data->catalogLayout[i]->name == text) {
                NineMLLayout * test = data->catalogLayout[i];
                data->catalogLayout[i] = this->viewVZ->editLayout;
                delete ((population *) this->viewVZ->currObject)->layoutType;
                ((population *) this->viewVZ->currObject)->layoutType = new NineMLLayoutData(data->catalogLayout[i]);
                delete test;
                saved = true;
            }
        }
        if (!saved) {
            // save new
            data->catalogLayout.push_back(this->viewVZ->editLayout);
        }

    }

    // clear existing elements:
    clearLayout(this->viewVZ->panel->layout(), "body");
    clearLayout(this->viewVZ->panel->layout(), "select");

    // go to standard viewVZ:
    redrawFromObject(this->viewVZ->currObject->getName());
    viewVZ->treeView->show();

    this->viewVZ->OpenGLWidget->clearLocations();
    this->viewVZ->OpenGLWidget->redraw();

    // remove error box
    this->viewVZ->errors->hide();

}

void viewVZLayoutEditHandler::previewEditedLayout() {

    // validate it:
    QStringList err;
    err = this->viewVZ->editLayout->validateComponent();

    err.removeLast();
    if (err.size() > 0) {
        viewVZeditDisplayErrors(err);
        return;
    }

    // no errors, show preview dialog:
    layoutEditPreviewDialog previewDialog(this->viewVZ->editLayout, this->viewVZ->OpenGLWidget);
    previewDialog.exec();


}

void viewVZLayoutEditHandler::viewVZeditDisplayErrors(QStringList errs) {

    QString errString;

    if (errs.size() > 0) {
        this->viewVZ->errors->setStyleSheet("QLabel { background: rgba(255,0,0,50); }");
    } else {
        this->viewVZ->errors->setStyleSheet("QLabel { background: rgba(255,255,255,0); }");
    }

    for (uint i = 0; i < (uint) errs.size(); ++i) {
        errString += errs[i] + "\n";
    }

    this->viewVZ->errors->setText(errString);

}
