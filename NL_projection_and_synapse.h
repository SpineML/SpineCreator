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

#include "NL_systemobject.h"

// DIRECTION DEFINES
#define HORIZ 0
#define VERT 1

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

// A synapse's parent is a projection, a pointer to the parent is held
// in proj.
class synapse : public systemObject
{
public:
    synapse() {isVisualised=false;}
    synapse(QSharedPointer <projection> proj, projectObject * data, bool dontAddInputs = false);
    synapse(QSharedPointer <projection> proj, nl_rootdata * data, bool dontAddInputs = false);
    ~synapse();
    QSharedPointer<ComponentInstance> postsynapseType;
    QSharedPointer<ComponentInstance> weightUpdateType;
    connection *connectionType;
    /*!
     * A label for the connection type. Filled with the connection
     * generator, as read from the metadata.xml (For example, the
     * connectivity may be defined by a pythonscript_connection, even
     * though this->connectionType is a csv_connection after
     * generation).
     */
    QString connectionTypeStr;
    bool isVisualised;
    /*!
     * Presumably this is the parent projection.
     */
    QSharedPointer <projection> proj;
    QString getName();
    int getSynapseIndex();
    virtual void delAll(nl_rootdata *);
    QSharedPointer < systemObject > newFromExisting(QMap <systemObject *, QSharedPointer <systemObject> > &);
    void remapSharedPointers(QMap <systemObject *, QSharedPointer <systemObject> >);
};

// A projection contains synapses.
class projection : public systemObject
{
public:
    projection();
    void readFromXML(QDomElement  &e, QDomDocument * , QDomDocument * meta, projectObject *data, QSharedPointer<projection>);
    virtual ~projection();
    QVector < bezierCurve > curves;
    QPointF start;
    bool is_clicked(float, float,float);
    virtual void add_curves();

    QSharedPointer <population> destination;
    QSharedPointer <population> source;

    QVector <QSharedPointer <synapse> > synapses;
    int currTarg;
    QString getName();
    virtual void remove(nl_rootdata *);
    virtual void delAll(nl_rootdata *);
    virtual void delAll(projectObject *);
    void move(float,float);

    void connect(QSharedPointer<projection> in);
    void disconnect();

    void setStyle(drawStyle style);
    drawStyle style();

    virtual void draw(QPainter *painter, float GLscale, float viewX, float viewY, int width, int height, QImage , drawStyle style);
    void drawInputs(QPainter *painter, float GLscale, float viewX, float viewY, int width, int height, QImage, drawStyle style);
    void drawHandles(QPainter *painter, float GLscale, float viewX, float viewY, int width, int height);
    void moveEdge(int, float, float);
    virtual void write_model_meta_xml(QDomDocument &meta, QDomElement &root);
    void read_inputs_from_xml(QDomElement  &e, QDomDocument * meta, projectObject *data, QSharedPointer<projection>);
    void print();
    QPointF findBoxEdge(QSharedPointer <population> pop, float xGL, float yGL);
    void setAutoHandles(QSharedPointer <population> pop1, QSharedPointer <population>pop2, QPointF end);
    void animate(QSharedPointer<systemObject> movingObj, QPointF delta, QSharedPointer<projection> thisSharedPointer);
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

    QSharedPointer < systemObject > newFromExisting(QMap<systemObject *, QSharedPointer<systemObject> > &);
    void remapSharedPointers(QMap <systemObject *, QSharedPointer <systemObject> >);

    trans tempTrans;
    void setupTrans(float GLscale, float viewX, float viewY, int width, int height);
    QPointF transformPoint(QPointF point);
    QPainterPath makeIntersectionLine(int first, int last);

    QVector < QSharedPointer<genericInput> > disconnectedInputs;

    // If true, show projection label.
    bool showLabel;

protected:
    cPoint selectedControlPoint;

private:

    /*!
     * Return true if there is more than one synapse and the synapses
     * do not all share the same connectivity pattern.
     */
    bool multipleConnTypes(void);

    /*!
     * Draw the projection label. Arguments are all variables from
     * projections::draw.
     */
    void drawLabel (QPainter* painter, QPen& linePen, QPen& pointerLinePen, QPen& labelPen,
                    const float GLscale, const float scale);

    /*!
     * produce an arrow head.
     */
    QPolygonF makeArrowHead (QPainterPath& path, const float GLscale);

    /*!
     * Using this->curves, find a suitable label position for the
     * projection label. Place the label on the outside edge of the
     * curve. syn is the synapse number and influences the label
     * position. The scale is also used.  Startline pos is the
     * position at which a "pointer line" from the label text to the
     * object of the label should begin.
     */
    QPointF getLabelPos (QFont& f, int syn, const QString& tstr, const float scale,
                         QPointF& startLinePos);

    /*!
     * Get a location on a cubic bezier curve. t is the position on
     * the curve and must be in range 0 to 1. curveIndex is the index
     * into this->curves.
     */
    QPointF getBezierPos (int curveIndex, float t);

    int srcPos;
    int dstPos;
    drawStyle projDrawStyle;
};

#endif // PROJECTIONS_H
