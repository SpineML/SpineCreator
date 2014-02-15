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

#include "nineml_alscene.h"
#include "nineml_graphicsitems.h"
#include <typeinfo>
#include <algorithm>
#include "undocommands.h"

NineMLALScene::NineMLALScene(RootComponentItem *r) :
    QGraphicsScene()
{
    mode = ModeSelect;
    root = r;
    transition = NULL;
}

NineMLALScene::~NineMLALScene()
{
    //lots of deletion to do
    foreach(RegimeGraphicsItem *rgi, rg_items)
    {
        removeRegimeItem(rgi); //this will delete an on conditions
    }

    if (pl_item->scene() != NULL){
        root->gvlayout->removeGVItem(pl_item);
        removeItem(pl_item);
    }
    delete pl_item;
    pl_item = NULL;

    if (portl_item->scene() != NULL){
        root->gvlayout->removeGVItem(portl_item);
        removeItem(portl_item);
    }
    delete portl_item;
    //portl_item->deleteLater();
    portl_item = NULL;
}

void NineMLALScene::initialiseScene(NineMLComponent *al)
{

    //create paramater list item
    pl_item = new ParameterListGraphicsItem(root);
    addItem(pl_item);

    //create analoge port list item
    portl_item = new PortListGraphicsItem(root);
    addItem(portl_item);

    //create all regime graphics items
    for (uint i=0; i<al->RegimeList.size();i++)
    {
        addRegimeItem(al->RegimeList[i]);
    }

    //create on condition/event items (must be done after all regime items are created)
    for (uint i=0; i<al->RegimeList.size();i++)
    {
        Regime* r = al->RegimeList[i];

        //create on conditions
        for(uint j=0; j<r->OnConditionList.size(); j++)
        {
            addOnConditionItem(al->RegimeList[i], r->OnConditionList[j]);
        }

        //create on events
        for(uint j=0; j<r->OnEventList.size(); j++)
        {
            addOnEventItem(al->RegimeList[i], r->OnEventList[j]);
        }
        //create on impulse
        for(uint j=0; j<r->OnImpulseList.size(); j++)
        {
            addOnImpulseItem(al->RegimeList[i], r->OnImpulseList[j]);
        }
    }

    QRectF sceneRect = this->itemsBoundingRect();
    sceneRect.setX(sceneRect.x()-5000);
    sceneRect.setY(sceneRect.y()-5000);
    sceneRect.setWidth(sceneRect.width()+5000);
    sceneRect.setHeight(sceneRect.height()+5000);
    //sceneRect.setRect(-5000.0,-5000.0, 10000.0,10000.0);
    this->setSceneRect(sceneRect);

}


Agnode_t * NineMLALScene::getRegimeGVNode(Regime *r)
{
    for (uint i=0; i<rg_items.size(); i++)
    {
        RegimeGraphicsItem *rg = rg_items[i];
        if (rg->isRegime(r))
            return rg->getGVNode();
    }
    qDebug() << "Regime not found in rg items whilst looking for gv node";
    return NULL;
}

void NineMLALScene::setParamsVisibility(bool visible)
{
    if (visible){
        root->gvlayout->removeGVItem(pl_item);
        removeItem(pl_item);
    }else{
        root->gvlayout->addGVItem(pl_item);
        addItem(pl_item);
    }
    root->requestLayoutUpdate();
}

void NineMLALScene::setPortsVisibility(bool visible)
{
    if (visible){
        root->gvlayout->removeGVItem(portl_item);
        removeItem(portl_item);
    }else{
        root->gvlayout->addGVItem(portl_item);
        addItem(portl_item);
    }
    root->requestLayoutUpdate();
}

void NineMLALScene::moveItemUp()
{
    QGraphicsItem *g = NULL;
    QList <QGraphicsItem*> selected = selectedItems();
    foreach(QGraphicsItem *i, selected)
    {
        if (i->flags() & QGraphicsItem::ItemIsSelectable)
        {
            g = i;
            break;
        }
    }
    if (g != NULL)
    {

        switch (g->type()){
            case (ParameterTextItem::Type):
            {
                ParameterTextItem *pi = (ParameterTextItem*)g;
                Parameter *p = pi->parameter;
                if(moveUp(&root->al->ParameterList, p))
                    pl_item->moveMemberUp(pi);
                root->properties->updateReorderingIcons(pi);
                break;
            }
            case (StateVariableTextItem::Type):
            {
                StateVariableTextItem *svgi = (StateVariableTextItem*)g;
                StateVariable *sv = svgi->state_variable;
                if(moveUp(&root->al->StateVariableList, sv))
                    pl_item->moveMemberUp(svgi);
                root->properties->updateReorderingIcons(svgi);
                break;
            }
            case (AliasTextItem::Type):
            {
                AliasTextItem *agi = (AliasTextItem*)g;
                Alias *al = agi->alias;
                if(moveUp(&root->al->AliasList, al))
                    pl_item->moveMemberUp(agi);
                root->properties->updateReorderingIcons(agi);
                break;
            }
            case (ImpulsePortTextItem::Type):
            {
                ImpulsePortTextItem *gi = (ImpulsePortTextItem*)g;
                ImpulsePort *ap = gi->port;
                if(moveUp(&root->al->ImpulsePortList, ap))
                    portl_item->moveMemberUp(gi);
                root->properties->updateReorderingIcons(gi);
                break;
            }
            case (AnalogPortTextItem::Type):
            {
                AnalogPortTextItem *gi = (AnalogPortTextItem*)g;
                AnalogPort *ap = gi->port;
                if(moveUp(&root->al->AnalogPortList, ap))
                    portl_item->moveMemberUp(gi);
                root->properties->updateReorderingIcons(gi);
                break;
            }

            case (EventPortTextItem::Type):
            {
                EventPortTextItem *gi = (EventPortTextItem*)g;
                EventPort *ep = gi->port;
                if(moveUp(&root->al->EventPortList, ep))
                    portl_item->moveMemberUp(gi);
                root->properties->updateReorderingIcons(gi);
                break;
            }

            case (TimeDerivativeTextItem::Type):
            {
                TimeDerivativeTextItem *gi = (TimeDerivativeTextItem*)g;
                TimeDerivative *td = gi->time_derivative;
                RegimeGraphicsItem *rgi =  (RegimeGraphicsItem*)gi->getTextItemGroup();
                if(moveUp(&rgi->regime->TimeDerivativeList, td))
                    gi->getTextItemGroup()->moveMemberUp(gi);
                root->properties->updateReorderingIcons(gi);
                break;
            }


            case (StateAssignmentTextItem::Type):
            {
                StateAssignmentTextItem *sati = (StateAssignmentTextItem*)g;
                StateAssignment *sa = sati->assignment;
                NineMLTransitionItem* ti = (NineMLTransitionItem*)sati->getTextItemGroup();
                switch(ti->transitionType()){
                    case(TRANSITION_TYPE_ON_CONDITION):{
                        OnConditionGraphicsItem *ocgi = (OnConditionGraphicsItem*)ti;
                        if(moveUp(&ocgi->on_condition->StateAssignList, sa))
                            sati->getTextItemGroup()->moveMemberUp(sati);
                        root->properties->updateReorderingIcons(sati);
                        break;
                    }
                    case(TRANSITION_TYPE_ON_EVENT):{
                        OnEventGraphicsItem *oegi = (OnEventGraphicsItem*)ti;
                        if(moveUp(&oegi->on_event->StateAssignList, sa))
                            sati->getTextItemGroup()->moveMemberUp(sati);
                        root->properties->updateReorderingIcons(sati);
                        break;
                    }
                    case(TRANSITION_TYPE_ON_IMPULSE):{
                        OnImpulseGraphicsItem *oigi = (OnImpulseGraphicsItem*)ti;
                        if(moveUp(&oigi->on_impulse->StateAssignList, sa))
                            sati->getTextItemGroup()->moveMemberUp(sati);
                        root->properties->updateReorderingIcons(sati);
                        break;
                    }
                }
                break;
            }
            case (EventOutTextItem::Type):
            {
                EventOutTextItem *eoti = (EventOutTextItem*)g;
                EventOut *eo = eoti->event_out;
                NineMLTransitionItem* ti = (NineMLTransitionItem*)eoti->getTextItemGroup();
                switch(ti->transitionType()){
                    case(TRANSITION_TYPE_ON_CONDITION):{
                        OnConditionGraphicsItem *ocgi = (OnConditionGraphicsItem*)ti;
                        if(moveUp(&ocgi->on_condition->eventOutList, eo))
                            eoti->getTextItemGroup()->moveMemberUp(eoti);
                        root->properties->updateReorderingIcons(eoti);
                        break;
                    }
                    case(TRANSITION_TYPE_ON_EVENT):{
                        OnEventGraphicsItem *oegi = (OnEventGraphicsItem*)ti;
                        if(moveUp(&oegi->on_event->eventOutList, eo))
                            eoti->getTextItemGroup()->moveMemberUp(eoti);
                        root->properties->updateReorderingIcons(eoti);
                        break;
                    }
                    case(TRANSITION_TYPE_ON_IMPULSE):{
                        OnImpulseGraphicsItem *oigi = (OnImpulseGraphicsItem*)ti;
                        if(moveUp(&oigi->on_impulse->eventOutList, eo))
                            eoti->getTextItemGroup()->moveMemberUp(eoti);
                        root->properties->updateReorderingIcons(eoti);
                        break;
                    }
                }
                break;
            }
            case (ImpulseOutTextItem::Type):
            {
                ImpulseOutTextItem *ioti = (ImpulseOutTextItem*)g;
                ImpulseOut *io = ioti->impulse_out;
                NineMLTransitionItem* ti = (NineMLTransitionItem*)ioti->getTextItemGroup();
                switch(ti->transitionType()){
                    case(TRANSITION_TYPE_ON_CONDITION):{
                        OnConditionGraphicsItem *ocgi = (OnConditionGraphicsItem*)ti;
                        if(moveUp(&ocgi->on_condition->impulseOutList, io))
                            ioti->getTextItemGroup()->moveMemberUp(ioti);
                        root->properties->updateReorderingIcons(ioti);
                        break;
                    }
                    case(TRANSITION_TYPE_ON_EVENT):{
                        OnEventGraphicsItem *oegi = (OnEventGraphicsItem*)ti;
                        if(moveUp(&oegi->on_event->impulseOutList, io))
                            ioti->getTextItemGroup()->moveMemberUp(ioti);
                        root->properties->updateReorderingIcons(ioti);
                        break;
                    }
                    case(TRANSITION_TYPE_ON_IMPULSE):{
                        OnImpulseGraphicsItem *oigi = (OnImpulseGraphicsItem*)ti;
                        if(moveUp(&oigi->on_impulse->impulseOutList, io))
                            ioti->getTextItemGroup()->moveMemberUp(ioti);
                        root->properties->updateReorderingIcons(ioti);
                        break;
                    }
                }
                break;
            }

            //others
            default:{
                //qDebug() << "Move up not processed!";
                break;
            }
        }
    }

}

void NineMLALScene::moveItemDown()
{
    QGraphicsItem *g = NULL;
    QList <QGraphicsItem*> selected = selectedItems();
    foreach(QGraphicsItem *i, selected)
    {
        if (i->flags() & QGraphicsItem::ItemIsSelectable)
        {
            g = i;
            break;
        }
    }
    if (g != NULL)
    {

        switch (g->type()){
            case (ParameterTextItem::Type):
            {
                ParameterTextItem *pi = (ParameterTextItem*)g;
                Parameter *p = pi->parameter;
                if(moveDown(&root->al->ParameterList, p))
                    pl_item->moveMemberDown(pi);
                root->properties->updateReorderingIcons(pi);
                break;
            }
            case (StateVariableTextItem::Type):
            {
                StateVariableTextItem *svgi = (StateVariableTextItem*)g;
                StateVariable *sv = svgi->state_variable;
                if(moveDown(&root->al->StateVariableList, sv))
                    pl_item->moveMemberDown(svgi);
                root->properties->updateReorderingIcons(svgi);
                break;
            }
            case (AliasTextItem::Type):
            {
                AliasTextItem *agi = (AliasTextItem*)g;
                Alias *al = agi->alias;
                if(moveDown(&root->al->AliasList, al))
                    pl_item->moveMemberDown(agi);
                root->properties->updateReorderingIcons(agi);
                break;
            }
            case (AnalogPortTextItem::Type):
            {
                AnalogPortTextItem *gi = (AnalogPortTextItem*)g;
                AnalogPort *ap = gi->port;
                if(moveDown(&root->al->AnalogPortList, ap))
                    portl_item->moveMemberDown(gi);
                root->properties->updateReorderingIcons(gi);
                break;
            }
            case (ImpulsePortTextItem::Type):
            {
                ImpulsePortTextItem *gi = (ImpulsePortTextItem*)g;
                ImpulsePort *ap = gi->port;
                if(moveDown(&root->al->ImpulsePortList, ap))
                    portl_item->moveMemberDown(gi);
                root->properties->updateReorderingIcons(gi);
                break;
            }
            case (EventPortTextItem::Type):
            {
                EventPortTextItem *gi = (EventPortTextItem*)g;
                EventPort *ep = gi->port;
                if(moveDown(&root->al->EventPortList, ep))
                    portl_item->moveMemberDown(gi);
                root->properties->updateReorderingIcons(gi);
                break;
            }

            case (TimeDerivativeTextItem::Type):
            {
                TimeDerivativeTextItem *gi = (TimeDerivativeTextItem*)g;
                TimeDerivative *td = gi->time_derivative;
                RegimeGraphicsItem *rgi =  (RegimeGraphicsItem*)gi->getTextItemGroup();
                if(moveDown(&rgi->regime->TimeDerivativeList, td))
                    gi->getTextItemGroup()->moveMemberDown(gi);
                root->properties->updateReorderingIcons(gi);
                break;
            }

            case (StateAssignmentTextItem::Type):
            {
                StateAssignmentTextItem *sati = (StateAssignmentTextItem*)g;
                StateAssignment *sa = sati->assignment;
                NineMLTransitionItem* ti = (NineMLTransitionItem*)sati->getTextItemGroup();
                switch(ti->transitionType()){
                    case(TRANSITION_TYPE_ON_CONDITION):{
                        OnConditionGraphicsItem *ocgi = (OnConditionGraphicsItem*)ti;
                        if(moveDown(&ocgi->on_condition->StateAssignList, sa))
                            sati->getTextItemGroup()->moveMemberDown(sati);
                        root->properties->updateReorderingIcons(sati);
                        break;
                    }
                    case(TRANSITION_TYPE_ON_EVENT):{
                        OnEventGraphicsItem *oegi = (OnEventGraphicsItem*)ti;
                        if(moveDown(&oegi->on_event->StateAssignList, sa))
                            sati->getTextItemGroup()->moveMemberDown(sati);
                        root->properties->updateReorderingIcons(sati);
                        break;
                    }
                    case(TRANSITION_TYPE_ON_IMPULSE):{
                        OnImpulseGraphicsItem *oigi = (OnImpulseGraphicsItem*)ti;
                        if(moveDown(&oigi->on_impulse->StateAssignList, sa))
                            sati->getTextItemGroup()->moveMemberDown(sati);
                        root->properties->updateReorderingIcons(sati);
                        break;
                    }
                }
                break;
            }
            case (EventOutTextItem::Type):
            {
                EventOutTextItem *eoti = (EventOutTextItem*)g;
                EventOut *eo = eoti->event_out;
                NineMLTransitionItem* ti = (NineMLTransitionItem*)eoti->getTextItemGroup();
                switch(ti->transitionType()){
                    case(TRANSITION_TYPE_ON_CONDITION):{
                        OnConditionGraphicsItem *ocgi = (OnConditionGraphicsItem*)ti;
                        if(moveDown(&ocgi->on_condition->eventOutList, eo))
                            eoti->getTextItemGroup()->moveMemberDown(eoti);
                        root->properties->updateReorderingIcons(eoti);
                        break;
                    }
                    case(TRANSITION_TYPE_ON_EVENT):{
                        OnEventGraphicsItem *oegi = (OnEventGraphicsItem*)ti;
                        if(moveDown(&oegi->on_event->eventOutList, eo))
                            eoti->getTextItemGroup()->moveMemberDown(eoti);
                        root->properties->updateReorderingIcons(eoti);
                        break;
                    }
                    case(TRANSITION_TYPE_ON_IMPULSE):{
                        OnImpulseGraphicsItem *oigi = (OnImpulseGraphicsItem*)ti;
                        if(moveDown(&oigi->on_impulse->eventOutList, eo))
                            eoti->getTextItemGroup()->moveMemberDown(eoti);
                        root->properties->updateReorderingIcons(eoti);
                        break;
                    }
                }
                break;
            }

            case (ImpulseOutTextItem::Type):
            {
                ImpulseOutTextItem *ioti = (ImpulseOutTextItem*)g;
                ImpulseOut *io = ioti->impulse_out;
                NineMLTransitionItem* ti = (NineMLTransitionItem*)ioti->getTextItemGroup();
                switch(ti->transitionType()){
                    case(TRANSITION_TYPE_ON_CONDITION):{
                        OnConditionGraphicsItem *ocgi = (OnConditionGraphicsItem*)ti;
                        if(moveDown(&ocgi->on_condition->impulseOutList, io))
                            ioti->getTextItemGroup()->moveMemberDown(ioti);
                        root->properties->updateReorderingIcons(ioti);
                        break;
                    }
                    case(TRANSITION_TYPE_ON_EVENT):{
                        OnEventGraphicsItem *oegi = (OnEventGraphicsItem*)ti;
                        if(moveDown(&oegi->on_event->impulseOutList, io))
                            ioti->getTextItemGroup()->moveMemberDown(ioti);
                        root->properties->updateReorderingIcons(ioti);
                        break;
                    }
                    case(TRANSITION_TYPE_ON_IMPULSE):{
                        OnImpulseGraphicsItem *oigi = (OnImpulseGraphicsItem*)ti;
                        if(moveDown(&oigi->on_impulse->impulseOutList, io))
                            ioti->getTextItemGroup()->moveMemberDown(ioti);
                        root->properties->updateReorderingIcons(ioti);
                        break;
                    }
                }
                break;
            }


            //others
            default:{
                qDebug() << "Move up not processed!";
                break;
            }
        }
    }
}

void NineMLALScene::deleteSelectedItem()
{
    root->properties->clear();
    QGraphicsItem *g = NULL;
    foreach(QGraphicsItem *i, selectedItems())
    {
        if (i->flags() & QGraphicsItem::ItemIsSelectable)
        {
            g = i;
            break;
        }
    }
    if (g != NULL)
    {

        const int type = g->type();
        if (type == RegimeGraphicsItem::Type)
        {
            //delete the graphics item
            RegimeGraphicsItem *ri = (RegimeGraphicsItem*)g;
            removeRegimeItem(ri);
            root->notifyDataChange();
        }
        else if (type == TimeDerivativeTextItem::Type)
        {
            //remove the graphics item
            TimeDerivativeTextItem *ti = (TimeDerivativeTextItem*)g;
            TimeDerivative *td = ti->time_derivative;
            RegimeGraphicsItem *rgi = (RegimeGraphicsItem*)ti->getTextItemGroup();
            rgi->regime->TimeDerivativeList.erase(std::remove(rgi->regime->TimeDerivativeList.begin(), rgi->regime->TimeDerivativeList.end(), td), rgi->regime->TimeDerivativeList.end());
            delete td;
            td = NULL;
            rgi->removeMember(ti);
            root->notifyDataChange();
        }
        else if (type == ParameterTextItem::Type)
        {
            //remove the graphics item
            ParameterTextItem *pi = (ParameterTextItem*)g;
            Parameter *p = pi->parameter;
            root->al->ParameterList.erase(std::remove(root->al->ParameterList.begin(), root->al->ParameterList.end(), p), root->al->ParameterList.end());
            //update any impulse send ports
            for (uint i=0;i<root->al->ImpulsePortList.size(); i++){
                ImpulsePort *ip = root->al->ImpulsePortList[i];
                if (ip->parameter == p)
                    ip->parameter = NULL;
            }
            delete p;
            p = NULL;
            pl_item->removeMember(pi);
            root->notifyDataChange();
        }
        else if (type == StateVariableTextItem::Type)
        {
            //remove the graphics item
            StateVariableTextItem *svti = (StateVariableTextItem*)g;
            StateVariable *sv = svti->state_variable;
            root->al->StateVariableList.erase(std::remove(root->al->StateVariableList.begin(), root->al->StateVariableList.end(), sv), root->al->StateVariableList.end());
            //update any time derivatives and on conditions
            for (uint i=0; i<root->al->RegimeList.size();i++){
                Regime * r = root->al->RegimeList[i];
                //time derivatives
                for (uint j=0;j<r->TimeDerivativeList.size(); j++){
                    TimeDerivative *td = r->TimeDerivativeList[j];
                    if (td->variable == sv)
                        td->variable = NULL;
                }
                //on condition state assignments
                for (uint j=0; j<r->OnConditionList.size();j++){
                    OnCondition *oc = r->OnConditionList[j];
                    for (uint k=0; k<oc->StateAssignList.size();k++){
                        StateAssignment *sa = oc->StateAssignList[k];
                        if (sa->variable == sv)
                            sa->variable = NULL;
                    }
                }
                //on event state assignments
                for (uint j=0; j<r->OnEventList.size();j++){
                    OnEvent *oe = r->OnEventList[j];
                    for (uint k=0; k<oe->StateAssignList.size();k++){
                        StateAssignment *sa = oe->StateAssignList[k];
                        if (sa->variable == sv)
                            sa->variable = NULL;
                    }
                }
                //on impulse state assignments
                for (uint j=0; j<r->OnImpulseList.size();j++){
                    OnImpulse *oi = r->OnImpulseList[j];
                    for (uint k=0; k<oi->StateAssignList.size();k++){
                        StateAssignment *sa = oi->StateAssignList[k];
                        if (sa->variable == sv)
                            sa->variable = NULL;
                    }
                }
            }
            //update any analog send ports
            for (uint i=0;i<root->al->AnalogPortList.size(); i++){
                AnalogPort *ap = root->al->AnalogPortList[i];
                if (ap->variable == sv)
                    ap->variable = NULL;
            }
            //update any impulse send ports
            for (uint i=0;i<root->al->ImpulsePortList.size(); i++){
                ImpulsePort *ip = root->al->ImpulsePortList[i];
                if (ip->parameter == sv)
                    ip->parameter = NULL;
            }
            //delete
            delete sv;
            sv = NULL;
            pl_item->removeMember(svti);
            root->notifyDataChange();
        }
        else if (type == AliasTextItem::Type)
        {
            //remove the graphics item
            AliasTextItem *ati = (AliasTextItem*)g;
            Alias *a = ati->alias;
            root->al->AliasList.erase(std::remove(root->al->AliasList.begin(), root->al->AliasList.end(), a), root->al->AliasList.end());
            //update any analog send ports
            for (uint i=0;i<root->al->AnalogPortList.size(); i++){
                AnalogPort *ap = root->al->AnalogPortList[i];
                if (ap->variable == a)
                    ap->variable = NULL;
            }
            //update any impulse send ports
            for (uint i=0;i<root->al->ImpulsePortList.size(); i++){
                ImpulsePort *ip = root->al->ImpulsePortList[i];
                if (ip->parameter == a)
                    ip->parameter = NULL;
            }
            //delete
            delete a;
            a = NULL;
            pl_item->removeMember(ati);
            root->notifyDataChange();
        }
        else if (type == AnalogPortTextItem::Type)
        {
            //remove the graphics item
            AnalogPortTextItem *apti = (AnalogPortTextItem*)g;
            AnalogPort *ap = apti->port;
            root->al->AnalogPortList.erase(std::remove(root->al->AnalogPortList.begin(), root->al->AnalogPortList.end(), ap), root->al->AnalogPortList.end());
            delete ap;
            ap = NULL;
            portl_item->removeMember(apti);
            root->notifyDataChange();
        }
        else if (type == EventPortTextItem::Type)
        {
            //remove the graphics item
            EventPortTextItem *epti = (EventPortTextItem*)g;
            EventPort *ep = epti->port;
            root->al->EventPortList.erase(std::remove(root->al->EventPortList.begin(), root->al->EventPortList.end(), ep), root->al->EventPortList.end());
            //update any event outputs
            for (uint i=0; i<root->al->RegimeList.size();i++){
                Regime * r = root->al->RegimeList[i];
                //on condition event outputs
                for (uint j=0; j<r->OnEventList.size();j++){
                    OnEvent *oe = r->OnEventList[j];
                    for (uint k=0; k<oe->eventOutList.size();k++){
                        EventOut *eo = oe->eventOutList[k];
                        if (eo->port == ep)
                            eo->port = NULL;
                    }
                }
                //on event event outputs
                for (uint j=0; j<r->OnConditionList.size();j++){
                    OnCondition *oc = r->OnConditionList[j];
                    for (uint k=0; k<oc->eventOutList.size();k++){
                        EventOut *eo = oc->eventOutList[k];
                        if (eo->port == ep)
                            eo->port = NULL;
                    }
                }
                //on impulse event outputs
                for (uint j=0; j<r->OnImpulseList.size();j++){
                    OnImpulse *oi = r->OnImpulseList[j];
                    for (uint k=0; k<oi->eventOutList.size();k++){
                        EventOut *eo = oi->eventOutList[k];
                        if (eo->port == ep)
                            eo->port = NULL;
                    }
                }
            }
            delete ep;
            ep = NULL;
            portl_item->removeMember(epti);
            root->notifyDataChange();
        }
        else if (type == ImpulsePortTextItem::Type)
        {
            //remove the graphics item
            ImpulsePortTextItem *ipti = (ImpulsePortTextItem*)g;
            ImpulsePort *ip = ipti->port;
            root->al->ImpulsePortList.erase(std::remove(root->al->ImpulsePortList.begin(), root->al->ImpulsePortList.end(), ip), root->al->ImpulsePortList.end());
            //update any event outputs
            for (uint i=0; i<root->al->RegimeList.size();i++){
                Regime * r = root->al->RegimeList[i];
                //on event impulse outputs
                for (uint j=0; j<r->OnEventList.size();j++){
                    OnEvent *oe = r->OnEventList[j];
                    for (uint k=0; k<oe->impulseOutList.size();k++){
                        ImpulseOut *eo = oe->impulseOutList[k];
                        if (eo->port == ip)
                            eo->port = NULL;
                    }
                }
                //on condition impulse outputs
                for (uint j=0; j<r->OnConditionList.size();j++){
                    OnCondition *oc = r->OnConditionList[j];
                    for (uint k=0; k<oc->impulseOutList.size();k++){
                        ImpulseOut *eo = oc->impulseOutList[k];
                        if (eo->port == ip)
                            eo->port = NULL;
                    }
                }
                //on impulse impulse outputs
                for (uint j=0; j<r->OnImpulseList.size();j++){
                    OnImpulse *oi = r->OnImpulseList[j];
                    for (uint k=0; k<oi->impulseOutList.size();k++){
                        ImpulseOut *eo = oi->impulseOutList[k];
                        if (eo->port == ip)
                            eo->port = NULL;
                    }
                }
            }
            delete ip;
            ip = NULL;
            portl_item->removeMember(ipti);
            root->notifyDataChange();
        }
        else if (type == StateAssignmentTextItem::Type)
        {
            //remove the graphics item
            StateAssignmentTextItem *sati = (StateAssignmentTextItem*)g;
            StateAssignment *sa = sati->assignment;
            NineMLTransitionItem* ti = (NineMLTransitionItem*)sati->getTextItemGroup();
            switch(ti->transitionType()){
                case(TRANSITION_TYPE_ON_CONDITION):{
                    OnConditionGraphicsItem *ocgi = (OnConditionGraphicsItem*)ti;
                    ocgi->on_condition->StateAssignList.erase(std::remove(ocgi->on_condition->StateAssignList.begin(), ocgi->on_condition->StateAssignList.end(), sa), ocgi->on_condition->StateAssignList.end());
                    delete sa;
                    sa = NULL;
                    ocgi->removeMember(sati);
                    root->notifyDataChange();
                    break;
                }
                case(TRANSITION_TYPE_ON_EVENT):{
                    OnEventGraphicsItem *oegi = (OnEventGraphicsItem*)ti;
                    oegi->on_event->StateAssignList.erase(std::remove(oegi->on_event->StateAssignList.begin(), oegi->on_event->StateAssignList.end(), sa), oegi->on_event->StateAssignList.end());
                    delete sa;
                    sa = NULL;
                    oegi->removeMember(sati);
                    root->notifyDataChange();
                    break;
                }
                case(TRANSITION_TYPE_ON_IMPULSE):{
                    OnImpulseGraphicsItem *oigi = (OnImpulseGraphicsItem*)ti;
                    oigi->on_impulse->StateAssignList.erase(std::remove(oigi->on_impulse->StateAssignList.begin(), oigi->on_impulse->StateAssignList.end(), sa), oigi->on_impulse->StateAssignList.end());
                    delete sa;
                    sa = NULL;
                    oigi->removeMember(sati);
                    root->notifyDataChange();
                    break;
                }
            }


        }
        else if (type == EventOutTextItem::Type)
        {
            //remove the graphics item
            EventOutTextItem *eoti = (EventOutTextItem*)g;
            EventOut *eo = eoti->event_out;
            NineMLTransitionItem* ti = (NineMLTransitionItem*)eoti->getTextItemGroup();
            switch(ti->transitionType()){
                case(TRANSITION_TYPE_ON_CONDITION):{
                    OnConditionGraphicsItem *ocgi = (OnConditionGraphicsItem*)ti;
                    ocgi->on_condition->eventOutList.erase(std::remove(ocgi->on_condition->eventOutList.begin(), ocgi->on_condition->eventOutList.end(), eo), ocgi->on_condition->eventOutList.end());
                    delete eo;
                    eo = NULL;
                    ocgi->removeMember(eoti);
                    root->notifyDataChange();
                    break;
                }
                case(TRANSITION_TYPE_ON_EVENT):{
                    OnEventGraphicsItem *oegi = (OnEventGraphicsItem*)ti;
                    oegi->on_event->eventOutList.erase(std::remove(oegi->on_event->eventOutList.begin(), oegi->on_event->eventOutList.end(), eo), oegi->on_event->eventOutList.end());
                    delete eo;
                    eo = NULL;
                    oegi->removeMember(eoti);
                    root->notifyDataChange();
                    break;
                }
                case(TRANSITION_TYPE_ON_IMPULSE):{
                    OnImpulseGraphicsItem *oigi = (OnImpulseGraphicsItem*)ti;
                    oigi->on_impulse->eventOutList.erase(std::remove(oigi->on_impulse->eventOutList.begin(), oigi->on_impulse->eventOutList.end(), eo), oigi->on_impulse->eventOutList.end());
                    delete eo;
                    eo = NULL;
                    oigi->removeMember(eoti);
                    root->notifyDataChange();
                    break;
                }
            }
        }
        else if (type == ImpulseOutTextItem::Type)
        {
            //remove the graphics item
            ImpulseOutTextItem *ioti = (ImpulseOutTextItem*)g;
            ImpulseOut *io = ioti->impulse_out;
            NineMLTransitionItem* ti = (NineMLTransitionItem*)ioti->getTextItemGroup();
            switch(ti->transitionType()){
                case(TRANSITION_TYPE_ON_CONDITION):{
                    OnConditionGraphicsItem *ocgi = (OnConditionGraphicsItem*)ti;
                    ocgi->on_condition->impulseOutList.erase(std::remove(ocgi->on_condition->impulseOutList.begin(), ocgi->on_condition->impulseOutList.end(), io), ocgi->on_condition->impulseOutList.end());
                    delete io;
                    io = NULL;
                    ocgi->removeMember(ioti);
                    root->notifyDataChange();
                    break;
                }
                case(TRANSITION_TYPE_ON_EVENT):{
                    OnEventGraphicsItem *oegi = (OnEventGraphicsItem*)ti;
                    oegi->on_event->impulseOutList.erase(std::remove(oegi->on_event->impulseOutList.begin(), oegi->on_event->impulseOutList.end(), io), oegi->on_event->impulseOutList.end());
                    delete io;
                    io = NULL;
                    oegi->removeMember(ioti);
                    root->notifyDataChange();
                    break;
                }
                case(TRANSITION_TYPE_ON_IMPULSE):{
                    OnImpulseGraphicsItem *oigi = (OnImpulseGraphicsItem*)ti;
                    oigi->on_impulse->impulseOutList.erase(std::remove(oigi->on_impulse->impulseOutList.begin(), oigi->on_impulse->impulseOutList.end(), io), oigi->on_impulse->impulseOutList.end());
                    delete io;
                    io = NULL;
                    oigi->removeMember(ioti);
                    root->notifyDataChange();
                    break;
                }
            }
        }
        else if (type == OnConditionGraphicsItem::Type)
        {
            //delete the graphics item
            OnConditionGraphicsItem *ocgi = (OnConditionGraphicsItem*)g;
            removeOnCondition(ocgi);
            root->notifyDataChange();
        }
        else if (type == OnEventGraphicsItem::Type)
        {
            //delete the graphics item
            OnEventGraphicsItem *oegi = (OnEventGraphicsItem*)g;
            removeOnEvent(oegi);
            root->notifyDataChange();
        }
        else if (type == OnImpulseGraphicsItem::Type)
        {
            //delete the graphics item
            OnImpulseGraphicsItem *oigi = (OnImpulseGraphicsItem*)g;
            removeOnImpulse(oigi);
            root->notifyDataChange();
        }
        else
            qDebug() << "Deletion not handled!";

        //update any referencing items
        for (uint i=0; i<rg_items.size();i++){
            rg_items[i]->updateMemberContents();
        }
        for (uint i=0; i<oc_items.size();i++){
            oc_items[i]->updateMemberContents();
        }
        for (uint i=0; i<oe_items.size();i++){
            oe_items[i]->updateMemberContents();
        }
        for (uint i=0; i<oi_items.size();i++){
            oi_items[i]->updateMemberContents();
        }
        portl_item->updateMemberContents();


        //update layout
        root->requestLayoutUpdate();
    }
}




RegimeGraphicsItem* NineMLALScene::addRegimeItem(Regime *r)
{
    RegimeGraphicsItem *rg = new RegimeGraphicsItem(r, root);
    addItem(rg);
    rg_items.push_back(rg);
    connect(root, SIGNAL(initialRegimeChanged()), rg, SLOT(updateContent()));
    return rg;
}



void NineMLALScene::removeRegimeItem(RegimeGraphicsItem *ri)
{
    Regime *r = ri->regime;

    foreach (OnConditionGraphicsItem *ocgi, oc_items)
    {
        Regime *src_regime = ocgi->getSourceRegime();
        Regime *dst_regime = ocgi->getSynapseRegime();
        if ((src_regime == r)||(dst_regime == r))
        {
            removeOnCondition(ocgi);
        }
    }

    foreach (OnEventGraphicsItem *oegi, oe_items)
    {
        Regime *src_regime = oegi->getSourceRegime();
        Regime *dst_regime = oegi->getSynapseRegime();
        if ((src_regime == r)||(dst_regime == r))
        {
            removeOnEvent(oegi);
        }
    }

    foreach (OnImpulseGraphicsItem *oigi, oi_items)
    {
        Regime *src_regime = oigi->getSourceRegime();
        Regime *dst_regime = oigi->getSynapseRegime();
        if ((src_regime == r)||(dst_regime == r))
        {
            removeOnImpulse(oigi);
        }
    }

    if (root->al->initial_regime == r){
        root->al->initial_regime = NULL;
    }

    //remove regime graphics items
    rg_items.erase(std::remove(rg_items.begin(), rg_items.end(), ri), rg_items.end());
    root->gvlayout->removeGVItem(ri);
    root->al->RegimeList.erase(std::remove(root->al->RegimeList.begin(), root->al->RegimeList.end(), r), root->al->RegimeList.end());
    delete r;
    r = NULL;
    removeItem(ri);
    delete ri;
    //ri->deleteLater();
    ri = NULL;
}

OnConditionGraphicsItem* NineMLALScene::addOnConditionItem(Regime *r, OnCondition* oc)
{
    OnConditionGraphicsItem *oci = new OnConditionGraphicsItem(r, oc, root);
    addItem(oci);
    oc_items.push_back(oci);
    return oci;
}

void NineMLALScene::removeOnCondition(OnConditionGraphicsItem *ocgi)
{
    oc_items.erase(std::remove(oc_items.begin(), oc_items.end(), ocgi), oc_items.end());
    root->gvlayout->removeGVItem(ocgi);
    ocgi->src_regime->OnConditionList.erase(std::remove(ocgi->src_regime->OnConditionList.begin(), ocgi->src_regime->OnConditionList.end(), ocgi->on_condition), ocgi->src_regime->OnConditionList.end());
    delete ocgi->on_condition;
    ocgi->on_condition = NULL;
    removeItem(ocgi);
    delete ocgi;
    //ocgi->deleteLater();
    ocgi = NULL;
}

OnEventGraphicsItem *NineMLALScene::addOnEventItem(Regime *r, OnEvent *oe)
{
    OnEventGraphicsItem *oei = new OnEventGraphicsItem(r, oe, root);
    addItem(oei);
    oe_items.push_back(oei);
    return oei;
}

OnImpulseGraphicsItem *NineMLALScene::addOnImpulseItem(Regime *r, OnImpulse *oi)
{
    OnImpulseGraphicsItem *oii = new OnImpulseGraphicsItem(r, oi, root);
    addItem(oii);
    oi_items.push_back(oii);
    return oii;
}

void NineMLALScene::removeOnEvent(OnEventGraphicsItem *oegi)
{
    oe_items.erase(std::remove(oe_items.begin(), oe_items.end(), oegi), oe_items.end());
    root->gvlayout->removeGVItem(oegi);
    oegi->src_regime->OnEventList.erase(std::remove(oegi->src_regime->OnEventList.begin(), oegi->src_regime->OnEventList.end(), oegi->on_event), oegi->src_regime->OnEventList.end());
    delete oegi->on_event;
    oegi->on_event = NULL;
    removeItem(oegi);
    delete oegi;
    //oegi->deleteLater();
    oegi = NULL;
}

void NineMLALScene::removeOnImpulse(OnImpulseGraphicsItem *oigi)
{
    oi_items.erase(std::remove(oi_items.begin(), oi_items.end(), oigi), oi_items.end());
    root->gvlayout->removeGVItem(oigi);
    oigi->src_regime->OnImpulseList.erase(std::remove(oigi->src_regime->OnImpulseList.begin(), oigi->src_regime->OnImpulseList.end(), oigi->on_impulse), oigi->src_regime->OnImpulseList.end());
    delete oigi->on_impulse;
    oigi->on_impulse = NULL;
    removeItem(oigi);
    delete oigi;
    //oigi->deleteLater();
    oigi = NULL;
}


void NineMLALScene::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if (mouseEvent->button() == Qt::LeftButton)
    {
        switch(mode){
            case (ModeSelect):
            {
                //code duplicated in root->clearslection
                root->clearSelection();
                QGraphicsScene::mousePressEvent(mouseEvent);
                break;
            }
            case (ModeInsertOnCondition):
            case(ModeInsertOnEvent):
            case(ModeInsertOnImpulse):
            {
                QGraphicsScene::mousePressEvent(mouseEvent);
                root->clearSelection();
                QList <QGraphicsItem*> selected = items(mouseEvent->scenePos());
                for (int i=0; i<selected.size(); i++)
                {
                    QGraphicsItem *g = selected[i];
                    if (g->type() == RegimeGraphicsItem::Type)
                    {
                        QRectF bounds = g->boundingRect();
                        QPointF offset = QPointF(bounds.width(), bounds.height());
                        offset /= 2.0;
                        QPointF p = g->pos();
                        p += offset;
                        p -= QPointF(RegimeGraphicsItem::padding*2, RegimeGraphicsItem::padding*2);
                        transition_origin_point = p;
                        transition = new ArrowItem();
                        transition->setZValue(2);
                        transition->setLineWidth(5);
                        addItem(transition);
                        transition_origin = (RegimeGraphicsItem*)g;
                    }
                }

                break;
            }
        }
    }
    //mouseEvent->accept();
}

void NineMLALScene::mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent){
    QGraphicsScene::mouseMoveEvent(mouseEvent);
    switch(mode){
        case (ModeSelect):
        {
            break;
        }
        case (ModeInsertOnCondition):
        case(ModeInsertOnEvent):
        case(ModeInsertOnImpulse):
        {
            if (transition != NULL)
            {
                QPainterPath line;
                line.moveTo(transition_origin_point.x(), transition_origin_point.y());
                line.lineTo(mouseEvent->scenePos());
                transition->setPath(line);
                transition->setPenColour(Qt::red);
                //if regime is selected then line should be red
                QList <QGraphicsItem*> selected = items(mouseEvent->scenePos());
                for (int i=0; i<selected.size(); i++)
                {
                    QGraphicsItem *g = selected[i];
                    if (g->type() == RegimeGraphicsItem::Type)
                    {
                        transition->setPenColour(Qt::green);
                    }
                }
            }
            break;
        }
    }
    //mouseEvent->accept();

}

void NineMLALScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if (mouseEvent->button() == Qt::LeftButton)
    {
        QGraphicsScene::mouseReleaseEvent(mouseEvent);
        switch(mode){
            case (ModeSelect):
            {
                if (selectedItems().size() < 1){
                    root->clearSelection();
                    root->properties->createEmptySelectionProperties();
                }

                break;
            }
            case (ModeInsertOnCondition):
            case(ModeInsertOnEvent):
            case(ModeInsertOnImpulse):
            {
                if (transition != NULL)
                {
                    QList <QGraphicsItem*> selected = items(mouseEvent->scenePos());
                    for (int i=0; i<selected.size(); i++)
                    {
                        QGraphicsItem *g = selected[i];
                        if (g->type() == RegimeGraphicsItem::Type)
                        {
                            RegimeGraphicsItem *rg = (RegimeGraphicsItem*)g;

                            NineMLComponent * oldComponent = new NineMLComponent(root->al);

                            //create a transition
                            if (mode == ModeInsertOnCondition)
                            {
                                OnCondition * oc = new OnCondition();
                                oc->target_regime_name = rg->getRegimeName();
                                // validate to set up pointers
                                QStringList errs;
                                oc->validateOnCondition(root->al, &errs);
                                // clear errors
                                QSettings settings;
                                settings.remove("errors");
                                settings.remove("warnings");
                                transition_origin->regime->OnConditionList.push_back(oc);
                                OnConditionGraphicsItem *ocg = addOnConditionItem(transition_origin->regime, oc);
                                root->requestLayoutUpdate();
                                root->setSelectionMode(ModeSelect); //return to selection mode
                                ocg->setSelected(true);
                                root->notifyDataChange();
                            }
                            else if (mode == ModeInsertOnEvent)
                            {
                                OnEvent *oe = new OnEvent();
                                oe->target_regime_name = rg->getRegimeName();
                                // validate to set up pointers
                                QStringList errs;
                                oe->validateOnEvent(root->al, &errs);
                                // clear errors
                                QSettings settings;
                                settings.remove("errors");
                                settings.remove("warnings");
                                transition_origin->regime->OnEventList.push_back(oe);
                                OnEventGraphicsItem * oei = addOnEventItem(transition_origin->regime, oe);
                                root->requestLayoutUpdate();
                                root->setSelectionMode(ModeSelect); //return to selection mode
                                oei->setSelected(true);
                                root->notifyDataChange();

                            }
                            else if (mode == ModeInsertOnImpulse)
                            {
                                OnImpulse *oi = new OnImpulse();
                                oi->target_regime_name = rg->getRegimeName();
                                // validate to set up pointers
                                QStringList errs;
                                oi->validateOnImpulse(root->al, &errs);
                                // clear errors
                                QSettings settings;
                                settings.remove("errors");
                                settings.remove("warnings");
                                transition_origin->regime->OnImpulseList.push_back(oi);
                                OnImpulseGraphicsItem * oii = addOnImpulseItem(transition_origin->regime, oi);
                                root->requestLayoutUpdate();
                                root->setSelectionMode(ModeSelect); //return to selection mode
                                oii->setSelected(true);
                                root->notifyDataChange();

                            }


                            root->alPtr->undoStack.push(new changeComponent(root, oldComponent, "Add Transition"));
                        }
                    }
                    delete transition;
                    transition = NULL;
                }
                break;
            }
        }
    }
    //mouseEvent->accept();
}


void NineMLALScene::setMode(ALSceneMode m)
{
    mode = m;
}




