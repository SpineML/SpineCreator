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



/* GVLayout */
GVLayout::GVLayout()
{
    //GV graph
    gvc = gvContext();
    gvgraph = agopen((char*)"g", AGDIGRAPH);
    agsafeset(gvgraph, (char*)"splines", (char*)"true", (char*)"");
    agsafeset(gvgraph, (char*)"overlap", (char*)"false", (char*)"");
    agsafeset(gvgraph, (char*)"rankdir", (char*)"LR", (char*)"");
    agsafeset(gvgraph, (char*)"nodesep", (char*)"2.0", (char*)"");
    agsafeset(gvgraph, (char*)"labelloc", (char*)"t", (char*)"");
}

GVLayout::~GVLayout()
{
    agclose(gvgraph);
    gvFreeContext(gvc);
}

void GVLayout::updateLayout()
{
    //qDebug() << "layout update! " << items.size();
    gvLayout(gvc, gvgraph, "dot");

    //update all graphviz items in the layout
    for (int i=0;i<items.size(); i++)
    {
        GVItem *gv_item = items[i];
        gv_item->updateLayout();
    }

    //gvRender(gvc, gvgraph, "dot", stdout);

    //free after update to avoid crashes when node are removed!
    gvFreeLayout(gvc, gvgraph);
}

Agraph_t *GVLayout::getGVGraph()
{
    return gvgraph;
}

void GVLayout::addGVItem(GVItem *item)
{
    items.push_back(item);
}

void GVLayout::removeGVItem(GVItem *item)
{
    items.erase(std::remove(items.begin(), items.end(), item), items.end());
}

GVItem::GVItem(GVLayout *layout)
{
    this->layout = layout;
    layout->addGVItem(this);
}


/* GVNode */
GVNode::GVNode(GVLayout *layout, QString name)
    : GVItem(layout)
{
    gv_node = agnode(layout->getGVGraph(), name.toUtf8().data());
    agsafeset(gv_node, (char*)"fixedsize", (char*)"true", (char*)"");
    agsafeset(gv_node, (char*)"shape", (char*)"rectangle", (char*)"");

}

GVNode::~GVNode()
{
    agdelete(layout->getGVGraph(), gv_node);
}

Agnode_t * GVNode::getGVNode()
{
    return gv_node;
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

QPointF GVNode::getGVNodePosition(QPointF offset)
{
    QPointF position;
    position = QPointF((gv_node->u.coord.x), (layout->getGVGraph()->u.bb.UR.y - gv_node->u.coord.y));
    position -= offset;
    return position;
}

void GVNode::renameGVNode(QString name)
{
    QString temp(gv_node->name);
    agstrfree(gv_node->name);
    gv_node->name = agstrdup(name.toUtf8().data());
}


/* GVEdge */
GVEdge::GVEdge(GVLayout *layout, Agnode_t *src, Agnode_t *dst)
    : GVItem(layout)
{
    //get src regime
    gv_edge = agedge(layout->getGVGraph(), src, dst);
}

GVEdge::~GVEdge()
{
    agdelete(layout->getGVGraph(), gv_edge);
}


void GVEdge::setGVEdgeLabelSize(int width_pixels, int height_pixels)
{
    //update GV l width
    char label[256];
    sprintf(label,"<table width=\"%d\" height=\"%d\"><tr><td>Label</td></tr></table>", width_pixels, height_pixels);
    char* html = agstrdup_html(label);
    agsafeset(gv_edge, (char*)"label", html, (char*)"html");
    agstrfree(html);
    //qDebug() << "GVEdge setGVEdgeLabelSize (" << width_pixels<< ", " << height_pixels << ")";
}

QPointF GVEdge::getGVEdgeLabelPosition(QPointF offset)
{
    QPointF position;
    position = QPointF(gv_edge->u.label->pos.x, layout->getGVGraph()->u.bb.UR.y - gv_edge->u.label->pos.y);
    position -= offset;
    return position;
}

int GVEdge::getGVEdgeSplinesCount()
{
    return gv_edge->u.spl->list->size;
}

QPointF GVEdge::getGVEdgeSplinesPoint(int i)
{
    QPointF spline = QPointF(gv_edge->u.spl->list->list[i].x, layout->getGVGraph()->u.bb.UR.y - gv_edge->u.spl->list->list[i].y);
    return spline;
}

QPointF GVEdge::getGVEdgeSplinesEndPoint()
{
    QPointF spline = QPointF(gv_edge->u.spl->list->ep.x, layout->getGVGraph()->u.bb.UR.y - gv_edge->u.spl->list->ep.y);
    return spline;
}









