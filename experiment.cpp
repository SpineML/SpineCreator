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

#include "experiment.h"
#include "rootdata.h"
#include "projectobject.h"

experiment::experiment()
{
    // defaults
    setup.dt = 0.1;
    setup.duration = 1;
    setup.solver = ForwardEuler;
    setup.simType = "BRAHMS";
    setup.exptProcedure = SingleRun;

    name = "New Experiment";
    description = "add a description for the experiment here";
    selected = false;
    editing = true;
    subEdit = false;

}

experiment::~experiment() {

    for (int i = 0; i < ins.size(); ++i) {
        delete ins[i];
    }
    for (int i = 0; i < outs.size(); ++i) {
        delete outs[i];
    }
    for (int i = 0; i < lesions.size(); ++i) {
        delete lesions[i];
    }
    for (int i = 0; i < changes.size(); ++i) {
        delete changes[i];
    }

}

experiment::experiment(experiment * /*exptToCopy*/)
{
    // add copy constructor here
}

exptBox * experiment::getBox(viewELExptPanelHandler * panel) {

#ifdef Q_OS_MAC
    QFont titleFont("Helvetica [Cronyx]", 16);
    QFont descFont("Helvetica [Cronyx]", 12);
#else
    QFont titleFont("Helvetica [Cronyx]", 14);
    QFont descFont("Helvetica [Cronyx]", 8);
#endif


    exptBox * box = new exptBox();
    QGridLayout * layout = new QGridLayout;
    box->setLayout(layout);
    layout->setSpacing(0);

    this->currBoxPtr = box;

    int index = -1;

    for (int i = 0; i < panel->data->experiments.size(); ++i) {

        if (panel->data->experiments[i] == this) index = i;
    }
    if (index == -1) {qDebug() << "experiments.cpp: big error"; exit(-1);}

    if (editing)
    {
#ifdef Q_OS_MAC
        layout->setSpacing(6);
#endif
        // set up the experiment editing box
        // title
        currBoxPtr->setStyleSheet("background-color :rgba(200,200,200,255)");
        QRegExp regExp("[A-Za-z0-9 ][A-Za-z0-9 ][A-Za-z0-9 ][A-Za-z0-9 ][A-Za-z0-9 ][A-Za-z0-9 ][A-Za-z0-9 ][A-Za-z0-9 ][A-Za-z0-9 ][A-Za-z0-9 ][A-Za-z0-9 ][A-Za-z0-9 ][A-Za-z0-9 ][A-Za-z0-9 ][A-Za-z0-9 ][A-Za-z0-9 ][A-Za-z0-9 ][A-Za-z0-9 ][A-Za-z0-9 ][A-Za-z0-9 ]");
        QRegExpValidator *validator = new QRegExpValidator(regExp, this);
        QLineEdit * editTitle = new QLineEdit;
        editTitle->setValidator(validator);
        editTitle->setText(name);
        editTitle->setMaximumWidth(401);
        layout->addWidget(editTitle,0,0,1,1);

        // description
        QTextEdit * editDesc = new QTextEdit;
        editDesc->setMaximumWidth(400);
        editDesc->setPlainText(description);
        layout->addWidget(editDesc,1,0,2,1);

        // done button
        QPushButton * done = new QPushButton("Done");
        done->setProperty("index", (int) index);
        done->setProperty("title", qVariantFromValue((void *) editTitle));
        done->setProperty("desc", qVariantFromValue((void *) editDesc));
        QObject::connect(done, SIGNAL(clicked()), panel, SLOT(doneEditExperiment()));
        layout->addWidget(done,0,2,1,1);

        // cancel button
        QPushButton * cancel = new QPushButton("Cancel");
        cancel->setProperty("index", (int) index);
        QObject::connect(cancel, SIGNAL(clicked()), panel, SLOT(cancelEditExperiment()));
        layout->addWidget(cancel,1,2,1,1);

        return box;
    }
    if (selected)
    {
        // set up a selected box
        currBoxPtr->setStyleSheet("background-color :rgba(0,0,0,40)");
        // title
        QLabel * nameLabel = new QLabel(name);
        nameLabel->setFont(titleFont);
        nameLabel->setStyleSheet("background-color :transparent");
        nameLabel->setMinimumWidth(200);
        layout->addWidget(nameLabel,0,0,1,1);

        // dexription
        QLabel * descLabel = new QLabel(description);
        descLabel->setStyleSheet("background-color :transparent");
        descLabel->setMinimumWidth(200);
        descLabel->setWordWrap(true);
        descLabel->setFont(descFont);
        layout->addWidget(descLabel,1,0,2,1);

        // a label to make the layout look nice
        QLabel * spacerLabel = new QLabel();
        spacerLabel->setStyleSheet("background-color :transparent");
        layout->addWidget(spacerLabel,1,1,1,2);

        // up and down buttons
        QPushButton * up = new QPushButton;
        up->setMaximumWidth(32);
        up->setFlat(true);
        up->setIcon(QIcon(":/icons/toolbar/up.png"));
        up->setProperty("index", (int) index);
        up->setProperty("type", 1);
        layout->addWidget(up,0,2,1,1);
        QObject::connect(up, SIGNAL(clicked()), panel, SLOT(moveExperiment()));

        QPushButton * down = new QPushButton;
        down->setMaximumWidth(32);
        down->setFlat(true);
        down->setIcon(QIcon(":/icons/toolbar/down.png"));
        down->setProperty("index", (int) index);
        down->setProperty("type", 0);
        layout->addWidget(down,0,3,1,1);

        QObject::connect(down, SIGNAL(clicked()), panel, SLOT(moveExperiment()));
        if (index == 0) {
            up->setEnabled(false);
        }
        if (index == (int) panel->data->experiments.size()-1) {
            down->setEnabled(false);
        }

        // edit button
        QPushButton * edit = new QPushButton;
        edit->setMaximumWidth(32);
        //edit->setIconSize(QSize(32,32));
        edit->setFlat(true);
        edit->setIcon(QIcon(":/icons/toolbar/edit.png"));
        edit->setProperty("index", (int) index);
        layout->addWidget(edit,1,3,1,1);
        QObject::connect(edit, SIGNAL(clicked()), panel, SLOT(editExperiment()));

        // delete button
        QPushButton * del = new QPushButton;
        del->setMaximumWidth(32);
        del->setFlat(true);
        del->setIcon(QIcon(":/icons/toolbar/delShad.png"));
        del->setProperty("index", (int) index);
        layout->addWidget(del,2,3,1,1);
        QObject::connect(del, SIGNAL(clicked()), panel, SLOT(delExperiment()));

        // run button...
        QCommonStyle style;
        QToolButton * run = new QToolButton();
        // if any subcomponents of the experiment asre being edited we should disable this
        if (this->subEdit) {
            run->setText("Run (disabled while editing)");
            run->setEnabled(false);
        } else {
            run->setText("Run experiment");
        }
        run->setStyleSheet("QToolButton { color: black; border: 0px; background-color :transparent;}");
        run->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        run->setToolTip("Run the experiment in the chosen simulator");
        run->setIcon(style.standardIcon(QStyle::SP_MediaPlay));
        layout->addWidget(run,3,0,1,1);
        QObject::connect(run, SIGNAL(clicked()), panel, SLOT(run()));

    }
    else
    {
        // set up an unselected box
        currBoxPtr->setStyleSheet("background-color :transparent");
        QLabel * nameLabel = new QLabel(name);
        layout->addWidget(nameLabel,0,0,1,1);
        nameLabel->setFont(titleFont);

        QPushButton * up = new QPushButton;
        up->setMaximumWidth(32);
        up->setFlat(true);
        up->setIcon(QIcon(":/icons/toolbar/up.png"));
        up->setProperty("index", (int) index);
        up->setProperty("type", 1);
        layout->addWidget(up,0,2,1,1);
        QObject::connect(up, SIGNAL(clicked()), panel, SLOT(moveExperiment()));

        QPushButton * down = new QPushButton;
        down->setMaximumWidth(32);
        down->setFlat(true);
        down->setIcon(QIcon(":/icons/toolbar/down.png"));
        down->setProperty("index", (int) index);
        down->setProperty("type", 0);
        layout->addWidget(down,0,3,1,1);
        QObject::connect(down, SIGNAL(clicked()), panel, SLOT(moveExperiment()));

        if (index == 0) {
            up->setEnabled(false);
        }
        if (index == (int) panel->data->experiments.size()-1) {
            down->setEnabled(false);
        }
    }

    return box;

}

void experiment::select(QVector < experiment * > * experiments) {

    // deselect all
    for (int i = 0; i < experiments->size(); ++i) {
        (*experiments)[i]->deselect();
        (*experiments)[i]->editing = false;
    }

    // select this experiment
    this->selected = true;
}

void experiment::deselect() {

    // select this experiment
    this->selected = false;

}

void experiment::purgeBadPointer(QSharedPointer <NineMLComponentData> ptr)
{
    // inputs
    for (int i = 0; i < ins.size(); ++i) {

        exptInput * in = ins[i];

        // are we using the component?
        if (in->target == ptr) {
            ins.erase(ins.begin()+i);
            qDebug() << "Deleting this (in)";
            delete in;
            --i;
        }
    }

    // outputs
    for (int i = 0; i < outs.size(); ++i) {

        exptOutput * out = outs[i];

        // are we using the component?
        if (out->source == ptr) {
            outs.erase(outs.begin()+i);
            qDebug() << "Deleting this (out)";
            delete out; // You're deleting the "this" that called purgeBadPointer.
            --i;
        }
    }

    // pars
    for (int i = 0; i < changes.size(); ++i) {

        exptChangeProp * change = changes[i];

        // are we using the component?
        if (change->component == ptr) {
            changes.erase(changes.begin()+i);
            qDebug() << "Deleting this (change)";
            delete change;
            --i;

        }
    }
}

void experiment::purgeBadPointer(QSharedPointer<NineMLComponent> ptr, QSharedPointer<NineMLComponent> newPtr) {

    // inputs
    for (int i = 0; i < ins.size(); ++i) {

        exptInput * in = ins[i];

        // are we using the component?
        if (!in->target.isNull()
            && in->target->component == ptr && in->portName.size() > 0) {

            // if so, update
            // port
            bool portFound = false;
            if (in->portIsAnalog) {
                for (int j = 0; j < newPtr->AnalogPortList.size(); ++j)
                    if (newPtr->AnalogPortList[j]->name == in->portName && (newPtr->AnalogPortList[j]->mode == AnalogRecvPort || newPtr->AnalogPortList[j]->mode == AnalogReducePort)) {
                        portFound = true;
                    }
            } else {
                for (int j = 0; j < newPtr->EventPortList.size(); ++j)
                    if (newPtr->EventPortList[j]->name == in->portName && newPtr->EventPortList[j]->mode == EventRecvPort){
                        portFound = true;
                    }
                for (int j = 0; j < newPtr->ImpulsePortList.size(); ++j)
                    if (newPtr->ImpulsePortList[j]->name == in->portName && newPtr->ImpulsePortList[j]->mode == ImpulseRecvPort){
                        portFound = true;
                    }
            }

            // if not found
            if (!portFound) {
                in->portName = "";
                in->set = false;
                in->edit = true;
            }

            // component
            in->target->component = newPtr;

        } else if (in->target.isNull()) {
            DBG() << "Warning: one of the experiment inputs had a null target in experiment::purgeBadPointer";
        }
    }

    // outputs
    for (int i = 0; i < outs.size(); ++i) {

        exptOutput * out = outs[i];

        // are we using the component?
        if (out->source && out->source->component == ptr && out->portName.size() > 0) {

            // if so, update
            // port
            bool portFound = false;
            if (out->portIsAnalog) {
                for (int j = 0; j < newPtr->AnalogPortList.size(); ++j)
                    if (newPtr->AnalogPortList[j]->name == out->portName && newPtr->AnalogPortList[j]->mode == AnalogSendPort) {
                        portFound = true;
                    }
            } else {
                for (int j = 0; j < newPtr->EventPortList.size(); ++j)
                    if (newPtr->EventPortList[j]->name == out->portName && newPtr->EventPortList[j]->mode== EventSendPort){
                        portFound = true;
                    }
                for (int j = 0; j < newPtr->ImpulsePortList.size(); ++j)
                    if (newPtr->ImpulsePortList[j]->name == out->portName && newPtr->ImpulsePortList[j]->mode == ImpulseSendPort){
                        portFound = true;
                    }
            }

            // if not found
            if (!portFound) {
                out->portName = "";
                out->set = false;
                out->edit = true;
                qDebug() << "moo3";
            }

            // component
            out->source->component = newPtr;

        } else if (!out->source) {
            DBG() << "Warning: one of the experiment outputs had a null source in experiment::purgeBadPointer";
        }
    }
}

void experiment::updateChanges(QSharedPointer <NineMLComponentData> ptr) {

    // par changes
    for (int i = 0; i < changes.size(); ++i) {

        exptChangeProp * change = changes[i];

        // are we using the component?
        if (change->component == ptr) {

            // if so, update
            // property
            bool propFound = false;
            for (int j = 0; j < ptr->ParameterList.size(); ++j)
                if (ptr->ParameterList[j]->name == change->par->name) {
                    propFound = true;
                    // This code is bad as there is no need to do this
                    //delete change->par;
                    //change->par = new ParameterData(ptr->ParameterList[j]);
                }
            for (int j = 0; j < ptr->StateVariableList.size(); ++j)
                if (ptr->StateVariableList[j]->name == change->par->name){
                    propFound = true;
                    // This code is bad as there is no need to do this
                    //delete change->par;
                    //change->par = new StateVariableData(ptr->StateVariableList[j]);
                }

            // if not found
            if (!propFound) {
                change->par = NULL;
                change->set = false;
                change->edit = true;
            }

            // component
            change->component = ptr;

        }


    }

}

// ################################## exptBox

exptBox::exptBox( QWidget * parent ):QFrame(parent)
{
    this->setStyleSheet("background-color :transparent");
}

void exptBox::mouseReleaseEvent ( QMouseEvent * )
{
    emit clicked();
}

// ################################### exptIn

QVBoxLayout * exptInput::drawInput(rootData * data, viewELExptPanelHandler *handler) {

    QVBoxLayout * layout = new QVBoxLayout;

    bool portsExist = false;

    QStringList elementList;
    for (int i = 0; i < data->populations.size(); ++i) {
        portsExist = false;
        for (int p = 0; p < data->populations[i]->neuronType->component->AnalogPortList.size(); ++p) {
            if (data->populations[i]->neuronType->component->AnalogPortList[p]->mode== AnalogRecvPort \
                    || data->populations[i]->neuronType->component->AnalogPortList[p]->mode== AnalogReducePort) {
                portsExist = true;
                break;
            }
        }
        if (!portsExist) {
            // if it is a spike source we don't care about the component
            if (data->populations[i]->isSpikeSource) {
                portsExist = true;
            }
        }
        if (portsExist) {
            elementList << data->populations[i]->neuronType->getXMLName();
        }
        for (int j = 0; j < data->populations[i]->projections.size(); ++j) {
            for (int k = 0; k < data->populations[i]->projections[j]->synapses.size(); ++k) {
                portsExist = false;
                for (int p = 0; p < data->populations[i]->projections[j]->synapses[k]->weightUpdateType->component->AnalogPortList.size(); ++p) {
                    if (data->populations[i]->projections[j]->synapses[k]->weightUpdateType->component->AnalogPortList[p]->mode== AnalogRecvPort \
                            || data->populations[i]->projections[j]->synapses[k]->weightUpdateType->component->AnalogPortList[p]->mode== AnalogReducePort) {
                        portsExist = true;
                        break;
                    }
                }
                /*if (!portsExist) {
                    for (int p = 0; p < data->network[i]->projections[j]->synapses[k]->weightUpdateType->component->EventPortList.size(); ++p) {
                        if (data->network[i]->projections[j]->synapses[k]->weightUpdateType->component->EventPortList[p]->mode== EventRecvPort  && (data->network[i]->neuronType->component->name == "SpikeSource")) {
                            portsExist = true;
                            break;
                        }
                    }
                }*/
                if (portsExist)
                    elementList << data->populations[i]->projections[j]->synapses[k]->weightUpdateType->getXMLName();
                portsExist = false;
                for (int p = 0; p < data->populations[i]->projections[j]->synapses[k]->postsynapseType->component->AnalogPortList.size(); ++p) {
                    if (data->populations[i]->projections[j]->synapses[k]->postsynapseType->component->AnalogPortList[p]->mode== AnalogRecvPort \
                            || data->populations[i]->projections[j]->synapses[k]->postsynapseType->component->AnalogPortList[p]->mode== AnalogReducePort) {
                        portsExist = true;
                        break;
                    }
                }
                /*if (!portsExist) {
                    for (int p = 0; p < data->network[i]->projections[j]->synapses[k]->postsynapseType->component->EventPortList.size(); ++p) {
                        if (data->network[i]->projections[j]->synapses[k]->postsynapseType->component->EventPortList[p]->mode== EventRecvPort  && (data->network[i]->neuronType->component->name == "SpikeSource")) {
                            portsExist = true;
                            break;
                        }
                    }
                }*/
                if (portsExist)
                    elementList << data->populations[i]->projections[j]->synapses[k]->postsynapseType->getXMLName();
            }
        }
    }

    if (edit) {
        // frame to add
        QFrame * frame = new QFrame;

        // This doesn't work quite as hoped, so commented out for all platforms:
        // frame->setStyleSheet("background-color :rgba(0,0,0,40)");

        layout->addWidget(frame);
        frame->setMaximumWidth(380);
        frame->setContentsMargins(0,0,0,0);


        // new layout to contain name and port boxes
        QHBoxLayout * nameAndPort = new QHBoxLayout;
        frame->setLayout(new QVBoxLayout);
        QVBoxLayout *frameLay = qobject_cast<QVBoxLayout *> (frame->layout());
        CHECK_CAST(frameLay)

        // input name
        QLineEdit * nameEdit = new QLineEdit;
        nameEdit->setText(this->name);
        nameEdit->setProperty("ptr", qVariantFromValue((void *) this));
        connect(nameEdit, SIGNAL(editingFinished()), handler, SLOT(setInputName()));
        frameLay->addWidget(nameEdit);

        frameLay->addLayout(nameAndPort);
        frameLay->setContentsMargins(0,0,0,0);
        frameLay->setSpacing(0);
        nameAndPort->setContentsMargins(0,0,0,0);
        nameAndPort->setSpacing(2);

        // add Component LineEdit
        QLineEdit * lineEdit = new QLineEdit;
        if (set) {
            lineEdit->setText(this->target->getXMLName());
        } else {
            lineEdit->setText("Type component name");
        }
        QCompleter *completer = new QCompleter(elementList, this);
        completer->setCaseSensitivity(Qt::CaseInsensitive);
        lineEdit->setCompleter(completer);
        lineEdit->setMaximumWidth(220);
        lineEdit->setProperty("ptr", qVariantFromValue((void *) this));
        connect(lineEdit, SIGNAL(editingFinished()), handler, SLOT(setInputComponent()));
        nameAndPort->addWidget(lineEdit);

        // add port dropdown
        QComboBox * portBox = new QComboBox;
        portBox->setMaximumWidth(90);
        if (set) {
            // if broken NOT NEEDED ANYMORE
            /*if (!data->isValidPointer(this->target)) {
                set = false;
                return this->drawInput(data, handler);
            }*/
            // if spike source
            if (this->target->owner->type == populationObject) {
                QSharedPointer <population> pop = qSharedPointerDynamicCast<population> (this->target->owner);
                CHECK_CAST(pop)
                if (pop->isSpikeSource) {
                    portBox->addItem("spike in");
                    portBox->setDisabled(true);
                    // point to dummy event port
                    portName = "in";
                    portIsAnalog = false;
                }
            }
            // add
            portBox->addItem("AnalogPorts");
            int currentRow = 0;
            QModelIndex ind = portBox->model()->index(currentRow,0);
            portBox->model()->setData(ind, QVariant(0), Qt::UserRole-1);
            ++currentRow;
             for (int i = 0; i < this->target->component->AnalogPortList.size(); ++i) {
                if (this->target->component->AnalogPortList[i]->mode == AnalogRecvPort || this->target->component->AnalogPortList[i]->mode == AnalogReducePort) {
                    portBox->addItem(this->target->component->AnalogPortList[i]->name, qVariantFromValue((void *) this->target->component->AnalogPortList[i]));
                    if (portName.size() == 0) {
                        portBox->setCurrentIndex(1);
                        this->portName = this->target->component->AnalogPortList[i]->name;
                        this->portIsAnalog = true;
                    } else {
                        if (this->portName == this->target->component->AnalogPortList[i]->name)
                           portBox->setCurrentIndex(currentRow);
                    }
                    ++currentRow;
                }
            }
             // not needed with spike sources
            /*portBox->addItem("EventPorts");
            ind = portBox->model()->index(currentRow,0);
            portBox->model()->setData(ind, QVariant(0), Qt::UserRole-1);
            for (int i = 0; i < this->target->component->EventPortList.size(); ++i) {
                if (this->target->component->EventPortList[i]->mode == EventRecvPort) {
                    portBox->addItem(this->target->component->EventPortList[i]->name, qVariantFromValue((void *) this->target->component->EventPortList[i]));
                    if (!(portBox->currentIndex() == 1)) {
                        if (port==NULL) {
                            portBox->setCurrentIndex(2);
                            this->port = this->target->component->EventPortList[i];
                        } else {
                            if (this->port == this->target->component->EventPortList[i])
                               portBox->setCurrentIndex(currentRow+1);
                        }
                    }
                    ++currentRow;
                }
            }*/
        } else {
            portBox->setDisabled(true);
        }
        portBox->setProperty("ptr", qVariantFromValue((void *) this));
        nameAndPort->addWidget(portBox);
        connect(portBox, SIGNAL(currentIndexChanged(int)), handler, SLOT(setInputPort(int)));

        // if input is set, then add the input type section
        if (this->set) {

            // Type dropdown
            QComboBox * type = new QComboBox;
            type->setProperty("ptr", qVariantFromValue((void *) this));
            type->addItem(exptInLookup::toString(constant), (int) constant); if (this->inType == constant) type->setCurrentIndex(0);
            type->addItem(exptInLookup::toString(arrayConstant), (int) arrayConstant); if (this->inType == arrayConstant) type->setCurrentIndex(1);
            type->addItem(exptInLookup::toString(timevarying), (int) timevarying); if (this->inType == timevarying) type->setCurrentIndex(2);
            type->addItem(exptInLookup::toString(arrayTimevarying), (int) arrayTimevarying); if (this->inType == arrayTimevarying) type->setCurrentIndex(3);
            type->addItem(exptInLookup::toString(external), (int) external); if (this->inType == external) type->setCurrentIndex(4);
            // spike source only for explicit spike list
            if (!this->portIsAnalog)
                type->addItem(exptInLookup::toString(spikeList), (int) spikeList); if (this->inType == spikeList) type->setCurrentIndex(5);
            QObject ::connect(type, SIGNAL(currentIndexChanged(int)), handler, SLOT(setInputType(int)));
            frameLay->addWidget(type);

            // rate distribution
            if (!this->portIsAnalog) {
                QFormLayout * formLay = new QFormLayout;
                QComboBox * dist = new QComboBox;
                dist->addItem("Regular");
                dist->addItem("Poisson");
                if (rateDistribution == Regular)
                    dist->setCurrentIndex(0);
                if (rateDistribution == Poisson)
                    dist->setCurrentIndex(1);
                formLay->addRow("Rate distribution:", dist);
                frameLay->addLayout(formLay);
                dist->setProperty("ptr", qVariantFromValue((void *) this));
                QObject ::connect(dist, SIGNAL(currentIndexChanged(int)), handler, SLOT(setInputRateDistributionType(int)));
            }

            // extra type bits
            if (this->inType == constant) {
                QFormLayout * formLay = new QFormLayout;
                QDoubleSpinBox * spin = new QDoubleSpinBox;
                spin->setMaximum(10000.0);
                spin->setMinimum(-10000.0);
                spin->setDecimals(6);
                if (!this->params.empty()) {
                    spin->setValue(this->params[0]);
                }
                frameLay->addLayout(formLay);
                if (this->portIsAnalog)
                    formLay->addRow(" Value:", spin);
                else
                    formLay->addRow(" Rate:", spin);
                spin->setProperty("ptr", qVariantFromValue((void *) this));
                QObject ::connect(spin, SIGNAL(valueChanged(double)), handler, SLOT(setInputTypeData(double)));
            }

            if (this->inType == timevarying) {
                QGridLayout * gridlay = new QGridLayout;
                gridlay->setMargin(0);
                frameLay->addLayout(gridlay);
                QTableWidget * table = new QTableWidget;
                table->setProperty("ptr", qVariantFromValue((void *) this));
                table->setItemDelegate(new NTableDelegate(table));
                gridlay->addWidget(table,0,0,4,1);
                table->setColumnCount(2);
                // add items from params
                if (params.size()<2) {
                    params.resize(2);
                    params[0] = 0;
                    params[1] = 1;
                }
                table->setRowCount(qCeil(params.size()/2));
                for (int i = 0; i < params.size(); ++i) {
                    QTableWidgetItem * item = new QTableWidgetItem;
                    item->setData(Qt::DisplayRole, params[i]);
                    table->setItem(qFloor(i/2), i%2, item);
                }

                // labels for header
                QStringList headers;
                headers.push_back("Time (ms)");
                if (this->portIsAnalog)
                    headers.push_back("Value");
                else
                    headers.push_back("Rate");
                table->setHorizontalHeaderLabels(headers);
                table->verticalHeader()->hide();
                table->setMaximumWidth(220);

                QObject::connect(table, SIGNAL(cellChanged(int,int)), handler, SLOT(setInputParams(int,int)));

                // add row
                QPushButton * addRow = new QPushButton("Add row");
                addRow->setProperty("ptr", qVariantFromValue((void *) this));
                gridlay->addWidget(addRow,2,1,1,1);
                QObject::connect(addRow, SIGNAL(clicked()), handler, SLOT(setInputAddTVRow()));

                // del row
                QPushButton * delRow = new QPushButton("Delete row");
                delRow->setProperty("ptr", qVariantFromValue((void *) this));
                gridlay->addWidget(delRow,3,1,1,1);
                QObject::connect(delRow, SIGNAL(clicked()), handler, SLOT(setInputDelTVRow()));
                if (params.size() < 3) {
                    delRow->setDisabled(true);
                }
            }

            if (this->inType == arrayConstant) {
                QGridLayout * gridlay = new QGridLayout;
                gridlay->setMargin(0);
                frameLay->addLayout(gridlay);
                QTableWidget * table = new QTableWidget;
                table->setProperty("ptr", qVariantFromValue((void *) this));
                table->setItemDelegate(new NTableDelegate(table));
                gridlay->addWidget(table,0,0,4,1);
                int componentSize;
                if (this->target->owner->type == populationObject) {
                    QSharedPointer <population> pop = qSharedPointerDynamicCast<population> (this->target->owner);
                    CHECK_CAST(pop)
                    componentSize = pop->numNeurons;
                }
                if (this->target->owner->type == projectionObject) {
                    QSharedPointer <projection> proj = qSharedPointerDynamicCast<projection> (this->target->owner);
                    CHECK_CAST(proj)
                    componentSize = proj->destination->numNeurons;
                }
                table->setColumnCount(1);
                table->setRowCount(componentSize);
                if (componentSize == 0) {
                    DBG() << "Warning: resizing this->params to 0 (exptInput::drawInput)";
                }
                this->params.resize(componentSize);
                this->params.fill(0);
                // add items from params
                for (int i = 0; i < params.size(); ++i) {
                    QTableWidgetItem * item = new QTableWidgetItem;
                    item->setData(Qt::DisplayRole, params[i]);
                    table->setItem(i, 0, item);
                }

                // fill with values
                QDoubleSpinBox * fillValue = new QDoubleSpinBox;
                fillValue->setMinimum(-10000.0);
                fillValue->setMaximum(10000.0);
                fillValue->setDecimals(6);
                gridlay->addWidget(fillValue,0,1,1,1);
                QPushButton * fill = new QPushButton("Fill");
                fill->setToolTip("Fill all indices with the value above");
                gridlay->addWidget(fill,1,1,1,1);
                fill->setProperty("ptr", qVariantFromValue((void *) this));
                fill->setProperty("ptrSpin", qVariantFromValue((void *) fillValue));
                QObject::connect(fill, SIGNAL(clicked()), handler, SLOT(fillInputParams()));

                // labels for header
                QStringList headers;
                headers.push_back("Value");
                table->setHorizontalHeaderLabels(headers);
                table->setMaximumWidth(220);

                QObject::connect(table, SIGNAL(cellChanged(int,int)), handler, SLOT(setInputParams(int,int)));

            }

            if (this->inType == arrayTimevarying) {
                QGridLayout * gridlay = new QGridLayout;
                gridlay->setMargin(0);
                frameLay->addLayout(gridlay);
                QTableWidget * table = new QTableWidget;
                table->setProperty("ptr", qVariantFromValue((void *) this));
                table->setItemDelegate(new NTableDelegate(table));
                gridlay->addWidget(table,0,0,4,1);
                table->setColumnCount(2);

                // add items from params
                int index = -1;
                int indexIndex = 0;
                bool indexFound = false;
                for (int i = 0; i < params.size(); i+=2) {
                    if (params[i] == -1) {
                        index = params[i+1];
                        indexIndex = i;
                        if (index == currentIndex)
                            indexFound = true;
                        continue;
                    }
                    if (index == currentIndex) {
                        table->setRowCount(table->rowCount()+1);
                        QTableWidgetItem * item = new QTableWidgetItem;
                        item->setData(Qt::DisplayRole, params[i]);
                        table->setItem(qFloor((i-indexIndex-2.0)/2.0), (i-indexIndex-2)%2, item);
                        item = new QTableWidgetItem;
                        item->setData(Qt::DisplayRole, params[i+1]);
                        table->setItem(qFloor((i+1-indexIndex-2.0)/2.0), (i+1-indexIndex-2)%2, item);

                    }

                }

                // if nothing found then add new
                if (!indexFound) {
                    params.push_back(-1);
                    params.push_back(currentIndex);
                    params.push_back(0);
                    params.push_back(1);
                    table->setRowCount(1);
                    QTableWidgetItem * item = new QTableWidgetItem;
                    item->setData(Qt::DisplayRole, params[params.size()-2]);
                    table->setItem(0, 0, item);
                    item = new QTableWidgetItem;
                    item->setData(Qt::DisplayRole, params[params.size()-1]);
                    table->setItem(0, 1, item);
                }

                // labels for header
                QStringList headers;
                headers.push_back("Time (ms)");
                if (this->portIsAnalog)
                    headers.push_back("Value");
                else
                    headers.push_back("Rate");
                table->setHorizontalHeaderLabels(headers);
                table->verticalHeader()->hide();
                table->setMaximumWidth(220);

                QObject::connect(table, SIGNAL(cellChanged(int,int)), handler, SLOT(setInputParams(int,int)));

                // index spinbox
                QSpinBox * spin = new QSpinBox;
                spin->setProperty("ptr", qVariantFromValue((void *) this));
                spin->setMinimum(0);
                spin->setToolTip ("Index of the neuron within the population");
                int componentSize;
                if (this->target->owner->type == populationObject) {
                    QSharedPointer <population> pop = qSharedPointerDynamicCast<population> (this->target->owner);
                    CHECK_CAST(pop)
                    componentSize = pop->numNeurons;
                }
                if (this->target->owner->type == projectionObject) {
                    QSharedPointer <projection> proj = qSharedPointerDynamicCast<projection> (this->target->owner);
                    CHECK_CAST(proj)
                    componentSize = proj->destination->numNeurons;
                }
                spin->setMaximum(componentSize-1);
                spin->setValue(currentIndex);
                gridlay->addWidget(spin,0,1,1,1);
                QObject::connect(spin, SIGNAL(valueChanged(int)), handler, SLOT(changeInputIndex(int)));

                // add row
                QPushButton * addRow = new QPushButton("Add row");
                addRow->setProperty("ptr", qVariantFromValue((void *) this));
                gridlay->addWidget(addRow,2,1,1,1);
                QObject::connect(addRow, SIGNAL(clicked()), handler, SLOT(setInputAddTVRow()));

                // del row
                QPushButton * delRow = new QPushButton("Delete row");
                delRow->setProperty("ptr", qVariantFromValue((void *) this));
                gridlay->addWidget(delRow,3,1,1,1);
                QObject::connect(delRow, SIGNAL(clicked()), handler, SLOT(setInputDelTVRow()));
                if (params.size() < 3) {
                    delRow->setDisabled(true);
                }
            }

            // external object is a bit different!
            if (this->inType == external) {

                QFormLayout * formlay = new QFormLayout;
                formlay->setMargin(0);
                frameLay->addLayout(formlay);

                // add items from params
                // command line
                QLineEdit * command = new QLineEdit;
                command->setProperty("ptr", qVariantFromValue((void *) this));
                command->setProperty("type", "command");
                command->setText(externalInput.commandline);
                QObject ::connect(command, SIGNAL(editingFinished()), handler, SLOT(setInputExternalData()));
                command->setToolTip("A command to launch the external input (if required, otherwise leave blank)");
                formlay->addRow("Command:", command);

                // host ip
                QLineEdit * host = new QLineEdit;
                host->setProperty("ptr", qVariantFromValue((void *) this));
                host->setProperty("type", "host");
                host->setText(externalInput.host);
                host->setToolTip("The host computer IP address, or 127.0.0.1 if the external input is on the same computer as the experiment");
                QObject ::connect(host, SIGNAL(editingFinished()), handler, SLOT(setInputExternalData()));
                formlay->addRow("Host IP:", host);

                // host port
                QSpinBox * port = new QSpinBox;
                port->setProperty("ptr", qVariantFromValue((void *) this));
                port->setProperty("type", "port");
                port->setMinimum(49152);
                port->setMaximum(65535);
                port->setValue(externalInput.port);
                port->setToolTip("The port that the external input will be expecting a connection on");
                QObject ::connect(port, SIGNAL(valueChanged(int)), handler, SLOT(setInputExternalData()));
                formlay->addRow("Port:", port);

                // host timestep
                QDoubleSpinBox * timestep = new QDoubleSpinBox;
                timestep->setProperty("ptr", qVariantFromValue((void *) this));
                timestep->setProperty("type", "timestep");
                timestep->setMinimum(0.0);
                timestep->setMaximum(1000);
                timestep->setToolTip("The timestep in ms of the external source (0.0 locks the timestep to the current experiment timestep)");
                timestep->setValue(externalInput.timestep);
                QObject ::connect(timestep, SIGNAL(valueChanged(double)), handler, SLOT(setInputExternalData()));
                formlay->addRow("dt:", timestep);

                // size
                QSpinBox * size = new QSpinBox;
                size->setProperty("ptr", qVariantFromValue((void *) this));
                size->setProperty("type", "size");
                size->setMinimum(1);
                size->setMaximum(100000000);
                if (this->target->owner->type == populationObject) {
                    QSharedPointer <population> pop = qSharedPointerDynamicCast<population> (this->target->owner);
                    CHECK_CAST(pop)
                    this->externalInput.size = pop->numNeurons;
                }
                size->setValue(externalInput.size);
                size->setEnabled(false);
                QObject ::connect(size, SIGNAL(valueChanged(int)), handler, SLOT(setInputExternalData()));
                formlay->addRow("Size:", size);

            }

        }

        // accept all
        QPushButton * accept = new QPushButton;
        accept->setMaximumWidth(32);
        accept->setFlat(true);
        accept->setIcon(QIcon(":/icons/toolbar/acptShad.png"));
        accept->setProperty("ptr", qVariantFromValue((void *) this));
        nameAndPort->addWidget(accept);
        if (!set)
            accept->setDisabled(true);
        QObject::connect(accept, SIGNAL(clicked()), handler, SLOT(acceptInput()));

        // delete
        QPushButton * del = new QPushButton;
        del->setMaximumWidth(32);
        del->setFlat(true);
        del->setIcon(QIcon(":/icons/toolbar/delShad.png"));
        del->setProperty("ptr", qVariantFromValue((void *) this));
        nameAndPort->addWidget(del);
        QObject::connect(del, SIGNAL(clicked()), handler, SLOT(delInput()));



    } else {

        // check for badness NOT NEEDED ANYMORE
        /*if (!data->isValidPointer(this->target)) {
            qDebug() << "input refers to target that can't be found";
            this->set = false;
            this->edit = true;
            return this->drawInput(data, handler);
        }*/

        // new layout to contain name and port boxes
        QFrame * frame = new QFrame;
        frame->setStyleSheet("background-color :rgba(255,255,255,255)");
        layout->addWidget(frame);

        QGridLayout * descAndEdit = new QGridLayout;
        frame->setLayout(descAndEdit);

        // add name
        QLabel * name = new QLabel(this->name);
        name->setMaximumWidth(200);
        name->setWordWrap(true);
        descAndEdit->addWidget(name,0,0,1,1);
        QFont nameFont("Helvetica [Cronyx]", 12);
        name->setFont(nameFont);

        // create string:
        QString desc;
        desc = "Input to component <b>" + this->target->getXMLName() + "</b> port <b>" + this->portName + "</b>.\n";

        if (this->inType == constant) {
            // FIXME: What if this->params is empty here?
            if (this->portIsAnalog) {
                desc += "Constant analog input with a value of <b>" + QString::number(this->params[0]) + "</b>.";
            } else {
                desc += "Spiking input with a constant spike rate of <b>" + QString::number(this->params[0]) + "</b>.";
            }
        }
        if (this->inType == timevarying) {

            TVGraph * drawable = new TVGraph(this->params);
            drawable->setMinimumWidth(200);
            drawable->setMinimumHeight(55);

            descAndEdit->addWidget(drawable,2,0,1,1);

        }
        if (this->inType == arrayTimevarying) {
            desc += "Up to the first 10 neuron inputs are shown.";
            TVGraph * drawable = new TVGraph(this->params);
            drawable->setMinimumWidth(200);
            drawable->setMinimumHeight(55);

            descAndEdit->addWidget(drawable,2,0,1,1);
        }

        if (this->inType == external) {

            if (this->portIsAnalog)
                desc += "External analog input from " + this->externalInput.host + " on port " + QString::number(this->externalInput.port) + " with timestep " + QString::number(this->externalInput.timestep);
            else
                desc += "External spike rate input from"  + this->externalInput.host + " on port " + QString::number(this->externalInput.port) + " with timestep " + QString::number(this->externalInput.timestep);

        }

        // description of input
        QLabel * description = new QLabel(desc);
        description->setMaximumWidth(200);
        description->setWordWrap(true);
        descAndEdit->addWidget(description,1,0,1,1);
        QFont descFont("Helvetica [Cronyx]", 8);
        description->setFont(descFont);

        // edit button
        QPushButton * edit = new QPushButton;
        edit->setMaximumWidth(32);
        edit->setFlat(true);
        edit->setIcon(QIcon(":/icons/toolbar/edit.png"));
        edit->setProperty("ptr", qVariantFromValue((void *) this));
        descAndEdit->addWidget(edit,1,1,1,1);
        QObject::connect(edit, SIGNAL(clicked()), handler, SLOT(editInput()));
    }

    return layout;
}



// ################################### exptOut

QVBoxLayout * exptOutput::drawOutput(rootData * data, viewELExptPanelHandler *handler) {

    QVBoxLayout * layout = new QVBoxLayout;

    bool portsExist = false;

    QStringList elementList;
    for (int i = 0; i < data->populations.size(); ++i) {
        portsExist = false;
        for (int p = 0; p < data->populations[i]->neuronType->component->AnalogPortList.size(); ++p) {
            if (data->populations[i]->neuronType->component->AnalogPortList[p]->mode== AnalogSendPort) {
                portsExist = true;
                break;
            }
        }
        if (!portsExist) {
            for (int p = 0; p < data->populations[i]->neuronType->component->EventPortList.size(); ++p) {
                if (data->populations[i]->neuronType->component->EventPortList[p]->mode== EventSendPort) {
                    portsExist = true;
                    break;
                }
            }
        }
        if (portsExist)
            elementList << data->populations[i]->neuronType->getXMLName();
        for (int j = 0; j < data->populations[i]->projections.size(); ++j) {
            for (int k = 0; k < data->populations[i]->projections[j]->synapses.size(); ++k) {
                portsExist = false;
                for (int p = 0; p < data->populations[i]->projections[j]->synapses[k]->weightUpdateType->component->AnalogPortList.size(); ++p) {
                    if (data->populations[i]->projections[j]->synapses[k]->weightUpdateType->component->AnalogPortList[p]->mode== AnalogSendPort) {
                        portsExist = true;
                        break;
                    }
                }
                if (!portsExist) {
                    for (int p = 0; p < data->populations[i]->projections[j]->synapses[k]->weightUpdateType->component->EventPortList.size(); ++p) {
                        if (data->populations[i]->projections[j]->synapses[k]->weightUpdateType->component->EventPortList[p]->mode== EventSendPort) {
                            portsExist = true;
                            break;
                        }
                    }
                }
                if (portsExist)
                    elementList << data->populations[i]->projections[j]->synapses[k]->weightUpdateType->getXMLName();
                portsExist = false;
                for (int p = 0; p < data->populations[i]->projections[j]->synapses[k]->postsynapseType->component->AnalogPortList.size(); ++p) {
                    if (data->populations[i]->projections[j]->synapses[k]->postsynapseType->component->AnalogPortList[p]->mode== AnalogSendPort) {
                        portsExist = true;
                        break;
                    }
                }
                if (!portsExist) {
                    for (int p = 0; p < data->populations[i]->projections[j]->synapses[k]->postsynapseType->component->EventPortList.size(); ++p) {
                        if (data->populations[i]->projections[j]->synapses[k]->postsynapseType->component->EventPortList[p]->mode== EventSendPort) {
                            portsExist = true;
                            break;
                        }
                    }
                }
                if (portsExist)
                    elementList << data->populations[i]->projections[j]->synapses[k]->postsynapseType->getXMLName();
            }
        }
    }

    if (edit) {
        // frame to add
        QFrame * frame = new QFrame;
        // setStyleSheet removed here.
        layout->addWidget(frame);
        frame->setMaximumWidth(380);
        frame->setContentsMargins(0,0,0,0);

        // new layout to contain name and port boxes
        QHBoxLayout * nameAndPort = new QHBoxLayout;
        frame->setLayout(new QVBoxLayout);
        QVBoxLayout *frameLay = qobject_cast<QVBoxLayout *> (frame->layout());
        CHECK_CAST(frameLay)

        // output name
        QLineEdit * nameEdit = new QLineEdit;
        nameEdit->setText(this->name);
        nameEdit->setProperty("ptr", qVariantFromValue((void *) this));
        connect(nameEdit, SIGNAL(editingFinished()), handler, SLOT(setOutputName()));
        frameLay->addWidget(nameEdit);

        frameLay->addLayout(nameAndPort);
        frameLay->setContentsMargins(0,0,0,0);
        frameLay->setSpacing(0);
        nameAndPort->setContentsMargins(0,0,0,0);
        nameAndPort->setSpacing(2);

        // add Component LineEdit
        QLineEdit * lineEdit = new QLineEdit;
        if (set) {
            lineEdit->setText(this->source->getXMLName());
        } else {
            lineEdit->setText("Type component name");
        }
        QCompleter *completer = new QCompleter(elementList, this);
        completer->setCaseSensitivity(Qt::CaseInsensitive);
        lineEdit->setCompleter(completer);
        lineEdit->setMinimumWidth(200);
        lineEdit->setProperty("ptr", qVariantFromValue((void *) this));
        connect(lineEdit, SIGNAL(editingFinished()), handler, SLOT(setOutputComponent()));
        nameAndPort->addWidget(lineEdit);

        // add port dropdown
        QComboBox * portBox = new QComboBox;
        portBox->setMaximumWidth(90);
        if (set) {
            // if broken NOT NEEDED ANYMORE
            /*if (!data->isValidPointer(this->source)) {
                set = false;
                return this->drawOutput(data, handler);
            }*/
            // add
            portBox->addItem("AnalogPorts");
            int currentRow = 0;
            QModelIndex ind = portBox->model()->index(currentRow,0);
            portBox->model()->setData(ind, QVariant(0), Qt::UserRole-1);
            ++currentRow;
             for (int i = 0; i < this->source->component->AnalogPortList.size(); ++i) {
                if (this->source->component->AnalogPortList[i]->mode == AnalogSendPort) {
                    portBox->addItem(this->source->component->AnalogPortList[i]->name, qVariantFromValue((void *) this->source->component->AnalogPortList[i]));
                    if (portName.isEmpty()) {
                        portBox->setCurrentIndex(1);
                        this->portName = this->source->component->AnalogPortList[i]->name;
                        this->portIsAnalog = true;
                    } else {
                        if (this->portName == this->source->component->AnalogPortList[i]->name)
                           portBox->setCurrentIndex(currentRow);
                    }
                    ++currentRow;
                }
            }
            portBox->addItem("EventPorts");
            ind = portBox->model()->index(currentRow,0);
            portBox->model()->setData(ind, QVariant(0), Qt::UserRole-1);
            for (int i = 0; i < this->source->component->EventPortList.size(); ++i) {
                if (this->source->component->EventPortList[i]->mode == EventSendPort) {
                    portBox->addItem(this->source->component->EventPortList[i]->name, qVariantFromValue((void *) this->source->component->EventPortList[i]));
                    if (!(portBox->currentIndex() == 1)) {
                        if (portName.isEmpty()) {
                            portBox->setCurrentIndex(2);
                            this->portName = this->source->component->EventPortList[i]->name;
                            this->portIsAnalog = false;
                        } else {
                            if (this->portName == this->source->component->EventPortList[i]->name)
                               portBox->setCurrentIndex(currentRow+1);
                        }
                    }
                    ++currentRow;
                }
            }
        } else {
            portBox->setDisabled(true);
        }
        portBox->setProperty("ptr", qVariantFromValue((void *) this));
        nameAndPort->addWidget(portBox);
        connect(portBox, SIGNAL(currentIndexChanged(int)), handler, SLOT(setOutputPort(int)));

        // Indices
        if (set) {
            QHBoxLayout * indices = new QHBoxLayout();
            indices->addWidget(new QLabel("Indices:"));
            QLineEdit * indicesString = new QLineEdit();
            indicesString->setText(this->indices);
            indicesString->setMinimumWidth(200);
            indicesString->setProperty("ptr", qVariantFromValue((void *) this));
            indicesString->setToolTip("Indices to log - 'all' for all indices or comma separated list of indices (first index is index 0)");
            connect(indicesString, SIGNAL(editingFinished()), handler, SLOT(setOutputIndices()));
            indices->addWidget(indicesString);
            frameLay->addLayout(indices);

            // times...
            QHBoxLayout * times = new QHBoxLayout();
            times->addWidget(new QLabel("Start time (ms):"));
            QDoubleSpinBox * start_t = new QDoubleSpinBox;
            start_t->setMaximum(10000000);
            start_t->setMinimum(0);
            start_t->setValue(this->startTime);
            start_t->setProperty("ptr", qVariantFromValue((void *) this));
            start_t->setToolTip("Time for the logging to commence (ms)");
            connect(start_t, SIGNAL(valueChanged(double)), handler, SLOT(setOutputStartT(double)));
            times->addWidget(start_t);
            times->addWidget(new QLabel("End time (ms):"));
            QDoubleSpinBox * end_t = new QDoubleSpinBox;
            end_t->setMaximum(10000000);
            end_t->setMinimum(0);
            end_t->setValue(this->endTime);
            end_t->setProperty("ptr", qVariantFromValue((void *) this));
            end_t->setToolTip("Time for the logging to finish (ms)");
            connect(end_t, SIGNAL(valueChanged(double)), handler, SLOT(setOutputEndT(double)));
            times->addWidget(end_t);
            frameLay->addLayout(times);
        }

        // accept all
        QPushButton * accept = new QPushButton;
        accept->setMaximumWidth(32);
        accept->setFlat(true);
        accept->setIcon(QIcon(":/icons/toolbar/acptShad.png"));
        accept->setProperty("ptr", qVariantFromValue((void *) this));
        nameAndPort->addWidget(accept);
        if (!set)
            accept->setDisabled(true);
        QObject::connect(accept, SIGNAL(clicked()), handler, SLOT(acceptOutput()));

        // delete
        QPushButton * del = new QPushButton;
        del->setMaximumWidth(32);
        del->setFlat(true);
        del->setIcon(QIcon(":/icons/toolbar/delShad.png"));
        del->setProperty("ptr", qVariantFromValue((void *) this));
        nameAndPort->addWidget(del);
        QObject::connect(del, SIGNAL(clicked()), handler, SLOT(delOutput()));

        if (set) {

            // isExternalCheckbox
            QCheckBox * externToggle = new QCheckBox;
            externToggle->setChecked(isExternal);
            externToggle->setProperty("ptr", qVariantFromValue((void *) this));
            externToggle->setText("Output to external program");
            frameLay->addWidget(externToggle);
            QObject::connect(externToggle, SIGNAL(toggled(bool)), handler, SLOT(toggleExternalOutput(bool)));

            // external object
            QFormLayout * formlay = new QFormLayout;
            formlay->setMargin(0);
            frameLay->addLayout(formlay);

            // add items from params
            // command line
            QLineEdit * command = new QLineEdit;
            command->setProperty("ptr", qVariantFromValue((void *) this));
            command->setProperty("type", "command");
            command->setText(externalOutput.commandline);
            if (!isExternal)
                command->setEnabled(false);
            QObject ::connect(command, SIGNAL(editingFinished()), handler, SLOT(setOutputExternalData()));
            QObject ::connect(externToggle, SIGNAL(toggled(bool)), command, SLOT(setEnabled(bool)));
            formlay->addRow("Command:", command);

            // host
            QLineEdit * host = new QLineEdit;
            host->setProperty("ptr", qVariantFromValue((void *) this));
            host->setProperty("type", "host");
            host->setText(externalOutput.host);
            if (!isExternal)
                host->setEnabled(false);
            host->setToolTip("The host computer IP address, or 127.0.0.1 if the external output is on the same computer as the experiment");
            QObject ::connect(host, SIGNAL(editingFinished()), handler, SLOT(setOutputExternalData()));
            QObject ::connect(externToggle, SIGNAL(toggled(bool)), host, SLOT(setEnabled(bool)));
            formlay->addRow("Host IP:", host);

            // port
            QSpinBox * port = new QSpinBox;
            port->setProperty("ptr", qVariantFromValue((void *) this));
            port->setProperty("type", "port");
            port->setMinimum(49152);
            port->setMaximum(65535);
            port->setValue(externalOutput.port);
            if (!isExternal)
                port->setEnabled(false);
            QObject ::connect(port, SIGNAL(valueChanged(int)), handler, SLOT(setOutputExternalData()));
            QObject ::connect(externToggle, SIGNAL(toggled(bool)), port, SLOT(setEnabled(bool)));
            formlay->addRow("Port:", port);

            // host timestep
            QDoubleSpinBox * timestep = new QDoubleSpinBox;
            timestep->setProperty("ptr", qVariantFromValue((void *) this));
            timestep->setProperty("type", "timestep");
            timestep->setMinimum(0.0);
            timestep->setMaximum(1000);
            timestep->setToolTip("The timestep in ms of the external target (0.0 locks the timestep to the current experiment timestep)");
            timestep->setValue(externalOutput.timestep);
            if (!isExternal)
                timestep->setEnabled(false);
            QObject ::connect(timestep, SIGNAL(valueChanged(double)), handler, SLOT(setOutputExternalData()));
            QObject ::connect(externToggle, SIGNAL(toggled(bool)), timestep, SLOT(setEnabled(bool)));
            formlay->addRow("dt:", timestep);

        }



    } else {

        // check for badness NOT NEEDED ANYMORE
        /*
        if (!data->isValidPointer(this->source)) {
            qDebug() << "moo4";
            this->set = false;
            this->edit = true;
            return this->drawOutput(data, handler);
        }*/

        // new layout to contain name and port boxes
        QFrame * frame = new QFrame;
        frame->setStyleSheet("background-color :rgba(255,255,255,255)");
        layout->addWidget(frame);

        QGridLayout * descAndEdit = new QGridLayout;
        frame->setLayout(descAndEdit);

        // add name
        QLabel * name = new QLabel(this->name);
        name->setMaximumWidth(200);
        name->setWordWrap(true);
        descAndEdit->addWidget(name,0,0,1,1);
        QFont nameFont("Helvetica [Cronyx]", 12);
        name->setFont(nameFont);

        QString inds;
        if (indices == "all")
            inds = " logging all indices";
        else if (indices.size() > 10)
            inds = " logging selected indices";
        else
            inds = " logging indices " + indices;
        QLabel * desc = new QLabel();
        if (!isExternal)
            desc->setText("Output from component <b>" + this->source->getXMLName() + "</b> port <b>" + this->portName + "</b>" + inds);
        else
            desc->setText(QString("Output from component <b>") + this->source->getXMLName() + \
                          QString("</b> port <b>") + this->portName + QString("</b>") + inds + \
                          QString(" to external program at ") + this->externalOutput.host + \
                          QString(" on port ") + QString::number(this->externalOutput.port) + \
                    QString(" timestep ") + QString::number(externalOutput.timestep));
        desc->setMaximumWidth(200);
        desc->setWordWrap(true);
        descAndEdit->addWidget(desc, 1,0,1,1);
        QFont descFont("Helvetica [Cronyx]", 8);
        desc->setFont(descFont);

        // edit button
        QPushButton * edit = new QPushButton;
        edit->setMaximumWidth(32);
        edit->setFlat(true);
        edit->setIcon(QIcon(":/icons/toolbar/edit.png"));
        edit->setProperty("ptr", qVariantFromValue((void *) this));
        descAndEdit->addWidget(edit, 1,1,1,1);
        QObject::connect(edit, SIGNAL(clicked()), handler, SLOT(editOutput()));
    }

    return layout;
}

// ################# LESIONS and CHANGES TO PARAMETERS

QVBoxLayout * exptLesion::drawLesion(rootData * data, viewELExptPanelHandler *handler) {

    QVBoxLayout * layout = new QVBoxLayout;

    QStringList elementList;
    for (int i = 0; i < data->populations.size(); ++i) {
        for (int p = 0; p < data->populations[i]->projections.size(); ++p) {
            elementList << data->populations[i]->projections[p]->getName();
        }
    }

     if (edit) {
        // frame to add
        QFrame * frame = new QFrame;
        // setStyleSheet here if required.
        layout->addWidget(frame);
        frame->setMaximumWidth(380);
        frame->setContentsMargins(0,0,0,0);

        frame->setLayout(new QHBoxLayout);
        QHBoxLayout *frameLay = qobject_cast<QHBoxLayout *> (frame->layout());
        CHECK_CAST(frameLay)

        frameLay->setContentsMargins(0,0,0,0);
        frameLay->setSpacing(0);

        // add Component LineEdit
        QLineEdit * lineEdit = new QLineEdit;
        if (set) {
            // if broken NOT NEEDED ANYMORE
            /*if (!data->isValidPointer(this->proj)) {
                set = false;
                return this->drawLesion(data, handler);
            }*/
            lineEdit->setText(this->proj->getName());
        } else {
            lineEdit->setText("Type projection name");
        }
        QCompleter *completer = new QCompleter(elementList, this);
        completer->setCaseSensitivity(Qt::CaseInsensitive);
        lineEdit->setCompleter(completer);
        lineEdit->setMaximumWidth(220);
        lineEdit->setProperty("ptr", qVariantFromValue((void *) this));
        connect(lineEdit, SIGNAL(editingFinished()), handler, SLOT(setLesionProjection()));
        frameLay->addWidget(lineEdit);

        // accept all
        QPushButton * accept = new QPushButton;
        accept->setMaximumWidth(32);
        accept->setFlat(true);
        accept->setIcon(QIcon(":/icons/toolbar/acptShad.png"));
        accept->setProperty("ptr", qVariantFromValue((void *) this));
        frameLay->addWidget(accept);
        if (!set)
            accept->setDisabled(true);
        QObject::connect(accept, SIGNAL(clicked()), handler, SLOT(acceptLesion()));

        // delete
        QPushButton * del = new QPushButton;
        del->setMaximumWidth(32);
        del->setFlat(true);
        del->setIcon(QIcon(":/icons/toolbar/delShad.png"));
        del->setProperty("ptr", qVariantFromValue((void *) this));
        frameLay->addWidget(del);
        QObject::connect(del, SIGNAL(clicked()), handler, SLOT(delLesion()));


    } else {

         // check for badness NOT NEEDED ANYMORE
         /*
         if (!data->isValidPointer(this->proj)) {
             this->set = false;
             this->edit = true;
             return this->drawLesion(data, handler);
         }*/

        // new layout to contain name and port boxes
        QFrame * frame = new QFrame;
        frame->setStyleSheet("background-color :rgba(255,255,255,255)");
        layout->addWidget(frame);

        QHBoxLayout * descAndEdit = new QHBoxLayout;
        frame->setLayout(descAndEdit);

        QLabel * name = new QLabel("Lesion projection from <b>" + this->proj->getName() + "</b>");
        name->setMaximumWidth(200);
        name->setWordWrap(true);
        descAndEdit->addWidget(name);
        QFont descFont("Helvetica [Cronyx]", 8);
        name->setFont(descFont);

        // edit button
        QPushButton * edit = new QPushButton;
        edit->setMaximumWidth(32);
        edit->setFlat(true);
        edit->setIcon(QIcon(":/icons/toolbar/edit.png"));
        edit->setProperty("ptr", qVariantFromValue((void *) this));
        descAndEdit->addWidget(edit);
        QObject::connect(edit, SIGNAL(clicked()), handler, SLOT(editLesion()));
    }

    return layout;
}

QVBoxLayout * exptChangeProp::drawChangeProp(rootData * data, viewELExptPanelHandler *handler) {

    QVBoxLayout * layout = new QVBoxLayout;

    QStringList elementList;
    for (int i = 0; i < data->populations.size(); ++i) {
        elementList << data->populations[i]->neuronType->getXMLName();
        for (int j = 0; j < data->populations[i]->projections.size(); ++j) {
            for (int k = 0; k < data->populations[i]->projections[j]->synapses.size(); ++k) {
                elementList << data->populations[i]->projections[j]->synapses[k]->weightUpdateType->getXMLName();
                elementList << data->populations[i]->projections[j]->synapses[k]->postsynapseType->getXMLName();
            }
        }
    }

     if (edit) {
        // frame to add
        QFrame * frame = new QFrame;
        // Setting of stylesheet removed here.
        layout->addWidget(frame);
        frame->setMaximumWidth(380);
        frame->setContentsMargins(0,0,0,0);

        frame->setLayout(new QVBoxLayout);
        QVBoxLayout *frameLay = (QVBoxLayout *) frame->layout();
        frameLay->setContentsMargins(0,0,0,0);
        frameLay->setSpacing(0);

        // add an alias to the changed property
        QHBoxLayout * nameLay = new QHBoxLayout;
        QLineEdit * nameEdit = new QLineEdit(this->name);
        nameLay->addWidget(nameEdit);
        nameEdit->setProperty("ptr", qVariantFromValue((void *) this));
        connect(nameEdit, SIGNAL(editingFinished()), handler, SLOT(setChangeParName()));
        frameLay->addLayout(nameLay);

        QHBoxLayout * componentLay = new QHBoxLayout;
        QVBoxLayout * vbox = qobject_cast<QVBoxLayout *> (frame->layout());
        CHECK_CAST(vbox)
        vbox->addLayout(componentLay);

        // add Component LineEdit
        QLineEdit * lineEdit = new QLineEdit;
        if (set) {
            // if broken NOT NEEDED ANYMORE
            /*if (!data->isValidPointer(this->component)) {
                set = false;
                return this->drawChangeProp(data, handler);
            }*/
            lineEdit->setText(this->component->getXMLName());
        } else {
            lineEdit->setText("Type component name");
        }
        QCompleter *completer = new QCompleter(elementList, this);
        completer->setCaseSensitivity(Qt::CaseInsensitive);
        lineEdit->setCompleter(completer);
        lineEdit->setMaximumWidth(220);
        lineEdit->setProperty("ptr", qVariantFromValue((void *) this));
        connect(lineEdit, SIGNAL(editingFinished()), handler, SLOT(setChangeParComponent()));
        componentLay->addWidget(lineEdit);

        // add drop down list for par selection
        if (set) {

            QComboBox * parList = new QComboBox;

            int counter = 0;

            for (int i = 0; i < component->ParameterList.size(); ++i) {
                parList->addItem(component->ParameterList[i]->name, qVariantFromValue((void *) component->ParameterList[i]));
                if (this->par->name == component->ParameterList[i]->name) {
                    parList->setCurrentIndex(counter);
                }
                ++counter;
            }
            for (int i = 0; i < component->component->StateVariableList.size(); ++i) {
                parList->addItem(component->StateVariableList[i]->name, qVariantFromValue((void *) component->StateVariableList[i]));
                if (this->par->name == component->StateVariableList[i]->name) {
                    parList->setCurrentIndex(counter);
                }
                ++counter;
            }
            frameLay->addWidget(parList);
            parList->setProperty("ptr", qVariantFromValue((void *) this));
            connect(parList, SIGNAL(currentIndexChanged(QString)), handler, SLOT(setChangeProp(QString)));

        }

        // par options:
        if (set) {

            QFormLayout * parLay = new QFormLayout;
            frameLay->addLayout(parLay);

            // pointer to current parameter
            ParameterData * currPar = this->par;

            QString name = currPar->name;
            float value;

            switch (currPar->currType) {
            case Undefined:
                {QHBoxLayout * buttons = new QHBoxLayout;
                buttons->setSpacing(0);

                buttons->addStretch();

                // ADD BUTTON FOR FIXED VALUE
                QPushButton * currButton = new QPushButton("Fixed");
                currButton->setMaximumWidth(70);
                currButton->setMaximumHeight(28);
                currButton->setToolTip("A single value for all instances");
                currButton->setProperty("action","updateType");
                currButton->setProperty("newType","FixedValue");
                buttons->addWidget(currButton);

                currButton->setProperty("ptr", qVariantFromValue((void *) currPar));

                // add connection:
                connect(currButton, SIGNAL(clicked()), data, SLOT(updatePar()));
                connect(currButton, SIGNAL(clicked()), handler, SLOT(redraw()));

                // ADD BUTTON FOR STATISTICAL VALUE
                currButton = new QPushButton("Random");
                currButton->setMaximumWidth(70);
                currButton->setMaximumHeight(28);
                currButton->setToolTip("Values drawn from a statistical distribution");
                currButton->setProperty("action","updateType");
                currButton->setProperty("newType","Statistical");
                buttons->addWidget(currButton);

                currButton->setProperty("ptr", qVariantFromValue((void *) currPar));

                // add connection:
                connect(currButton, SIGNAL(clicked()), data, SLOT(updatePar()));
                connect(currButton, SIGNAL(clicked()), handler, SLOT(redraw()));

                // ADD BUTTON FOR EXPLICIT VALUES
                currButton = new QPushButton("Explicit");
                currButton->setMaximumWidth(70);
                currButton->setMaximumHeight(28);
                currButton->setToolTip("A complete list of values for each instance");
                currButton->setProperty("action","updateType");
                currButton->setProperty("newType","Explicit");
                buttons->addWidget(currButton);

                currButton->setProperty("ptr", qVariantFromValue((void *) currPar));

                // add connection:
                connect(currButton, SIGNAL(clicked()), data, SLOT(updatePar()));
                connect(currButton, SIGNAL(clicked()), handler, SLOT(redraw()));


                parLay->addRow(name, buttons);
                //parLay->itemAt(parLay->rowCount()-1,QFormLayout::LabelRole)->widget()->setProperty("type",type + parType);

                }
                break;

            case FixedValue:
                {
                QHBoxLayout * buttons = new QHBoxLayout;
                buttons->setSpacing(0);

                value = currPar->value[0];

                QDoubleSpinBox *parSpin = new QDoubleSpinBox;
                parSpin->setRange(-200000, 200000);
                parSpin->setSingleStep(0.1);
                parSpin->setDecimals(5);
                parSpin->setValue(value);
                parSpin->setSuffix(" " + currPar->dims->toString());
                parSpin->setProperty("valToChange", "0");
                parSpin->setProperty("action","changeVal");
                parSpin->setProperty("ptr", qVariantFromValue((void *) currPar));

                buttons->addWidget(parSpin);

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
                connect(goBack, SIGNAL(clicked()), handler, SLOT(redraw()));

                parLay->addRow(name, buttons);
                //parLay->itemAt(parLay->rowCount()-1,QFormLayout::LabelRole)->widget()->setProperty("type",type + parType);

                connect(parSpin, SIGNAL(editingFinished()), data, SLOT (updatePar()));
                //connect(parSpin, SIGNAL(valueChanged(double)), handler, SLOT (redraw(double)));

                }
                break;


            case Statistical:
                {
                QHBoxLayout * buttons = new QHBoxLayout;
                buttons->setSpacing(0);

                value = currPar->value[0];

                // add drop down to choose the distribution from
                QComboBox * distribution = new QComboBox;
                distribution->addItem(QIcon(":/icons/toolbar/delShad.png"),"None");
                distribution->addItem(QIcon(":/icons/toolbar/delShad.png"),"Uniform");
                distribution->addItem(QIcon(":/icons/toolbar/delShad.png"),"Normal");
                distribution->setMaximumWidth(45);
                distribution->setCurrentIndex(currPar->value[0]);
                distribution->setProperty("ptr", qVariantFromValue((void *) currPar));
                connect(distribution, SIGNAL(currentIndexChanged(int)), data, SLOT (updatePar(int)));
                connect(distribution, SIGNAL(currentIndexChanged(int)), handler, SLOT (redraw(int)));

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
                    QDoubleSpinBox *parSpin = new QDoubleSpinBox;
                    parSpin->setRange(-200000, 200000);
                    parSpin->setSingleStep(0.1);
                    parSpin->setMaximumWidth(60);
                    parSpin->setDecimals(5);
                    parSpin->setValue(currPar->value[1]);
                    parSpin->setProperty("valToChange", "1");
                    parSpin->setToolTip("lower bound value");
                    parSpin->setProperty("action","changeVal");
                    parSpin->setProperty("ptr", qVariantFromValue((void *) currPar));
                    connect(parSpin, SIGNAL(editingFinished()), data, SLOT (updatePar()));
                    //connect(parSpin, SIGNAL(valueChanged(double)), handler, SLOT (redraw(double)));
                    buttons->addWidget(parSpin);

                    buttons->addStretch();
                    buttons->addWidget(new QLabel("<"));

                    parSpin = new QDoubleSpinBox;
                    parSpin->setRange(-200000, 200000);
                    parSpin->setSingleStep(0.1);
                    parSpin->setMaximumWidth(60);
                    parSpin->setDecimals(5);
                    parSpin->setValue(currPar->value[2]);
                    parSpin->setProperty("valToChange", "2");
                    parSpin->setToolTip("upper bound value");
                    parSpin->setProperty("action","changeVal");
                    parSpin->setProperty("ptr", qVariantFromValue((void *) currPar));
                    connect(parSpin, SIGNAL(editingFinished()), data, SLOT (updatePar()));
                    //connect(parSpin, SIGNAL(valueChanged(double)), handler, SLOT (redraw(double)));
                    buttons->addWidget(parSpin);}
                    break;

                case 2:
                    // normal distribution - mean and SD spinboxes
                {
                    buttons->addStretch();
                    QLabel * label = new QLabel("x");
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
                    parSpin->setProperty("action","changeVal");
                    parSpin->setProperty("ptr", qVariantFromValue((void *) currPar));
                    connect(parSpin, SIGNAL(editingFinished()), data, SLOT (updatePar()));
                    //connect(parSpin, SIGNAL(valueChanged(double)), handler, SLOT (redraw(double)));
                    buttons->addWidget(parSpin);

                    buttons->addStretch();
                    label = new QLabel("s");
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
                    parSpin->setProperty("action","changeVal");
                    parSpin->setProperty("ptr", qVariantFromValue((void *) currPar));
                    connect(parSpin, SIGNAL(editingFinished()), data, SLOT (updatePar()));
                    //connect(parSpin, SIGNAL(valueChanged(double)), handler, SLOT (redraw(double)));
                    buttons->addWidget(parSpin);}
                    break;
                }

                // add seed box
                QLabel * label = new QLabel("seed");
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
                buttons->addWidget(seedSpin);

                buttons->addStretch();

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
                connect(goBack, SIGNAL(clicked()), handler, SLOT (redraw()));


                parLay->addRow(name, buttons);
                //parLay->itemAt(parLay->rowCount()-1,QFormLayout::LabelRole)->widget()->setProperty("type",type + parType);
                }
                break;

            case ExplicitList:
                {
                QHBoxLayout * buttons = new QHBoxLayout;
                buttons->setSpacing(0);
                buttons->addStretch();

                // ADD BUTTON FOR LIST VALUES
                QPushButton * currButton = new QPushButton("Edit");
                currButton->setMaximumWidth(70);
                currButton->setMaximumHeight(28);
                currButton->setToolTip("Edit the explicit value list");
                currButton->setProperty("action","editList");
                buttons->addWidget(currButton);

                currButton->setProperty("ptr", qVariantFromValue((void *) currPar));
                currButton->setProperty("ptrComp", qVariantFromValue((void *) component->component.data()));

                // add connection:
                connect(currButton, SIGNAL(clicked()), data, SLOT(updatePar()));
                connect(currButton, SIGNAL(clicked()), handler, SLOT (redraw()));

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
                connect(goBack, SIGNAL(clicked()), handler, SLOT (redraw()));


                parLay->addRow(name, buttons);

                //parLay->itemAt(parLay->rowCount()-1,QFormLayout::LabelRole)->widget()->setProperty("type",type + parType);
                }
                break;
            }
        }


        // accept all
        QPushButton * accept = new QPushButton;
        accept->setMaximumWidth(32);
        accept->setFlat(true);
        accept->setIcon(QIcon(":/icons/toolbar/acptShad.png"));
        accept->setProperty("ptr", qVariantFromValue((void *) this));
        componentLay->addWidget(accept);
        if (!set)
            accept->setDisabled(true);
        QObject::connect(accept, SIGNAL(clicked()), handler, SLOT(acceptChangedProp()));

        // delete
        QPushButton * del = new QPushButton;
        del->setMaximumWidth(32);
        del->setFlat(true);
        del->setIcon(QIcon(":/icons/toolbar/delShad.png"));
        del->setProperty("ptr", qVariantFromValue((void *) this));
        componentLay->addWidget(del);
        QObject::connect(del, SIGNAL(clicked()), handler, SLOT(delChangedProp()));


    } else {

         // check for badness NOT NEEDED ANYMORE
         /*if (!data->isValidPointer(this->component)) {
             this->set = false;
             this->edit = true;
             return this->drawChangeProp(data, handler);
         }*/

        // new layout to contain name and port boxes
        QFrame * frame = new QFrame;
        frame->setStyleSheet("background-color :rgba(255,255,255,255)");
        layout->addWidget(frame);

        QGridLayout * descAndEdit = new QGridLayout;
        frame->setLayout(descAndEdit);

        // add name
        QLabel * name = new QLabel(this->name);
        name->setMaximumWidth(200);
        name->setWordWrap(true);
        descAndEdit->addWidget(name,0,0,1,2);
        QFont nameFont("Helvetica [Cronyx]", 12);
        name->setFont(nameFont);

        QString labelText;
        labelText = "<b>" + this->name + "</b>\n" + "Property " + par->name + " of <b>" + this->component->getXMLName() + "</b> changed from ";

        for (int i = 0; i < component->ParameterList.size(); ++i) {
            if (par->name == component->ParameterList[i]->name) {
                ParameterData * origPar = component->ParameterList[i];
                switch (origPar->currType) {
                case Undefined:
                    labelText += "<b>undefined value</b> to ";
                    break;
                case FixedValue:
                    labelText += "<b>fixed value of " + QString::number(origPar->value[0]) + "</b> to ";
                    break;
                case Statistical:
                    switch (qRound(origPar->value[0])) {
                    case 0:
                        labelText += "<b>unselected random value" + QString::number(origPar->value[0]) + "</b> to ";
                        break;
                    case 1:
                        labelText += "<b>uniformly distributed random value between " + QString::number(origPar->value[1]) + " and " + QString::number(origPar->value[2]) + "</b> to ";
                        break;
                    case 2:
                        labelText += "<b>Gaussian distributed random value with mean " + QString::number(origPar->value[1]) + " and standard deviation " + QString::number(origPar->value[2]) + "</b> to ";
                        break;
                    }
                    break;
                case ExplicitList:
                    labelText += "<b>explicit values</b> to ";
                    break;
                }
            }
        }
        for (int i = 0; i < component->StateVariableList.size(); ++i) {
            if (par->name == component->StateVariableList[i]->name) {
                ParameterData * origPar = component->StateVariableList[i];
                switch (origPar->currType) {
                case Undefined:
                    labelText += "<b>undefined value</b> to ";
                    break;
                case FixedValue:
                    labelText += "<b>fixed value of " + QString::number(origPar->value[0]) + "</b> to ";
                    break;
                case Statistical:
                    switch (qRound(origPar->value[0])) {
                    case 0:
                        labelText += "<b>unselected random value" + QString::number(origPar->value[0]) + "</b> to ";
                        break;
                    case 1:
                        labelText += "<b>uniformly distributed random value between " + QString::number(origPar->value[1]) + " and " + QString::number(origPar->value[2]) + "</b> to ";
                        break;
                    case 2:
                        labelText += "<b>Gaussian distributed random value with mean " + QString::number(origPar->value[1]) + " and standard deviation " + QString::number(origPar->value[2]) + "</b> to ";
                        break;
                    }
                    break;
                case ExplicitList:
                    labelText += "<b>explicit values</b> to ";
                    break;
                }
            }
        }

        switch (par->currType) {
        case Undefined:
            labelText += "<b>undefined value</b> to ";
            break;
        case FixedValue:
            labelText += "<b>fixed value of " + QString::number(par->value[0]) + "</b>";
            break;
        case Statistical:
            switch (qRound(par->value[0])) {
            case 0:
                labelText += "<b>unselected random value" + QString::number(par->value[0]) + "</b>";
                break;
            case 1:
                labelText += "<b>uniformly distributed random value between " + QString::number(par->value[1]) + " and " + QString::number(par->value[2]) + "</b>";
                break;
            case 2:
                labelText += "<b>Gaussian distributed random value with mean " + QString::number(par->value[1]) + " and standard deviation " + QString::number(par->value[2]) + "</b>";
                break;
            }
            break;
        case ExplicitList:
            labelText += "<b>explicit values</b>";
            break;
        }

        QLabel * desc = new QLabel(labelText);
        desc->setMaximumWidth(200);
        desc->setWordWrap(true);
        descAndEdit->addWidget(desc,1,0,1,1);
        QFont descFont("Helvetica [Cronyx]", 8);
        desc->setFont(descFont);

        // edit button
        QPushButton * edit = new QPushButton;
        edit->setMaximumWidth(32);
        edit->setFlat(true);
        edit->setIcon(QIcon(":/icons/toolbar/edit.png"));
        edit->setProperty("ptr", qVariantFromValue((void *) this));
        descAndEdit->addWidget(edit,1,1,1,1);
        QObject::connect(edit, SIGNAL(clicked()), handler, SLOT(editLesion()));
    }

    return layout;
}

// #################### WRITE OUT XML:

void experiment::writeXML(QXmlStreamWriter * writer, projectObject *data) {

    // write out XML
    writer->writeStartDocument();
    writer->writeStartElement("SpineML");
    writer->writeAttribute("xmlns:UL", "http://www.shef.ac.uk/SpineMLNetworkLayer");
    writer->writeAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
    writer->writeAttribute("xmlns", "http://www.shef.ac.uk/SpineMLExperimentLayer");
    writer->writeAttribute("xsi:schemaLocation", "http://www.shef.ac.uk/SpineMLNetworkLayer SpineMLNetworkLayer.xsd http://www.shef.ac.uk/SpineMLExperimentLayer SpineMLExperimentLayer.xsd");

    writer->writeStartElement("Experiment");
    writer->writeAttribute("name", this->name);
    writer->writeAttribute("description", this->description);
    writer->writeStartElement("Model");
    writer->writeAttribute("network_layer_url", "model.xml");

    for (int i = 0; i < changes.size(); ++i) {
        changes[i]->writeXML(writer, data);
    }

    for (int i = 0; i < lesions.size(); ++i) {
        lesions[i]->writeXML(writer, data);
    }

    writer->writeEndElement(); // Model

    writer->writeStartElement("Simulation");
    writer->writeAttribute("duration", QString::number(this->setup.duration));
    writer->writeAttribute("preferred_simulator", this->setup.simType);
    switch (this->setup.solver) {
     case ForwardEuler:
        writer->writeEmptyElement("EulerIntegration");
        writer->writeAttribute("dt", QString::number(this->setup.dt));
        break;
    case RungeKutta:
        writer->writeEmptyElement("RungeKuttaIntegration");
        writer->writeAttribute("dt", QString::number(this->setup.dt));
        writer->writeAttribute("order",QString::number(float(this->setup.solverOrder)));
        break;
    }

    writer->writeEndElement(); // Simulation

    for (int i = 0; i < ins.size(); ++i) {
        ins[i]->writeXML(writer, data);
    }

    for (int i = 0; i < outs.size(); ++i) {
        outs[i]->writeXML(writer, data);
    }

    writer->writeEndElement(); // Experiment
    writer->writeEndElement(); // NineML
    writer->writeEndDocument();

}

void exptInput::writeXML(QXmlStreamWriter * writer, projectObject * data) {

    if (!data->isValidPointer(target))
        return;

    if (!set || edit)
        return;

    switch (this->inType) {
    case constant:
        writer->writeEmptyElement("ConstantInput");
        writer->writeAttribute("target", this->target->getXMLName());
        writer->writeAttribute("port", this->portName);
        writer->writeAttribute("value", QString::number(float(this->params[0])));
        writer->writeAttribute("name", this->name);
        if (!this->portIsAnalog) {
            if (this->rateDistribution == Regular)
                writer->writeAttribute("rate_based_input", "regular");
            else if (this->rateDistribution == Poisson)
                writer->writeAttribute("rate_based_input", "poisson");
        }
        break;
    case timevarying:
        writer->writeStartElement("TimeVaryingInput");
        writer->writeAttribute("target", this->target->getXMLName());
        writer->writeAttribute("port", this->portName);
        writer->writeAttribute("name", this->name);
        if (!this->portIsAnalog) {
            if (this->rateDistribution == Regular)
                writer->writeAttribute("rate_based_input", "regular");
            else if (this->rateDistribution == Poisson)
                writer->writeAttribute("rate_based_input", "poisson");
        }
        for (int i = 0; i < this->params.size(); i+=2) {
            writer->writeEmptyElement("TimePointValue");
            writer->writeAttribute("time", QString::number(float(this->params[i])));
            writer->writeAttribute("value", QString::number(float(this->params[i+1])));
        }
        writer->writeEndElement(); // TimeVaryingInput
        break;
    case arrayConstant:
    {
        writer->writeEmptyElement("ConstantArrayInput");
        writer->writeAttribute("target", this->target->getXMLName());
        writer->writeAttribute("port", this->portName);
        // construct string for array_value:
        QString array = "";
        for (int i = 0; i < params.size(); ++i)
            array += QString::number(params[i]) + ",";
        array.chop(1);
        writer->writeAttribute("array_size",QString::number(float(params.size())));
        writer->writeAttribute("array_value", array);
        writer->writeAttribute("name", this->name);
        if (!this->portIsAnalog) {
            if (this->rateDistribution == Regular)
                writer->writeAttribute("rate_based_input", "regular");
            else if (this->rateDistribution == Poisson)
                writer->writeAttribute("rate_based_input", "poisson");
        }
    }
        break;
    case arrayTimevarying:
    {
        writer->writeStartElement("TimeVaryingArrayInput");
        writer->writeAttribute("target", this->target->getXMLName());
        writer->writeAttribute("port", this->portName);
        writer->writeAttribute("name", this->name);
        if (!this->portIsAnalog) {
            if (this->rateDistribution == Regular)
                writer->writeAttribute("rate_based_input", "regular");
            else if (this->rateDistribution == Poisson)
                writer->writeAttribute("rate_based_input", "poisson");
        }
        int index = -1;
        QString arrayT = "";
        QString arrayV = "";
        for (int n = 0; n < params.size(); n+=2) {
            if (params[n] == -1 || n+1 == params.size()-1) {
                if (index != -1) {
                    if (n+1 == params.size()-1) {
                        // construct string for arrays:
                        arrayT += QString::number(params[n]) + ",";
                        arrayV += QString::number(params[n+1]) + ",";
                    }
                    arrayT.chop(1);
                    arrayV.chop(1);
                    if (arrayT.size() > 0) {
                        writer->writeEmptyElement("TimePointArrayValue");
                        writer->writeAttribute("index", QString::number(float(index)));
                        writer->writeAttribute("array_time", arrayT);
                        writer->writeAttribute("array_value", arrayV);
                    }
                }
                if (params[n] == -1) {
                    arrayV = "";
                    arrayT = "";
                    index = params[n+1];
                }
            }
            else
            {
                // construct string for arrays:
                arrayT += QString::number(params[n]) + ",";
                arrayV += QString::number(params[n+1]) + ",";
            }
        }
        writer->writeEndElement(); // TimeVaryingArrayInput
    }
        break;
    case external:
    {
        writer->writeEmptyElement("ExternalInput");
        writer->writeAttribute("target", this->target->getXMLName());
        writer->writeAttribute("port", this->portName);
        writer->writeAttribute("name", this->name);
        // construct string for array_value:
        QString array = "";
        for (int i = 0; i < params.size(); ++i)
            array += QString::number(params[i]) + ",";
        array.chop(1);
        writer->writeAttribute("tcp_port",QString::number(this->externalInput.port));
        if (this->target->owner->type == populationObject) {
            QSharedPointer <population> pop = qSharedPointerDynamicCast<population> (this->target->owner);
            CHECK_CAST(pop)
            this->externalInput.size = pop->numNeurons;
        }
        writer->writeAttribute("size",QString::number(this->externalInput.size));
        writer->writeAttribute("command", this->externalInput.commandline);
        // lookup the host name if it is not an IP
        // regular expression for an IP address
        QRegExp rx("^(?:[0-9]{1,3}[.]){3}[0-9]{1,3}$");
        if (rx.exactMatch(this->externalInput.host)) {
            writer->writeAttribute("host", this->externalInput.host);
        } else {
            QHostInfo ip = QHostInfo::fromName(this->externalInput.host);
            qDebug() << "Hostname = " << ip.hostName();
            if (ip.addresses().size() > 2) {
                writer->writeAttribute("host", ip.addresses()[1].toString());
            }
        }
        writer->writeAttribute("timestep", QString::number(this->externalInput.timestep));
    }
        break;
    case spikeList:

        break;
    }


}

void exptOutput::writeXML(QXmlStreamWriter * writer, projectObject * data) {

    if (!data->isValidPointer(source))
        return;

    if (!set || edit)
        return;

    // check indices
    if (indices != "all") {
        QStringList inds = indices.split(",");
        for (int i = 0; i < inds.size(); ++i) {
            if (source->component->type == "neuron_body") {
                QSharedPointer <population> pop = qSharedPointerDynamicCast<population> (source->owner);
                CHECK_CAST(pop)
                if (inds[i].toInt() < 0 || inds[i].toInt() > pop->numNeurons-1) {
                    QMessageBox msgBox;
                    msgBox.setIcon(QMessageBox::Critical);
                    msgBox.setText("Output index out of range - indices must be between 0 and the number of neurons - 1. Output will not be logged.");
                    msgBox.exec();
                    return;
                }
            }
            if (source->component->type == "postsynapse") {
                QSharedPointer <projection> proj = qSharedPointerDynamicCast<projection> (source->owner);
                CHECK_CAST(proj)
                QSharedPointer <population> pop = proj->destination;
                if (inds[i].toInt() < 0 || inds[i].toInt() > pop->numNeurons-1) {
                    QMessageBox msgBox;
                    msgBox.setIcon(QMessageBox::Warning);
                    msgBox.setText("Output index out of range - indices must be between 0 and the number of target neurons - 1. Output will not be logged.");
                    msgBox.exec();
                    return;
                }
            }
        }
    }

    writer->writeEmptyElement("LogOutput");
    writer->writeAttribute("name", this->name);
    writer->writeAttribute("target", this->source->getXMLName());
    writer->writeAttribute("port", this->portName);
    writer->writeAttribute("start_time", QString::number(this->startTime)); // add later
    writer->writeAttribute("end_time", QString::number(this->endTime)); // add later


    if (indices != "all") {
        writer->writeAttribute("indices",indices);
    }
    if (isExternal) {
        writer->writeAttribute("tcp_port",QString::number(this->externalOutput.port));
        writer->writeAttribute("size",QString::number(this->externalOutput.size));
        writer->writeAttribute("command", this->externalOutput.commandline);
        writer->writeAttribute("host", this->externalOutput.host);
        writer->writeAttribute("timestep", QString::number(this->externalOutput.timestep));
    }

}

void exptLesion::writeXML(QXmlStreamWriter * xmlOut, projectObject*)
{
    if (!set || edit) {
        return;
    }

    xmlOut->writeEmptyElement("Lesion");
    xmlOut->writeAttribute("src_population",this->proj->source->neuronType->getXMLName());
    xmlOut->writeAttribute("dst_population",this->proj->destination->neuronType->getXMLName());
}

void exptChangeProp::writeXML(QXmlStreamWriter * xmlOut, projectObject * data) {

    if (!data->isValidPointer(component))
        return;

    if (!set || edit)
        return;

    xmlOut->writeStartElement("Configuration");
    xmlOut->writeAttribute("target", this->component->getXMLName());
    xmlOut->writeStartElement("UL:Property");

    xmlOut->writeAttribute("alias", this->name);
    xmlOut->writeAttribute("name",this->par->name);
    xmlOut->writeAttribute("dimension", this->par->dims->toString());

      if (this->par->currType == FixedValue) {
          xmlOut->writeEmptyElement("UL:FixedValue");
          xmlOut->writeAttribute("value", QString::number(this->par->value[0]));
      }
      if (this->par->currType == Statistical) {

          switch (int(round(this->par->value[0]))) {
          case 0:
              break;
          case 1:
          {
              xmlOut->writeEmptyElement("UL:UniformDistribution");
              xmlOut->writeAttribute("minimum", QString::number(this->par->value[1]));
              xmlOut->writeAttribute("maximum", QString::number(this->par->value[2]));
              xmlOut->writeAttribute("seed", QString::number(this->par->value[3]));
          }
              break;
          case 2:
          {
              xmlOut->writeEmptyElement("UL:NormalDistribution");
              xmlOut->writeAttribute("mean", QString::number(this->par->value[1]));
              xmlOut->writeAttribute("variance", QString::number(this->par->value[2]));
              xmlOut->writeAttribute("seed", QString::number(this->par->value[3]));
           }
              break;
          }

      }
      if (this->par->currType == ExplicitList) {
          xmlOut->writeStartElement("UL:ValueList");
          for (int ind = 0; ind < this->par->value.size(); ++ind) {
              xmlOut->writeEmptyElement("UL:Value");
              xmlOut->writeAttribute("index", QString::number(float(this->par->indices[ind])));
              xmlOut->writeAttribute("value", QString::number(float(this->par->value[ind])));
          }
         xmlOut->writeEndElement(); // valueList
      }

      xmlOut->writeEndElement(); // Property
      xmlOut->writeEndElement(); // Configuration


}

// ###################### READ IN XML:

QSharedPointer <NineMLComponentData> getTargetFromData(QString TargetName, projectObject * data) {

    // find Synapse in model
    for (int i = 0; i < data->network.size(); ++i) {
        if (data->network[i]->neuronType->getXMLName() == TargetName) {
            return data->network[i]->neuronType;
            break;
        }
        for (int j = 0; j < data->network[i]->projections.size(); ++j) {
            for (int k = 0; k < data->network[i]->projections[j]->synapses.size(); ++k) {
                if (data->network[i]->projections[j]->synapses[k]->weightUpdateType->getXMLName() == TargetName) {
                    return data->network[i]->projections[j]->synapses[k]->weightUpdateType;
                    break;
                }
                if (data->network[i]->projections[j]->synapses[k]->postsynapseType->getXMLName() == TargetName) {
                    return data->network[i]->projections[j]->synapses[k]->postsynapseType;
                    break;
                }
            }
        }
    }
    QSharedPointer<NineMLComponentData> null;

    return null;
}

Port * findPortInComponent(QString portName, QSharedPointer <NineMLComponentData> target) {

    // find port in Synapse:
    for (int i = 0; i < target->component->AnalogPortList.size(); ++i) {
        if (target->component->AnalogPortList[i]->name == portName && \
                (target->component->AnalogPortList[i]->mode == AnalogRecvPort || \
                 target->component->AnalogPortList[i]->mode == AnalogReducePort)) {
            return target->component->AnalogPortList[i];
        }
    }
    for (int i = 0; i < target->component->EventPortList.size(); ++i) {
        if (target->component->EventPortList[i]->name == portName && \
                target->component->EventPortList[i]->mode == EventRecvPort) {
            return target->component->EventPortList[i];
        }
    }
    return NULL;
}

Port * findOutputPortInComponent(QString portName, QSharedPointer <NineMLComponentData> Synapse) {

    // find port in Synapse:
    for (int i = 0; i < Synapse->component->AnalogPortList.size(); ++i) {
        if (Synapse->component->AnalogPortList[i]->name == portName && \
                Synapse->component->AnalogPortList[i]->mode == AnalogSendPort) {
            return Synapse->component->AnalogPortList[i];
        }
    }
    for (int i = 0; i < Synapse->component->EventPortList.size(); ++i) {
        if (Synapse->component->EventPortList[i]->name == portName && \
                Synapse->component->EventPortList[i]->mode == EventSendPort) {
            return Synapse->component->EventPortList[i];
        }
    }
    return NULL;

}

void experiment::readXML(QXmlStreamReader * reader, projectObject * data) {

    while (reader->readNextStartElement()) {

        if (reader->name() == "SpineML") {
            while(reader->readNextStartElement()) {
                if (reader->name() == "Experiment") {
                    // get name and description
                    if (reader->attributes().hasAttribute("name"))
                        this->name = reader->attributes().value("name").toString();
                    if (reader->attributes().hasAttribute("description"))
                        this->description = reader->attributes().value("description").toString();

                    // fetch experiment
                    while (reader->readNextStartElement()) {
                        //cerr << reader->name().toString().toStdString() << " ##EXPT#####\n";

                        if (reader->name() == "Model") {
                            while (reader->readNextStartElement()) {

                                //cerr << reader->name().toString().toStdString() << " ##MODEL#####\n";

                                if (reader->name() == "Configuration") {

                                    QSharedPointer <NineMLComponentData> component;
                                    QString SynapseName;
                                    if (reader->attributes().hasAttribute("target"))
                                        SynapseName = reader->attributes().value("target").toString();
                                    else
                                        {
                                        QSettings settings;
                                        int num_errs = settings.beginReadArray("errors");
                                        settings.endArray();
                                        settings.beginWriteArray("errors");
                                            settings.setArrayIndex(num_errs + 1);
                                            settings.setValue("errorText", "Error in Experiment '" + this->name + "'' file - Target field missing");
                                        settings.endArray();
                                        } // ERROR - no target

                                    component = getTargetFromData(SynapseName, data);

                                    if (component.isNull())
                                    {
                                        QSettings settings;
                                        int num_errs = settings.beginReadArray("errors");
                                        settings.endArray();
                                        settings.beginWriteArray("errors");
                                            settings.setArrayIndex(num_errs + 1);
                                            settings.setValue("errorText", "Error in Experiment '" + this->name + "'' - Experiment references missing target");
                                        settings.endArray();
                                    }

                                    while(reader->readNextStartElement()) {

                                        //cerr << reader->name().toString().toStdString() << " ##PROP#####\n";

                                        if (reader->name() == "Property") {
                                            exptChangeProp * chge = new exptChangeProp;
                                            chge->component = component;
                                            chge->readXML(reader);
                                            this->changes.push_back(chge);
                                        } else {
                                            reader->skipCurrentElement();
                                        }

                                    }
                                    //cerr << reader->name().toString().toStdString() << " ##AFTERPROP#####\n";
                                }

                                else if (reader->name() == "Lesion") {
                                    exptLesion * lesion = new exptLesion;
                                    lesion->readXML(reader, data);
                                    this->lesions.push_back(lesion);
                                    reader->readNextStartElement();
                                } else {
                                    reader->skipCurrentElement();
                                }

                            }
                            //cerr << reader->name().toString().toStdString() << " ##AFTERMODEL#####\n";
                        } else if (reader->name() == "Simulation") {

                            if (reader->attributes().hasAttribute("duration"))
                                this->setup.duration = reader->attributes().value("duration").toString().toFloat();
                            else
                                this->setup.duration = 1.0;
                                // ERROR - no duration
                            if (reader->attributes().hasAttribute("preferred_simulator"))
                                this->setup.simType = reader->attributes().value("preferred_simulator").toString();
                            else
                                this->setup.simType = "BRAHMS";

                            while (reader->readNextStartElement()) {

                                //cerr << reader->name().toString().toStdString() << " ##SOLVER#####\n";

                                if (reader->name() == "EulerIntegration") {
                                    this->setup.solver = ForwardEuler;
                                    if (reader->attributes().hasAttribute("dt"))
                                        this->setup.dt = reader->attributes().value("dt").toString().toFloat();
                                    else
                                        this->setup.dt = 0.1;
                                        // ERROR - no dt
                                    reader->skipCurrentElement();
                                }
                                else if (reader->name() == "RungeKuttaIntegration") {
                                    this->setup.solver = RungeKutta;
                                    if (reader->attributes().hasAttribute("dt"))
                                        this->setup.dt = reader->attributes().value("dt").toString().toFloat();
                                    else
                                        this->setup.dt = 0.1;
                                        // ERROR - no dt
                                    if (reader->attributes().hasAttribute("order"))
                                        this->setup.solverOrder = reader->attributes().value("order").toString().toFloat();
                                    else
                                        this->setup.solverOrder = 2;
                                        // ERROR - no order
                                    reader->skipCurrentElement();
                                } else {
                                    reader->skipCurrentElement();
                                    this->setup.solver = ForwardEuler;
                                    this->setup.dt = 0.1;
                                    // ERROR - unknown integration type
                                }

                            }

                        } else if (reader->name() == "ConstantInput") {
                            exptInput * newIn = new exptInput;
                            newIn->readXML(reader, data);
                            this->ins.push_back(newIn);
                            reader->skipCurrentElement();
                        } else if (reader->name() == "TimeVaryingInput") {
                            exptInput * newIn = new exptInput;
                            newIn->readXML(reader, data);
                            this->ins.push_back(newIn);
                        } else if (reader->name() == "ConstantArrayInput") {
                            exptInput * newIn = new exptInput;
                            newIn->readXML(reader, data);
                            this->ins.push_back(newIn);
                            reader->skipCurrentElement();
                        } else if (reader->name() == "TimeVaryingArrayInput") {
                            exptInput * newIn = new exptInput;
                            newIn->readXML(reader, data);
                            this->ins.push_back(newIn);
                        } else if (reader->name() == "ExternalInput") {
                            exptInput * newIn = new exptInput;
                            newIn->readXML(reader, data);
                            this->ins.push_back(newIn);
                            reader->skipCurrentElement();
                        } else if (reader->name() == "SpikeList") {
                            exptInput * newIn = new exptInput;
                            newIn->readXML(reader, data);
                            this->ins.push_back(newIn);
                            reader->skipCurrentElement();
                        } else if (reader->name() == "LogOutput") {
                            exptOutput * newOut = new exptOutput;
                            newOut->readXML(reader, data);
                            this->outs.push_back(newOut);
                            reader->skipCurrentElement();
                        } else {
                            reader->skipCurrentElement();
                        }
                    }
                }
                else
                {
                    {
                        QSettings settings;
                        int num_errs = settings.beginReadArray("errors");
                        settings.endArray();
                        settings.beginWriteArray("errors");
                            settings.setArrayIndex(num_errs + 1);
                            settings.setValue("errorText", "Error in Experiment '" + this->name + "'' - Experiment file badly malformed");
                        settings.endArray();
                    }
                }
            }
        }
        else
        {
            {
                QSettings settings;
                int num_errs = settings.beginReadArray("errors");
                settings.endArray();
                settings.beginWriteArray("errors");
                    settings.setArrayIndex(num_errs + 1);
                    settings.setValue("errorText", "Error in Experiment '" + this->name + "'' - Experiment file badly malformed");
                settings.endArray();
            }
        }

    }
    this->editing = false;

}

void exptInput::readXML(QXmlStreamReader * reader, projectObject * data) {

    // get name
    if (reader->attributes().hasAttribute("name"))
        this->name = reader->attributes().value("name").toString();
    else
    {
        QSettings settings;
        int num_errs = settings.beginReadArray("errors");
        settings.endArray();
        settings.beginWriteArray("errors");
            settings.setArrayIndex(num_errs + 1);
            settings.setValue("errorText", "XML error in Experiment Input - 'name' attribute missing");
        settings.endArray();
    }

    // get rate distribution if there
    QString rateDistributionString;
    if (reader->attributes().hasAttribute("rate_based_input")) {
        rateDistributionString = reader->attributes().value("rate_based_input").toString();
        if (rateDistributionString == "regular")
            this->rateDistribution = Regular;
        else if (rateDistributionString == "poisson")
            this->rateDistribution = Poisson;
    }

    // get Synapse name
    QString TargetName;
    if (reader->attributes().hasAttribute("target"))
        TargetName = reader->attributes().value("target").toString();
    else
    {
        QSettings settings;
        int num_errs = settings.beginReadArray("errors");
        settings.endArray();
        settings.beginWriteArray("errors");
            settings.setArrayIndex(num_errs + 1);
            settings.setValue("errorText", "XML error in Experiment Input - 'target' attribute missing");
        settings.endArray();
    }

    // find Synapse in model
    target = getTargetFromData(TargetName, data);

    // handle if not found
    if (target == NULL)
    {
        QSettings settings;
        int num_errs = settings.beginReadArray("errors");
        settings.endArray();
        settings.beginWriteArray("errors");
            settings.setArrayIndex(num_errs + 1);
            settings.setValue("errorText", "Error in Experiment Input - references missing target " + TargetName);
        settings.endArray();
    }

    // get port name
    if (reader->attributes().hasAttribute("port"))
        portName = reader->attributes().value("port").toString();
    else
    {
        QSettings settings;
        int num_errs = settings.beginReadArray("errors");
        settings.endArray();
        settings.beginWriteArray("errors");
            settings.setArrayIndex(num_errs + 1);
            settings.setValue("errorText", "XML error in Experiment Input - 'port' attribute missing");
        settings.endArray();
    }

    // find port in Synapse
    if (target != NULL) {

        Port * port = NULL;

        port = findPortInComponent(portName, this->target);

        // if spike source
        if (this->target->owner->type == populationObject) {
            QSharedPointer <population> pop = qSharedPointerDynamicCast<population> (this->target->owner);
            if (pop->isSpikeSource) {
                // point to dummy event port
                port = &(this->eventport);
            }
        }

        // handle if not found
        if (port == NULL)
        {
            QSettings settings;
            int num_errs = settings.beginReadArray("errors");
            settings.endArray();
            settings.beginWriteArray("errors");
                settings.setArrayIndex(num_errs + 1);
                settings.setValue("errorText", "XML error in Experiment Input - references missing port " + portName);
            settings.endArray();
        } else {
            portName = port->name;
            portIsAnalog = port->isAnalog();
        }
    }

    if (reader->name() == "ConstantInput") {

        this->inType = constant;

        // get value
        params.clear();
        if (reader->attributes().hasAttribute("value"))
            this->params.push_back(reader->attributes().value("value").toString().toFloat());
        else
        {
            QSettings settings;
            int num_errs = settings.beginReadArray("errors");
            settings.endArray();
            settings.beginWriteArray("errors");
                settings.setArrayIndex(num_errs + 1);
                settings.setValue("errorText", "XML error in Experiment Input - 'value' attribute missing");
            settings.endArray();
        }

    } else if (reader->name() == "TimeVaryingInput") {

        this->inType = timevarying;
        params.clear();

        while (reader->readNextStartElement()) {

            if (reader->name() == "TimePointValue") {

                // get time
                if (reader->attributes().hasAttribute("time"))
                {this->params.push_back(reader->attributes().value("time").toString().toFloat());}
                else
                {
                    QSettings settings;
                    int num_errs = settings.beginReadArray("errors");
                    settings.endArray();
                    settings.beginWriteArray("errors");
                        settings.setArrayIndex(num_errs + 1);
                        settings.setValue("errorText", "XML error in Experiment Input - 'time' attribute missing");
                    settings.endArray();
                }

                // get value
                if (reader->attributes().hasAttribute("value"))
                {this->params.push_back(reader->attributes().value("value").toString().toFloat());}
                else
                {
                    QSettings settings;
                    int num_errs = settings.beginReadArray("errors");
                    settings.endArray();
                    settings.beginWriteArray("errors");
                        settings.setArrayIndex(num_errs + 1);
                        settings.setValue("errorText", "XML error in Experiment Input - 'value' attribute missing");
                    settings.endArray();
                }

                reader->readNextStartElement();
            }

        }

    } else if (reader->name() == "ConstantArrayInput") {

        this->inType = arrayConstant;

        // get array size
        params.clear();

        int array_size;
        if (reader->attributes().hasAttribute("array_size"))
            array_size = reader->attributes().value("array_size").toString().toInt();
        else
        {
            QSettings settings;
            int num_errs = settings.beginReadArray("errors");
            settings.endArray();
            settings.beginWriteArray("errors");
                settings.setArrayIndex(num_errs + 1);
                settings.setValue("errorText", "XML error in Experiment Input - 'array_size' attribute missing");
            settings.endArray();
        }

        QString array;
        if (reader->attributes().hasAttribute("array_value"))
            array = reader->attributes().value("array_value").toString();
        else
        {
            QSettings settings;
            int num_errs = settings.beginReadArray("errors");
            settings.endArray();
            settings.beginWriteArray("errors");
                settings.setArrayIndex(num_errs + 1);
                settings.setValue("errorText", "XML error in Experiment Input -  missing array_value tag");
            settings.endArray();
        }

        QStringList arrayValues = array.split(",");

        for (int i = 0; i < (int) arrayValues.size(); ++i) {
            params.push_back(arrayValues[i].toFloat());
        }

        if ((int) params.size() != array_size)
        {
            QSettings settings;
            int num_errs = settings.beginReadArray("errors");
            settings.endArray();
            settings.beginWriteArray("errors");
                settings.setArrayIndex(num_errs + 1);
                settings.setValue("errorText", "Error in Experiment Input -  time and value arrays different sizes");
            settings.endArray();
        }

    } else if (reader->name() == "TimeVaryingArrayInput") {

        this->inType = arrayTimevarying;

        params.clear();

        while (reader->readNextStartElement()) {

            if (reader->name() == "TimePointArrayValue") {

                // get index
                if (reader->attributes().hasAttribute("index"))
                {this->params.push_back(-1);
                    this->params.push_back(reader->attributes().value("index").toString().toFloat());}
                else
                {
                    QSettings settings;
                    int num_errs = settings.beginReadArray("errors");
                    settings.endArray();
                    settings.beginWriteArray("errors");
                        settings.setArrayIndex(num_errs + 1);
                        settings.setValue("errorText", "XML error in Experiment Input - 'index' attribute missing");
                    settings.endArray();
                }

                // get array_time
                QString array_time_string;
                if (reader->attributes().hasAttribute("array_time"))
                    array_time_string = reader->attributes().value("array_time").toString();
                else
                {
                    QSettings settings;
                    int num_errs = settings.beginReadArray("errors");
                    settings.endArray();
                    settings.beginWriteArray("errors");
                        settings.setArrayIndex(num_errs + 1);
                        settings.setValue("errorText", "XML error in Experiment Input - 'array_time' attribute missing");
                    settings.endArray();
                }



                // get array_value
                QString array_value_string;
                if (reader->attributes().hasAttribute("array_value"))
                    array_value_string = reader->attributes().value("array_value").toString();
                else
                {
                    QSettings settings;
                    int num_errs = settings.beginReadArray("errors");
                    settings.endArray();
                    settings.beginWriteArray("errors");
                        settings.setArrayIndex(num_errs + 1);
                        settings.setValue("errorText", "XML error in Experiment Input - 'array_value' attribute missing");
                    settings.endArray();
                }

                // unpack

                QStringList arrayTimes = array_time_string.split(",");
                QStringList arrayValues = array_value_string.split(",");

                for (int i = 0; i < arrayValues.size(); ++i) {
                    params.push_back(arrayTimes[i].toFloat());
                    params.push_back(arrayValues[i].toFloat());
                }


                reader->skipCurrentElement();
            }

        }

    } else if (reader->name() == "ExternalInput") {

        this->inType = external;

        params.clear();

        if (reader->attributes().hasAttribute("tcp_port"))
            externalInput.port = reader->attributes().value("tcp_port").toString().toInt();
        else
        {
            QSettings settings;
            int num_errs = settings.beginReadArray("errors");
            settings.endArray();
            settings.beginWriteArray("errors");
                settings.setArrayIndex(num_errs + 1);
                settings.setValue("errorText", "XML error in Experiment Input - 'tcp_port' attribute missing");
            settings.endArray();
        }

        // not required
        if (reader->attributes().hasAttribute("host")) {
            externalInput.host = reader->attributes().value("host").toString();
        }

        // not required
        if (reader->attributes().hasAttribute("timestep")) {
            externalInput.timestep = reader->attributes().value("timestep").toString().toDouble();
        }

        if (reader->attributes().hasAttribute("size"))
            externalInput.size = reader->attributes().value("size").toString().toInt();
        else
        {
            QSettings settings;
            int num_errs = settings.beginReadArray("errors");
            settings.endArray();
            settings.beginWriteArray("errors");
                settings.setArrayIndex(num_errs + 1);
                settings.setValue("errorText", "XML error in Experiment Input - 'size' attribute missing");
            settings.endArray();
        }

        if (reader->attributes().hasAttribute("command"))
            externalInput.commandline = reader->attributes().value("command").toString();
        else
        {
            QSettings settings;
            int num_errs = settings.beginReadArray("errors");
            settings.endArray();
            settings.beginWriteArray("errors");
                settings.setArrayIndex(num_errs + 1);
                settings.setValue("errorText", "XML error in Experiment Input - 'command' attribute missing");
            settings.endArray();
        }


    } else if (reader->name() == "SpikeList") {

    }
    this->edit = false;
    this->set = true;
}

void exptOutput::readXML(QXmlStreamReader * reader, projectObject * data) {

    // get output name
    if (reader->attributes().hasAttribute("name"))
        this->name = reader->attributes().value("name").toString();
    else
    {
        QSettings settings;
        int num_errs = settings.beginReadArray("errors");
        settings.endArray();
        settings.beginWriteArray("errors");
            settings.setArrayIndex(num_errs + 1);
            settings.setValue("errorText", "Error in Experiment Input -  missing name tag");
        settings.endArray();
    }

    // get Synapse name
    QString SynapseName;
    if (reader->attributes().hasAttribute("target"))
        SynapseName = reader->attributes().value("target").toString();
    else
    {
        QSettings settings;
        int num_errs = settings.beginReadArray("errors");
        settings.endArray();
        settings.beginWriteArray("errors");
            settings.setArrayIndex(num_errs + 1);
            settings.setValue("errorText", "Error in Experiment Output -  missing target tag");
        settings.endArray();
    }

    // find Synapse in model
    source = getTargetFromData(SynapseName, data);

    if (source == NULL)
    {
        QSettings settings;
        int num_errs = settings.beginReadArray("errors");
        settings.endArray();
        settings.beginWriteArray("errors");
            settings.setArrayIndex(num_errs + 1);
            settings.setValue("errorText", "Error in Experiment Output -  references missing target: " + SynapseName);
        settings.endArray();
    }

    // get port name
    if (source != NULL) {
        if (reader->attributes().hasAttribute("port"))
            portName = reader->attributes().value("port").toString();
        else
        {
            QSettings settings;
            int num_errs = settings.beginReadArray("errors");
            settings.endArray();
            settings.beginWriteArray("errors");
                settings.setArrayIndex(num_errs + 1);
                settings.setValue("errorText", "Error in Experiment Output -  missing port tag");
            settings.endArray();
        }

        // find port in Synapse
        Port * port = NULL;
        port = findOutputPortInComponent(portName, this->source);

        if (port == NULL)
        {
            QSettings settings;
            int num_errs = settings.beginReadArray("errors");
            settings.endArray();
            settings.beginWriteArray("errors");
                settings.setArrayIndex(num_errs + 1);
                settings.setValue("errorText", "Error in Experiment Output -  references missing port " + portName);
            settings.endArray();
        }
        if (port != NULL) {
            // get indices
            if (reader->attributes().hasAttribute("indices")) {
                indices = reader->attributes().value("indices").toString();
            } else {
                indices = "all";
            }
            portName = port->name;
            portIsAnalog = port->isAnalog();
        }
    }

    if (reader->attributes().hasAttribute("start_time")) {
        this->startTime = reader->attributes().value("start_time").toString().toDouble();
    }

    if (reader->attributes().hasAttribute("end_time")) {
        this->endTime = reader->attributes().value("end_time").toString().toDouble();
    }

    if (reader->attributes().hasAttribute("tcp_port")) {

        this->isExternal = true;

        externalOutput.port = reader->attributes().value("tcp_port").toString().toInt();

        // not required
        if (reader->attributes().hasAttribute("host")) {
            externalOutput.host = reader->attributes().value("host").toString();
        }

        // not required
        if (reader->attributes().hasAttribute("timestep")) {
            externalOutput.timestep = reader->attributes().value("timestep").toString().toDouble();
        }

        if (reader->attributes().hasAttribute("size"))
            externalOutput.size = reader->attributes().value("size").toString().toInt();
        else
        {
            QSettings settings;
            int num_errs = settings.beginReadArray("errors");
            settings.endArray();
            settings.beginWriteArray("errors");
                settings.setArrayIndex(num_errs + 1);
                settings.setValue("errorText", "XML error in Experiment Output - 'size' attribute missing");
            settings.endArray();
        }

        if (reader->attributes().hasAttribute("command"))
            externalOutput.commandline = reader->attributes().value("command").toString();
        else
        {
            QSettings settings;
            int num_errs = settings.beginReadArray("errors");
            settings.endArray();
            settings.beginWriteArray("errors");
                settings.setArrayIndex(num_errs + 1);
                settings.setValue("errorText", "XML error in Experiment Output - 'command' attribute missing");
            settings.endArray();
        }
    }

    this->edit = false;
    this->set = true;

}

void exptChangeProp::readXML(QXmlStreamReader * reader) {

    if (reader->attributes().hasAttribute("alias")) {
        this->name = reader->attributes().value("alias").toString();
    }

    if (reader->attributes().hasAttribute("name")) {
        QString tempName = reader->attributes().value("name").toString();

        // find associated par on component
        for (int i = 0; i < this->component->ParameterList.size(); ++i) {
            if (this->component->ParameterList[i]->name == tempName)
                this->par = new ParameterData(this->component->ParameterList[i]);
        }
        // find associated par on component
        for (int i = 0; i < this->component->StateVariableList.size(); ++i) {
            if (this->component->StateVariableList[i]->name == tempName)
                this->par = new StateVariableData(this->component->StateVariableList[i]);
        }
    }
    else
    {
        QSettings settings;
        int num_errs = settings.beginReadArray("errors");
        settings.endArray();
        settings.beginWriteArray("errors");
            settings.setArrayIndex(num_errs + 1);
            settings.setValue("errorText", "Error in Experiment Property Change - missing name tag");
        settings.endArray();
    }

    while (reader->readNextStartElement()) {

        //cerr << reader->name().toString().toStdString() << " ##TYPE#####\n";

        if (reader->name() == "FixedValue") {
            this->par->currType = FixedValue;
            this->par->value.resize(1);
            if (reader->attributes().hasAttribute("value"))
                this->par->value[0] = reader->attributes().value("value").toString().toFloat();
            else
            {
                QSettings settings;
                int num_errs = settings.beginReadArray("errors");
                settings.endArray();
                settings.beginWriteArray("errors");
                    settings.setArrayIndex(num_errs + 1);
                    settings.setValue("errorText", "Error in Experiment Property Change - missing value tag");
                settings.endArray();
            }
            reader->readNextStartElement();
        } else if (reader->name() == "UniformDistribution") {
            this->par->currType = Statistical;
            this->par->value.resize(3);
            this->par->value[0] = 1;
            if (reader->attributes().hasAttribute("minimum"))
                this->par->value[1] = reader->attributes().value("minimum").toString().toFloat();
            else
            {
                QSettings settings;
                int num_errs = settings.beginReadArray("errors");
                settings.endArray();
                settings.beginWriteArray("errors");
                    settings.setArrayIndex(num_errs + 1);
                    settings.setValue("errorText", "Error in Experiment Property Change - missing minimum tag");
                settings.endArray();
            }
            if (reader->attributes().hasAttribute("maximum"))
                this->par->value[2] = reader->attributes().value("maximum").toString().toFloat();
            else
            {
                QSettings settings;
                int num_errs = settings.beginReadArray("errors");
                settings.endArray();
                settings.beginWriteArray("errors");
                    settings.setArrayIndex(num_errs + 1);
                    settings.setValue("errorText", "Error in Experiment Property Change - missing maximum tag");
                settings.endArray();
            }
            if (reader->attributes().hasAttribute("seed"))
                this->par->value[3] = reader->attributes().value("seed").toString().toFloat();
            else
            {
                QSettings settings;
                int num_errs = settings.beginReadArray("errors");
                settings.endArray();
                settings.beginWriteArray("errors");
                    settings.setArrayIndex(num_errs + 1);
                    settings.setValue("errorText", "Error in Experiment Property Change - missing seed tag");
                settings.endArray();
            }
            reader->readNextStartElement();
        } else if (reader->name() == "NormalDistribution") {
            this->par->currType = Statistical;
            this->par->value.resize(3);
            this->par->value[0] = 2;
            if (reader->attributes().hasAttribute("mean"))
                this->par->value[1] = reader->attributes().value("mean").toString().toFloat();
            else
            {
                QSettings settings;
                int num_errs = settings.beginReadArray("errors");
                settings.endArray();
                settings.beginWriteArray("errors");
                    settings.setArrayIndex(num_errs + 1);
                    settings.setValue("errorText", "Error in Experiment Property Change - missing mean tag");
                settings.endArray();
            }
            if (reader->attributes().hasAttribute("variance"))
                this->par->value[2] = reader->attributes().value("variance").toString().toFloat();
            else
            {
                QSettings settings;
                int num_errs = settings.beginReadArray("errors");
                settings.endArray();
                settings.beginWriteArray("errors");
                    settings.setArrayIndex(num_errs + 1);
                    settings.setValue("errorText", "Error in Experiment Property Change - missing variance tag");
                settings.endArray();
            }
            if (reader->attributes().hasAttribute("seed"))
                this->par->value[3] = reader->attributes().value("seed").toString().toFloat();
            else
            {
                QSettings settings;
                int num_errs = settings.beginReadArray("errors");
                settings.endArray();
                settings.beginWriteArray("errors");
                    settings.setArrayIndex(num_errs + 1);
                    settings.setValue("errorText", "Error in Experiment Property Change - missing seed tag");
                settings.endArray();
            }
            reader->readNextStartElement();
        } else if (reader->name() == "ValueList") {
            this->par->currType = ExplicitList;
            // clear lists as they have inherited the values at startup
            this->par->indices.clear();
            this->par->value.clear();
            while (reader->readNextStartElement()) {

                //cerr << reader->name().toString().toStdString() << " ##VALINST#####\n";

                if (reader->name() == "Value") {
                    if (reader->attributes().hasAttribute("value"))
                        this->par->value.push_back(reader->attributes().value("value").toString().toFloat());
                    else
                    {
                        QSettings settings;
                        int num_errs = settings.beginReadArray("errors");
                        settings.endArray();
                        settings.beginWriteArray("errors");
                            settings.setArrayIndex(num_errs + 1);
                            settings.setValue("errorText", "Error in Experiment Property Change - missing value tag");
                        settings.endArray();
                    }
                    if (reader->attributes().hasAttribute("index"))
                        this->par->indices.push_back(reader->attributes().value("index").toString().toFloat());
                    else
                    {
                        QSettings settings;
                        int num_errs = settings.beginReadArray("errors");
                        settings.endArray();
                        settings.beginWriteArray("errors");
                            settings.setArrayIndex(num_errs + 1);
                            settings.setValue("errorText", "Error in Experiment Property Change - missing index tag");
                        settings.endArray();
                    }
                }
                reader->readNextStartElement();
            }

        } else {
            {
                QSettings settings;
                int num_errs = settings.beginReadArray("errors");
                settings.endArray();
                settings.beginWriteArray("errors");
                    settings.setArrayIndex(num_errs + 1);
                    settings.setValue("errorText", "Error in Experiment Property Change - type of change not recognised");
                settings.endArray();
            }
        }

    }
    this->set = true;
    this->edit = false;

}

void exptLesion::readXML(QXmlStreamReader * reader, projectObject * data) {

    QString srcName;
    QString dstName;

    this->proj.clear();

    if (reader->attributes().hasAttribute("src_population"))
        srcName = reader->attributes().value("src_population").toString();
    else
    {
        QSettings settings;
        int num_errs = settings.beginReadArray("errors");
        settings.endArray();
        settings.beginWriteArray("errors");
            settings.setArrayIndex(num_errs + 1);
            settings.setValue("errorText", "Error in Experiment Lesion - missing src_population tag");
        settings.endArray();
    }
    if (reader->attributes().hasAttribute("dst_population"))
        dstName = reader->attributes().value("dst_population").toString();
    else
    {
        QSettings settings;
        int num_errs = settings.beginReadArray("errors");
        settings.endArray();
        settings.beginWriteArray("errors");
            settings.setArrayIndex(num_errs + 1);
            settings.setValue("errorText", "Error in Experiment Lesion - missing dst_population tag");
        settings.endArray();
    }

    // find projection
    for (int i = 0; i < data->network.size(); ++i) {
        for (int j = 0; j < data->network[i]->projections.size(); ++j) {
            if (data->network[i]->projections[j]->source->neuronType->getXMLName() == srcName && data->network[i]->projections[j]->destination->neuronType->getXMLName() == dstName) {
                this->proj = data->network[i]->projections[j];
            }
        }
    }

    if (this->proj == NULL) {
        {
            QSettings settings;
            int num_errs = settings.beginReadArray("errors");
            settings.endArray();
            settings.beginWriteArray("errors");
                settings.setArrayIndex(num_errs + 1);
                settings.setValue("errorText", "Error in Experiment Lesion - references missing projection");
            settings.endArray();
        }
    }
    this->set = true;
    this->edit = false;

}

// ################# table delegate allowing numbers only

NTableDelegate::NTableDelegate(QObject *parent) : QStyledItemDelegate(parent)
{
}

QWidget* NTableDelegate::createEditor(QWidget* parent,const QStyleOptionViewItem &,const QModelIndex &) const
{
    QLineEdit* editor = new QLineEdit(parent);
    QDoubleValidator* val = new QDoubleValidator(editor);
    //val->setBottom(0); // doesn't allow negative inputs
    val->setNotation(QDoubleValidator::StandardNotation);
    editor->setValidator(val);
    return editor;
}
void NTableDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    double value = index.model()->data(index,Qt::EditRole).toDouble();
    QLineEdit* line = static_cast<QLineEdit*>(editor);
    line->setText(QString().setNum(value));
}
void NTableDelegate::setModelData(QWidget* editor,QAbstractItemModel* model,const QModelIndex &index) const
{
    QLineEdit* line = static_cast<QLineEdit*>(editor);
    QString value = line->text();
    model->setData(index,value);
}
void NTableDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &) const
{
    editor->setGeometry(option.rect);
}

// #################################### DRAWING timevarying graph

TVGraph::TVGraph() {

    QPalette palette(TVGraph::palette());
    palette.setColor(backgroundRole(), Qt::white);
    setPalette(palette);

}

TVGraph::TVGraph(QVector <float> vals) {

    QPalette palette(TVGraph::palette());
    palette.setColor(backgroundRole(), Qt::white);
    setPalette(palette);
    this->vals = vals;

}

TVGraph::~TVGraph() {}

void TVGraph::paintEvent(QPaintEvent *) {


    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.setPen(Qt::darkGray);
    painter.drawLine(10, 5, 10, 48);
    painter.drawLine(7, 45, 195, 45);

    painter.setPen(Qt::darkGreen);

    if (this->vals.size() > 0) {

        if (this->vals[0] == -1) {

            int col = 0;

            QVector < QColor > cols;

            cols.push_back(QColor(255,0,0,255));
            cols.push_back(QColor(0,255,0,255));
            cols.push_back(QColor(0,0,255,255));
            cols.push_back(QColor(255,255,0,255));
            cols.push_back(QColor(255,0,255,255));
            cols.push_back(QColor(0,255,255,255));
            cols.push_back(QColor(125,125,125,255));
            cols.push_back(QColor(125,0,0,255));
            cols.push_back(QColor(0,125,0,255));
            cols.push_back(QColor(0,0,125,255));

            float maxTime = 0;
            float maxVal = 0;

            // find extent of the time and value
            for (int i = 1; i < vals.size(); i += 2) {
                if (vals[i-1] == -1) continue;
                if (vals[i] > maxVal) maxVal = vals[i];
            }
            for (int i = 0; i < vals.size(); i += 2) {
                if (vals[i] > maxTime) maxTime = vals[i];
            }
            maxTime *= 1.1;

            for (int currentIndex = 0; currentIndex < 10; ++currentIndex) {

                QVector < float > curr;

                ++col;

                bool copy = false;
                // move current index to new vector
                for (int i = 0; i < vals.size(); i+=2) {
                    if (vals[i] == -1 && copy) break;
                    if (copy)
                    {
                        curr.push_back(vals[i]);
                        curr.push_back(vals[i+1]);
                    }
                    if (vals[i] == -1 && vals[i+1] == currentIndex) {
                        copy = true;
                    }
                }

                if (curr.size() == 0) {--col; continue;}

                painter.setPen(cols[col]);

                // create a path using the values
                QPainterPath path;
                path.moveTo(QPoint(10,45));
                for (int i = 0; i < curr.size(); i += 2) {
                    path.lineTo(qRound((curr[i]/maxTime)*185.0)+10, path.currentPosition().y());
                    path.lineTo(path.currentPosition().x(), qRound(45.0-(curr[i+1]/maxVal)*40.0));
                }
                path.lineTo(200, path.currentPosition().y());
                painter.strokePath(path, painter.pen());
            }
            painter.setPen(Qt::darkGray);
            painter.setFont(QFont("Helvetica [Cronyx]", 6));

            painter.drawText(QRectF(0,5,50,10),QString::number(maxVal));
            painter.drawText(QRectF(0,25,10,10),"in");
            painter.drawText(QRectF(190,45,50,10),QString::number(maxTime));
            painter.drawText(QRectF(0,45,10,10),"0,0");
            painter.drawText(QRectF(80,45,30,10),"t(ms)");


        } else {
            // find extent of the time and value
            float maxTime = vals[vals.size()-2]*1.1;
            float maxVal = 0;
            for (int i = 1; i < vals.size(); i += 2) {
                if (vals[i] > maxVal) maxVal = vals[i];
            }

            // create a path using the values
            QPainterPath path;
            path.moveTo(QPoint(10,45));
            for (int i = 0; i < vals.size(); i += 2) {
                path.lineTo(qRound((vals[i]/maxTime)*185.0)+10, path.currentPosition().y());
                path.lineTo(path.currentPosition().x(), qRound(45.0-(vals[i+1]/maxVal)*40.0));
            }
            path.lineTo(200, path.currentPosition().y());
            painter.strokePath(path, painter.pen());

            painter.setPen(Qt::darkGray);
            painter.setFont(QFont("Helvetica [Cronyx]", 6));

            painter.drawText(QRectF(0,5,50,10),QString::number(maxVal));
            painter.drawText(QRectF(0,25,10,10),"in");
            painter.drawText(QRectF(190,45,50,10),QString::number(maxTime));
            painter.drawText(QRectF(0,45,10,10),"0,0");
            painter.drawText(QRectF(80,45,30,10),"t(ms)");
        }

    }

}
