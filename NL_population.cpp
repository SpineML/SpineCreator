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

#include "NL_population.h"
#include "EL_experiment.h"
#include "SC_projectobject.h"
#include <sstream>
#include <iomanip>

population::population(float x, float y, float size, float aspect_ratio, QString name)
{
    this->x = x;
    this->y = y;
    this->targx = x;
    this->targy = y;
#ifdef Q_OS_MAC
    this->animspeed = 0.2f;//0.1;
#else
    this->animspeed = 0.2f;//0.1;
#endif
    this->size = size;
    this->aspect_ratio = aspect_ratio;
    this->left = this->x-this->size/(2.0)*this->aspect_ratio;
    this->right = this->x+this->size/(2.0)*this->aspect_ratio;
    this->top = this->y+this->size/2.0;
    this->bottom = this->y-this->size/2.0;
    if (name.isEmpty()) {
        name = "New population";
    }
    this->name = name;
    this->neuronTypeName = "none";
    this->numNeurons = 1;
    this->colour = QColor(0,0,0,255);
    this->type = populationObject;
    this->isVisualised = false;
    loc3.x = 0;
    loc3.y = 0;
    loc3.z = 0;

    isSpikeSource = false;
}

population::population(QSharedPointer <population> data, QSharedPointer<population> thisSharedPointer)
{
    this->x = data->x;
    this->y = data->y;
    this->targx = data->targx;
    this->targy = data->targy;
    this->animspeed = data->animspeed;
    this->size = data->size;
    this->aspect_ratio = data->aspect_ratio;
    this->left = data->left;
    this->right = data->right;
    this->top = data->top;
    this->bottom = data->bottom;
    this->name = data->name;
    this->neuronTypeName = data->neuronTypeName;
    this->numNeurons = data->numNeurons;
    this->colour = data->colour;
    this->type = populationObject;
    this->isVisualised = false;
    loc3.x = data->loc3.x;
    loc3.y = data->loc3.y;
    loc3.z = data->loc3.z;
    this->neuronType = QSharedPointer<ComponentInstance>(new ComponentInstance(data->neuronType));
    // fix owner
    this->neuronType->owner = thisSharedPointer;

    isSpikeSource = false;
}

void
population::readFromXML (QDomElement  &e, QDomDocument *, QDomDocument * meta,
                         projectObject * data, QSharedPointer<population> thisSharedPointer)
{
    // defaults
    this->x = 0;
    this->y = 0;
    this->targx = 0;
    this->targy = 0;
#ifdef Q_OS_MAC
    this->animspeed = 0.2f;//0.1;
#else
    this->animspeed = 0.2f;//0.1;
#endif
    this->size = 1.0f;
    this->aspect_ratio = 3.0/4.0;
    this->left = this->x-this->size/(2.0)*this->aspect_ratio;
    this->right = this->x+this->size/(2.0)*this->aspect_ratio;
    this->top = this->y+this->size/2.0;
    this->bottom = this->y-this->size/2.0;
    this->name = name;
    this->neuronTypeName = "none";
    this->numNeurons = 1;
    this->colour = QColor(0,0,0,255);
    this->type = populationObject;
    this->isVisualised = false;
    loc3.x = 0;
    loc3.y = 0;
    loc3.z = 0;

    // //////////////////  fetch in the model data /////////////////////

    // make sure we don't get all crashy if layout not specified
    this->layoutType = QSharedPointer<NineMLLayoutData> (new NineMLLayoutData(data->catalogLAY[0]));
    //this->layoutType->component = data->catalogLAY[0];

    this->type = populationObject;

    QDomNode n = e.firstChild();

    while(!n.isNull()) {

        if (n.toElement().tagName() == "LL:Neuron") {

            // get attributes
            this->name = n.toElement().attribute("name");
            if (this->name == "") {
                QSettings settings;
                int num_errs = settings.beginReadArray("errors");
                settings.endArray();
                settings.beginWriteArray("errors");
                settings.setArrayIndex(num_errs + 1);
                settings.setValue("errorText",  "XML error: missing Neuron attribute 'name'");
                settings.endArray();
            }

            this->numNeurons = n.toElement().attribute("size").toInt();
            if (this->numNeurons == 0) {
                QSettings settings;
                int num_errs = settings.beginReadArray("errors");
                settings.endArray();
                settings.beginWriteArray("errors");
                settings.setArrayIndex(num_errs + 1);
                settings.setValue("errorText",  "XML error: missing Neuron attribute 'size', or 'size' is zero");
                settings.endArray();
            }
            this->neuronTypeName = n.toElement().attribute("url");
            QString real_url = this->neuronTypeName;
            if (this->neuronTypeName == "") {
                QSettings settings;
                int num_errs = settings.beginReadArray("errors");
                settings.endArray();
                settings.beginWriteArray("errors");
                settings.setArrayIndex(num_errs + 1);
                settings.setValue("errorText",  "XML error: missing Neuron attribute 'url'");
                settings.endArray();
            }

            QStringList tempName = this->neuronTypeName.split('.');
            // first section will hold the name
            if (tempName.size() > 0) {
                this->neuronTypeName = tempName[0];
            }
            this->neuronTypeName.replace("_", " ");

            // do we have errors - if so abort here
            {
                QSettings settings;
                int num_errs = settings.beginReadArray("errors");
                settings.endArray();
                if (num_errs > 0) {
                    DBG() << "Aborting on errors";
                    return;
                }
            }

            ///////////// FIND AND LOAD NEURON

            this->neuronType.clear();

            if (this->neuronTypeName == "SpikeSource") {
                // make a spikes source
                makeSpikeSource(thisSharedPointer);
            } else {
                this->isSpikeSource = false;
            }

            // if not found then match
            if (!this->isSpikeSource) {
                for (int i = 0; i < data->catalogNB.size(); ++i) {
                    if (neuronTypeName == data->catalogNB[i]->name) {
                        this->neuronType = QSharedPointer<ComponentInstance>(new ComponentInstance((QSharedPointer<Component>) data->catalogNB[i]));
                        this->neuronType->owner = thisSharedPointer;
                        this->neuronType->import_parameters_from_xml(n);
                    }
                }
            }

            // if still missing then we have a problem
            if (this->neuronType == NULL) {
                this->neuronType = QSharedPointer<ComponentInstance>(new ComponentInstance(data->catalogNB[0]));
                this->neuronType->owner = thisSharedPointer;
                QSettings settings;
                int num_errs = settings.beginReadArray("errors");
                settings.endArray();
                settings.beginWriteArray("warning");
                    settings.setArrayIndex(num_errs + 1);
                    settings.setValue("warnText",  "Network references component '" + this->neuronTypeName + "' which is not found");
                settings.endArray();
            }

        } else if (n.toElement().tagName() == "LL:Projection") {

            // handled elsewhere

        } else if (n.toElement().tagName() == "Layout") {

            this->layoutName = n.toElement().attribute("url");
            QString real_url = this->layoutName;
            if (this->layoutName == "") {
                QSettings settings;
                int num_errs = settings.beginReadArray("errors");
                settings.endArray();
                settings.beginWriteArray("errors");
                settings.setArrayIndex(num_errs + 1);
                settings.setValue("errorText", "XML error: missing Layout attribute 'url'");
                settings.endArray();
            }
            this->layoutName.chop(4);
            //this->layoutName.replace('_', ' ');

            ///////////// FIND AND LOAD LAYOUT

            bool layFound = false;

            // DO FETCH OF LAYOUT NODE HERE -  IF WE HAVE IT
            for (int i = 0; i < data->catalogLAY.size(); ++i) {
                if (this->layoutName == data->catalogLAY[i]->name) {
                    this->layoutType.clear();
                    this->layoutType = QSharedPointer<NineMLLayoutData> (new NineMLLayoutData(data->catalogLAY[i]));
                    this->layoutType->component = data->catalogLAY[i];
                    this->layoutType->import_parameters_from_xml(n);
                    layFound = true;
                    break;
                }
            }
            if (!layFound) {
                this->layoutType.clear();
                this->layoutType = QSharedPointer<NineMLLayoutData> (new NineMLLayoutData(data->catalogLAY[0]));
                QSettings settings;
                int num_errs = settings.beginReadArray("warnings");
                settings.endArray();
                settings.beginWriteArray("warnings");
                settings.setArrayIndex(num_errs + 1);
                settings.setValue("warnText",  "Network references missing Layout '" + layoutName + "'");
                settings.endArray();
            }

        } else {
            QSettings settings;
            int num_errs = settings.beginReadArray("errors");
            settings.endArray();
            settings.beginWriteArray("errors");
            settings.setArrayIndex(num_errs + 1);
            settings.setValue("errorText",  "XML error: misplaced or unknown tag '" + n.toElement().tagName() + "'");
            settings.endArray();
        }

        n = n.nextSibling();
    }

    // //////////////////  fetch in the metadata ///////////////////////
    QDomNode findN = meta->documentElement().firstChild();
    QDomElement metaE;

    // locate the matching metadata node
    while( !findN.isNull() ) {
        metaE = findN.toElement();
        if (metaE.tagName() == "population") {
            if (metaE.attribute("name","") == this->name) {
                break;
            }
        }
        findN = findN.nextSibling();
    }

    n = metaE.firstChild();
    while( !n.isNull() )
    {
        QDomElement e2 = n.toElement();
        if( e2.tagName() == "xPos" ) {
            this->x = e2.attribute("value", "").toFloat() + data->getCursorPos().x;
            this->targx = this->x;
        }

        if( e2.tagName() == "yPos" ) {
            this->y = e2.attribute("value", "").toFloat() + data->getCursorPos().y;
            this->targy = this->y;
        }

        if( e2.tagName() == "animSpeed" ) {
            this->animspeed = e2.attribute("value", "").toFloat();
        }

        if( e2.tagName() == "aspectRatio" ) {
            this->aspect_ratio = e2.attribute("value", "").toFloat();
        }

        if( e2.tagName() == "colour" ) {
            this->colour.setRed(e2.attribute("red", "").toUInt());
            this->colour.setGreen(e2.attribute("green", "").toUInt());
            this->colour.setBlue(e2.attribute("blue", "").toInt());
        }

        if( e2.tagName() == "size" ) {
            this->size = e2.attribute("value", "").toFloat();
        }

        if( e2.tagName() == "tag" ) {
            this->tag = e2.attribute("value", "").toFloat();
        }

        if( e2.tagName() == "x3D" ) {
            this->loc3.x = e2.attribute("value", "").toFloat();
        }

        if( e2.tagName() == "y3D" ) {
            this->loc3.y = e2.attribute("value", "").toFloat();
        }

        if( e2.tagName() == "z3D" ) {
            this->loc3.z = e2.attribute("value", "").toFloat();
        }

        if( e2.tagName() == "is_visualised" ) {
            this->isVisualised = e2.attribute("value", "").toFloat();
        }

        n = n.nextSibling();
    }

    // calculate some values:
    this->left = this->x-this->size/(2.0)*this->aspect_ratio;
    this->right = this->x+this->size/(2.0)*this->aspect_ratio;
    this->top = this->y+this->size/2.0;
    this->bottom = this->y-this->size/2.0;
}

void population::setupBounds()
{
    this->left = this->x-this->size/(2.0)*this->aspect_ratio;
    this->right = this->x+this->size/(2.0)*this->aspect_ratio;
    this->top = this->y+this->size/2.0;
    this->bottom = this->y-this->size/2.0;
}

#define DBGRI() qDebug() << __FUNCTION__ << ": "
void population::read_inputs_from_xml(QDomElement  &e, QDomDocument * meta, projectObject * data)
{
    //DBGBRK();
    // e starts out as a LL:Population
    //DBGRI() << "population::read_inputs_from_xml called for element " << e.tagName();
    QDomNodeList nListNRN = e.elementsByTagName("LL:Neuron");
    //DBGRI() << "LL:Neuron list has size " << nListNRN.size();
    if (nListNRN.size() == 1) {
        QDomElement e1 = nListNRN.item(0).toElement();
        QString popname = e1.attribute("name", "unknown");

        // Population name and ComponentInstance name should be the same.
        //DBGRI() << "Population name: " << popname << " neuronType (ComponentInstance) name:" << this->neuronType->getXMLName();

        QDomNodeList nList = e1.elementsByTagName("LL:Input");
        //DBGRI() << "LL:Input list has size " << nList.size();
        for (int nrni = 0; nrni < (int) nList.size(); ++nrni) {

            QDomElement e2 = nList.item(nrni).toElement();

            QSharedPointer<genericInput> newInput = QSharedPointer<genericInput>(new genericInput);
            newInput->srcCmpt.clear();
            newInput->dstCmpt.clear();
            //DBGRI() << "Setting destination to be neuronType->owner, with name: " << this->neuronType->owner->getName();
            newInput->destination = this->neuronType->owner;
            newInput->projInput = false;

            // read in src from XML and locate src in existing projectobject data:
            QString srcName = e2.attribute("src");
            //DBGRI() << "srcName: " << srcName;
            for (int i = 0; i < data->network.size(); ++i) {
                //DBGRI() << "XML element type: " << data->network[i]->neuronType->getXMLName();
                if (data->network[i]->neuronType->getXMLName() == srcName) {
                    //DBGRI() << "Set newInput->src to the thing with name " << data->network[i]->neuronType->getXMLName();
                    newInput->srcCmpt = data->network[i]->neuronType;
                    newInput->source = data->network[i];
                }
                for (int j = 0; j < data->network[i]->projections.size(); ++j) {
                    //DBGRI() << "   Projection name: " << data->network[i]->projections[j]->getName();
                    for (int k = 0; k < data->network[i]->projections[j]->synapses.size(); ++k) {
                        //DBGRI() << "       Synapse: " << data->network[i]->projections[j]->synapses[k]->getName();
                        if (data->network[i]->projections[j]->synapses[k]->weightUpdateCmpt->getXMLName() == srcName) {
                            //DBGRI() << "       WU: " << data->network[i]->projections[j]->synapses[k]->weightUpdateType->getXMLName();
                            newInput->srcCmpt  = data->network[i]->projections[j]->synapses[k]->weightUpdateCmpt;
                            //DBGRI() << "(From WU) Set newInput->src to the thing with name " << newInput->srcCmpt->getXMLName();
                            newInput->source = data->network[i]->projections[j];
                        }
                        if (data->network[i]->projections[j]->synapses[k]->postSynapseCmpt->getXMLName() == srcName) {
                            //DBGRI() << "       PS: " << data->network[i]->projections[j]->synapses[k]->postsynapseType->getXMLName();
                            newInput->srcCmpt  = data->network[i]->projections[j]->synapses[k]->postSynapseCmpt;
                            //DBGRI() << "(From PS) Set newInput->src to the thing with name " << newInput->srcCmpt->getXMLName();
                            newInput->source = data->network[i]->projections[j];
                        }
                    }
                }
            }

            // read in port names
            newInput->srcPort = e2.attribute("src_port");
            newInput->dstPort = e2.attribute("dst_port");

            // get connectivity
            QDomNodeList type = e2.elementsByTagName("AllToAllConnection");
            if (type.count() == 1) {
                delete newInput->conn;
                newInput->conn = new alltoAll_connection;
                QDomNode cNode = type.item(0);
                newInput->conn->import_parameters_from_xml(cNode);
                newInput->conn->setParent (newInput);
            }
            type = e2.elementsByTagName("OneToOneConnection");
            if (type.count() == 1) {
                delete newInput->conn;
                newInput->conn = new onetoOne_connection;
                newInput->conn->setParent(newInput);
                QDomNode cNode = type.item(0);
                newInput->conn->import_parameters_from_xml(cNode);
                newInput->conn->setParent (newInput);
            }
            type = e2.elementsByTagName("FixedProbabilityConnection");
            if (type.count() == 1) {
                delete newInput->conn;
                newInput->conn = new fixedProb_connection;
                QDomNode cNode = type.item(0);
                newInput->conn->import_parameters_from_xml(cNode);
                newInput->conn->setParent (newInput);
            }
            type = e2.elementsByTagName("ConnectionList");
            if (type.count() == 1) {
                delete newInput->conn;
                newInput->conn = new csv_connection;
                QDomNode cNode = type.item(0);
                newInput->conn->srcPop = qSharedPointerDynamicCast <population> (newInput->source);
                newInput->conn->dstPop = qSharedPointerDynamicCast <population> (newInput->destination);
                newInput->conn->import_parameters_from_xml(cNode);
                newInput->conn->setParent (newInput);
            }

            if (newInput->srcCmpt != (QSharedPointer <ComponentInstance>)0) {
                // neuronType is a ComponentInstance, owner is a systemObject. newInput->dst is a ComponentInstance.
                newInput->dstCmpt = this->neuronType; // FIXME. This doesn't seem to be the right thing.
                //DBGRI() << "For population with name " << this->name << ", set newInput->dst to the thing with name " << newInput->dstCmpt->getXMLName();
                //DBGRI() << "Pushing back the newInput!";
                this->neuronType->inputs.push_back(newInput);
                newInput->srcCmpt->outputs.push_back(newInput);
            } else {
                // Error
            }
        }
    }

    // read metadata. This should add the curves to the generic inputs.
    for (int i = 0; i < this->neuronType->inputs.size(); ++i) {
        //DBGRI() << "Reading metadata for input " << i;
        this->neuronType->inputs[i]->read_meta_data(meta, data->getCursorPos());
    }

    this->neuronType->matchPorts();
    //DBGRI() << "population::read_inputs_from_xml returning";
}

void population::delAll(nl_rootdata * data)
{
    // remove the projections (they'll take themselves off the vector)
    while ( this->projections.size()) {
        this->projections[0]->delAll(data);
    }

    // remove the reverse projections (they'll take themselves off the vector)
    while (this->reverseProjections.size()) {
        this->reverseProjections[0]->delAll(data);
    }

    neuronType->removeReferences();
}

void population::delAll(projectObject * data)
{
    // remove the projections (they'll take themselves off the vector)
    while ( this->projections.size()) {
        this->projections[0]->delAll(data);
    }

    // remove the reverse projections (they'll take themselves off the vector)
    while (this->reverseProjections.size()) {
        this->reverseProjections[0]->delAll(data);
    }

    neuronType->removeReferences();
}

population::~population()
{
    if (isSpikeSource) {
        this->neuronType->component.clear();
    }
    neuronType.clear();
}


QString population::getName()
{
    return this->name;
}

bool population::within_bounds(float x, float y)
{
    // cerr("v = %f %f %f %f %f %f", x, y, top, bottom, left, right);
    if (x > this->left && x < this->right && y > this->bottom && y < this->top) {
        return 1;
    } else {
        return 0;
    }
}

bool population::is_clicked(float x, float y, float)
{
    if (this->within_bounds(x, y)) {
        return 1;
    } else {
        return 0;
    }
}

void population::animate(QSharedPointer<population>thisSharedPointer)
{
    // do animation:
    //DBG() << "stuff " << float(this->targx) << " " << float(this->targy) << endl;
    float delta[2];
    delta[HORIZ] = this->animspeed*(this->targx - this->x);
    delta[VERT] = this->animspeed*(this->targy - this->y);

    this->x = this->x + delta[HORIZ];
    this->y = this->y + delta[VERT];
    this->left = this->x-this->size/(2.0)*this->aspect_ratio;
    this->right = this->x+this->size/(2.0)*this->aspect_ratio;
    this->top = this->y+this->size/2.0;
    this->bottom = this->y-this->size/2.0;

    // update projections:
    for (int i = 0; i < this->projections.size(); ++i) {
        this->projections[i]->animate(thisSharedPointer, QPointF(delta[HORIZ], delta[VERT]), this->projections[i]);
    }

    // update reverse projections
    for (int i = 0; i < this->reverseProjections.size(); ++i) {
        this->reverseProjections[i]->animate(thisSharedPointer, QPointF(delta[HORIZ], delta[VERT]), this->reverseProjections[i]);
    }

    // update inputs
    for (int i = 0; i < this->neuronType->inputs.size(); ++i) {
        this->neuronType->inputs[i]->animate(thisSharedPointer, QPointF(delta[HORIZ], delta[VERT]));
    }

    // update outputs
    for (int i = 0; i < this->neuronType->outputs.size(); ++i) {
        this->neuronType->outputs[i]->animate(thisSharedPointer, QPointF(delta[HORIZ], delta[VERT]));
    }
}

void population::setupTrans(float GLscale, float viewX, float viewY, int width, int height)
{
    this->tempTrans.GLscale = GLscale;
    this->tempTrans.viewX = viewX;
    this->tempTrans.viewY = viewY;
    this->tempTrans.width = float(width);
    this->tempTrans.height = float(height);
}

QPointF population::transformPoint(QPointF point)
{
    point.setX(((point.x()+this->tempTrans.viewX)*this->tempTrans.GLscale+this->tempTrans.width)/2);
    point.setY(((-point.y()+this->tempTrans.viewY)*this->tempTrans.GLscale+this->tempTrans.height)/2);
    return point;
}

void population::draw(QPainter *painter, float GLscale, float viewX, float viewY, int width, int height, QImage image, drawStyle style)
{
    float scale = GLscale/200.0;

    this->setupTrans(GLscale, viewX, viewY, width, height);

    if (this->isSpikeSource) {
        style = spikeSourceDrawStyle;
    }

    switch (style) {
    case microcircuitDrawStyle:
    {
        // draw circle
        QPen oldPen = painter->pen();
        QPen pen = painter->pen();
        pen.setWidthF((pen.widthF()+1.0)*2*scale);
        painter->setPen(pen);
        painter->drawEllipse(transformPoint(QPointF(this->x, this->y)),0.5*GLscale/2.0,0.5*GLscale/2.0);
        painter->setPen(oldPen);
        QFont oldFont = painter->font();
        QFont font = painter->font();
        font.setPointSizeF(GLscale/10.0);
        painter->setFont(font);
        // print label
        QStringList text = this->name.split(" ");
        if (text.size()>0) {
            QString title = text.at(0);
            if (title.size() > 5)
                title.resize(5);
            painter->drawText(QRectF(transformPoint(QPointF(this->x-0.5, this->y-0.2)),transformPoint(QPointF(this->x+0.5, this->y+0.2))), Qt::AlignCenter, title);
            painter->setFont(oldFont);
        }
        return;
    }
    case layersDrawStyle:
    {
        return;
    }
    case spikeSourceDrawStyle:
    {
        // draw circle
        QPen oldPen = painter->pen();
        QPen pen = painter->pen();
        pen.setWidthF((pen.widthF()+1.0));//*GLscale/100.0
        pen.setColor(QColor(200,200,200,0));
        painter->setPen(pen);
        QBrush brush;
        brush.setStyle(Qt::SolidPattern);
        QColor col(this->colour);
        col.setAlpha(100);
        brush.setColor(col);
        QBrush oldBrush = painter->brush();
        painter->setBrush(brush);
        painter->drawEllipse(transformPoint(QPointF(this->x, this->y)),0.5*GLscale/2.0,0.5*GLscale/2.0);
        QFont oldFont = painter->font();
        QFont font = painter->font();
        font.setPointSizeF(GLscale/10.0);
        painter->setFont(font);
        // print label
        pen.setColor(QColor(0,0,0,255));
        painter->setPen(pen);
        //painter->drawText(QRectF(transformPoint(QPointF(this->x-0.5, this->y-0.2)),transformPoint(QPointF(this->x+0.5, this->y+0.2))), Qt::AlignCenter, "SS");
        painter->setFont(oldFont);
        painter->setBrush(oldBrush);
        painter->setPen(oldPen);
        QImage ssimage(":/images/ssBig.png");
        QRectF imRect(transformPoint(QPointF(this->x, this->y))-QPointF(0.4*GLscale/2.0,0.4*GLscale/2.0),QSizeF(0.4*GLscale,0.4*GLscale));
        painter->drawImage(imRect, ssimage);
        return;
        break;
    }
    case standardDrawStyle:
    case standardDrawStyleExcitatory:
    case saveNetworkImageDrawStyle:
    default:
        // do nothing here, break out into the code below.
        break;
    }

    // transform the co-ordinates manually (using the qt transformation leads to blurry fonts!)
    float left = ((this->left+viewX)*GLscale+float(width))/2;
    float right = ((this->right+viewX)*GLscale+float(width))/2;
    float top = ((-this->top+viewY)*GLscale+float(height))/2;
    float bottom = ((-this->bottom+viewY)*GLscale+float(height))/2;

    QRectF rectangle(left, top, right-left, bottom-top);

    QRectF rectangleInner(left+2*scale, top+2*scale, right-left-8*scale, bottom-top-4*scale);

    QColor col(this->colour);
    col.setAlpha(100);
    QPainterPath path;
    path.addRoundedRect(rectangle,0.05*GLscale,0.05*GLscale);

    painter->fillPath(path, col);

    painter->drawImage(rectangle, image);

    // Draw a dark grey border around the population
    painter->setPen(QColor(200,200,200,255));
    painter->drawRoundedRect(rectangle,0.05*GLscale,0.05*GLscale);
    painter->setPen(QColor(0,0,0,255));

    QString displayed_name = this->name;

    if (displayed_name.size() > 13) {
        displayed_name.resize(10);
        displayed_name = displayed_name + "...";
    }

    QString displayed_comp_name = this->neuronType->component->name;

    if (displayed_comp_name.size() > 14) {
        displayed_comp_name.resize(11);
        displayed_comp_name = displayed_comp_name + "...";
    }

    QFont oldFont = painter->font();
    QFont font = painter->font();

    QString text = displayed_name + "\n" + QString::number(this->numNeurons);// + "\n" + displayed_comp_name;
    font.setPointSizeF(1.5*GLscale/20.0);
    painter->setFont(font);
    painter->drawText(rectangleInner, Qt::AlignRight|Qt::AlignTop, text);

    font.setPointSizeF(1.3*GLscale/20.0);
    painter->setFont(font);
    painter->setPen(QColor(60,60,60,255));
    painter->drawText(rectangleInner, Qt::AlignRight|Qt::AlignBottom, displayed_comp_name);

    painter->setFont(oldFont);
}

void population::drawSynapses(QPainter *painter, float GLscale, float viewX, float viewY, int width, int height, drawStyle style)
{
    QPen oldPen = painter->pen();
    QPen pen(QColor(0,0,255,255));
    pen.setWidthF(1.5);
#ifdef Q_OS_MAC
    pen.setWidthF(0.75);
#endif
    painter->setPen(pen);

    // so we could have an inherited class with this function
    QImage ignored;

    // draw projections
    for (int i = 0; i < this->projections.size(); ++i) {
        this->projections[i]->draw(painter, GLscale, viewX, viewY, width, height, ignored, style);
    }
    painter->setPen(oldPen);
}

void population::drawInputs(QPainter *painter, float GLscale, float viewX, float viewY, int width, int height, drawStyle style)
{
    painter->setPen(QColor(0,210,0,255));

    // so we could have an inherited class with this function
    QImage ignored;

    // draw neuron inputs
    for (int i = 0; i < this->neuronType->inputs.size(); ++i) {
        this->neuronType->inputs[i]->draw(painter, GLscale, viewX, viewY, width, height, ignored, style);
    }

    // draw projection inputs
    for (int i = 0; i < this->projections.size(); ++i) {
        this->projections[i]->drawInputs(painter, GLscale, viewX, viewY, width, height, ignored, style);
    }
    painter->setPen(QColor(0,0,0,255));
}

QPainterPath * population::addToPath(QPainterPath * path)
{
    path->addRect(this->getLeft(), this->getBottom(), this->size*this->aspect_ratio, this->size);
    return path;
}

float population::leftBound(float pos)
{
    return this->left + (pos-this->x);
}

float population::rightBound(float pos)
{
    return this->right + (pos-this->x);
}

float population::topBound(float pos)
{
    return this->top + (pos-this->y);
}

float population::bottomBound(float pos)
{
    return this->bottom + (pos-this->y);
}

float population::getLeft()
{
    return this->left;
}

float population::getRight()
{
    return this->right;
}

float population::getTop()
{
    return this->top;
}

float population::getBottom()
{
    return this->bottom;
}

float population::getSide(int dir, int which)
{
    if (dir == HORIZ && which == LOWER) {
        return this->left;
    }
    if (dir == HORIZ && which == UPPER) {
        return this->right;
    }
    if (dir == VERT && which == LOWER) {
        return this->bottom;
    }
    if (dir == VERT && which == UPPER) {
        return this->top;
    }
    return -10000.0;
}

bool population::connectsTo(QSharedPointer <population> pop)
{
    for (int i = 0; i < this->reverseProjections.size(); ++i) {
        if (this->reverseProjections[i]->source->name == pop->name) {
            return true;
        }
    }
    return false;
}

QPointF population::currentLocation()
{
    return QPointF(this->targx, this->targy);
}

void population::move(float x, float y)
{
    this->targx = x + this->locationOffset.x();
    this->targy = y + this->locationOffset.y();
}

void population::write_population_xml(QXmlStreamWriter &xmlOut)
{
    // population tag
    xmlOut.writeStartElement("LL:Population");

    // NEURON /////////////////

    xmlOut.writeStartElement("LL:Neuron");

    xmlOut.writeAttribute("name", this->name);

    xmlOut.writeAttribute("size", QString::number(this->numNeurons));

    if (this->isSpikeSource) {
        xmlOut.writeAttribute("url", "SpikeSource");
    } else {
        this->neuronType->write_node_xml(xmlOut);
    }
    xmlOut.writeEndElement(); // neuron

    // LAYOUT /////////////////

    xmlOut.writeStartElement("Layout");

    this->layoutType->write_node_xml(xmlOut);

    xmlOut.writeEndElement(); // layout

    // PROJECTIONS ///////////

    if (this->projections.size()>0) {

        for (int i = 0; i < this->projections.size(); ++i) {

            QSharedPointer <projection> projection = this->projections[i];

            // locate the src and dst so we can easily access information from them:
            //QSharedPointer <population> src = this;
            QSharedPointer <population> dst = projection->destination;


            // write out projection synapses:
            xmlOut.writeStartElement("LL:Projection");
            xmlOut.writeAttribute("dst_population", dst->name);

            for (int j = 0; j < projection->synapses.size(); ++j) {

                // add each Synapse
                xmlOut.writeStartElement("LL:Synapse");

                projection->synapses[j]->connectionType->srcPop = projection->source;
                projection->synapses[j]->connectionType->dstPop = projection->destination;
                projection->synapses[j]->connectionType->write_node_xml(xmlOut);

                xmlOut.writeStartElement("LL:WeightUpdate");
                xmlOut.writeAttribute("name", projection->synapses[j]->weightUpdateCmpt->getXMLName());

                projection->synapses[j]->weightUpdateCmpt->write_node_xml(xmlOut);
                xmlOut.writeEndElement(); // synapse

                xmlOut.writeStartElement("LL:PostSynapse");
                xmlOut.writeAttribute("name", projection->synapses[j]->postSynapseCmpt->getXMLName());
                projection->synapses[j]->postSynapseCmpt->write_node_xml(xmlOut);
                xmlOut.writeEndElement(); // postsynapse

                xmlOut.writeEndElement(); // Synapse

            }
            xmlOut.writeEndElement(); // projection

        }
    }
    xmlOut.writeEndElement(); // population
}


void population::write_model_meta_xml(QDomDocument &meta, QDomElement &root)
{
    // write a new element for this population:
    QDomElement pop = meta.createElement( "population" );
    root.appendChild(pop);
    pop.setAttribute("name", this->name);

    // add tags for each bit of metadata
    // x position
    QDomElement xPos = meta.createElement( "xPos" );
    pop.appendChild(xPos);
    // To avoid metaData.xml changing arbitrarily, impose a
    // granularity limit on float x.
    stringstream xx;
    xx << std::setprecision(METADATA_FLOAT_PRECISION) << this->x;
    xPos.setAttribute("value", xx.str().c_str());

    // y position
    QDomElement yPos = meta.createElement( "yPos" );
    pop.appendChild(yPos);
    stringstream yy;
    yy << std::setprecision(METADATA_FLOAT_PRECISION) << this->y;
    yPos.setAttribute("value", yy.str().c_str());

    // this->animspeed;
    QDomElement animSpeed = meta.createElement( "animSpeed" );
    pop.appendChild(animSpeed);
    animSpeed.setAttribute("value", this->animspeed);

    // this->aspect_ratio;
    QDomElement aspect_ratio = meta.createElement( "aspectRatio" );
    pop.appendChild(aspect_ratio);
    aspect_ratio.setAttribute("value", this->aspect_ratio);

    // this->colour;
    QDomElement colour = meta.createElement( "colour" );
    pop.appendChild(colour);
    colour.setAttribute("red", this->colour.red());
    colour.setAttribute("green", this->colour.green());
    colour.setAttribute("blue", this->colour.blue());

    // this->size;
    QDomElement size = meta.createElement( "size" );
    pop.appendChild(size);
    size.setAttribute("value", this->size);

    // this->tag;
    QDomElement tag = meta.createElement( "tag" );
    pop.appendChild(tag);
    tag.setAttribute("value", this->tag);

    // this->neuronType->inputs;
    for (int i = 0; i < this->neuronType->inputs.size(); ++i) {
        this->neuronType->inputs[i]->write_model_meta_xml(meta, root);
    }

    // this->projections;
    for (int i = 0; i < this->projections.size(); ++i) {
        this->projections[i]->write_model_meta_xml(meta, root);
    }

    // 3d x position
    QDomElement x3D = meta.createElement( "x3D" );
    pop.appendChild(x3D);
    x3D.setAttribute("value", this->loc3.x);

    // 3d y position
    QDomElement y3D = meta.createElement( "y3D" );
    pop.appendChild(y3D);
    y3D.setAttribute("value", this->loc3.y);

    // 3d z position
    QDomElement z3D = meta.createElement( "z3D" );
    pop.appendChild(z3D);
    z3D.setAttribute("value", this->loc3.z);

    // isvis?
    QDomElement isvis = meta.createElement( "is_visualised" );
    pop.appendChild(isvis);
    isvis.setAttribute("value", this->isVisualised);
}

void population::load_projections_from_xml(QDomElement  &e, QDomDocument * doc, QDomDocument * meta, projectObject * data)
{
    ///// ADD Synapses:
    QDomNodeList cList = e.elementsByTagName("LL:Projection");
    for (int i = 0; i < (int) cList.count(); ++i) {
        QDomElement e2 = cList.item(i).toElement();
        //DBG() << "Reading " << e2.tagName() << " dst_population: " << e2.attribute("dst_population", "no-dest-specified");
        this->projections.push_back(QSharedPointer<projection>(new projection()));
        this->projections.back()->readFromXML(e2, doc, meta, data, this->projections.back());
    }
    //DBG() << "Read " << cList.count() << " projections from XML";
}

void population::getNeuronLocations(QVector <loc> *locations,QColor * cols)
{
    // find what has that name, and send back the details

    if (this->layoutType->component->name == "none") {

        locations->clear();

        // linear layout by default:
        for (int i = 0; i < (int) this->numNeurons; ++i) {

            loc newLoc;

            //do a square:
            newLoc.x = i%10;
            newLoc.y = floor(float(i) / 10.0);
            newLoc.z = 0;
            locations->push_back(newLoc);

        }
        *cols = this->colour;
        return;

    } else {
        locations->clear();
        *cols = this->colour;
        // generate the locations!
        QString err = "";
        this->layoutType->generateLayout(this->numNeurons, locations, err);
        //if (err != "") emit statusBarUpdate(err, 2000);
        return;
    }
}

void population::makeSpikeSource(QSharedPointer<population> thisSharedPointer)
{
    this->isSpikeSource = true;
    // make component
    QSharedPointer<Component> ss = QSharedPointer<Component>(new Component());
    ss->name = "SpikeSource";
    ss->EventPortList.push_back(new EventPort);
    ss->EventPortList.back()->name = "spike";
    ss->EventPortList.back()->mode = EventSendPort;
    this->neuronTypeName = "SpikeSource";
    this->neuronType = QSharedPointer<ComponentInstance>(new ComponentInstance(ss));
    this->neuronType->owner = thisSharedPointer;
}

QSharedPointer <systemObject> population::newFromExisting(QMap <systemObject *, QSharedPointer <systemObject> > & objectMap)
{
    // create a new, identical, population

    QSharedPointer <population> newPop = QSharedPointer <population>(new population());

    newPop->x = this->x;
    newPop->y = this->y;
    newPop->targx = this->targx;
    newPop->targy = this->targy;
    newPop->animspeed = this->animspeed;
    newPop->size = this->size;
    newPop->aspect_ratio = this->aspect_ratio;
    newPop->left = this->left;
    newPop->right = this->right;
    newPop->top = this->top;
    newPop->bottom = this->bottom;
    newPop->name = this->name;
    newPop->neuronTypeName = this->neuronTypeName;
    newPop->numNeurons = this->numNeurons;
    newPop->colour = this->colour;
    newPop->type = populationObject;
    newPop->isVisualised = this->isVisualised;
    loc3.x = this->loc3.x;
    loc3.y = this->loc3.y;
    loc3.z = this->loc3.z;

    // neuron body...
    newPop->neuronType = QSharedPointer<ComponentInstance>(new ComponentInstance(this->neuronType, true/*copy inputs / outputs*/));

    // layout...
    newPop->layoutType = QSharedPointer<NineMLLayoutData>(new NineMLLayoutData(this->layoutType));

    newPop->isSpikeSource = this->isSpikeSource;

    // copy across the projections!
    newPop->projections = this->projections;
    newPop->reverseProjections = this->reverseProjections;

    objectMap.insert(this, newPop);

    return newPop;
}

void population::remapSharedPointers(QMap<systemObject *, QSharedPointer<systemObject> > pointerMap)
{
    // let's do this!
    this->neuronType->remapPointers(pointerMap);

    // and update any projections:
    for (int i = 0; i < this->projections.size(); ++i) {
        // if the proj is in the pointermap...
        DBG() << pointerMap << this->projections[i].data();
        if (pointerMap.contains(this->projections[i].data())) {
            // remap input
            this->projections[i] = qSharedPointerDynamicCast <projection> (pointerMap[this->projections[i].data()]);
            if (!this->projections[i]) {
                DBG() << "Error casting shared pointer to projection in population::remapSharedPointers";
                exit(-1);
            }
        } else {
            // remove input
            this->projections.erase(this->projections.begin()+i);
            --i;
        }
    }
    // and reverse projections:
    for (int i = 0; i < this->reverseProjections.size(); ++i) {
        // if the proj is in the pointermap...
        if (pointerMap.contains(this->reverseProjections[i].data())) {
            // remap input
            this->reverseProjections[i] = qSharedPointerDynamicCast <projection> (pointerMap[this->reverseProjections[i].data()]);
            if (!this->reverseProjections[i]) {
                DBG() << "Error casting shared pointer to reverse projection in population::remapSharedPointers";
                exit(-1);
            }
        } else {
            // remove input
            this->reverseProjections.erase(this->reverseProjections.begin()+i);
            --i;
        }
    }
}

void population::print()
{
    DBG() << this->name << "   ###########################";
    DBG() << "X = " << this->x << " Y = " << this->y;
    DBG() << "size = " << this->size << " aspect = " << this->aspect_ratio;
    DBG() << "numNeurons = " << this->numNeurons;
    DBG() << "Number of projections out = " << float(this->projections.size());
    for (int i = 0; i < this->projections.size(); ++i) {
        this->projections[i]->print();
    }
    DBG() << "Number of projections in  = " << float(this->reverseProjections.size());
}
