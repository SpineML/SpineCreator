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

#ifndef GENERICINPUT_H
#define GENERICINPUT_H

#include "globalHeader.h"
#include "projections.h"

class genericInput : public projection // inherit systemObject through projection
{
public:
    genericInput();
    genericInput(QSharedPointer <NineMLComponentData> src, QSharedPointer <NineMLComponentData> dst, bool projInput = false);
    ~genericInput();

    virtual QString getName();
    virtual void draw(QPainter *painter, float GLscale, float viewX, float viewY, int width, int height, QImage , drawStyle style);
    /*virtual void move(float, float);
    virtual void draw(QPainter, float, float, float, int, int, QImage);
    virtual bool is_clicked(float, float, float);*/
    void remove(rootData *);
    void delAll(rootData *);

    void animate(QSharedPointer<systemObject>movingObj, QPointF delta);
    void moveSelectedControlPoint(float xGL, float yGL);
    void write_model_meta_xml(QDomDocument &meta, QDomElement &root) ;

    void read_meta_data(QDomDocument * meta);

    void addCurves();
    void connect(QSharedPointer<genericInput> in);
    void disconnect();

    QSharedPointer<systemObject> destination;
    QSharedPointer<systemObject> source;

    QSharedPointer <NineMLComponentData> src;
    QSharedPointer <NineMLComponentData> dst;
    QString srcPort;
    QString dstPort;
    bool projInput;
    int srcPos;
    int dstPos;

    connection * connectionType;

    bool isVisualised;


};

#endif // GENERICINPUT_H
