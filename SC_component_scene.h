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

#ifndef NINEML_ALSCENE_H
#define NINEML_ALSCENE_H

#include <QtGui>
#include <algorithm>
#include "SC_component_propertiesmanager.h"
#include "SC_component_gvitems.h"
#include "SC_component_rootcomponentitem.h"

class NineMLALScene : public QGraphicsScene
{
    Q_OBJECT
public:
    NineMLALScene(RootComponentItem *r);
    ~NineMLALScene();

    void setMode(ALSceneMode mode);
    void initialiseScene(QSharedPointer<Component>al);
    RegimeGraphicsItem* addRegimeItem(Regime *r);
    OnConditionGraphicsItem* addOnConditionItem(Regime *r, OnCondition* oc);
    OnEventGraphicsItem* addOnEventItem(Regime *r, OnEvent* oe);
    OnImpulseGraphicsItem* addOnImpulseItem(Regime *r, OnImpulse* oe);



    void removeRegimeItem(RegimeGraphicsItem *r);
    void removeOnCondition(OnConditionGraphicsItem* oc);
    void removeOnEvent(OnEventGraphicsItem* oei);
    void removeOnImpulse(OnImpulseGraphicsItem* oei);

    Agnode_t* getRegimeGVNode(Regime* r);

    void setParamsVisibility(bool visible);
    void setPortsVisibility(bool visible);
    void moveItemUp();
    void moveItemDown();
    void deleteSelectedItem();





protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent);

    template <typename T> bool moveUp(QVector <T> *vector, T item){
        int index = std::find(vector->begin(), vector->end(), item) - vector->begin();

        if ((index > 0)&&(index < (int)vector->size())){
            std::swap((*vector)[index], (*vector)[index-1]);
            return true;
        }else{
            return false;
        }
    }
    template <typename T> bool moveDown(QVector <T> *vector, T item){
        int index = std::find(vector->begin(), vector->end(), item) - vector->begin();

        if ((index >= 0)&&(index < (int)vector->size() -1)){
            std::swap((*vector)[index], (*vector)[index+1]);
            return true;
        }else{
            return false;
        }
    }


public: //TODO: private these up

    QVector <RegimeGraphicsItem*> rg_items;
    QVector <OnConditionGraphicsItem*> oc_items;
    QVector <OnEventGraphicsItem*> oe_items;
    QVector <OnImpulseGraphicsItem*> oi_items;
    ParameterListGraphicsItem *pl_item;
    PortListGraphicsItem *portl_item;
    AnnotationListGraphicsItem *annotl_item;

private:
    ALSceneMode mode;
    RootComponentItem *root;    //DO WE REALLY NEED THIS OR JUST THE al

    //QGraphicsLineItem *transition_line;
    ArrowItem *transition;
    QPointF transition_origin_point;
    RegimeGraphicsItem *transition_origin;

};

#endif // NINEML_ALSCENE_H
