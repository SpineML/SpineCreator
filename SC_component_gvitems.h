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

/*
 * NB: Graphviz is used to lay out the components in the component
 * view (and not the plots in the graphing/data view window).
 *
 * The actual rendering of the components in the component view is
 * handled by Qt code - see GroupedTextItem::paint.
 */

#ifndef GVITEMS_H
#define GVITEMS_H

#include <QtGui>
// By default, use libcgraph from Graphviz, but if the user requests,
// use the deprecated libgraph.
#include <graphviz/gvc.h>
#ifdef USE_LIBGRAPH_NOT_LIBCGRAPH
# include <graphviz/graph.h>
#else
# define WITH_CGRAPH 1
# include <graphviz/cgraph.h>
#endif
#include <vector>
#include <algorithm>
#include "SC_component_grouptextitems.h"

using namespace std;

#define GV_DPI 72.0

class GVLayout;

class GVItem
{
public:
    GVItem(GVLayout *l);
    virtual void updateLayout() = 0;
    virtual void updateGVData() = 0;

protected:
    GVLayout *layout;
};


class GVLayout
{
public:
    GVLayout();
    ~GVLayout();
    void updateLayout();
    Agraph_t *getGVGraph();
    void addGVItem(GVItem *item);
    void removeGVItem(GVItem *item);
private:
    Agraph_t *gvgraph;
    GVC_t* gvc;
    QVector <GVItem*> items;
};


class GVNode : public GVItem
{
public:
    GVNode(GVLayout *layout, QString name);
    ~GVNode();
    Agnode_t* getGVNode();
    void setGVNodeSize(qreal width_inches, qreal height_inches);
    QPointF getGVNodePosition(QPointF offset);
#ifdef USE_LIBGRAPH_NOT_LIBCGRAPH
    void renameGVNode(QString name);
#endif
protected:
    Agnode_t *gv_node;
};


class GVEdge :  public GVItem
{
public:
    GVEdge(GVLayout *layout, Agnode_t *src, Agnode_t *dst);
    ~GVEdge();
    void setGVEdgeLabelSize(int width_pixels, int height_pixels);
    QPointF getGVEdgeLabelPosition(QPointF offset);
    int getGVEdgeSplinesCount();
    QPointF getGVEdgeSplinesPoint(int item);
    QPointF getGVEdgeSplinesEndPoint();
protected:
    Agedge_t *gv_edge;
};

#endif // GVITEMS_H
