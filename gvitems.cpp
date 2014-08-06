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
    this->gvc = gvContext();

#ifdef USE_LIBGRAPH_NOT_LIBCGRAPH
    this->gvgraph = agopen((char*)"g", AGDIGRAPH);
#else
    this->gvgraph = agopen((char*)"g", Agdirected, NULL);
#endif

    agsafeset(this->gvgraph, (char*)"splines", (char*)"true", (char*)"");
    agsafeset(this->gvgraph, (char*)"overlap", (char*)"false", (char*)"");
    agsafeset(this->gvgraph, (char*)"rankdir", (char*)"LR", (char*)"");
    agsafeset(this->gvgraph, (char*)"nodesep", (char*)"2.0", (char*)"");
    agsafeset(this->gvgraph, (char*)"labelloc", (char*)"t", (char*)"");
}

GVLayout::~GVLayout()
{
    agclose(this->gvgraph);
    gvFreeContext (this->gvc);
}

void GVLayout::updateLayout()
{
    DBG() << "layout update with " << items.size() << " items";
    gvLayout (this->gvc, this->gvgraph, "dot");
    //update all graphviz items in the layout
    for (uint i=0; i<this->items.size(); i++)
    {
        GVItem *gv_item = this->items[i];
        gv_item->updateLayout();
    }


    // For debugging, this shows the content of the graph (ok for libgraph and libcgraph):
    //gvRender (this->gvc, this->gvgraph, "dot", stdout);

    // The original comment for gvFreeLayout() was:
    // free after update to avoid crashes when node are removed!
    // When gvc is used with cgraph, this actually causes a crash in Graphviz
    // 2.26.3 and 2.30.1, due to bug (in Graphviz) 2.32+ are immune.
    // The crash occurs on a subsequent call to gvLayout().
    // See http://www.graphviz.org/mantisbt/view.php?id=2467
    gvFreeLayout (this->gvc, this->gvgraph);
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
#ifdef USE_LIBGRAPH_NOT_LIBCGRAPH
    this->gv_node = agnode(this->layout->getGVGraph(), name.toUtf8().data());
#else
    this->gv_node = agnode(this->layout->getGVGraph(), name.toUtf8().data(), TRUE);
#endif
    DBG() << "ID for node '" << name << "' is: " << ND_id(this->gv_node);
    agsafeset(this->gv_node, (char*)"fixedsize", (char*)"true", (char*)"");
    agsafeset(this->gv_node, (char*)"shape", (char*)"rectangle", (char*)"");
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
    //DBG() << "setGVNodeSize (" << w << ", " << h << ") in inches for node " << ND_id(this->gv_node);
}

// used in nineml_graphicsitems.cpp:122 or thereabouts.  This method
// is a bit bizarre; I would apply the offset in the client code to
// make this a pure position accessor.
QPointF GVNode::getGVNodePosition(QPointF offset)
{
    QPointF position;
#ifdef USE_LIBGRAPH_NOT_LIBCGRAPH
    position = QPointF((gv_node->u.coord.x), (layout->getGVGraph()->u.bb.UR.y - gv_node->u.coord.y));
#else
    position = QPointF(ND_coord(this->gv_node).x,
                               (GD_bb(this->layout->getGVGraph()).UR.y - ND_coord(this->gv_node).y));
#endif
    //DBG() << "getGVNodePosition: actual position: " << position
    //      << " for node ID " << ND_id(this->gv_node);
    position -= offset;
    return position;
}

#ifdef USE_LIBGRAPH_NOT_LIBCGRAPH
void GVNode::renameGVNode(QString name)
{
    QString temp(gv_node->name);
    agstrfree(gv_node->name);
    gv_node->name = agstrdup(name.toUtf8().data());
}
#endif

/* GVEdge */
GVEdge::GVEdge(GVLayout *l, Agnode_t *src, Agnode_t *dst)
    : GVItem(l)
{
    //get src regime
#ifdef USE_LIBGRAPH_NOT_LIBCGRAPH
    this->gv_edge = agedge(this->layout->getGVGraph(), src, dst);
#else
    this->gv_edge = agedge(this->layout->getGVGraph(), src, dst, NULL, 1);
#endif
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
#ifdef USE_LIBGRAPH_NOT_LIBCGRAPH
    char* html = agstrdup_html(label);
    agsafeset(this->gv_edge, (char*)"label", html, (char*)"html");
    agstrfree(html);
#else
    char* html = agstrdup_html(this->layout->getGVGraph(), label);
    agsafeset(this->gv_edge, (char*)"label", html, (char*)"html");
    agstrfree(this->layout->getGVGraph(), html);
#endif
    //DBG() << "(" << width_pixels<< ", " << height_pixels << ") for label '" << agget (this->gv_edge, (char*)"label") << "'";
}

QPointF GVEdge::getGVEdgeLabelPosition(QPointF offset)
{
    QPointF position(0,0);

#ifdef USE_LIBGRAPH_NOT_LIBCGRAPH
    position = QPointF(this->gv_edge->u.label->pos.x,
                       this->layout->getGVGraph()->u.bb.UR.y - this->gv_edge->u.label->pos.y);
#else
    qreal a = 0;
    qreal b = 0;
    textlabel_t* edgelabel = ED_label(this->gv_edge);
    if (edgelabel != NULL) {
        a = edgelabel->pos.x;
    } else {
        DBG() << "Warning: edge label doesn't exist in this->gv_edge...";
    }
    //char* l = agget (this->gv_edge, (char*)"label");
    b = (GD_bb(this->layout->getGVGraph()).UR.y - ED_label(this->gv_edge)->pos.y);
    position = QPointF(a,b);
#endif

    position -= offset;
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
    return spline;
}

QPointF GVEdge::getGVEdgeSplinesEndPoint()
{
    QPointF spline = QPointF(ED_spl(this->gv_edge)->list->ep.x,
                             GD_bb(this->layout->getGVGraph()).UR.y - ED_spl(this->gv_edge)->list->ep.y);
    return spline;
}
