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

#include "undocommands.h"
#include "projections.h"
#include "rootdata.h"
#include "genericinput.h"
#include "nineML_classes.h"
#include "connection.h"
#include "mainwindow.h"
#include "nineml_rootcomponentitem.h"
#include "projectobject.h"

// ######## DELETE SELECTION #################

delSelection::delSelection(rootData * data, vector <systemObject * > list, QUndoCommand *parent) :
    QUndoCommand(parent)
{

    this->data = data;
    this->setText("delete selection");

    vector < systemObject * > populations;
    vector < systemObject * > projections;

    // spawn children
    for (uint i = 0; i < list.size(); ++i) {
        // seperate out:
        if (list[i]->type == populationObject) {
            population * pop = (population *) list[i];
            populations.push_back(list[i]);
            new delPopulation(data, pop, this);
        }
    }
    for (uint i = 0; i < list.size(); ++i) {
        if (list[i]->type == projectionObject) {
            projection * proj = (projection *) list[i];
            bool alreadyDeleted = false;
            // check if any of the populations are the source or dest of this proj
            // and it has therefore already been handled
            for (uint j = 0; j < populations.size(); ++j)
                if (proj->source == populations[j] || proj->destination == populations[j])
                    alreadyDeleted = true;
            if (!alreadyDeleted) {
                new delProjection(data, proj, this);
                projections.push_back(list[i]);
            }
        }
    }
    for (uint i = 0; i < list.size(); ++i) {
        if (list[i]->type == inputObject) {
            genericInput * input = (genericInput *) list[i];
            // check if the input has already been handled
            bool alreadyDeleted = false;
            for (uint j = 0; j < populations.size(); ++j) {
                if (input->source == populations[j] || input->destination == populations[j])
                    alreadyDeleted = true;
                population * pop = (population *) populations[j];
                for (uint k = 0; k <pop->projections.size(); ++k) {
                    projection * proj = pop->projections[k];
                    for (uint l = 0; l < proj->synapses.size(); ++l) {
                        if (input->src == proj->synapses[l]->weightUpdateType || input->dst == proj->synapses[l]->weightUpdateType || \
                                input->src == proj->synapses[l]->postsynapseType || input->dst == proj->synapses[l]->postsynapseType)
                            alreadyDeleted = true;
                    }
                }
                for (uint k = 0; k <pop->reverseProjections.size(); ++k) {
                    projection * proj = pop->reverseProjections[k];
                    for (uint l = 0; l < proj->synapses.size(); ++l) {
                        if (input->src == proj->synapses[l]->weightUpdateType || input->dst == proj->synapses[l]->weightUpdateType || \
                                input->src == proj->synapses[l]->postsynapseType || input->dst == proj->synapses[l]->postsynapseType)
                            alreadyDeleted = true;
                    }
                }

            }
            if (!alreadyDeleted)
                new delInput(data, input, this);
        }
    }
}

void delSelection::undo()
{
    // do children by calling parent class function:
    QUndoCommand::undo();
}

void delSelection::redo()
{
    // do children by calling parent class function:
    QUndoCommand::redo();
}

// ######## ADD POPULATION #################
addPopulationCmd::addPopulationCmd(rootData * data, population* pop, QUndoCommand *parent) :
    QUndoCommand(parent)
{
    this->pop = pop;
    this->data = data;
    this->setText("add population");

    selIndex = 1;
    isDeleted = true;
}

void addPopulationCmd::undo()
{
    pop->isDeleted = true;
    isDeleted = true;
    // remove from system:
    for (uint i = 0; i < data->populations.size(); ++i) {
        if (this->pop == data->populations[i])
            data->populations.erase(data->populations.begin()+i);
    }
    // might be selected:
    for (uint i = 0; i < data->selList.size(); ++i) {
        if (this->pop == data->selList[i]) {
            data->selList.erase(data->selList.begin()+i);
            selIndex = i;
            if (data->selList.size()==0) {
                data->cursor.x = this->pop->currentLocation().x();
                data->cursor.y = this->pop->currentLocation().y();
            }
        }
    }
    // do children by calling parent class function:
    QUndoCommand::undo();
}

void addPopulationCmd::redo()
{
    // add to system
    data->populations.push_back(pop);

    pop->isDeleted = false;
    isDeleted = false;
    if (selIndex != -1) {
        data->selList.push_back(pop);
        data->cursor.x = -100000;
        data->cursor.y = -100000;
    }
    // do children by calling parent class function:
    QUndoCommand::redo();
}

// ######## DELETE POPULATION #################

delPopulation::delPopulation(rootData * data, population* pop, QUndoCommand *parent) :
    QUndoCommand(parent)
{
    this->pop = pop;
    this->data = data;
    this->setText("delete population " + this->pop->getName());

    // spawn children
    for (uint i = 0; i < this->pop->neuronType->inputs.size(); ++i) {
        new delInput(data, this->pop->neuronType->inputs[i], this);
    }
    for (uint i = 0; i < this->pop->neuronType->outputs.size(); ++i) {
        // ok, find out if the input for this output is deleted
        bool destination_deleted = false;
        for (uint j = 0; j < data->selList.size(); ++j) {
            if (this->pop->neuronType->outputs[i]->destination == data->selList[j])
                destination_deleted = true;
            if (data->selList[j]->type == populationObject) {
                population * pop2 = (population *) data->selList[j];
                for (uint k = 0; k < pop2->projections.size(); ++k)
                    if (this->pop->neuronType->outputs[i]->destination == pop2->projections[k])
                        destination_deleted = true;
                for (uint k = 0; k < pop2->reverseProjections.size(); ++k)
                    if (this->pop->neuronType->outputs[i]->destination == pop2->reverseProjections[k])
                        destination_deleted = true;
            }
        }
        if (!destination_deleted)
            new delInput(data, this->pop->neuronType->outputs[i], this);

    }
    for (uint i = 0; i < this->pop->projections.size(); ++i) {
        new delProjection(data, this->pop->projections[i], this);
    }
    // only delete reverse projections if the source is not in the selList (otherwise we delete twice, which is a problem)
    for (uint i = 0; i < this->pop->reverseProjections.size(); ++i) {
        bool source_deleted = false;
        for (uint j = 0; j < data->selList.size(); ++j) {
            if (this->pop->reverseProjections[i]->source == data->selList[j])
                source_deleted = true;
        }
        if (!source_deleted)
            new delProjection(data, this->pop->reverseProjections[i], this);
    }
    // go into experiments and delete all referencing objects
    for (uint i = 0; i < this->data->experiments.size(); ++i) {
        // for each experiment
        experiment * currExpt = this->data->experiments[i];
        // loop through inputs
        for (uint j = 0; j < currExpt->ins.size(); ++j) {
            // if input references deleted pop the push a delete
            exptInput * in = currExpt->ins[j];
            if (in->target == this->pop->neuronType) {
                new deleteInputUndo(data, currExpt, in, this);
            }
        }
        // loop through outputs
        for (uint j = 0; j < currExpt->outs.size(); ++j) {
            // if input references deleted pop the push a delete
            exptOutput * out = currExpt->outs[j];
            if (out->source == this->pop->neuronType) {
                new deleteOutputUndo(data, currExpt, out, this);
            }
        }
        // loop through chnaged properties
        for (uint j = 0; j < currExpt->changes.size(); ++j) {
            // if input references deleted pop the push a delete
            exptChangeProp * prop = currExpt->changes[j];
            if (prop->component == this->pop->neuronType) {
                new deleteChangePropUndo(data, currExpt, prop, this);
            }
        }
    }
    index = -1;
    selIndex = -1;
    isDeleted = true;
}

void delPopulation::undo()
{
    pop->isDeleted = false;
    // MUST HAVE A LOCAL COPY OR INCOMING UNDOS CAN CHANGE STATE BEFORE OUTGOING DESTRUCTOR CALLED
    isDeleted = false;
    if (index != -1) {
        data->populations.insert(data->populations.begin()+index, pop);
    }
    if (selIndex != -1) {
        data->selList.push_back(pop);
        data->cursor.x = -100000;
        data->cursor.y = -100000;
    }
    // do children by calling parent class function:
    QUndoCommand::undo();
}

void delPopulation::redo()
{
    // do children by calling parent class function:
    QUndoCommand::redo();

    // remove from system
    for (uint i = 0; i < data->populations.size(); ++i) {
        if (pop == data->populations[i]) {
            data->populations.erase(data->populations.begin()+i);
            index = i;
        }
    }
    // must be selected:
    for (uint i = 0; i < data->selList.size(); ++i) {
        if (this->pop == data->selList[i]) {
            data->selList.erase(data->selList.begin()+i);
            selIndex = i;
            if (data->selList.size()==0) {
                data->cursor.x = this->pop->currentLocation().x();
                data->cursor.y = this->pop->currentLocation().y();
            }
        }
    }
    pop->isDeleted = true;
    isDeleted = true;

}

// ######## MOVE POPULATION #################
movePopulation::movePopulation(rootData * data, population* pop, const QPointF& oldPos, const QPointF& newPos, QUndoCommand *parent) :
    QUndoCommand(parent)
{
    this->pop = pop;
    this->data = data;
    this->setText("move population");
    this->oldPos = oldPos;
    this->newPos = newPos;
}

void movePopulation::undo()
{
    pop->move (this->oldPos.x(), this->oldPos.y());
    data->redrawViews();
    // undo children by calling parent class function:
    QUndoCommand::undo();
}

void movePopulation::redo()
{
    // This zeroes the relative location, as newPos has been extracted
    // from the population's targx, targy, which are absolution and
    // not mouse positions.
    pop->setLocationOffset(0, 0);
    pop->move (this->newPos.x(), this->newPos.y());
    data->redrawViews();
    QUndoCommand::redo();
}

// ######## MOVE PROJECTION HANDLE #################
moveProjectionHandle::moveProjectionHandle(rootData * data, projection* proj, const QPointF& oldPos, const QPointF& newPos, QUndoCommand *parent) :
    QUndoCommand(parent)
{
    this->proj = proj;
    this->data = data;
    this->setText("move handle");
    this->oldPos = oldPos;
    this->newPos = newPos;
}

void moveProjectionHandle::undo()
{
    proj->moveSelectedControlPoint (this->oldPos.x(), this->oldPos.y());
    data->redrawViews();
    QUndoCommand::undo();
}

void moveProjectionHandle::redo()
{
    proj->moveSelectedControlPoint (this->newPos.x(), this->newPos.y());
    data->redrawViews();
    QUndoCommand::redo();
}

// ######## ADD PROJECTION #################

addProjection::addProjection(rootData * data, projection* proj, QUndoCommand *parent) :
    QUndoCommand(parent)
{
    this->proj = proj;
    this->data = data;
    this->setText("add projection " + this->proj->getName());

    // spawn children
    new addSynapse(data, this->proj, this);

    selIndex = -1;
    isDeleted = true;
}

void addProjection::undo()
{
    proj->disconnect();
    proj->isDeleted = true;
    isDeleted = true;
    // might be selected:
    for (uint i = 0; i < data->selList.size(); ++i) {
        if (this->proj == data->selList[i]) {
            data->selList.erase(data->selList.begin()+i);
            selIndex = i;
            if (data->selList.size()==0) {
                data->cursor.x = this->proj->currentLocation().x();
                data->cursor.y = this->proj->currentLocation().y();
            }
        }
    }
    // do children by calling parent class function:
    QUndoCommand::undo();
}

void addProjection::redo()
{

    proj->connect();
    proj->isDeleted = false;
    isDeleted = false;
    if (selIndex != -1) {
        data->selList.push_back(proj);
        data->cursor.x = -100000;
        data->cursor.y = -100000;
    }
    // do children by calling parent class function:
    QUndoCommand::redo();
}

// ######## DELETE PROJECTION #################

delProjection::delProjection(rootData * data, projection* proj, QUndoCommand *parent) :
    QUndoCommand(parent)
{
    isChild = false;
    if (!parent==0 && parent->text() != "Delete selection") isChild = true;
    this->proj = proj;
    this->data = data;
    this->setText("delete projection " + this->proj->getName());
    // spawn children
    for (uint i = 0; i < this->proj->synapses.size(); ++i) {
        new delSynapse(data, this->proj, this->proj->synapses[i], this);
    }
    // go into experiments and delete all referencing objects
    for (uint i = 0; i < this->data->experiments.size(); ++i) {
        // for each experiment
        experiment * currExpt = this->data->experiments[i];
        // loop through lesions
        for (uint j = 0; j < currExpt->lesions.size(); ++j) {
            // if input references deleted pop the push a delete
            exptLesion * lesion = currExpt->lesions[j];
            if (lesion->proj == this->proj) {
                new deleteLesionUndo(data, currExpt, lesion, this);
            }
        }
    }
    selIndex = -1;
    isDeleted = true;
}

void delProjection::undo()
{
    proj->connect();
    proj->isDeleted = false;
    isDeleted = false;
    if (selIndex != -1) {
        data->selList.push_back(proj);
        data->cursor.x = -100000;
        data->cursor.y = -100000;
    }
    // do children by calling parent class function:
    QUndoCommand::undo();
}

void delProjection::redo()
{
    // do children by calling parent class function:
    QUndoCommand::redo();

    proj->disconnect();
    proj->isDeleted = true;
    isDeleted = true;
    // might be selected:
    for (uint i = 0; i < data->selList.size(); ++i) {
        if (this->proj == data->selList[i]) {
            data->selList.erase(data->selList.begin()+i);
            selIndex = i;
            if (data->selList.size()==0) {
                data->cursor.x = this->proj->currentLocation().x();
                data->cursor.y = this->proj->currentLocation().y();
            }
        }
    }

}

// ######## ADD SYNAPSE #################

addSynapse::addSynapse(rootData * data, projection * proj, QUndoCommand *parent) :
    QUndoCommand(parent)
{
    this->syn = NULL;
    this->proj = proj;
    this->data = data;
    this->setText("add synapse to " + this->proj->getName());
    syn = new synapse(proj, data, true);
    // spawn children for projInputs
    new addInput(data, proj->source->neuronType, this->syn->weightUpdateType, this);
    new addInput(data, this->syn->weightUpdateType, this->syn->postsynapseType, this);
    new addInput(data, this->syn->postsynapseType, this->proj->destination->neuronType, this);
    isDeleted = false;
}

void addSynapse::undo()
{
    // delete Synapse
    isDeleted = true;
    syn->isDeleted = true;
    QUndoCommand::undo();
    data->reDrawAll();
}

void addSynapse::redo()
{
    // create new Synapse on projection
    isDeleted = false;
    syn->isDeleted = false;
    QUndoCommand::redo();
    data->reDrawAll();
}

// ######## DELETE SYNAPSE #################

delSynapse::delSynapse(rootData * data, projection * proj, synapse * syn, QUndoCommand *parent) :
    QUndoCommand(parent)
{
    isChild = false;
    if (!parent==0) isChild = true;
    this->syn = syn;
    this->data = data;
    this->proj = proj;
    this->setText("delete synapse from " + this->proj->getName());
    isUndone = false;

    // spawn children
    for (uint i = 0; i < this->syn->postsynapseType->inputs.size(); ++i) {
        new delInput(data, this->syn->postsynapseType->inputs[i], this);
    }
    for (uint i = 0; i < this->syn->postsynapseType->outputs.size(); ++i) {
        // ok, find out if the input for this output is deleted
        bool destination_deleted = false;
        for (uint j = 0; j < data->selList.size(); ++j) {
            if (this->syn->postsynapseType->outputs[i]->destination == data->selList[j])
                destination_deleted = true;
            if (data->selList[j]->type == populationObject) {
                population * pop = (population *) data->selList[j];
                for (uint k = 0; k < pop->projections.size(); ++k)
                    if (this->syn->postsynapseType->outputs[i]->destination == pop->projections[k])
                        destination_deleted = true;
                for (uint k = 0; k < pop->reverseProjections.size(); ++k)
                    if (this->syn->postsynapseType->outputs[i]->destination == pop->reverseProjections[k])
                        destination_deleted = true;
            }
        }
        if (!destination_deleted)
            new delInput(data, this->syn->postsynapseType->outputs[i], this);
    }
    for (uint i = 0; i < this->syn->weightUpdateType->inputs.size(); ++i) {
        new delInput(data, this->syn->weightUpdateType->inputs[i], this);
    }
    for (uint i = 0; i < this->syn->weightUpdateType->outputs.size(); ++i) {
        // ok, find out if the input for this output is deleted
        bool destination_deleted = false;
        for (uint j = 0; j < data->selList.size(); ++j) {
            if (this->syn->weightUpdateType->outputs[i]->destination == data->selList[j])
                destination_deleted = true;
            if (data->selList[j]->type == populationObject) {
                population * pop = (population *) data->selList[j];
                for (uint k = 0; k < pop->projections.size(); ++k)
                    if (this->syn->weightUpdateType->outputs[i]->destination == pop->projections[k])
                        destination_deleted = true;
                for (uint k = 0; k < pop->reverseProjections.size(); ++k)
                    if (this->syn->weightUpdateType->outputs[i]->destination == pop->reverseProjections[k])
                        destination_deleted = true;
            }
        }
        if (!destination_deleted) {
            new delInput(data, this->syn->weightUpdateType->outputs[i], this);
        }
    }
    // go into experiments and delete all referencing objects
    for (uint i = 0; i < this->data->experiments.size(); ++i) {
        // for each experiment
        experiment * currExpt = this->data->experiments[i];
        // loop through inputs
        for (uint j = 0; j < currExpt->ins.size(); ++j) {
            // if input references deleted pop the push a delete
            exptInput * in = currExpt->ins[j];
            if (in->target == this->syn->weightUpdateType || in->target == this->syn->postsynapseType) {
                new deleteInputUndo(data, currExpt, in, this);
            }
        }
        // loop through outputs
        for (uint j = 0; j < currExpt->outs.size(); ++j) {
            // if input references deleted pop the push a delete
            exptOutput * out = currExpt->outs[j];
            if (out->source == this->syn->weightUpdateType || out->source == this->syn->postsynapseType) {
                new deleteOutputUndo(data, currExpt, out, this);
            }
        }
        // loop through changed properties
        for (uint j = 0; j < currExpt->changes.size(); ++j) {
            // if input references deleted pop the push a delete
            exptChangeProp * prop = currExpt->changes[j];
            if (prop->component == this->syn->weightUpdateType || prop->component == this->syn->postsynapseType) {
                new deleteChangePropUndo(data, currExpt, prop, this);
            }
        }
    }
    projPos = -1;
}

void delSynapse::undo()
{
    // add to on projection
    if (projPos != -1)
        proj->synapses.insert(proj->synapses.begin()+projPos, syn);
    isUndone = true;
    // do children by calling parent class function:
    QUndoCommand::undo();
    data->reDrawAll();
}

void delSynapse::redo()
{
    // do children by calling parent class function:
    QUndoCommand::redo();

    // remove from projection
    for (uint i = 0; i < proj->synapses.size(); ++i) {
        if (proj->synapses[i] == syn) {
            proj->synapses.erase(proj->synapses.begin()+i);
            projPos = i;
        }
    }
    isUndone = false;

    data->reDrawAll();

}


// ######## ADD GENERIC INPUT #################

addInput::addInput(rootData * data, NineMLComponentData * src, NineMLComponentData * dst, QUndoCommand *parent) :
    QUndoCommand(parent)
{
    this->data = data;
    this->src = src;
    this->dst = dst;
    this->setText("add Input from " + this->src->getXMLName() + " to " + this->dst->getXMLName());
    this->input = new genericInput(src, dst, !(parent==0));
    input->disconnect();
}

void addInput::undo()
{
    // delete input (must disconnect it first!)
    input->disconnect();
    isDeleted = true;
}

void addInput::redo()
{
    // create new Synapse on projection
    input->connect();
    isDeleted = false;
}

// ######## DELETE GENERIC INPUT #################

delInput::delInput(rootData * data, genericInput * input, QUndoCommand *parent) :
    QUndoCommand(parent)
{
    isChild = false;
    if (!parent==0 && parent->text() != "Delete selection") isChild = true;
    this->input = input;
    this->data = data;
    this->setText("delete Input from " + this->input->src->getXMLName() + " to " + this->input->dst->getXMLName());
    input->isDeleted = true;
    isDeleted = true;
    selIndex = -1;

    // sanity
    if (input->source == NULL || input->destination == NULL) {
        qDebug() << "ERROR - input without source or destination set";
    }
}

void delInput::undo()
{
    // disconnect ties for input
    input->connect();
    input->isDeleted = false;
    isDeleted = false;
    if (selIndex != -1) {
        data->selList.push_back(input);
        data->cursor.x = -100000;
        data->cursor.y = -100000;
    }
}

void delInput::redo()
{
    // reconnect ties for input
    input->disconnect();
    // might be selected:
    for (uint i = 0; i < data->selList.size(); ++i) {
        if (this->input == data->selList[i]) {
            data->selList.erase(data->selList.begin()+i);
            selIndex = i;
            if (data->selList.size()==0) {
                data->cursor.x = this->input->currentLocation().x();
                data->cursor.y = this->input->currentLocation().y();
            }
        }
    }
    input->isDeleted = true;
    isDeleted = true;
}

// ######## CHANGE CONNECTION #################

changeConnection:: changeConnection(rootData * data, systemObject * ptr, int index, QUndoCommand *parent) :
    QUndoCommand(parent)
{
    this->index = index;
    this->ptr = ptr;
    this->data = data;
    this->setText("change connection type on " + this->ptr->getName());
}

void changeConnection::undo()
{
    if (ptr->type == inputObject) {
        delete ((genericInput *) ptr)->connectionType;
        ((genericInput *) ptr)->connectionType = oldConn;
    }
    if (ptr->type == synapseObject) {
        delete ((synapse *) ptr)->connectionType;
        ((synapse *) ptr)->connectionType = oldConn;
    }
    isUndone = true;
    data->reDrawAll();
}

void changeConnection::redo()
{
    if (ptr->type == inputObject) {
        oldConn = ((genericInput *) ptr)->connectionType;
        switch(index) {
        case AlltoAll:
            ((genericInput *) ptr)->connectionType = new alltoAll_connection;
            break;
        case OnetoOne:
            ((genericInput *) ptr)->connectionType = new onetoOne_connection;
            break;
        case FixedProb:
            ((genericInput *) ptr)->connectionType = new fixedProb_connection;
            break;
        case CSV:
            ((genericInput *) ptr)->connectionType = new csv_connection;
            break;
        case DistanceBased:
            ((genericInput *) ptr)->connectionType = new distanceBased_connection;
            ((distanceBased_connection *)((genericInput *) ptr)->connectionType)->src = (population *) ((genericInput *) ptr)->source;
            ((distanceBased_connection *)((genericInput *) ptr)->connectionType)->dst = (population *) ((genericInput *) ptr)->destination;
            break;
        case Kernel:
            ((genericInput *) ptr)->connectionType = new kernel_connection;
            ((kernel_connection *)((genericInput *) ptr)->connectionType)->src = (population *) ((genericInput *) ptr)->source;
            ((kernel_connection *)((genericInput *) ptr)->connectionType)->dst = (population *) ((genericInput *) ptr)->destination;
            break;
        case Python:
            ((genericInput *) ptr)->connectionType = new pythonscript_connection;
            ((pythonscript_connection *)((genericInput *) ptr)->connectionType)->src = (population *) ((genericInput *) ptr)->source;
            ((pythonscript_connection *)((genericInput *) ptr)->connectionType)->dst = (population *) ((genericInput *) ptr)->destination;
            break;
        case CSA:
            break;
        case none:
            break;
        }
    }
    if (ptr->type == synapseObject) {
        oldConn = ((synapse *) ptr)->connectionType;
        switch(index) {
        case AlltoAll:
            ((synapse *) ptr)->connectionType = new alltoAll_connection;
            break;
        case OnetoOne:
            ((synapse *) ptr)->connectionType = new onetoOne_connection;
            break;
        case FixedProb:
            ((synapse *) ptr)->connectionType = new fixedProb_connection;
            break;
        case CSV:
            ((synapse *) ptr)->connectionType = new csv_connection;
            break;
        case DistanceBased:
            ((synapse *) ptr)->connectionType = new distanceBased_connection;
            ((distanceBased_connection *)((synapse *) ptr)->connectionType)->src = (population *) ((synapse *) ptr)->proj->source;
            ((distanceBased_connection *)((synapse *) ptr)->connectionType)->dst = (population *) ((synapse *) ptr)->proj->destination;
            break;
        case Kernel:
            ((synapse *) ptr)->connectionType = new kernel_connection;
            ((kernel_connection *)((synapse *) ptr)->connectionType)->src = (population *) ((synapse *) ptr)->proj->source;
            ((kernel_connection *)((synapse *) ptr)->connectionType)->dst = (population *) ((synapse *) ptr)->proj->destination;
            break;
        case Python:
            ((synapse *) ptr)->connectionType = new pythonscript_connection;
            ((pythonscript_connection *)((synapse *) ptr)->connectionType)->src = (population *) ((synapse *) ptr)->proj->source;
            ((pythonscript_connection *)((synapse *) ptr)->connectionType)->dst = (population *) ((synapse *) ptr)->proj->destination;
            break;
        case CSA:
            break;
        case none:
            break;
        }
    }
    isUndone = false;
    data->reDrawAll();
}

// ######## SET SIZE #################

setSizeUndo::setSizeUndo(rootData * data, population * ptr, int value, QUndoCommand *parent) :
    QUndoCommand(parent)
{
    this->value = value;
    this->oldValue = ptr->numNeurons;
    this->ptr = ptr;
    this->data = data;
    this->setText("set " + this->ptr->getName() + " size to " + QString::number(value));
    firstRedo = true;
}

void setSizeUndo::undo()
{
    ptr->numNeurons = oldValue;
    if (data->main->viewVZ.OpenGLWidget != NULL) {
        data->main->viewVZ.OpenGLWidget->parsChangedProjections();
    }
    data->reDrawPanel();
}

void setSizeUndo::redo()
{
    ptr->numNeurons = value;
    if (data->main->viewVZ.OpenGLWidget != NULL) {
        data->main->viewVZ.OpenGLWidget->parsChangedProjections();
    }
    if (!firstRedo) {
        data->reDrawPanel();
    }
    firstRedo = false;
}

// ######## SET LOC 3D #################

setLoc3Undo::setLoc3Undo(rootData * data, population * ptr, int index, int value, QUndoCommand *parent) :
    QUndoCommand(parent)
{
    this->index = index;
    this->value = value;
    if (index == 0)
        this->oldValue = ptr->loc3.x;
    if (index == 1)
        this->oldValue = ptr->loc3.y;
    if (index == 2)
        this->oldValue = ptr->loc3.z;
    this->ptr = ptr;
    this->data = data;
    if (index == 0)
        this->setText("set " + this->ptr->getName() + " x location to " + QString::number(value));
    if (index == 1)
        this->setText("set " + this->ptr->getName() + " y location to " + QString::number(value));
    if (index == 2)
        this->setText("set " + this->ptr->getName() + " z location to " + QString::number(value));
}

void setLoc3Undo::undo()
{
    if (index == 0)
        ptr->loc3.x = oldValue;
    if (index == 1)
        ptr->loc3.y = oldValue;
    if (index == 2)
        ptr->loc3.z = oldValue;
}

void setLoc3Undo::redo()
{
    if (index == 0)
        ptr->loc3.x = value;
    if (index == 1)
        ptr->loc3.y = value;
    if (index == 2)
        ptr->loc3.z = value;
}

// ######## UPDATE PAR #################

updateParUndo::updateParUndo(rootData * data, ParameterData * ptr, int index, float value, QUndoCommand *parent) :
    QUndoCommand(parent)
{
    this->value = value;
    this->oldValue = ptr->value[index];
    this->ptr = ptr;
    this->data = data;
    this->setText("set " + this->ptr->name + " to " + QString::number(value));
    this->index = index;
    this->firstRedo = true;
}

void updateParUndo::undo()
{
    ptr->value[index] = oldValue;
    data->reDrawPanel();
}

void updateParUndo::redo()
{
    ptr->value[index] = value;
    if (!firstRedo) {
        data->reDrawPanel();
    }
    firstRedo = false;
}

// ######## UPDATE CONN PROB #################

updateConnProb::updateConnProb(rootData * data, fixedProb_connection * ptr, float value, QUndoCommand *parent) :
    QUndoCommand(parent)
{
    this->value = value;
    this->oldValue = ptr->p;
    this->ptr = ptr;
    this->data = data;
    this->setText("set " + this->ptr->name + " to " + QString::number(value));
    firstRedo = true;
}

void updateConnProb::undo()
{
    ptr->p = oldValue;
    data->setTitle();
}

void updateConnProb::redo()
{
    ptr->p = value;
    firstRedo = false;
    data->setTitle();
}

// ######## UPDATE CONN EQUATION #################

updateConnEquation::updateConnEquation(rootData * data, distanceBased_connection * ptr, QString newEq, QUndoCommand *parent) :
    QUndoCommand(parent)
{
    this->value = newEq;
    this->oldValue = ptr->equation;
    this->ptr = ptr;
    this->data = data;
    this->setText("set " + this->ptr->name + " equation to " + value);
    firstRedo = true;
}

void updateConnEquation::undo()
{
    ptr->equation = oldValue;
    ptr->setUnchanged(false);
    data->setTitle();
}

void updateConnEquation::redo()
{
    ptr->equation = value;
    firstRedo = false;
    ptr->setUnchanged(false);
    data->setTitle();
}

// ######## UPDATE CONN DELAY EQUATION #################

updateConnDelayEquation::updateConnDelayEquation(rootData * data, distanceBased_connection * ptr, QString newEq, QUndoCommand *parent) :
    QUndoCommand(parent)
{
    this->value = newEq;
    this->oldValue = ptr->delayEquation;
    this->ptr = ptr;
    this->data = data;
    this->setText("set " + this->ptr->name + " delay equation to " + value);
    firstRedo = true;
}

void updateConnDelayEquation::undo()
{
    ptr->delayEquation = oldValue;
    data->setTitle();
}

void updateConnDelayEquation::redo()
{
    ptr->delayEquation = value;
    firstRedo = false;
    data->setTitle();
}

// ######## UPDATE CONN PYTHON SCRIPT PAR VALUE #################

undoUpdatePythonConnectionScriptPar::undoUpdatePythonConnectionScriptPar(rootData * data, pythonscript_connection * ptr, float new_val, QString par_name, QUndoCommand *parent) :
    QUndoCommand(parent)
{
    this->value = new_val;
    this->par_name = par_name;
    int index = -1;
    for (int i = 0; i < ptr->parNames.size();++i) {
        if (ptr->parNames[i] == par_name) {
            index = i;
        }
    }
    if (index != -1) {
        this->oldValue = ptr->parValues[index];
    }
    this->ptr = ptr;
    this->data = data;
    this->setText("set " + this->ptr->name + " parameter to " + QString::number(value));
    firstRedo = true;
}

void undoUpdatePythonConnectionScriptPar::undo()
{
    int index = -1;
    for (int i = 0; i < ptr->parNames.size();++i) {
        if (ptr->parNames[i] == par_name) {
            index = i;
        }
    }
    if (index != -1) {
        ptr->parValues[index] = this->oldValue;
    }
    data->setTitle();
}

void undoUpdatePythonConnectionScriptPar::redo()
{
    int index = -1;
    for (int i = 0; i < ptr->parNames.size();++i) {
        if (ptr->parNames[i] == par_name) {
            index = i;
        }
    }
    if (index != -1) {
        ptr->parValues[index] = this->value;
    }
    firstRedo = false;
    data->setTitle();
}

// ######## UPDATE CONN PYTHON SCRIPT PROP #################

undoUpdatePythonConnectionScriptProp::undoUpdatePythonConnectionScriptProp(rootData * data, pythonscript_connection * ptr, QString par_name, QUndoCommand *parent) :
    QUndoCommand(parent)
{
    this->par_name = par_name;
    if (this->par_name == "-no weight set-") {
        this->par_name = "";
    }
    this->oldProp = ptr->weightProp;
    this->ptr = ptr;
    this->data = data;
    this->setText("set " + this->ptr->name + " weight property to " + par_name);
    firstRedo = true;
}

void undoUpdatePythonConnectionScriptProp::undo()
{
    ptr->weightProp = oldProp;
    data->setTitle();
}

void undoUpdatePythonConnectionScriptProp::redo()
{
    ptr->weightProp = par_name;
    firstRedo = false;
    data->setTitle();
}

// ######## CHANGE PAR TYPE #################

updateParType::updateParType(rootData * data, ParameterData * ptr, QString newType, QUndoCommand *parent) :
    QUndoCommand(parent)
{
    if (newType == "FixedValue") {
        this->newType = FixedValue;
    }
    if (newType == "Statistical") {
        this->newType = Statistical;
    }
    if (newType == "Explicit") {
        this->newType = ExplicitList;
    }
    if (newType == "Undefined") {
        this->newType = Undefined;
    }
    this->oldValues = ptr->value;
    this->oldType = ptr->currType;
    this->ptr = ptr;
    this->data = data;
    this->setText("set " + this->ptr->name + " to type: " + newType);
}

void updateParType::undo()
{
    ptr->currType = oldType;
    ptr->value = oldValues;
    data->reDrawAll();
}

void updateParType::redo()
{
    ptr->value.clear();
    ptr->currType = newType;
    if (this->newType == FixedValue) {
        ptr->value.resize(1,0);
    }
    if (this->newType == Statistical) {
        ptr->value.resize(1,0);
    }
    if (this->newType == ExplicitList) {
        ptr->value.resize(1,0);
        ptr->indices.resize(1);
        ptr->indices[0] = 0;
    }
    data->reDrawAll();
}

// ######## CHANGE TITLE #################

updateTitle::updateTitle(population * ptr, QString newName, QString oldName, QUndoCommand *parent) :
    QUndoCommand(parent)
{
    this->ptr = ptr;
    this->oldName = oldName;
    this->newName = newName;
    this->setText("rename " + oldName + " to " + newName);
}

void updateTitle::undo()
{
    // set name
    ptr->name = oldName;
}

void updateTitle::redo()
{
    // set name
    ptr->name = newName;
}


// ######## CHANGE MODEL TITLE #################

updateModelTitle::updateModelTitle(rootData * data, QString newName, projectObject * project, QUndoCommand *parent) :
    QUndoCommand(parent)
{
    QSettings settings;
    this->data = data;
    this->oldName = data->currProject->name;
    this->newName = newName;
    this->setText("model rename " + oldName + " to " + newName);
    this->project = project;
}

void updateModelTitle::undo()
{
    // set name
    project->name = oldName;
    data->main->setProjectMenu();
    data->setCaptionOut(project->name);
}

void updateModelTitle::redo()
{
    // set name
    project->name = newName;
    data->main->setProjectMenu();
    data->setCaptionOut(project->name);
}

// ######## CHANGE POP/PROJ COMPONENT #################

updateComponentTypeUndo::updateComponentTypeUndo(rootData * data, NineMLComponentData * componentData, NineMLComponent * newComponent, QUndoCommand *parent) :
    QUndoCommand(parent)
{
    this->data = data;
    this->componentData = componentData;
    this->newComponent = newComponent;
    this->oldComponent = componentData->component;
    this->oldParDatas = componentData->ParameterList;
    this->oldSVDatas = componentData->StateVariableList;
    this->setText("change component from " + this->oldComponent->name + " to " + this->newComponent->name);
    // save port values
    for (uint i = 0; i < componentData->inputs.size(); ++i){
        srcPortsInputs.push_back(componentData->inputs[i]->srcPort);
        dstPortsInputs.push_back(componentData->inputs[i]->dstPort);
    }
    for (uint i = 0; i < componentData->outputs.size(); ++i){
        srcPortsOutputs.push_back(componentData->outputs[i]->srcPort);
        dstPortsOutputs.push_back(componentData->outputs[i]->dstPort);
    }
    // set up new component
    this->componentData->migrateComponent(newComponent);
    // store new ParData and SVData
    this->newParDatas = this->componentData->ParameterList;
    this->newSVDatas = this->componentData->StateVariableList;

    // find experimental references and update
    for (uint i = 0; i < this->data->experiments.size(); ++i) {
        // for each experiment
        experiment * currExpt = this->data->experiments[i];
        // loop through chnaged properties
        for (uint j = 0; j < currExpt->changes.size(); ++j) {
            // if input references deleted pop the push a delete
            exptChangeProp * prop = currExpt->changes[j];
            // search for parameter name in new component
            if (prop->component == this->componentData) {
                bool found = false;
                for (uint k = 0; k < this->newComponent->ParameterList.size(); ++k) {
                    if (prop->par->name == this->newComponent->ParameterList[k]->name) {
                        found = true;
                    }
                }
                for (uint k = 0; k < this->newComponent->StateVariableList.size(); ++k) {
                    if (prop->par->name == this->newComponent->StateVariableList[k]->name) {
                        found = true;
                    }
                }
                // if we can't find the parameter then add a delete for the changed prop
                if (!found) {
                    new deleteChangePropUndo(this->data, currExpt, prop, this);
                }
            }
        }
    }

}

updateComponentTypeUndo::~updateComponentTypeUndo()
{
    // clear up the lists!
    if (isRedone) {
        // delete old parDatas
        for (uint i = 0; i < oldParDatas.size(); ++i)
            delete oldParDatas[i];

        // delete old SVDatas
        for (uint i = 0; i < oldSVDatas.size(); ++i)
            delete oldSVDatas[i];
    }
    if (!isRedone) {
        // delete old parDatas
        for (uint i = 0; i < newParDatas.size(); ++i)
            delete newParDatas[i];

        // delete old SVDatas
        for (uint i = 0; i < newSVDatas.size(); ++i)
            delete newSVDatas[i];
    }
}

void updateComponentTypeUndo::undo()
{
    // copy old versions across
    componentData->ParameterList = oldParDatas;
    componentData->StateVariableList = oldSVDatas;
    componentData->component = oldComponent;
    // restore port values
    for (uint i = 0; i < componentData->inputs.size(); ++i){
        componentData->inputs[i]->srcPort = srcPortsInputs[i];
        componentData->inputs[i]->dstPort = dstPortsInputs[i];
    }
    for (uint i = 0; i < componentData->outputs.size(); ++i){
        componentData->outputs[i]->srcPort = srcPortsOutputs[i];
        componentData->outputs[i]->dstPort = dstPortsOutputs[i];
    }
    data->reDrawAll();
    isRedone = false;
    // do children by calling parent class function:
    QUndoCommand::undo();
}

void updateComponentTypeUndo::redo()
{
    // copy new component across
    componentData->ParameterList = newParDatas;
    componentData->StateVariableList = newSVDatas;
    componentData->component = newComponent;
    componentData->matchPorts();

    data->reDrawAll();
    isRedone = true;
    // do children by calling parent class function:
    QUndoCommand::redo();
}

// ######## UPDATE LAYOUT MIN DIST #################

updateLayoutMinDist::updateLayoutMinDist(rootData * data, NineMLLayoutData * ptr, float value, QUndoCommand *parent) :
    QUndoCommand(parent)
{
    this->value = value;
    this->oldValue = ptr->minimumDistance;
    this->ptr = ptr;
    this->data = data;
    this->setText("set " + this->ptr->component->name + " min distance to " + QString::number(value));
}

void updateLayoutMinDist::undo()
{
    ptr->minimumDistance = oldValue;
}

void updateLayoutMinDist::redo()
{
    ptr->minimumDistance = value;
}

// ######## UPDATE LAYOUT SEED #################

updateLayoutSeed::updateLayoutSeed(rootData * data, NineMLLayoutData * ptr, float value, QUndoCommand *parent) :
    QUndoCommand(parent)
{
    this->value = value;
    this->oldValue = ptr->seed;
    this->ptr = ptr;
    this->data = data;
    this->setText("set " + this->ptr->component->name + " seed to " + QString::number(value));
}

void updateLayoutSeed::undo()
{
    ptr->seed = oldValue;
}

void updateLayoutSeed::redo()
{
    ptr->seed = value;
}

// ######## PASTE PARS #################

pastePars::pastePars(rootData * data, NineMLComponentData * source, NineMLComponentData * dest, QUndoCommand *parent) :
    QUndoCommand(parent)
{
    this->data = data;
    this->source = new NineMLComponentData(source);
    this->dest = dest;
    // copy old stuff to here
    this->oldData = new NineMLComponentData(dest);
    this->setText("paste properties");
}

void pastePars::undo()
{
    this->dest->copyParsFrom(oldData);
    data->reDrawAll();
}

void pastePars::redo()
{
    this->dest->copyParsFrom(source);
    data->reDrawAll();
}

// ######## COMPONENT #################

changeComponent::changeComponent(RootComponentItem * root, NineMLComponent * oldComponent, QString message, QUndoCommand *parent) :
    QUndoCommand(parent)
{
    this->viewCL = &root->main->viewCL;
    this->setText(message);
    this->unChangedComponent = oldComponent;
    this->changedComponent = new NineMLComponent(this->viewCL->root->al);
    first_redo = true;
}

void changeComponent::undo()
{
    // load the old version, copying across the pointer to the source component
    NineMLComponent * alPtr = this->viewCL->root->alPtr;
    this->viewCL->mainWindow->initialiseModel(this->unChangedComponent);
    this->viewCL->root->alPtr = alPtr;
    viewCL->fileList->disconnect();
    viewCL->root->main->addComponentsToFileList();
    QObject::connect(viewCL->fileList, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), viewCL->root->main, SLOT(fileListItemChanged(QListWidgetItem*,QListWidgetItem*)));

    viewCL->mainWindow->updateTitle(true);
}

void changeComponent::redo()
{
    if (!first_redo) {
        // load the new version, copying across the pointer to the source component
        NineMLComponent * alPtr = this->viewCL->root->alPtr;
        this->viewCL->mainWindow->initialiseModel(this->changedComponent);
        this->viewCL->root->alPtr = alPtr;
    }
    first_redo = false;

    viewCL->fileList->disconnect();
    viewCL->root->main->addComponentsToFileList();
    QObject::connect(viewCL->fileList, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), viewCL->root->main, SLOT(fileListItemChanged(QListWidgetItem*,QListWidgetItem*)));

    viewCL->mainWindow->updateTitle(true);

}

changeComponentType::changeComponentType(RootComponentItem * root, vector <NineMLComponent *> * old_lib, vector <NineMLComponent *> * new_lib, NineMLComponent * component, QString message, QUndoCommand *parent) :
    QUndoCommand(parent)
{
    this->viewCL = &root->main->viewCL;
    this->old_lib = old_lib;
    this->new_lib = new_lib;
    this->setText(message);
    this->component = component;
    first_redo = true;
}

void changeComponentType::undo()
{
    // move from new to old
    // find new:
    for (uint i = 0; i < new_lib->size(); ++i) {
        if ((*new_lib)[i] == component) {
            new_lib->erase(new_lib->begin()+i);
            old_lib->push_back(component);
        }
    }
    viewCL->fileList->disconnect();
    viewCL->root->main->addComponentsToFileList();
    QObject::connect(viewCL->fileList, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), viewCL->root->main, SLOT(fileListItemChanged(QListWidgetItem*,QListWidgetItem*)));

    viewCL->mainWindow->updateTitle(true);
}

void changeComponentType::redo()
{
    // find old:
    for (uint i = 0; i < old_lib->size(); ++i) {
        qDebug() << (*old_lib)[i]->name;
        if ((*old_lib)[i] == component) {
            old_lib->erase(old_lib->begin()+i);
            new_lib->push_back(component);
        }
    }
    viewCL->fileList->disconnect();
    viewCL->root->main->addComponentsToFileList();
    QObject::connect(viewCL->fileList, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), viewCL->root->main, SLOT(fileListItemChanged(QListWidgetItem*,QListWidgetItem*)));

    viewCL->mainWindow->updateTitle(true);
}

// ######## EXPERIMENT #################

// delete an output - the output has no dependencies so is easy to remove and add back
deleteOutputUndo::deleteOutputUndo(rootData * data, experiment * expt, exptOutput * output, QUndoCommand *parent) :
    QUndoCommand(parent)
{
    this->data = data;
    this->expt = expt;
    this->output = output;
    this->setText("Output removed from experiment");
}

void deleteOutputUndo::undo()
{
    // add the output back into the experiment
    this->expt->outs.insert(this->expt->outs.begin()+this->location, this->output);
    // redraw to show user the result
    this->data->main->viewELhandler->redraw();
}

void deleteOutputUndo::redo()
{
    // remove the reference to the output from the experiment list
    for (uint i = 0; i < this->expt->outs.size(); ++i) {
        if (this->expt->outs[i] == this->output) {
            this->expt->outs.erase(this->expt->outs.begin()+i);
            this->location = i;
        }
    }
    // redraw to show user the result
    this->data->main->viewELhandler->redraw();

}

// delete an input - the output has no dependencies so is easy to remove and add back
deleteInputUndo::deleteInputUndo(rootData * data, experiment * expt, exptInput * input, QUndoCommand *parent) :
    QUndoCommand(parent)
{
    this->data = data;
    this->expt = expt;
    this->input = input;
    this->setText("Input removed from experiment");
}

void deleteInputUndo::undo()
{
    // add the output back into the experiment
    this->expt->ins.insert(this->expt->ins.begin()+this->location, this->input);
    // redraw to show user the result
    this->data->main->viewELhandler->redraw();
}

void deleteInputUndo::redo()
{
    // remove the reference to the output from the experiment list
    for (uint i = 0; i < this->expt->ins.size(); ++i) {
        if (this->expt->ins[i] == this->input) {
            this->expt->ins.erase(this->expt->ins.begin()+i);
            this->location = i;
        }
    }
    // redraw to show user the result
    this->data->main->viewELhandler->redraw();

}

// delete a changed property - the output has no dependencies so is easy to remove and add back
deleteChangePropUndo::deleteChangePropUndo(rootData * data, experiment * expt, exptChangeProp * prop, QUndoCommand *parent) :
    QUndoCommand(parent)
{
    this->data = data;
    this->expt = expt;
    this->prop = prop;
    this->setText("Changed property removed from experiment");
}

void deleteChangePropUndo::undo()
{
    // add the output back into the experiment
    this->expt->changes.insert(this->expt->changes.begin()+this->location, this->prop);
    // redraw to show user the result
    this->data->main->viewELhandler->redraw();
}

void deleteChangePropUndo::redo()
{
    // remove the reference to the output from the experiment list
    for (uint i = 0; i < this->expt->changes.size(); ++i) {
        if (this->expt->changes[i] == this->prop) {
            this->expt->changes.erase(this->expt->changes.begin()+i);
            this->location = i;
        }
    }
    // redraw to show user the result
    this->data->main->viewELhandler->redraw();

}

// delete a lesion - the output has no dependencies so is easy to remove and add back
deleteLesionUndo::deleteLesionUndo(rootData * data, experiment * expt, exptLesion * lesion, QUndoCommand *parent) :
    QUndoCommand(parent)
{
    this->data = data;
    this->expt = expt;
    this->lesion = lesion;
    this->setText("Lesion removed from experiment");
}

void deleteLesionUndo::undo()
{
    // add the output back into the experiment
    this->expt->lesions.insert(this->expt->lesions.begin()+this->location, this->lesion);
    // redraw to show user the result
    this->data->main->viewELhandler->redraw();
}

void deleteLesionUndo::redo()
{
    // remove the reference to the output from the experiment list
    for (uint i = 0; i < this->expt->lesions.size(); ++i) {
        if (this->expt->lesions[i] == this->lesion) {
            this->expt->lesions.erase(this->expt->lesions.begin()+i);
            this->location = i;
        }
    }
    // redraw to show user the result
    this->data->main->viewELhandler->redraw();

}
