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
#include "NL_systemobject.h"

// This is the number of connections there has to be for the system to
// start writing these connections into a binary file. If there are
// fewer connections than this number, the connections will be written
// inline into the XML.
#define MIN_CONNS_TO_FORCE_BINARY 30

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
    UNIT_S,
    UNIT_A,
    UNIT_cd,
    UNIT_mol,
    UNIT_degC,
    UNIT_s,
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
class Component;
class StateVariable;

typedef enum
{
    COMPONENT_PARAMETER,
    COMPONENT_ANALOG_PORT,
    COMPONENT_EVENT_PORT,
    COMPONENT_IMPULSE_PORT,
    COMPONENT_MATHINLINE,
    COMPONENT_TRIGGER,
    COMPONENT_STATE_VARIABLE,
    COMPONENT_ALIAS,
    COMPONENT_TIME_DERIVATIVE,
    COMPONENT_STATE_ASSIGNMENT,
    COMPONENT_EVENT_OUT,
    COMPONENT_IMPULSE_OUT,
    COMPONENT_ON_CONDITION,
    COMPONENT_ON_EVENT,
    COMPONENT_ON_IMPULSE,
    COMPONENT_REGIME,
    COMPONENT
}ComponentObjectType;

/*!
 * \brief The ComponentObject class is the top level object which all component type objects inherit. The type can be
 * querieid from the virtual Type function which will return a ComponentObjectType. This allows objects to be re-cast
 * from ComponentObjects to the correct type.
 */
class ComponentModelObject
{
public:
    ComponentModelObject(){}
    ~ComponentModelObject(){}
    virtual ComponentObjectType Type() = 0;
};

/*!
 * \brief The ComponentRootObject class represents a component layer object which used to be used to hold the location
 * of the original compoenent model file. It is inhereted by the ComponentLayout as is mostly defunct now that projects
 * are used.
 *
 * TODO: Candidiate for removal
 */
class ComponentRootObject
{
public:
    ComponentRootObject(){path="temp"; filePath="";}
    ~ComponentRootObject(){}
    QString path;
    QString filePath;
};


/*!
 * \brief The Parameter class represent a paramater model object in the compenent layer schema. Signals are availabel for
 * when the name changes so that any visual objects (graphics item type objects) can detect changes and update their
 * visual representation. Functions exist for reading and writing from component XML file.
 */
class Parameter: public QObject, public ComponentModelObject {
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
    virtual ComponentObjectType Type(){return COMPONENT_PARAMETER;}

    /*!
     * The file_name, if the data are saved as explicit binary data.
     */
    QString filename;
signals:
    void nameChanged();
};

/*!
 * \brief The ParameterInstance class represents an instance of the data that is described by a parameter componenet.
 * Put another way when the properties of a parameter are read from the network layer this is the data that is used to
 * instanciate the instances which match the description of the underlying parameter object. Reading and writing functions
 * are avilaable for XML and binary data formats.
 *
 */
class ParameterInstance {
public:
    QString name;
    dim * dims;
    QVector < double > value;
    QVector < int > indices;
    ParameterType currType;
    int seed;

    /*!
     * The file_name, if the data are saved as explicit binary data.
     */
    QString filename;
    ParameterInstance(Parameter *data);
    ParameterInstance(ParameterInstance *data);
    ParameterInstance(QString dimString){dims = new dim(dimString);}
    ~ParameterInstance(){delete dims;}
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


/*!
 * \brief The Port class represent a port model object in the compenent layer schema. Signals are availabel for
 * when the name changes so that any visual objects (graphics item type objects) can detect changes and update their
 * visual representation. Functions exist for reading and writing from component XML file.
 * TODO: This should probably be a virtual class as ports are either analogue, impule or event.
 */
class Port: public QObject, public ComponentModelObject {
    Q_OBJECT
public:
    QString name;
    dim * dims;
    Port(Port *data);
    QString getName();
    void setName(QString name);
    Port(){dims = new dim("?"); isPost = false;}
    virtual ~Port(){delete dims;}
    void readIn(QDomElement e);
    void writeOut(QDomDocument *doc, QDomElement &parent);
    virtual bool isAnalog();
    bool isPost;
signals:
    void nameChanged();
};

/*!
 * \brief The AnalogPort class represent an AnalogPort model object in the compenent layer schema.
 */
class AnalogPort: public Port {
public:
    AnalogPortMode mode;
    ReduceOperation op;
    StateVariable *variable;
    AnalogPort(AnalogPort *data);
    AnalogPort() : Port() {variable=NULL; op = ReduceOperationAddition; isPerConn = false;}
    virtual ~AnalogPort(){}
    void readIn(QDomElement e);
    void writeOut(QDomDocument *doc, QDomElement &parent);
    bool isAnalog();
    virtual ComponentObjectType Type(){return COMPONENT_ANALOG_PORT;}
    int validateAnalogPort(Component *component, QStringList *);
    bool isPerConn;
};

/*!
 * \brief The EventPort class represent an EventPort model object in the compenent layer schema.
 */
class EventPort: public Port {
public:
    EventPortMode mode;
    EventPort(EventPort *data);
    EventPort() : Port() {mode=EventSendPort;}
    virtual ~EventPort(){}
    void readIn(QDomElement e);
    void writeOut(QDomDocument *doc, QDomElement &parent);
    bool isAnalog();
    virtual ComponentObjectType Type(){return COMPONENT_EVENT_PORT;}
};

/*!
 * \brief The ImpulsePort class represent an ImpulsePort model object in the compenent layer schema.
 */
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
    virtual ComponentObjectType Type(){return COMPONENT_IMPULSE_PORT;}
    int validateImpulsePort(Component *component, QStringList *);
};

/*!
 * \brief The MathInLine class is a mathematical expression using terms defined in the compenent layer. It is used to
 * represent the mathematics in in model objects such as Alias, OnCondition, TimeDerivative, etc.
 */
class MathInLine: public ComponentModelObject {
public:

    QString equation;
    MathInLine(MathInLine *data);
    MathInLine(){}
    virtual ~MathInLine(){}
    QString getHTMLSafeEquation();
    int validateMathInLine(Component *component, QStringList * errs);
    int validateMathInLine(NineMLLayout * component, QStringList * errs);
    void readIn(QDomElement e);
    void writeOut(QDomDocument *doc, QDomElement &parent);
    virtual ComponentObjectType Type(){return COMPONENT_MATHINLINE;}

private:
    /*!
     * Remove an approved list of operators from the testequation,
     * then set up a function list of allowed functions. If what
     * remains in testequation is only FuncList, then the testequation
     * will validate.
     */
    void validateMathSetup(QString& testequation, QStringList& FuncList);
};

/*!
 * \brief The Trigger class is used by OnConditions to hold some mathematic expressesions which evauluate to a conditional.
 */
class Trigger: public ComponentModelObject {
public:
    MathInLine *maths;
    Trigger(Trigger *data);
    Trigger(){maths=new MathInLine;}
    virtual ~Trigger();
    int validateTrigger(Component *component, QStringList * errs);
    void readIn(QDomElement e);
    void writeOut(QDomDocument *doc, QDomElement &parent);
    virtual ComponentObjectType Type(){return COMPONENT_TRIGGER;}
};

/*!
 * \brief The StateVariable class represent a StateVariable model object in the compenent layer schema. It
 * is a type of parameter and can be used as such in mathmatic expressions.
 */
class StateVariable: public Parameter {
public:
    StateVariable(StateVariable *data);
    StateVariable() : Parameter() {}
    virtual ~StateVariable();
    void readIn(QDomElement e);
    void writeOut(QDomDocument *doc, QDomElement &parent);
    virtual ComponentObjectType Type(){return COMPONENT_STATE_VARIABLE;}
};

/*!
 * \brief The StateVariableInstance class represents an instance of the data that is described by a StateVariable componenet.
 * Put another way when the properties of a state variable are read from the network layer this is the data that is used to
 * instanciate the instances which match the description of the underlying state variable object. Reading and writing functions
 * are avilaable for XML and binary data formats.
 */
class StateVariableInstance: public ParameterInstance {
public:
    StateVariableInstance(StateVariable *data);
    StateVariableInstance(StateVariableInstance *data);
    //StateVariableData(){}
    virtual ~StateVariableInstance(){}
    void readIn(QDomElement e);
    void writeOut(QDomDocument *doc, QDomElement &parent);
};


/*!
 * \brief The Alias class represents an mathmatic expression which can then be used a state variable in expressions.
 */
class Alias: public StateVariable {
public:
    //QString name;
    //dim * dims;
    MathInLine* maths;
    Alias(Alias *data);
    Alias(): StateVariable() {maths=new MathInLine();}
    virtual ~Alias();
    int validateAlias(Component *component, QStringList * errs);
    int validateAlias(NineMLLayout *component, QStringList * errs);
    void readIn(QDomElement e);
    void writeOut(QDomDocument *doc, QDomElement &parent);
    virtual ComponentObjectType Type(){return COMPONENT_ALIAS;}

};

/*!
 * \brief The TimeDerivative class is an object for holding data of a time derivative defined in the component layer schema.
 * Usual component layer functiosn exist for file reading/writing.
 */
class TimeDerivative: public ComponentModelObject {
public:
    // temp name
    QString variable_name;

    StateVariable *variable;
    MathInLine* maths;
    TimeDerivative(TimeDerivative *data);
    TimeDerivative(){variable=NULL; maths=new MathInLine;}
    virtual ~TimeDerivative();
    int validateTimeDerivative(Component *component, QStringList * errs);
    void readIn(QDomElement e);
    void writeOut(QDomDocument *doc, QDomElement &parent);
    virtual ComponentObjectType Type(){return COMPONENT_TIME_DERIVATIVE;}
};

/*!
 * \brief The StateAssignment class  is an object for holding data of a state assignment defined in the component layer schema.
 * Usual component layer functiosn exist for file reading/writing.
 */
class StateAssignment: public ComponentModelObject {
public:
    // temp name
    QString name;

    StateVariable *variable;
    MathInLine *maths;
    StateAssignment(StateAssignment *data);
    StateAssignment(){variable=NULL; maths=new MathInLine;}
    virtual ~StateAssignment();
    int validateStateAssignment(Component *component, QStringList *);
    int validateStateAssignment(NineMLLayout *component, QStringList * errs);
    void readIn(QDomElement e);
    void writeOut(QDomDocument *doc, QDomElement &parent);
    virtual ComponentObjectType Type(){return COMPONENT_STATE_ASSIGNMENT;}
};

/*!
 * \brief The EventOut class  is an object for holding data of a EventOutt defined in the component layer schema.
 * Usual component layer functions exist for file reading/writing.
 */
class EventOut: public ComponentModelObject {
public:
    // temp name
    QString port_name;
    EventPort *port;
    EventOut(EventOut *);
    EventOut(){port=NULL;}
    virtual ~EventOut(){}
    int validateEventOut(Component *component, QStringList *);
    void readIn(QDomElement e);
    void writeOut(QDomDocument *doc, QDomElement &parent);
    virtual ComponentObjectType Type(){return COMPONENT_EVENT_OUT;}
};

/*!
 * \brief The ImpulseOut class  is an object for holding data of a ImpulseOut defined in the component layer schema.
 * Usual component layer functions exist for file reading/writing.
 */
class ImpulseOut: public ComponentModelObject {
public:
    // temp name
    QString port_name;
    ImpulsePort *port;
    ImpulseOut(ImpulseOut *);
    ImpulseOut(){port=NULL;}
    virtual ~ImpulseOut(){}
    int validateImpulseOut(Component *component, QStringList * errs);
    void readIn(QDomElement e);
    void writeOut(QDomDocument *doc, QDomElement &parent);
    virtual ComponentObjectType Type(){return COMPONENT_IMPULSE_OUT;}
};

/*!
 * \brief The OnCondition class  is an obect for holding data of a OnCondition defined in the component layer schema. I.e a
 * transition between regimes. It therefore needs to have a target regime and will be owned by another regime.
 * Usual component layer functions exist for file reading/writing.
 */
class OnCondition: public ComponentModelObject {
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
    int validateOnCondition(Component *component, QStringList * errs);
    void readIn(QDomElement e);
    void writeOut(QDomDocument *doc, QDomElement &parent);
    virtual ComponentObjectType Type(){return COMPONENT_ON_CONDITION;}
};

/*!
 * \brief The OnEvent class  is an object for holding data of a OnEvent defined in the component layer schema. I.e a
 * transition between regimes. It therefore needs to have a target regime and will be owned by another regime.
 * Usual component layer functions exist for file reading/writing.
 */
class OnEvent: public ComponentModelObject {
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
    int validateOnEvent(Component *component, QStringList * errs);
    void readIn(QDomElement e);
    void writeOut(QDomDocument *doc, QDomElement &parent);
    virtual ComponentObjectType Type(){return COMPONENT_ON_EVENT;}
};

/*!
 * \brief The OnImpulse class  is an object for holding data of a OnImpulse defined in the component layer schema. I.e a
 * transition between regimes. It therefore needs to have a target regime and will be owned by another regime.
 * Usual component layer functions exist for file reading/writing.
 */
class OnImpulse: public ComponentModelObject {
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
    int validateOnImpulse(Component *component, QStringList * errs);
    void readIn(QDomElement e);
    void writeOut(QDomDocument *doc, QDomElement &parent);
    virtual ComponentObjectType Type(){return COMPONENT_ON_IMPULSE;}
};


/*!
 * \brief The Regime class is an object for holding data of a Regime defined in the component layer schema.
 */
class Regime: public ComponentModelObject {
public:
    QString name;
    QVector <TimeDerivative*> TimeDerivativeList;
    QVector <OnCondition* > OnConditionList;
    QVector <OnEvent*> OnEventList;
    QVector <OnImpulse*> OnImpulseList;
    Regime(Regime *data);
    Regime(){}
    virtual ~Regime();
    int validateRegime(Component *component, QStringList * errs);
    void readIn(QDomElement e);
    void writeOut(QDomDocument *doc, QDomElement &parent);
    virtual ComponentObjectType Type(){return COMPONENT_REGIME;}
};

/*!
 * \brief The Component class is the data object which holds all data representing a component. I.e. all the regimes,
 * ports, paramaters, state variables etc. It is the root holder of these objects and stores many of them in vectors.
 * It is responsible for ownership.
 */
class Component: public ComponentModelObject, public ComponentRootObject
{
public:

    //temp name
    QString initial_regime_name;

    QString name;
    QString type;
    bool islearning;
    Regime *initial_regime;
    QVector <Regime*> RegimeList;
    QVector <StateVariable*> StateVariableList;
    QVector <Parameter*> ParameterList;
    QVector <Alias*> AliasList;
    QVector <AnalogPort*> AnalogPortList;
    QVector <EventPort*> EventPortList;
    QVector <ImpulsePort*> ImpulsePortList;
    Component(QSharedPointer<Component>data);
    Component& operator=(const Component& data);
    Component();
    virtual ~Component();
    void updateFrom(QSharedPointer<Component>data);
    QStringList validateComponent();
    void load(QDomDocument *doc);
    void write(QDomDocument *doc);
    virtual ComponentObjectType Type(){return COMPONENT;}
    QUndoStack undoStack;
    QSharedPointer<Component> editedVersion;
    QString getXMLName();
};


/*!
 * \brief The ComponentRootInstance class is a virtual class  that represents an instance of the data that is described by a
 * whole component or layout.
 * Put another way when the properties of a whole component are read from the network layer this is the data that is used to
 * instanciate the instances which match the description of the underlying parameter and state variable objects.
 *
 */
class ComponentRootInstance
{
public:
    NineMLType type;
    QVector <StateVariableInstance*> StateVariableList;
    QVector <ParameterInstance*> ParameterList;
    void write_node_xml(QXmlStreamWriter &);
    ComponentRootInstance(){}
    virtual ~ComponentRootInstance(){}
};

/*!
 * \brief The ComponentInstance class derives from the ComponentRootInstance class but is specific for SpineML Components and not Layouts.
 */
class ComponentInstance: public ComponentRootInstance
{
public:
    QVector < QSharedPointer<genericInput> > inputs;
    QVector < QSharedPointer<genericInput> > outputs;
    QSharedPointer<Component> component;
    ComponentInstance(QSharedPointer<Component>data);
    ComponentInstance(QSharedPointer <ComponentInstance>data, bool copy_io = false);
    void remapPointers(QMap <systemObject *, QSharedPointer <systemObject> > pointerMap);
    void copyFrom(QSharedPointer <ComponentInstance>, QSharedPointer<Component>, QSharedPointer<ComponentInstance> thisSharedPointer);
    ComponentInstance& operator=(const ComponentInstance& data);
    ComponentInstance(){}
    virtual ~ComponentInstance();
    QString getXMLName();
    void matchPorts();
    QStringList getPortMatches(int index, bool isOutput);
    void removeReferences();
    QSharedPointer<systemObject> owner;
    void import_parameters_from_xml(QDomNode &e);
    void migrateComponent(QSharedPointer<Component> newComponent);
    void addInput(QSharedPointer <ComponentInstance>, bool = false);
    void copyParsFrom(QSharedPointer <ComponentInstance> data);
};


#endif // NINEML_STRUCTS_H
