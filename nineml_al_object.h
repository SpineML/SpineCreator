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

#ifndef NINEML_AL_OBJECT_H
#define NINEML_AL_OBJECT_H

//#include "nineML_classes.h"
//#include <sstream>

//QString QString::number(float);


// DEPRECATED - see nineML_classes.h/.cpp ComponentClass
/*
class nineml_al_object
{
public:
    nineml_al_object();
  //  nineml_al_object(nineml_al_object & src);
    ~nineml_al_object();

    QString name;
    vector < Regime > regimeList;
    vector < StateVariable > StateVariableList;
    vector < Parameter > ParameterList;
    vector < Alias > AliasList;
    vector < AnalogPort > analogPortList;
    vector < EventPort > eventPortList;

    void load(QDomDocument);
    void write_node_xml(QDomElement &node, QDomDocument &doc);

};
*/


#endif // NINEML_AL_OBJECT_H
