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
#ifndef PROPERTIESMANAGER_H

#define PROPERTIESMANAGER_H

#include <QtGui>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "nineml_graphicsitems.h"

class FilterObject : public QObject
{
    Q_OBJECT

protected:
    bool eventFilter(QObject *, QEvent *event);
};

class PropertiesManager : public QFormLayout
{
    Q_OBJECT
public:
    explicit PropertiesManager(RootComponentItem *root);
    ~PropertiesManager();

    void clear();
    void createEmptySelectionProperties();
    void createRegimeProperties(RegimeGraphicsItem *ri);
    void createTimeDerivativeProperties(TimeDerivativeTextItem *tdi);
    void createParameterListProperties();
    void createParameterProperties(ParameterTextItem *pti);
    void createStateVariableProperties(StateVariableTextItem *svti);
    void createAliasProperties(AliasTextItem *ati);
    void createPortListProperties();
    void createAnalogPortProperties(AnalogPortTextItem *ap);
    void createEventPortProperties(EventPortTextItem *ev);
    void createImpulsePortProperties(ImpulsePortTextItem *ip);
    void createOnConditionProperties(OnConditionGraphicsItem *oci);
    void createOnEventProperties(OnEventGraphicsItem *oei);
    void createOnImpulseProperties(OnImpulseGraphicsItem *oii);
    void createStateAssignmentProperties(StateAssignmentTextItem *i);
    void createEventOutProperties(EventOutTextItem *i);
    void createImpulseOutProperties(ImpulseOutTextItem *i);
    void updateReorderingIcons(GroupedTextItem *item);


protected:
    static void clearLayoutItems(QLayout *layout);
    QComboBox* getPrefixCombo(Prefix selected);
    QComboBox* getUnitCombo(Unit selected);


private:
    RootComponentItem *root;
    QRegExpValidator *validator;
    QRegExpValidator *componentNameValidator;

};

#endif // PROPERTIESMANAGER_H
