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


#include <algorithm>
#include <typeinfo>

#include "SC_component_graphicsitems.h"
#include "SC_component_scene.h"
#include "SC_component_propertiesmanager.h"
#include "SC_undocommands.h"




/**************************************************************************************************************************/

/* Arrow item*/
ArrowItem::ArrowItem()
    : QGraphicsItem()
{
    width = 1;
    colour = Qt::black;
    this->setZValue(0);
}

void ArrowItem::setPath(QPainterPath path)
{
    prepareGeometryChange();

    this->path = path;

    //calculate arrow head polygon
    QPointF end_point = path.pointAtPercent(1.0);
    QPointF temp_end_point = path.pointAtPercent(0.95);
    QLineF line = QLineF(end_point, temp_end_point).unitVector();
    line.setLength(10.0);
    QPointF t = line.p2() - line.p1();
    QLineF normal = line.normalVector();
    normal.setLength(normal.length()*0.35);
    QPointF a1 = normal.p2() + t;
    normal.setLength(-normal.length());
    QPointF a2 = normal.p2() + t;
    arrow_head.clear();
    arrow_head << end_point << a1 << a2 << end_point;
    path.addPolygon(arrow_head);
}

void ArrowItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->setPen(QPen(colour, width));
    painter->setBrush(QColor(0,0,0,0));
    painter->drawPath(path);
    painter->setBrush(colour);
    painter->drawPolygon(arrow_head);
}

QRectF ArrowItem::boundingRect() const
{
    QRectF bounds = path.boundingRect().united(arrow_head.boundingRect());
    qreal w = width/2.0;
    bounds.adjust(-w, -w, w, w);
    return bounds;
}

void ArrowItem::setPenColour(QColor col)
{
    this->colour = col;
}

void ArrowItem::setLineWidth(int w)
{
    width = w;
}


/* NineMLNodeItem */

NineMLNodeItem::NineMLNodeItem(GVLayout *layout, QString name)
    : TextItemGroup(), GVNode(layout, name)
{
    //in front of labels
    this->setZValue(1);
}

NineMLNodeItem::~NineMLNodeItem()
{
}

void NineMLNodeItem::updateLayout()
{
    //DBG() << "Node item updateLayout (from GVItem virtual method)";
    QRectF bounds = boundingRect();
    QPointF offset = QPointF(bounds.width(), bounds.height());
    offset /= 2.0;
    QPointF pos = GVNode::getGVNodePosition(offset);
    pos += QPointF(padding, padding);
    //DBG() << "Calling setPos(" << pos << ") on node position...";
    // setPos is a Qt thing: QGraphicsItem::setPos
    setPos(pos);
}

void NineMLNodeItem::addMember(GroupedTextItem *item)
{
    TextItemGroup::addMember(item);
    updateGVData();
}

void NineMLNodeItem::addMemberAtIndex(GroupedTextItem *item, int index)
{
    members.insert(members.begin()+index, item);
    updateItemDimensions();
    updateGVData();
}

void NineMLNodeItem::removeMember(GroupedTextItem *item)
{
    TextItemGroup::removeMember(item);
    updateItemDimensions();
    updateGVData();
}

void NineMLNodeItem::updateGVData()
{
    TextItemGroup::updateItemDimensions();
    QRectF bounds = boundingRect();
    GVNode::setGVNodeSize((bounds.width())/GV_DPI,  (bounds.height())/GV_DPI);
    //qDebug() << "NineMLNodeItem updateGVData " << bounds.width() << " " << bounds.height() << " pixels";
    layout->updateLayout();
}


NineMLTransitionItem::NineMLTransitionItem(GVLayout *layout, Agnode_t *src, Agnode_t *dst, QGraphicsScene *scene)
    : TextItemGroup(), GVEdge(layout, src, dst)
{
    arrow = new ArrowItem();
    //add direct to scene if possible for correct z ordering
    if (scene){
        scene->addItem(arrow);
    }
    //else
    //    arrow->setParentItem(this);
    this->setZValue(1);
}

NineMLTransitionItem::~NineMLTransitionItem()
{
    //arrow is a member of the scene (no parent) so remove and then delete
    arrow->scene()->removeItem(arrow);
    //arrow->deleteLater();
    delete arrow;
    arrow = NULL;
}

void NineMLTransitionItem::updateLayout()
{
    //update textboxgroup (i.e. gv label) position
    QRectF bounds = TextItemGroup::boundingRect();
    QPointF offset = QPointF(bounds.width(), bounds.height());
    offset /= 2.0;
    QPointF pos = GVEdge::getGVEdgeLabelPosition(offset);
    pos += QPointF(padding,padding);
    setPos(pos);

    //update curve points and set arrow path
    QPainterPath path = QPainterPath();
    QPointF spline_start = GVEdge::getGVEdgeSplinesPoint(0);
    path.moveTo(spline_start.x(), spline_start.y());
    for(int i=1; i<GVEdge::getGVEdgeSplinesCount(); i+=3)
    {
        QPointF p1 = GVEdge::getGVEdgeSplinesPoint(i+0);
        QPointF p2 = GVEdge::getGVEdgeSplinesPoint(i+1);
        QPointF p3 = GVEdge::getGVEdgeSplinesPoint(i+2);
        path.cubicTo(p1.x(), p1.y(), p2.x(), p2.y(), p3.x(), p3.y());
    }
    QPointF spline_end = GVEdge::getGVEdgeSplinesEndPoint();
    path.lineTo(spline_end.x(), spline_end.y());
    path.translate(-TEXT_PADDING, -TEXT_PADDING);
    arrow->setPath(path);
}

void NineMLTransitionItem::updateGVData()
{
    TextItemGroup::updateItemDimensions();
    QRectF bounds = TextItemGroup::boundingRect();
    GVEdge::setGVEdgeLabelSize((int)bounds.width(), (int)bounds.height());
    //DBG() << "NineMLTransitionItem updateGVData " << bounds.width() << " " << bounds.height();
    layout->updateLayout();
}

void NineMLTransitionItem::addMember(GroupedTextItem *item)
{
    TextItemGroup::addMember(item);
    updateGVData();
}

void NineMLTransitionItem::addMemberAtIndex(GroupedTextItem *item, int index)
{
    members.insert(members.begin()+index, item);
    updateItemDimensions();
    updateGVData();
}

void NineMLTransitionItem::removeMember(GroupedTextItem *item)
{
    TextItemGroup::removeMember(item);
    updateItemDimensions();
    updateGVData();
}


/* NineMLTextItem */
NineMLTextItem::NineMLTextItem(GVItem *gv_item, TextItemGroup *parent)
    :GroupedTextItem(parent)
{
    this->gv_item = gv_item;
}

NineMLTextItem::~NineMLTextItem()
{
}

void NineMLTextItem::setPlainText(const QString &text)
{
    GroupedTextItem::setPlainText(text);
    gv_item->updateGVData();
}


RegimeGraphicsItem::RegimeGraphicsItem(Regime* r, RootComponentItem *root)
    : NineMLNodeItem(root->gvlayout, r->name)
{
    regime = r;
    this->root = root;

    //create name
    setRegimeName(regime->name);

    //create regime equations
    for(int i=0; i<r->TimeDerivativeList.size(); i++)
    {
        addTimeDerivativeItem(regime->TimeDerivativeList[i]);
    }
}

void RegimeGraphicsItem::handleSelection()
{
    root->properties->createRegimeProperties(this);
}

void RegimeGraphicsItem::setRegimeName(QString n)
{
    QSharedPointer<Component> oldComponent = QSharedPointer<Component> (new Component(root->al));
    regime->name = n;

    // rename the gv node which is a parent class of
    // RegimeGraphicsItem.  Shouldn't be required - NineMLNodeItem
    // already set name. Nonetheless, if compiling for libgraph, we'll
    // call this, as the old implementation of the function works.
#ifdef USE_LIBGRAPH_NOT_LIBCGRAPH
    renameGVNode(n);
#endif

    //update the dst regime name in any onconditions
    for (int i=0;i<root->al->RegimeList.size();i++)
    {
        Regime *r = root->al->RegimeList[i];
        for (int c=0;c<r->OnConditionList.size();c++)
        {
            OnCondition *oc = r->OnConditionList[c];
            if (oc->target_regime == this->regime)
                oc->target_regime_name = n;
        }
        for (int e=0;e<r->OnEventList.size();e++)
        {
            OnEvent *oe = r->OnEventList[e];
            if (oe->target_regime == this->regime)
                oe->target_regime_name = n;
        }
    }

    updateContent();
    root->notifyDataChange();
    if (qobject_cast < QLineEdit *> (sender())) {
        root->alPtr->undoStack.push(new changeComponent(root, oldComponent, "Set Regime name"));
    } else {
        oldComponent.clear();
    }
}

void RegimeGraphicsItem::updateContent()
{
    QString title_text = regime->name;
    if (root->al->initial_regime == regime) {
        title_text.append( "*");
    }
    title->setPlainText(title_text);
    //need to force a layout call for a TitleTextItem
    updateGVData();
}

QString RegimeGraphicsItem::getRegimeName()
{
    return regime->name;
}

bool RegimeGraphicsItem::isRegime(Regime *r)
{
    return (regime == r);
}

void RegimeGraphicsItem::addTimeDerivativeItem(TimeDerivative *td)
{
    TimeDerivativeTextItem *tdi = new TimeDerivativeTextItem(this, td, root);
    addMember(tdi);
}


/* TimeDerivativeTextItem implementation */
TimeDerivativeTextItem::TimeDerivativeTextItem(RegimeGraphicsItem *parent, TimeDerivative* td,  RootComponentItem *r)
    : NineMLTextItem(parent, parent)
{
    time_derivative = td;
    root = r;
    updateContent();
    if (time_derivative->variable)
        connect(time_derivative->variable, SIGNAL(nameChanged()), this, SLOT(updateContent()));
}

void TimeDerivativeTextItem::handleSelection()
{
    root->properties->createTimeDerivativeProperties(this);
}

StateVariable * TimeDerivativeTextItem::getVariable()
{
    return time_derivative->variable;
}

MathInLine * TimeDerivativeTextItem::getMaths()
{
    return time_derivative->maths;
}

void TimeDerivativeTextItem::setVariable(QString p)
{
    QSharedPointer<Component> oldComponent = QSharedPointer<Component> (new Component(root->al));
    time_derivative->variable_name = p;
    if (time_derivative->variable != NULL)
    {
        disconnect(time_derivative->variable, SIGNAL(nameChanged()), this, SLOT(updateContent()));
    }
    for (int i=0;i<root->al->StateVariableList.size();i++)
    {
        if (root->al->StateVariableList[i]->getName().compare(p)==0){
            time_derivative->variable = root->al->StateVariableList[i];
            connect(root->al->StateVariableList[i], SIGNAL(nameChanged()), this, SLOT(updateContent()));
        }
    }
    updateContent();
    root->notifyDataChange();
    if (qobject_cast < QComboBox *> (sender())) {
        root->alPtr->undoStack.push(new changeComponent(root, oldComponent, "Set TD variable"));
    } else {
        oldComponent.clear();
    }
}

void TimeDerivativeTextItem::setMaths(QString m)
{
    QSharedPointer<Component> oldComponent = QSharedPointer<Component> (new Component(root->al));
    // if the sender is a QLineEdit
    QLineEdit * source = qobject_cast < QLineEdit *> (sender());

    time_derivative->maths->equation = m;
    QStringList errs;
    time_derivative->maths->validateMathInLine(root->al.data(), &errs);

    // sort out errors
    QSettings settings;
    int num_errs = settings.beginReadArray("warnings");
    settings.endArray();

    if (num_errs != 0 && source) {

        // show errors by changing lineedit colour
        QPalette p = source->palette();
        p.setColor( QPalette::Normal, QPalette::Base, QColor(255, 200, 200) );
        source->setPalette(p);

        // clear errors
        settings.remove("warnings");

    }
    if (num_errs == 0 && source) {

        // show no errors by changing lineedit colour
        QPalette p = source->palette();
        p.setColor( QPalette::Normal, QPalette::Base, QColor(255, 255, 255) );
        source->setPalette(p);

        // clear errors
        settings.remove("errors");
    }

    updateContent();
    root->notifyDataChange();
    if (qobject_cast < QLineEdit *> (sender())) {
        root->alPtr->undoStack.push(new changeComponent(root, oldComponent, "Set TD maths"));
    } else {
        oldComponent.clear();
    }
}

void TimeDerivativeTextItem::updateContent()
{
    if (time_derivative->variable != NULL)
    {
        time_derivative->variable_name = time_derivative->variable->name;
        QString text = "d";
        text.append(time_derivative->variable->getName());
        text.append("/dt = ");
        text.append(time_derivative->maths->equation);
        setPlainText(text);
    }else
        setPlainText("Warning: Select a State Variable");
}


/* OnConditionGraphicsItem */
OnConditionGraphicsItem::OnConditionGraphicsItem(Regime *src_r, OnCondition *c, RootComponentItem *root)
    : NineMLTransitionItem(root->gvlayout, root->scene->getRegimeGVNode(src_r), root->scene->getRegimeGVNode(c->target_regime), root->scene)
{
    on_condition = c;
    src_regime = src_r;
    this->root = root;

    //get src regime
    //gv_edge = agedge(gv_graph, root->getRegimeGVNode(src_regime), root->getRegimeGVNode(c->target_regime));


    setRounded(false);
    setPadding(3);
    setColour(Qt::lightGray);
    setBorderColour(Qt::lightGray);

    //create name
    title->setColour(Qt::blue);
    title->setDefaultTextColor(Qt::white);
    title->setPlainText("Transition");
    updateGVData();

    trigger_item = new OnConditionTriggerTextItem(this, on_condition->trigger, root);
    addMember(trigger_item);

    //create state assigments equations
    for(int i=0; i<on_condition->StateAssignList.size(); i++)
    {
        addStateAssignment(on_condition->StateAssignList[i]);
    }

    //create event outputs
    for(int i=0; i<on_condition->eventOutList.size(); i++)
    {
       addEventOut(on_condition->eventOutList[i]);
    }

    //create impulse outputs
    for(int i=0; i<on_condition->impulseOutList.size(); i++)
    {
       addImpulseOut(on_condition->impulseOutList[i]);
    }
}

void OnConditionGraphicsItem::handleSelection()
{
    root->properties->createOnConditionProperties(this);
}

MathInLine * OnConditionGraphicsItem::getTriggerMaths()
{
    return trigger_item->getMaths();
}

void OnConditionGraphicsItem::setTriggerMaths(QString m)
{
    QSharedPointer<Component> oldComponent = QSharedPointer<Component> (new Component(root->al));
    // if the sender is a QLineEdit
    QLineEdit * source = qobject_cast < QLineEdit *> (sender());

    trigger_item->setMaths(m);

    // sort out errors
    QSettings settings;
    int num_errs = settings.beginReadArray("warnings");
    settings.endArray();

    if (num_errs != 0 && source) {

        // show errors by changing lineedit colour
        QPalette p = source->palette();
        p.setColor( QPalette::Normal, QPalette::Base, QColor(255, 200, 200) );
        source->setPalette(p);

        // clear errors
        settings.remove("warnings");

    }
    if (num_errs == 0 && source) {

        // show no errors by changing lineedit colour
        QPalette p = source->palette();
        p.setColor( QPalette::Normal, QPalette::Base, QColor(255, 255, 255) );
        source->setPalette(p);

        // clear errors
        settings.remove("errors");
    }
    root->notifyDataChange();
    if (qobject_cast < QLineEdit *> (sender())) {
        root->alPtr->undoStack.push(new changeComponent(root, oldComponent, "Set OC trigger"));
    } else {
        oldComponent.clear();
    }
}

Regime * OnConditionGraphicsItem::getSynapseRegime()
{
    return on_condition->target_regime;
}

Regime * OnConditionGraphicsItem::getSourceRegime()
{
    return src_regime;
}

void OnConditionGraphicsItem::setSynapseRegime(QString r)
{
    //QSharedPointer<NineMLComponent> oldComponent = QSharedPointer<NineMLComponent>(new NineMLComponent(root->al));
    on_condition->target_regime_name = r;
    for (int i=0;i<root->al->RegimeList.size();i++) {
        if (root->al->RegimeList[i]->name.compare(r)==0)
            on_condition->target_regime = root->al->RegimeList[i];
    }
    //Dont need to redraw!!
    //if (qobject_cast < QComboBox *> (sender()))
    //    root->alPtr->undoStack.push(new changeComponent(root, oldComponent));
    //else
    //    oldComponent.clear();
}

void OnConditionGraphicsItem::addStateAssignment(StateAssignment *sa)
{
    StateAssignmentTextItem *s = new StateAssignmentTextItem(this, sa, root);
    int index;
    for(index=1; index< members.size();index++){    //INDEX STARTS AT 1 (NOT 0) DUE TO TRIGGER ITEM
        int type = members[index]->type();
        if (type != StateAssignmentTextItem::Type)
            break;
    }
    addMemberAtIndex(s, index);
}

void OnConditionGraphicsItem::addEventOut(EventOut *eo)
{
    EventOutTextItem *e = new EventOutTextItem(this, eo, root);
    int index;
    for(index=1; index< members.size();index++){    //INDEX STARTS AT 1 (NOT 0) DUE TO TRIGGER ITEM
        int type = members[index]->type();
        if (type == ImpulseOutTextItem::Type)
            break;
    }
    addMemberAtIndex(e, index);
}

void OnConditionGraphicsItem::addImpulseOut(ImpulseOut *io)
{
    ImpulseOutTextItem *e = new ImpulseOutTextItem(this, io, root);
    addMember(e);
}


/* OnConditionTriggerTextItem */
OnConditionTriggerTextItem::OnConditionTriggerTextItem(OnConditionGraphicsItem *parent, Trigger *t, RootComponentItem *r)
    : NineMLTextItem(parent, parent)
{
    trigger = t;
    root = r;
    setFlag(QGraphicsItem::ItemIsSelectable, false);
    setColour(Qt::green);
    updateContent();
}

MathInLine * OnConditionTriggerTextItem::getMaths()
{
    return trigger->maths;
}

void OnConditionTriggerTextItem::setMaths(QString m)
{
    trigger->maths->equation = m;
    QStringList errs;
    trigger->maths->validateMathInLine(root->al.data(), &errs);
    updateContent();
    root->notifyDataChange();
}

void OnConditionTriggerTextItem::updateContent()
{
    if (trigger != NULL)
    {
        QString text = "@ OnCondition( ";
        text.append(trigger->maths->equation);
        text.append(" )");
        setPlainText(text);
    }else
        setPlainText("Warning: Select an OnCondition");
}

void OnConditionTriggerTextItem::handleSelection()
{
    //yeahhhhhhh
}


/* StateAssignmentTextItem */
StateAssignmentTextItem::StateAssignmentTextItem(NineMLTransitionItem *parent, StateAssignment* a,  RootComponentItem *r)
    : NineMLTextItem(parent, parent)
{
    assignment = a;
    root = r;
    setColour(Qt::lightGray);
    updateContent();
    if (assignment->variable != NULL)
        connect(assignment->variable, SIGNAL(nameChanged()), this, SLOT(updateContent()));
}

void StateAssignmentTextItem::handleSelection()
{
    root->properties->createStateAssignmentProperties(this);
}

StateVariable * StateAssignmentTextItem::getVariable()
{
    return assignment->variable;
}

MathInLine * StateAssignmentTextItem::getMaths()
{
    return assignment->maths;
}

void StateAssignmentTextItem::setVariable(QString p)
{
    QSharedPointer<Component> oldComponent = QSharedPointer<Component> (new Component(root->al));
    assignment->name = p;
    if (assignment->variable != NULL)
        disconnect(assignment->variable, SIGNAL(nameChanged()), this, SLOT(updateContent()));
    for (int i=0;i<root->al->StateVariableList.size();i++)
    {
        if (root->al->StateVariableList[i]->getName().compare(p)==0)
        {
           assignment->variable = root->al->StateVariableList[i];
           connect(assignment->variable, SIGNAL(nameChanged()), this, SLOT(updateContent()));
        }
    }
    updateContent();
    root->notifyDataChange();
    if (qobject_cast < QComboBox *> (sender())) {
        root->alPtr->undoStack.push(new changeComponent(root, oldComponent, "Set SA variable"));
    } else {
        oldComponent.clear();
    }
}

void StateAssignmentTextItem::setMaths(QString m)
{
    QSharedPointer<Component> oldComponent = QSharedPointer<Component> (new Component(root->al));
    // if the sender is a QLineEdit
    QLineEdit * source = qobject_cast < QLineEdit *> (sender());

    assignment->maths->equation = m;
    QStringList errs;
    assignment->maths->validateMathInLine(root->al.data(), &errs);

    // sort out errors
    QSettings settings;
    int num_errs = settings.beginReadArray("warnings");
    settings.endArray();

    if (num_errs != 0 && source) {

        // show errors by changing lineedit colour
        QPalette p = source->palette();
        p.setColor( QPalette::Normal, QPalette::Base, QColor(255, 200, 200) );
        source->setPalette(p);

        // clear errors
        settings.remove("warnings");

    }
    if (num_errs == 0 && source) {

        // show no errors by changing lineedit colour
        QPalette p = source->palette();
        p.setColor( QPalette::Normal, QPalette::Base, QColor(255, 255, 255) );
        source->setPalette(p);

        // clear errors
        settings.remove("errors");
    }

    updateContent();
    root->notifyDataChange();
    if (qobject_cast < QLineEdit *> (sender()))
        root->alPtr->undoStack.push(new changeComponent(root, oldComponent, "Set SA maths"));
    else
        oldComponent.clear();
}

void StateAssignmentTextItem::updateContent()
{
    if (assignment->variable != NULL)
    {
        assignment->name = assignment->variable->name;
        QString text = "Assign: ";
        text.append(assignment->variable->getName());
        text.append(" := ");
        text.append(assignment->maths->equation);
        setPlainText(text);
    }else
        setPlainText("Warning: Select a Variable");
}


/* EventOutTextItem */
EventOutTextItem::EventOutTextItem(NineMLTransitionItem *parent, EventOut *e, RootComponentItem *r)
    : NineMLTextItem(parent, parent)
{
    event_out = e;
    root = r;
    setColour(Qt::lightGray);
    updateContent();
    if (event_out->port)
        connect(event_out->port, SIGNAL(nameChanged()), this, SLOT(updateContent()));
}

EventPort * EventOutTextItem::getEventPort()
{
    return event_out->port;
}

void EventOutTextItem::setEventPort(QString m)
{
    QSharedPointer<Component> oldComponent = QSharedPointer<Component> (new Component(root->al));
    event_out->port_name = m;
    if (event_out->port != NULL)
        disconnect(event_out->port, SIGNAL(nameChanged()), this, SLOT(updateContent()));
    for (int i=0;i<root->al->EventPortList.size();i++)
    {
        if (root->al->EventPortList[i]->getName().compare(m)==0)
        {
            event_out->port = root->al->EventPortList[i];
            connect(event_out->port, SIGNAL(nameChanged()), this, SLOT(updateContent()));
        }
    }
    updateContent();
    root->notifyDataChange();
    if (qobject_cast < QComboBox *> (sender()))
        root->alPtr->undoStack.push(new changeComponent(root, oldComponent, "Set EventOut port"));
    else
        oldComponent.clear();
}

void EventOutTextItem::updateContent()
{
    if (event_out->port != NULL)
    {
        QString text = "Emit Event: ";
        text.append(event_out->port->getName());
        setPlainText(text);
    }else
        setPlainText("Warning: Select an Event Port");
}

void EventOutTextItem::handleSelection()
{
    root->properties->createEventOutProperties(this);
}


/* ImpulseOutTextItem */
ImpulseOutTextItem::ImpulseOutTextItem(NineMLTransitionItem *parent, ImpulseOut *i, RootComponentItem *r)
    : NineMLTextItem(parent, parent)
{
    impulse_out = i;
    root = r;
    setColour(Qt::lightGray);
    updateContent();
    if (impulse_out->port){
        connect(impulse_out->port, SIGNAL(nameChanged()), this, SLOT(updateContent()));
        if (impulse_out->port->parameter != NULL)
            connect(impulse_out->port->parameter, SIGNAL(nameChanged()), this, SLOT(updateContent()));
    }
}

ImpulsePort * ImpulseOutTextItem::getImpulsePort()
{
    return impulse_out->port;
}

void ImpulseOutTextItem::setImpulsePort(QString m)
{
    QSharedPointer<Component> oldComponent = QSharedPointer<Component> (new Component(root->al));
    impulse_out->port_name = m;
    if (impulse_out->port != NULL){
        disconnect(impulse_out->port, SIGNAL(nameChanged()), this, SLOT(updateContent()));
        if (impulse_out->port->parameter !=NULL){
            disconnect(impulse_out->port->parameter, SIGNAL(nameChanged()), this, SLOT(updateContent()));
        }

    }
    for (int i=0;i<root->al->ImpulsePortList.size();i++)
    {
        if (root->al->ImpulsePortList[i]->parameter != NULL){
            if (root->al->ImpulsePortList[i]->parameter->getName().compare(m)==0)
            {
                impulse_out->port = root->al->ImpulsePortList[i];
                connect(impulse_out->port->parameter, SIGNAL(nameChanged()), this, SLOT(updateContent()));
                connect(impulse_out->port, SIGNAL(nameChanged()), this, SLOT(updateContent()));
            }
        }
    }
    updateContent();
    root->notifyDataChange();
    root->alPtr->undoStack.push(new changeComponent(root, oldComponent, "Set ImpulseOut port"));
}

void ImpulseOutTextItem::updateContent()
{
    if (impulse_out->port != NULL)
    {
        QString text = "Emit Impulse: ";
        if (impulse_out->port->parameter != NULL){
            impulse_out->port_name = impulse_out->port->parameter->name;
            text.append(impulse_out->port->parameter->getName());
            setPlainText(text);
        }else
            setPlainText("Warning: Configure selected Impulse Port");
    }else
        setPlainText("Warning: Select an Impulse Port");
}

void ImpulseOutTextItem::handleSelection()
{
    root->properties->createImpulseOutProperties(this);
}


/* ParameterListGraphicsItem */
ParameterListGraphicsItem::ParameterListGraphicsItem(RootComponentItem *r)
    :NineMLNodeItem(r->gvlayout, "Parameters")
{
    root = r;

    setRounded(false);
    setPadding(5);
    setColour(Qt::white);
    setBorderColour(Qt::black);

    //create name
    title->setColour(Qt::red);
    title->setDefaultTextColor(Qt::white);
    title->setPlainText("Params, Vars & Alias");

    updateGVData();

    //create parameter items
    for (int i=0;i< r->al->ParameterList.size(); i++)
    {
        addParameterItem(r->al->ParameterList[i]);
    }

    //create state variable items
    for (int i=0;i< r->al->StateVariableList.size(); i++)
    {
        addStateVariableItem(r->al->StateVariableList[i]);
    }

    //create alias items
    for (int i=0;i< r->al->AliasList.size(); i++)
    {
        addAliasItem(r->al->AliasList[i]);
    }
}

void ParameterListGraphicsItem::addParameterItem(Parameter *p)
{
    ParameterTextItem *pt = new ParameterTextItem(this, p, root);
    //index is first member whihc is not a paramenter
    int index = 0;
    for(index=0; index< members.size();index++){
        int type = members[index]->type();
        if (type != ParameterTextItem::Type)
            break;
    }
    addMemberAtIndex(pt, index);
}

void ParameterListGraphicsItem::addStateVariableItem(StateVariable *sv)
{
    StateVariableTextItem *svti = new StateVariableTextItem(this, sv, root);
    //index is before first occourance of alias
    int index = 0;
    for(index=0; index< members.size();index++){
        int type = members[index]->type();
        if (type == AliasTextItem::Type)
            break;
    }
    addMemberAtIndex(svti, index);
}

void ParameterListGraphicsItem::addAliasItem(Alias *a)
{
    AliasTextItem *ati = new AliasTextItem(this, a, root);
    addMember(ati);
}

void ParameterListGraphicsItem::handleSelection()
{
    root->properties->createParameterListProperties();
}


/* ParameterTextItem */
ParameterTextItem::ParameterTextItem(ParameterListGraphicsItem *parent, Parameter *param, RootComponentItem *r)
    : NineMLTextItem(parent, parent)
{
    parameter = param;
    root = r;
    setColour(Qt::white);
    setDefaultTextColor(Qt::darkRed);
    updateContent();
}

void ParameterTextItem::updateContent()
{
    if (parameter != NULL)
    {
        QString text = "Parameter: ";
        text.append(parameter->getName());
        QString unit = parameter->dims->toString();
        if (unit != "?"){
            text.append(" ["+unit+"]");
        }
        setPlainText(text);
    }else
        setPlainText("Warning: Select a Parameter");
}

QString ParameterTextItem::getName()
{
    return parameter->getName();
}

void ParameterTextItem::setName(QString n)
{
    QSharedPointer<Component> oldComponent = QSharedPointer<Component> (new Component(root->al));
    parameter->setName(n);
    updateContent();
    if (qobject_cast < QLineEdit *> (sender()))
        root->alPtr->undoStack.push(new changeComponent(root, oldComponent, "Set Parameter name"));
    else
        oldComponent.clear();
}

void ParameterTextItem::setDimsPrefix(QString p)
{
    QSharedPointer<Component> oldComponent = QSharedPointer<Component> (new Component(root->al));
    QStringList list = p.split(" ");
    if (list.size() == 0) {
        return;
    }
    parameter->dims->setPrefix(list[0]);
    updateContent();
    if (qobject_cast < QComboBox *> (sender()))
        root->alPtr->undoStack.push(new changeComponent(root, oldComponent, "Set Par dims prefix"));
    else
        oldComponent.clear();
}

void ParameterTextItem::setDimsUnit(QString u)
{
    QSharedPointer<Component> oldComponent = QSharedPointer<Component> (new Component(root->al));
    QStringList list = u.split(" ");
    if (list.size() == 0) {
        return;
    }
    parameter->dims->setUnit(list[0]);
    updateContent();
    if (qobject_cast < QComboBox *> (sender()))
        root->alPtr->undoStack.push(new changeComponent(root, oldComponent, "Set Par dims suffix"));
    else
        oldComponent.clear();
}

void ParameterTextItem::handleSelection()
{
    root->properties->createParameterProperties(this);
}


/* StateVariableTextItem */
StateVariableTextItem::StateVariableTextItem(ParameterListGraphicsItem *parent, StateVariable *state_var, RootComponentItem *r)
    : NineMLTextItem(parent, parent)
{
    state_variable = state_var;
    setColour(Qt::white);
    setDefaultTextColor(Qt::darkGray);
    root = r;
    updateContent();
}

void StateVariableTextItem::updateContent()
{
    if (state_variable != NULL)
    {
        QString text = "State Variable: ";
        text.append(state_variable->getName());
        QString unit = state_variable->dims->toString();
        if (unit != "?"){
            text.append(" ["+unit+"]");
        }
        setPlainText(text);
    }else
        setPlainText("Warning: Select a State Variable");
}

QString StateVariableTextItem::getName()
{
    return state_variable->getName();
}

void StateVariableTextItem::setName(QString n)
{
    QSharedPointer<Component> oldComponent = QSharedPointer<Component> (new Component(root->al));
    state_variable->setName(n);
    updateContent();
    root->notifyDataChange();
    if (qobject_cast < QLineEdit *> (sender()))
        root->alPtr->undoStack.push(new changeComponent(root, oldComponent, "Set SV name"));
    else
         oldComponent.clear();
}

void StateVariableTextItem::setDimsPrefix(QString p)
{
    QSharedPointer<Component> oldComponent = QSharedPointer<Component> (new Component(root->al));
    QStringList list = p.split(" ");
    if (list.size() == 0) {
        return;
    }
    state_variable->dims->setPrefix(list[0]);
    updateContent();
    if (qobject_cast < QComboBox *> (sender()))
        root->alPtr->undoStack.push(new changeComponent(root, oldComponent, "Set SV dims prefix"));
    else
         oldComponent.clear();
}

void StateVariableTextItem::setDimsUnit(QString u)
{
    QSharedPointer<Component> oldComponent = QSharedPointer<Component> (new Component(root->al));
    QStringList list = u.split(" ");
    if (list.size() == 0) {
        return;
    }
    state_variable->dims->setUnit(list[0]);
    updateContent();
    if (qobject_cast < QComboBox *> (sender()))
        root->alPtr->undoStack.push(new changeComponent(root, oldComponent, "Set SV dims suffix"));
    else
         oldComponent.clear();
}

void StateVariableTextItem::handleSelection()
{
    root->properties->createStateVariableProperties(this);
}

/************************************************************/


AliasTextItem::AliasTextItem(ParameterListGraphicsItem *parent, Alias *a, RootComponentItem *r)
    : NineMLTextItem(parent, parent)
{
    alias = a;
    root = r;
    setColour(Qt::white);
    setDefaultTextColor(Qt::darkMagenta);
    updateContent();
}


void AliasTextItem::updateContent()
{
    if (alias != NULL)
    {
        QString text = "Alias: ";
        text.append(alias->getName());
        text.append(" = ");
        text.append(alias->maths->equation);
        setPlainText(text);
    }else
        setPlainText("Warning: Select a State Variable");
}

QString AliasTextItem::getName()
{
    return alias->getName();
}

void AliasTextItem::setName(QString n)
{
    QSharedPointer<Component> oldComponent = QSharedPointer<Component> (new Component(root->al));
    alias->setName(n);
    updateContent();
    root->notifyDataChange();
    if (qobject_cast < QLineEdit *> (sender()))
        root->alPtr->undoStack.push(new changeComponent(root, oldComponent, "Set Alias name"));
    else
         oldComponent.clear();
}

MathInLine * AliasTextItem::getMaths()
{
    return alias->maths;
}

void AliasTextItem::setMaths(QString m)
{
    QSharedPointer<Component> oldComponent = QSharedPointer<Component>(new Component(root->al));
    // if the sender is a QLineEdit
    QLineEdit * source = qobject_cast < QLineEdit *> (sender());

    alias->maths->equation = m;
    QStringList errs;
    alias->maths->validateMathInLine(root->al.data(), &errs);

    // sort out errors
    QSettings settings;
    int num_errs = settings.beginReadArray("warnings");
    settings.endArray();

    if (num_errs != 0 && source) {

        // show errors by changing lineedit colour
        QPalette p = source->palette();
        p.setColor( QPalette::Normal, QPalette::Base, QColor(255, 200, 200) );
        source->setPalette(p);

        // clear errors
        settings.remove("warnings");

    }
    if (num_errs == 0 && source) {

        // show no errors by changing lineedit colour
        QPalette p = source->palette();
        p.setColor( QPalette::Normal, QPalette::Base, QColor(255, 255, 255) );
        source->setPalette(p);

        // clear errors
        settings.remove("errors");
    }


    updateContent();
    root->notifyDataChange();
    if (qobject_cast < QLineEdit *> (sender()))
        root->alPtr->undoStack.push(new changeComponent(root, oldComponent, "Set Alias maths"));
    else
        oldComponent.clear();
}

void AliasTextItem::handleSelection()
{
    root->properties->createAliasProperties(this);
}


/************************************************************/

PortListGraphicsItem::PortListGraphicsItem(RootComponentItem *r)
    :NineMLNodeItem(r->gvlayout, "Ports")
{
    root = r;

    setRounded(false);
    setPadding(5);
    setColour(Qt::white);
    setBorderColour(Qt::black);

    //create name
    title->setColour(Qt::blue);
    title->setDefaultTextColor(Qt::white);
    title->setPlainText("Ports");

    updateGVData();

    //create analog port items
    for (int i=0;i< r->al->AnalogPortList.size(); i++)
    {
        addAnalogePortItem(r->al->AnalogPortList[i]);
    }

    //create event port items
    for (int i=0;i< r->al->EventPortList.size(); i++)
    {
        addEventPortItem(r->al->EventPortList[i]);
    }

    //create event port items
    for (int i=0;i< r->al->ImpulsePortList.size(); i++)
    {
        addImpulsePortItem(r->al->ImpulsePortList[i]);
    }

}

void PortListGraphicsItem::addAnalogePortItem(AnalogPort *ap)
{
    AnalogPortTextItem *api = new AnalogPortTextItem(this, ap, root);
    //add at index after last analog port item
    int index = 0;
    for(index=0; index< members.size();index++){
        int type = members[index]->type();
        if (type != AnalogPortTextItem::Type)
            break;
    }
    addMemberAtIndex(api, index);
}


void PortListGraphicsItem::addEventPortItem(EventPort *ep)
{
    EventPortTextItem *epi = new EventPortTextItem(this, ep, root);
    int index = 0;
    for(index=0; index< members.size();index++){
        int type = members[index]->type();
        if (type == ImpulsePortTextItem::Type)
            break;
    }
    addMemberAtIndex(epi, index);
}

void PortListGraphicsItem::addImpulsePortItem(ImpulsePort *ip)
{
    ImpulsePortTextItem *ipi = new ImpulsePortTextItem(this, ip, root);
    addMember(ipi);
}





void PortListGraphicsItem::handleSelection()
{
    root->properties->createPortListProperties();
}

/************************************************************/


AnalogPortTextItem::AnalogPortTextItem(PortListGraphicsItem *parent, AnalogPort *p, RootComponentItem *r)
    : NineMLTextItem(parent, parent)
{
    port = p;
    root = r;
    updateContent();

}


void AnalogPortTextItem::updateContent()
{
    QString text = "Analog ";
    switch (port->mode){
        case(AnalogSendPort):
        {
            if (port->variable != NULL)
            {
                port->name = port->variable->name;
                text.append("Send: ");
                text.append(port->variable->getName());
            }
            else
            {
                text = "Warning: Select a Port Variable";
            }
            break;
        }
        case(AnalogRecvPort):
        {
            text.append("Rcv: " + port->getName());
            QString unit = port->dims->toString();
            if (unit != "?")
                text.append(" ["+unit+"]");
            break;
        }
        case(AnalogReducePort):
        {
            text.append("Reduce (");
            if (port->op == ReduceOperationAddition)
                text.append("+");
            else
                text.append("None");
            text.append("): ");
            text.append(port->getName());
            QString unit = port->dims->toString();
            if (unit != "?")
                text.append(" ["+unit+"]");
            break;
        }
    }
    setPlainText(text);
}


void AnalogPortTextItem::handleSelection()
{
    root->properties->createAnalogPortProperties(this);
}

QString AnalogPortTextItem::getName()
{
    return port->getName();
}

void AnalogPortTextItem::setName(QString n)
{
    QSharedPointer<Component> oldComponent = QSharedPointer<Component>(new Component(root->al));
    port->setName(n);
    updateContent();
    root->notifyDataChange();
    if (port->variable != NULL)
        disconnect(port->variable, SIGNAL(nameChanged()), this, SLOT(updateContent()));
    port->variable = NULL;
    if (qobject_cast < QLineEdit *> (sender()))
        root->alPtr->undoStack.push(new changeComponent(root, oldComponent, "Set Analog Port name"));
    else
        oldComponent.clear();
}

AnalogPortMode AnalogPortTextItem::getPortMode()
{
    return port->mode;
}

ReduceOperation AnalogPortTextItem::getPortReduceOp()
{
    return port->op;
}

StateVariable * AnalogPortTextItem::getVariable()
{
    return port->variable;
}

void AnalogPortTextItem::setVariable(QString v)
{
    QSharedPointer<Component> oldComponent = QSharedPointer<Component>(new Component(root->al));
    port->name = v;
    //could be either an analog port
    if (port->variable != NULL)
        disconnect(port->variable, SIGNAL(nameChanged()), this, SLOT(updateContent()));
    for (int i=0;i<root->al->StateVariableList.size();i++)
    {
        if (root->al->StateVariableList[i]->getName().compare(v)==0)
        {
            port->variable = root->al->StateVariableList[i];
            connect(port->variable, SIGNAL(nameChanged()), this, SLOT(updateContent()));
        }
    }
    for (int i=0;i<root->al->AliasList.size();i++)
    {
        if (root->al->AliasList[i]->getName().compare(v)==0)
        {
            port->variable = root->al->AliasList[i];
            connect(port->variable, SIGNAL(nameChanged()), this, SLOT(updateContent()));
        }
    }
    updateContent();
    root->notifyDataChange();
    if (qobject_cast < QComboBox *> (sender()))
        root->alPtr->undoStack.push(new changeComponent(root, oldComponent, "Set Analog Port variable"));
    else
        oldComponent.clear();

}

void AnalogPortTextItem::setPortMode(QString p)
{
    QSharedPointer<Component> oldComponent = QSharedPointer<Component>(new Component(root->al));
    if (p.compare("Send") == 0){
        if (port->variable != NULL)
            disconnect(port->variable, SIGNAL(nameChanged()), this, SLOT(updateContent()));
        port->mode = AnalogSendPort;
        port->setName("");
        port->dims->reset();
    }
    else if (p.compare("Receive") == 0)
    {
        if (port->mode == AnalogSendPort){  //only reset name and dims if changing from a send
            port->setName("");
            if (port->variable != NULL)
                disconnect(port->variable, SIGNAL(nameChanged()), this, SLOT(updateContent()));
            port->variable = NULL;
            port->dims->reset();
        }
        port->mode = AnalogRecvPort;
    }
    else if (p.compare("Reduce") == 0)
    {
        if (port->mode == AnalogSendPort){  //only reset name and dims if changing from a send
            port->setName("");
            if (port->variable != NULL)
                disconnect(port->variable, SIGNAL(nameChanged()), this, SLOT(updateContent()));
            port->variable = NULL;
            port->dims->reset();
        }
        port->mode = AnalogReducePort;
    }
    updateContent();
    //clear and reset properties
    root->properties->clear();
    root->properties->createAnalogPortProperties(this);
    root->notifyDataChange();
    if (qobject_cast < QComboBox *> (sender()))
        root->alPtr->undoStack.push(new changeComponent(root, oldComponent, "Set Analog Port mode"));
    else
        oldComponent.clear();

}

void AnalogPortTextItem::setPortReduceOp(QString p)
{
    QSharedPointer<Component> oldComponent = QSharedPointer<Component>(new Component(root->al));
    if (p.compare("None") == 0)
        port->op = ReduceOperationNone;
    else if (p.compare("Addition") == 0)
        port->op = ReduceOperationAddition;
    updateContent();
    root->notifyDataChange();
    if (qobject_cast < QComboBox *> (sender()))
        root->alPtr->undoStack.push(new changeComponent(root, oldComponent, "Set Analog Port reduce op"));
    else
        oldComponent.clear();
}

void AnalogPortTextItem::setDimsPrefix(QString p)
{
    QSharedPointer<Component> oldComponent = QSharedPointer<Component>(new Component(root->al));
    if ((port->mode == AnalogRecvPort)||(port->mode == AnalogReducePort)){
        QStringList list = p.split(" ");
        if (list.size() == 0) {
            return;
        }
        port->dims->setPrefix(list[0]);
        updateContent();
    }
    if (qobject_cast < QComboBox *> (sender()))
        root->alPtr->undoStack.push(new changeComponent(root, oldComponent, "Set AP dims prefix"));
    else
        oldComponent.clear();
}

void AnalogPortTextItem::setDimsUnit(QString u)
{
    QSharedPointer<Component> oldComponent = QSharedPointer<Component>(new Component(root->al));
    if ((port->mode == AnalogRecvPort)||(port->mode == AnalogReducePort)){
        QStringList list = u.split(" ");
        if (list.size() == 0) {
            return;
        }
        port->dims->setUnit(list[0]);
        updateContent();
    }
    if (qobject_cast < QComboBox *> (sender()))
        root->alPtr->undoStack.push(new changeComponent(root, oldComponent, "Set AP dims suffix"));
    else
        oldComponent.clear();
}

void AnalogPortTextItem::setIsPostState(bool b)
{
    this->port->isPost = b;
}

void AnalogPortTextItem::setIsPerConnState(bool b)
{
    this->port->isPerConn = b;
}

/************************************************************/


EventPortTextItem::EventPortTextItem(PortListGraphicsItem *parent, EventPort *p, RootComponentItem *r)
    : NineMLTextItem(parent, parent)
{
    port = p;
    root = r;
    updateContent();
}

void EventPortTextItem::updateContent()
{
    QString text = "Event ";
    switch (port->mode){
        case(EventSendPort):
        {
            text.append("Send: ");
            text.append(port->getName());
            break;
        }
        case(EventRecvPort):
        {
            text.append("Rcv: ");
            text.append(port->getName());
            break;
        }
    }
    setPlainText(text);
}


void EventPortTextItem::handleSelection()
{
    root->properties->createEventPortProperties(this);
}

QString EventPortTextItem::getName()
{
    return port->getName();
}

EventPortMode EventPortTextItem::getPortMode()
{
    return port->mode;
}

void EventPortTextItem::setName(QString n)
{
    QSharedPointer<Component> oldComponent = QSharedPointer<Component>(new Component(root->al));
    port->setName(n);
    updateContent();
    root->notifyDataChange();
    if (qobject_cast < QLineEdit *> (sender()))
        root->alPtr->undoStack.push(new changeComponent(root, oldComponent, "Set Event Port name"));
    else
        oldComponent.clear();
}

void EventPortTextItem::setPortMode(QString p)
{
    QSharedPointer<Component> oldComponent = QSharedPointer<Component>(new Component(root->al));
    if (p.compare("Send") == 0)
        port->mode = EventSendPort;
    else if (p.compare("Receive") == 0)
        port->mode = EventRecvPort;
    updateContent();
    root->notifyDataChange();
    if (qobject_cast < QComboBox *> (sender()))
        root->alPtr->undoStack.push(new changeComponent(root, oldComponent, "Set EventPort mode"));
    else
        oldComponent.clear();
}

void EventPortTextItem::setIsPostState(bool b)
{
    this->port->isPost = b;
}

/************************************************************/



ImpulsePortTextItem::ImpulsePortTextItem(PortListGraphicsItem *parent, ImpulsePort *p, RootComponentItem *r)
    : NineMLTextItem(parent, parent)
{
    port = p;
    root = r;
    updateContent();

    if (port->parameter)
        connect(port->parameter, SIGNAL(nameChanged()), this, SLOT(updateContent()));
}

void ImpulsePortTextItem::updateContent()
{
    QString text = "Impulse ";
    switch (port->mode){
        case(ImpulseSendPort):
        {
            if (port->parameter != NULL)
            {
                port->name = port->parameter->name;
                text.append("Send: "+port->parameter->getName());
            }
            else
            {
                text = "Warning: Select a Parameter";
            }
            break;
        }
        case(ImpulseRecvPort):
        {
            text.append("Rcv: "+port->getName());
            QString unit = port->dims->toString();
            if (unit != "?")
                text.append( "["+unit+"]");
            break;
        }
    }
    setPlainText(text);
}


void ImpulsePortTextItem::handleSelection()
{
    root->properties->createImpulsePortProperties(this);
}

QString ImpulsePortTextItem::getName()
{
    return port->getName();
}

void ImpulsePortTextItem::setName(QString n)
{
    QSharedPointer<Component> oldComponent = QSharedPointer<Component>(new Component(root->al));
    port->setName(n);
    port->parameter = NULL;
    updateContent();
    root->notifyDataChange();
    if (qobject_cast < QLineEdit *> (sender()))
        root->alPtr->undoStack.push(new changeComponent(root, oldComponent, "Set IP name"));
    else
        oldComponent.clear();
}

Parameter *ImpulsePortTextItem::getParameter()
{
    return port->parameter;
}

ImpulsePortMode ImpulsePortTextItem::getPortMode()
{
    return port->mode;
}


void ImpulsePortTextItem::setParameter(QString n)
{
    QSharedPointer<Component> oldComponent = QSharedPointer<Component>(new Component(root->al));
    //could be either an analog port
    if (port->parameter != NULL)
        disconnect(port->parameter, SIGNAL(nameChanged()), this, SLOT(updateContent()));
    for (int i=0;i<root->al->StateVariableList.size();i++)
    {
        if (root->al->StateVariableList[i]->getName().compare(n)==0)
        {
            port->parameter = root->al->StateVariableList[i];
            connect(port->parameter, SIGNAL(nameChanged()), this, SLOT(updateContent()));
        }
    }
    for (int i=0;i<root->al->ParameterList.size();i++)
    {
        if (root->al->ParameterList[i]->getName().compare(n)==0)
        {
            port->parameter = root->al->ParameterList[i];
            connect(port->parameter, SIGNAL(nameChanged()), this, SLOT(updateContent()));
        }
    }
    for (int i=0;i<root->al->AliasList.size();i++)
    {
        if (root->al->AliasList[i]->getName().compare(n)==0)
        {
            port->parameter = root->al->AliasList[i];
            connect(port->parameter, SIGNAL(nameChanged()), this, SLOT(updateContent()));
        }
    }
    port->name = port->parameter->name;
    updateContent();
    root->notifyDataChange();
    if (qobject_cast < QComboBox *> (sender()))
        root->alPtr->undoStack.push(new changeComponent(root, oldComponent, "Set IP Parameter"));
    else
        oldComponent.clear();

}

void ImpulsePortTextItem::setPortMode(QString p)
{
    QSharedPointer<Component> oldComponent = QSharedPointer<Component>(new Component(root->al));
    if (p.compare("Send") == 0){
        if (port->parameter != NULL)
            disconnect(port->parameter, SIGNAL(nameChanged()), this, SLOT(updateContent()));
        port->mode = ImpulseSendPort;
        port->setName("");
    }
    else if (p.compare("Receive") == 0){
        port->mode = ImpulseRecvPort;
        if (port->parameter != NULL)
            disconnect(port->parameter, SIGNAL(nameChanged()), this, SLOT(updateContent()));
        port->parameter = NULL;
        port->setName("");
    }
    port->dims->reset();
    updateContent();
    root->properties->clear();
    root->properties->createImpulsePortProperties(this);
    root->notifyDataChange();
    if (qobject_cast < QComboBox *> (sender()))
        root->alPtr->undoStack.push(new changeComponent(root, oldComponent, "Set IP mode"));
    else
        oldComponent.clear();
}

void ImpulsePortTextItem::setDimsPrefix(QString p)
{
    QSharedPointer<Component> oldComponent = QSharedPointer<Component>(new Component(root->al));
    QStringList list = p.split(" ");
    if (list.size() == 0) {
        return;
    }
    port->dims->setPrefix(list[0]);
    updateContent();
    if (qobject_cast < QComboBox *> (sender()))
        root->alPtr->undoStack.push(new changeComponent(root, oldComponent, "Set IP dims prefix"));
    else
        oldComponent.clear();
}

void ImpulsePortTextItem::setDimsUnit(QString u)
{
    QSharedPointer<Component> oldComponent = QSharedPointer<Component>(new Component(root->al));
    QStringList list = u.split(" ");
    if (list.size() == 0) {
        return;
    }
    port->dims->setUnit(list[0]);
    updateContent();
    if (qobject_cast < QComboBox *> (sender()))
        root->alPtr->undoStack.push(new changeComponent(root, oldComponent, "Set IP dims suffix"));
    else
        oldComponent.clear();
}

/************************************************************/

/************************************************************/
OnEventGraphicsItem::OnEventGraphicsItem(Regime *src_r, OnEvent *e, RootComponentItem *root)
    : NineMLTransitionItem(root->gvlayout, root->scene->getRegimeGVNode(src_r), root->scene->getRegimeGVNode(e->target_regime), root->scene)
{
    on_event = e;
    src_regime = src_r;
    this->root = root;

    setRounded(false);
    setPadding(3);
    setColour(Qt::lightGray);
    setBorderColour(Qt::lightGray);

    //create name
    title->setColour(Qt::blue);
    title->setDefaultTextColor(Qt::white);
    title->setPlainText("Transition");
    updateGVData();

    trigger_item = new OnEventTriggerTextItem(this, root);
    addMember(trigger_item);

    //create state assigments equations
    for(int i=0; i<on_event->StateAssignList.size(); i++)
    {
        addStateAssignment(on_event->StateAssignList[i]);
    }

    //create event outputs
    for(int i=0; i<on_event->eventOutList.size(); i++)
    {
       addEventOut(on_event->eventOutList[i]);
    }

    //create impulse outputs
    for(int i=0; i<on_event->impulseOutList.size(); i++)
    {
       addImpulseOut(on_event->impulseOutList[i]);
    }
}

void OnEventGraphicsItem::handleSelection()
{
    root->properties->createOnEventProperties(this);
}

EventPort * OnEventGraphicsItem::getEventPort()
{
    return trigger_item->on_event->src_port;
}

void OnEventGraphicsItem::setEventPort(QString m)
{
    QSharedPointer<Component> oldComponent = QSharedPointer<Component>(new Component(root->al));
    on_event->src_port_name = m;
    trigger_item->setEventPort(m);
    root->notifyDataChange();
    if (qobject_cast < QComboBox *> (sender()))
        root->alPtr->undoStack.push(new changeComponent(root, oldComponent, "Set OE Event Port"));
    else
        oldComponent.clear();
}

Regime * OnEventGraphicsItem::getSynapseRegime()
{
    return on_event->target_regime;
}

Regime * OnEventGraphicsItem::getSourceRegime()
{
    return src_regime;
}

void OnEventGraphicsItem::setSynapseRegime(QString r)
{
   // QSharedPointer<NineMLComponent> oldComponent = QSharedPointer<NineMLComponent>(new NineMLComponent(root->al));
    on_event->target_regime_name = r;
    for (int i=0;i<root->al->RegimeList.size();i++)
    {
        if (root->al->RegimeList[i]->name.compare(r)==0)
                on_event->target_regime = root->al->RegimeList[i];
    }
    //root->alPtr->undoStack.push(new changeComponent(root, oldComponent));
}

void OnEventGraphicsItem::addStateAssignment(StateAssignment *sa)
{
    StateAssignmentTextItem *s = new StateAssignmentTextItem(this, sa, root);
    int index;
    for(index=1; index< members.size();index++){    //INDEX STARTS AT 1 (NOT 0) DUE TO TRIGGER ITEM
        int type = members[index]->type();
        if (type != StateAssignmentTextItem::Type)
            break;
    }
    addMemberAtIndex(s, index);
}

void OnEventGraphicsItem::addEventOut(EventOut *eo)
{
    EventOutTextItem *e = new EventOutTextItem(this, eo, root);
    int index;
    for(index=1; index< members.size();index++){    //INDEX STARTS AT 1 (NOT 0) DUE TO TRIGGER ITEM
        int type = members[index]->type();
        if (type == ImpulseOutTextItem::Type)
            break;
    }
    addMemberAtIndex(e, index);
}

void OnEventGraphicsItem::addImpulseOut(ImpulseOut *io)
{
    ImpulseOutTextItem *e = new ImpulseOutTextItem(this, io, root);
    addMember(e);
}

/************************************************************/


OnEventTriggerTextItem::OnEventTriggerTextItem(OnEventGraphicsItem *parent, RootComponentItem *r)
    : NineMLTextItem(parent, parent)
{
    on_event = parent->on_event;
    root = r;
    setFlag(QGraphicsItem::ItemIsSelectable, false);
    setColour(Qt::green);
    updateContent();
    if (on_event->src_port)
        connect(on_event->src_port, SIGNAL(nameChanged()), this, SLOT(updateContent()));
}

EventPort *OnEventTriggerTextItem::getEventPort()
{
    return on_event->src_port;
}

void OnEventTriggerTextItem::setEventPort(QString m)
{
    on_event->src_port_name = m;
    if (on_event->src_port != NULL)
        disconnect(on_event->src_port, SIGNAL(nameChanged()), this, SLOT(updateContent()));
    for (int i=0;i<root->al->EventPortList.size();i++)
    {
        if (root->al->EventPortList[i]->getName().compare(m) == 0)
        {
            on_event->src_port = root->al->EventPortList[i];
            connect(root->al->EventPortList[i], SIGNAL(nameChanged()), this, SLOT(updateContent()));
            qDebug() << "Set Event Port" << on_event->src_port->getName();
        }
    }
    updateContent();
    root->notifyDataChange();
}

void OnEventTriggerTextItem::updateContent()
{
    if (on_event->src_port != NULL)
    {
        on_event->src_port_name = on_event->src_port->name;
        QString text = "@ OnEvent( ";
        text.append(on_event->src_port->getName());
        text.append(" )");
        setPlainText(text);
    }else
        setPlainText("Warning: Select an OnEvent source Port");
}

void OnEventTriggerTextItem::handleSelection()
{
    //yeahhhhhhh
}

/************************************************************/


/************************************************************/
OnImpulseGraphicsItem::OnImpulseGraphicsItem(Regime *src_r, OnImpulse *e, RootComponentItem *root)
    : NineMLTransitionItem(root->gvlayout, root->scene->getRegimeGVNode(src_r), root->scene->getRegimeGVNode(e->target_regime), root->scene)
{
    on_impulse = e;
    src_regime = src_r;
    this->root = root;

    setRounded(false);
    setPadding(3);
    setColour(Qt::lightGray);
    setBorderColour(Qt::lightGray);

    //create name
    title->setColour(Qt::blue);
    title->setDefaultTextColor(Qt::white);
    title->setPlainText("Transition");
    updateGVData();

    trigger_item = new OnImpulseTriggerTextItem(this, root);
    addMember(trigger_item);

    //create state assigments equations
    for(int i=0; i<on_impulse->StateAssignList.size(); i++)
    {
        addStateAssignment(on_impulse->StateAssignList[i]);
    }

    //create event outputs
    for(int i=0; i<on_impulse->eventOutList.size(); i++)
    {
       addEventOut(on_impulse->eventOutList[i]);
    }

    //create impulse outputs
    for(int i=0; i<on_impulse->impulseOutList.size(); i++)
    {
       addImpulseOut(on_impulse->impulseOutList[i]);
    }
}

void OnImpulseGraphicsItem::handleSelection()
{
    root->properties->createOnImpulseProperties(this);
}

ImpulsePort * OnImpulseGraphicsItem::getImpulsePort()
{
    return trigger_item->on_impulse->src_port;
}

void OnImpulseGraphicsItem::setImpulsePort(QString m)
{
    QSharedPointer<Component> oldComponent = QSharedPointer<Component>(new Component(root->al));
    on_impulse->src_port_name = m;
    trigger_item->setImpulsePort(m);
    root->notifyDataChange();
    if (qobject_cast < QComboBox *> (sender()))
        root->alPtr->undoStack.push(new changeComponent(root, oldComponent, "Set OI Impulse Port"));
    else
        oldComponent.clear();
}

Regime * OnImpulseGraphicsItem::getSynapseRegime()
{
    return on_impulse->target_regime;
}

Regime * OnImpulseGraphicsItem::getSourceRegime()
{
    return src_regime;
}

void OnImpulseGraphicsItem::setSynapseRegime(QString r)
{
    on_impulse->target_regime_name = r;
    for (int i=0;i<root->al->RegimeList.size();i++)
    {
        if (root->al->RegimeList[i]->name.compare(r)==0)
                on_impulse->target_regime = root->al->RegimeList[i];
    }
}

void OnImpulseGraphicsItem::addStateAssignment(StateAssignment *sa)
{
    StateAssignmentTextItem *s = new StateAssignmentTextItem(this, sa, root);
    int index;
    for(index=1; index< members.size();index++){    //INDEX STARTS AT 1 (NOT 0) DUE TO TRIGGER ITEM
        int type = members[index]->type();
        if (type != StateAssignmentTextItem::Type)
            break;
    }
    addMemberAtIndex(s, index);
}

void OnImpulseGraphicsItem::addEventOut(EventOut *eo)
{
    EventOutTextItem *e = new EventOutTextItem(this, eo, root);
    int index;
    for(index=1; index< members.size();index++){    //INDEX STARTS AT 1 (NOT 0) DUE TO TRIGGER ITEM
        int type = members[index]->type();
        if (type == ImpulseOutTextItem::Type)
            break;
    }
    addMemberAtIndex(e, index);
}

void OnImpulseGraphicsItem::addImpulseOut(ImpulseOut *io)
{
    ImpulseOutTextItem *e = new ImpulseOutTextItem(this, io, root);
    addMember(e);
}

/************************************************************/


OnImpulseTriggerTextItem::OnImpulseTriggerTextItem(OnImpulseGraphicsItem *parent, RootComponentItem *r)
    : NineMLTextItem(parent, parent)
{
    on_impulse = parent->on_impulse;
    root = r;
    setFlag(QGraphicsItem::ItemIsSelectable, false);
    setColour(Qt::green);
    updateContent();
    if (on_impulse->src_port)
        connect(on_impulse->src_port, SIGNAL(nameChanged()), this, SLOT(updateContent()));
}

ImpulsePort *OnImpulseTriggerTextItem::getImpulsePort()
{
    return on_impulse->src_port;
}

void OnImpulseTriggerTextItem::setImpulsePort(QString m)
{
    on_impulse->src_port_name = m;
    if (on_impulse->src_port != NULL)
        disconnect(on_impulse->src_port, SIGNAL(nameChanged()), this, SLOT(updateContent()));
    for (int i=0;i<root->al->ImpulsePortList.size();i++)
    {
        if (root->al->ImpulsePortList[i]->getName().compare(m) == 0)
        {
            on_impulse->src_port = root->al->ImpulsePortList[i];
            connect(root->al->ImpulsePortList[i], SIGNAL(nameChanged()), this, SLOT(updateContent()));
            qDebug() << "Set Impulse Port" << on_impulse->src_port->getName();
        }
    }
    updateContent();
    root->notifyDataChange();
}

void OnImpulseTriggerTextItem::updateContent()
{
    if (on_impulse->src_port != NULL)
    {
        on_impulse->src_port_name = on_impulse->src_port->name;
        QString text = "@ OnImpulse( ";
        text.append(on_impulse->src_port->getName());
        text.append(" )");
        setPlainText(text);
    }else
        setPlainText("Warning: Select an OnImpulse source Port");
}

void OnImpulseTriggerTextItem::handleSelection()
{
    //yeahhhhhhh
}

/************************************************************/

