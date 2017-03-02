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
#include "NL_projection_and_synapse.h"

class genericInput : public projection // inherit systemObject through projection
{
public:
    genericInput();
    genericInput(QSharedPointer <ComponentInstance> srcCmpt, QSharedPointer <ComponentInstance> dstCmpt, bool projInput = false);
    ~genericInput();

    /*!
     * Returns an identifier for this genericInput connection.
     */
    virtual QString getName();

    virtual void draw(QPainter *painter, float GLscale, float viewX, float viewY, int width, int height, QImage , drawStyle style);

    void delAll(nl_rootdata *);

    void animate(QSharedPointer<systemObject>movingObj, QPointF delta);
    void moveSelectedControlPoint(float xGL, float yGL);
    void write_model_meta_xml(QDomDocument &meta, QDomElement &root) ;

    /*!
     * Read the meta-data information in the QDomDocument* meta. Use
     * cursorPos to offset the positions so that imported networks
     * will appear at the location on the screen of the cursor.
     */
    void read_meta_data(QDomDocument * meta, cursorType cursorPos);

    // Override add_curves from projection.
    void add_curves();

    void connect(QSharedPointer<genericInput> in);
    void disconnect();

    // NB: Very confusing, name clashes
    // QSharedPointer<systemObject> destination(source) in the parent
    // class (projection), which it overrides.
    QSharedPointer<systemObject> destination;
    QSharedPointer<systemObject> source;

    /*!
     * Fixme: Coder will ask "why destination and dst?". These are
     * required as the source and destination objects could be of type
     * synapse and then you don't know if it's the enclosed weight
     * update or postsynapse component that forms the end of the
     * genericinput connection. Refactor to be dstCmpt and srcCmpt.
     */
    //@{
    QSharedPointer <ComponentInstance> dstCmpt;
    QSharedPointer <ComponentInstance> srcCmpt;
    //@}

    QString srcPort;
    QString dstPort;
    bool projInput;
    int srcPos;
    int dstPos;

    connection * conn;

    bool isVisualised;

    QSharedPointer <systemObject> newFromExisting(QMap<systemObject *, QSharedPointer<systemObject> > &objectMap);

    void remapSharedPointers(QMap <systemObject *, QSharedPointer <systemObject> >);
};

#endif // GENERICINPUT_H
