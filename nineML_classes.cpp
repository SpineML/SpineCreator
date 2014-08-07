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
**           Author: Alex Cope, Paul Richmond                             **
**  Website/Contact: http://bimpa.group.shef.ac.uk/                       **
****************************************************************************/


#include "nineML_classes.h"
#include "projections.h"
#include "nineml_layout_classes.h"
#include "genericinput.h"
#include "population.h"

QString dim::toString() {
    // do stuff

    QString unit;
    QString prefix;

    unit = getUnitString();
    prefix = getPrefixString();
    unit = prefix+unit;

    return unit;
}

void dim::fromString(QString in) {
    // do stuff

    if (in.size() == 0) return;

    if (in.size() > 1 && in.startsWith("G"))
        {scale = 9; in.remove(0,1);}
    if (in.size() > 1 && in.startsWith("M"))
        {scale = 6; in.remove(0,1);}
    if (in.size() > 1 && in.startsWith("k"))
        {scale = 3; in.remove(0,1);}
    if (in.size() > 1 && in.startsWith("c") && in != "cd")
        {scale = 2; in.remove(0,1);}
    if (in.size() > 1 && in.startsWith("m") && in != "mol")
        {scale = -3; in.remove(0,1);}
    if (in.size() > 1 && in.startsWith("u"))
        {scale = -6; in.remove(0,1);}
    if (in.size() > 1 && in.startsWith("n"))
        {scale = -9; in.remove(0,1);}
    if (in.size() > 1 && in.startsWith("p"))
        {scale = -12; in.remove(0,1);}
    if (in.size() > 1 && in.startsWith("f"))
        {scale = -15; in.remove(0,1);}

    if (in == "V") {
        m=1; l=2; t=-3; I=-1; Cd = 0; mol = 0; temp = 0;}
    if (in == "Ohm") {
        m=1; l=2; t=-3; I=-2; Cd = 0; mol = 0; temp = 0;}
    if (in == "g") {
        m=1; l=0; t=0; I=0; Cd = 0; mol = 0; temp = 0;}
    if (in == "m") {
        m=0; l=1; t=0; I=0; Cd = 0; mol = 0; temp = 0;}
    if (in == "s") {
        m=0; l=0; t=1; I=0; Cd = 0; mol = 0; temp = 0;}
    if (in == "A") {
        m=0; l=0; t=0; I=1; Cd = 0; mol = 0; temp = 0;}
    if (in == "cd") {
        m=0; l=0; t=0; I=0; Cd = 1; mol = 0; temp = 0;}
    if (in == "mol") {
        m=0; l=0; t=0; I=0; Cd = 0; mol = 1; temp = 0;}
    if (in == "degC") {
        m=0; l=0; t=0; I=0; Cd = 0; mol = 0; temp = 1;}
    if (in == "S") {
        m=-1; l=-2; t=3; I=2; Cd = 0; mol = 0; temp = 0;}
    if (in == "F") {
        m=-1; l=-2; t=4; I=2; Cd = 0; mol = 0; temp = 0;}
    if (in == "Hz") {
        m=0; l=0; t=-1; I=0; Cd = 0; mol = 0; temp = 0;}
    if ((in == "?")||(in == "")) {
        m=0; l=0; t=0; I=0; Cd = 0; mol = 0; temp = 0;}
}

void dim::setPrefix(QString p)
{
    if (p == "")
        scale = 0;
    else if (p == "G")
        scale = 9;
    else if (p == "M")
        scale = 6;
    else if (p == "k")
        scale = 3;
    else if (p == "c")
        scale = 2;
    else if (p == "m")
        scale = -3;
    else if (p == "u")
        scale = -6;
    else if (p == "n")
        scale = -9;
    else if (p == "p")
        scale = -12;
    else if (p == "f")
        scale = -15;
}

void dim::setUnit(QString u)
{
    if (u == "V") {
        m=1; l=2; t=-3; I=-1; Cd = 0; mol = 0; temp = 0;}
    else if (u == "Ohm") {
        m=1; l=2; t=-3; I=-2; Cd = 0; mol = 0; temp = 0;}
    else if (u == "g") {
        m=1; l=0; t=0; I=0; Cd = 0; mol = 0; temp = 0;}
    else if (u == "m") {
        m=0; l=1; t=0; I=0; Cd = 0; mol = 0; temp = 0;}
    else if (u == "s") {
        m=0; l=0; t=1; I=0; Cd = 0; mol = 0; temp = 0;}
    else if (u == "A") {
        m=0; l=0; t=0; I=1; Cd = 0; mol = 0; temp = 0;}
    else if (u == "cd") {
        m=0; l=0; t=0; I=0; Cd = 1; mol = 0; temp = 0;}
    else if (u == "mol") {
        m=0; l=0; t=0; I=0; Cd = 0; mol = 1; temp = 0;}
    else if (u == "degC") {
        m=0; l=0; t=0; I=0; Cd = 0; mol = 0; temp = 1;}
    else if (u == "S") {
        m=-1; l=-2; t=3; I=2; Cd = 0; mol = 0; temp = 0;}
    else if (u == "F") {
        m=-1; l=-2; t=4; I=2; Cd = 0; mol = 0; temp = 0;}
    else if (u == "Hz") {
        m=0; l=0; t=-1; I=0; Cd = 0; mol = 0; temp = 0;}
    else{
        m=0; l=0; t=0; I=0; Cd = 0; mol = 0; temp = 0;}
}

QString dim::getPrefixString()
{
    QString prefix;

    if (scale==9)
        prefix = "G";
    else if (scale==6)
        prefix = "M";
    else if (scale==3)
        prefix = "k";
    else if (scale==2)
        prefix = "c";
    else if (scale==-3)
        prefix = "m";
    else if (scale==-6)
        prefix = "u";
    else if (scale==-9)
        prefix = "n";
    else if (scale==-12)
        prefix = "p";
    else if (scale==-15)
        prefix = "f";
    else
        prefix = "";

    return prefix;
}

QString dim::getUnitString()
{
    QString unit;

    if (m==1 && l==2 && t==-3 && I==-1 && Cd == 0 && mol == 0 && temp == 0)
        unit = "V";
    else if (m==1 && l==2 && t==-3 && I==-2 && Cd == 0 && mol == 0 && temp == 0)
        unit = "Ohm";
    else if (m==1 && l==0 && t==0 && I==0 && Cd == 0 && mol == 0 && temp == 0)
        unit = "g";
    else if (m==0 && l==1 && t==0 && I==0 && Cd == 0 && mol == 0 && temp == 0)
        unit = "m";
    else if (m==0 && l==0 && t==1 && I==0 && Cd == 0 && mol == 0 && temp == 0)
        unit = "s";
    else if (m==0 && l==0 && t==0 && I==1 && Cd == 0 && mol == 0 && temp == 0)
        unit = "A";
    else if (m==0 && l==0 && t==0 && I==0 && Cd == 1 && mol == 0 && temp == 0)
        unit = "cd";
    else if (m==0 && l==0 && t==0 && I==0 && Cd == 0 && mol == 1 && temp == 0)
        unit = "mol";
    else if (m==0 && l==0 && t==0 && I==0 && Cd == 0 && mol == 0 && temp == 1)
        unit = "degC";
    else if (m==-1 && l==-2 && t==3 && I==2 && Cd == 0 && mol == 0 && temp == 0)
        unit = "S";
    else if (m==-1 && l==-2 && t==4 && I==2 && Cd == 0 && mol == 0 && temp == 0)
        unit = "F";
    else if (m==0 && l==0 && t==-1 && I==0 && Cd == 0 && mol == 0 && temp == 0)
        unit = "Hz";
    // default for dimensionless
    //if (m==0 && l==0 && t==0 && I==0 && Cd == 0 && mol == 0 && temp == 0)
    else
        unit = "?";

    return unit;

}

Prefix dim::getPrefix()
{
    if (scale==9)
        return PREFIX_G;
    else if (scale==6)
        return PREFIX_M;
    else if (scale==3)
        return PREFIX_k;
    else if (scale==2)
        return PREFIX_c;
    else if (scale==-3)
        return PREFIX_m;
    else if (scale==-6)
        return PREFIX_u;
    else if (scale==-9)
        return PREFIX_n;
    else if (scale==-12)
        return PREFIX_p;
    else if (scale==-15)
        return PREFIX_f;
    else
        return PREFIX_NONE;
}

Unit dim::getUnit()
{
    if (m==1 && l==2 && t==-3 && I==-1 && Cd == 0 && mol == 0 && temp == 0)
        return UNIT_V;
    else if (m==1 && l==2 && t==-3 && I==-2 && Cd == 0 && mol == 0 && temp == 0)
        return UNIT_Ohm;
    else if (m==1 && l==0 && t==0 && I==0 && Cd == 0 && mol == 0 && temp == 0)
        return UNIT_g;
    else if (m==0 && l==1 && t==0 && I==0 && Cd == 0 && mol == 0 && temp == 0)
        return UNIT_m;
    else if (m==0 && l==0 && t==1 && I==0 && Cd == 0 && mol == 0 && temp == 0)
        return UNIT_s;
    else if (m==0 && l==0 && t==0 && I==1 && Cd == 0 && mol == 0 && temp == 0)
        return UNIT_A;
    else if (m==0 && l==0 && t==0 && I==0 && Cd == 1 && mol == 0 && temp == 0)
        return UNIT_cd;
    else if (m==0 && l==0 && t==0 && I==0 && Cd == 0 && mol == 1 && temp == 0)
        return UNIT_mol;
    else if (m==0 && l==0 && t==0 && I==0 && Cd == 0 && mol == 0 && temp == 1)
        return UNIT_degC;
    else if (m==-1 && l==-2 && t==3 && I==2 && Cd == 0 && mol == 0 && temp == 0)
        return UNIT_S;
    else if (m==-1 && l==-2 && t==4 && I==2 && Cd == 0 && mol == 0 && temp == 0)
        return UNIT_F;
    else if (m==0 && l==0 && t==-1 && I==0 && Cd == 0 && mol == 0 && temp == 0)
        return UNIT_Hz;
    // default for dimensionless
    //if (m==0 && l==0 && t==0 && I==0 && Cd == 0 && mol == 0 && temp == 0)
    else
        return UNIT_NONE;
}

void dim::reset()
{
    m = 0;
    l = 0;
    t = 0;
    I = 0;
    Cd = 0;
    mol = 0;
    temp = 0;
    scale = 1;
}

bool operator==(dim &dim1, dim &dim2)
{
    return ((dim1.I==dim2.I)&&(dim1.l==dim2.l)&&(dim1.m==dim2.m)&&(dim1.mol==dim2.mol)&&(dim1.scale==dim2.scale)&&(dim1.t==dim2.t)&&(dim1.temp==dim2.temp));
}

dim::dim(QString str) {

    m = 0;
    l = 0;
    t = 0;
    I = 0;
    Cd = 0;
    mol = 0;
    temp = 0;
    scale = 1;

    if (str != "")
        this->fromString(str);

}


void NineMLComponent::load(QDomDocument *doc)
{
    QDomNode n = doc->documentElement().firstChild();
    while( !n.isNull() )
    {
        QDomElement e = n.toElement();
        if( e.tagName() == "ComponentClass" )
        {
            this->name = e.attribute("name","");

            if (this->name =="") {
                QSettings settings;
                int num_errs = settings.beginReadArray("errors");
                settings.endArray();
                settings.beginWriteArray("errors");
                    settings.setArrayIndex(num_errs + 1);
                    settings.setValue("errorText",  "XML error: expected 'name' attribute'");
                settings.endArray();
            }

            // default to unsorted if no type found
            this->type = e.attribute("type", "");
            QDomNode n2 = e.firstChild();
            while( !n2.isNull() )
            {
                QDomElement e2 = n2.toElement();
                if( e2.tagName() == "Parameter" )
                {
                    Parameter *tempPar = new Parameter;
                    tempPar->readIn(e2);
                    this->ParameterList.push_back(tempPar);
                }
                else if( e2.tagName() == "Dynamics" )
                {
                    this->initial_regime_name = e2.attribute("initial_regime");
                    QDomNode n3 = e2.firstChild();
                    while( !n3.isNull() )
                    {
                        QDomElement e3 = n3.toElement();
                        if( e3.tagName() == "Regime" )
                        {
                            Regime *tempRegime  = new Regime;
                            tempRegime->readIn(e3);
                            this->RegimeList.push_back(tempRegime);
                        }
                        else if( e3.tagName() == "StateVariable" )
                        {
                            StateVariable *tempSV  = new StateVariable;
                            tempSV->readIn(e3);
                            this->StateVariableList.push_back(tempSV);
                        }
                        else if( e3.tagName() == "Alias" )
                        {
                            Alias *tempAlias  = new Alias;
                            tempAlias->readIn(e3);
                            this->AliasList.push_back(tempAlias);
                        }  else {
                            QSettings settings;
                            int num_errs = settings.beginReadArray("errors");
                            settings.endArray();
                            settings.beginWriteArray("errors");
                                settings.setArrayIndex(num_errs + 1);
                                settings.setValue("errorText",  "XML error: misplaced or unknown tag '" + e3.tagName() + "'");
                            settings.endArray();
                        }

                        n3 = n3.nextSibling();
                    }
                }
                else if( e2.tagName() == "AnalogSendPort" || e2.tagName() == "AnalogReceivePort" || e2.tagName() == "AnalogReducePort")
                {
                    AnalogPort *tempAP = new AnalogPort;
                    tempAP->readIn(e2);
                    this->AnalogPortList.push_back(tempAP);
                }
                else if( e2.tagName() == "EventSendPort" || e2.tagName() == "EventReceivePort")
                {
                    EventPort *tempEP = new EventPort;
                    tempEP->readIn(e2);
                    this->EventPortList.push_back(tempEP);

                }
                else if( e2.tagName() == "ImpulseSendPort" || e2.tagName() == "ImpulseReceivePort")
                {
                    ImpulsePort *tempIP = new ImpulsePort;
                    tempIP->readIn(e2);
                    this->ImpulsePortList.push_back(tempIP);

                } else {
                    QSettings settings;
                    int num_errs = settings.beginReadArray("errors");
                    settings.endArray();
                    settings.beginWriteArray("errors");
                        settings.setArrayIndex(num_errs + 1);
                        settings.setValue("errorText",  "XML error: misplaced or unknown tag '" + e2.tagName() + "'");
                    settings.endArray();
                }
                n2 = n2.nextSibling();
            }

        } else {
            QSettings settings;
            int num_errs = settings.beginReadArray("errors");
            settings.endArray();
            settings.beginWriteArray("errors");
                settings.setArrayIndex(num_errs + 1);
                settings.setValue("errorText",  "XML error: expected 'ComponentClass' tag'");
            settings.endArray();
        }
        n = n.nextSibling();
    }

    // check for errors - no point validating if we have XML errors!
    QSettings settings;
    int num_errs = settings.beginReadArray("errors");
    settings.endArray();
    if (num_errs > 0)
        return;

    // no XML errors - so validate this!
    QStringList validated = validateComponent();
    if (validated.size() > 1) {
       /* QMessageBox msgBox;
        QString message;
        for (int i = 0; i < (int) validated.size(); ++i) {
            message += validated[i] + "\n";
        }
        msgBox.setText(message);
        msgBox.exec();*/
    }
}



void NineMLComponent::write(QDomDocument *doc)
{
    // validate this (must be validated if is in memory)
    QStringList validated = validateComponent();

    // check for errors:
    QSettings settings;
    int num_errs = settings.beginReadArray("errors");
    settings.endArray();

    num_errs += settings.beginReadArray("warnings");
    settings.endArray();

    QString errors;

    if (num_errs != 0) {

        errors = errors + "<b>Errors found in current component:</b><br/><br/>";

        // list errors
        settings.beginReadArray("errors");
        for (int j = 1; j < num_errs; ++j) {
            settings.setArrayIndex(j);
            errors = errors + settings.value("errorText", "").toString();
            errors = errors + "<br/>";
        }
        settings.endArray();
        settings.beginReadArray("warnings");
        for (int j = 1; j < num_errs; ++j) {
            settings.setArrayIndex(j);
            errors = errors + settings.value("warnText", "").toString();
            errors = errors + "<br/>";
        }
        settings.endArray();

        // clear errors
        settings.remove("errors");
        settings.remove("warnings");

    }

    if (!errors.isEmpty()) {
        // display errors
        QMessageBox msgBox;
        msgBox.setText("<P><b>" + this->name + ":Component validation failed</b></P>" + errors);
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setTextFormat(Qt::RichText);
        msgBox.setModal(false);
        msgBox.exec();
        return;
    }

    // write out:

    // create the root of the file:
    QDomElement root = doc->createElement( "SpineML" );
    QDomProcessingInstruction xmlDeclaration = doc->createProcessingInstruction("xml", "version=\"1.0\"");
    doc->appendChild(xmlDeclaration);
    doc->appendChild( root );
    root.setAttribute("xmlns", "http://www.shef.ac.uk/SpineMLComponentLayer");
    root.setAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
    root.setAttribute("xsi:schemaLocation", "http://www.shef.ac.uk/SpineMLComponentLayer SpineMLComponentLayer.xsd");
    QDomElement CClass = doc->createElement( "ComponentClass" );
    CClass.setAttribute("name", this->name);
    CClass.setAttribute("type", this->type);
    root.appendChild(CClass);

    QDomElement dynamics = doc->createElement( "Dynamics" );
    if (this->initial_regime != NULL)
        dynamics.setAttribute("initial_regime", this->initial_regime->name);
    else
        dynamics.setAttribute("initial_regime", "Error: No initial regime!");
    CClass.appendChild(dynamics);

    // declarations
    for (int i = 0; i < this->AnalogPortList.size(); ++i) {
        AnalogPortList[i]->writeOut(doc, CClass);
    }
    for (int i = 0; i < this->EventPortList.size(); ++i) {
        EventPortList[i]->writeOut(doc, CClass);
    }
    for (int i = 0; i < this->ImpulsePortList.size(); ++i) {
        ImpulsePortList[i]->writeOut(doc, CClass);
    }
    for (int i = 0; i < this->ParameterList.size(); ++i) {
        ParameterList[i]->writeOut(doc, CClass);
    }

    // dynamics
    for (int i = 0; i < this->RegimeList.size(); ++i) {
        RegimeList[i]->writeOut(doc, dynamics);
    }
    for (int i = 0; i < this->AliasList.size(); ++i) {
        AliasList[i]->writeOut(doc, dynamics);
    }
    for (int i = 0; i < this->StateVariableList.size(); ++i) {
        StateVariableList[i]->writeOut(doc, dynamics);
    }

}

void NineMLData::write_node_xml(QXmlStreamWriter &xmlOut) {

    // definition
    QString simpleName;
    if (this->type == NineMLLayoutType) {
        simpleName = (static_cast<NineMLLayoutData *> (this))->component->name.simplified();
    }
    if (this->type == NineMLComponentType) {
        NineMLComponentData * ptr = static_cast <NineMLComponentData *> (this);
        simpleName = ptr->component->name.simplified();
    }
    simpleName.replace( " ", "_" );
    xmlOut.writeAttribute("url", simpleName + ".xml");

    // wriet the seed and minumum distance for the Layout
    if (this->type == NineMLLayoutType) {
        NineMLLayoutData * lay = static_cast<NineMLLayoutData *> (this);
        xmlOut.writeAttribute("seed", QString::number(lay->seed));
        xmlOut.writeAttribute("minimum_distance", QString::number(lay->minimumDistance));
    }

    if (this->type == NineMLComponentType) {
        NineMLComponentData * ptr = static_cast <NineMLComponentData *> (this);
        for (int i = 0; i < ptr->inputs.size(); ++i) {
            if (ptr->inputs[i]->projInput == true) {

                // add the special input for the synapse
                if (ptr->component->type == "weight_update") {

                    // check we have ports
                    if (ptr->inputs[i]->srcPort.size() == 0 || ptr->inputs[i]->dstPort.size() == 0) {
                        QSettings settings;
                        int num_errs = settings.beginReadArray("warnings");
                        settings.endArray();
                        settings.beginWriteArray("warnings");
                            settings.setArrayIndex(num_errs + 1);
                            settings.setValue("warnText",  "No matched ports between '" + ptr->inputs[i]->src->getXMLName() + "' and '" + ptr->inputs[i]->dst->getXMLName() + "'");
                        settings.endArray();
                    }

                    xmlOut.writeAttribute("input_src_port", ptr->inputs[i]->srcPort);
                    xmlOut.writeAttribute("input_dst_port", ptr->inputs[i]->dstPort);

                }

                // add the special input for the postsynapse
                if (ptr->component->type == "postsynapse") {

                    // check we have ports
                    if (ptr->inputs[i]->srcPort.size() == 0 || ptr->inputs[i]->dstPort.size() == 0) {
                        QSettings settings;
                        int num_errs = settings.beginReadArray("warnings");
                        settings.endArray();
                        settings.beginWriteArray("warnings");
                            settings.setArrayIndex(num_errs + 1);
                            settings.setValue("warnText",  "No matched ports between '" + ptr->inputs[i]->src->getXMLName() + "' and '" +ptr->inputs[i]->dst->getXMLName() + "'");
                        settings.endArray();
                    }


                    xmlOut.writeAttribute("input_src_port", ptr->inputs[i]->srcPort);
                    xmlOut.writeAttribute("input_dst_port", ptr->inputs[i]->dstPort);

                }

            }
        }


        if (ptr->component->type == "postsynapse") {

            // search out the Synapse population and add the postsynapticMapping
            for (int i = 0; i < (qSharedPointerDynamicCast<projection> (ptr->owner))->destination->neuronType->inputs.size(); ++i) {
                if ((qSharedPointerDynamicCast<projection> (ptr->owner))->destination->neuronType->inputs[i]->src == this && \
                    (qSharedPointerDynamicCast<projection> (ptr->owner))->destination->neuronType->inputs[i]->projInput == true) {

                    // check we have ports
                    if (qSharedPointerDynamicCast<projection> (ptr->owner)->destination->neuronType->inputs[i]->srcPort.size() == 0 || (qSharedPointerDynamicCast<projection> (ptr->owner))->destination->neuronType->inputs[i]->dstPort.size() == 0) {
                        QSettings settings;
                        int num_errs = settings.beginReadArray("warnings");
                        settings.endArray();
                        settings.beginWriteArray("warnings");
                            settings.setArrayIndex(num_errs + 1);
                            settings.setValue("warnText",  "No matched ports between '" + (qSharedPointerDynamicCast<projection> (ptr->owner))->destination->neuronType->inputs[i]->src->getXMLName() + "' and '" + (qSharedPointerDynamicCast<projection> (ptr->owner))->destination->neuronType->inputs[i]->dst->getXMLName() + "'");
                        settings.endArray();
                    }


                    xmlOut.writeAttribute("output_src_port", (qSharedPointerDynamicCast<projection> (ptr->owner))->destination->neuronType->inputs[i]->srcPort);
                    xmlOut.writeAttribute("output_dst_port", (qSharedPointerDynamicCast<projection> (ptr->owner))->destination->neuronType->inputs[i]->dstPort);

                }
            }

        }
    }

    if (this->ParameterList.size()+this->StateVariableList.size() > 0) {
        for (int i = 0; i < this->ParameterList.size(); ++i) {
            xmlOut.writeStartElement("Property");

            xmlOut.writeAttribute("name",this->ParameterList[i]->name);
            xmlOut.writeAttribute("dimension", this->ParameterList[i]->dims->toString());

              if (this->ParameterList[i]->currType == FixedValue) {
                  xmlOut.writeEmptyElement("FixedValue");
                  xmlOut.writeAttribute("value", QString::number(this->ParameterList[i]->value[0]));
              }
              if (this->ParameterList[i]->currType == Statistical) {

                  switch (int(round(this->ParameterList[i]->value[0]))) {
                  case 0:
                      break;
                  case 1:
                  {
                      xmlOut.writeEmptyElement("UniformDistribution");
                      xmlOut.writeAttribute("minimum", QString::number(this->ParameterList[i]->value[1]));
                      xmlOut.writeAttribute("maximum", QString::number(this->ParameterList[i]->value[2]));
                      xmlOut.writeAttribute("seed", QString::number(this->ParameterList[i]->value[3]));
                  }
                      break;
                  case 2:
                  {
                      xmlOut.writeEmptyElement("NormalDistribution");
                      xmlOut.writeAttribute("mean", QString::number(this->ParameterList[i]->value[1]));
                      xmlOut.writeAttribute("variance", QString::number(this->ParameterList[i]->value[2]));
                      xmlOut.writeAttribute("seed", QString::number(this->ParameterList[i]->value[3]));
                   }
                      break;
                  }

              }
              if (this->ParameterList[i]->currType == ExplicitList) {
                  // Write out the explicit list either to XML or to binary by calling a built in function
                  this->ParameterList[i]->writeExplicitListNodeData(xmlOut);
                  /*xmlOut.writeStartElement("ValueList");
                  for (int ind = 0; ind < this->ParameterList[i]->value.size(); ++ind) {
                      xmlOut.writeEmptyElement("Value");
                      xmlOut.writeAttribute("index", QString::number(float(this->ParameterList[i]->indices[ind])));
                      xmlOut.writeAttribute("value", QString::number(float(this->ParameterList[i]->value[ind])));
                  }
                  xmlOut.writeEndElement(); // valueList*/
              }

              xmlOut.writeEndElement(); // property
        }
        for (int i = 0; i < this->StateVariableList.size(); ++i) {
            xmlOut.writeStartElement("Property");

            xmlOut.writeAttribute("name",this->StateVariableList[i]->name);
            xmlOut.writeAttribute("dimension", this->StateVariableList[i]->dims->toString());

              if (this->StateVariableList[i]->currType == FixedValue) {
                  xmlOut.writeEmptyElement("FixedValue");
                  xmlOut.writeAttribute("value", QString::number(this->StateVariableList[i]->value[0]));
              }
              if (this->StateVariableList[i]->currType == Statistical) {

                  switch (int(round(this->StateVariableList[i]->value[0]))) {
                  case 0:
                      break;
                  case 1:
                  {
                      xmlOut.writeEmptyElement("UniformDistribution");
                      xmlOut.writeAttribute("minimum", QString::number(this->StateVariableList[i]->value[1]));
                      xmlOut.writeAttribute("maximum", QString::number(this->StateVariableList[i]->value[2]));
                      xmlOut.writeAttribute("seed", QString::number(this->StateVariableList[i]->value[3]));
                  }
                      break;
                  case 2:
                  {
                      xmlOut.writeEmptyElement("NormalDistribution");
                      xmlOut.writeAttribute("mean", QString::number(this->StateVariableList[i]->value[1]));
                      xmlOut.writeAttribute("variance", QString::number(this->StateVariableList[i]->value[2]));
                      xmlOut.writeAttribute("seed", QString::number(this->StateVariableList[i]->value[3]));
                   }
                      break;
                  }
              }
              if (this->StateVariableList[i]->currType == ExplicitList) {
                  this->StateVariableList[i]->writeExplicitListNodeData(xmlOut);
                  /*
                  xmlOut.writeStartElement("ValueList");
                  for (int ind = 0; ind < this->StateVariableList[i]->value.size(); ++ind) {
                      xmlOut.writeEmptyElement("Value");
                      xmlOut.writeAttribute("index", QString::number(float(this->StateVariableList[i]->indices[ind])));
                      xmlOut.writeAttribute("value", QString::number(float(this->StateVariableList[i]->value[ind])));
                  }
                 xmlOut.writeEndElement(); // valueList
                 */
              }

              xmlOut.writeEndElement(); // property
        }
    }

    // if it is not a layout, add the inputs
    if (this->type == NineMLComponentType) {

        NineMLComponentData * ptr = static_cast <NineMLComponentData *> (this);
        bool inputsTag = false;

        for (int i = 0; i < ptr->inputs.size(); ++i) {
           if (ptr->inputs[i]->projInput == false) {
               inputsTag = true;
           }
        }

        if (inputsTag) {
            // inputs
            for (int i = 0; i < ptr->inputs.size(); ++i) {
               if (ptr->inputs[i]->projInput == false) {

                   // check we have ports
                   if (ptr->inputs[i]->srcPort.size() == 0 || ptr->inputs[i]->dstPort.size() == 0) {
                       QSettings settings;
                       int num_errs = settings.beginReadArray("warnings");
                       settings.endArray();
                       settings.beginWriteArray("warnings");
                           settings.setArrayIndex(num_errs + 1);
                           settings.setValue("warnText",  "No matched ports between '" + ptr->inputs[i]->src->getXMLName() + "' and '" + ptr->inputs[i]->dst->getXMLName() + "'");
                       settings.endArray();
                   }

                  xmlOut.writeStartElement("LL:Input");
                  xmlOut.writeAttribute("src", ptr->inputs[i]->src->getXMLName());
                  xmlOut.writeAttribute("src_port", ptr->inputs[i]->srcPort);
                  xmlOut.writeAttribute("dst_port", ptr->inputs[i]->dstPort);
                  if (ptr->inputs[i]->connectionType->type == Kernel) {
                      ((kernel_connection *) ptr->inputs[i]->connectionType)->src = qSharedPointerDynamicCast <population> (ptr->inputs[i]->source);
                      ((kernel_connection *) ptr->inputs[i]->connectionType)->dst = qSharedPointerDynamicCast <population> (ptr->inputs[i]->destination);
                  }
                  if (ptr->inputs[i]->connectionType->type == Python) {
                      ((pythonscript_connection *) ptr->inputs[i]->connectionType)->src = qSharedPointerDynamicCast <population> (ptr->inputs[i]->source);
                      ((pythonscript_connection *) ptr->inputs[i]->connectionType)->dst = qSharedPointerDynamicCast <population> (ptr->inputs[i]->destination);
                  }
                  ptr->inputs[i]->connectionType->write_node_xml(xmlOut);
                  xmlOut.writeEndElement(); // input
              }

            }
        }



    }
}

void Parameter::readIn(QDomElement e)
{
    this->name = e.attribute("name","");
    if (this->name == "") {
      QSettings settings;
      int num_errs = settings.beginReadArray("errors");
      settings.endArray();
      settings.beginWriteArray("errors");
          settings.setArrayIndex(num_errs + 1);
          settings.setValue("errorText",  "XML error: missing Parameter attribute 'name'");
      settings.endArray();
    }
    // we need dims in here too eventually
    //delete dims;
    this->dims->fromString(e.attribute("dimension",""));

}

void Parameter::writeOut(QDomDocument * doc, QDomElement &parent)
{
    QDomElement parameter = doc->createElement( "Parameter" );
    parameter.setAttribute("name", this->name);
    parameter.setAttribute("dimension", this->dims->toString());
    parent.appendChild(parameter);

}

QString Parameter::getName()
{
    return name;
}

void Parameter::setName(QString name)
{
    this->name = name;
    emit nameChanged();
}


void StateAssignment::readIn(QDomElement e) {

    // we need to handle the statevariable stuff here... how?
    this->name = e.attribute("variable","");
    if (this->name == "") {
      QSettings settings;
      int num_errs = settings.beginReadArray("errors");
      settings.endArray();
      settings.beginWriteArray("errors");
          settings.setArrayIndex(num_errs + 1);
          settings.setValue("errorText",  "XML error: missing StateAssignment attribute 'variable'");
      settings.endArray();
    }
    QDomNode n = e.firstChild();
    QDomElement e2 = n.toElement();
    if( e2.tagName() == "MathInline" )
    {
        this->maths = new MathInLine;
        this->maths->readIn(e2);
    } else {
      QSettings settings;
      int num_errs = settings.beginReadArray("errors");
      settings.endArray();
      settings.beginWriteArray("errors");
          settings.setArrayIndex(num_errs + 1);
          settings.setValue("errorText",  "XML error: missing StateAssignment 'MathInLine' tag'");
      settings.endArray();
    }
}

void StateAssignment::writeOut(QDomDocument * doc, QDomElement &parent)
{
    QDomElement StateAssignment = doc->createElement( "StateAssignment" );
    StateAssignment.setAttribute("variable", this->name);
    parent.appendChild(StateAssignment);

    this->maths->writeOut(doc, StateAssignment);

}


void OnEvent::readIn(QDomElement e) {
    this->target_regime_name = e.attribute("target_regime","");
    if (this->target_regime_name == "") {
      QSettings settings;
      int num_errs = settings.beginReadArray("errors");
      settings.endArray();
      settings.beginWriteArray("errors");
          settings.setArrayIndex(num_errs + 1);
          settings.setValue("errorText",  "XML error: missing OnEvent 'target_regime' attribute'");
      settings.endArray();
    }
    this->src_port_name = e.attribute("src_port","");
    if (this->target_regime_name == "") {
      QSettings settings;
      int num_errs = settings.beginReadArray("errors");
      settings.endArray();
      settings.beginWriteArray("errors");
          settings.setArrayIndex(num_errs + 1);
          settings.setValue("errorText",  "XML error: missing OnEvent 'src_port' attribute'");
      settings.endArray();
    }
    QDomNode n = e.firstChild();
    while( !n.isNull() )
    {
        QDomElement e2 = n.toElement();
        if( e2.tagName() == "StateAssignment" )
        {
            StateAssignment *tempSA = new StateAssignment;
            tempSA->readIn(e2);
            this->StateAssignList.push_back(tempSA);
        }
        else if( e2.tagName() == "EventOut" )
        {
            EventOut *tempEO = new EventOut;
            tempEO->readIn(e2);
            this->eventOutList.push_back(tempEO);
        }
        else if( e2.tagName() == "ImpulseOut" )
        {
            ImpulseOut *tempIO = new ImpulseOut;
            tempIO->readIn(e2);
            this->impulseOutList.push_back(tempIO);
        } else {
            QSettings settings;
            int num_errs = settings.beginReadArray("errors");
            settings.endArray();
            settings.beginWriteArray("errors");
                settings.setArrayIndex(num_errs + 1);
                settings.setValue("errorText",  "XML error: misplaced or unknown tag - '" + e2.tagName() + "'");
            settings.endArray();
        }
        n = n.nextSibling();
    }
}

void OnEvent::writeOut(QDomDocument * doc, QDomElement &parent)
{
    QDomElement OnEvent = doc->createElement( "OnEvent" );
    parent.appendChild(OnEvent);
    if (target_regime != NULL)
        OnEvent.setAttribute("target_regime", this->target_regime->name);
    else
        OnEvent.setAttribute("target_regime", "Error: No Synapse Regime!");

    if (src_port != NULL)
        OnEvent.setAttribute("src_port", this->src_port->getName());
    else
        OnEvent.setAttribute("src_port", "Error: No Source Event Port!");

    for (int i = 0; i < this->StateAssignList.size(); ++i) {
        this->StateAssignList[i]->writeOut(doc, OnEvent);
    }
    for (int i = 0; i < this->eventOutList.size(); ++i) {
        this->eventOutList[i]->writeOut(doc, OnEvent);
    }
    for (int i = 0; i < this->impulseOutList.size(); ++i) {
        this->impulseOutList[i]->writeOut(doc, OnEvent);
    }
}


void OnImpulse::readIn(QDomElement e) {
    this->target_regime_name = e.attribute("target_regime","");
    if (this->target_regime_name == "") {
      QSettings settings;
      int num_errs = settings.beginReadArray("errors");
      settings.endArray();
      settings.beginWriteArray("errors");
          settings.setArrayIndex(num_errs + 1);
          settings.setValue("errorText",  "XML error: missing OnImpulse 'target_regime' attribute'");
      settings.endArray();
    }
    this->src_port_name = e.attribute("src_port","");
    if (this->target_regime_name == "") {
      QSettings settings;
      int num_errs = settings.beginReadArray("errors");
      settings.endArray();
      settings.beginWriteArray("errors");
          settings.setArrayIndex(num_errs + 1);
          settings.setValue("errorText",  "XML error: missing OnImpulse 'src_port' attribute'");
      settings.endArray();
    }
    QDomNode n = e.firstChild();
    while( !n.isNull() )
    {
        QDomElement e2 = n.toElement();
        if( e2.tagName() == "StateAssignment" )
        {
            StateAssignment *tempSA = new StateAssignment;
            tempSA->readIn(e2);
            this->StateAssignList.push_back(tempSA);
        }
        else if( e2.tagName() == "EventOut" )
        {
            EventOut *tempEO = new EventOut;
            tempEO->readIn(e2);
            this->eventOutList.push_back(tempEO);
        }
        else if( e2.tagName() == "ImpulseOut" )
        {
            ImpulseOut *tempIO = new ImpulseOut;
            tempIO->readIn(e2);
            this->impulseOutList.push_back(tempIO);
        } else {
            QSettings settings;
            int num_errs = settings.beginReadArray("errors");
            settings.endArray();
            settings.beginWriteArray("errors");
                settings.setArrayIndex(num_errs + 1);
                settings.setValue("errorText",  "XML error: misplaced or unknown tag - '" + e2.tagName() + "'");
            settings.endArray();
        }
        n = n.nextSibling();
    }
}

void OnImpulse::writeOut(QDomDocument * doc, QDomElement &parent)
{
    QDomElement OnImpulse = doc->createElement( "OnImpulse" );
    parent.appendChild(OnImpulse);
    if (target_regime != NULL)
        OnImpulse.setAttribute("target_regime", this->target_regime->name);
    else
        OnImpulse.setAttribute("target_regime", "");

    if (src_port != NULL)
        OnImpulse.setAttribute("src_port", this->src_port->getName());
    else
        OnImpulse.setAttribute("src_port", "");

    for (int i = 0; i < this->StateAssignList.size(); ++i) {
        this->StateAssignList[i]->writeOut(doc, OnImpulse);
    }
    for (int i = 0; i < this->eventOutList.size(); ++i) {
        this->eventOutList[i]->writeOut(doc, OnImpulse);
    }
    for (int i = 0; i < this->impulseOutList.size(); ++i) {
        this->impulseOutList[i]->writeOut(doc, OnImpulse);
    }
}


void Trigger::readIn(QDomElement e) {
    QDomNode n = e.firstChild();
    QDomElement e2 = n.toElement();
    if( e2.tagName() == "MathInline" )
    {
        this->maths = new MathInLine;
        this->maths->readIn(e2);
    } else {
        QSettings settings;
        int num_errs = settings.beginReadArray("errors");
        settings.endArray();
        settings.beginWriteArray("errors");
            settings.setArrayIndex(num_errs + 1);
            settings.setValue("errorText",  "XML error: missing Trigger 'MathInLine' tag'");
        settings.endArray();
      }
}

void Trigger::writeOut(QDomDocument * doc, QDomElement &parent)
{
    QDomElement Trigger = doc->createElement( "Trigger" );
    parent.appendChild(Trigger);

    this->maths->writeOut(doc, Trigger);

}

void OnCondition::readIn(QDomElement e) {
    this->target_regime_name = e.attribute("target_regime","");
    if (this->target_regime_name == "") {
      QSettings settings;
      int num_errs = settings.beginReadArray("errors");
      settings.endArray();
      settings.beginWriteArray("errors");
          settings.setArrayIndex(num_errs + 1);
          settings.setValue("errorText",  "XML error: missing OnCondition 'target_regime' attribute'");
      settings.endArray();
    }
    QDomNode n = e.firstChild();
    while( !n.isNull() )
    {
        QDomElement e2 = n.toElement();
        if( e2.tagName() == "StateAssignment" )
        {
            StateAssignment *tempSA = new StateAssignment;
            tempSA->readIn(e2);
            this->StateAssignList.push_back(tempSA);
        }
        else if( e2.tagName() == "Trigger" )
        {
            this->trigger = new Trigger;
            this->trigger->readIn(e2);
        }
        else if( e2.tagName() == "EventOut" )
        {
            EventOut *tempEO = new EventOut;
            tempEO->readIn(e2);
            this->eventOutList.push_back(tempEO);
        }
        else if( e2.tagName() == "ImpulseOut" )
        {
            ImpulseOut *tempIO = new ImpulseOut;
            tempIO->readIn(e2);
            this->impulseOutList.push_back(tempIO);
        } else {
            QSettings settings;
            int num_errs = settings.beginReadArray("errors");
            settings.endArray();
            settings.beginWriteArray("errors");
                settings.setArrayIndex(num_errs + 1);
                settings.setValue("errorText",  "XML error: misplaced or unknown tag - '" + e2.tagName() + "'");
            settings.endArray();
        }
        n = n.nextSibling();
    }
}

void OnCondition::writeOut(QDomDocument * doc, QDomElement &parent)
{
    QDomElement OnCondition = doc->createElement( "OnCondition" );
    parent.appendChild(OnCondition);
    if (this->target_regime != NULL)
        OnCondition.setAttribute("target_regime", this->target_regime->name);
    else
        OnCondition.setAttribute("target_regime", "Error: No Synapse Regime!");

    for (int i = 0; i < this->StateAssignList.size(); ++i) {
        this->StateAssignList[i]->writeOut(doc, OnCondition);
    }
    for (int i = 0; i < this->eventOutList.size(); ++i) {
        this->eventOutList[i]->writeOut(doc, OnCondition);
    }
    for (int i = 0; i < this->impulseOutList.size(); ++i) {
        this->impulseOutList[i]->writeOut(doc, OnCondition);
    }

    this->trigger->writeOut(doc, OnCondition);

}

void Alias::readIn(QDomElement e) {
    this->name = e.attribute("name","");
    if (this->name == "") {
      QSettings settings;
      int num_errs = settings.beginReadArray("errors");
      settings.endArray();
      settings.beginWriteArray("errors");
          settings.setArrayIndex(num_errs + 1);
          settings.setValue("errorText",  "XML error: missing Alias 'name' attribute'");
      settings.endArray();
    }
    QDomNode n = e.firstChild();
    QDomElement e2 = n.toElement();
    if( e2.tagName() == "MathInline" )
    {
        this->maths = new MathInLine;
        this->maths->readIn(e2);
    } else {
        QSettings settings;
        int num_errs = settings.beginReadArray("errors");
        settings.endArray();
        settings.beginWriteArray("errors");
            settings.setArrayIndex(num_errs + 1);
            settings.setValue("errorText",  "XML error: missing Alias 'MathInLine' tag'");
        settings.endArray();
    }
    //delete dims;
    this->dims->fromString(e.attribute("dimension",""));
}

void Alias::writeOut(QDomDocument * doc, QDomElement &parent)
{
    QDomElement Alias = doc->createElement( "Alias" );
    parent.appendChild(Alias);
    Alias.setAttribute("name", this->name);
    Alias.setAttribute("dimension", this->dims->toString());
    this->maths->writeOut(doc, Alias);

}

void StateVariable::readIn(QDomElement e) {
    this->name = e.attribute("name","");
    if (this->name == "") {
      QSettings settings;
      int num_errs = settings.beginReadArray("errors");
      settings.endArray();
      settings.beginWriteArray("errors");
          settings.setArrayIndex(num_errs + 1);
          settings.setValue("errorText",  "XML error: missing StateVariable 'name' attribute'");
      settings.endArray();
    }
    //delete dims;
    this->dims->fromString(e.attribute("dimension",""));
}

void StateVariable::writeOut(QDomDocument * doc, QDomElement &parent)
{
    QDomElement StateVariable = doc->createElement( "StateVariable" );
    parent.appendChild(StateVariable);
    StateVariable.setAttribute("name", this->name);
    StateVariable.setAttribute("dimension", this->dims->toString());

}

void TimeDerivative::readIn(QDomElement e) {
    this->variable_name = e.attribute("variable","");
    if (this->variable_name == "") {
      QSettings settings;
      int num_errs = settings.beginReadArray("errors");
      settings.endArray();
      settings.beginWriteArray("errors");
          settings.setArrayIndex(num_errs + 1);
          settings.setValue("errorText",  "XML error: missing TimeDerivative 'variable' attribute'");
      settings.endArray();
    }
    QDomNode n = e.firstChild();
    QDomElement e2 = n.toElement();
    if( e2.tagName() == "MathInline" )
    {
        this->maths = new MathInLine;
        this->maths->readIn(e2);
    } else {
        QSettings settings;
        int num_errs = settings.beginReadArray("errors");
        settings.endArray();
        settings.beginWriteArray("errors");
            settings.setArrayIndex(num_errs + 1);
            settings.setValue("errorText",  "XML error: missing TimeDerivative 'MathInLine' tag'");
        settings.endArray();
    }
}

void TimeDerivative::writeOut(QDomDocument * doc, QDomElement &parent)
{
    QDomElement TimeDerivative = doc->createElement( "TimeDerivative" );
    if (this->variable != NULL)
        TimeDerivative.setAttribute("variable", this->variable->getName());
    else
        TimeDerivative.setAttribute("variable", "Error: No variable!");
    parent.appendChild(TimeDerivative);
    this->maths->writeOut(doc, TimeDerivative);

}

void MathInLine::readIn(QDomElement e) {
    this->equation = e.text();
    // it is not fatal if equation is blank - but validation should flag it up
    /*if (this->equation == "") {
      QSettings settings;
      int num_errs = settings.beginReadArray("errors");
      settings.endArray();
      settings.beginWriteArray("errors");
          settings.setArrayIndex(num_errs + 1);
          settings.setValue("errorText",  "XML error: missing MathInLine equation'");
      settings.endArray();
    }*/
}

void MathInLine::writeOut(QDomDocument * doc, QDomElement &parent)
{
    QDomElement MathInLine = doc->createElement( "MathInline" );
    parent.appendChild(MathInLine);
    QString out = this->equation;
    out.replace("&gt;", ">");
    out.replace("&lt;", "<");
    QDomText text = doc->createTextNode(out);
    MathInLine.appendChild(text);
}


void EventOut::readIn(QDomElement e) {
    this->port_name = e.attribute("port","");
    if (this->port_name == "") {
      QSettings settings;
      int num_errs = settings.beginReadArray("errors");
      settings.endArray();
      settings.beginWriteArray("errors");
          settings.setArrayIndex(num_errs + 1);
          settings.setValue("errorText",  "XML error: missing EventOut 'port' attribute");
      settings.endArray();
    }
}

void EventOut::writeOut(QDomDocument * doc, QDomElement &parent)
{
    QDomElement EventOut = doc->createElement( "EventOut" );
    parent.appendChild(EventOut);
    if (this->port != NULL)
        EventOut.setAttribute("port", this->port->getName());
    else
        EventOut.setAttribute("port", "Error: No Event Port!");

}

void ImpulseOut::readIn(QDomElement e) {
    this->port_name = e.attribute("port","");
    if (this->port_name == "") {
      QSettings settings;
      int num_errs = settings.beginReadArray("errors");
      settings.endArray();
      settings.beginWriteArray("errors");
          settings.setArrayIndex(num_errs + 1);
          settings.setValue("errorText",  "XML error: missing ImpulseOut 'port' attribute");
      settings.endArray();
    }
}

void ImpulseOut::writeOut(QDomDocument * doc, QDomElement &parent)
{
    QDomElement ImpulseOut = doc->createElement( "ImpulseOut" );
    parent.appendChild(ImpulseOut);
    if (this->port != NULL){
        if (this->port->mode == ImpulseSendPort){
            if (this->port->parameter != NULL){
                ImpulseOut.setAttribute("port", this->port->parameter->getName());
            }else{
                ImpulseOut.setAttribute("port", "Error: No Paramter in Impulse Send Port!");
            }
        }else{
            ImpulseOut.setAttribute("port", this->port->getName());
        }
    }else{
        ImpulseOut.setAttribute("port", "Error: No Impulse Port!");
    }

}


void AnalogPort::readIn(QDomElement e) {
    this->name = e.attribute("name","");
    if (this->name == "") {
      QSettings settings;
      int num_errs = settings.beginReadArray("errors");
      settings.endArray();
      settings.beginWriteArray("errors");
          settings.setArrayIndex(num_errs + 1);
          settings.setValue("errorText",  "XML error: missing AnalogPort 'name' attribute");
      settings.endArray();
    }
    if (e.tagName()=="AnalogReceivePort") {
        this->mode=AnalogRecvPort;
        this->dims->fromString(e.attribute("dimension",""));
    } else if (e.tagName()=="AnalogSendPort") {
        this->mode=AnalogSendPort;
    } else if (e.tagName()=="AnalogReducePort") {
        this->mode=AnalogReducePort;
        this->op = ReduceOperationAddition;
        this->dims->fromString(e.attribute("dimension",""));
        QString reduce_op = e.attribute("reduce_op","");
        if (reduce_op.compare("+") != 0){
            this->op = ReduceOperationNone;
        }
    } else {
        QSettings settings;
        int num_errs = settings.beginReadArray("errors");
        settings.endArray();
        settings.beginWriteArray("errors");
            settings.setArrayIndex(num_errs + 1);
            settings.setValue("errorText",  "XML error: misplaced or unknown tag - '" + e.tagName() + "'");
        settings.endArray();
    }
}

void AnalogPort::writeOut(QDomDocument * doc, QDomElement &parent)
{
    QDomElement AnalogPort;

    if (this->mode==AnalogSendPort) {
        AnalogPort = doc->createElement( "AnalogSendPort" );
        AnalogPort.setAttribute("name", this->getName());
    }
    if (this->mode==AnalogRecvPort) {
        AnalogPort = doc->createElement( "AnalogReceivePort" );
        AnalogPort.setAttribute("name", this->getName());
        AnalogPort.setAttribute("dimension", this->dims->toString());
    }
    if (this->mode==AnalogReducePort) {
        AnalogPort = doc->createElement( "AnalogReducePort" );
        AnalogPort.setAttribute("name", this->getName());
        if (this->op == ReduceOperationNone) {
            //AnalogPort.setAttribute("reduce_op", "");
        }
        if (this->op == ReduceOperationAddition) {
            AnalogPort.setAttribute("reduce_op", "+");
        }
        AnalogPort.setAttribute("dimension", this->dims->toString());

    }
    parent.appendChild(AnalogPort);

}

void EventPort::readIn(QDomElement e) {
    this->name = e.attribute("name","");
    if (this->name == "") {
      QSettings settings;
      int num_errs = settings.beginReadArray("errors");
      settings.endArray();
      settings.beginWriteArray("errors");
          settings.setArrayIndex(num_errs + 1);
          settings.setValue("errorText",  "XML error: missing EventPort 'name' attribute");
      settings.endArray();
    }
    if (e.tagName()=="EventReceivePort") {
        this->mode=EventRecvPort;
    } else if (e.tagName()=="EventSendPort") {
        this->mode=EventSendPort;
    }  else {
        QSettings settings;
        int num_errs = settings.beginReadArray("errors");
        settings.endArray();
        settings.beginWriteArray("errors");
            settings.setArrayIndex(num_errs + 1);
            settings.setValue("errorText",  "XML error: misplaced or unknown tag - '" + e.tagName() + "'");
        settings.endArray();
    }
}

void EventPort::writeOut(QDomDocument * doc, QDomElement &parent)
{
    QDomElement EventPort;

    if (this->mode==EventSendPort) {
        EventPort = doc->createElement( "EventSendPort" );
        EventPort.setAttribute("name", this->getName());
    }
    if (this->mode==EventRecvPort) {
        EventPort = doc->createElement( "EventReceivePort" );
        EventPort.setAttribute("name", this->getName());
    }

    parent.appendChild(EventPort);

}

void ImpulsePort::readIn(QDomElement e) {
    this->name = e.attribute("name","");
    if (this->name == "") {
      QSettings settings;
      int num_errs = settings.beginReadArray("errors");
      settings.endArray();
      settings.beginWriteArray("errors");
          settings.setArrayIndex(num_errs + 1);
          settings.setValue("errorText",  "XML error: missing ImpulsePort 'name' attribute");
      settings.endArray();
    }
    if (e.tagName()=="ImpulseReceivePort") {
        this->mode=ImpulseRecvPort;
        this->dims->fromString(e.attribute("dimension",""));
    } else if (e.tagName()=="ImpulseSendPort") {
        this->mode=ImpulseSendPort;
    }  else {
        QSettings settings;
        int num_errs = settings.beginReadArray("errors");
        settings.endArray();
        settings.beginWriteArray("errors");
            settings.setArrayIndex(num_errs + 1);
            settings.setValue("errorText",  "XML error: misplaced or unknown tag - '" + e.tagName() + "'");
        settings.endArray();
    }
}

void ImpulsePort::writeOut(QDomDocument * doc, QDomElement &parent)
{
    QDomElement ImpulsePort;

    if (this->mode==ImpulseSendPort) {
        ImpulsePort = doc->createElement( "ImpulseSendPort" );
        if (this->parameter != NULL)
            ImpulsePort.setAttribute("name", this->parameter->getName());
        else
            ImpulsePort.setAttribute("name", "Error: No paramater named!");
    }
    if (this->mode==ImpulseRecvPort) {
        ImpulsePort = doc->createElement( "ImpulseReceivePort" );
        ImpulsePort.setAttribute("name", this->getName());
        ImpulsePort.setAttribute("dimension", this->dims->toString());
    }

    parent.appendChild(ImpulsePort);

}

void Regime::readIn(QDomElement e)
{

    this->name = e.attribute("name","");
    if (this->name == "") {
      QSettings settings;
      int num_errs = settings.beginReadArray("errors");
      settings.endArray();
      settings.beginWriteArray("errors");
          settings.setArrayIndex(num_errs + 1);
          settings.setValue("errorText",  "XML error: missing Regime 'name' attribute");
      settings.endArray();
    }
    QDomNode n = e.firstChild();
    while( !n.isNull() )
    {
        QDomElement e2 = n.toElement();
        if( e2.tagName() == "TimeDerivative" )
        {
            TimeDerivative *tempTD = new TimeDerivative;
            tempTD->readIn(e2);
            this->TimeDerivativeList.push_back(tempTD);
        }
        if( e2.tagName() == "OnEvent" )
        {
            OnEvent *tempOE = new OnEvent;
            tempOE->readIn(e2);
            this->OnEventList.push_back(tempOE);
        }
        if( e2.tagName() == "OnCondition" )
        {
            OnCondition *tempOC = new OnCondition;
            tempOC->readIn(e2);
            this->OnConditionList.push_back(tempOC);
        }
        if( e2.tagName() == "OnImpulse" )
        {
            OnImpulse *tempOI = new OnImpulse;
            tempOI->readIn(e2);
            this->OnImpulseList.push_back(tempOI);
        }
        n = n.nextSibling();
    }

}

void Regime::writeOut(QDomDocument *doc, QDomElement &parent)
{

    QDomElement regime = doc->createElement( "Regime" );
    regime.setAttribute("name", this->name);
    parent.appendChild(regime);

    // write children
    for (int i = 0; i < this->TimeDerivativeList.size(); ++i) {
        this->TimeDerivativeList[i]->writeOut(doc, regime);
    }
    for (int i = 0; i < this->OnEventList.size(); ++i) {
        this->OnEventList[i]->writeOut(doc, regime);
    }
    for (int i = 0; i < this->OnConditionList.size(); ++i) {
        this->OnConditionList[i]->writeOut(doc, regime);
    }
    for (int i = 0; i < this->OnImpulseList.size(); ++i) {
        this->OnImpulseList[i]->writeOut(doc, regime);
    }

}

Parameter::Parameter(Parameter *data)
{
    name = data->name;
    dims = new dim(data->dims->toString());
}


ParameterData::ParameterData(Parameter *data)
{
    name = data->name;
    dims = new dim(data->dims->toString());
    currType = Undefined;
}

ParameterData::ParameterData(ParameterData *data)
{
    value = data->value;
    indices = data->indices;
    name = data->name;
    dims = new dim(data->dims->toString());
    currType = data->currType;
}

void ParameterData::writeExplicitListNodeData(QXmlStreamWriter &xmlOut) {

    QSettings settings;
    // fetch the option for whether we write binary data for saving
    bool writeBinary = settings.value("fileOptions/saveBinaryConnections", "error").toBool();

    // fetch if we are exporting for simulation
    bool exportBinary = false;
    if (settings.value("export_for_simulation", "false").toBool()) {
        // override save binary option
        writeBinary = settings.value("export_binary").toBool();
        qDebug() << "Export for sim detected";
    }

    // if we have few indices, or we are forbidden from using binary data
    if (this->indices.size() < 31 || !writeBinary) {
        xmlOut.writeStartElement("ValueList");
        for (int ind = 0; ind < this->value.size(); ++ind) {
            xmlOut.writeEmptyElement("Value");
            xmlOut.writeAttribute("index", QString::number(float(this->indices[ind])));
            xmlOut.writeAttribute("value", QString::number(float(this->value[ind])));
        }
       xmlOut.writeEndElement(); // valueList

    } else {

        xmlOut.writeStartElement("ValueList");

        // Save as binary data
        QString filePathString = settings.value("files/currentFileName", "error").toString();

        if (filePathString == "error") {
            qDebug() << "Error getting current project path - THIS SHOULD NEVER HAPPEN! (nineML_classes.cpp ParameterData::writeExplicitListNodeData)";
            return;
        }

        QDir saveDir(filePathString);
        saveDir.cdUp();
        if (exportBinary) {
            saveDir.setCurrent(settings.value("simulator_export_path").toString());
        }

        //generate a unique filename to save the par or sv under
        QStringList filters;
        filters << "explicitDataBinaryFile*";
        saveDir.setNameFilters(filters);

        QStringList files = saveDir.entryList();

        QString baseName = "explicitDataBinaryFile";
        QString uniqueName;
        bool unique = false;
        int index = 0;
        while(!unique) {
            unique = true;
            uniqueName = baseName + QString::number(float(index));
            for (int i = 0; i < (int)files.count(); ++i) {
                // see if the new name is unique
                if (uniqueName == files[i]) {
                    unique = false;
                    ++index;
                }
            }
        }
        QString fileName = uniqueName;

        // construct the save file name based upon whether we are saving the project or outputting for simulation
        QString saveFileName;
        saveFileName = saveDir.absoluteFilePath(fileName + ".bin");
        // add a tag to the binary file
        xmlOut.writeEmptyElement("BinaryFile");
        xmlOut.writeAttribute("file_name", fileName + ".bin");
        xmlOut.writeAttribute("num_elements", QString::number(float(this->value.size())));

        // write out binary data
        QFile export_file(saveFileName);

        if (!export_file.open( QIODevice::WriteOnly)) {
            QMessageBox msgBox;
            msgBox.setText("Error creating binary file '" + saveFileName
                           + "' - is there sufficient disk space?");
            msgBox.exec();
            return;
        }

        // write out the data to the save file, index first, then value, then next index-value pair...
        QDataStream access(&export_file);
        for (int i = 0; i < this->value.size(); ++i) {
            access.writeRawData((char*) &this->indices[i], sizeof(int));
            access.writeRawData((char*) &this->value[i], sizeof(float));
        }

        xmlOut.writeEndElement(); // valueList

    }


}

void ParameterData::readExplicitListNodeData(QDomNode &n) {

    // read in an XML list
    QDomNodeList propValInst = n.toElement().elementsByTagName("Value");
    for (int ind = 0; ind < (int) propValInst.count(); ++ind) {
        this->indices.push_back(propValInst.item(ind).toElement().attribute("index").toInt());
        this->value.push_back(propValInst.item(ind).toElement().attribute("value").toFloat());
    }
    // read in binary data
    QDomNodeList binaryValInst = n.toElement().elementsByTagName("BinaryFile");
    if (binaryValInst.count() == 1) {

        // is a binary file so load accordingly

        // set number of connections
        int num_elements = binaryValInst.at(0).toElement().attribute("num_elements").toUInt();

        // get a handle to the saved file
        QSettings settings;
        QString filePathString = settings.value("files/currentFileName", "error").toString();

        if (filePathString == "error") {
            qDebug() << "Error getting current project path - THIS SHOULD NEVER HAPPEN!";
            return;}

        QDir filePath(filePathString);

        // get file name and path
        QString fileName = binaryValInst.at(0).toElement().attribute("file_name");

        QFile fileIn(filePath.absoluteFilePath(fileName));

        // check that the data file exists!
        if (!fileIn.open(QIODevice::ReadOnly)) {
            QSettings settings;
            int num_errs = settings.beginReadArray("errors");
            settings.endArray();
            settings.beginWriteArray("errors");
                settings.setArrayIndex(num_errs + 1);
                settings.setValue("errorText",  "Error: Binary file referenced in network not found: " + fileName);
            settings.endArray();
            return;
        }

        this->value.clear();
        this->indices.clear();

        // get the count of the number of value-index pairs we have read in
        int count = 0;

        // load in the binary packed data
        while (!(fileIn.atEnd())) {

            // read in the required values:
            // first two int32s
            int index;
            float value;

            fileIn.read((char *) &index,sizeof(int));
            fileIn.read((char *) &value,sizeof(float));

            this->indices.push_back(index);
            this->value.push_back(value);
            ++count;
        }

        if (count != num_elements) {
            qDebug() << "Mismatch between the number of rows in the XML and in the binary";
        }

    }
}

Port::Port(Port *data)
{
    name = data->name;
    dims = new dim(data->dims->toString());
}

bool Port::isAnalog() {return true;}

QString Port::getName()
{
    return name;
}

void Port::setName(QString name)
{
    this->name = name;
    emit nameChanged();
}

AnalogPort::AnalogPort(AnalogPort *data): Port(data)
{
    mode = data->mode;
    op = data->op;
    variable = NULL;
}

bool AnalogPort::isAnalog() {return true;}

int AnalogPort::validateAnalogPort(NineMLComponent * component, QStringList * )
{
    //if mode is send then validate
    int failures = 0;

    if (mode == AnalogSendPort)
    {
        bool match = false;
        for(int i=0;i<component->StateVariableList.size();i++)
        {
            if (component->StateVariableList[i]->getName() == name)
            {
                variable = component->StateVariableList[i];
                match = true;
            }
        }
        if (!match)
        {
            for(int i=0;i<component->AliasList.size();i++)
            {
                if (component->AliasList[i]->getName() == name)
                {
                    variable = component->AliasList[i];
                    match = true;
                }
            }
        }
        if (!match)
        {
            QSettings settings;
            int num_errs = settings.beginReadArray("errors");
            settings.endArray();
            settings.beginWriteArray("errors");
                settings.setArrayIndex(num_errs + 1);
                settings.setValue("errorText",   "Error: AnalogPort references missing StateVariable or Alias " + name);
            settings.endArray();
        }
    }
    else
    {
        variable = NULL;
    }
    return failures;
}

EventPort::EventPort(EventPort *data): Port(data)
{
    mode = data->mode;
}

bool EventPort::isAnalog() {return false;}


ImpulsePort::ImpulsePort(ImpulsePort *data): Port(data)
{
    mode = data->mode;
    parameter = NULL;
}

bool ImpulsePort::isAnalog() {return false;}

int ImpulsePort::validateImpulsePort(NineMLComponent * component, QStringList * )
{
    //if mode is send then validate
    int failures = 0;

    if (mode == ImpulseSendPort)
    {
        bool match = false;
        for(int i=0;i<component->ParameterList.size();i++)
        {
            if (component->ParameterList[i]->getName() == name)
            {
                parameter = component->ParameterList[i];
                match = true;
            }
        }
        for(int i=0;i<component->StateVariableList.size();i++)
        {
            if (component->StateVariableList[i]->getName() == name)
            {
                parameter = component->StateVariableList[i];
                match = true;
            }
        }
        for(int i=0;i<component->AliasList.size();i++)
        {
            if (component->AliasList[i]->getName() == name)
            {
                parameter = component->AliasList[i];
                match = true;
            }
        }

        if (!match)
        {
            QSettings settings;
            int num_errs = settings.beginReadArray("errors");
            settings.endArray();
            settings.beginWriteArray("errors");
                settings.setArrayIndex(num_errs + 1);
                settings.setValue("errorText",   "Error: ImpulsePort references missing StateVariable or Alias " + name);
            settings.endArray();
        }
    }
    else
    {
        parameter = NULL;
    }
    return failures;
}



MathInLine::MathInLine(MathInLine *data)
{
    equation = data->equation;
}

QString MathInLine::getHTMLSafeEquation()
{
    QString result = equation;
    result.replace(QString("<"), QString("&lt;"));
    result.replace(QString(">"), QString("&gt;"));
    return result;
}

Trigger::Trigger(Trigger *data)
{
    maths = new MathInLine(data->maths);
}

Trigger::~Trigger()
{
    delete maths;
}

StateVariable::StateVariable(StateVariable *data) : Parameter(data){
    //name = data->name;
    //delete dims;
    //dims->fromString(data->dims->toString());
}

StateVariable::~StateVariable()
{
    //delete dims;
}

StateVariableData::StateVariableData(StateVariable *data) : ParameterData(data){
    name = data->name;
    dims->fromString(data->dims->toString());
    currType = Undefined;
}

StateVariableData::StateVariableData(StateVariableData *data) : ParameterData(data)
{
    value = data->value;
    indices = data->indices;
    name = data->name;
    dims->fromString(data->dims->toString());
    currType = data->currType;
}

Alias::Alias(Alias *data) : StateVariable(data)
{
    //name = data->name;
    //delete dims;
    //dims->fromString(data->dims->toString());
    maths = new MathInLine(data->maths);
}

Alias::~Alias()
{
    delete maths;
}

TimeDerivative::TimeDerivative(TimeDerivative *data)
{
    variable = NULL;
    variable_name = data->variable_name;
    maths = new MathInLine(data->maths);
}

TimeDerivative::~TimeDerivative()
{
    delete maths;
}

StateAssignment::StateAssignment(StateAssignment *data)
{
    name = data->name;
    variable = NULL;
    maths = new MathInLine(data->maths);
}

StateAssignment::~StateAssignment()
{
    delete maths;
}

EventOut::EventOut(EventOut * data)
{
    port = NULL;
    this->port_name = data->port_name;
}

ImpulseOut::ImpulseOut(ImpulseOut * data)
{
    port = NULL;
    this->port_name = data->port_name;
}

OnCondition::OnCondition(OnCondition *data)
{
    target_regime = NULL;
    target_regime_name = data->target_regime_name;
    trigger = new Trigger(data->trigger);
    StateAssignList = QVector <StateAssignment*>(data->StateAssignList.size());
    eventOutList = QVector <EventOut*>(data->eventOutList.size());
    impulseOutList = QVector <ImpulseOut*>(data->impulseOutList.size());
    for (int i=0; i<data->StateAssignList.size(); i++)
    {
        StateAssignList[i] = new StateAssignment(data->StateAssignList[i]);
    }
    for (int i=0; i<data->eventOutList.size(); i++)
    {
        eventOutList[i] = new EventOut(data->eventOutList[i]);
    }
    for (int i=0; i<data->impulseOutList.size(); i++)
    {
        impulseOutList[i] = new ImpulseOut(data->impulseOutList[i]);
    }
}

OnCondition::~OnCondition()
{
    delete trigger;
    for (int i=0; i<StateAssignList.size(); i++)
    {
        delete StateAssignList[i];
    }
    for (int i=0; i<eventOutList.size(); i++)
    {
        delete eventOutList[i];
    }
    for (int i=0; i<impulseOutList.size(); i++)
    {
        delete impulseOutList[i];
    }
}

OnEvent::OnEvent(OnEvent *data)
{
    target_regime = NULL;
    this->target_regime_name = data->target_regime_name;
    src_port = NULL;
    this->src_port_name = data->src_port_name;
    StateAssignList = QVector <StateAssignment*>(data->StateAssignList.size());
    eventOutList = QVector <EventOut*>(data->eventOutList.size());
    impulseOutList = QVector <ImpulseOut*>(data->impulseOutList.size());
    for (int i=0; i<data->StateAssignList.size(); i++)
    {
        StateAssignList[i] = new StateAssignment(data->StateAssignList[i]);
    }
    for (int i=0; i<data->eventOutList.size(); i++)
    {
        eventOutList[i] = new EventOut(data->eventOutList[i]);
    }
    for (int i=0; i<data->impulseOutList.size(); i++)
    {
        impulseOutList[i] = new ImpulseOut(data->impulseOutList[i]);
    }

}

OnEvent::~OnEvent()
{
    for (int i=0; i<StateAssignList.size(); i++)
    {
        delete StateAssignList[i];
    }
    for (int i=0; i<eventOutList.size(); i++)
    {
        delete eventOutList[i];
    }
    for (int i=0; i<impulseOutList.size(); i++)
    {
        delete impulseOutList[i];
    }
}

OnImpulse::OnImpulse(OnImpulse *data)
{
    target_regime = NULL;
    this->target_regime_name = data->target_regime_name;
    src_port = NULL;
    this->src_port_name = data->src_port_name;
    StateAssignList = QVector <StateAssignment*>(data->StateAssignList.size());
    eventOutList = QVector <EventOut*>(data->eventOutList.size());
    impulseOutList = QVector <ImpulseOut*>(data->impulseOutList.size());
    for (int i=0; i<data->StateAssignList.size(); i++)
    {
        StateAssignList[i] = new StateAssignment(data->StateAssignList[i]);
    }
    for (int i=0; i<data->eventOutList.size(); i++)
    {
        eventOutList[i] = new EventOut(data->eventOutList[i]);
    }
    for (int i=0; i<data->impulseOutList.size(); i++)
    {
        impulseOutList[i] = new ImpulseOut(data->impulseOutList[i]);
    }

}

OnImpulse::~OnImpulse()
{
    for (int i=0; i<StateAssignList.size(); i++)
    {
        delete StateAssignList[i];
    }
    for (int i=0; i<eventOutList.size(); i++)
    {
        delete eventOutList[i];
    }
    for (int i=0; i<impulseOutList.size(); i++)
    {
        delete impulseOutList[i];
    }
}

Regime::Regime(Regime *data)
{
    name = data->name;
    TimeDerivativeList = QVector <TimeDerivative*>(data->TimeDerivativeList.size());
    OnConditionList = QVector <OnCondition*>(data->OnConditionList.size());
    OnEventList = QVector <OnEvent*>(data->OnEventList.size());
    OnImpulseList = QVector <OnImpulse*>(data->OnImpulseList.size());
    for (int i=0; i<data->TimeDerivativeList.size(); i++)
    {
        TimeDerivativeList[i] = new TimeDerivative(data->TimeDerivativeList[i]);
    }
    for (int i=0; i<data->OnConditionList.size(); i++)
    {
        OnConditionList[i] = new OnCondition(data->OnConditionList[i]);
    }
    for (int i=0; i<data->OnEventList.size(); i++)
    {
        OnEventList[i] = new OnEvent(data->OnEventList[i]);
    }
    for (int i=0; i<data->OnImpulseList.size(); i++)
    {
        OnImpulseList[i] = new OnImpulse(data->OnImpulseList[i]);
    }
}

Regime::~Regime()
{
    for (int i=0; i<TimeDerivativeList.size(); i++)
    {
        delete TimeDerivativeList[i];
        TimeDerivativeList[i] = NULL;
    }
    for (int i=0; i<OnConditionList.size(); i++)
    {
        delete OnConditionList[i];
        OnConditionList[i] = NULL;
    }
    for (int i=0; i<OnEventList.size(); i++)
    {
        delete OnEventList[i];
        OnEventList[i] = NULL;
    }
    for (int i=0; i<OnImpulseList.size(); i++)
    {
        delete OnImpulseList[i];
        OnImpulseList[i] = NULL;
    }
}

NineMLComponent::NineMLComponent()
{
    initial_regime = NULL;
    editedVersion.clear();
    path = "temp";
    type = "neuron_body";
}

NineMLComponent::NineMLComponent(QSharedPointer<NineMLComponent>data)
{
    name = data->name;
    this->type = data->type;
    this->path = data->path;
    this->filePath = data->filePath;
    this->initial_regime_name = data->initial_regime_name;
    RegimeList = QVector <Regime*>(data->RegimeList.size());
    StateVariableList = QVector <StateVariable*>(data->StateVariableList.size());
    ParameterList = QVector <Parameter*>(data->ParameterList.size());
    AliasList = QVector <Alias*>(data->AliasList.size());
    AnalogPortList = QVector <AnalogPort*>(data->AnalogPortList.size());
    EventPortList = QVector <EventPort*>(data->EventPortList.size());
    ImpulsePortList = QVector <ImpulsePort*>(data->ImpulsePortList.size());
    for (int i=0; i<data->RegimeList.size(); i++)
    {
        RegimeList[i] = new Regime(data->RegimeList[i]);
    }
    for (int i=0; i<data->StateVariableList.size(); i++)
    {
        StateVariableList[i] = new StateVariable(data->StateVariableList[i]);
    }
    for (int i=0; i<data->ParameterList.size(); i++)
    {
        ParameterList[i] = new Parameter(data->ParameterList[i]);
    }
    for (int i=0; i<data->AliasList.size(); i++)
    {
        AliasList[i] = new Alias(data->AliasList[i]);
    }
    for (int i=0; i<data->AnalogPortList.size(); i++)
    {
        AnalogPortList[i] = new AnalogPort(data->AnalogPortList[i]);
    }
    for (int i=0; i<data->EventPortList.size(); i++)
    {
        EventPortList[i] = new EventPort(data->EventPortList[i]);
    }
    for (int i=0; i<data->ImpulsePortList.size(); i++)
    {
        ImpulsePortList[i] = new ImpulsePort(data->ImpulsePortList[i]);
    }
    //validate this
    QStringList errs = validateComponent();
    // check for errors:
    QSettings settings;
    int num_errs = settings.beginReadArray("errors");
    settings.endArray();

    num_errs += settings.beginReadArray("warnings");
    settings.endArray();

    QString errors;

    if (num_errs != 0) {

        errors = errors + "<b>Errors found in current component:</b><br/><br/>";

        // list errors
        settings.beginReadArray("errors");
        for (int j = 1; j < num_errs; ++j) {
            settings.setArrayIndex(j);
            errors = errors + settings.value("errorText", "").toString();
            errors = errors + "<br/>";
        }
        settings.endArray();
        settings.beginReadArray("warnings");
        for (int j = 1; j < num_errs; ++j) {
            settings.setArrayIndex(j);
            errors = errors + settings.value("warnText", "").toString();
            errors = errors + "<br/>";
        }
        settings.endArray();

        // clear errors
        settings.remove("errors");
        settings.remove("warnings");

    }

    /*if (!errors.isEmpty()) {
        // display errors
        QMessageBox msgBox;
        msgBox.setText("<P><b>Component validation failed</b></P>" + errors);
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setTextFormat(Qt::RichText);
        msgBox.setModal(false);
        msgBox.exec();
        return;
    }*/

    // don't copy this...
    editedVersion.clear();
}

NineMLComponent::~NineMLComponent()
{
    foreach (Regime *r, RegimeList)
    {
        delete r;
        r = NULL;
    }
    foreach (StateVariable *sv, StateVariableList)
    {
        delete sv;
        sv = NULL;
    }
    foreach (Parameter *p, ParameterList)
    {
        delete p;
        p = NULL;
    }
    foreach (Alias *a, AliasList)
    {
        delete a;
        a = NULL;
    }
    foreach (AnalogPort *ap, AnalogPortList)
    {
        delete ap;
        ap = NULL;
    }
    foreach (EventPort *ep, EventPortList)
    {
        delete ep;
        ep = NULL;
    }
    foreach (ImpulsePort *ip, ImpulsePortList)
    {
        delete ip;
        ip = NULL;
    }

    // decrement the shared pointer
    if (!editedVersion.isNull()) {
        editedVersion.clear();
    }
}

// assignment operator required for the base class
NineMLComponent& NineMLComponent::operator=(const NineMLComponent& data)
{
    name = data.name;
    type = data.type;
    path = data.path;
    filePath = data.filePath;
    RegimeList = QVector <Regime*>(data.RegimeList.size());
    StateVariableList = QVector <StateVariable*>(data.StateVariableList.size());
    ParameterList = QVector <Parameter*>(data.ParameterList.size());
    AliasList = QVector <Alias*>(data.AliasList.size());
    AnalogPortList = QVector <AnalogPort*>(data.AnalogPortList.size());
    EventPortList = QVector <EventPort*>(data.EventPortList.size());
    ImpulsePortList = QVector <ImpulsePort*>(data.ImpulsePortList.size());
    initial_regime_name = data.initial_regime_name;
    for (int i=0; i<data.RegimeList.size(); i++)
    {
        RegimeList[i] = new Regime(data.RegimeList[i]);
    }
    for (int i=0; i<data.StateVariableList.size(); i++)
    {
        StateVariableList[i] = new StateVariable(data.StateVariableList[i]);
    }
    for (int i=0; i<data.ParameterList.size(); i++)
    {
        ParameterList[i] = new Parameter(data.ParameterList[i]);
    }
    for (int i=0; i<data.AliasList.size(); i++)
    {
        AliasList[i] = new Alias(data.AliasList[i]);
    }
    for (int i=0; i<data.AnalogPortList.size(); i++)
    {
        AnalogPortList[i] = new AnalogPort(data.AnalogPortList[i]);
    }
    for (int i=0; i<data.EventPortList.size(); i++)
    {
        EventPortList[i] = new EventPort(data.EventPortList[i]);
    }
    for (int i=0; i<data.ImpulsePortList.size(); i++)
    {
        ImpulsePortList[i] = new ImpulsePort(data.ImpulsePortList[i]);
    }
    QStringList validated = validateComponent();
    // check for errors:
    QSettings settings;
    int num_errs = settings.beginReadArray("errors");
    settings.endArray();

    num_errs += settings.beginReadArray("warnings");
    settings.endArray();

    QString errors;

    if (num_errs != 0) {

        errors = errors + "<b>Errors found in current component:</b><br/><br/>";

        // list errors
        settings.beginReadArray("errors");
        for (int j = 1; j < num_errs; ++j) {
            settings.setArrayIndex(j);
            errors = errors + settings.value("errorText", "").toString();
            errors = errors + "<br/>";
        }
        settings.endArray();
        settings.beginReadArray("warnings");
        for (int j = 1; j < num_errs; ++j) {
            settings.setArrayIndex(j);
            errors = errors + settings.value("warnText", "").toString();
            errors = errors + "<br/>";
        }
        settings.endArray();

        // clear errors
        settings.remove("errors");
        settings.remove("warnings");

    }

    if (!errors.isEmpty()) {
        // display errors
        QMessageBox msgBox;
        msgBox.setText("<P><b>Component validation failed</b></P>" + errors);
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setTextFormat(Qt::RichText);
        msgBox.setModal(false);
        msgBox.exec();
    }

    return *this;
}

void NineMLComponent::updateFrom(QSharedPointer<NineMLComponent>  data)
{
    // remove existing data
    foreach (Regime *r, RegimeList)
    {
        delete r;
        r = NULL;
    }
    foreach (StateVariable *sv, StateVariableList)
    {
        delete sv;
        sv = NULL;
    }
    foreach (Parameter *p, ParameterList)
    {
        delete p;
        p = NULL;
    }
    foreach (Alias *a, AliasList)
    {
        delete a;
        a = NULL;
    }
    foreach (AnalogPort *ap, AnalogPortList)
    {
        delete ap;
        ap = NULL;
    }
    foreach (EventPort *ep, EventPortList)
    {
        delete ep;
        ep = NULL;
    }
    foreach (ImpulsePort *ip, ImpulsePortList)
    {
        delete ip;
        ip = NULL;
    }

    // decrement shared pointer
    if (!editedVersion.isNull()) {
        editedVersion.clear();
    }

    name = data->name;
    type = data->type;
    path = data->path;
    filePath = data->filePath;
    initial_regime_name = data->initial_regime_name;

    RegimeList = QVector <Regime*>(data->RegimeList.size());
    StateVariableList = QVector <StateVariable*>(data->StateVariableList.size());
    ParameterList = QVector <Parameter*>(data->ParameterList.size());
    AliasList = QVector <Alias*>(data->AliasList.size());
    AnalogPortList = QVector <AnalogPort*>(data->AnalogPortList.size());
    EventPortList = QVector <EventPort*>(data->EventPortList.size());
    ImpulsePortList = QVector <ImpulsePort*>(data->ImpulsePortList.size());
    for (int i=0; i<data->RegimeList.size(); i++)
    {
        RegimeList[i] = new Regime(data->RegimeList[i]);
    }
    for (int i=0; i<data->StateVariableList.size(); i++)
    {
        StateVariableList[i] = new StateVariable(data->StateVariableList[i]);
    }
    for (int i=0; i<data->ParameterList.size(); i++)
    {
        ParameterList[i] = new Parameter(data->ParameterList[i]);
    }
    for (int i=0; i<data->AliasList.size(); i++)
    {
        AliasList[i] = new Alias(data->AliasList[i]);
    }
    for (int i=0; i<data->AnalogPortList.size(); i++)
    {
        AnalogPortList[i] = new AnalogPort(data->AnalogPortList[i]);
    }
    for (int i=0; i<data->EventPortList.size(); i++)
    {
        EventPortList[i] = new EventPort(data->EventPortList[i]);
    }
    for (int i=0; i<data->ImpulsePortList.size(); i++)
    {
        ImpulsePortList[i] = new ImpulsePort(data->ImpulsePortList[i]);
    }

    // validate to fill in blanks
    QStringList validated = validateComponent();
    // check for errors:
    QSettings settings;
    int num_errs = settings.beginReadArray("errors");
    settings.endArray();

    num_errs += settings.beginReadArray("warnings");
    settings.endArray();

    // clear errors if any
    settings.remove("errors");
    settings.remove("warnings");
}

// copy constructor required for the base class
NineMLComponentData::NineMLComponentData(QSharedPointer<NineMLComponent>data)
{

    type = NineMLComponentType;
    StateVariableList = QVector <StateVariableData*>(data->StateVariableList.size());
    ParameterList = QVector <ParameterData*>(data->ParameterList.size());
    for (int i=0; i<data->StateVariableList.size(); i++)
    {
        StateVariableList[i] = new StateVariableData(data->StateVariableList[i]);
    }
    for (int i=0; i<data->ParameterList.size(); i++)
    {
        ParameterList[i] = new ParameterData(data->ParameterList[i]);
    }
    this->component = data;
}

// duplicate
NineMLComponentData::NineMLComponentData(QSharedPointer <NineMLComponentData>data)
{

    type = NineMLComponentType;
    StateVariableList = QVector <StateVariableData*>(data->StateVariableList.size());
    ParameterList = QVector <ParameterData*>(data->ParameterList.size());
    for (int i=0; i<data->StateVariableList.size(); i++)
    {
        StateVariableList[i] = new StateVariableData(data->StateVariableList[i]);
    }
    for (int i=0; i<data->ParameterList.size(); i++)
    {
        ParameterList[i] = new ParameterData(data->ParameterList[i]);
    }

    // we don't copy inputs / outputs or owner

    // copy component reference
    this->component = data->component;
}

void NineMLComponentData::copyFrom(QSharedPointer <NineMLComponentData>src, QSharedPointer<NineMLComponent>data, QSharedPointer<NineMLComponentData> thisSharedPointer)
{

    // copy owner
    owner = src->owner;


    //copy inputs and outputs across and delete originals:
    for (int i = 0; i < src->inputs.size(); ++i) {
        inputs.push_back(src->inputs[i]);
    }
    src->inputs.clear();

    for (int i = 0; i < src->outputs.size(); ++i) {
        outputs.push_back(src->outputs[i]);
    }
    src->outputs.clear();

    // update reference in outputs:
    for (int i = 0; i < outputs.size(); ++i) {
        outputs[i]->src = thisSharedPointer;
        // remove the port name reference
        outputs[i]->srcPort.clear();
    }

    // update reference in inputs:
    for (int i = 0; i < inputs.size(); ++i) {
        inputs[i]->dst = thisSharedPointer;
        // remove dstPort name reference
        this->inputs[i]->dstPort.clear();
    }

    type = NineMLComponentType;
    StateVariableList = QVector <StateVariableData*>(data->StateVariableList.size());
    ParameterList = QVector <ParameterData*>(data->ParameterList.size());
    for (int i=0; i<data->StateVariableList.size(); i++)
    {
        StateVariableList[i] = new StateVariableData(data->StateVariableList[i]);
    }
    for (int i=0; i<data->ParameterList.size(); i++)
    {
        ParameterList[i] = new ParameterData(data->ParameterList[i]);
    }

    this->component = data;

    // lastly, match up input ports
    this->matchPorts();

    // also do this on outputs:
    for (int i = 0; i < this->outputs.size(); ++i) {
        this->outputs[i]->dst->matchPorts();
    }
}

// assignment operator required for the base class
/*NineMLComponentData& NineMLComponentData::operator=(const NineMLComponentData& data)
{
    StateVariableList = QVector <StateVariableData*>(data.StateVariableList.size());
    ParameterList = QVector <ParameterData*>(data.ParameterList.size());
    AliasList = QVector <AliasData*>(data.AliasList.size());
    for (int i=0; i<data.StateVariableList.size(); i++)
    {
        StateVariableList[i] = new StateVariable(data.StateVariableList[i]);
    }
    for (int i=0; i<data.ParameterList.size(); i++)
    {
        ParameterList[i] = new Parameter(data.ParameterList[i]);
    }
    for (int i=0; i<data.AliasList.size(); i++)
    {
        AliasList[i] = new Alias(data.AliasList[i]);
    }

    return *this;
}*/

void MathInLine::validateMathSetup(QString& testequation, QStringList& FuncList)
{
    testequation = equation;
    testequation.replace('+', ' ');
    testequation = testequation.replace('-', ' ');
    testequation = testequation.replace('*', ' ');
    testequation = testequation.replace('/', ' ');
    testequation = testequation.replace('(', ' ');
    testequation = testequation.replace(')', ' ');
    testequation = testequation.replace('<', ' ');
    testequation = testequation.replace('>', ' ');
    testequation = testequation.replace('=', ' ');
    testequation = testequation.replace(',', ' ');
    testequation = testequation.replace('&', ' ');
    testequation = testequation.replace('|', ' ');
    testequation = testequation.replace('!', ' ');

    // remove double whitespaces
    testequation = testequation.simplified();

    FuncList.push_back("pow");
    FuncList.push_back("exp");
    FuncList.push_back("sin");
    FuncList.push_back("cos");
    FuncList.push_back("log");
    FuncList.push_back("log10");
    FuncList.push_back("sinh");
    FuncList.push_back("cosh");
    FuncList.push_back("tanh");
    FuncList.push_back("sqrt");
    FuncList.push_back("atan");
    FuncList.push_back("asin");
    FuncList.push_back("acos");
    FuncList.push_back("asinh");
    FuncList.push_back("acosh");
    FuncList.push_back("atanh");
    FuncList.push_back("atan2");
    FuncList.push_back("ceil");
    FuncList.push_back("floor");
    FuncList.push_back("randomUniform");
    FuncList.push_back("randomNormal");
    FuncList.push_back("randomExponential");
    FuncList.push_back("rand");
    FuncList.push_back("mod");

    // not strictly functions...
    FuncList.push_back("t");
    FuncList.push_back("dt");
}

int MathInLine::validateMathInLine(NineMLComponent* component, QStringList * )
{
    if (equation.size() == 0) {
        return 1;
    }

    QString test = equation;
    QStringList FuncList;
    this->validateMathSetup (test, FuncList);

    // tokenise
    QStringList splitTest;
    splitTest = test.split(' ');

    // check each token...
    for (int i = 0; i < (int) splitTest.count(); ++i) {

        bool recognised = false;

        // see if it is in the component
        for (int j = 0; j<component->ParameterList.size(); j++) {
            if (component->ParameterList[j]->name.compare(splitTest[i]) == 0)
                recognised = true;
        }
        for (int j = 0; j<component->StateVariableList.size(); j++) {
            if (component->StateVariableList[j]->name.compare(splitTest[i]) == 0)
                recognised = true;
        }
        for (int j = 0; j<component->AliasList.size(); j++) {
            if (component->AliasList[j]->name.compare(splitTest[i]) == 0)
                recognised = true;
        }
        for (int j = 0; j<component->AnalogPortList.size(); j++) {
            if (component->AnalogPortList[j]->name.compare(splitTest[i]) == 0)
                recognised = true;
        }
        for (int j = 0; j<component->ImpulsePortList.size(); j++) {
            if (component->ImpulsePortList[j]->name.compare(splitTest[i]) == 0)
                recognised = true;
        }
        for (int j = 0; j<(int)FuncList.size(); j++) {
            if (FuncList[j].compare(splitTest[i]) == 0)
                recognised = true;
        }
        // see if it is a number...
        if (splitTest[i][0] > 47 && splitTest[i][0] < 58 && splitTest[i][0] != '.') {
            recognised = true;
        }

        // if a token is not recognised, then let the user know - this may be better done elsewhere...
        if (!recognised) {
          QSettings settings;
          int num_errs = settings.beginReadArray("warnings");
          settings.endArray();
          settings.beginWriteArray("warnings");
              settings.setArrayIndex(num_errs + 1);
              settings.setValue("warnText",  "Warning: MathInLine contains unrecognised token " + splitTest[i]);
          settings.endArray();
        }
    }

    if (equation.count("(") != equation.count(")")) {
        QSettings settings;
        int num_errs = settings.beginReadArray("warnings");
        settings.endArray();
        settings.beginWriteArray("warnings");
            settings.setArrayIndex(num_errs + 1);
            settings.setValue("warnText",  "Warning: MathInLine contains mis-matched brackets");
        settings.endArray();
    }

    return 0;
}

int Trigger::validateTrigger(NineMLComponent * component, QStringList * errs)
{
    // mathinline pointer may be null
    if (maths != (MathInLine *)0) {
        return maths->validateMathInLine(component, errs);
    } else {
        // should never get here - is a major error
        errs->push_back("MathInline missing from Trigger");
        return 1;
    }
}

int Alias::validateAlias(NineMLComponent * component, QStringList * errs)
{
    // mathinline pointer may be null
    if (maths != (MathInLine *)0) {
        return maths->validateMathInLine(component, errs);
    } else {
        errs->push_back("MathInline missing from Alias");
        return 1;
    }
}

int TimeDerivative::validateTimeDerivative(NineMLComponent * component, QStringList * errs)
{
    int failures = 0;
    // mathinline pointer may be null
    if (maths != (MathInLine *)0) {
        failures += maths->validateMathInLine(component, errs);
    } else {
        errs->push_back("MathInline missing from TimeDerivative");
        failures += 1;
    }
    bool match = false;
    for(int i=0; i<component->StateVariableList.size(); i++)
    {
        if (component->StateVariableList[i]->getName() == variable_name)
        {variable = component->StateVariableList[i];
            match = true;}

    }
    if (!match) {
      QSettings settings;
      int num_errs = settings.beginReadArray("errors");
      settings.endArray();
      settings.beginWriteArray("errors");
          settings.setArrayIndex(num_errs + 1);
          settings.setValue("errorText",  "Error: TimeDerivative references missing StateVariable " + variable_name);
      settings.endArray();
    }
    return failures;
}

int StateAssignment::validateStateAssignment(NineMLComponent * component, QStringList * errs)
{
    int failures = 0;
    // mathinline pointer may be null
    if (maths != (MathInLine *)0) {
        failures += maths->validateMathInLine(component, errs);
    } else {
        // should never get here - is a major error
        QSettings settings;
        int num_errs = settings.beginReadArray("errors");
        settings.endArray();
        settings.beginWriteArray("errors");
            settings.setArrayIndex(num_errs + 1);
            settings.setValue("errorText",   "Error: MathInline missing from State Assignment");
        settings.endArray();
    }
    bool match = false;
    for(int i=0; i<component->StateVariableList.size(); i++)
    {
        if (component->StateVariableList[i]->name.compare(name) == 0)
        {variable = component->StateVariableList[i];
            match = true;}

    }
    if (!match) {
        QSettings settings;
        int num_errs = settings.beginReadArray("errors");
        settings.endArray();
        settings.beginWriteArray("errors");
            settings.setArrayIndex(num_errs + 1);
            settings.setValue("errorText",   "Error: StateAssignment references missing StateVariable " + name);
        settings.endArray();
      }
    return failures;
}

int EventOut::validateEventOut(NineMLComponent * component, QStringList * )
{
    int failures = 0;
    bool match = false;
    for(int i=0; i<component->EventPortList.size(); i++)
    {
        if (component->EventPortList[i]->name.compare(port_name) == 0)
        {port = component->EventPortList[i];
            match = true;}
    }
    if (!match) {
        QSettings settings;
        int num_errs = settings.beginReadArray("errors");
        settings.endArray();
        settings.beginWriteArray("errors");
            settings.setArrayIndex(num_errs + 1);
            settings.setValue("errorText",   "Error: EventOut references missing EventPort " + port_name);
        settings.endArray();
      }
    return failures;
}


int ImpulseOut::validateImpulseOut(NineMLComponent * component, QStringList * )
{
    int failures = 0;
    bool match = false;
    for(int i=0; i<component->ImpulsePortList.size(); i++)
    {
        if (component->ImpulsePortList[i]->name.compare(port_name) == 0)
        {
            if (component->ImpulsePortList[i]->mode == ImpulseSendPort){
                port = component->ImpulsePortList[i];
                match = true;
            }
        }
    }
    if (!match) {
        QSettings settings;
        int num_errs = settings.beginReadArray("errors");
        settings.endArray();
        settings.beginWriteArray("errors");
            settings.setArrayIndex(num_errs + 1);
            settings.setValue("errorText",   "Error: ImpulseOut references missing ImpulsePort " + port_name);
        settings.endArray();
      }
    failures += !match;
    return failures;
}

int OnCondition::validateOnCondition(NineMLComponent * component, QStringList * errs)
{
    int failures = 0;
    bool match = false;
    for(int i=0;i<component->RegimeList.size();i++)
    {
        if (component->RegimeList[i]->name.compare(target_regime_name) == 0)
        {target_regime = component->RegimeList[i];
            match = true;}
    }
    if (!match) {
        QSettings settings;
        int num_errs = settings.beginReadArray("errors");
        settings.endArray();
        settings.beginWriteArray("errors");
            settings.setArrayIndex(num_errs + 1);
            settings.setValue("errorText",   "Error: OnCondition references missing Regime " + target_regime_name);
        settings.endArray();
      }
    failures += !match;
    for(int i=0; i<StateAssignList.size(); i++)
    {
        failures += StateAssignList[i]->validateStateAssignment(component, errs);
    }
    for(int i=0; i<eventOutList.size(); i++)
    {
        failures += eventOutList[i]->validateEventOut(component, errs);
    }
    for(int i=0; i<impulseOutList.size(); i++)
    {
        failures += impulseOutList[i]->validateImpulseOut(component, errs);
    }
    return failures;
}

int OnEvent::validateOnEvent(NineMLComponent * component, QStringList * errs)
{
    int failures = 0;
    bool match = false;
    for(int i=0;i<component->RegimeList.size();i++)
    {
        if (component->RegimeList[i]->name.compare(target_regime_name) == 0)
        {target_regime = component->RegimeList[i];
            match = true;}
    }
    if (!match) {
        QSettings settings;
        int num_errs = settings.beginReadArray("errors");
        settings.endArray();
        settings.beginWriteArray("errors");
            settings.setArrayIndex(num_errs + 1);
            settings.setValue("errorText",   "Error: OnEvent references missing Regime " + target_regime_name);
        settings.endArray();
      }
    failures += !match;
    match = false;
    for(int i=0; i<component->EventPortList.size(); i++)
    {
        if (component->EventPortList[i]->name.compare(src_port_name) == 0)
        {src_port = component->EventPortList[i];
            match = true;}
    }
    if (!match) {
        QSettings settings;
        int num_errs = settings.beginReadArray("errors");
        settings.endArray();
        settings.beginWriteArray("errors");
            settings.setArrayIndex(num_errs + 1);
            settings.setValue("errorText",   "Error: OnEvent references missing EventPort " + src_port_name);
        settings.endArray();
      }
    failures += !match;
    for(int i=0; i<StateAssignList.size(); i++)
    {
        failures += StateAssignList[i]->validateStateAssignment(component, errs);
    }
    for(int i=0; i<eventOutList.size(); i++)
    {
        failures += eventOutList[i]->validateEventOut(component, errs);
    }
    for(int i=0; i<impulseOutList.size(); i++)
    {
        failures += impulseOutList[i]->validateImpulseOut(component, errs);
    }
    return failures;
}


int OnImpulse::validateOnImpulse(NineMLComponent * component, QStringList * errs)
{
    int failures = 0;
    bool match = false;
    for(int i=0;i<component->RegimeList.size();i++)
    {
        if (component->RegimeList[i]->name.compare(target_regime_name) == 0)
        {target_regime = component->RegimeList[i];
            match = true;}
    }
    if (!match) {
        QSettings settings;
        int num_errs = settings.beginReadArray("errors");
        settings.endArray();
        settings.beginWriteArray("errors");
            settings.setArrayIndex(num_errs + 1);
            settings.setValue("errorText",   "Error: OnImpulse references missing Regime " + target_regime_name);
        settings.endArray();
      }
    failures += !match;
    match = false;
    for(int i=0; i<component->ImpulsePortList.size(); i++)
    {
        if (component->ImpulsePortList[i]->name.compare(src_port_name) == 0)
        {src_port = component->ImpulsePortList[i];
            match = true;}
    }
    if (!match) {
        QSettings settings;
        int num_errs = settings.beginReadArray("errors");
        settings.endArray();
        settings.beginWriteArray("errors");
            settings.setArrayIndex(num_errs + 1);
            settings.setValue("errorText",   "Error: OnImpulse references missing Regime " + src_port_name);
        settings.endArray();
      }
    for(int i=0; i<StateAssignList.size(); i++)
    {
        failures += StateAssignList[i]->validateStateAssignment(component, errs);
    }
    for(int i=0; i<eventOutList.size(); i++)
    {
        failures += eventOutList[i]->validateEventOut(component, errs);
    }
    for(int i=0; i<impulseOutList.size(); i++)
    {
        failures += impulseOutList[i]->validateImpulseOut(component, errs);
    }
    return failures;
}

int Regime::validateRegime(NineMLComponent * component, QStringList * errs)
{
    int failures = 0;
    for(int i=0; i<TimeDerivativeList.size(); i++)
    {
        failures += TimeDerivativeList[i]->validateTimeDerivative(component, errs);
    }
    for(int i=0; i<OnConditionList.size(); i++)
    {
        failures += OnConditionList[i]->validateOnCondition(component, errs);
    }
    for(int i=0; i<OnEventList.size(); i++)
    {
        failures += OnEventList[i]->validateOnEvent(component, errs);
    }
    for(int i=0; i<OnImpulseList.size(); i++)
    {
        failures += OnImpulseList[i]->validateOnImpulse(component, errs);
    }
    return failures;
}

QStringList NineMLComponent::validateComponent()
{
    QStringList errs;
    bool initial_regime_match = false;
    this->initial_regime = NULL;
    int failures = 0;

    for(int i=0; i<RegimeList.size(); i++)
    {
        failures += RegimeList[i]->validateRegime(this, &errs);
        if (this->initial_regime_name == RegimeList[i]->name){
            initial_regime_match = true;
            this->initial_regime = RegimeList[i];
        }
    }
    for(int i=0; i<AliasList.size(); i++)
    {
        failures += AliasList[i]->validateAlias(this, &errs);
    }
    for (int i=0; i<AnalogPortList.size(); ++i)
    {
        failures += AnalogPortList[i]->validateAnalogPort(this, &errs);
    }
    for (int i=0; i<ImpulsePortList.size(); ++i)
    {
        failures += ImpulsePortList[i]->validateImpulsePort(this, &errs);
    }

    if (!initial_regime_match){
        if (RegimeList.size() > 0) {
            this->initial_regime_name = RegimeList[0]->name;
            this->initial_regime = RegimeList[0];
        }
    }

    errs.push_back("Total errors: " + QString::number(float(failures)));

    return errs;
}

void NineMLComponentData::import_parameters_from_xml(QDomNode &n)
{
    type = NineMLComponentType;
    QDomNodeList nList = n.toElement().elementsByTagName("Property");

    for (int j = 0; j < nList.count(); ++j) {
        QDomNode n = nList.item(j);

        // extract value and dimensions from node
        for (int i = 0; i < this->ParameterList.size(); ++i) {
            if (n.toElement().attribute("name") == this->ParameterList[i]->name) {

                QDomNodeList propVal = n.toElement().elementsByTagName("FixedValue");
                if (propVal.size() == 1) {
                    this->ParameterList[i]->currType = FixedValue;
                    this->ParameterList[i]->value.resize(1);
                    this->ParameterList[i]->value.fill(0);
                    this->ParameterList[i]->value[0] = propVal.item(0).toElement().attribute("value").toFloat();
                }
                propVal = n.toElement().elementsByTagName("UniformDistribution");
                if (propVal.size() == 1) {
                    this->ParameterList[i]->currType = Statistical;
                    this->ParameterList[i]->value.resize(4);
                    this->ParameterList[i]->value.fill(0);
                    this->ParameterList[i]->value[0] = 1;
                    this->ParameterList[i]->value[1] = propVal.item(0).toElement().attribute("minimum").toFloat();
                    this->ParameterList[i]->value[2] = propVal.item(0).toElement().attribute("maximum").toFloat();
                    this->ParameterList[i]->value[3] = propVal.item(0).toElement().attribute("seed").toFloat();
                }
                propVal = n.toElement().elementsByTagName("NormalDistribution");
                if (propVal.size() == 1) {
                    this->ParameterList[i]->currType = Statistical;
                    this->ParameterList[i]->value.resize(4);
                    this->ParameterList[i]->value.fill(0);
                    this->ParameterList[i]->value[0] = 2;
                    this->ParameterList[i]->value[1] = propVal.item(0).toElement().attribute("mean").toFloat();
                    this->ParameterList[i]->value[2] = propVal.item(0).toElement().attribute("variance").toFloat();
                    this->ParameterList[i]->value[3] = propVal.item(0).toElement().attribute("seed").toFloat();
                }

                propVal = n.toElement().elementsByTagName("ValueList");
                if (propVal.size() == 1) {
                    this->ParameterList[i]->currType = ExplicitList;
                    // now use the Parameter or StateVariable method to read the data in
                    QDomNode n2 = propVal.item(0);
                    this->ParameterList[i]->readExplicitListNodeData(n2);
                    /*
                    QDomNodeList propValInst = n.toElement().elementsByTagName("Value");
                    for (int ind = 0; ind < (int) propValInst.count(); ++ind) {
                        this->ParameterList[i]->indices.push_back(propValInst.item(ind).toElement().attribute("index").toInt());
                        this->ParameterList[i]->value.push_back(propValInst.item(ind).toElement().attribute("value").toFloat());
//                        this->ParameterList[i]->dims = new dim(propUnit.item(0).toElement().text());
                    }
                    */
                }
            }
        }

        // extract value and dimensions from node
        for (int i = 0; i < this->StateVariableList.size(); ++i) {
            if (n.toElement().attribute("name") == this->StateVariableList[i]->name) {

                QDomNodeList propVal = n.toElement().elementsByTagName("FixedValue");
                if (propVal.size() == 1) {
                    this->StateVariableList[i]->currType = FixedValue;
                    this->StateVariableList[i]->value.resize(1);
                    this->StateVariableList[i]->value.fill(0);
                    this->StateVariableList[i]->value[0] = propVal.item(0).toElement().attribute("value").toFloat();
                }
                propVal = n.toElement().elementsByTagName("UniformDistribution");
                if (propVal.size() == 1) {
                    this->StateVariableList[i]->currType = Statistical;
                    this->StateVariableList[i]->value.resize(4);
                    this->StateVariableList[i]->value.fill(0);
                    this->StateVariableList[i]->value[0] = 1;
                    this->StateVariableList[i]->value[1] = propVal.item(0).toElement().attribute("minimum").toFloat();
                    this->StateVariableList[i]->value[2] = propVal.item(0).toElement().attribute("maximum").toFloat();
                    this->StateVariableList[i]->value[3] = propVal.item(0).toElement().attribute("seed").toFloat();
                }
                propVal = n.toElement().elementsByTagName("NormalDistribution");
                if (propVal.size() == 1) {
                    this->StateVariableList[i]->currType = Statistical;
                    this->StateVariableList[i]->value.resize(4);
                    this->StateVariableList[i]->value.fill(0);
                    this->StateVariableList[i]->value[0] = 2;
                    this->StateVariableList[i]->value[1] = propVal.item(0).toElement().attribute("mean").toFloat();
                    this->StateVariableList[i]->value[2] = propVal.item(0).toElement().attribute("variance").toFloat();
                    this->StateVariableList[i]->value[3] = propVal.item(0).toElement().attribute("seed").toFloat();
                }

                propVal = n.toElement().elementsByTagName("ValueList");
                if (propVal.size() == 1) {
                    this->StateVariableList[i]->currType = ExplicitList;
                    // now use the Parameter or StateVariable method to read the data in
                    QDomNode n2 = propVal.item(0);
                    this->StateVariableList[i]->readExplicitListNodeData(n2);
                    /*
                    QDomNodeList propValInst = n.toElement().elementsByTagName("Value");
                    for (int ind = 0; ind < (int) propValInst.count(); ++ind) {
                        this->StateVariableList[i]->indices.push_back(propValInst.item(ind).toElement().attribute("index").toInt());
                        this->StateVariableList[i]->value.push_back(propValInst.item(ind).toElement().attribute("value").toFloat());
//                        this->StateVariableList[i]->dims = new dim(propUnit.item(0).toElement().text());
                    }
                    */
                }

            }
        }
    }
}

QString NineMLComponent::getXMLName() {

    QString nameSanitised = name;
    nameSanitised.replace(" ", "_");
    return nameSanitised + ".xml";

}

QString NineMLComponentData::getXMLName() {

    // generate a unique name in order to link up ports in the XML

    if (this->component->type == "neuron_body") {
        return this->owner->getName();
    }
    if (this->component->type == "weight_update") {
        // find which Synapse we are attached to
        QSharedPointer <projection> projOwner = qSharedPointerDynamicCast <projection> (this->owner);
        CHECK_CAST(projOwner)
        for (int i = 0; i < projOwner->synapses.size(); ++i) {
            if (projOwner->synapses[i]->weightUpdateType == this) {
                return this->owner->getName() + " Synapse " + QString::number(float(i)) + " weight_update";
            }
        }
    }
    if (this->component->type == "postsynapse") {
        // find which Synapse we are attached to
        QSharedPointer <projection> projOwner = qSharedPointerDynamicCast <projection> (this->owner);
        CHECK_CAST(projOwner)
        for (int i = 0; i < projOwner->synapses.size(); ++i) {
            if (projOwner->synapses[i]->postsynapseType == this) {
                return this->owner->getName() + " Synapse " +  QString::number(float(i)) + " postsynapse";
            }
        }
    }
    if (this->component->type == "moo") {
        return "Reading the none from the Unsorted list - bad!";
    }
    return "error (" + this->component->name + ")";

}

void NineMLComponentData::removeReferences() {

    // remove the inputs (they'll take themselves off the vector)
    while (this->inputs.size()) {
        // delete
        this->inputs[0]->delAll(NULL);
    }

    // remove the reverse inputs (they'll take themselves off the vector)
    while (this->outputs.size()) {
        // delete
        this->outputs[0]->delAll(NULL);
    }

}

void NineMLComponentData::addInput(QSharedPointer <NineMLComponentData>, bool) {

    qDebug() << "This shouldn't be called - NineMLComponentData::addInput(QSharedPointer <NineMLComponentData> src, bool isProj)";

    //QSharedPointer<genericInput> newInput = new genericInput(src, this, isProj);

    // suppress warning
    //newInput = NULL;

}

NineMLComponentData::~NineMLComponentData() {

    // not needed anymore
    //this->removeReferences();

    /*for (int i = 0; i < this->inputs.size(); ++i) {
        delete inputs[i];
    }*/

    for (int i = 0; i < this->ParameterList.size(); ++i) {
        delete this->ParameterList[i];
    }

    for (int i = 0; i < this->StateVariableList.size(); ++i) {
        delete this->StateVariableList[i];
    }

    //qDebug() << "Deleting NineMLComponentData";
}

void NineMLComponentData::matchPorts() {

    // attempt to match by type and dimensions
    for (int i = 0; i < this->inputs.size();  ++i) {

        QStringList portPairs = this->getPortMatches(i, false);

        // check that port names are still valid - as switching components can invalidate them sometimes
        if (!inputs[i]->srcPort.isEmpty() && !inputs[i]->dstPort.isEmpty()) {
            bool isValid = false;
            for (int j = 0; j < portPairs.size(); ++j) {
                // skip if the port pair is still valid
                if (inputs[i]->srcPort + "->" + inputs[i]->dstPort == portPairs[j])
                    isValid = true;
            }
            if (isValid)
                continue;
            inputs[i]->srcPort = "";
            inputs[i]->dstPort = "";

        }

        else {

            // now we have all the matches select the first one:
            if (portPairs.size() > 0) {
                QString portPair = portPairs.at(0);
                QStringList ports = portPair.split("->");
                // for safety
                if (ports.size()>1) {
                    inputs[i]->srcPort = ports.at(0);
                    inputs[i]->dstPort = ports.at(1);
                }
            }
        }
    }

    // attempt to match by type and dimensions (outputs)
    for (int i = 0; i < this->outputs.size();  ++i) {

        if (outputs[i]->srcPort.isEmpty() || outputs[i]->dstPort.isEmpty()) {

            QStringList portPairs = this->getPortMatches(i, true);

            // now we have all the matches select the first one:
            if (portPairs.size() > 0) {
                QString portPair = portPairs.at(0);
                QStringList ports = portPair.split("->");
                // for safety
                if (ports.size()>1) {
                    outputs[i]->srcPort = ports.at(0);
                    outputs[i]->dstPort = ports.at(1);
                }
            }
        }
    }

}

QStringList NineMLComponentData::getPortMatches(int index, bool isOutput) {

    // find pairs of ports that could match
    QStringList portPairs;

    QSharedPointer<genericInput> currInput;
    if (!isOutput) {
        currInput = this->inputs[index];
    } else {
        currInput = this->outputs[index];
    }

    for (int j = 0; j < currInput->src->component->AnalogPortList.size(); ++j) {

        AnalogPort * currSendPort = currInput->src->component->AnalogPortList[j];

        // if is a send port
        if (currSendPort->mode == AnalogSendPort) {

            for (int k = 0; k < currInput->dst->component->AnalogPortList.size(); ++k) {

                AnalogPort * currRecvPort = currInput->dst->component->AnalogPortList[k];

                if (currRecvPort->mode == AnalogRecvPort || currRecvPort->mode == AnalogReducePort) {

                    // check the send port source dims against the recv port

                    if (currSendPort->variable->Type() == NINEML_ALIAS || currSendPort->variable->dims->toString() == currRecvPort->dims->toString()) {
                        // if they match then add to the list of possible pairings
                        QString portPair = currSendPort->name + "->" + currRecvPort->name;
                        portPairs.push_back(portPair);
                    }
                }
            }
        }
    }

    for (int j = 0; j < currInput->src->component->EventPortList.size(); ++j) {

        EventPort * currSendPort = currInput->src->component->EventPortList[j];

        // if is a send port
        if (currSendPort->mode == EventSendPort) {

            for (int k = 0; k < currInput->dst->component->EventPortList.size(); ++k) {

                EventPort * currRecvPort = currInput->dst->component->EventPortList[k];

                if (currRecvPort->mode == EventRecvPort) {

                    // event ports have no dimensions so always match
                    QString portPair = currSendPort->name + "->" + currRecvPort->name;
                    portPairs.push_back(portPair);

                }
            }
        }
    }

    for (int j = 0; j < currInput->src->component->ImpulsePortList.size(); ++j) {

        ImpulsePort * currSendPort = currInput->src->component->ImpulsePortList[j];

        // if is a send port
        if (currSendPort->mode == ImpulseSendPort) {

            for (int k = 0; k < currInput->dst->component->ImpulsePortList.size(); ++k) {

                ImpulsePort * currRecvPort = currInput->dst->component->ImpulsePortList[k];

                if (currRecvPort->mode == ImpulseRecvPort) {

                    // check the send port source dims against the recv port
                    if (currSendPort->parameter->Type() == NINEML_ALIAS || currSendPort->parameter->dims->toString() == currRecvPort->dims->toString()) {
                        // if they match then add to the list of possible pairings
                        QString portPair = currSendPort->name + "->" + currRecvPort->name;
                        portPairs.push_back(portPair);
                    }
                }
            }
        }
    }
    return portPairs;
}

void NineMLComponentData::migrateComponent(QSharedPointer<NineMLComponent> newComponent) {

    QVector < ParameterData * > oldParList;
    QVector < StateVariableData * > oldSVList;

    // copy old list
    oldParList = this->ParameterList;

    // clear parameter list ready for refilling
    this->ParameterList.clear();

    // add new list - copying across as needed
    for (int i = 0; i < newComponent->ParameterList.size(); ++i) {
        bool inNew = false;
        for (int j = 0; j < oldParList.size(); ++j) {
            if (newComponent->ParameterList[i]->name == oldParList[j]->name) {
                this->ParameterList.push_back(new ParameterData(oldParList[j]));
                // but may change dims!
                this->ParameterList.back()->dims->fromString(newComponent->ParameterList[i]->dims->toString());
                inNew = true;
            }
        }
        if (!inNew) {
            ParameterList.push_back(new ParameterData(newComponent->ParameterList[i]));
        }
    }

    // copy old list
    oldSVList = this->StateVariableList;

    // clear sv list ready for refilling
    this->StateVariableList.clear();

    // add new list - copying across as needed
    for (int i = 0; i < newComponent->StateVariableList.size(); ++i) {
        bool inNew = false;
        for (int j = 0; j < oldSVList.size(); ++j) {
            if (newComponent->StateVariableList[i]->name == oldSVList[j]->name) {
                this->StateVariableList.push_back(new StateVariableData(oldSVList[j]));
                // but may change dims!
                this->StateVariableList.back()->dims->fromString(newComponent->StateVariableList[i]->dims->toString());
                inNew = true;
            }
        }
        if (!inNew) {
            StateVariableList.push_back(new StateVariableData(newComponent->StateVariableList[i]));
        }
    }

    this->component = newComponent;

}

void NineMLComponentData::copyParsFrom(QSharedPointer <NineMLComponentData> data) {

    if (this->component->name == "none")
        return;

    // add new list - copying across as needed
    for (int i = 0; i < data->ParameterList.size(); ++i) {
        for (int j = 0; j < this->ParameterList.size(); ++j) {
            if (data->ParameterList[i]->name == this->ParameterList[j]->name) {
                dim * oldDims = new dim(this->ParameterList[j]->dims->toString());
                delete this->ParameterList[j];
                this->ParameterList[j] = new ParameterData(data->ParameterList[i]);
                // but may change dims!
                this->ParameterList[j]->dims->fromString(oldDims->toString());
            }
        }
    }

    // add new list - copying across as needed
    for (int i = 0; i < data->StateVariableList.size(); ++i) {
        for (int j = 0; j < this->StateVariableList.size(); ++j) {
            if (data->StateVariableList[i]->name == this->StateVariableList[j]->name) {
                dim * oldDims = new dim(this->StateVariableList[j]->dims->toString());
                delete this->StateVariableList[j];
                this->StateVariableList[j] = new StateVariableData(data->StateVariableList[i]);
                // but may change dims!
                this->StateVariableList[j]->dims->fromString(oldDims->toString());
            }
        }
    }

}
