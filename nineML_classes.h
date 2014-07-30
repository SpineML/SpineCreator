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
    QVector < float > value;
    QVector < int > indices;
    ParameterType currType;
    int seed;

    ParameterData(Parameter *data);
    ParameterData(ParameterData *data);
    ParameterData(QString dimString){dims = new dim(dimString);}
    ~ParameterData(){delete dims;}
    void readIn(QDomElement e);
    void writeOut(QDomDocument *doc, QDomElement &parent);
    /*!
     * \brief writeExplicitListNodeData
     * \param xmlOut
     *
     * Function called to write the data for the explicit list Parameters
     * and State Variables, either as XML or as binary data
     */
    void writeExplicitListNodeData(QXmlStreamWriter &xmlOut);
    /*!
     * \brief readExplicitListNodeData
     * \param n
     *
     * Function called to read the data for explicit list Parameters
     * and State Variables, either as XML lists or binary data
     */
    void readExplicitListNodeData(QDomNode &n);
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
    int validateAnalogPort(NineMLComponent *component, QStringList *);
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
    int validateImpulsePort(NineMLComponent *component, QStringList *);
};

class MathInLine: public NineMLObject {
public:

    QString equation;
    MathInLine(MathInLine *data);
    MathInLine(){}
    virtual ~MathInLine(){}
    QString getHTMLSafeEquation();
    int validateMathInLine(NineMLComponent *component, QStringList * errs);
    int validateMathInLine(NineMLLayout * component, QStringList * errs);
    void readIn(QDomElement e);
    void writeOut(QDomDocument *doc, QDomElement &parent);
    virtual NineMLObjectType Type(){return NINEML_MATHINLINE;}

private:
    /*!
     * Remove an approved list of operators from the testequation,
     * then set up a function list of allowed functions. If what
     * remains in testequation is only FuncList, then the testequation
     * will validate.
     */
    void validateMathSetup(QString& testequation, QStringList& FuncList);
};

class Trigger: public NineMLObject {
public:
    MathInLine *maths;
    Trigger(Trigger *data);
    Trigger(){maths=new MathInLine;}
    virtual ~Trigger();
    int validateTrigger(NineMLComponent *component, QStringList * errs);
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
    int validateAlias(NineMLComponent *component, QStringList * errs);
    int validateAlias(NineMLLayout *component, QStringList * errs);
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
    int validateTimeDerivative(NineMLComponent *component, QStringList * errs);
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
    int validateStateAssignment(NineMLComponent *component, QStringList *);
    int validateStateAssignment(NineMLLayout *component, QStringList * errs);
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
    int validateEventOut(NineMLComponent *component, QStringList *);
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
    int validateImpulseOut(NineMLComponent *component, QStringList * errs);
    void readIn(QDomElement e);
    void writeOut(QDomDocument *doc, QDomElement &parent);
    virtual NineMLObjectType Type(){return NINEML_IMPULSE_OUT;}
};


class OnCondition: public NineMLObject {
public:
    // temp name
    QString target_regime_name;

    Regime *target_regime;
    QVector <StateAssignment*> StateAssignList;
    Trigger *trigger;
    QVector <EventOut*> eventOutList;
    QVector <ImpulseOut*> impulseOutList;
    OnCondition(OnCondition *data);
    OnCondition(){target_regime=NULL; trigger=new Trigger;}
    virtual ~OnCondition();
    int validateOnCondition(NineMLComponent *component, QStringList * errs);
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
    QVector <StateAssignment*> StateAssignList;
    QVector <EventOut*> eventOutList;
    QVector <ImpulseOut*> impulseOutList;
    OnEvent(OnEvent *data);
    OnEvent(){target_regime=NULL; src_port=NULL;}
    virtual ~OnEvent();
    int validateOnEvent(NineMLComponent *component, QStringList * errs);
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
    QVector <StateAssignment*> StateAssignList;
    QVector <EventOut*> eventOutList;
    QVector <ImpulseOut*> impulseOutList;
    OnImpulse(OnImpulse *data);
    OnImpulse(){target_regime=NULL; src_port=NULL;}
    virtual ~OnImpulse();
    int validateOnImpulse(NineMLComponent *component, QStringList * errs);
    void readIn(QDomElement e);
    void writeOut(QDomDocument *doc, QDomElement &parent);
    virtual NineMLObjectType Type(){return NINEML_ON_IMPULSE;}
};

class Regime: public NineMLObject {
public:
    QString name;
    QVector <TimeDerivative*> TimeDerivativeList;
    QVector <OnCondition* > OnConditionList;
    QVector <OnEvent*> OnEventList;
    QVector <OnImpulse*> OnImpulseList;
    Regime(Regime *data);
    Regime(){}
    virtual ~Regime();
    int validateRegime(NineMLComponent *component, QStringList * errs);
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
    QVector <Regime*> RegimeList;
    QVector <StateVariable*> StateVariableList;
    QVector <Parameter*> ParameterList;
    QVector <Alias*> AliasList;
    QVector <AnalogPort*> AnalogPortList;
    QVector <EventPort*> EventPortList;
    QVector <ImpulsePort*> ImpulsePortList;
    NineMLComponent(QSharedPointer<NineMLComponent>data);
    NineMLComponent& operator=(const NineMLComponent& data);
    NineMLComponent();
    virtual ~NineMLComponent();
    void updateFrom(QSharedPointer<NineMLComponent>data);
    QStringList validateComponent();
    void load(QDomDocument *doc);
    void write(QDomDocument *doc);
    virtual NineMLObjectType Type(){return NINEML_COMPONENT;}
    QUndoStack undoStack;
    QSharedPointer<NineMLComponent> editedVersion;
    QString getXMLName();
};



class NineMLData
{
public:
    NineMLType type;
    QVector <StateVariableData*> StateVariableList;
    QVector <ParameterData*> ParameterList;
    void write_node_xml(QXmlStreamWriter &);
    NineMLData(){}
    virtual ~NineMLData(){}
};

class NineMLComponentData: public NineMLData
{
public:
    QVector < QSharedPointer<genericInput> > inputs;
    QVector < QSharedPointer<genericInput> > outputs;
    QSharedPointer<NineMLComponent> component;
    NineMLComponentData(QSharedPointer<NineMLComponent>data);
    NineMLComponentData(QSharedPointer <NineMLComponentData>data);
    void copyFrom(QSharedPointer <NineMLComponentData>, QSharedPointer<NineMLComponent>, QSharedPointer<NineMLComponentData> thisSharedPointer);
    NineMLComponentData& operator=(const NineMLComponentData& data);
    NineMLComponentData(){}
    virtual ~NineMLComponentData();
    QString getXMLName();
    void matchPorts();
    QStringList getPortMatches(int index, bool isOutput);
    void removeReferences();
    QSharedPointer<systemObject> owner;
    void import_parameters_from_xml(QDomNode &e);
    void migrateComponent(QSharedPointer<NineMLComponent> newComponent);
    void addInput(QSharedPointer <NineMLComponentData>, bool = false);
    void copyParsFrom(QSharedPointer <NineMLComponentData> data);
};


#endif // NINEML_STRUCTS_H
