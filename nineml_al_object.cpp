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

#include "nineml_al_object.h"

// DEPRECATED
/*
QString QString::number(float val) {
    stringstream ss (stringstream::in | stringstream::out);
    ss << float(val);
    QString returnVal = ss.str().c_str();
    return returnVal;
}

nineml_al_object::nineml_al_object()
{
}


nineml_al_object::~nineml_al_object()
{
}

void nineml_al_object::load(QDomDocument doc)
{
    QDomNode n = doc.documentElement().firstChild();
    while( !n.isNull() )
    {
        QDomElement e = n.toElement();
        if( e.tagName() == "ComponentClass" )
        {
            this->name = e.attribute("name","");
            QDomNode n2 = e.firstChild();
            while( !n2.isNull() )
            {
                QDomElement e2 = n2.toElement();
                if( e2.tagName() == "Parameter" )
                {
                    Parameter *tempPar = new Parameter;
                    tempPar->readIn(e2);
                    this->ParameterList.push_back(*tempPar);
                }
                if( e2.tagName() == "Dynamics" )
                {
                    QDomNode n3 = e2.firstChild();
                    while( !n3.isNull() )
                    {
                        QDomElement e3 = n3.toElement();
                        if( e3.tagName() == "Regime" )
                        {
                            Regime *tempRegime  = new Regime;
                            tempRegime->readIn(e3);
                            this->regimeList.push_back(*tempRegime);
                        }
                        if( e3.tagName() == "StateVariable" )
                        {
                            StateVariable *tempSV  = new StateVariable;
                            tempSV->readIn(e3);
                            this->StateVariableList.push_back(*tempSV);
                        }
                        if( e3.tagName() == "Alias" )
                        {
                            Alias *tempAlias  = new Alias;
                            tempAlias->readIn(e3);
                            this->AliasList.push_back(*tempAlias);
                        }

                        n3 = n3.nextSibling();
                    }
                }
                if( e2.tagName() == "AnalogPort" )
                {
                    AnalogPort *tempAP = new AnalogPort;
                    tempAP->readIn(e2);
                    this->analogPortList.push_back(*tempAP);
                }
                if( e2.tagName() == "EventPort" )
                {
                    EventPort *tempEP = new EventPort;
                    tempEP->readIn(e2);
                    this->eventPortList.push_back(*tempEP);

                }
                n2 = n2.nextSibling();
            }

        }
        n = n.nextSibling();
    }*/
    // Read out to check what we have:
    /*for (int i = 0; i < this->regimeList.size(); ++i) {
        for (int j = 0; j < this->regimeList[i].TimeDerivativeList.size(); ++j) {
            QString test = this->regimeList[i].TimeDerivativeList[j].name;
            cout << "moo " << test.toStdString() << "\n";
        }
    }*/
/*
}

void nineml_al_object::write_node_xml(QDomElement &node, QDomDocument &doc) {
    QDomElement def = doc.createElement( "definition" );
    QDomElement url = doc.createElement( "url" );
    QDomText text;

    // strip out whitespace from the url & replace space with _ so we can get a nice file
    QString simpleName = this->name.simplified();
    simpleName.replace( " ", "_" );
    text = doc.createTextNode(simpleName + ".xml");
    url.appendChild(text);
    def.appendChild(url);
    node.appendChild(def);

    QDomElement prop = doc.createElement( "properties" );


    for (unsigned int i = 0; i < this->ParameterList.size(); ++i) {
        QDomElement par = doc.createElement( this->ParameterList[i].name );
        QDomElement val = doc.createElement( "value" );
        text = doc.createTextNode(QString::number(this->ParameterList[i].value));
        val.appendChild(text);
        par.appendChild(val);
        QDomElement unit = doc.createElement( "unit" );
        text = doc.createTextNode("ms");
        unit.appendChild(text);
        par.appendChild(unit);
        prop.appendChild(par);

        //unit.setNodeValue(stringify(this->ParameterList[i].unit));
    }
    for (unsigned int i = 0; i < this->StateVariableList.size(); ++i) {
        QDomElement par = doc.createElement( this->StateVariableList[i].name );
        QDomElement val = doc.createElement( "value" );
        text = doc.createTextNode(QString::number(this->StateVariableList[i].initial_val));
        val.appendChild(text);
        par.appendChild(val);
        QDomElement unit = doc.createElement( "unit" );
        text = doc.createTextNode("ms");
        unit.appendChild(text);
        par.appendChild(unit);
        prop.appendChild(par);
        //unit.setNodeValue(stringify(this->ParameterList[i].unit));
    }
    for (unsigned int i = 0; i < this->AliasList.size(); ++i) {
        QDomElement par = doc.createElement( this->AliasList[i].name );
        QDomElement val = doc.createElement( "value" );
        text = doc.createTextNode(QString::number(this->AliasList[i].initial_val));
        val.appendChild(text);
        par.appendChild(val);
        QDomElement unit = doc.createElement( "unit" );
        text = doc.createTextNode("ms");
        unit.appendChild(text);
        par.appendChild(unit);
        prop.appendChild(par);
        //unit.setNodeValue(stringify(this->ParameterList[i].unit));
    }

    node.appendChild(prop);

}
*/
