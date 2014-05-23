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

#include "gvitems.h"

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
        gv_item->updateLayout();
    }
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
GVNode::GVNode(GVLayout *l, QString name)
    : GVItem(l)
{
    this->gv_node = agnode(this->layout->getGVGraph(), name.toUtf8().data(), TRUE); // TRUE means create if not existent
    agsafeset(this->gv_node, (char*)"fixedsize", (char*)"true", (char*)"");
    agsafeset(this->gv_node, (char*)"shape", (char*)"rectangle", (char*)"");
}

GVNode::~GVNode()
{
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
    agsafeset(gv_node, (char*)"width", w, (char*)"");
    agsafeset(gv_node, (char*)"height", h, (char*)"");
    //qDebug() << "GVNode setGVNodeSize(" << w << ", " << h << ")";
}

// used once only in nineml_graphicsitems.cpp:122
QPointF GVNode::getGVNodePosition(QPointF offset)
{
    DBG() << "Called. Warning - this is a no-op!";
    QPointF position(0,0);
#if 0
    position = QPointF((this->gv_node->u.coord.x),
                       (this->layout->getGVGraph()->u.bb.UR.y - gv_node->u.coord.y));
    position -= offset;
#endif
    return position;
}

// Is the the right approach?
void GVNode::renameGVNode(QString name)
{
    QString temp(this->gv_node->base.data->name);
    agstrfree(this->layout->getGVGraph(), this->gv_node->base.data->name);
    this->gv_node->base.data->name = agstrdup(this->layout->getGVGraph(), name.toUtf8().data());
}


/* GVEdge */
GVEdge::GVEdge(GVLayout *l, Agnode_t *src, Agnode_t *dst)
    : GVItem(l)
{
    // get src regime
    this->gv_edge = agedge(this->layout->getGVGraph(), src, dst, NULL, 1);
}

GVEdge::~GVEdge()
{
    agdelete(this->layout->getGVGraph(), this->gv_edge);
}


void GVEdge::setGVEdgeLabelSize(int width_pixels, int height_pixels)
{
    //update GV l width
    char label[256];
    sprintf(label,"<table width=\"%d\" height=\"%d\"><tr><td>Label</td></tr></table>", width_pixels, height_pixels);
    char* html = agstrdup_html(this->layout->getGVGraph(), label);
    agsafeset(this->gv_edge, (char*)"label", html, (char*)"html");
    agstrfree(this->layout->getGVGraph(), html);
    //qDebug() << "GVEdge setGVEdgeLabelSize (" << width_pixels<< ", " << height_pixels << ")";
}

// Used only once in ./nineml_graphicsitems.cpp:186
QPointF GVEdge::getGVEdgeLabelPosition(QPointF offset)
{
    DBG() << "Called. Warning - this is a no-op!";
    QPointF position(0,0);
#if 0
    position = QPointF(this->gv_edge->u.label->pos.x,
                       this->layout->getGVGraph()->u.bb.UR.y - this->gv_edge->u.label->pos.y);
    position -= offset;
#endif
    return position;
}

// Used only once in ./nineml_graphicsitems.cpp:194
uint GVEdge::getGVEdgeSplinesCount()
{
    DBG() << "Called. Warning - no-op";
#if 0
    return this->gv_edge->u.spl->list->size;
#else
    return 0;
#endif
}

// Used a few times in /nineml_graphicsitems.cpp
QPointF GVEdge::getGVEdgeSplinesPoint(uint i)
{
    DBG() << "Called. Warning - no-op!";
#if 0
    QPointF spline = QPointF(this->gv_edge->u.spl->list->list[i].x,
                             this->layout->getGVGraph()->u.bb.UR.y - this->gv_edge->u.spl->list->list[i].y);
#endif
    QPointF spline(0,0);
    return spline;
}

// Used once only in ./nineml_graphicsitems.cpp:201
QPointF GVEdge::getGVEdgeSplinesEndPoint()
{
    DBG() << "Called. Warning - no-op!";
#if 0
    QPointF spline = QPointF(this->gv_edge->u.spl->list->ep.x, this->layout->getGVGraph()->u.bb.UR.y - gv_edge->u.spl->list->ep.y);
#endif
    QPointF spline(0,0);
    return spline;
}
