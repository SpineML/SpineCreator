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

#ifndef SYSTEMOBJECT_H
#define SYSTEMOBJECT_H

#include "globalHeader.h"

enum systemObjectType {

    populationObject,
    projectionObject,
    synapseObject,
    inputObject,
    nullObject

};

class systemObject
{
public:

    virtual QString getName() { return "oops";}

    virtual void move(float, float) {}
    virtual void draw(QPainter *, float, float, float, int, int, QImage, drawStyle){}
    virtual bool is_clicked(float, float, float) {return true;}
    virtual void remove(rootData *) {delete this;}
    virtual void delAll(rootData *) {delete this;}

    virtual void setRelativeLocation(float x, float y)

    {

        relativeLocation =  currentLocation() - QPointF(x,y);

    }

    virtual QPointF currentLocation() {
        return QPointF(0,0);
    }

    systemObjectType type;
    int tag;
    bool connected;
    bool isDeleted;
    systemObject();
    virtual ~systemObject() {}

    QPointF relativeLocation;

};

#endif // SYSTEMOBJECT_H
