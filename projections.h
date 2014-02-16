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

#ifndef PROJECTIONS_H
#define PROJECTIONS_H

#include "globalHeader.h"

#include "systemobject.h"

// DIRECTION DEFINES
#define HORIZ 0
#define VERT 1

struct edge {
    int dir;
    float len;
};

struct bezierCurve {
    QPointF C1;
    QPointF C2;
    QPointF end;
};

enum cPointType {
    C1,
    C2,
    p_end
};

struct cPoint {
    bool start;
    int ind; // A control point is in a curve. This is the index in projection::curves of the relevant curve.
    cPointType type;
};


class synapse : public systemObject
{
public:
    synapse() {isVisualised=false;}
    synapse(projection * proj, projectObject * data, bool dontAddInputs = false);
    synapse(projection * proj, rootData * data, bool dontAddInputs = false);
    ~synapse();
    NineMLComponentData *postsynapseType;
    NineMLComponentData *weightUpdateType;
    connection *connectionType;
    bool isVisualised;
    projection * proj;
    QString getName();
    virtual void delAll(rootData *);
};

class projection : public systemObject
{
public:
    projection();
    projection(QDomElement  &e, QDomDocument * , QDomDocument * meta, projectObject *data);
    virtual ~projection();
    vector < edge > edges;
    vector < bezierCurve > curves;
    QPointF start;
    bool is_clicked(float, float,float);
    void add_curves();

    population * source;
    population * destination;
    vector <synapse *> synapses;
    int currTarg;
    QString getName();
    virtual void remove(rootData *);
    virtual void delAll(rootData *);
    virtual void delAll(projectObject *);
    void move(float,float);

    void connect();
    void disconnect();

    virtual void draw(QPainter *painter, float GLscale, float viewX, float viewY, int width, int height, QImage , drawStyle style);
    void drawInputs(QPainter *painter, float GLscale, float viewX, float viewY, int width, int height, QImage, drawStyle style);
    void drawHandles(QPainter *painter, float GLscale, float viewX, float viewY, int width, int height);
    void moveEdge(int, float, float);
    virtual void write_model_meta_xml(QDomDocument &meta, QDomElement &root);
    void read_inputs_from_xml(QDomElement  &e, QDomDocument * meta, projectObject *data);
    void print();
    QPointF findBoxEdge(population * pop, float xGL, float yGL);
    void setAutoHandles(population * pop1, population *pop2, QPointF end);
    virtual void animate(systemObject * movingObj, QPointF delta);
    virtual void moveSelectedControlPoint(float xGL, float yGL);
    bool selectControlPoint(float xGL, float yGL, float GLscale);
    bool deleteControlPoint(float xGL, float yGL, float GLscale);
    void insertControlPoint(float xGL, float yGL, float GLscale);
    QPointF currentLocation();
    QPointF selectedControlPointLocation();

    virtual void setLocationOffsetRelTo(float x, float y)
    {
        locationOffset =  this->start - QPointF(x,y);
    }

    trans tempTrans;
    void setupTrans(float GLscale, float viewX, float viewY, int width, int height);
    QPointF transformPoint(QPointF point);
    QPainterPath makeIntersectionLine(int first, int last);

    vector < genericInput * > disconnectedInputs;

protected:
    cPoint selectedControlPoint;

private:
    int srcPos;
    int dstPos;
};

#endif // PROJECTIONS_H
