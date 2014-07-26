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
**           Author: Paul Richmond                                        **
**  Website/Contact: http://bimpa.group.shef.ac.uk/                       **
****************************************************************************/

// graphviz/types.h provides node/graph/edge data access macros
#define WITH_CGRAPH 1
//#include <graphviz/types.h>

#include "gvitems.h"
#include <sstream>

// Debugging define here, as we don't include globalHeader.h
#define DBG() qDebug() << __FUNCTION__ << ": "

/* GVLayout */
GVLayout::GVLayout()
{
    //GV graph
    this->gvgraph = agopen((char*)"g", Agdirected, NULL);
    agsafeset(this->gvgraph, (char*)"splines", (char*)"true", (char*)"");
    agsafeset(this->gvgraph, (char*)"overlap", (char*)"false", (char*)"");
    agsafeset(this->gvgraph, (char*)"rankdir", (char*)"LR", (char*)"");
    agsafeset(this->gvgraph, (char*)"nodesep", (char*)"2.0", (char*)"");
    agsafeset(this->gvgraph, (char*)"labelloc", (char*)"t", (char*)"");
}

GVLayout::~GVLayout()
{
    agclose(this->gvgraph);
}

void GVLayout::updateLayout()
{
    DBG() << "layout update with " << items.size() << " items";
    //update all graphviz items in the layout
    for (uint i=0;i<this->items.size(); i++)
    {
        GVItem *gv_item = this->items[i];
        DBG() << "update layout on an item...";
        gv_item->updateLayout();
    }


    // Debugging:
    agwrite (this->gvgraph, stdout);
}

Agraph_t *GVLayout::getGVGraph()
{
    return this->gvgraph;
}

void GVLayout::addGVItem(GVItem *item)
{
    items.push_back(item);
}

void GVLayout::removeGVItem(GVItem *item)
{
    items.erase(std::remove(items.begin(), items.end(), item), items.end());
}

GVItem::GVItem(GVLayout *l)
{
    this->layout = l;
    this->layout->addGVItem(this);
}


/* GVNode */
GVNode::GVNode(GVLayout *l, QString name, const QPointF& initialPos)
    : GVItem(l)
{
    DBG() << "GVNode constructor for GVNode name " << name << "...";
    this->gv_node = agnode(this->layout->getGVGraph(), name.toUtf8().data(), TRUE); // TRUE means create if not existent
    DBG() << "ID for " << name << " is: " << ND_id(this->gv_node);
    agsafeset(this->gv_node, (char*)"fixedsize", (char*)"true", (char*)"");
    agsafeset(this->gv_node, (char*)"shape", (char*)"rectangle", (char*)"");
    stringstream ss;
    ss << initialPos.x() << "," << initialPos.y();
    agsafeset(this->gv_node, (char*)"pos", (char*)ss.str().c_str(), (char*)"");
}

GVNode::~GVNode()
{
    DBG() << "Delete node ID " << ND_id(this->gv_node);
    agdelete(this->layout->getGVGraph(), this->gv_node);
}

Agnode_t * GVNode::getGVNode()
{
    return this->gv_node;
}

void GVNode::setGVNodeSize(qreal width_inches, qreal height_inches)
{
    //update GV node size
    char w[16];
    char h[16];
    sprintf(w,"%f", width_inches);
    sprintf(h,"%f", height_inches);
    agsafeset(this->gv_node, (char*)"width", w, (char*)"");
    agsafeset(this->gv_node, (char*)"height", h, (char*)"");
    DBG() << "setGVNodeSize (" << w << ", " << h << ") in inches for node " << ND_id(this->gv_node);
}

// This is not the right way to go about things!
void GVNode::setGVNodePosition (const QPointF& position)
{
    // Need to use agsafeset here somehow to set x & y.
    ND_coord(this->gv_node).x = position.x();
    ND_coord(this->gv_node).y = position.y();
}

QPointF GVNode::getGVNodePosition(void)
{
    return this->weirdStoredPosition;
}

// used once only in nineml_graphicsitems.cpp:122
// This doesn't actually _change_ the position. Also, why use that offset?
QPointF GVNode::getGVNodePosition(QPointF offset)
{
    DBG() << "offset: " << offset;
    QPointF position = QPointF(ND_coord(this->gv_node).x,
                               (GD_bb(this->layout->getGVGraph()).UR.y - ND_coord(this->gv_node).y));
    DBG() << "getGVNodePosition: actual position: " << position
          << " for node ID " << ND_id(this->gv_node);
    position -= offset;
    DBG() << "getGVNodePosition: returning: " << position*2
          << " for node ID " << ND_id(this->gv_node);
    this->weirdStoredPosition = position*2;
    return this->weirdStoredPosition;
}

int GVNode::getId(void)
{
    return ND_id(this->gv_node);
}

double GVNode::getHeightPoints (void)
{
    char* h = agget (this->gv_node, (char*)"height");
    stringstream hh;
    hh << h;
    double h_inches = 0.0;
    hh >> h_inches;
    return h_inches*GV_DPI;
}

QPointF GVNode::getPosition(void)
{
    // Fixme: Prolly don't need storedPosition class attribute.
    this->storedPosition = QPointF(ND_coord(this->gv_node).x,
                             (GD_bb(this->layout->getGVGraph()).UR.y - ND_coord(this->gv_node).y));

    DBG() << "getPosition: actual position: " << this->storedPosition
          << " for node ID " << ND_id(this->gv_node);

    DBG() << "getPosition: agget pos attr: '" << agget (this->gv_node, (char*)"pos") << "'";

    return this->storedPosition;
}

/* GVEdge */
GVEdge::GVEdge(GVLayout *l, Agnode_t *src, Agnode_t *dst)
    : GVItem(l)
{
    // get src regime
    this->gv_edge = agedge(this->layout->getGVGraph(), src, dst, NULL, 1);
    // Ensure the edge has a label:
    // char* s = "default label";
    // agset (this->gv_edge, "label", agstrdup (this->layout->getGVGraph(),s));
}

GVEdge::~GVEdge()
{
    agdelete(this->layout->getGVGraph(), this->gv_edge);
}


void GVEdge::setGVEdgeLabelSize(int width_pixels, int height_pixels)
{
    // update GV l width
    char label[256];
    sprintf(label,"<table width=\"%d\" height=\"%d\"><tr><td>Label</td></tr></table>", width_pixels, height_pixels);
    char* html = agstrdup_html(this->layout->getGVGraph(), label);
    agsafeset(this->gv_edge, (char*)"label", html, (char*)"html");
    agstrfree(this->layout->getGVGraph(), html);

    DBG() << "(" << width_pixels<< ", " << height_pixels << ") for label '" << agget (this->gv_edge, (char*)"label") << "'";
}

QPointF GVEdge::getGVEdgeLabelPosition(QPointF offset)
{
    QPointF position(0,0);
#if 0
    qreal a = 0;
    qreal b = 0;
    // This crashes as the edge label doesn't have a position OR the
    // edge doesn't even have a label..
    a = ED_label(this->gv_edge)->pos.x;

    char* l = agget (this->gv_edge, (char*)"label");

    b = (GD_bb(this->layout->getGVGraph()).UR.y - ED_label(this->gv_edge)->pos.y);
    position = QPointF(a,b);

#endif
    DBG() << position << " before";
    position -= offset;
    DBG() << position << " after";
    return position;
}

uint GVEdge::getGVEdgeSplinesCount()
{
    return ED_spl(this->gv_edge)->list->size;
}

QPointF GVEdge::getGVEdgeSplinesPoint(uint i)
{
    QPointF spline = QPointF(ED_spl(this->gv_edge)->list->list[i].x,
                             GD_bb(this->layout->getGVGraph()).UR.y - ED_spl(this->gv_edge)->list->list[i].y);
    DBG() << spline;
    return spline;
}

// Used once only in ./nineml_graphicsitems.cpp:201
QPointF GVEdge::getGVEdgeSplinesEndPoint()
{
    QPointF spline = QPointF(ED_spl(this->gv_edge)->list->ep.x,
                             GD_bb(this->layout->getGVGraph()).UR.y - ED_spl(this->gv_edge)->list->ep.y);

    DBG() << spline;
    return spline;
}
