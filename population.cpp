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

#include "population.h"
#include "experiment.h"
#include "projectobject.h"

population::population(float x, float y, float size, float aspect_ratio, QString name)
{
    this->x = x;
    this->y = y;
    this->targx = x;
    this->targy = y;
#ifdef Q_OS_MAC
    this->animspeed = 0.2;//0.1;
#else
    this->animspeed = 0.2;//0.1;
#endif
    this->size = size;
    this->aspect_ratio = aspect_ratio;
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

    isSpikeSource = false;

}

population::population(population * data)
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
    this->neuronType = new NineMLComponentData(this->neuronType);
    // fix owner
    this->neuronType->owner = this;

    isSpikeSource = false;

}

population::population(QDomElement  &e, QDomDocument *, QDomDocument * meta, projectObject * data)
{
    // defaults
    this->x = 0;
    this->y = 0;
    this->targx = 0;
    this->targy = 0;
#ifdef Q_OS_MAC
    this->animspeed = 0.2;//0.1;
#else
    this->animspeed = 0.2;//0.1;
#endif
    this->size = 1.0f;
    this->aspect_ratio = aspect_ratio;
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
    this->layoutType = new NineMLLayoutData(data->catalogLAY[0]);
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
            if (tempName.size() > 0)
                this->neuronTypeName = tempName[0];
            this->neuronTypeName.replace("_", " ");

            // do we have errors - if so abort here
            {
                QSettings settings;
                int num_errs = settings.beginReadArray("errors");
                settings.endArray();
                if (num_errs > 0)
                    return;
            }

            ///////////// FIND AND LOAD NEURON

            this->neuronType = NULL;

            if (this->neuronTypeName == "SpikeSource")
                // make a spikes source
                makeSpikeSource();
            else
                this->isSpikeSource = false;

            // if not found then match
            if (!this->isSpikeSource) {

                for (uint i = 0; i < data->catalogNB.size(); ++i) {
                    if (neuronTypeName == data->catalogNB[i]->name) {
                        this->neuronType = new NineMLComponentData((NineMLComponent *) data->catalogNB[i]);
                        this->neuronType->owner = this;
                        this->neuronType->import_parameters_from_xml(n);
                    }
                }

            }

            // if still missing then we have a problem
            if (this->neuronType == NULL) {
                this->neuronType = new NineMLComponentData(data->catalogNB[0]);
                this->neuronType->owner = this;
                QSettings settings;
                int num_errs = settings.beginReadArray("errors");
                settings.endArray();
                settings.beginWriteArray("warning");
                    settings.setArrayIndex(num_errs + 1);
                    settings.setValue("warnText",  "Network references component '" + this->neuronTypeName + "' which is not found");
                settings.endArray();
                return;
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
                    settings.setValue("errorText",  "XML error: missing Layout attribute 'url'");
                settings.endArray();
            }
            this->layoutName.chop(4);
            //this->layoutName.replace('_', ' ');

            ///////////// FIND AND LOAD LAYOUT

            bool layFound = false;

            // DO FETCH OF LAYOUT NODE HERE -  IF WE HAVE IT
            for (unsigned int i = 0; i < data->catalogLAY.size(); ++i) {
                if (this->layoutName == data->catalogLAY[i]->name) {
                    delete this->layoutType;
                    this->layoutType = new NineMLLayoutData(data->catalogLAY[i]);
                    this->layoutType->component = data->catalogLAY[i];
                    this->layoutType->import_parameters_from_xml(n);
                    layFound = true;
                    break;
                }
            }
            if (!layFound) {
                delete this->layoutType;
                this->layoutType = new NineMLLayoutData(data->catalogLAY[0]);
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
            this->x = e2.attribute("value", "").toFloat();
            this->targx = x;
        }

        if( e2.tagName() == "yPos" ) {
            this->y = e2.attribute("value", "").toFloat();
            this->targy = y;
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

void population::setupBounds() {
    this->left = this->x-this->size/(2.0)*this->aspect_ratio;
    this->right = this->x+this->size/(2.0)*this->aspect_ratio;
    this->top = this->y+this->size/2.0;
    this->bottom = this->y-this->size/2.0;
}

void population::read_inputs_from_xml(QDomElement  &e, QDomDocument * meta, projectObject * data) {

    QDomNodeList nListNRN = e.elementsByTagName("LL:Neuron");

    if (nListNRN.size() == 1) {
        e = nListNRN.item(0).toElement();

        QDomNodeList nList = e.elementsByTagName("LL:Input");
        for (uint i = 0; i < (uint) nList.size(); ++i) {

            QDomElement e2 = nList.item(i).toElement();

            genericInput * newInput = new genericInput;
            newInput->src = (NineMLComponentData *)0;
            newInput->dst = (NineMLComponentData *)0;
            newInput->destination = this;
            newInput->projInput = false;

            // read in and locate src:
            QString srcName = e2.attribute("src");

            for (uint i = 0; i < data->network.size(); ++i) {
                if (data->network[i]->neuronType->getXMLName() == srcName) {
                    newInput->src = data->network[i]->neuronType;
                    newInput->source = data->network[i];
                }
                for (uint j = 0; j < data->network[i]->projections.size(); ++j) {
                    for (uint k = 0; k < data->network[i]->projections[j]->synapses.size(); ++k) {
                        if (data->network[i]->projections[j]->synapses[k]->weightUpdateType->getXMLName() == srcName) {
                            newInput->src  = data->network[i]->projections[j]->synapses[k]->weightUpdateType;
                            newInput->source = data->network[i]->projections[j];
                        }
                        if (data->network[i]->projections[j]->synapses[k]->postsynapseType->getXMLName() == srcName) {
                            newInput->src  = data->network[i]->projections[j]->synapses[k]->postsynapseType;
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
                delete newInput->connectionType;
                newInput->connectionType = new alltoAll_connection;
                QDomNode cNode = type.item(0);
                newInput->connectionType->import_parameters_from_xml(cNode);
            }
            type = e2.elementsByTagName("OneToOneConnection");
            if (type.count() == 1) {
                delete newInput->connectionType;
                newInput->connectionType = new onetoOne_connection;
                QDomNode cNode = type.item(0);
                newInput->connectionType->import_parameters_from_xml(cNode);
            }
            type = e2.elementsByTagName("FixedProbabilityConnection");
            if (type.count() == 1) {
                delete newInput->connectionType;
                newInput->connectionType = new fixedProb_connection;
                QDomNode cNode = type.item(0);
                newInput->connectionType->import_parameters_from_xml(cNode);
            }
            type = e2.elementsByTagName("ConnectionList");
            if (type.count() == 1) {
                delete newInput->connectionType;
                newInput->connectionType = new csv_connection;
                QDomNode cNode = type.item(0);
                newInput->connectionType->import_parameters_from_xml(cNode);
            }
            type = e2.elementsByTagName("DistanceBasedConnection");
            if (type.count() == 1) {
                delete newInput->connectionType;
                newInput->connectionType = new distanceBased_connection;
                QDomNode cNode = type.item(0);
                newInput->connectionType->import_parameters_from_xml(cNode);
            }
            type = e2.elementsByTagName("KernelConnection");
            if (type.count() == 1) {
                delete newInput->connectionType;
                newInput->connectionType = new kernel_connection;
                QDomNode cNode = type.item(0);
                newInput->connectionType->import_parameters_from_xml(cNode);
                ((kernel_connection *) newInput->connectionType)->src = (population *) newInput->source;
                ((kernel_connection *) newInput->connectionType)->dst = (population *) newInput->destination;
            }


            if (newInput->src != (NineMLComponentData *)0) {
                newInput->dst = this->neuronType;
                this->neuronType->inputs.push_back(newInput);
                newInput->src->outputs.push_back(newInput);}

            else {}
                // ERRR

        }
    }

    // read metadata
    for (uint i = 0; i < this->neuronType->inputs.size(); ++i)
        this->neuronType->inputs[i]->read_meta_data(meta);

    this->neuronType->matchPorts();
}

void population::remove(rootData * data)
{
    // remove from experiment
    for (uint j = 0; j < data->experiments.size(); ++j) {
        data->experiments[j]->purgeBadPointer(this);
    }
    delete this;
}

void population::delAll(rootData * data)
{
    // remove from experiment
    for (uint j = 0; j < data->experiments.size(); ++j) {
        data->experiments[j]->purgeBadPointer(this);
    }

    // remove the projections (they'll take themselves off the vector)
    while ( this->projections.size()) {
        // delete
        this->projections[0]->delAll(data);
    }

    // remove the reverse projections (they'll take themselves off the vector)
    while (this->reverseProjections.size()) {
        // delete
        this->reverseProjections[0]->delAll(data);
    }

    neuronType->removeReferences();

    delete this;
}

void population::delAll(projectObject * data) {

    // remove from experiment
    for (int j = 0; j < data->experiments.size(); ++j) {
        data->experimentList[j]->purgeBadPointer(this); // But purgeBadPointer will delete this!?
    }

    // remove the projections (they'll take themselves off the vector)
    while ( this->projections.size()) {
        // delete
        this->projections[0]->delAll(data);
    }

    // remove the reverse projections (they'll take themselves off the vector)
    while (this->reverseProjections.size()) {
        // delete
        this->reverseProjections[0]->delAll(data);
    }

    neuronType->removeReferences();

    delete this;
}

population::~population()
{
    //qDebug() << "Population Deleted " << this->getName();
    // remove componentData

    if (isSpikeSource) {
        delete this->neuronType->component;
    }

    delete neuronType;
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

bool population::is_clicked(float x, float y, float) {

    if (this->within_bounds(x, y)) {
        return 1;
    } else {
        return 0;
    }

}

void population::animate() {

    // do animation:
    //cout << "stuff " << float(this->targx) << " " << float(this->targy) << endl;
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
    for (unsigned int i = 0; i < this->projections.size(); ++i) {
        this->projections[i]->animate(this, QPointF(delta[HORIZ], delta[VERT]));
    }

    // update reverse projections
    for (unsigned int i = 0; i < this->reverseProjections.size(); ++i) {
        this->reverseProjections[i]->animate(this, QPointF(delta[HORIZ], delta[VERT]));
    }

    // update inputs
    for (unsigned int i = 0; i < this->neuronType->inputs.size(); ++i) {
        this->neuronType->inputs[i]->animate((systemObject *) this, QPointF(delta[HORIZ], delta[VERT]));
    }

    // update outputs
    for (unsigned int i = 0; i < this->neuronType->outputs.size(); ++i) {
        this->neuronType->outputs[i]->animate((systemObject *) this, QPointF(delta[HORIZ], delta[VERT]));
    }

}

void population::setupTrans(float GLscale, float viewX, float viewY, int width, int height) {

    this->tempTrans.GLscale = GLscale;
    this->tempTrans.viewX = viewX;
    this->tempTrans.viewY = viewY;
    this->tempTrans.width = float(width);
    this->tempTrans.height = float(height);

}

QPointF population::transformPoint(QPointF point) {

    point.setX(((point.x()+this->tempTrans.viewX)*this->tempTrans.GLscale+this->tempTrans.width)/2);
    point.setY(((-point.y()+this->tempTrans.viewY)*this->tempTrans.GLscale+this->tempTrans.height)/2);
    return point;
}


void population::draw(QPainter *painter, float GLscale, float viewX, float viewY, int width, int height, QImage image, drawStyle style) {

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
        pen.setWidthF((pen.widthF()+1.0)*GLscale/100.0);
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

        return;
    case standardDrawStyle:
        // do nothing
        break;
    case spikeSourceDrawStyle:
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

    // transform the co-ordinates manually (using the qt transformation leads to blurry fonts!)
    float left = ((this->left+viewX)*GLscale+float(width))/2;
    float right = ((this->right+viewX)*GLscale+float(width))/2;
    float top = ((-this->top+viewY)*GLscale+float(height))/2;
    float bottom = ((-this->bottom+viewY)*GLscale+float(height))/2;

    QRectF rectangle(left, top, right-left, bottom-top);

    QRectF rectangleInner(left+2, top+2, right-left-4, bottom-top-4);

    QColor col(this->colour);
    col.setAlpha(100);
    painter->fillRect(rectangle, col);

    painter->drawImage(rectangle, image);

    painter->setPen(QColor(200,200,200,255));
    painter->drawRect(rectangle);
    painter->setPen(QColor(0,0,0,255));

    QString displayed_name = this->name;

    if (displayed_name.size() > 14) {
        displayed_name.resize(11);
        displayed_name = displayed_name + "...";
    }

    QString displayed_comp_name = this->neuronType->component->name;

    if (displayed_comp_name.size() > 14) {
        displayed_comp_name.resize(11);
        displayed_comp_name = displayed_comp_name + "...";
    }

    QString text = displayed_name + "\n" + QString::number(this->numNeurons) + "\n" + displayed_comp_name;
    painter->drawText(rectangleInner, Qt::AlignRight, text);

}

void population::drawSynapses(QPainter *painter, float GLscale, float viewX, float viewY, int width, int height, drawStyle style) {

    QPen oldPen = painter->pen();
    QPen pen(QColor(0,0,255,255));
    pen.setWidthF(1.5);
    painter->setPen(pen);

    // so we could have an inherited class with this function
    QImage ignored;

    // draw projections
    for (unsigned int i = 0; i < this->projections.size(); ++i) {
        this->projections[i]->draw(painter, GLscale, viewX, viewY, width, height, ignored, style);
    }
    painter->setPen(oldPen);


}

void population::drawInputs(QPainter *painter, float GLscale, float viewX, float viewY, int width, int height, drawStyle style) {

    painter->setPen(QColor(0,210,0,255));

    // so we could have an inherited class with this function
    QImage ignored;

    // draw neuron inputs
    for (unsigned int i = 0; i < this->neuronType->inputs.size(); ++i) {
        this->neuronType->inputs[i]->draw(painter, GLscale, viewX, viewY, width, height, ignored, style);
    }

    // draw projection inputs
    for (unsigned int i = 0; i < this->projections.size(); ++i) {
        this->projections[i]->drawInputs(painter, GLscale, viewX, viewY, width, height, ignored, style);
    }
    painter->setPen(QColor(0,0,0,255));


}

QPainterPath * population::addToPath(QPainterPath * path) {

    path->addRect(this->getLeft(), this->getBottom(), this->size*this->aspect_ratio, this->size);
    return path;

}

float population::leftBound(float pos) {
    return this->left + (pos-this->x);
}

float population::rightBound(float pos) {
    return this->right + (pos-this->x);
}

float population::topBound(float pos) {
    return this->top + (pos-this->y);
}

float population::bottomBound(float pos) {
    return this->bottom + (pos-this->y);
}

float population::getLeft() {
    return this->left;
}

float population::getRight() {
    return this->right;
}

float population::getTop() {
    return this->top;
}

float population::getBottom() {
    return this->bottom;
}

float population::getSide(int dir, int which) {
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

bool population::connectsTo(population * pop) {

    for (unsigned int i = 0; i < this->reverseProjections.size(); ++i) {
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

void population::write_population_xml(QXmlStreamWriter &xmlOut) {

    // population tag
    xmlOut.writeStartElement("LL:Population");

    // NEURON /////////////////

    xmlOut.writeStartElement("LL:Neuron");

    xmlOut.writeAttribute("name", this->name);

    xmlOut.writeAttribute("size", QString::number(this->numNeurons));

    if (this->isSpikeSource) {
        xmlOut.writeAttribute("url", "SpikeSource");
    } else
        this->neuronType->write_node_xml(xmlOut);

    xmlOut.writeEndElement(); // neuron

    // LAYOUT /////////////////

    xmlOut.writeStartElement("Layout");

    this->layoutType->write_node_xml(xmlOut);

    xmlOut.writeEndElement(); // layout

    // PROJECTIONS ///////////

    if (this->projections.size()>0) {

        for (unsigned int i = 0; i < this->projections.size(); ++i) {

            projection * projection = this->projections[i];

            // locate the src and dst so we can easily access information from them:
            //population * src = this;
            population * dst = projection->destination;


            // write out projection synapses:
            xmlOut.writeStartElement("LL:Projection");
            xmlOut.writeAttribute("dst_population", dst->name);

            for (unsigned int j = 0; j < projection->synapses.size(); ++j) {

                // add each Synapse
                xmlOut.writeStartElement("LL:Synapse");

                if (projection->synapses[j]->connectionType->type == Kernel) {
                    ((kernel_connection *) projection->synapses[j]->connectionType)->src = projection->source;
                    ((kernel_connection *) projection->synapses[j]->connectionType)->dst = projection->destination;
                }
                if (projection->synapses[j]->connectionType->type == DistanceBased) {
                    ((distanceBased_connection *) projection->synapses[j]->connectionType)->src = projection->source;
                    ((distanceBased_connection *) projection->synapses[j]->connectionType)->dst = projection->destination;
                }
                projection->synapses[j]->connectionType->write_node_xml(xmlOut);

                xmlOut.writeStartElement("LL:WeightUpdate");
                xmlOut.writeAttribute("name", projection->synapses[j]->weightUpdateType->getXMLName());

                projection->synapses[j]->weightUpdateType->write_node_xml(xmlOut);
                xmlOut.writeEndElement(); // synapse

                xmlOut.writeStartElement("LL:PostSynapse");
                xmlOut.writeAttribute("name", projection->synapses[j]->postsynapseType->getXMLName());
                projection->synapses[j]->postsynapseType->write_node_xml(xmlOut);
                xmlOut.writeEndElement(); // postsynapse

                xmlOut.writeEndElement(); // Synapse

            }
            xmlOut.writeEndElement(); // projection

        }
    }
    xmlOut.writeEndElement(); // population
}



void population::write_model_meta_xml(QDomDocument &meta, QDomElement &root) {

    // write a new element for this population:
    QDomElement pop = meta.createElement( "population" );
    root.appendChild(pop);
    pop.setAttribute("name", this->name);

    // add tags for each bit of metadata
    // x position
    QDomElement xPos = meta.createElement( "xPos" );
    pop.appendChild(xPos);
    xPos.setAttribute("value", this->x);

    // y position
    QDomElement yPos = meta.createElement( "yPos" );
    pop.appendChild(yPos);
    yPos.setAttribute("value", this->y);

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
    for (unsigned int i = 0; i < this->neuronType->inputs.size(); ++i) {
        this->neuronType->inputs[i]->write_model_meta_xml(meta, root);
    }

    // this->projections;
    for (unsigned int i = 0; i < this->projections.size(); ++i) {
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

void population::load_projections_from_xml(QDomElement  &e, QDomDocument * doc, QDomDocument * meta, projectObject * data) {

    ///// ADD Synapses:
    QDomNodeList cList = e.elementsByTagName("LL:Projection");

    for (unsigned int i = 0; i < (uint) cList.count(); ++i) {
        QDomElement e2 = cList.item(i).toElement();
        this->projections.push_back(new projection(e2, doc, meta, data));
    }

}

void population::getNeuronLocations(vector<loc> *locations,QColor * cols) {

    // find what has that name, and send back the details

    if (this->layoutType->component->name == "none") {

        locations->clear();

        // linear layout by default:
        for (unsigned int i = 0; i < (uint) this->numNeurons; ++i) {

            loc newLoc;

            //do a square:
            newLoc.x = i%10;
            newLoc.y = floor(float(i) / 10.0);
            newLoc.z = 0;
            locations->push_back(newLoc);

        }
        *cols = this->colour;
        return;
    }
    else
    {
        locations->clear();
        *cols = this->colour;
        // generate the locations!
        QString err = "";
        this->layoutType->generateLayout(this->numNeurons, locations, err);
        //if (err != "") emit statusBarUpdate(err, 2000);
        return;

    }

}

void population::makeSpikeSource() {

    this->isSpikeSource = true;

    // make component
    NineMLComponent * ss = new NineMLComponent();
    ss->name = "SpikeSource";
    ss->EventPortList.push_back(new EventPort);
    ss->EventPortList.back()->name = "spike";
    ss->EventPortList.back()->mode = EventSendPort;
    this->neuronTypeName = "SpikeSource";
    this->neuronType = new NineMLComponentData(ss);
    this->neuronType->owner = this;
}

void population::print() {

    cerr << this->name.toStdString() << "   ###########################\n\n";

    cerr << "X = " << this->x << " Y = " << this->y << "\n";

    cerr << "size = " << this->size << " aspect = " << this->aspect_ratio << "\n";

    cerr << "numNeurons = " << this->numNeurons << "\n";

    cerr << "Number of projections out = " << float(this->projections.size()) << "\n";
    for (uint i = 0; i < this->projections.size(); ++i) {
        this->projections[i]->print();
    }

    cerr << "Number of projections in  = " << float(this->reverseProjections.size()) << "\n";

    cerr << "\n\n\n";
}



