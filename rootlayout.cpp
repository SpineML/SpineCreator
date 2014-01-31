/***************************************************************************
**                                                                        **
**  This file is part of SpineCreator, an easy to use, GUI for            **
**  describing spiking neural network models.                             **
**  Copyright (C) 2013 Alex Cope, Paul Richmond                           **
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

#include "rootlayout.h"
#include "projectobject.h"

/*
 Alex Cope 2012
 The RootLayout class is the base layout for the sidebar which
 holds the properties of the currently selected item, and allows
 them to be set and modified.
 */

rootLayout::rootLayout(rootData * data, QWidget *parent) :
    QVBoxLayout(parent)
{

    //this->title = new QLabel("");

    this->setSpacing(10);

    initModelHeader(data);
    initPopulationHeader(data);
    initProjectionHeader(data);
    initInputHeader(data);
    initTabBox(data);
    initTabBoxPopulation(data);
    initTabBoxProjection(data);
    initFinish(data);

    // init state
    emit hideHeader();

    // configure model panel
    QSettings settings;
    emit setModelName(settings.value("model/model_name", "err").toString());

    // show model panel
    emit showModel();

}

rootLayout::~rootLayout() {

    // remove all widgets / layouts
    clearOld();

}

// add the stuff that we want
void rootLayout::initModelHeader(rootData * ) {

    // create model name dialog
    QSettings settings;
    QString model_name = settings.value("model/model_name", "err").toString();
    QFormLayout * titleLayout = new QFormLayout();
    QLineEdit * name = new QLineEdit;
    name->setText(model_name);
    titleLayout->addRow("Model name", name);
    this->addLayout(titleLayout);

    // connect to update model name
    connect(name,SIGNAL(editingFinished()), this, SLOT(modelNameChanged()));

    // connect for hide
    connect(this, SIGNAL(hideHeader()), name, SLOT(hide()));
    connect(this, SIGNAL(hideHeader()), titleLayout->itemAt(0)->widget(), SLOT(hide()));

    // connect for show
    connect(this, SIGNAL(showModel()), name, SLOT(show()));
    connect(this, SIGNAL(showModel()), titleLayout->itemAt(0)->widget(), SLOT(show()));

}

// add the stuff that we want
void rootLayout::initPopulationHeader(rootData * data) {

    ////// TITLE
    // add title display
    QHBoxLayout * titleLayout = new QHBoxLayout();
    QString titleString = "<u><b>temp</b></u>";
    QLabel *title  = new QLabel(titleString);
    titleLayout->addWidget(title);
    QPushButton * titleRename = new QPushButton("Rename");
    titleRename->setFixedWidth(80);
    titleLayout->addWidget(titleRename);

    // add title rename (hidden)
    QHBoxLayout * renameLayout = new QHBoxLayout();
    QLineEdit * renameBox = new QLineEdit();
    renameBox->setText("temp");
    renameBox->hide();
    renameLayout->addWidget(renameBox);
    QPushButton * renameDone = new QPushButton("Done");
    renameDone->setFixedWidth(80);
    renameDone->hide();
    renameLayout->addWidget(renameDone);

    // pointers to set the names to the final name
    renameDone->setProperty("ptrTitle", qVariantFromValue((void *) title));
    renameBox->setProperty("ptrTitle", qVariantFromValue((void *) title));
    renameDone->setProperty("ptrRename", qVariantFromValue((void *) renameBox));
    renameBox->setProperty("ptrRename", qVariantFromValue((void *) renameBox));

    // connect up for hiding / showing
    connect(titleRename, SIGNAL(clicked()), title, SLOT(hide()));
    connect(titleRename, SIGNAL(clicked()), titleRename, SLOT(hide()));
    connect(titleRename, SIGNAL(clicked()), renameBox, SLOT(show()));
    connect(titleRename, SIGNAL(clicked()), renameDone, SLOT(show()));
    connect(titleRename, SIGNAL(clicked()), renameBox, SLOT(setFocus()));
    connect(titleRename, SIGNAL(clicked()), renameBox, SLOT(selectAll()));

    connect(renameDone, SIGNAL(clicked()), title, SLOT(show()));
    connect(renameDone, SIGNAL(clicked()), titleRename, SLOT(show()));
    connect(renameDone, SIGNAL(clicked()), renameBox, SLOT(hide()));
    connect(renameDone, SIGNAL(clicked()), renameDone, SLOT(hide()));
    connect(renameDone, SIGNAL(clicked()), titleRename, SLOT(setFocus()));

    connect(renameBox, SIGNAL(editingFinished()), title, SLOT(show()));
    connect(renameBox, SIGNAL(editingFinished()), titleRename, SLOT(show()));
    connect(renameBox, SIGNAL(editingFinished()), renameBox, SLOT(hide()));
    connect(renameBox, SIGNAL(editingFinished()), renameDone, SLOT(hide()));
    connect(renameBox, SIGNAL(editingFinished()), titleRename, SLOT(setFocus()));

    // connect to stuff that does things!
    connect(renameBox, SIGNAL(editingFinished()), data, SLOT(renamePopulation()));
    connect(renameDone, SIGNAL(clicked()), data, SLOT(renamePopulation()));

    // connect for hide
    connect(this, SIGNAL(hideHeader()), title, SLOT(hide()));
    connect(this, SIGNAL(hideHeader()), titleRename, SLOT(hide()));
    connect(this, SIGNAL(hideHeader()), renameBox, SLOT(hide()));
    connect(this, SIGNAL(hideHeader()), renameDone, SLOT(hide()));

    // connect for show
    connect(this, SIGNAL(showPopulation()), title, SLOT(show()));
    connect(this, SIGNAL(showPopulation()), titleRename, SLOT(show()));
    connect(this, SIGNAL(showSpikeSource()), title, SLOT(show()));
    connect(this, SIGNAL(showSpikeSource()), titleRename, SLOT(show()));

    // connect for update of pop name
    connect(this, SIGNAL(setPopulationTitle(QString)), title, SLOT(setText(QString)));
    connect(this, SIGNAL(setPopulationName(QString)), renameBox, SLOT(setText(QString)));

    // add to layout
    this->addLayout(titleLayout);
    this->addLayout(renameLayout);

    ////// SIZE
    QHBoxLayout * tempBox = new QHBoxLayout();
    this->addLayout(tempBox);
    QLabel * sizeLabel = new QLabel("Population size");
    QLabel * SSsizeLabel = new QLabel("No. spike trains");
    tempBox->addWidget(sizeLabel);
    tempBox->addWidget(SSsizeLabel);
    QSpinBox *sizeSpin = new QSpinBox;
    sizeSpin->setRange(1, 2000000);
    sizeSpin->setSingleStep(1);
    sizeSpin->setValue(0);
    tempBox->addWidget(sizeSpin);

    // connect to update size
    connect(sizeSpin, SIGNAL(editingFinished()), data, SLOT (setSize()));

    // connect for hide
    connect(this, SIGNAL(hideHeader()), sizeSpin, SLOT(hide()));
    connect(this, SIGNAL(hideHeader()), sizeLabel, SLOT(hide()));
    connect(this, SIGNAL(hideHeader()), SSsizeLabel, SLOT(hide()));

    // connect for show
    connect(this, SIGNAL(showPopulation()), sizeSpin, SLOT(show()));
    connect(this, SIGNAL(showPopulation()), sizeLabel, SLOT(show()));
    connect(this, SIGNAL(showSpikeSource()), sizeSpin, SLOT(show()));
    connect(this, SIGNAL(showSpikeSource()), SSsizeLabel, SLOT(show()));

    // connect for update of pop size
    connect(this, SIGNAL(setPopulationSize(int)), sizeSpin, SLOT(setValue(int)));

    ////// COLOUR
    QHBoxLayout * colourBox = new QHBoxLayout();
    this->addLayout(colourBox);
    QLabel * colourLabel = new QLabel("Colour");
    colourBox->addWidget(colourLabel);
    QPushButton * colourButton = new QPushButton("Select");
    colourBox->addWidget(colourButton);
    connect(colourButton, SIGNAL(clicked()), data, SLOT (selectColour()));

    // connect for hide
    connect(this, SIGNAL(hideHeader()), colourButton, SLOT(hide()));
    connect(this, SIGNAL(hideHeader()), colourLabel, SLOT(hide()));

    // connect for show
    connect(this, SIGNAL(showPopulation()), colourButton, SLOT(show()));
    connect(this, SIGNAL(showPopulation()), colourLabel, SLOT(show()));

    ////// SPIKE SOURCE
    QLabel * ssLabel = new QLabel("<b>Configure Spike Sources in the Experiment</b>");
    this->addWidget(ssLabel);

    // connect for hide
    connect(this, SIGNAL(hideHeader()), ssLabel, SLOT(hide()));

    // connect for show
    connect(this, SIGNAL(showSpikeSource()), ssLabel, SLOT(show()));


}

// add the stuff that we want
void rootLayout::initProjectionHeader(rootData * data) {

    // TITLE /////////

    QHBoxLayout * titleLayout = new QHBoxLayout();
    QLabel *title  = new QLabel("temp");
    titleLayout->addWidget(title);
    this->addLayout(titleLayout);

    // connect for hide
    connect(this, SIGNAL(hideHeader()), title, SLOT(hide()));

    // connect for show
    connect(this, SIGNAL(showProjection()), title, SLOT(show()));

    // connect for name update
    connect(this, SIGNAL(setProjectionName(QString)), title, SLOT(setText(QString)));

    // SYNAPSES /////////

    // add layout
    QHBoxLayout * synLayout = new QHBoxLayout();
    synLayout->setSpacing(0);
    this->addLayout(synLayout);

    // creat name - we need to reference this as a ptr - so put it here
    QLabel *syn  = new QLabel("temp");
    syn->setAlignment(Qt::AlignHCenter);

    // connect for hide
    connect(this, SIGNAL(hideHeader()), syn, SLOT(hide()));
    // connect for show
    connect(this, SIGNAL(showProjection()), syn, SLOT(show()));
    // connect for set name
    connect(this, SIGNAL(setProjectionSynapseName(QString)), syn, SLOT(setText(QString)));

    // add navigation through synapses - left
    QPushButton * left = new QPushButton("<");
    left->setMaximumWidth(50);
    left->setProperty("direction", "left");
    synLayout->addWidget(left);

    // connect for hide
    connect(this, SIGNAL(hideHeader()), left, SLOT(hide()));
    // connect for show
    connect(this, SIGNAL(showProjection()), left, SLOT(show()));
    // connect to change synapse
    connect(left, SIGNAL(clicked()), data, SLOT (changeSynapse()));

    synLayout->addStretch();

    // add the name
    synLayout->addWidget(syn);
    synLayout->addStretch();

    // add navigation through synapses - right
    QPushButton * right = new QPushButton(">");
    right->setMaximumWidth(50);
    right->setProperty("direction", "right");
    synLayout->addWidget(right);

    // connect for hide
    connect(this, SIGNAL(hideHeader()), right, SLOT(hide()));
    // connect for show
    connect(this, SIGNAL(showProjection()), right, SLOT(show()));
    // connect to change synapse
    connect(right, SIGNAL(clicked()), data, SLOT (changeSynapse()));

    // remove Synapse
    QPushButton * rem = new QPushButton("-");
    rem->setMaximumWidth(50);
    rem->setProperty("direction", "rem");
    synLayout->addWidget(rem);


    // connect for hide
    connect(this, SIGNAL(hideHeader()), rem, SLOT(hide()));
    // connect for show
    connect(this, SIGNAL(setProjectionSynapseMinusOn()), rem, SLOT(show()));
    // connect to change synapse
    connect(rem, SIGNAL(clicked()), data, SLOT (changeSynapse()));

    // add Synapse
    QPushButton * add = new QPushButton("+");
    add->setMaximumWidth(50);
    add->setProperty("direction", "add");
    synLayout->addWidget(add);

    // connect for hide
    connect(this, SIGNAL(hideHeader()), add, SLOT(hide()));
    // connect for show
    connect(this, SIGNAL(showProjection()), add, SLOT(show()));
    // connect to change synapse
    connect(add, SIGNAL(clicked()), data, SLOT (changeSynapse()));

}

// add the stuff that we want
void rootLayout::initInputHeader(rootData * data) {

    // TITLE ///////////////
    QHBoxLayout * titleLayout = new QHBoxLayout();
    QLabel *title  = new QLabel("temp");
    titleLayout->addWidget(title);
    this->addLayout(titleLayout);

    // connect for hide
    connect(this, SIGNAL(hideHeader()), title, SLOT(hide()));
    // connect for show
    connect(this, SIGNAL(showInput()), title, SLOT(show()));
    // connect for set title
    connect(this, SIGNAL(setInputName(QString)), title, SLOT(setText(QString)));

    QHBoxLayout * inputLay = new QHBoxLayout;
    inputSrcName = new QLabel();
    inputLay->addWidget(inputSrcName);

    // connect for hide
    connect(this, SIGNAL(hideHeader()), inputSrcName, SLOT(hide()));
    // connect for show
    connect(this, SIGNAL(showInput()), inputSrcName, SLOT(show()));

    ////// PORT COMBOBOX
    inputPortSelection = new QComboBox;
    inputPortSelection->setMaximumWidth(200);
    inputPortSelection->setProperty("type", "portMatches");

    // connect for hide
    connect(this, SIGNAL(hideHeader()), inputPortSelection, SLOT(hide()));
    // connect for show
    connect(this, SIGNAL(showInput()), inputPortSelection, SLOT(show()));
    // connect for function
    connect(inputPortSelection, SIGNAL(currentIndexChanged(QString)), data, SLOT(updatePortMap(QString)));

    inputLay->addWidget(inputPortSelection);

    this->addLayout(inputLay);

    // CONNECTION COMBOBOX
    inputConnectionComboBox = this->addDropBox(this, "Connectivity", "input");

    // connect for hide
    connect(this, SIGNAL(hideHeader()), inputConnectionComboBox, SLOT(hide()));
    // connect for show
    connect(this, SIGNAL(showInput()), inputConnectionComboBox, SLOT(show()));
}

// add the stuff that we want
void rootLayout::initTabBox(rootData *) {

    tabs = new QTabWidget;
    tabs->setProperty("header","true");
    this->addWidget(tabs);

    tab1 = new QFrame;
    tab1->setContentsMargins(0,0,0,0);
    tabs->addTab(tab1, "temp");
    QVBoxLayout * layout1 = new QVBoxLayout;
    layout1->addStretch();
    tab1->setLayout(layout1);

    tab2 = new QFrame;
    tab2->setContentsMargins(0,0,0,0);
    tabs->addTab(tab2, "temp2");
    QVBoxLayout * layout2 = new QVBoxLayout;
    layout2->addStretch();
    tab2->setLayout(layout2);

    tab3 = new QFrame;
    tab3->setContentsMargins(0,0,0,0);
    tabs->addTab(tab3, "temp3");
    QVBoxLayout * layout3 = new QVBoxLayout;
    layout3->addStretch();
    tab3->setLayout(layout3);

    // connect for hide
    connect(this, SIGNAL(hideHeader()), tabs, SLOT(hide()));

    // connect for show
    connect(this, SIGNAL(showPopulation()), tabs, SLOT(show()));
    connect(this, SIGNAL(showProjection()), tabs, SLOT(show()));

}

// add the stuff that we want
void rootLayout::initTabBoxPopulation(rootData *) {

    ////// LAYOUT

    layoutComboBox = this->addDropBox((QVBoxLayout *) tab2->layout(), "Layout", "layout");

    // connect for configure
    connect(this, SIGNAL(setLayoutType(int)), layoutComboBox, SLOT(setCurrentIndex(int)));

    // connect for hide
    connect(this, SIGNAL(hideHeader()), layoutComboBox, SLOT(hide()));

    // connect for show
    connect(this, SIGNAL(showPopulation()), layoutComboBox, SLOT(show()));

    ////// TYPE

    neuronComboBox = this->addDropBox((QVBoxLayout *) tab1->layout(),"Neuron type", "neuron");

    // connect for configure
    connect(this, SIGNAL(setNeuronType(int)), neuronComboBox, SLOT(setCurrentIndex(int)));

    // connect for hide
    connect(this, SIGNAL(hideHeader()), neuronComboBox, SLOT(hide()));

    // connect for show
    connect(this, SIGNAL(showPopulation()), neuronComboBox, SLOT(show()));

}

// add the stuff that we want
void rootLayout::initTabBoxProjection(rootData * ) {

    ////// WU TYPE

    weightUpdateComboBox = this->addDropBox((QVBoxLayout *) tab1->layout(),"Weight Update", "weight_update");

    // connect for configure
    connect(this, SIGNAL(setWeightUpdateType(int)), weightUpdateComboBox, SLOT(setCurrentIndex(int)));

    // connect for hide
    connect(this, SIGNAL(hideHeader()), weightUpdateComboBox, SLOT(hide()));

    // connect for show
    connect(this, SIGNAL(showProjection()), weightUpdateComboBox, SLOT(show()));

    ////// PSP

    postSynapseComboBox = this->addDropBox((QVBoxLayout *) tab2->layout(),"Post-synapse", "postsynapse");

    // connect for configure
    connect(this, SIGNAL(setPostSynapseType(int)), postSynapseComboBox, SLOT(setCurrentIndex(int)));

    // connect for hide
    connect(this, SIGNAL(hideHeader()), postSynapseComboBox, SLOT(hide()));

    // connect for show
    connect(this, SIGNAL(showProjection()), postSynapseComboBox, SLOT(show()));

    ////// CONNECTION

    connectionComboBox = this->addDropBox((QVBoxLayout *) tab3->layout(),"Connectivity", "conn");

    // connect for configure
    connect(this, SIGNAL(setConnectionType(int)), connectionComboBox, SLOT(setCurrentIndex(int)));

    // connect for hide
    connect(this, SIGNAL(hideHeader()), connectionComboBox, SLOT(hide()));

    // connect for show
    connect(this, SIGNAL(showProjection()), connectionComboBox, SLOT(show()));

}

void rootLayout::initFinish(rootData * data) {

    // add copy / paste buttons
  for (int i = 0; i < 2; ++i) {

        // select tab
        QVBoxLayout * tabLayout;
        if (i == 0)
            tabLayout = (QVBoxLayout *) tab1->layout();
        else
            tabLayout = (QVBoxLayout *) tab2->layout();

        // layout
        QHBoxLayout * cp = new QHBoxLayout();
        tabLayout->insertLayout(tabLayout->count() - 1, cp);

        QPushButton * copy = new QPushButton("Copy");

        // set what the source is
        if (i == 0)
            copy->setProperty("source", "tab1");
        if (i == 1)
            copy->setProperty("source", "tab2");
        copy->setMaximumWidth(60);

        QPushButton * paste = new QPushButton("Paste");

        // set what the source is
        if (i == 0)
            paste->setProperty("source", "tab1");
        if (i == 1)
            paste->setProperty("source", "tab2");
        paste->setMaximumWidth(60);

        // tooltips
        copy->setToolTip("Copy Parameters and State Variables");
        paste->setToolTip("Paste Parameters and State Variables");

        // if no data hide paste
        if (data->clipboardCData == NULL)
            paste->setEnabled(false);

        // add to layout
        QLabel * propLabel = new QLabel("<b>Properties</b>");
        cp->addWidget(propLabel);

        cp->addStretch();
        cp->addWidget(copy);
        cp->addWidget(paste);

        // link to hide / show
        if (i == 0) {
            connect(this, SIGNAL(showTab0CopyPaste()), copy, SLOT(show()));
            connect(this, SIGNAL(showTab0CopyPaste()), paste, SLOT(show()));
            connect(this, SIGNAL(showTab0CopyPaste()), propLabel, SLOT(show()));
        }
        if (i == 1) {
            connect(this, SIGNAL(showTab1CopyPaste()), copy, SLOT(show()));
            connect(this, SIGNAL(showTab1CopyPaste()), paste, SLOT(show()));
            connect(this, SIGNAL(showTab1CopyPaste()), propLabel, SLOT(show()));
        }

        connect(this, SIGNAL(hideHeader()), copy, SLOT(hide()));
        connect(this, SIGNAL(hideHeader()), paste, SLOT(hide()));
        connect(this, SIGNAL(hideHeader()), propLabel, SLOT(hide()));

        // link to enable paste:
        connect(this, SIGNAL(allowPaste(bool)), paste, SLOT(setEnabled(bool)));

        // link up
        connect(copy, SIGNAL(clicked()), data, SLOT(copyParsToClipboard()));
        connect(paste, SIGNAL(clicked()), data, SLOT(pasteParsFromClipboard()));

    }

    // initial update of the lists
    updateLayoutList(data);
    updateComponentLists(data);

    // and add the stretch
    this->addStretch();
}

void rootLayout::updateLayoutList(rootData * data) {

    // upadte the layout list
    layoutComboBox->clear();
    // we don't want to set stuff while this occurs
    layoutComboBox->disconnect(data);
    for (unsigned int i = 0; i < data->catalogLayout.size(); ++i) {
        layoutComboBox->addItem(data->catalogLayout[i]->name);
    }

    connect(layoutComboBox, SIGNAL(activated(int)), data, SLOT(updateType(int)));

}

void rootLayout::updateComponentLists(rootData * data) {

    // update the component lists

    // we don't want to set stuff while this occurs
    neuronComboBox->disconnect(data);
    weightUpdateComboBox->disconnect(data);
    postSynapseComboBox->disconnect(data);

    // neuron
    neuronComboBox->clear();
    for (unsigned int i = 0; i < data->catalogNrn.size(); ++i) {
        if (!(data->catalogNrn[i]->name == "none"))
            neuronComboBox->addItem(data->catalogNrn[i]->path + "/" + data->catalogNrn[i]->name);
        else
            neuronComboBox->addItem("-select component-");
    }
    QModelIndex ind = neuronComboBox->model()->index(0,0);
    neuronComboBox->model()->setData(ind, QVariant(0), Qt::UserRole-1);

    // weightupdate
    weightUpdateComboBox->clear();
    for (unsigned int i = 0; i < data->catalogWU.size(); ++i) {
        if (!(data->catalogWU[i]->name == "none"))
            weightUpdateComboBox->addItem(data->catalogWU[i]->path + "/" + data->catalogWU[i]->name);
        else
            weightUpdateComboBox->addItem("-select component-");
    }
    ind = weightUpdateComboBox->model()->index(0,0);
    weightUpdateComboBox->model()->setData(ind, QVariant(0), Qt::UserRole-1);

    // postsynapse
    postSynapseComboBox->clear();
    for (unsigned int i = 0; i < data->catalogPS.size(); ++i) {
        if (!(data->catalogPS[i]->name == "none"))
            postSynapseComboBox->addItem(data->catalogPS[i]->path + "/" + data->catalogPS[i]->name);
        else
            postSynapseComboBox->addItem("-select component-");
    }
    ind = postSynapseComboBox->model()->index(0,0);
    postSynapseComboBox->model()->setData(ind, QVariant(0), Qt::UserRole-1);

    connect(neuronComboBox, SIGNAL(activated(int)), data, SLOT(updateType(int)));
    connect(weightUpdateComboBox, SIGNAL(activated(int)), data, SLOT(updateType(int)));
    connect(postSynapseComboBox, SIGNAL(activated(int)), data, SLOT(updateType(int)));

}

void rootLayout::modelNameChanged() {

    QString model_name = ((QLineEdit *) sender())->text();
    emit setCaption(model_name);

}

void rootLayout::recursiveDeleteLater(QLayout * parentLayout) {

    QLayoutItem * item;
    while (parentLayout->count() > 0) {
        item = parentLayout->takeAt(0);
        if (item->widget()) {
            item->widget()->hide();
            forDeleting.push_back(item->widget());
            item->widget()->disconnect((QObject *)0);
        }
        if (item->layout())
            recursiveDeleteLater(item->layout());
        delete item;
    }
    parentLayout->deleteLater();

}

void rootLayout::clearOld() {

    // loop while items remain
    while (this->count() > 0)
    {
        // remove and return first item
        QLayoutItem *item = this->takeAt(0);
        // if item is a widget
        if (item->widget() != 0) {
            item->widget()->hide();
            forDeleting.push_back(item->widget());
            item->widget()->disconnect((QObject *)0);
        }
        // if item is a spacer deleting causes a segfault
        else if (item->spacerItem() != 0) {
            //delete item->spacerItem();
        }
        // if item is a layout recursively delete contents
        else if (item->layout() != 0) {
            recursiveDeleteLater(item->layout());
        }
        delete item;
    }
}

void rootLayout::updatePanel(rootData* data) {

    // update libraries
    updateLayoutList(data);
    updateComponentLists(data);

    // if we are not on view 1 do not update...
    if (!this->parentWidget()->isVisible())
        return;

    // hide everything
    emit hideHeader();

    // find if there is only one selection, if not then clear all
    if (data->selList.size() != 1) {

        // configure model panel
        QSettings settings;
        emit setModelName(data->currProject->name);

        // remove input properties
        emit deleteProperties();

        // show model panel
        emit showModel();

        return;

    } else {

        lastObject = data->selList[0];

        // find what the selected item is...
        if (data->selList[0]->type == populationObject) {

            popSelected((population *) data->selList[0], data);
            return;

        }
        if (data->selList[0]->type == projectionObject) {

            projSelected((projection *) data->selList[0], data);
            return;

        }
        if (data->selList[0]->type == inputObject) {

            inSelected((genericInput *) data->selList[0], data);
            return;

        }

    }

}

void rootLayout::popSelected(population * pop, rootData* data) {

    if (pop->neuronTypeName.compare("noPop") == 0)
        return;

    // spikesource has none of this info
    if (pop->isSpikeSource) {
        emit showSpikeSource();
        emit setPopulationName(pop->name);
        emit setPopulationTitle("<u><b>" + pop->name + "</b></u>");
        emit setPopulationSize(pop->numNeurons);
        return;
    }

    // configure
    emit showPopulation();
    emit setPopulationName(pop->name);
    emit setPopulationTitle("<u><b>" + pop->name + "</b></u>");
    emit setPopulationSize(pop->numNeurons);
    for (unsigned int i = 0; i < data->catalogNrn.size(); ++i) {
        if (pop->neuronType->component == data->catalogNrn[i]) {
            emit setNeuronType(i);
        }
    }

    // if no data hide paste
    emit allowPaste(data->clipboardCData != NULL);

    // set tab
    if (tabs->currentIndex() > 1) {
        tabs->setCurrentIndex(0);
    }

    tabs->removeTab(2);

    tabs->setTabText(0, "Neuron Body");
    tabs->setTabText(1, "Layout");

    emit deleteProperties();
    drawParamsLayout(data);

}

void rootLayout::projSelected(projection * proj, rootData* data) {

    // configure
    emit showProjection();
    emit setProjectionName("<u><b>" + proj->getName() + "</b></u>");

    // make sure we are not off the end due to deletes
    while (proj->currTarg > (int) proj->synapses.size()-1)
        proj->currTarg--;

    // Synapse
    emit setProjectionSynapseName("Synapse " + QString::number(proj->currTarg));
    if (proj->synapses.size() > 1) {
        emit setProjectionSynapseMinusOn();
    }

    // if no data hide paste
    emit allowPaste(data->clipboardCData != NULL);

    if (tabs->count() < 3)
        tabs->addTab(tab3, "Connectivity");

    tabs->setTabText(0, "Weight Update");
    tabs->setTabText(1, "PostSynapse");
    tabs->setTabText(2, "Connectivity");

    // set comboboxes
    for (unsigned int i = 0; i < data->catalogWU.size(); ++i) {
        if (proj->synapses[proj->currTarg]->weightUpdateType->component == data->catalogWU[i]) {
            emit setWeightUpdateType(i);
        }
    }
    for (unsigned int i = 0; i < data->catalogPS.size(); ++i) {
        if (proj->synapses[proj->currTarg]->postsynapseType->component == data->catalogPS[i]) {
            emit setPostSynapseType(i);
        }
    }

    if(!connectionComboBox-disconnect(data))
        qDebug() << "hmmm....";
    connectionComboBox->clear();
    connectionComboBox->setProperty("ptr", qVariantFromValue((void *) proj->synapses[proj->currTarg]));
    connectionComboBox->addItem("All to All");
    connectionComboBox->addItem("One to One");
    // disable 1-2-1 if src and dst pops not the same size
    if (proj->source->numNeurons != proj->destination->numNeurons) {
        QModelIndex ind = connectionComboBox->model()->index(1,0);
        connectionComboBox->model()->setData(ind, QVariant(0), Qt::UserRole-1);
    }
    connectionComboBox->addItem("Fixed Probability");
    connectionComboBox->addItem("Explicit List");
    connectionComboBox->addItem("Distance Based Probability");
    connectionComboBox->addItem("Kernel");
    connectionComboBox->setCurrentIndex(proj->synapses[proj->currTarg]->connectionType->type);
    connect(connectionComboBox, SIGNAL(activated(int)), data, SLOT(updateType(int)));

    emit deleteProperties();
    drawParamsLayout(data);

}

void rootLayout::inSelected(genericInput * in, rootData* data) {

    emit showInput();
    emit setInputName("<u><b>" + in->getName() + "</b></u>");
    emit deleteProperties();

    QString XMLname = in->src->getXMLName();
    inputSrcName->setToolTip(XMLname);
    // shorten if too long (tooltip will have full name)
    if (XMLname.size() > 12) {
        XMLname.resize(10);
        XMLname.append("...");
    } else {
        // make label consistent size
        XMLname.insert(13, " ");

    }
    inputSrcName->setText(XMLname);

    // add port options
    QStringList portPairs;

    for (uint i = 0; i < in->dst->inputs.size(); ++i)
        if (in == in->dst->inputs[i])
            portPairs = in->dst->getPortMatches(i, false);

    QString currPortPair = in->srcPort + "->" + in->dstPort;

    inputPortSelection->setProperty("ptr", qVariantFromValue((void *) in));
    inputPortSelection->disconnect(data);
    inputPortSelection->clear();
    for (int p = 0; p < portPairs.size(); ++p) {
        inputPortSelection->addItem(portPairs.at(p));
        if (currPortPair == portPairs.at(p)) {
            inputPortSelection->setCurrentIndex(inputPortSelection->count()-1);
        }
    }
    if (portPairs.size() == 0) {
        inputPortSelection->setDisabled(true);
        // make sure we don't have gibberish loaded in
        in->srcPort.clear();
        in->dstPort.clear();
    }
    connect(inputPortSelection, SIGNAL(currentIndexChanged(QString)), data, SLOT(updatePortMap(QString)));

    // configure connectivity dropdown
    inputConnectionComboBox->disconnect(data);
    inputConnectionComboBox->clear();
    inputConnectionComboBox->setProperty("ptr", qVariantFromValue((void *) in));
    inputConnectionComboBox->addItem("All to All");
    inputConnectionComboBox->addItem("One to One");
    inputConnectionComboBox->addItem("Fixed Probability");
    inputConnectionComboBox->addItem("Explicit List");
    if (in->src->owner->type == populationObject && in->dst->owner->type == populationObject) {
        inputConnectionComboBox->addItem("Distance Based Probability");
        inputConnectionComboBox->addItem("Kernel");
    }
    inputConnectionComboBox->setCurrentIndex(in->connectionType->type);
    connect(inputConnectionComboBox, SIGNAL(activated(int)), data, SLOT(updateType(int)));

    QFormLayout * varLayout = new QFormLayout;

    // add to delete props
    connect(this, SIGNAL(deleteProperties()), varLayout, SLOT(deleteLater()));

    // other connectivity:
    switch (in->connectionType->type) {
        case AlltoAll:
            break;
        case OnetoOne:
            break;
        case FixedProb:
            {// draw in p and seed boxes:
            QDoubleSpinBox *pSpin = new QDoubleSpinBox;
            pSpin->setRange(0, 1);
            pSpin->setSingleStep(0.1);
            pSpin->setMaximumWidth(60);
            pSpin->setDecimals(3);
            pSpin->setValue(((fixedProb_connection *) in->connectionType)->p);
            pSpin->setProperty("valToChange", "1");
            pSpin->setProperty("conn", "true");
            pSpin->setToolTip("connection probability");
            pSpin->setProperty("ptr", qVariantFromValue((void *) in->connectionType));
            pSpin->setProperty("action","changeConnProb");
            connect(pSpin, SIGNAL(valueChanged(double)), data, SLOT (updatePar()));
            varLayout->addRow("Probability", pSpin);
            // add to delete props
            connect(this, SIGNAL(deleteProperties()), varLayout->itemAt(varLayout->rowCount()-1,QFormLayout::LabelRole)->widget(), SLOT(deleteLater()));
            connect(this, SIGNAL(deleteProperties()), varLayout->itemAt(varLayout->rowCount()-1,QFormLayout::FieldRole)->widget(), SLOT(deleteLater()));
            }
            break;
        case CSV:
            {QPushButton *edit = new QPushButton("Edit");
            varLayout->addRow("Connection list", edit);
            edit->setMaximumWidth(70);
            edit->setMaximumHeight(28);
            edit->setToolTip("Edit the explicit value list");
            edit->setProperty("ptr", qVariantFromValue((void *) in->connectionType));
            // add to delete props
            connect(this, SIGNAL(deleteProperties()), varLayout->itemAt(varLayout->rowCount()-1,QFormLayout::LabelRole)->widget(), SLOT(deleteLater()));
            connect(this, SIGNAL(deleteProperties()), varLayout->itemAt(varLayout->rowCount()-1,QFormLayout::FieldRole)->widget(), SLOT(deleteLater()));
            // add connection:
            connect(edit, SIGNAL(clicked()), data, SLOT(editConnections()));
            }
            break;
        case CSA:
            break;
        case DistanceBased:
            varLayout->addRow("", new QLabel("Configure in visualiser"));
            // add to delete props
            //connect(this, SIGNAL(deleteProperties()), varLayout->itemAt(varLayout->rowCount()-1,QFormLayout::LabelRole)->widget(), SLOT(deleteLater()));
            connect(this, SIGNAL(deleteProperties()), varLayout->itemAt(varLayout->rowCount()-1,QFormLayout::FieldRole)->widget(), SLOT(deleteLater()));
            break;
        case Kernel:
            varLayout->addRow("", new QLabel("Configure in visualiser"));
            // add to delete props
            //connect(this, SIGNAL(deleteProperties()), varLayout->itemAt(varLayout->rowCount()-1,QFormLayout::LabelRole)->widget(), SLOT(deleteLater()));
            connect(this, SIGNAL(deleteProperties()), varLayout->itemAt(varLayout->rowCount()-1,QFormLayout::FieldRole)->widget(), SLOT(deleteLater()));
            break;
        case none:
            break;
    }

    // delay
    switch (in->connectionType->type) {
        case AlltoAll:
        case OnetoOne:
        case FixedProb:
        case CSA:
        case Kernel:
        case CSV:
            // add delay box:
            drawSingleParam(varLayout, in->connectionType->delay, data, true, "conn", NULL, in->connectionType);
            break;
        case DistanceBased:
            // add delay box:
            if (((distanceBased_connection *)in->connectionType)->delayEquation == "")
                drawSingleParam(varLayout, in->connectionType->delay, data, true, "conn", NULL, in->connectionType);
            break;
        case none:
            break;
    }

    this->insertLayout(this->count()-2,varLayout);


}


QComboBox* rootLayout::addDropBox(QVBoxLayout  * lay, QString name, QString type) {

    QHBoxLayout * box = new QHBoxLayout();
    lay->insertLayout(lay->count() -1, box);
    QLabel * nameLabel = new QLabel("<b>" + name + ":</b>");
    if (type == "layout" || type == "neuron")
        connect(this, SIGNAL(showPopulation()), nameLabel, SLOT(show()));
    else if (type == "input")
        connect(this, SIGNAL(showInput()), nameLabel, SLOT(show()));
    else
        connect(this, SIGNAL(showProjection()), nameLabel, SLOT(show()));
    connect(this, SIGNAL(hideHeader()), nameLabel, SLOT(hide()));
    box->addWidget(nameLabel);
    QComboBox *select = new QComboBox();
    select->setMaximumWidth(150);
    select->setProperty("type", type);
    box->addWidget(select);
    return select;

}

void rootLayout::addDivider() {

    QFrame * div1 = new QFrame();
    div1->setFrameShape(QFrame::HLine);
    div1->setProperty("header","true");
    this->insertWidget(this->count() - 1, div1);

}

void rootLayout::drawParamsLayout(rootData * data) {
    // Redraw the parameters panel...

    // number of 9ML objects that make up the element
    int numBoxes;

    bool connectionBool = false;

    // get the number of boxes for the different object types
    switch (data->selList[0]->type) {
        case populationObject:
            numBoxes = 2;
            break;
        case projectionObject:
            numBoxes = 3;
            break;
        default:
            cerr << "Something has gone badly wrong!";
            exit(-1);
            break;
    }

    NineMLData * type9ml = NULL;
    QString type = "layoutParam";

    for (int i = 0; i < numBoxes; ++i) {

        bool skipTab = false;

        // select tab
        QVBoxLayout * tabLayout;
        if (i == 0)
            tabLayout = (QVBoxLayout *) tab1->layout();
        else if (i == 1)
            tabLayout = (QVBoxLayout *) tab2->layout();
        else if (i == 2)
            tabLayout = (QVBoxLayout *) tab3->layout();

        switch (data->selList[0]->type) {
        case populationObject:
            if (i == 0) {
                type9ml = (NineMLData *) ((population *) data->selList[0])->neuronType;
                type = "nrn";
                // check if current component validates
                ((NineMLComponentData *) type9ml)->component->validateComponent();
                QSettings settings;
                int num_errs = settings.beginReadArray("errors");
                settings.endArray();
                num_errs += settings.beginReadArray("warnings");
                settings.endArray();

                settings.remove("errors");
                settings.remove("warnings");

                // doesn't validate - warn and skip
                if (num_errs != 0) {
                    QLabel * validateWarning = new QLabel("Component does not validate: please correct");
                    tabLayout->insertWidget(tabLayout->count() - (1), validateWarning);
                    // connect to delete
                    connect(this, SIGNAL(deleteProperties()), validateWarning, SLOT(deleteLater()));
                    skipTab = true;
                }
            }
            if (i == 1) {

                type9ml = (NineMLData *) ((population *) data->selList[0])->layoutType;
                type = "layout";
            }
            break;
        case projectionObject:
            if (i == 0) {
                type9ml = ((projection *) data->selList[0])->synapses[((projection *) data->selList[0])->currTarg]->weightUpdateType;
                type = "syn";
                // check if current component validates
                ((NineMLComponentData *) type9ml)->component->validateComponent();
                QSettings settings;
                int num_errs = settings.beginReadArray("errors");
                settings.endArray();
                num_errs += settings.beginReadArray("warnings");
                settings.endArray();

                settings.remove("errors");
                settings.remove("warnings");

                // doesn't validate - warn and skip
                if (num_errs != 0) {
                    QLabel * validateWarning = new QLabel("Component does not validate: please correct");
                    tabLayout->insertWidget(tabLayout->count() - (1), validateWarning);
                    // connect to delete
                    connect(this, SIGNAL(deleteProperties()), validateWarning, SLOT(deleteLater()));
                    skipTab = true;
                }
            }
            if (i == 1) {
                type9ml = ((projection *) data->selList[0])->synapses[((projection *) data->selList[0])->currTarg]->postsynapseType;
                type = "psp";
                // check if current component validates
                ((NineMLComponentData *) type9ml)->component->validateComponent();
                QSettings settings;
                int num_errs = settings.beginReadArray("errors");
                settings.endArray();
                num_errs += settings.beginReadArray("warnings");
                settings.endArray();

                settings.remove("errors");
                settings.remove("warnings");

                // doesn't validate - warn and skip
                if (num_errs != 0) {
                    QLabel * validateWarning = new QLabel("Component does not validate: please correct");
                    tabLayout->insertWidget(tabLayout->count() - (1), validateWarning);
                    // connect to delete
                    connect(this, SIGNAL(deleteProperties()), validateWarning, SLOT(deleteLater()));
                    skipTab = true;
                }
            }
            if (i == 2) {
                connectionBool = true;
                type = "conn";
            }
            break;
        case synapseObject:
            break;
        case inputObject:
            break;
        case nullObject:
            break;
        }

        connection * conn;
        if (i == 2) // connection
            conn = ((projection *) data->selList[0])->synapses[((projection *) data->selList[0])->currTarg]->connectionType;
        else if (type9ml->type == NineMLComponentType) {

            // hide / show copy / paste
            if (((NineMLComponentData *) type9ml)->component->name != "none") {
                // set what the source is
                if (i == 0)
                    emit showTab0CopyPaste();
                if (i == 1)
                    emit showTab1CopyPaste();
            }

        }

        // don't draw stuff if the component doesn't validate
        if (skipTab) continue;

        for (int j = 0; j < 2; ++j) {

            // configure:
            QString parType;
            QString boxTitle;
            int listSize = -1;            

            if (!connectionBool) {
                if (j == 0) {
                    parType = "Par";
                    boxTitle = "<i>Parameters</i>";
                    listSize = type9ml->ParameterList.size();
                }
                if (j == 1) {
                    parType = "State";
                    boxTitle = "<i>State variable initial values</i>";
                    listSize = type9ml->StateVariableList.size();
                }
            } else {
                if (j == 0) {
                    parType = "Par";
                    boxTitle = "Connection Parameters";
                    listSize = 1;
                }
                if (j == 1) continue;
            }

            // now draw based upon the selected type

            if (listSize > 0) {
                QHBoxLayout * Box = new QHBoxLayout();
                tabLayout->insertLayout(tabLayout->count() - (1), Box);
                QLabel * sectionTitle = new QLabel(boxTitle);
                Box->addWidget(sectionTitle);

                // connect to delete
                connect(this, SIGNAL(deleteProperties()), sectionTitle, SLOT(deleteLater()));
                connect(this, SIGNAL(deleteProperties()), Box, SLOT(deleteLater()));

                QFormLayout * varLayout = new QFormLayout();
                tabLayout->insertLayout(tabLayout->count() - (1), varLayout);
                connect(this, SIGNAL(deleteProperties()), varLayout, SLOT(deleteLater()));

                for (int l = 0; l < listSize; ++l) {

                    // pointer to current parameter
                    ParameterData * currPar;

                    if (!connectionBool) {
                        if (j == 0) {
                            currPar = type9ml->ParameterList[l];
                            if (currPar->name == "numNeurons" && type9ml->type == NineMLLayoutType) continue;
                            }
                        if (j == 1) {
                             currPar = type9ml->StateVariableList[l];
                             if (currPar->name == "x" && type9ml->type == NineMLLayoutType) continue;
                             if (currPar->name == "y" && type9ml->type == NineMLLayoutType) continue;
                             if (currPar->name == "z" && type9ml->type == NineMLLayoutType) continue;
                        }
                    } else {
                        currPar = conn->delay;
                    }


                    // pars
                    if (connectionBool) {
                        if (conn->type == DistanceBased) {
                            if (((distanceBased_connection *) conn)->delayEquation == "")
                                drawSingleParam(varLayout, currPar, data, connectionBool, type, type9ml, conn);
                        } else {
                            drawSingleParam(varLayout, currPar, data, connectionBool, type, type9ml, conn);
                        }
                    } else {
                        drawSingleParam(varLayout, currPar, data, connectionBool, type, type9ml, conn);
                    }

                }
                ///
                if (connectionBool) {
                    switch (conn->type) {
                    case AlltoAll:
                        break;
                    case OnetoOne:
                        break;
                    case FixedProb:
                        {// draw in p and seed boxes:
                        QDoubleSpinBox *pSpin = new QDoubleSpinBox;
                        pSpin->setRange(0, 1);
                        pSpin->setSingleStep(0.1);
                        pSpin->setMaximumWidth(60);
                        pSpin->setDecimals(3);
                        pSpin->setValue(((fixedProb_connection *) conn)->p);
                        pSpin->setProperty("valToChange", "1");
                        pSpin->setProperty("conn", "true");
                        pSpin->setToolTip("connection probability");
                        pSpin->setProperty("ptr", qVariantFromValue((void *) conn));
                        pSpin->setProperty("action","changeConnProb");
                        connect(pSpin, SIGNAL(valueChanged(double)), data, SLOT (updatePar()));
                        connect(this, SIGNAL(deleteProperties()), pSpin, SLOT(deleteLater()));
                        varLayout->addRow("Probability", pSpin);
                        connect(this, SIGNAL(deleteProperties()), varLayout->itemAt(varLayout->rowCount()-1,QFormLayout::LabelRole)->widget(), SLOT(deleteLater()));


                    }
                        break;
                    case CSV:
                        {
                        // handled in the connection dialog
                        QPushButton *edit = new QPushButton("Edit");
                        varLayout->addRow("Connection list", edit);
                        connect(this, SIGNAL(deleteProperties()), varLayout->itemAt(varLayout->rowCount()-1,QFormLayout::LabelRole)->widget(), SLOT(deleteLater()));
                        edit->setMaximumWidth(70);
                        edit->setMaximumHeight(28);
                        edit->setToolTip("Edit the explicit value list");
                        edit->setProperty("ptr", qVariantFromValue((void *) conn));
                        connect(this, SIGNAL(deleteProperties()), edit, SLOT(deleteLater()));

                        // add connection:
                        connect(edit, SIGNAL(clicked()), data, SLOT(editConnections()));
                        }
                        break;
                    case CSA:
                        break;
                    case DistanceBased:
                        varLayout->addRow("", new QLabel("Configure in visualiser"));
                        //connect(this, SIGNAL(deleteProperties()), varLayout->itemAt(varLayout->rowCount()-1,QFormLayout::LabelRole)->widget(), SLOT(deleteLater()));
                        connect(this, SIGNAL(deleteProperties()), varLayout->itemAt(varLayout->rowCount()-1,QFormLayout::FieldRole)->widget(), SLOT(deleteLater()));
                        break;
                    case Kernel:
                        varLayout->addRow("", new QLabel("Configure in visualiser"));
                        //connect(this, SIGNAL(deleteProperties()), varLayout->itemAt(varLayout->rowCount()-1,QFormLayout::LabelRole)->widget(), SLOT(deleteLater()));
                        connect(this, SIGNAL(deleteProperties()), varLayout->itemAt(varLayout->rowCount()-1,QFormLayout::FieldRole)->widget(), SLOT(deleteLater()));
                        break;
                    case none:
                        break;
                    }
                }

            }

        }
        // inputs:

        // connections have no inputs
        if (connectionBool) continue;

        if (type9ml->type == NineMLComponentType) {

            NineMLComponentData * componentData = (NineMLComponentData *) type9ml;

            if (componentData->component->name != "none") {

                // inputs
                QHBoxLayout * inputLayHeader = new QHBoxLayout;
                connect(this, SIGNAL(deleteProperties()), inputLayHeader, SLOT(deleteLater()));
                inputLayHeader->addWidget(new QLabel("<b>Inputs</b>"));
                connect(this, SIGNAL(deleteProperties()), inputLayHeader->itemAt(inputLayHeader->count()-1)->widget(), SLOT(deleteLater()));
                tabLayout->insertLayout(tabLayout->count()-1, inputLayHeader);

                for (uint input = 0; input < componentData->inputs.size(); ++input) {

                    QHBoxLayout * inputLay = new QHBoxLayout;
                    connect(this, SIGNAL(deleteProperties()), inputLay, SLOT(deleteLater()));
                    QString XMLname = componentData->inputs[input]->src->getXMLName();
                    QLabel * srcName = new QLabel();
                    connect(this, SIGNAL(deleteProperties()), srcName, SLOT(deleteLater()));
                    srcName->setToolTip(XMLname);
                    // shorten if too long (tooltip will have full name)
                    if (XMLname.size() > 16) {
                        XMLname.resize(13);
                        XMLname.append("...");
                    } else {
                        // make label consistent size
                        XMLname.insert(16, " ");

                    }
                    srcName->setText(XMLname);
                    inputLay->addWidget(srcName);

                    ////// PORT COMBOBOX

                    QComboBox * portMatches = new QComboBox;
                    connect(this, SIGNAL(deleteProperties()), portMatches, SLOT(deleteLater()));
                    portMatches->setMaximumWidth(200);
                    portMatches->setProperty("type", "portMatches");
                    portMatches->setProperty("ptr", qVariantFromValue((void *) componentData->inputs[input]));

                    genericInput * currInput = componentData->inputs[input];

                    QStringList portPairs = componentData->getPortMatches(input, false);

                    QString currPortPair = currInput->srcPort + "->" + currInput->dstPort;

                    for (int p = 0; p < portPairs.size(); ++p) {
                        portMatches->addItem(portPairs.at(p));
                        if (currPortPair == portPairs.at(p)) {
                            portMatches->setCurrentIndex(portMatches->count()-1);
                        }
                    }
                    if (portPairs.size() == 0) {
                        portMatches->setDisabled(true);
                        // also add stuff to indicate the issue
                        if (currInput->dst->component->name != "none") {
                            portMatches->addItem("No matches found");
                            portMatches->setToolTip("No compatible port matches were found. This may be due to inconsistent dimensions (including exponents).");
                        }
                        // make sure we don't have gibberish loaded in
                        currInput->srcPort.clear();
                        currInput->dstPort.clear();
                    }

                    connect(portMatches, SIGNAL(activated(QString)), data, SLOT(updatePortMap(QString)));

                    inputLay->addWidget(portMatches);

                    tabLayout->insertLayout(tabLayout->count()-1, inputLay);

                    /////// DELETE BUTTON
                    QPushButton * delInput = new QPushButton;
                    connect(this, SIGNAL(deleteProperties()), delInput, SLOT(deleteLater()));
                    delInput->setMaximumWidth(28);
                    delInput->setMaximumHeight(28);
                    delInput->setIcon(QIcon(":/icons/toolbar/delShad.png"));
                    delInput->setFlat(true);
                    delInput->setToolTip("Delete");
                    delInput->setFocusPolicy(Qt::NoFocus);
                    if (componentData->inputs[input]->projInput == true) {
                        delInput->setEnabled(false);
                        delInput->setIcon(QIcon(":/icons/toolbar/icons/colico.png"));
                        delInput->setToolTip("Can't delete projection essential inputs");
                    }
                    inputLay->addWidget(delInput);

                    delInput->setProperty("ptr", qVariantFromValue((void *) componentData->inputs[input]));
                    delInput->setProperty("ptrDst", qVariantFromValue((void *) componentData));

                    // add connection:
                    connect(delInput, SIGNAL(clicked()), data, SLOT(delgenericInput()));
                }

                // add an input
                QHBoxLayout * addInput = new QHBoxLayout;
                connect(this, SIGNAL(deleteProperties()), addInput, SLOT(deleteLater()));
                addInput->addWidget(new QLabel("Add from "));
                connect(this, SIGNAL(deleteProperties()), addInput->itemAt(addInput->count()-1)->widget(), SLOT(deleteLater()));

                tabLayout->insertLayout(tabLayout->count()-1, addInput);

                QStringList elementList;
                for (uint i = 0; i < data->system.size(); ++i) {
                    elementList << data->system[i]->neuronType->getXMLName();
                    for (uint j = 0; j < data->system[i]->projections.size(); ++j) {
                        for (uint k = 0; k < data->system[i]->projections[j]->synapses.size(); ++k) {
                            elementList << data->system[i]->projections[j]->synapses[k]->weightUpdateType->getXMLName();
                            elementList << data->system[i]->projections[j]->synapses[k]->postsynapseType->getXMLName();
                        }
                    }
                }
                QLineEdit *lineEdit = new QLineEdit;
                connect(this, SIGNAL(deleteProperties()), lineEdit, SLOT(deleteLater()));

                QCompleter *completer = new QCompleter(elementList, this);
                completer->setCaseSensitivity(Qt::CaseInsensitive);
                lineEdit->setCompleter(completer);
                lineEdit->setProperty("ptr", qVariantFromValue((void *) componentData));

                // add connection:
                connect(lineEdit, SIGNAL(editingFinished()), data, SLOT(addgenericInput()));

                addInput->addWidget(lineEdit);

            }

        }
    }

}

void rootLayout::drawSingleParam(QFormLayout * varLayout, ParameterData * currPar, rootData * data, bool connectionBool, QString type, NineMLData* type9ml, connection * conn) {


    QString name;
    ParameterType parType = Undefined;
    double value = -1;

    name = currPar->name;

    if (name.size() > 13) {
        name.resize(10);
        name = name + "...";
    }
    if (name.size() < 13) {
        name.insert(13, " ");
    }

    switch (currPar->currType) {
    case Undefined:
        {QHBoxLayout * buttons = new QHBoxLayout;
        connect(this, SIGNAL(deleteProperties()), buttons, SLOT(deleteLater()));
#ifdef Q_OS_MAC
        buttons->setSpacing(5);
#else
        buttons->setSpacing(0);
#endif
        buttons->addStretch();

        // ADD BUTTON FOR FIXED VALUE
        QPushButton * currButton = new QPushButton("Fixed");
        currButton->setMaximumWidth(80);
        currButton->setMaximumHeight(28);
        currButton->setToolTip("A single value for all instances");
        currButton->setProperty("action","updateType");
        currButton->setProperty("newType","FixedValue");
        buttons->addWidget(currButton);
        connect(this, SIGNAL(deleteProperties()), currButton, SLOT(deleteLater()));

        currButton->setProperty("ptr", qVariantFromValue((void *) currPar));

        // add connection:
        connect(currButton, SIGNAL(clicked()), data, SLOT(updatePar()));

        // ADD BUTTON FOR STATISTICAL VALUE
        currButton = new QPushButton("Random");
        currButton->setMaximumWidth(80);
        currButton->setMaximumHeight(28);
        //delInput->setIcon(QIcon(":/icons/toolbar/delShad.png"));
        //delInput->setFlat(true);
        currButton->setToolTip("Values drawn from a statistical distribution");
        //delInput->setFocusPolicy(Qt::NoFocus);
        currButton->setProperty("action","updateType");
        currButton->setProperty("newType","Statistical");
        buttons->addWidget(currButton);
        connect(this, SIGNAL(deleteProperties()), currButton, SLOT(deleteLater()));

        currButton->setProperty("ptr", qVariantFromValue((void *) currPar));

        // add connection:
        connect(currButton, SIGNAL(clicked()), data, SLOT(updatePar()));

        // ADD BUTTON FOR EXPLICIT VALUES
        // skip for connection types that don't support explicit lists
        bool skipExplicit = false;
        if (connectionBool)
            if (conn->type != CSV)
                skipExplicit = true;

        if (!skipExplicit) {
            currButton = new QPushButton("List");
            currButton->setMaximumWidth(80);
            currButton->setMaximumHeight(28);
            //delInput->setIcon(QIcon(":/icons/toolbar/delShad.png"));
            //delInput->setFlat(true);
            currButton->setToolTip("A complete list of values for each instance");
            //delInput->setFocusPolicy(Qt::NoFocus);
            currButton->setProperty("action","updateType");
            currButton->setProperty("newType","Explicit");
            buttons->addWidget(currButton);
            connect(this, SIGNAL(deleteProperties()), currButton, SLOT(deleteLater()));

            currButton->setProperty("ptr", qVariantFromValue((void *) currPar));

            // add connection:
            connect(currButton, SIGNAL(clicked()), data, SLOT(updatePar()));
        }

        varLayout->addRow(name, buttons);
        connect(this, SIGNAL(deleteProperties()), varLayout->itemAt(varLayout->rowCount()-1,QFormLayout::LabelRole)->widget(), SLOT(deleteLater()));
        varLayout->itemAt(varLayout->rowCount()-1,QFormLayout::LabelRole)->widget()->setProperty("type",type + parType);
        // add the full parameter name to the tooltip
        varLayout->itemAt(varLayout->rowCount()-1,QFormLayout::LabelRole)->widget()->setToolTip(currPar->name);

        }
        break;

    case FixedValue:
        {
        QHBoxLayout * buttons = new QHBoxLayout;
        connect(this, SIGNAL(deleteProperties()), buttons, SLOT(deleteLater()));
        buttons->setSpacing(0);

        value = currPar->value[0];

        QDoubleSpinBox *parSpin = new QDoubleSpinBox;
        parSpin->setRange(-200000, 200000);
        parSpin->setSingleStep(0.1);
        parSpin->setDecimals(5);
        parSpin->setValue(value);
        parSpin->setSuffix(" " + currPar->dims->toString());
        parSpin->setProperty("valToChange", "0");
        parSpin->setProperty("ptr", qVariantFromValue((void *) currPar));
        parSpin->setProperty("action","changeVal");
        connect(this, SIGNAL(deleteProperties()), parSpin, SLOT(deleteLater()));

        buttons->addWidget(parSpin);

        // add go back, if we are not a layout
        if (type != "layout") {
            QPushButton * goBack = new QPushButton;
            goBack->setMaximumWidth(28);
            goBack->setMaximumHeight(28);
            goBack->setIcon(QIcon(":/icons/toolbar/delShad.png"));
            goBack->setFlat(true);
            goBack->setToolTip("Select different value type");
            goBack->setFocusPolicy(Qt::NoFocus);
            buttons->addWidget(goBack);
            goBack->setProperty("action","updateType");
            goBack->setProperty("newType","Undefined");
            goBack->setProperty("ptr", qVariantFromValue((void *) currPar));
            connect(goBack, SIGNAL(clicked()), data, SLOT(updatePar()));
            connect(this, SIGNAL(deleteProperties()), goBack, SLOT(deleteLater()));
        }

        varLayout->addRow(name, buttons);
        connect(this, SIGNAL(deleteProperties()), varLayout->itemAt(varLayout->rowCount()-1,QFormLayout::LabelRole)->widget(), SLOT(deleteLater()));

        connect(parSpin, SIGNAL(editingFinished()), data, SLOT (updatePar()));
        parSpin->setProperty("type",type + parType);
        varLayout->itemAt(varLayout->rowCount()-1,QFormLayout::LabelRole)->widget()->setProperty("type",type + parType);
        // add the full parameter name to the tooltip
        varLayout->itemAt(varLayout->rowCount()-1,QFormLayout::LabelRole)->widget()->setToolTip(currPar->name);
        }
        break;


    case Statistical:
        {
        QHBoxLayout * buttons = new QHBoxLayout;
        connect(this, SIGNAL(deleteProperties()), buttons, SLOT(deleteLater()));
        buttons->setSpacing(0);

        value = currPar->value[0];

        // add drop down to choose the distribution from
        QComboBox * distribution = new QComboBox;
        distribution->addItem(QIcon(":/icons/toolbar/delShad.png"),"None");
        distribution->addItem(QIcon(":/icons/toolbar/delShad.png"),"Uniform");
        distribution->addItem(QIcon(":/icons/toolbar/delShad.png"),"Normal");
        //distribution->addItem("None");
        distribution->setMaximumWidth(45);
        distribution->setCurrentIndex(currPar->value[0]);
        distribution->setProperty("ptr", qVariantFromValue((void *) currPar));
        connect(distribution, SIGNAL(currentIndexChanged(int)), data, SLOT (updatePar(int)));
        connect(this, SIGNAL(deleteProperties()), distribution, SLOT(deleteLater()));

        buttons->addWidget(distribution);

        // now draw the spinboxes for the different types:
        switch (int(round(currPar->value[0]))) {

        case 0:
            // no distribution, no spinboxes
            break;

        case 1:
            // uniform distribution - min and max spinboxes
        {
            buttons->addStretch();
            buttons->addWidget(new QLabel(">"));
            connect(this, SIGNAL(deleteProperties()), buttons->itemAt(buttons->count()-1)->widget(), SLOT(deleteLater()));
            QDoubleSpinBox *parSpin = new QDoubleSpinBox;
            parSpin->setRange(-200000, 200000);
            parSpin->setSingleStep(0.1);
            parSpin->setMaximumWidth(60);
            parSpin->setDecimals(5);
            parSpin->setValue(currPar->value[1]);
            parSpin->setProperty("valToChange", "1");
            parSpin->setToolTip("lower bound value");
            parSpin->setProperty("ptr", qVariantFromValue((void *) currPar));
            parSpin->setProperty("action","changeVal");
            connect(parSpin, SIGNAL(valueChanged(double)), data, SLOT (updatePar()));
            connect(this, SIGNAL(deleteProperties()), parSpin, SLOT(deleteLater()));
            buttons->addWidget(parSpin);

            buttons->addStretch();
            buttons->addWidget(new QLabel("<"));
            connect(this, SIGNAL(deleteProperties()), buttons->itemAt(buttons->count()-1)->widget(), SLOT(deleteLater()));

            parSpin = new QDoubleSpinBox;
            parSpin->setRange(-200000, 200000);
            parSpin->setSingleStep(0.1);
            parSpin->setMaximumWidth(60);
            parSpin->setDecimals(5);
            parSpin->setValue(currPar->value[2]);
            parSpin->setProperty("valToChange", "2");
            parSpin->setToolTip("upper bound value");
            parSpin->setProperty("ptr", qVariantFromValue((void *) currPar));
            parSpin->setProperty("action","changeVal");
            connect(parSpin, SIGNAL(valueChanged(double)), data, SLOT (updatePar()));
            connect(this, SIGNAL(deleteProperties()), parSpin, SLOT(deleteLater()));
            buttons->addWidget(parSpin);}
            break;

        case 2:
            // normal distribution - mean and SD spinboxes
        {
            buttons->addStretch();
            QLabel * label = new QLabel("x");
            connect(this, SIGNAL(deleteProperties()), label, SLOT(deleteLater()));
            label->setToolTip("mean value");
            buttons->addWidget(label);
            QDoubleSpinBox *parSpin = new QDoubleSpinBox;
            parSpin->setRange(-200000, 200000);
            parSpin->setSingleStep(0.1);
            parSpin->setMaximumWidth(60);
            parSpin->setDecimals(5);
            parSpin->setValue(currPar->value[1]);
            parSpin->setProperty("valToChange", "1");
            parSpin->setToolTip("mean value");
            parSpin->setProperty("ptr", qVariantFromValue((void *) currPar));
            parSpin->setProperty("action","changeVal");
            connect(parSpin, SIGNAL(editingFinished()), data, SLOT (updatePar()));
            connect(this, SIGNAL(deleteProperties()), parSpin, SLOT(deleteLater()));
            buttons->addWidget(parSpin);

            buttons->addStretch();
            label = new QLabel("s");
            connect(this, SIGNAL(deleteProperties()), label, SLOT(deleteLater()));
            label->setToolTip("standard deviation value");
            buttons->addWidget(label);
            parSpin = new QDoubleSpinBox;
            parSpin->setRange(-200000, 200000);
            parSpin->setSingleStep(0.1);
            parSpin->setMaximumWidth(60);
            parSpin->setDecimals(5);
            parSpin->setValue(currPar->value[2]);
            parSpin->setProperty("valToChange", "2");
            parSpin->setToolTip("standard deviation value");
            parSpin->setProperty("ptr", qVariantFromValue((void *) currPar));
            parSpin->setProperty("action","changeVal");
            connect(parSpin, SIGNAL(editingFinished()), data, SLOT (updatePar()));
            connect(this, SIGNAL(deleteProperties()), parSpin, SLOT(deleteLater()));
            buttons->addWidget(parSpin);}
            break;
        }

        // add seed box
        QLabel * label = new QLabel("seed");
        connect(this, SIGNAL(deleteProperties()), label, SLOT(deleteLater()));
        label->setToolTip("seed for the random numbers");
        buttons->addWidget(label);
        QSpinBox * seedSpin = new QSpinBox;
        seedSpin->setRange(0, 200000);
        seedSpin->setMaximumWidth(60);
        seedSpin->setValue(currPar->value[3]);
        seedSpin->setProperty("valToChange", "3");
        seedSpin->setToolTip("seed value");
        seedSpin->setProperty("ptr", qVariantFromValue((void *) currPar));
        seedSpin->setProperty("action","changeVal");
        connect(seedSpin, SIGNAL(editingFinished()), data, SLOT (updatePar()));
        connect(this, SIGNAL(deleteProperties()), seedSpin, SLOT(deleteLater()));
        buttons->addWidget(seedSpin);

        buttons->addStretch();
        // add go back, if we are not a layout
        if (type != "layout") {
            QPushButton * goBack = new QPushButton;
            goBack->setMaximumWidth(28);
            goBack->setMaximumHeight(28);
            goBack->setIcon(QIcon(":/icons/toolbar/delShad.png"));
            goBack->setFlat(true);
            goBack->setToolTip("Select different value type");
            goBack->setFocusPolicy(Qt::NoFocus);
            buttons->addWidget(goBack);
            goBack->setProperty("action","updateType");
            goBack->setProperty("newType","Undefined");
            goBack->setProperty("ptr", qVariantFromValue((void *) currPar));
            connect(this, SIGNAL(deleteProperties()), goBack, SLOT(deleteLater()));
          connect(goBack, SIGNAL(clicked()), data, SLOT(updatePar()));
        }

        varLayout->addRow(name, buttons);
        connect(this, SIGNAL(deleteProperties()), varLayout->itemAt(varLayout->rowCount()-1,QFormLayout::LabelRole)->widget(), SLOT(deleteLater()));
        varLayout->itemAt(varLayout->rowCount()-1,QFormLayout::LabelRole)->widget()->setProperty("type",type + parType);
        // add the full parameter name to the tooltip
        varLayout->itemAt(varLayout->rowCount()-1,QFormLayout::LabelRole)->widget()->setToolTip(currPar->name);
        }
        break;

    case ExplicitList:
        {
        QHBoxLayout * buttons = new QHBoxLayout;
        connect(this, SIGNAL(deleteProperties()), buttons, SLOT(deleteLater()));
        buttons->setSpacing(0);
        buttons->addStretch();

        // ADD BUTTON FOR LIST VALUES
        QPushButton * currButton = new QPushButton("Edit");
        currButton->setMaximumWidth(70);
        currButton->setMaximumHeight(28);
        currButton->setToolTip("Edit the explicit value list");
        currButton->setProperty("action","editList");
        connect(this, SIGNAL(deleteProperties()), currButton, SLOT(deleteLater()));
        buttons->addWidget(currButton);

        currButton->setProperty("ptr", qVariantFromValue((void *) currPar));
        currButton->setProperty("ptrComp", qVariantFromValue((void *) type9ml));

        // add connection:
        connect(currButton, SIGNAL(clicked()), data, SLOT(updatePar()));

        // add go back, if we are not a layout
        if (type != "layout") {
            QPushButton * goBack = new QPushButton;
            goBack->setMaximumWidth(28);
            goBack->setMaximumHeight(28);
            goBack->setIcon(QIcon(":/icons/toolbar/delShad.png"));
            goBack->setFlat(true);
            goBack->setToolTip("Select different value type");
            goBack->setFocusPolicy(Qt::NoFocus);
            buttons->addWidget(goBack);
            goBack->setProperty("action","updateType");
            goBack->setProperty("newType","Undefined");
            goBack->setProperty("ptr", qVariantFromValue((void *) currPar));
            connect(goBack, SIGNAL(clicked()), data, SLOT(updatePar()));
            connect(this, SIGNAL(deleteProperties()), goBack, SLOT(deleteLater()));
        }

        varLayout->addRow(name, buttons);
        connect(this, SIGNAL(deleteProperties()), varLayout->itemAt(varLayout->rowCount()-1,QFormLayout::LabelRole)->widget(), SLOT(deleteLater()));
        varLayout->itemAt(varLayout->rowCount()-1,QFormLayout::LabelRole)->widget()->setProperty("type",type + parType);
        // add the full parameter name to the tooltip
        varLayout->itemAt(varLayout->rowCount()-1,QFormLayout::LabelRole)->widget()->setToolTip(currPar->name);
        }
        break;
    }

}

