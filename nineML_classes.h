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
**           Author: Alex Cope, Paul Richmond                             **
**  Website/Contact: http://bimpa.group.shef.ac.uk/                       **
****************************************************************************/

#ifndef NINEML_STRUCTS_H
#define NINEML_STRUCTS_H

#include "globalHeader.h"
#include "systemobject.h"

using namespace std;

typedef enum{
    AnalogSendPort,
    AnalogRecvPort,
    AnalogReducePort
} AnalogPortMode;

typedef enum{
    EventSendPort,
    EventRecvPort
} EventPortMode;

typedef enum{
    ImpulseSendPort,
    ImpulseRecvPort
} ImpulsePortMode;

typedef enum{
    ReduceOperationAddition,
    ReduceOperationNone
} ReduceOperation;

typedef enum{
    NineMLComponentType,
    NineMLLayoutType
} NineMLType;

typedef enum{
    ComponentTypeNeuronBody,
    ComponentTypeSynapse,
    ComponentTypePostsynapse,
    ComponentTypeGeneral
} ComponentType;


typedef enum{
    PREFIX_NONE,
    PREFIX_G,
    PREFIX_M,
    PREFIX_k,
    PREFIX_c,
    PREFIX_m,
    PREFIX_u,
    PREFIX_n,
    PREFIX_p,
    PREFIX_f
} Prefix;

typedef enum{
    UNIT_NONE,
    UNIT_V,
    UNIT_Ohm,
    UNIT_g,
    UNIT_m,
    UNIT_s,
    UNIT_A,
    UNIT_cd,
    UNIT_mol,
    UNIT_degC,
    UNIT_S,
    UNIT_F,
    UNIT_Hz
} Unit;

class dim {
public:
    dim(QString str);
    ~dim(){}
    int m;
    int l;
    int t;
    int I;
    int Cd;
    int mol;
    int temp;
    int scale;
    QString toString();
    void fromString(QString);

    void setPrefix(QString p);
    void setUnit(QString u);

    QString getPrefixString();
    QString getUnitString();

    Prefix getPrefix();
    Unit getUnit();

    void reset();

    friend bool operator==(dim &dim1, dim &dim2);
};

/**************************** Linked items *****************************/
class Regime;
class NineMLComponent;
class StateVariable;

typedef enum
{
    NINEML_PARAMETER,
    //NINEML_PORT, abstract
    NINEML_ANALOG_PORT,
    NINEML_EVENT_PORT,
    NINEML_IMPULSE_PORT,
    NINEML_MATHINLINE,
    NINEML_TRIGGER,
    NINEML_STATE_VARIABLE,
    NINEML_ALIAS,
    NINEML_TIME_DERIVATIVE,
    NINEML_STATE_ASSIGNMENT,
    NINEML_EVENT_OUT,
    NINEML_IMPULSE_OUT,
    NINEML_ON_CONDITION,
    NINEML_ON_EVENT,
    NINEML_ON_IMPULSE,
    NINEML_REGIME,
    NINEML_COMPONENT
}NineMLObjectType;

class NineMLObject
{
public:
    NineMLObject(){}
    ~NineMLObject(){}
    virtual NineMLObjectType Type() = 0;
};

class NineMLRootObject
{
public:
    NineMLRootObject(){path="temp"; filePath="";}
    ~NineMLRootObject(){}
    QString path;
    QString filePath;
};

class Parameter: public QObject, public NineMLObject {
    Q_OBJECT
public:
    QString getName();
    void setName(QString name);
    QString name;
    dim * dims;
    Parameter(Parameter *data);
    Parameter(){dims = new dim("?");}
    virtual ~Parameter(){delete dims;}
    void readIn(QDomElement e);
    void writeOut(QDomDocument *doc, QDomElement &parent);
    virtual NineMLObjectType Type(){return NINEML_PARAMETER;}
signals:
    void nameChanged();
};

class ParameterData {
public:
    QString name;
    dim * dims;
    vector < float > value;
    vector < int > indices;
    ParameterType currType;
    int seed;

    ParameterData(Parameter *data);
    ParameterData(ParameterData *data);
    ParameterData(QString dimString){dims = new dim(dimString);}
    ~ParameterData(){delete dims;}
    void readIn(QDomElement e);
    void writeOut(QDomDocument *doc, QDomElement &parent);
};

class Port: public QObject, public NineMLObject {
    Q_OBJECT
public:
    QString name;
    dim * dims;
    Port(Port *data);
    QString getName();
    void setName(QString name);
    Port(){dims = new dim("?");}
    virtual ~Port(){delete dims;}
    void readIn(QDomElement e);
    void writeOut(QDomDocument *doc, QDomElement &parent);
    virtual bool isAnalog();
signals:
    void nameChanged();
};

class AnalogPort: public Port {
public:
    AnalogPortMode mode;
    ReduceOperation op;
    StateVariable *variable;
    AnalogPort(AnalogPort *data);
    AnalogPort() : Port() {variable=NULL; op = ReduceOperationAddition;}
    virtual ~AnalogPort(){}
    void readIn(QDomElement e);
    void writeOut(QDomDocument *doc, QDomElement &parent);
    bool isAnalog();
    virtual NineMLObjectType Type(){return NINEML_ANALOG_PORT;}
    int validateAnalogPort(NineMLComponent* component, QStringList *);
};

class EventPort: public Port {
public:
    EventPortMode mode;
    EventPort(EventPort *data);
    EventPort() : Port() {mode=EventSendPort;}
    virtual ~EventPort(){}
    void readIn(QDomElement e);
    void writeOut(QDomDocument *doc, QDomElement &parent);
    bool isAnalog();
    virtual NineMLObjectType Type(){return NINEML_EVENT_PORT;}
};

class ImpulsePort: public Port {
public:
    ImpulsePortMode mode;
    Parameter *parameter;
    ImpulsePort(ImpulsePort *data);
    ImpulsePort() : Port() {parameter=NULL; mode=ImpulseSendPort;}
    virtual ~ImpulsePort(){}
    void readIn(QDomElement e);
    void writeOut(QDomDocument *doc, QDomElement &parent);
    bool isAnalog();
    virtual NineMLObjectType Type(){return NINEML_IMPULSE_PORT;}
    int validateImpulsePort(NineMLComponent* component, QStringList *);
};

class MathInLine: public NineMLObject {
public:

    QString equation;
    MathInLine(MathInLine *data);
    MathInLine(){}
    virtual ~MathInLine(){}
    QString getHTMLSafeEquation();
    int validateMathInLine(NineMLComponent* component, QStringList * errs);
    int validateMathInLine(NineMLLayout* component, QStringList * errs);
    void readIn(QDomElement e);
    void writeOut(QDomDocument *doc, QDomElement &parent);
    virtual NineMLObjectType Type(){return NINEML_MATHINLINE;}
};

class Trigger: public NineMLObject {
public:
    MathInLine *maths;
    Trigger(Trigger *data);
    Trigger(){maths=new MathInLine;}
    virtual ~Trigger();
    int validateTrigger(NineMLComponent* component, QStringList * errs);
    void readIn(QDomElement e);
    void writeOut(QDomDocument *doc, QDomElement &parent);
    virtual NineMLObjectType Type(){return NINEML_TRIGGER;}
};

class StateVariable: public Parameter {
public:
    StateVariable(StateVariable *data);
    StateVariable() : Parameter() {}
    virtual ~StateVariable();
    void readIn(QDomElement e);
    void writeOut(QDomDocument *doc, QDomElement &parent);
    virtual NineMLObjectType Type(){return NINEML_STATE_VARIABLE;}
};

class StateVariableData: public ParameterData {
public:
    StateVariableData(StateVariable *data);
    StateVariableData(StateVariableData *data);
    //StateVariableData(){}
    virtual ~StateVariableData(){}
    void readIn(QDomElement e);
    void writeOut(QDomDocument *doc, QDomElement &parent);
};

class Alias: public StateVariable {
public:
    //QString name;
    //dim * dims;
    MathInLine* maths;
    Alias(Alias *data);
    Alias(): StateVariable() {maths=new MathInLine();}
    virtual ~Alias();
    int validateAlias(NineMLComponent* component, QStringList * errs);
    int validateAlias(NineMLLayout* component, QStringList * errs);
    void readIn(QDomElement e);
    void writeOut(QDomDocument *doc, QDomElement &parent);
    virtual NineMLObjectType Type(){return NINEML_ALIAS;}

};

class TimeDerivative: public NineMLObject {
public:
    // temp name
    QString variable_name;

    StateVariable *variable;
    MathInLine* maths;
    TimeDerivative(TimeDerivative *data);
    TimeDerivative(){variable=NULL; maths=new MathInLine;}
    virtual ~TimeDerivative();
    int validateTimeDerivative(NineMLComponent* component, QStringList * errs);
    void readIn(QDomElement e);
    void writeOut(QDomDocument *doc, QDomElement &parent);
    virtual NineMLObjectType Type(){return NINEML_TIME_DERIVATIVE;}
};

class StateAssignment: public NineMLObject {
public:
    // temp name
    QString name;

    StateVariable *variable;
    MathInLine *maths;
    StateAssignment(StateAssignment *data);
    StateAssignment(){variable=NULL; maths=new MathInLine;}
    virtual ~StateAssignment();
    int validateStateAssignment(NineMLComponent* component, QStringList *);
    int validateStateAssignment(NineMLLayout* component, QStringList * errs);
    void readIn(QDomElement e);
    void writeOut(QDomDocument *doc, QDomElement &parent);
    virtual NineMLObjectType Type(){return NINEML_STATE_ASSIGNMENT;}
};

class EventOut: public NineMLObject {
public:
    // temp name
    QString port_name;
    EventPort *port;
    EventOut(EventOut *);
    EventOut(){port=NULL;}
    virtual ~EventOut(){}
    int validateEventOut(NineMLComponent* component, QStringList *);
    void readIn(QDomElement e);
    void writeOut(QDomDocument *doc, QDomElement &parent);
    virtual NineMLObjectType Type(){return NINEML_EVENT_OUT;}
};

class ImpulseOut: public NineMLObject {
public:
    // temp name
    QString port_name;
    ImpulsePort *port;
    ImpulseOut(ImpulseOut *);
    ImpulseOut(){port=NULL;}
    virtual ~ImpulseOut(){}
    int validateImpulseOut(NineMLComponent* component, QStringList * errs);
    void readIn(QDomElement e);
    void writeOut(QDomDocument *doc, QDomElement &parent);
    virtual NineMLObjectType Type(){return NINEML_IMPULSE_OUT;}
};


class OnCondition: public NineMLObject {
public:
    // temp name
    QString target_regime_name;

    Regime *target_regime;
    vector <StateAssignment*> StateAssignList;
    Trigger *trigger;
    vector <EventOut*> eventOutList;
    vector <ImpulseOut*> impulseOutList;
    OnCondition(OnCondition *data);
    OnCondition(){target_regime=NULL; trigger=new Trigger;}
    virtual ~OnCondition();
    int validateOnCondition(NineMLComponent* component, QStringList * errs);
    void readIn(QDomElement e);
    void writeOut(QDomDocument *doc, QDomElement &parent);
    virtual NineMLObjectType Type(){return NINEML_ON_CONDITION;}
};


class OnEvent: public NineMLObject {
public:
    // temp name
    QString target_regime_name;
    QString src_port_name;

    Regime *target_regime;
    EventPort *src_port;
    vector <StateAssignment*> StateAssignList;
    vector <EventOut*> eventOutList;
    vector <ImpulseOut*> impulseOutList;
    OnEvent(OnEvent *data);
    OnEvent(){target_regime=NULL; src_port=NULL;}
    virtual ~OnEvent();
    int validateOnEvent(NineMLComponent* component, QStringList * errs);
    void readIn(QDomElement e);
    void writeOut(QDomDocument *doc, QDomElement &parent);
    virtual NineMLObjectType Type(){return NINEML_ON_EVENT;}
};

class OnImpulse: public NineMLObject {
public:
    // temp name
    QString target_regime_name;
    QString src_port_name;

    Regime *target_regime;
    ImpulsePort *src_port;
    vector <StateAssignment*> StateAssignList;
    vector <EventOut*> eventOutList;
    vector <ImpulseOut*> impulseOutList;
    OnImpulse(OnImpulse *data);
    OnImpulse(){target_regime=NULL; src_port=NULL;}
    virtual ~OnImpulse();
    int validateOnImpulse(NineMLComponent* component, QStringList * errs);
    void readIn(QDomElement e);
    void writeOut(QDomDocument *doc, QDomElement &parent);
    virtual NineMLObjectType Type(){return NINEML_ON_IMPULSE;}
};

class Regime: public NineMLObject {
public:
    QString name;
    vector <TimeDerivative*> TimeDerivativeList;
    vector <OnCondition* > OnConditionList;
    vector <OnEvent*> OnEventList;
    vector <OnImpulse*> OnImpulseList;
    Regime(Regime *data);
    Regime(){}
    virtual ~Regime();
    int validateRegime(NineMLComponent* component, QStringList * errs);
    void readIn(QDomElement e);
    void writeOut(QDomDocument *doc, QDomElement &parent);
    virtual NineMLObjectType Type(){return NINEML_REGIME;}
};

class NineMLComponent: public NineMLObject, public NineMLRootObject
{
public:

    //temp name
    QString initial_regime_name;

    QString name;
    QString type;
    Regime *initial_regime;
    vector <Regime*> RegimeList;
    vector <StateVariable*> StateVariableList;
    vector <Parameter*> ParameterList;
    vector <Alias*> AliasList;
    vector <AnalogPort*> AnalogPortList;
    vector <EventPort*> EventPortList;
    vector <ImpulsePort*> ImpulsePortList;
    NineMLComponent(NineMLComponent *data);
    NineMLComponent& operator=(const NineMLComponent& data);
    NineMLComponent();
    virtual ~NineMLComponent();
    void updateFrom(NineMLComponent *data);
    QStringList validateComponent();
    void load(QDomDocument *doc);
    void write(QDomDocument *doc);
    virtual NineMLObjectType Type(){return NINEML_COMPONENT;}
    QUndoStack undoStack;
    NineMLComponent * editedVersion;
    QString getXMLName();
};



class NineMLData
{
public:
    NineMLType type;
    vector <StateVariableData*> StateVariableList;
    vector <ParameterData*> ParameterList;
    void write_node_xml(QXmlStreamWriter &);
    NineMLData(){}
    ~NineMLData(){}
};

class NineMLComponentData: public NineMLData
{
public:
    vector < genericInput *> inputs;
    vector < genericInput * > outputs;
    NineMLComponent * component;
    NineMLComponentData(NineMLComponent *data);
    NineMLComponentData(NineMLComponentData *data);
    NineMLComponentData(NineMLComponentData *, NineMLComponent *);
    NineMLComponentData& operator=(const NineMLComponentData& data);
    NineMLComponentData(){}
    virtual ~NineMLComponentData();
    QString getXMLName();
    void matchPorts();
    QStringList getPortMatches(int index, bool isOutput);
    void removeReferences();
    systemObject * owner;
    void import_parameters_from_xml(QDomNode &e);
    void migrateComponent(NineMLComponent * newComponent);
    void addInput(NineMLComponentData *, bool = false);
    void copyParsFrom(NineMLComponentData * data);
};


#endif // NINEML_STRUCTS_H
