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

#include "NL_genericinput.h"

#include "NL_connection.h"
#include "NL_projection_and_synapse.h"
#include "EL_experiment.h"

genericInput::genericInput()
{
    // only used for loading from file - and all info will be specified so no need to muck about here - except this:
    this->connectionType = new alltoAll_connection;
    // setParent has to be called with a QSharedPointer, so that has to be called externally.
    //this->connectionType->setParent (this);
    this->type = inputObject;
    // for reinserting on undo / redo
    srcPos = -1;
    dstPos = -1;
    isVisualised = false;
    source.clear();
    destination.clear();
}

genericInput::genericInput(QSharedPointer <ComponentInstance> src, QSharedPointer <ComponentInstance> dst, bool projInput)
{
    this->type = inputObject;

    this->src = src;
    this->dst = dst;
    this->source = src->owner;
    this->destination = dst->owner;

    this->projInput = projInput;

    // for reinserting on undo / redo
    srcPos = -1;
    dstPos = -1;

    // avoid projInputs being selectable at 0,0
    this->start = QPoint(-1000000, -1000000);

    this->selectedControlPoint.ind = -1;
    this->selectedControlPoint.start = false;

    this->connectionType = new onetoOne_connection;
    // setParent has to be called with a QSharedPointer, so that has to be called externally.
    //this->connectionType->setParent (this);

    // add curves if we are not a projection input
    if (!projInput) {
        this->add_curves();
    }

    isVisualised = false;
}

void genericInput::connect(QSharedPointer<genericInput> in)
{
    // connect can be called multiple times due to the nature of Undo
    for (int i = 0; i < dst->inputs.size(); ++i) {
        if (dst->inputs[i] == this) {
            // already there - give up
            return;
        }
    }
    for (int i = 0; i < src->outputs.size(); ++i) {
        if (src->outputs[i] == this) {
            // already there - give up
            return;
        }
    }

    dst->inputs.push_back(in);
    src->outputs.push_back(in);

    dst->matchPorts();
}

void genericInput::disconnect()
{
    for (int i = 0; i < dst->inputs.size(); ++i) {
        if (dst->inputs[i].data() == this) {
            dst->inputs.erase(dst->inputs.begin()+i);
            dstPos = i;
        }
    }
    for (int i = 0; i < src->outputs.size(); ++i) {
        if (src->outputs[i].data() == this) {
            src->outputs.erase(src->outputs.begin()+i);
            srcPos = i;
        }
    }
}

genericInput::~genericInput()
{
    delete this->connectionType;
}

QString genericInput::getName()
{
    return "input";
}

void genericInput::delAll(nl_rootdata *)
{
    // remove references so we don't get deleted twice
    this->disconnect();
}

void genericInput::draw(QPainter *painter, float GLscale, float viewX, float viewY, int width, int height, QImage, drawStyle style)
{
    float scale = GLscale/200.0;
    // Enforce a lower limit to scale:
    if (scale < 0.4f) {
        scale = 0.4f;
    }

    // setup for drawing curves
    this->setupTrans(GLscale, viewX, viewY, width, height);

    if (this->curves.size() > 0) {

        // The generic input colour:
        QColor colour = QColor(0,255,0,255);

        QPen oldPen = painter->pen();

        QPointF start;
        QPointF end;

        switch (style) {
        case spikeSourceDrawStyle:
        case microcircuitDrawStyle:
        {
            if (source != NULL) {
                if (source->type == projectionObject) {
                    QSharedPointer <projection> s = qSharedPointerDynamicCast <projection> (source);
                    CHECK_CAST(s);
                    if (s->curves.size() > 0 && s->destination != NULL) {
                        QLineF temp = QLineF(QPointF(s->destination->x, s->destination->y), s->curves.back().C2);
                        temp.setLength(0.6);
                        start = temp.p2();
                    } else {
                        // eek!
                        start = QPointF(0.0,0.0);
                    }
                } else if (source->type == populationObject) {
                    QSharedPointer <population> s = qSharedPointerDynamicCast <population> (source);
                    CHECK_CAST(s);
                    QLineF temp = QLineF(QPointF(s->x, s->y), this->curves.front().C1);
                    temp.setLength(0.6);
                    start = temp.p2();
                }
            }
            else
                start = this->start;

            if (destination != NULL) {
                if (destination->type == projectionObject) {
                    QSharedPointer <projection> d = qSharedPointerDynamicCast <projection> (destination);
                    CHECK_CAST(d);
                    if (d->curves.size() > 0 && d->destination != NULL) {
                        QLineF temp = QLineF(QPointF(d->destination->x, d->destination->y), d->curves.back().C2);
                        temp.setLength(0.55);
                        end = temp.p2();
                    } else {
                        // eek!
                        start = QPointF(0.0,0.0);
                    }
                } else if (destination->type == populationObject) {
                    QSharedPointer <population> d = qSharedPointerDynamicCast <population> (destination);
                    CHECK_CAST(d);
                    QLineF temp = QLineF(QPointF(d->x, d->y), this->curves.back().C2);
                    temp.setLength(0.55);
                    end = temp.p2();
                }
            }
            else
                end = this->curves.back().end;

            // set pen width
            QPen pen2 = painter->pen();
            pen2.setWidthF((pen2.widthF()+1.0)*GLscale/100.0);
            pen2.setColor(colour);
            painter->setPen(pen2);

            QPainterPath path;

            path.moveTo(this->transformPoint(start));


            for (int i = 0; i < this->curves.size(); ++i) {
                if (this->curves.size()-1 == i)
                    path.cubicTo(this->transformPoint(this->curves[i].C1), this->transformPoint(this->curves[i].C2), this->transformPoint(end));
                else
                    path.cubicTo(this->transformPoint(this->curves[i].C1), this->transformPoint(this->curves[i].C2), this->transformPoint(this->curves[i].end));
            }

            // draw start and end markers

            QPolygonF arrow_head;
            QPainterPath endPoint;
            //calculate arrow head polygon
            QPointF end_point = path.pointAtPercent(1.0);
            QPointF temp_end_point = path.pointAtPercent(0.995);
            QLineF line = QLineF(end_point, temp_end_point).unitVector();
            QLineF line2 = QLineF(line.p2(), line.p1());
            line2.setLength(line2.length()+0.05*GLscale/2.0);
            end_point = line2.p2();
            line.setLength(0.1*GLscale/2.0);
            QPointF t = line.p2() - line.p1();
            QLineF normal = line.normalVector();
            normal.setLength(normal.length()*0.8);
            QPointF a1 = normal.p2() + t;
            normal.setLength(-normal.length());
            QPointF a2 = normal.p2() + t;
            arrow_head.clear();
            arrow_head << end_point << a1 << a2 << end_point;
            endPoint.addPolygon(arrow_head);
            painter->fillPath(endPoint, colour);

            // DRAW
            painter->drawPath(path);
            painter->setPen(oldPen);

            break;
        }
        case layersDrawStyle:

            return;
        case standardDrawStyle:
        case standardDrawStyleExcitatory:
        case saveNetworkImageDrawStyle:
        {
            start = this->start;
            end = this->curves.back().end;

            // draw end marker
            QPainterPath endPoint;

            QSettings settings;
            float dpi_ratio = settings.value("dpi", 1.0).toFloat();
            if (style == saveNetworkImageDrawStyle) {
                // Ensure image output isn't affected by dpi_ratio:
                dpi_ratio = 1;
            }

            // account for hidpi in line width
            QPen linePen = painter->pen();
            linePen.setWidthF(scale*linePen.widthF()*dpi_ratio);
            if (style == saveNetworkImageDrawStyle) {
                // Wider lines and ensure colour is set for image output:
                linePen.setWidthF(linePen.widthF()*2);
                linePen.setColor(colour);
            }
            painter->setPen(linePen);

            QPainterPath path;

            // start curve drawing
            path.moveTo(this->transformPoint(start));

            // draw curves
            for (int i = 0; i < this->curves.size(); ++i) {
                if (this->curves.size()-1 == i) {
                    path.cubicTo(this->transformPoint(this->curves[i].C1),
                                 this->transformPoint(this->curves[i].C2),
                                 this->transformPoint(end));
                } else {
                    path.cubicTo(this->transformPoint(this->curves[i].C1),
                                 this->transformPoint(this->curves[i].C2),
                                 this->transformPoint(this->curves[i].end));
                }
            }

            // only draw number of synapses for Projections
            if (this->type == projectionObject) {
                QPen pen = painter->pen();
                QVector<qreal> dash;
                dash.push_back(4);
                for (int syn = 1; syn < this->synapses.size(); ++syn) {
                    dash.push_back(2.0);
                    dash.push_back(1.0);
                }
                if (synapses.size() > 1) {
                    dash.push_back(2.0);
                    dash.push_back(1.0);
                    dash.push_back(2.0);
                    pen.setWidthF((pen.widthF()+1.0) * 1.5);
                } else {
                    dash.push_back(0.0);
                }
                dash.push_back(100000.0);
                dash.push_back(0.0);

                pen.setDashPattern(dash);
                painter->setPen(pen);
            }

            // Draw the line before the end marker
            painter->drawPath(path);

            // Now draw the end marker
            if (this->type == projectionObject) {
                endPoint.addEllipse(this->transformPoint(this->curves.back().end),4,4);
                painter->drawPath(endPoint);
                painter->fillPath(endPoint, QColor(0,0,255,255));
            } else {
                endPoint.addEllipse(this->transformPoint(this->curves.back().end),
                                    0.02*dpi_ratio*GLscale,0.02*dpi_ratio*GLscale);
                painter->drawPath(endPoint);
                painter->fillPath(endPoint, QColor(0,210,0,255));
            }

            painter->setPen(oldPen);

            break;
        }
        } // switch
    }
}

void genericInput::add_curves()
{
    DBG() << "genericInput::add_curves called. Existing curves:" << this->curves.size();
    if (this->dst->owner.isNull() || this->src->owner.isNull()) {
        DBG() << "Can't lay out; dst->owner or src->owner object is null";
        return;
    }

    // add curves for drawing:
    bezierCurve newCurve;
    newCurve.end = dst->owner->currentLocation();
    this->start = src->owner->currentLocation();

    newCurve.C1 = 0.5*(dst->owner->currentLocation()+src->owner->currentLocation());
    newCurve.C2 = 0.5*(dst->owner->currentLocation()+src->owner->currentLocation());

    this->curves.push_back(newCurve);

    if (this->destination.isNull() || this->source.isNull()) {
        DBG() << "Warning: destination or source object is null";
    }

    if (this->source->type == populationObject) {
        bool handled = false;
        // if we are from a population to a projection and the pop is the Synapse of the proj, handle differently for aesthetics
        if (this->destination->type == projectionObject) {
            QSharedPointer <projection> proj = qSharedPointerDynamicCast <projection> (this->destination);
            CHECK_CAST(proj)
            if (proj->destination == this->source) {
                handled = true;
                QLineF line;
                line.setP1(this->source->currentLocation());
                line.setP2(this->destination->currentLocation());
                line = line.unitVector();
                line.setLength(1.6);
                this->curves.back().C2 = line.p2();
                line.setAngle(line.angle()+30.0);
                QSharedPointer <population> pop = qSharedPointerDynamicCast <population> (this->source);
                CHECK_CAST(pop)
                QPointF boxEdge = this->findBoxEdge(pop, line.p2().x(), line.p2().y());
                this->start = boxEdge;
                this->curves.back().C1 = line.p2();
            }
        }
        if (!handled) {
            QSharedPointer <population> pop = qSharedPointerDynamicCast <population> (this->source);
            CHECK_CAST(pop)
            QPointF boxEdge = this->findBoxEdge(pop, dst->owner->currentLocation().x(), dst->owner->currentLocation().y());
            this->start = boxEdge;
        }
    }

    if (this->destination->type == populationObject) {
        QSharedPointer <population> pop = qSharedPointerDynamicCast <population> (this->destination);
        CHECK_CAST(pop)
        QPointF boxEdge = this->findBoxEdge(pop, src->owner->currentLocation().x(), src->owner->currentLocation().y());
        this->curves.back().end = boxEdge;
    }

    // self connection population aesthetics
    if (this->destination == this->source && this->destination->type == populationObject) {
        QSharedPointer <population> pop = qSharedPointerDynamicCast <population> (this->destination);
        CHECK_CAST(pop)
        QPointF boxEdge = this->findBoxEdge(pop, this->destination->currentLocation().x(), 1000000.0);
        this->curves.back().end = boxEdge;
        pop = qSharedPointerDynamicCast <population> (this->source);
        CHECK_CAST(pop)
        boxEdge = this->findBoxEdge(pop, 1000000.0, 1000000.0);
        this->start = boxEdge;
        this->curves.back().C1 = QPointF(this->destination->currentLocation().x()+1.0, this->destination->currentLocation().y()+1.0);
        this->curves.back().C2 = QPointF(this->destination->currentLocation().x(), this->destination->currentLocation().y()+1.4);
    }

    // self projection connection aesthetics
    if (this->destination->type == projectionObject && this->source->type == projectionObject && this->destination == this->source) {
        QSharedPointer <projection> proj = qSharedPointerDynamicCast <projection> (this->destination);
        CHECK_CAST(proj)
        QLineF line;
        line.setP1(this->source->currentLocation());
        line.setP2(proj->curves.back().C2);
        line = line.unitVector();
        line.setLength(1.6);
        line.setAngle(line.angle()+20.0);
        this->curves.back().C2 = line.p2();
        line.setAngle(line.angle()+70.0);
        this->curves.back().C1 = line.p2();
    }
}

void genericInput::animate(QSharedPointer<systemObject> movingObj, QPointF delta)
{
    if (this->curves.size() > 0) {
        // if we are a self connection we get moved twice, so only move half as much each time
        if (!(this->destination == (QSharedPointer<systemObject>)0)) {
            if (this->source->getName() == this->destination->getName()) {
                delta = delta / 2;
            }
        }
        // source is moving
        if (movingObj->getName() == this->source->getName()) {
            this->start = this->start + delta;
            this->curves.front().C1 = this->curves.front().C1 + delta;
        }
        // if destination is set:
        if (!(this->destination == (QSharedPointer<systemObject>)0)) {
            // destination is moving
            if (movingObj->getName() == this->destination->getName()) {
                this->curves.back().end = this->curves.back().end + delta;
                this->curves.back().C2 = this->curves.back().C2 + delta;
            }
        }
    }
}

void genericInput::moveSelectedControlPoint(float xGL, float yGL)
{
    // convert to QPointF
    QPointF cursor(xGL, yGL);
    // move start
    if (this->selectedControlPoint.start) {
        // work out closest point on edge of source object

        if (source->type == projectionObject)
            return;

        if (source->type == populationObject) {
            QSharedPointer <population> pop = qSharedPointerDynamicCast <population> (this->source);
            CHECK_CAST(pop)
            QLineF line(QPointF(pop->x, pop->y), cursor);
            QLineF nextLine = line.unitVector();
            nextLine.setLength(1000.0);
            QPointF point = nextLine.p2();

            QPointF boxEdge = findBoxEdge(pop, point.x(), point.y());

            // realign the handle
            QLineF handle(QPointF(pop->x, pop->y), this->curves.front().C1);
            handle.setAngle(nextLine.angle());
            this->curves.front().C1 = handle.p2();

            // move the point
            this->start = boxEdge;
        }
        return;
    }
    // move other controls
    else if (this->selectedControlPoint.ind != -1) {
        // move end point
        if (this->selectedControlPoint.ind == (int) this->curves.size()-1 && (this->selectedControlPoint.type == p_end)) {

            if (destination->type == projectionObject)
                return;

            if (source->type == populationObject) {
                QSharedPointer <population> pop = qSharedPointerDynamicCast <population> (this->destination);
                CHECK_CAST(pop)
                // work out closest point on edge of destination population
                QLineF line(QPointF(pop->x, pop->y), cursor);
                QLineF nextLine = line.unitVector();
                nextLine.setLength(1000.0);
                QPointF point = nextLine.p2();

                QPointF boxEdge = findBoxEdge(pop, point.x(), point.y());

                // realign the handle
                QLineF handle(QPointF(pop->x, pop->y), this->curves.back().C2);
                handle.setAngle(nextLine.angle());
                this->curves.back().C2 = handle.p2();

                // move the point
                this->curves.back().end = boxEdge;
            }

            return;
        }

        // move other points
        switch (this->selectedControlPoint.type) {

        case C1:
            this->curves[this->selectedControlPoint.ind].C1 = cursor;
            break;

        case C2:
            this->curves[this->selectedControlPoint.ind].C2 = cursor;
            break;

        case p_end:
            // move control points either side as well
            this->curves[this->selectedControlPoint.ind+1].C1 = cursor - (this->curves[this->selectedControlPoint.ind].end - this->curves[this->selectedControlPoint.ind+1].C1);
            this->curves[this->selectedControlPoint.ind].C2 = cursor - (this->curves[this->selectedControlPoint.ind].end - this->curves[this->selectedControlPoint.ind].C2);
            this->curves[this->selectedControlPoint.ind].end = cursor;
            break;

        default:
            break;

        }
    }
}

void genericInput::write_model_meta_xml(QDomDocument &meta, QDomElement &root)
{
    // if we are a projection specific input, skip this
    if (this->projInput) return;

    // write a new element for this projection:
    QDomElement col = meta.createElement( "genericInput" );
    root.appendChild(col);

    // uniquely identify the input
    col.setAttribute("source", this->src->getXMLName());
    col.setAttribute("destination", this->dst->getXMLName());
    col.setAttribute("srcPort", this->srcPort);
    col.setAttribute("dstPort", this->dstPort);

    // start position
    QDomElement start = meta.createElement( "start" );
    col.appendChild(start);
    start.setAttribute("x", this->start.x());
    start.setAttribute("y", this->start.y());

    // bezierCurves
    QDomElement bcurves = meta.createElement( "curves" );
    col.appendChild(bcurves);

    for (int i = 0; i < this->curves.size(); ++i) {

        QDomElement curve = meta.createElement( "curve" );
        QDomElement C1 = meta.createElement( "C1" );
        C1.setAttribute("xpos", this->curves[i].C1.x());
        C1.setAttribute("ypos", this->curves[i].C1.y());
        curve.appendChild(C1);

        QDomElement C2 = meta.createElement( "C2" );
        C2.setAttribute("xpos", this->curves[i].C2.x());
        C2.setAttribute("ypos", this->curves[i].C2.y());
        curve.appendChild(C2);

        QDomElement end = meta.createElement( "end" );
        end.setAttribute("xpos", this->curves[i].end.x());
        end.setAttribute("ypos", this->curves[i].end.y());
        curve.appendChild(end);

        bcurves.appendChild(curve);
    }

    // write out connection metadata:
    // write container (name after the weight update)
    QDomElement c = meta.createElement( "connection" );

    // add the metadata description (if there is one)
    this->connectionType->write_metadata_xml(meta, c);

    col.appendChild(c);
}

void genericInput::read_meta_data(QDomDocument * meta, cursorType cursorPos)
{
    // skip if a special input for a projection
    if (this->projInput) {
        DBG() << "Special input for a projection; skip";
        return;
    }

    // now load the metadata for the projection:
    QDomNode metaNode = meta->documentElement().firstChild();

#if 0
    DBG() << "this:     source:" << this->src->getXMLName()
          << "destination:" << this->dst->getXMLName()
          << "srcPort:" <<  this->srcPort
          << "dstPort:" <<  this->dstPort;
#endif

    while(!metaNode.isNull()) {

#if 0
        DBG() << "metaNode: source:" << metaNode.toElement().attribute("source", "")
              << "destination:" << metaNode.toElement().attribute("destination", "")
              << "srcPort:" <<  metaNode.toElement().attribute("srcPort", "")
              << "dstPort:" <<  metaNode.toElement().attribute("dstPort", "");
#endif

        if (metaNode.toElement().attribute("source", "") == this->src->getXMLName()
            && metaNode.toElement().attribute("destination", "") == this->dst->getXMLName()
            && metaNode.toElement().attribute("srcPort", "") == this->srcPort
            && metaNode.toElement().attribute("dstPort", "") == this->dstPort) {

            QDomNode metaData = metaNode.toElement().firstChild();
            while (!metaData.isNull()) {

                if (metaData.toElement().tagName() == "start") {
                    this->start = QPointF(metaData.toElement().attribute("x","").toFloat()+cursorPos.x,
                                          metaData.toElement().attribute("y","").toFloat()+cursorPos.y);
                }

                // find the curves tag
                if (metaData.toElement().tagName() == "curves") {

                    // add each curve
                    QDomNodeList edgeNodeList = metaData.toElement().elementsByTagName("curve");
                    for (int i = 0; i < (int) edgeNodeList.count(); ++i) {
                        QDomNode vals = edgeNodeList.item(i).toElement().firstChild();
                        bezierCurve newCurve;
                        while (!vals.isNull()) {
                            if (vals.toElement().tagName() == "C1") {
                                newCurve.C1 = QPointF(vals.toElement().attribute("xpos").toFloat()+cursorPos.x,
                                                      vals.toElement().attribute("ypos").toFloat()+cursorPos.y);
                            }
                            if (vals.toElement().tagName() == "C2") {
                                newCurve.C2 = QPointF(vals.toElement().attribute("xpos").toFloat()+cursorPos.x,
                                                      vals.toElement().attribute("ypos").toFloat()+cursorPos.y);
                            }
                            if (vals.toElement().tagName() == "end") {
                                newCurve.end = QPointF(vals.toElement().attribute("xpos").toFloat()+cursorPos.x,
                                                       vals.toElement().attribute("ypos").toFloat()+cursorPos.y);
                            }

                            vals = vals.nextSibling();
                        }
                        // add the filled out curve to the list
                        DBG() << "Adding filled curve to this->curves from reading XML";
                        this->curves.push_back(newCurve);
                    }
                }

                // find tags for connection generators
                if (metaData.toElement().tagName() == "connection") {
                    // extract data for connection generator

                    // if we are not an empty node
                    if (!metaData.firstChildElement().isNull()) {

                        // add connection generator if we are a csv
                        if (this->connectionType->type == CSV) {
                            csv_connection * conn = dynamic_cast<csv_connection *> (this->connectionType);
                            CHECK_CAST(conn)
                            QSharedPointer <population> popsrc = qSharedPointerDynamicCast <population>(this->source);
                            CHECK_CAST(popsrc)
                            QSharedPointer <population> popdst = qSharedPointerDynamicCast <population>(this->destination);
                            CHECK_CAST(popdst)
                            // add generator
                            conn->generator = new pythonscript_connection(popsrc, popdst, conn);
                            pythonscript_connection * pyConn = dynamic_cast<pythonscript_connection *> (conn->generator);
                            CHECK_CAST(pyConn)
                            // extract data for connection generator
                            pyConn->read_metadata_xml(metaData);
                            // prevent regeneration
                            pyConn->setUnchanged(true);
                        }
                    }

                }

                metaData = metaData.nextSibling();
            }

            // remove attribute to avoid further match and return
            metaNode.toElement().removeAttribute("source");
            return;
        }

        metaNode = metaNode.nextSibling();
    }
}

QSharedPointer <systemObject> genericInput::newFromExisting(QMap<systemObject *, QSharedPointer<systemObject> > &objectMap)
{
    // create a new, identical, genericInput
    QSharedPointer <genericInput> newIn = QSharedPointer <genericInput>(new genericInput());

    newIn->curves = this->curves;
    newIn->start = this->start;
    newIn->isVisualised = this->isVisualised;

    newIn->connectionType = this->connectionType->newFromExisting();

    newIn->connectionType->setParent(newIn);

    newIn->source = this->source;
    newIn->destination = this->destination;
    newIn->projInput = this->projInput;

    newIn->srcPort = this->srcPort;
    newIn->dstPort = this->dstPort;

    newIn->src = this->src;
    newIn->dst = this->dst;

    objectMap.insert(this, newIn);

    return qSharedPointerCast <systemObject> (newIn);
}

void genericInput::remapSharedPointers(QMap<systemObject *, QSharedPointer<systemObject> > objectMap)
{

    // connection, if it has a generator
    if (this->connectionType->type == CSV) {
        csv_connection * c = dynamic_cast < csv_connection * > (this->connectionType);
        if (c && c->generator != NULL) {
            pythonscript_connection * g = dynamic_cast < pythonscript_connection * > (c->generator);
            if (g) {
                g->src = qSharedPointerDynamicCast <population> (objectMap[g->src.data()]);
                g->dst = qSharedPointerDynamicCast <population> (objectMap[g->dst.data()]);
                if (!g->src || !g->dst) {
                    qDebug() << "Error casting objectMap lookup to population in genericInput::remapSharedPointers";
                    exit(-1);
                }
            }
        }
    }

    // also we must do our own src, dst, source, and destination
    QSharedPointer < systemObject > oldSource = this->source;
    QSharedPointer < systemObject > oldDestination = this->destination;

    this->source = objectMap[this->source.data()];
    this->destination = objectMap[this->destination.data()];

    qDebug() << "Before src = " << this->src->getXMLName();
    qDebug() << "Before dst = " << this->dst->getXMLName();

    // now remap src and dst by finding out what they were, and using the new version
    if (oldSource->type == populationObject) {
        qDebug() << "source = " << oldSource.data() << " -> " << this->source.data();
        this->src = (qSharedPointerDynamicCast < population > (this->source))->neuronType;
    } else if (oldSource->type == projectionObject) {
        QSharedPointer < projection > p = qSharedPointerDynamicCast < projection > (oldSource);
        for (int i = 0; i < p->synapses.size(); ++i) {
            if (this->src == p->synapses[i]->weightUpdateType) {
                this->src = qSharedPointerDynamicCast < projection > (this->source)->synapses[i]->weightUpdateType;
            }
            if (this->src == p->synapses[i]->postsynapseType) {
                this->src = qSharedPointerDynamicCast < projection > (this->source)->synapses[i]->postsynapseType;
            }
        }
    }
    if (oldDestination->type == populationObject) {
        this->dst = (qSharedPointerDynamicCast < population > (this->destination))->neuronType;
    } else if (oldDestination->type == projectionObject) {
        QSharedPointer < projection > p = qSharedPointerDynamicCast < projection > (oldDestination);
        for (int i = 0; i < p->synapses.size(); ++i) {
            qDebug() << this->dst->getXMLName() << " " << p->synapses[i]->postsynapseType->getXMLName() << ": " << (this->dst == p->synapses[i]->postsynapseType);
            if (this->dst == p->synapses[i]->weightUpdateType) {
                this->dst = qSharedPointerDynamicCast < projection > (this->destination)->synapses[i]->weightUpdateType;
            }
            if (this->dst == p->synapses[i]->postsynapseType) {
                this->dst = qSharedPointerDynamicCast < projection > (this->destination)->synapses[i]->postsynapseType;
            }
        }
    }
    qDebug() << "After src = " << this->src->getXMLName();
    qDebug() << "After dst = " << this->dst->getXMLName();

}
