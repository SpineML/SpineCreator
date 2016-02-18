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

#include "CL_layout_classes.h"

NineMLLayout::NineMLLayout(QSharedPointer<NineMLLayout>data)
{

    name = data->name;
    //type = this->type;
    RegimeList = QVector <RegimeSpace*>(data->RegimeList.size());
    StateVariableList = QVector <StateVariable*>(data->StateVariableList.size());
    ParameterList = QVector <Parameter*>(data->ParameterList.size());
    AliasList = QVector <Alias*>(data->AliasList.size());
    for (int i=0; i<data->RegimeList.size(); i++)
    {
        RegimeList[i] = new RegimeSpace(data->RegimeList[i]);
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
    //validate this
    validateComponent();
}

NineMLLayout::~NineMLLayout()
{
    for (int i=0; i<RegimeList.size(); i++)
    {
        delete RegimeList[i];
    }
    for (int i=0; i<StateVariableList.size(); i++)
    {
        delete StateVariableList[i];
    }
    for (int i=0; i<ParameterList.size(); i++)
    {
        delete ParameterList[i];
    }
    for (int i=0; i<AliasList.size(); i++)
    {
        delete AliasList[i];
    }
}

// assignment operator required for the base class
NineMLLayout& NineMLLayout::operator=(const NineMLLayout& data)
{
    name = data.name;
    //type = this->type;
    RegimeList = QVector <RegimeSpace*>(data.RegimeList.size());
    StateVariableList = QVector <StateVariable*>(data.StateVariableList.size());
    ParameterList = QVector <Parameter*>(data.ParameterList.size());
    AliasList = QVector <Alias*>(data.AliasList.size());
    for (int i=0; i<data.RegimeList.size(); i++)
    {
        RegimeList[i] = new RegimeSpace(data.RegimeList[i]);
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
    QStringList validated = validateComponent();
    if (validated.size() > 1) {
        QMessageBox msgBox;
        QString message;
        for (int i = 0; i < (int) validated.size(); ++i) {
            message += validated[i] + "\n";
        }
        msgBox.setText(message);
        msgBox.exec();
    }

    return *this;
}

// copy constructor required for the base class
NineMLLayoutData::NineMLLayoutData(QSharedPointer<NineMLLayout>data)
{
    seed = 123;
    minimumDistance = 0.0;
    type = NineMLLayoutType;
    StateVariableList.resize(data->StateVariableList.size());
    ParameterList.resize(data->ParameterList.size());

    for (int i=0; i<data->StateVariableList.size(); i++)
    {
        StateVariableList[i] = new StateVariableInstance(data->StateVariableList[i]);
        StateVariableList[i]->currType = FixedValue;
        StateVariableList[i]->value.resize(1);
        StateVariableList[i]->value.fill(0);
    }
    for (int i=0; i<data->ParameterList.size(); i++)
    {
        ParameterList[i] = new ParameterInstance(data->ParameterList[i]);
        ParameterList[i]->currType = FixedValue;
        ParameterList[i]->value.resize(1);
        ParameterList[i]->value.fill(0);
    }
    this->component = data;
}

// copy constructor required for the base class
NineMLLayoutData::NineMLLayoutData(QSharedPointer<NineMLLayoutData>data)
{

    seed = data->seed;
    minimumDistance = data->minimumDistance;
    type = NineMLLayoutType;
    StateVariableList.resize(data->StateVariableList.size());
    ParameterList.resize(data->ParameterList.size());

    // copy data...
    for (int i=0; i<data->StateVariableList.size(); i++)
    {
        StateVariableList[i] = new StateVariableInstance(data->StateVariableList[i]);
    }
    for (int i=0; i<data->ParameterList.size(); i++)
    {
        ParameterList[i] = new ParameterInstance(data->ParameterList[i]);
    }

    // copy component reference...
    this->component = data->component;
}


QString NineMLLayout::getXMLName() {
    return this->name + ".xml";
}

void NineMLLayout::load(QDomDocument *doc)
{
    QDomNode n = doc->documentElement().firstChild();
    while( !n.isNull() )
    {
        QDomElement e = n.toElement();
        if( e.tagName() == "LayoutClass" )
        {
            this->name = e.attribute("name","");
            // default to unsorted in no type found
            //this->type = e.attribute("type", "unsorted");
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
                if( e2.tagName() == "Spatial" )
                {
                    QDomNode n3 = e2.firstChild();
                    while( !n3.isNull() )
                    {
                        QDomElement e3 = n3.toElement();
                        if( e3.tagName() == "Regime" )
                        {
                            RegimeSpace *tempRegime  = new RegimeSpace;
                            tempRegime->readIn(e3);
                            this->RegimeList.push_back(tempRegime);
                        }
                        if( e3.tagName() == "StateVariable" )
                        {
                            StateVariable *tempSV  = new StateVariable;
                            tempSV->readIn(e3);
                            this->StateVariableList.push_back(tempSV);
                        }
                        if( e3.tagName() == "Alias" )
                        {
                            Alias *tempAlias  = new Alias;
                            tempAlias->readIn(e3);
                            this->AliasList.push_back(tempAlias);
                        }

                        n3 = n3.nextSibling();
                    }
                }
                n2 = n2.nextSibling();
            }

        }
        n = n.nextSibling();
    }

    // validate this
    QStringList validated = validateComponent();
    if (validated.size() > 1) {
        QMessageBox msgBox;
        QString message;
        for (int i = 0; i < (int) validated.size(); ++i) {
            message += validated[i] + "\n";
        }
        msgBox.setText(message);
        msgBox.exec();
    }
}

void NineMLLayout::write(QDomDocument *doc)
{
    // validate this
    QStringList validated = validateComponent();
    if (validated.size() > 1) {
        QMessageBox msgBox;
        QString message;
        for (int i = 0; i < (int) validated.size(); ++i) {
            message += validated[i] + "\n";
        }
        msgBox.setText(message);
        msgBox.exec();
    }

    // write out:

    // create the root of the file:
    QDomElement root = doc->createElement( "SpineML" );
    QDomProcessingInstruction xmlDeclaration = doc->createProcessingInstruction("xml", "version=\"1.0\"");
    doc->appendChild(xmlDeclaration);
    doc->appendChild( root );
    root.setAttribute("xmlns", "http://nineml.org/9ML/0.1");

    QDomElement CClass = doc->createElement( "LayoutClass" );
    CClass.setAttribute("name", this->name);
    //CClass.setAttribute("type", this->type);
    root.appendChild(CClass);

    // declarations
    for (int i = 0; i < this->ParameterList.size(); ++i) {
        ParameterList[i]->writeOut(doc, CClass);
    }

    QDomElement space = doc->createElement( "Spatial" );
    CClass.appendChild(space);

    // space
    for (int i = 0; i < this->RegimeList.size(); ++i) {
        RegimeList[i]->writeOut(doc, space);
    }
    for (int i = 0; i < this->AliasList.size(); ++i) {
        AliasList[i]->writeOut(doc, space);
    }
    for (int i = 0; i < this->StateVariableList.size(); ++i) {
        StateVariableList[i]->writeOut(doc, space);
    }

}

QStringList NineMLLayout::validateComponent()
{
    QStringList errs;

    int failures = 0;
    for(int i=0; i<RegimeList.size(); i++)
    {
        failures += RegimeList[i]->validateRegime(this, &errs);
    }
    for(int i=0; i<AliasList.size(); i++)
    {
        failures += AliasList[i]->validateAlias(this, &errs);
    }

    errs.push_back("Total errors: " + QString::number(float(failures)));

    return errs;
}

void NineMLLayoutData::import_parameters_from_xml(QDomNode &nIn)
{
    type = NineMLLayoutType;

    this->seed = nIn.toElement().attribute("seed","123").toInt();
    this->minimumDistance = nIn.toElement().attribute("minimum_distance","0.0").toDouble();

    QDomNodeList nList = nIn.toElement().elementsByTagName("Property");

    for (int node = 0; node < nList.count(); ++node) {
        QDomNode n = nList.item(node);

        // extract value and dimensions from node
        QString propName = n.toElement().attribute("name","");
        if (propName == "") {
            // error
            QSettings settings;
            int num_errs = settings.beginReadArray("errors");
            settings.endArray();
            settings.beginWriteArray("errors");
                settings.setArrayIndex(num_errs + 1);
                settings.setValue("errorText",  "XML error: attribute 'name' not found in tag 'Property'");
            settings.endArray();
        }

        bool parFound = false;

        for (int i = 0; i < this->ParameterList.size(); ++i) {
            if (propName == this->ParameterList[i]->name) {
                parFound = true;
                QDomNodeList propVal = n.toElement().elementsByTagName("FixedValue");
                if (propVal.size() == 1) {
                    this->ParameterList[i]->value[0] = propVal.at(0).toElement().attribute("value","").toDouble();
                }
                break;
            }
        }
        if (!parFound) {
            for (int i = 0; i < this->StateVariableList.size(); ++i) {
                if (propName == this->StateVariableList[i]->name) {
                    parFound = true;
                    QDomNodeList propVal = n.toElement().elementsByTagName("FixedValue");
                    if (propVal.size() == 1) {
                        this->StateVariableList[i]->value[0] = propVal.at(0).toElement().attribute("value","").toDouble();
                    }
                    break;
                }
            }
        }

        if (!parFound) {
            // error
            QSettings settings;
            int num_errs = settings.beginReadArray("errors");
            settings.endArray();
            settings.beginWriteArray("errors");
                settings.setArrayIndex(num_errs + 1);
                settings.setValue("errorText",  "Error: property '" + propName + "' not found in Layout");
            settings.endArray();
        }
    }
}

enum axisType {
    XAXIS,
    YAXIS,
    ZAXIS

};

void rotateAxes(axisType axis, float angle, float x[3], float y[3], float z[3]) {


    // to radians
    angle = angle / 180 * M_PI;

    // precompute:
    float I_cosV = (1-cos(angle));
    float cosV = cos(angle);
    float sinV = sin(angle);

    float u[3];

    // select rotation axis:
    switch (axis) {
        case XAXIS:
        u[0] = x[0]; u[1] = x[1]; u[2] = x[2];
            break;

        case YAXIS:
        u[0] = y[0]; u[1] = y[1]; u[2] = y[2];
            break;

        case ZAXIS:
        u[0] = z[0]; u[1] = z[1]; u[2] = z[2];
            break;
    }

    float in[3];
    in[0] = x[0]; in[1] = x[1]; in[2] = x[2];
    // rotate x:
    x[0] = in[0]*(cosV+pow(u[0],2)*I_cosV) + in[1]*(u[0]*u[1]*I_cosV-u[2]*sinV) + in[2]*(u[0]*u[2]*I_cosV+u[1]*sinV);
    x[1] = in[0]*(u[1]*u[0]*I_cosV+u[2]*sinV) + in[1]*(cosV+pow(u[1],2)*I_cosV) + in[2]*(u[1]*u[2]*I_cosV-u[0]*sinV);
    x[2] = in[0]*(u[2]*u[0]*I_cosV-u[1]*sinV) + in[1]*(u[2]*u[1]*I_cosV+u[0]*sinV) + in[2]*(cosV+pow(u[2],2)*I_cosV);

    in[0] = y[0]; in[1] = y[1]; in[2] = y[2];
    // rotate y:
    y[0] = in[0]*(cosV+pow(u[0],2)*I_cosV) + in[1]*(u[0]*u[1]*I_cosV-u[2]*sinV) + in[2]*(u[0]*u[2]*I_cosV+u[1]*sinV);
    y[1] = in[0]*(u[1]*u[0]*I_cosV+u[2]*sinV) + in[1]*(cosV+pow(u[1],2)*I_cosV) + in[2]*(u[1]*u[2]*I_cosV-u[0]*sinV);
    y[2] = in[0]*(u[2]*u[0]*I_cosV-u[1]*sinV) + in[1]*(u[2]*u[1]*I_cosV+u[0]*sinV) + in[2]*(cosV+pow(u[2],2)*I_cosV);

    in[0] = z[0]; in[1] = z[1]; in[2] = z[2];
    // rotate z:
    z[0] = in[0]*(cosV+pow(u[0],2)*I_cosV) + in[1]*(u[0]*u[1]*I_cosV-u[2]*sinV) + in[2]*(u[0]*u[2]*I_cosV+u[1]*sinV);
    z[1] = in[0]*(u[1]*u[0]*I_cosV+u[2]*sinV) + in[1]*(cosV+pow(u[1],2)*I_cosV) + in[2]*(u[1]*u[2]*I_cosV-u[0]*sinV);
    z[2] = in[0]*(u[2]*u[0]*I_cosV-u[1]*sinV) + in[1]*(u[2]*u[1]*I_cosV+u[0]*sinV) + in[2]*(cosV+pow(u[2],2)*I_cosV);

}


void NineMLLayoutData::generateLayout(int numNeurons, QVector <loc> *locations, QString &errRet) {

    float result = 0;

    locations->clear();

    // if no layout attached
    if (this->component->name == "none") {

        // linear layout by default:
        for (int i = 0; i < (int) numNeurons; ++i) {

            loc newLoc;

            //do a square:
            newLoc.x = i%10;
            newLoc.y = floor(float(i) / 10.0);
            newLoc.z = 0;
            locations->push_back(newLoc);

        }

        return;

    }

    /*float x[3] = {1,0,0};
    float y[3] = {0,1,0};
    float z[3] = {0,0,1};*/

    // create the variable list:
    vector < lookup > varList;

    for (int i = 0; i < this->StateVariableList.size(); ++i) {
        varList.push_back(lookup(StateVariableList[i]->name, StateVariableList[i]->value[0]));
    }
    for (int i = 0; i < this->component->AliasList.size(); ++i) {
        varList.push_back(lookup(this->component->AliasList[i]->name, 0.0));
    }

    for (int i = 0; i < this->ParameterList.size(); ++i) {
        if (ParameterList[i]->name == "numNeurons") {
            // override number of neurons value
            varList.push_back(lookup(ParameterList[i]->name, numNeurons));
        } else {
            varList.push_back(lookup(ParameterList[i]->name, ParameterList[i]->value[0]));
        }
    }

    // provided:
    varList.push_back(lookup("e", (float)M_E));
    varList.push_back(lookup("pi",(float) M_PI));


    if (this->component->RegimeList.size() > 0) {

        // start in the first regime we come across

        RegimeSpace * regime = this->component->RegimeList.front();
        QVector <int> order;

        for (int i = 0; i < regime->TransformList.size(); ++i) {

            // resize if order is smaller
            if ((int) order.size() < regime->TransformList[i]->order) {
                order.resize(regime->TransformList[i]->order);
            }
            order[regime->TransformList[i]->order-1] = i;

        }

        // create stacks:
        vector < vector < valop > > trstacks;
        for (int trans = 0; trans < order.size(); ++trans) {
            QString err;
            vector < valop > newStack;
            trstacks.push_back(newStack);
            err = createStack(regime->TransformList[order[trans]]->maths->equation, varList, &trstacks.back());

            // if error doing maths...
            if (err != "") {
                // oops - notify the user FIX THIS TO MAKE IT NICER
                /*QMessageBox msgBox;
                msgBox.setText(err);
                msgBox.exec();*/
                errRet = err;
                return;
           }
        }
        vector < vector < valop > > alstacks;
        for (int j = 0; j < this->component->AliasList.size(); ++j) {
            QString err;
            vector < valop > newStack;
            alstacks.push_back(newStack);
            err = createStack(this->component->AliasList[j]->maths->equation, varList, &alstacks.back());

            // if error doing maths...
            if (err != "") {
                // oops - notify the user FIX THIS TO MAKE IT NICER
                /*QMessageBox msgBox;
                msgBox.setText(err);
                msgBox.exec();*/
                errRet = err;
                return;
           }
        }

        srand(this->seed);

        int loop = 0;

        for (int i = 0; i < (int) numNeurons; ++i) {

            if (loop > 1000) {
                errRet = "Cannot satisfy distance constraint";
                locations->clear();
                return;
            }

            // back up the variables in case we infringe minimum distance
            vector < lookup > varListBack = varList;

            // do aliases:
            for (int j = 0; j < this->component->AliasList.size(); ++j) {

                float result;
                // find the correct alias
                //Alias * currAlias;

                //currAlias = this->component->AliasList[j];

                result = interpretMaths(alstacks[j]);

                // assign back to the Alias:
                varList[StateVariableList.size()+j].value = result;

            }

            // do translations
            for (int trans = 0; trans < order.size(); ++trans) {

                result = interpretMaths(trstacks[trans]);

                // assign result to the given statevariable
                if (regime->TransformList[order[trans]]->type == TRANSLATE) {
                    for (int j = 0; j < this->StateVariableList.size(); ++j) {
                        if (varList[j].name == regime->TransformList[order[trans]]->variable->name) {
                            varList[j].value = result;
                        }
                    }
                }
            }


            // write out the location
            loc newLoc = {0,0,0};
            for (int sv = 0; sv < this->StateVariableList.size(); ++sv) {
                if (varList[sv].name == "x" ) {
                    newLoc.x = varList[sv].value;
                }
                if (varList[sv].name == "y" ) {
                    newLoc.y = varList[sv].value;
                }
                if (varList[sv].name == "z" ) {
                    newLoc.z = varList[sv].value;
                }
            }

            // check if minimum distance is infringed:
            if (this->minimumDistance > 0) {

                bool tooClose = false;

                for (int l = 0; l < locations->size(); ++l) {
                    if (pow((*locations)[l].x - newLoc.x,2) + pow((*locations)[l].y - newLoc.y, 2) + pow((*locations)[l].z - newLoc.z,2) < pow(this->minimumDistance,2)) {
                        tooClose = true;
                        break;
                    }
                }
                if (!tooClose) {
                    locations->push_back(newLoc);
                    loop = 0;
                } else {
                    // do this iteration again!
                    --i;
                    varList = varListBack;
                    ++loop;
                }
            } else
                locations->push_back(newLoc);

        }
    }


}

void RegimeSpace::readIn(QDomElement e)
{

    this->name = e.attribute("name","");
    QDomNode n = e.firstChild();
    while( !n.isNull() )
    {
        QDomElement e2 = n.toElement();
        if( e2.tagName() == "Transform" )
        {
            Transform *tempTD = new Transform;
            tempTD->readIn(e2);
            this->TransformList.push_back(tempTD);
        }

        if( e2.tagName() == "OnCondition" )
        {
            OnConditionSpace *tempOC = new OnConditionSpace;
            tempOC->readIn(e2);
            this->OnConditionList.push_back(tempOC);
       }
        n = n.nextSibling();
    }

}

void RegimeSpace::writeOut(QDomDocument *doc, QDomElement &parent)
{

    QDomElement regime = doc->createElement( "Regime" );
    regime.setAttribute("name", this->name);
    parent.appendChild(regime);

    // write children
    for (int i = 0; i < this->TransformList.size(); ++i) {
        this->TransformList[i]->writeOut(doc, regime);
    }
    for (int i = 0; i < this->OnConditionList.size(); ++i) {
        this->OnConditionList[i]->writeOut(doc, regime);
    }

}

RegimeSpace::RegimeSpace(RegimeSpace *data)
{
    name = data->name;
    OnConditionList = QVector <OnConditionSpace*>(data->OnConditionList.size());
    TransformList = QVector <Transform*>(data->TransformList.size());
    for (int i=0; i<data->OnConditionList.size(); i++)
    {
        OnConditionList[i] = new OnConditionSpace(data->OnConditionList[i]);
    }
    for (int i=0; i<data->TransformList.size(); i++)
    {
        TransformList[i] = new Transform(data->TransformList[i]);
    }

}

RegimeSpace::~RegimeSpace()
{

    for (int i=0; i<OnConditionList.size(); i++)
    {
        delete OnConditionList[i];
    }
    for (int i=0; i<TransformList.size(); i++)
    {
        delete TransformList[i];
    }
}

int RegimeSpace::validateRegime(NineMLLayout * component, QStringList * errs)
{
    int failures = 0;
    for(int i=0; i<OnConditionList.size(); i++)
    {
        failures += OnConditionList[i]->validateOnCondition(component, errs);
    }
    for(int i=0; i<TransformList.size(); i++)
    {
        failures += TransformList[i]->validateTransform(component, errs);
    }
    // validate transform order:
    vector <int> order;
    for (int i = 0; i <TransformList.size(); ++i) {
        // resize if order is smaller
        if ((int) order.size() < this->TransformList[i]->order+1) {
            order.resize(this->TransformList[i]->order+1, -1);
        }
        if (order[this->TransformList[i]->order] != -1) {++failures; errs->push_back("Duplicate Transform order value in Regime");}
        order[this->TransformList[i]->order] = i;
    }
    for (uint i = 0; i < order.size(); ++i) {
        if (i == 0 && order[i] != -1) {++failures; errs->push_back("Transform ordering must start from 1, not 0 in Regime");}
        if (order[i] == -1 && i != 0) {++failures; errs->push_back("Transform order is not contiguous in Regime");}
    }

    return failures;
}

void OnConditionSpace::readIn(QDomElement e) {
    this->target_regime_name = e.attribute("target_regime","");
    QDomNode n = e.firstChild();
    // number of each type
    int Triggers = 0;
    int StateAssignments = 0;
    int Transforms = 0;
    while( !n.isNull() )
    {
        QDomElement e2 = n.toElement();
        if( e2.tagName() == "StateAssignment" )
        {
            StateAssignment *tempSA = new StateAssignment;
            tempSA->readIn(e2);
            this->StateAssignList.push_back(tempSA);
            ++StateAssignments;
        }
        if( e2.tagName() == "Transform" )
        {
            Transform *tempTD = new Transform;
            tempTD->readIn(e2);
            this->TransformList.push_back(tempTD);
            ++Transforms;
        }
        if( e2.tagName() == "Trigger" )
        {
            this->trigger = new Trigger;
            this->trigger->readIn(e2);
            ++Triggers;
        }
        if (Triggers < 1) {
            this->trigger = new Trigger();
            this->trigger->maths = new MathInLine();
        }
        n = n.nextSibling();
    }
}

void OnConditionSpace::writeOut(QDomDocument * doc, QDomElement &parent)
{
    QDomElement OnCondition = doc->createElement( "OnCondition" );
    parent.appendChild(OnCondition);
    OnCondition.setAttribute("target_regime", this->target_regime_name);

    for (int i = 0; i < this->StateAssignList.size(); ++i) {
        this->StateAssignList[i]->writeOut(doc, OnCondition);
    }
    for (int i = 0; i < this->TransformList.size(); ++i) {
        this->TransformList[i]->writeOut(doc, OnCondition);
    }

    this->trigger->writeOut(doc, OnCondition);

}

OnConditionSpace::OnConditionSpace(OnConditionSpace *data)
{
    target_regime = NULL;
    target_regime_name = data->target_regime_name;
    trigger = new Trigger(data->trigger);
    StateAssignList = QVector <StateAssignment*>(data->StateAssignList.size());
    TransformList = QVector <Transform*>(data->TransformList.size());
    for (int i=0; i<data->StateAssignList.size(); i++)
    {
        StateAssignList[i] = new StateAssignment(data->StateAssignList[i]);
    }
    for (int i=0; i<data->TransformList.size(); i++)
    {
        TransformList[i] = new Transform(data->TransformList[i]);
    }

}

OnConditionSpace::~OnConditionSpace()
{
    delete trigger;
    for (int i=0; i<StateAssignList.size(); i++)
    {
        delete StateAssignList[i];
    }
    for (int i=0; i<TransformList.size(); i++)
    {
        delete TransformList[i];
    }

}

int OnConditionSpace::validateOnCondition(NineMLLayout * component, QStringList * errs)
{
    int failures = 0;
    bool match = false;
    for(int i=0;i<component->RegimeList.size();i++)
    {
        if (component->RegimeList[i]->name.compare(target_regime_name) == 0)
        {target_regime = component->RegimeList[i];
            match = true;}
    }
    if (!match) errs->push_back("OnCondition references missing Synapse Regime");
    failures += !match;
    for(int i=0; i<StateAssignList.size(); i++)
    {
        failures += StateAssignList[i]->validateStateAssignment(component, errs);
    }
    for(int i=0; i<TransformList.size(); i++)
    {
        failures += TransformList[i]->validateTransform(component, errs);
    }

    // validate transform order:
    vector <int> order;
    for (int i = 0; i <TransformList.size(); ++i) {
        // resize if order is smaller
        if ((int) order.size() < this->TransformList[i]->order+1) {
            order.resize(this->TransformList[i]->order+1, -1);
        }
        if (order[this->TransformList[i]->order] != -1) {++failures; errs->push_back("Duplicate Transform order value in OnCondition");}
        order[this->TransformList[i]->order] = i;
    }
    for (uint i = 0; i < order.size(); ++i) {
        if (i == 0 && order[i] != -1) {++failures; errs->push_back("Transform ordering must start from 1, not 0 in OnCondition");}
        if (order[i] == -1 && i != 0) {++failures; errs->push_back("Transform order is not contiguous in OnCondition");}
    }

    return failures;
}

void Transform::readIn(QDomElement e)
{
    this->order = e.attribute("order","").toInt();
    QString tempType = e.attribute("type","");
    if (tempType == "identity") type = IDENTITY;
    if (tempType == "translate") type = TRANSLATE;
    if (tempType == "rotate") type = ROTATE;
    if (tempType == "scale") type = SCALE;
    this->variableName = e.attribute("variable","");
    // we need dims in here too eventually

    QDomNode n = e.firstChild();
    QDomElement e2 = n.toElement();
    if( e2.tagName() == "MathInline" )
    {
        this->maths = new MathInLine;
        this->maths->readIn(e2);
    } else {
        // failure proofing against identities with no mathinline
        this->maths = new MathInLine;
        this->maths->equation = "0";
    }

}

void Transform::writeOut(QDomDocument * doc, QDomElement &parent)
{
    QDomElement transform = doc->createElement( "Transform" );
    transform.setAttribute("variable", this->variableName);
    transform.setAttribute("order", QString::number(this->order));
    transform.setAttribute("dimension", "??");
    switch (this->type) {
    case IDENTITY:
        transform.setAttribute("type", "identity");
        break;
    case TRANSLATE:
        transform.setAttribute("type", "translate");
        break;
    case ROTATE:
        transform.setAttribute("type", "rotate");
        break;
    case SCALE:
        transform.setAttribute("type", "scale");
        break;
    }

    this->maths->writeOut(doc, transform);

    parent.appendChild(transform);
    // we need dims in here too eventually

}

int Transform::validateTransform(NineMLLayout * component, QStringList * errs)
{
    int failures = 0;
    bool match = false;
    for(int i=0; i<component->StateVariableList.size(); i++)
    {
        if (component->StateVariableList[i]->name.compare(this->variableName) == 0)
        {variable = component->StateVariableList[i];
            match = true;}

    }
    if (!match) errs->push_back("Transform references missing StateVariable");
    failures += !match;
    if (maths != (MathInLine *)0) {
        failures += maths->validateMathInLine(component, errs);
    } else {
        failures += 1;
    }

    //if (failures > 0) std::cerr << "Error validating StateAssignment for " << component->name.toStdString() << "\n";
    return failures;
}

Transform::Transform(Transform *data)
{
    variableName = data->variableName;
    type = data->type;
    dims = data->dims;
    order = data->order;
    maths = new MathInLine(data->maths);
}

Transform::~Transform()
{
    delete maths;
}

TransformData::TransformData(Transform *data)
{
    dims = data->dims;
    value = 0;
}

int StateAssignment::validateStateAssignment(NineMLLayout * component, QStringList * errs)
{
    int failures = 0;
    bool match = false;
    for(int i=0; i<component->StateVariableList.size(); i++)
    {
        if (component->StateVariableList[i]->name.compare(name) == 0)
        {variable = component->StateVariableList[i];
            match = true;}

    }
    if (!match) errs->push_back("StateAssignment references missing StateVariable");
    failures += !match;
    //if (failures > 0) std::cerr << "Error validating StateAssignment for " << component->name.toStdString() << "\n";
    return failures;
}

int Alias::validateAlias(NineMLLayout * component, QStringList * errs)
{
    // mathinline pointer may be null
    if (maths != (MathInLine *)0) {
        return maths->validateMathInLine(component, errs);
    } else {
        return 1;
    }
}

int MathInLine::validateMathInLine(NineMLLayout * component, QStringList * errs)
{
    // remove operators

    if (equation.size() == 0) {
        return 1;
    }

    QString test = equation;

    test = equation;
    test.replace('+', ' ');
    test = test.replace('-', ' ');
    test = test.replace('*', ' ');
    test = test.replace('/', ' ');
    test = test.replace('(', ' ');
    test = test.replace(')', ' ');
    test = test.replace('<', ' ');
    test = test.replace('>', ' ');
    test = test.replace('=', ' ');
    test = test.replace(',', ' ');
    test = test.replace('!', ' ');

    QStringList FuncList;
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
    FuncList.push_back("rand");
    FuncList.push_back("mod");


    // not strictly functions...
    FuncList.push_back("t");
    FuncList.push_back("dt");

    // remove double whitespaces
    test = test.simplified();

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
            errs->push_back("MathString contains unrecognised token " + splitTest[i]);

        }

    }
    if (equation.count("(") != equation.count(")")) {
        errs->push_back("MathString contains mis-matched brackets");
    }

    return 0;
}
