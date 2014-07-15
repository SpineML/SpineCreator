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

#ifndef NINEML_LAYOUT_CLASSES_H
#define NINEML_LAYOUT_CLASSES_H

// include existing classes
#include "nineML_classes.h"
#include "cinterpreter.h"

enum transformType {
    IDENTITY,
    TRANSLATE,
    ROTATE,
    SCALE
};

class RegimeSpace;

class NineMLLayout: public NineMLRootObject
{
public:

    QString name;
    QString type;
    vector <RegimeSpace*> RegimeList;
    vector <StateVariable*> StateVariableList;
    vector <Parameter*> ParameterList;
    vector <Alias*> AliasList;
    NineMLLayout(QSharedPointer<NineMLLayout>data);
    NineMLLayout& operator=(const NineMLLayout& data);
    NineMLLayout(){}
    ~NineMLLayout();
    QStringList validateComponent();
    void load(QDomDocument *doc);
    void write(QDomDocument *doc);
    QString getXMLName();

};

class Transform
{
public:
    int order;
    transformType type;
    QString variableName;
    StateVariable * variable;
    MathInLine * maths;
    dim * dims;
    Transform(Transform *data);
    Transform(){}
    ~Transform();
    void readIn(QDomElement e);
    void writeOut(QDomDocument *doc, QDomElement &parent);
    int validateTransform(NineMLLayout *component, QStringList * errs);
};

class TransformData
{
public:
    float value;
    dim * dims;
    TransformData(Transform *data);
    TransformData(){}
    ~TransformData(){}
};

class OnConditionSpace {
public:
    // temp name
    QString target_regime_name;
    RegimeSpace *target_regime;
    vector <StateAssignment*> StateAssignList;
    vector <Transform*> TransformList;
    Trigger *trigger;
    OnConditionSpace(OnConditionSpace *data);
    OnConditionSpace(){}
    ~OnConditionSpace();
    int validateOnCondition(NineMLLayout *component, QStringList * errs);
    void readIn(QDomElement e);
    void writeOut(QDomDocument *doc, QDomElement &parent);
};

class RegimeSpace {
public:
    QString name;
    vector <Transform*> TransformList;
    vector <OnConditionSpace* > OnConditionList;
    RegimeSpace(RegimeSpace *data);
    RegimeSpace(){}
    ~RegimeSpace();
    int validateRegime(NineMLLayout *component, QStringList * errs);
    void readIn(QDomElement e);
    void writeOut(QDomDocument *doc, QDomElement &parent);
};

class NineMLLayoutData : public NineMLData
{
public:
    int seed;
    double minimumDistance;
    QSharedPointer<NineMLLayout> component;
    NineMLLayoutData(QSharedPointer<NineMLLayout>data);
    NineMLLayoutData& operator=(const NineMLLayoutData& data);
    NineMLLayoutData(){}
    ~NineMLLayoutData(){}
    void import_parameters_from_xml(QDomNode &e);
    void generateLayout(int numNeurons, vector<loc> *locations, QString &errRet);
    vector < loc > locations;
};


#endif // NINEML_LAYOUT_CLASSES_H
