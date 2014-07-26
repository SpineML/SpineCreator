/*! ** C++ ** */
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
 * NB: Graphviz is used to draw the components in the component view
 * (and not the plots in the graphing/data view window).
 */

#ifndef GVITEMS_H
#define GVITEMS_H

// Probably not necessary with graphviz 2.30+
#define WITH_CGRAPH 1

#include <QtGui>
#include <graphviz/gvc.h>
#include <graphviz/cgraph.h>
#include <vector>
#include <algorithm>
#include "grouptextitems.h"

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
    GVC_t* gv_context;
    vector <GVItem*> items;
};


class GVNode : public GVItem
{
public:
    GVNode(GVLayout *layout, QString name);
    ~GVNode();
    Agnode_t* getGVNode();
    int getId(void);
    void setGVNodeSize(qreal width_inches, qreal height_inches);
    double getHeightPoints (void);
    QPointF getPosition(void);
    QPointF getGVNodePosition(void);
    QPointF getGVNodePosition(QPointF offset);
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
    uint getGVEdgeSplinesCount();
    QPointF getGVEdgeSplinesPoint(uint item);
    QPointF getGVEdgeSplinesEndPoint();
protected:
    Agedge_t *gv_edge;
};

#endif // GVITEMS_H
