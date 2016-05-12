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
**           Author: Paul Richmond                                        **
**  Website/Contact: http://bimpa.group.shef.ac.uk/                       **
****************************************************************************/

#ifndef NINEML_GRAPHICSITEMS_H
#define NINEML_GRAPHICSITEMS_H

#include <QtGui>
#include "nineML_classes.h"
#include "gvitems.h"

class ArrowItem;
class NineMLNodeItem;
class NineMLTransitionItem;

class NineMLALScene;
class RootComponentItem;
class PropertiesManager;
class ParameterListGraphicsItem;
class PortListGraphicsItem;
class TimeDerivativeTextItem;
class ParameterTextItem;
class AnalogPortTextItem;
class EventPortTextItem;
class ImpulsePortTextItem;
class OnConditionTriggerTextItem;
class OnEventTriggerTextItem;
class OnImpulseTriggerTextItem;



/******************************************/
/* Abstract Items */


class ArrowItem : public QObject, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)
public:
    ArrowItem();
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *);
    QRectF boundingRect() const;
    void setPath(QPainterPath path);
    void setPenColour(QColor col);
    void setLineWidth(int w);
private:
    int width;
    QColor colour;
    QPainterPath path;
    QPolygonF arrow_head;
};

// TextItemGroup derives from QGraphicsItem (amoung other things)
class NineMLNodeItem : public TextItemGroup, public GVNode
{
    Q_OBJECT
public:
    NineMLNodeItem(GVLayout *layout, QString name);
    ~NineMLNodeItem();
    virtual void updateLayout();
    void updateGVData();
    void addMember(GroupedTextItem *item);            //overwrite TextItemGroup addMember
    void addMemberAtIndex(GroupedTextItem *item, int index);
    void removeMember(GroupedTextItem* item);          //overwrite TextItemGroup removeMember
};


typedef enum {
    TRANSITION_TYPE_ON_EVENT,
    TRANSITION_TYPE_ON_CONDITION,
    TRANSITION_TYPE_ON_IMPULSE
} NineMLTransitionItemType;


class NineMLTransitionItem: public TextItemGroup, public GVEdge
{
    Q_OBJECT
public:
    NineMLTransitionItem(GVLayout *layout, Agnode_t *src, Agnode_t *dst, QGraphicsScene *scene = 0);
    ~NineMLTransitionItem();
    virtual void updateLayout();
    void updateGVData();
    void addMember(GroupedTextItem * item);            //overwrite TextItemGroup addMember
    void addMemberAtIndex(GroupedTextItem *item, int index);
    void removeMember(GroupedTextItem* item);          //overwrite TextItemGroup removeMember
    virtual NineMLTransitionItemType transitionType() = 0;
protected:
    ArrowItem *arrow;
};


class NineMLTextItem: public GroupedTextItem
{
    Q_OBJECT
public:
    NineMLTextItem(GVItem *gv_item, TextItemGroup *parent = 0);
    ~NineMLTextItem();
    void setPlainText(const QString &text);                             //overwite GroupTextItem (to ensure Call of updatedims)
private:
    GVItem *gv_item;
};


/******************************************/
/* Concrete items */


class RegimeGraphicsItem : public NineMLNodeItem
{
    Q_OBJECT
public:
    RegimeGraphicsItem(Regime *r, RootComponentItem *root);
    void addTimeDerivativeItem(TimeDerivative* td);
    QString getRegimeName();
    bool isRegime(Regime *r);
    int type() const { return Type; }
public slots:
    void setRegimeName(QString n);
    void updateContent();
protected:
    virtual void handleSelection();

public:
    Regime *regime;
    enum { Type = UserType + 1 };
    static const int padding = 10;
private :
    RootComponentItem *root;
};


class TimeDerivativeTextItem: public NineMLTextItem
{
    Q_OBJECT
public:
    TimeDerivativeTextItem(RegimeGraphicsItem *parent, TimeDerivative* time_derivative, RootComponentItem *root);
    enum { Type = UserType + 2 };
    int type() const { return Type; }
    StateVariable* getVariable();
    MathInLine* getMaths();
public slots:
    virtual void updateContent();
    void setVariable(QString p);
    void setMaths(QString m);
protected:
    virtual void handleSelection();

public:
    TimeDerivative *time_derivative;
private :
    RootComponentItem *root;
};


class OnConditionGraphicsItem : public NineMLTransitionItem
{
    Q_OBJECT
public:
    OnConditionGraphicsItem(Regime *src_regime, OnCondition *c, RootComponentItem *root);
    enum { Type = UserType + 3 };
    int type() const { return Type; }
    void addStateAssignment(StateAssignment *sa);
    void addEventOut(EventOut* eo);
    void addImpulseOut(ImpulseOut* io);
    MathInLine* getTriggerMaths();
    Regime* getSynapseRegime();
    Regime* getSourceRegime();
    static const int padding = 4;
    virtual NineMLTransitionItemType transitionType(){return TRANSITION_TYPE_ON_CONDITION;}
public slots:
    void setTriggerMaths(QString m);
    void setSynapseRegime(QString m);
protected:
    virtual void handleSelection();
public:
    OnCondition *on_condition;
    Regime *src_regime;
private :
    OnConditionTriggerTextItem *trigger_item;
    RootComponentItem *root;
};


class OnConditionTriggerTextItem: public NineMLTextItem
{
    Q_OBJECT
public:
    OnConditionTriggerTextItem(OnConditionGraphicsItem *parent, Trigger* trigger, RootComponentItem *root);
    enum { Type = UserType + 4 };
    int type() const { return Type; }
    MathInLine* getMaths();
public slots:
    void setMaths(QString m);
    virtual void updateContent();
protected:
    virtual void handleSelection();
public:
    Trigger *trigger;
private :
    RootComponentItem *root;
};


class StateAssignmentTextItem: public NineMLTextItem
{
    Q_OBJECT
public:
    StateAssignmentTextItem(NineMLTransitionItem *parent, StateAssignment* assigment, RootComponentItem *root);
    enum { Type = UserType + 5 };
    int type() const { return Type; }
    StateVariable* getVariable();
    MathInLine* getMaths();
public slots:
    virtual void updateContent();
    void setVariable(QString p);
    void setMaths(QString m);
protected:
    virtual void handleSelection();
public:
    StateAssignment *assignment;
private :
    RootComponentItem *root;
};


class EventOutTextItem: public NineMLTextItem
{
    Q_OBJECT
public:
    EventOutTextItem(NineMLTransitionItem *parent, EventOut* event_out, RootComponentItem *root);
    enum { Type = UserType + 6 };
    int type() const { return Type; }
    EventPort* getEventPort();
public slots:
    virtual void updateContent();
    void setEventPort(QString m);
protected:
    virtual void handleSelection();
public:
    EventOut *event_out;
private :
    RootComponentItem *root;
};


class ParameterListGraphicsItem : public NineMLNodeItem
{
    Q_OBJECT
public:
    ParameterListGraphicsItem(RootComponentItem *root);
    void addParameterItem(Parameter* p);
    void addStateVariableItem(StateVariable* p);
    void addAliasItem(Alias* p);
public:
    enum { Type = UserType + 7 };
    int type() const { return Type; }
    static const int padding = 5;

protected:
    virtual void handleSelection();
private :
    RootComponentItem *root;
};


class ParameterTextItem: public NineMLTextItem
{
    Q_OBJECT
public:
    ParameterTextItem(ParameterListGraphicsItem *parent, Parameter* param, RootComponentItem *root);
    enum { Type = UserType + 8 };
    int type() const { return Type; }
    QString getName();
public slots:
    virtual void updateContent();
    void setName(QString m);
    void setDimsPrefix(QString p);
    void setDimsUnit(QString u);
protected:
    virtual void handleSelection();
public:
    Parameter *parameter;
private :
    RootComponentItem *root;
};


class StateVariableTextItem: public NineMLTextItem
{
    Q_OBJECT
public:
    StateVariableTextItem(ParameterListGraphicsItem *parent, StateVariable* state_var, RootComponentItem *root);
    enum { Type = UserType + 9 };
    int type() const { return Type; }
    QString getName();
public slots:
    virtual void updateContent();
    void setName(QString m);
    void setDimsPrefix(QString p);
    void setDimsUnit(QString u);
protected:
    virtual void handleSelection();
public:
    StateVariable *state_variable;
private :
    RootComponentItem *root;
};


class AliasTextItem: public NineMLTextItem
{
    Q_OBJECT
public:
    AliasTextItem(ParameterListGraphicsItem *parent, Alias* alias, RootComponentItem *root);
    enum { Type = UserType + 10 };
    int type() const { return Type; }
    QString getName();
    MathInLine* getMaths();
public slots:
    virtual void updateContent();
    void setName(QString m);
    void setMaths(QString m);
protected:
    virtual void handleSelection();
public:
    Alias *alias;
private :
    RootComponentItem *root;
};


class PortListGraphicsItem : public NineMLNodeItem
{
    Q_OBJECT
public:
    PortListGraphicsItem(RootComponentItem *root);
    void addAnalogePortItem(AnalogPort *ap);
    void addEventPortItem(EventPort *ep);
    void addImpulsePortItem(ImpulsePort *ip);

public:
    enum { Type = UserType + 11 };
    int type() const { return Type; }
    static const int padding = 5;
protected:
    virtual void handleSelection();
private :
    RootComponentItem *root;
};


class AnalogPortTextItem: public NineMLTextItem
{
    Q_OBJECT
public:
    AnalogPortTextItem(PortListGraphicsItem *parent, AnalogPort* p, RootComponentItem *root);
    enum { Type = UserType + 12 };
    int type() const { return Type; }
    QString getName();
    StateVariable* getVariable();
    AnalogPortMode getPortMode();
    ReduceOperation getPortReduceOp();

public slots:
    virtual void updateContent();
    void setName(QString);
    void setVariable(QString p);
    void setPortMode(QString p);
    void setPortReduceOp(QString p);
    void setDimsPrefix(QString p);
    void setDimsUnit(QString u);
    void setIsPostState(bool b);

protected:
    virtual void handleSelection();
public:
    AnalogPort *port;
private :
    RootComponentItem *root;
};


class EventPortTextItem: public NineMLTextItem
{
    Q_OBJECT
public:
    EventPortTextItem(PortListGraphicsItem *parent, EventPort* p, RootComponentItem *root);
    enum { Type = UserType + 13 };
    int type() const { return Type; }
    QString getName();
    EventPortMode getPortMode();

public slots:
    virtual void updateContent();
    void setName(QString n);
    void setPortMode(QString p);
    void setIsPostState(bool b);

protected:
    virtual void handleSelection();
public:
    EventPort *port;
private :
    RootComponentItem *root;
};


class OnEventGraphicsItem : public NineMLTransitionItem
{
    Q_OBJECT
public:
    OnEventGraphicsItem(Regime *src_regime, OnEvent *e, RootComponentItem *root);
    enum { Type = UserType + 14 };
    int type() const { return Type; }

    void addStateAssignment(StateAssignment *sa);
    void addEventOut(EventOut* eo);
    void addImpulseOut(ImpulseOut* io);

    EventPort* getEventPort();
    Regime* getSynapseRegime();
    Regime* getSourceRegime();
    virtual NineMLTransitionItemType transitionType(){return TRANSITION_TYPE_ON_EVENT;}

    static const int padding = 4;
public slots:
    void setEventPort(QString m);
    void setSynapseRegime(QString m);
protected:
    virtual void handleSelection();
public:
    OnEvent *on_event;
    Regime *src_regime;
private :
    OnEventTriggerTextItem *trigger_item;
    RootComponentItem *root;
};


class OnEventTriggerTextItem: public NineMLTextItem
{
    Q_OBJECT
public:
    OnEventTriggerTextItem(OnEventGraphicsItem *parent, RootComponentItem *root);
    enum { Type = UserType + 15 };
    int type() const { return Type; }
    EventPort* getEventPort();
public slots:
    void setEventPort(QString m);
    virtual void updateContent();
protected:
    virtual void handleSelection();
public:
    OnEvent *on_event;//EventPort *event_port; //stored in OnEvent
private :

    RootComponentItem *root;
};


class ImpulsePortTextItem: public NineMLTextItem
{
    Q_OBJECT
public:
    ImpulsePortTextItem(PortListGraphicsItem *parent, ImpulsePort* p, RootComponentItem *root);
    enum { Type = UserType + 16 };
    int type() const { return Type; }
    QString getName();
    Parameter *getParameter();
    ImpulsePortMode getPortMode();

public slots:
    virtual void updateContent();
    void setName(QString n);
    void setParameter(QString n);
    void setPortMode(QString p);
    void setDimsPrefix(QString p);
    void setDimsUnit(QString u);

protected:
    virtual void handleSelection();
public:
    ImpulsePort *port;
private :
    RootComponentItem *root;
};


class ImpulseOutTextItem: public NineMLTextItem
{
    Q_OBJECT
public:
    ImpulseOutTextItem(NineMLTransitionItem *parent, ImpulseOut* i, RootComponentItem *root);
    enum { Type = UserType + 17 };
    int type() const { return Type; }
    ImpulsePort* getImpulsePort();
public slots:
    virtual void updateContent();
    void setImpulsePort(QString m);
protected:
    virtual void handleSelection();
public:
    ImpulseOut *impulse_out;
private :
    RootComponentItem *root;
};


class OnImpulseGraphicsItem : public NineMLTransitionItem
{
    Q_OBJECT
public:
    OnImpulseGraphicsItem(Regime *src_regime, OnImpulse *e, RootComponentItem *root);
    enum { Type = UserType + 18 };
    int type() const { return Type; }

    void addStateAssignment(StateAssignment *sa);
    void addEventOut(EventOut* eo);
    void addImpulseOut(ImpulseOut* io);

    ImpulsePort* getImpulsePort();
    Regime* getSynapseRegime();
    Regime* getSourceRegime();
    virtual NineMLTransitionItemType transitionType(){return TRANSITION_TYPE_ON_IMPULSE;}

    static const int padding = 4;
public slots:
    void setImpulsePort(QString m);
    void setSynapseRegime(QString m);
protected:
    virtual void handleSelection();
public:
    OnImpulse *on_impulse;
    Regime *src_regime;
private :
    OnImpulseTriggerTextItem *trigger_item;
    RootComponentItem *root;
};


class OnImpulseTriggerTextItem: public NineMLTextItem
{
    Q_OBJECT
public:
    OnImpulseTriggerTextItem(OnImpulseGraphicsItem *parent, RootComponentItem *root);
    enum { Type = UserType + 19 };
    int type() const { return Type; }
    ImpulsePort* getImpulsePort();
public slots:
    void setImpulsePort(QString m);
    virtual void updateContent();
protected:
    virtual void handleSelection();
public:
    OnImpulse *on_impulse;//ImpulsePort *impulse_port; //stored in OnImpulse
private :

    RootComponentItem *root;
};


#endif // NINEML_GRAPHICSITEMS_H
