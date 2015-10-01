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


#include "propertiesmanager.h"
#include "nineml_graphicsitems.h"
#include "nineml_rootcomponentitem.h"

bool FilterObject::eventFilter(QObject *, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Z && (keyEvent->modifiers() == Qt::ControlModifier || keyEvent->modifiers() == (Qt::ControlModifier | Qt::ShiftModifier))) {
            // Special undo / redo handling
            return true;
        } else
            return false;
    }
    return false;
}

FilterObject eventFilterObject;

PropertiesManager::PropertiesManager(RootComponentItem *r) :
    QFormLayout()
{
    root = r;
    QRegExp rx("[A-Za-z_0-9]*");
    validator = new QRegExpValidator(rx, this);
    QRegExp rx2("[A-Za-z 0-9]*");
    componentNameValidator = new QRegExpValidator(rx2, this);
}

PropertiesManager::~PropertiesManager()
{
    clear();
    if (validator)
        delete validator;
    validator = NULL;
    if (componentNameValidator)
        delete componentNameValidator;
    componentNameValidator = NULL;
}

void PropertiesManager::clear()
{
    PropertiesManager::clearLayoutItems(this);
    root->addItemsToolbar->clear();
}


void PropertiesManager::clearLayoutItems(QLayout *layout)
{
    QLayoutItem *l = NULL;
    while ((l=layout->itemAt(0)) != NULL)
    {
        layout->removeItem(l);
        if (l->widget() != NULL){
            l->widget()->deleteLater();
            delete l;
            l = NULL;
        }else if (l->layout())
        {
            PropertiesManager::clearLayoutItems(l->layout());
        }
    }
}

QComboBox *PropertiesManager::getPrefixCombo(Prefix selected)
{
    QComboBox *dims_prefix = new QComboBox();
    dims_prefix->addItem("");
    dims_prefix->addItem("G giga");
    dims_prefix->addItem("M mega");
    dims_prefix->addItem("k kilo");
    dims_prefix->addItem("c centi");
    dims_prefix->addItem("m milli");
    dims_prefix->addItem("u micro");
    dims_prefix->addItem("n nano");
    dims_prefix->addItem("p pico");
    dims_prefix->addItem("f femto");
    dims_prefix->setCurrentIndex(selected);
    return dims_prefix;
}

QComboBox *PropertiesManager::getUnitCombo(Unit selected)
{
    QComboBox *dims_unit = new QComboBox();
    dims_unit->addItem("");
    dims_unit->addItem("V volt");
    dims_unit->addItem("Ohm");
    dims_unit->addItem("g gram");
    dims_unit->addItem("m metre");
    dims_unit->addItem("S siemen");
    dims_unit->addItem("A ampere");
    dims_unit->addItem("cd candela");
    dims_unit->addItem("mol");
    dims_unit->addItem("degC");
    dims_unit->addItem("s second");
    dims_unit->addItem("F farad");
    dims_unit->addItem("Hz hertz");
    dims_unit->setCurrentIndex(selected);
    return dims_unit;
}

void PropertiesManager::updateReorderingIcons(GroupedTextItem *item)
{
    int index = item->getIndexPosition();
    //move up
    if (index > 0)
        root->actionMove_Up->setEnabled(true);
    else
        root->actionMove_Up->setEnabled(false);
    //move down
    if (index < (item->getTextItemGroup()->getMemberCount()-1))
        root->actionMove_Down->setEnabled(true);
    else
        root->actionMove_Down->setEnabled(false);
}


void PropertiesManager::createEmptySelectionProperties()
{
    root->addItemsToolbar->addAction(root->actionAddRegime);
    root->actionMove_Up->setEnabled(false);
    root->actionMove_Down->setEnabled(false);
    root->actionDeleteItems->setEnabled(false);

    //component name
    QLabel *label = new QLabel("<b>Component Class</b>");
    addRow(label);
    QLineEdit *edit_name = new QLineEdit();
    edit_name->installEventFilter(&eventFilterObject);
    edit_name->setValidator(componentNameValidator);
    edit_name->setText(root->al->name);
    connect(edit_name, SIGNAL(textEdited(QString)), root, SLOT(setComponentClassName(QString)));
    addRow(tr("&Name:"),edit_name);

    //component type
    QComboBox *type = new QComboBox();
    type->addItem("neuron_body");
    type->addItem("weight_update");
    type->addItem("postsynapse");
    //type->addItem("generic_component");
    int typeVal;
    if (root->al->type == "neuron_body")
        typeVal = 0;
    if (root->al->type == "weight_update")
        typeVal = 1;
    if (root->al->type == "postsynapse")
        typeVal = 2;
    if (root->al->type == "generic_component")
        typeVal = 3;
    type->setCurrentIndex(typeVal);
    connect(type, SIGNAL(currentIndexChanged(QString)), root, SLOT(setComponentClassType(QString)));
    addRow(tr("&Component Type:"),type);

    //initial regime
    QComboBox *initial_regime = new QComboBox();
    bool match_init_r = false;
    for (int i=0; i<root->al->RegimeList.size(); i++){
        initial_regime->addItem(root->al->RegimeList[i]->name);
        if (root->al->initial_regime == root->al->RegimeList[i]){
            initial_regime->setCurrentIndex(i);
            match_init_r = true;
        }
    }
    if (!match_init_r){
        initial_regime->setCurrentIndex(-1);
    }
    connect(initial_regime, SIGNAL(currentIndexChanged(QString)), root, SLOT(setInitialRegime(QString)));
    addRow(tr("&Initial Regime:"),initial_regime);

    // add path combobox - NO LONGER USED
    /*QComboBox * path = new QComboBox;
    path->addItem("model");
    path->addItem("temp");
    path->addItem("lib");

    path->setToolTip("Store the component externally as a file (temp) or internally in the library (lib)");

    // select current path - NO LONGER USED
    if (root->al->path == "")
        root->al->path = "temp";
    if (root->al->path == "model")
        path->setCurrentIndex(0);
    else if (root->al->path == "temp")
        path->setCurrentIndex(1);
    else if (root->al->path == "lib")
        path->setCurrentIndex(2);

    // disable model path
    QModelIndex ind = path->model()->index(0,0);
    path->model()->setData(ind, QVariant(0), Qt::UserRole-1);
    addRow(tr("Path"), path);
    connect(path, SIGNAL(currentIndexChanged(QString)), root, SLOT(setPath(QString)));*/

    QPushButton * duplicate = new QPushButton("Duplicate component");
    connect(duplicate, SIGNAL(clicked()), root->main, SLOT(duplicate_component()));
    addWidget(duplicate);


    QPushButton * remove = new QPushButton("Remove component");
    connect(remove, SIGNAL(clicked()), root, SLOT(deleteComponent()));
    addWidget(remove);

}


void PropertiesManager::createRegimeProperties(RegimeGraphicsItem *i)
{
    QLabel *label = new QLabel("<b>Edit Regime</b>");
    addRow(label);
    QLineEdit *edit_name = new QLineEdit();
    edit_name->installEventFilter(&eventFilterObject);
    edit_name->setText(i->getRegimeName());
    edit_name->setValidator(validator);
    connect(edit_name, SIGNAL(textEdited(QString)), i, SLOT(setRegimeName(QString)));
    addRow(tr("&Name:"),edit_name);

    root->addItemsToolbar->addAction(root->actionAddTimeDerivative);
    root->actionDeleteItems->setEnabled(true);
}

void PropertiesManager::createTimeDerivativeProperties(TimeDerivativeTextItem *td)
{
    //move up down
    root->actionMove_Up->setEnabled(true);
    root->actionMove_Down->setEnabled(true);
    root->actionDeleteItems->setEnabled(true);

    QLabel *label = new QLabel("<b>Edit Time Derivative</b>");
    addRow(label);
    QComboBox *var = new QComboBox();
    for (int i=0; i< root->al->StateVariableList.size(); i++)
    {
        var->addItem(root->al->StateVariableList[i]->getName());
        if (td->getVariable() != NULL){
            if (root->al->StateVariableList[i]->getName().compare(td->getVariable()->getName()) == 0)
            {
                var->setCurrentIndex(i);
            }
        }
        else
            var->setCurrentIndex(-1);
    }
    connect(var, SIGNAL(currentIndexChanged(QString)), td, SLOT(setVariable(QString)));
    addRow(tr("&Variable:"),var);

    QLineEdit *maths = new QLineEdit();
    maths->installEventFilter(&eventFilterObject);
    maths->setText(td->getMaths()->equation);
    connect(maths, SIGNAL(textEdited(QString)), td, SLOT(setMaths(QString)));
    addRow(tr("&Maths:"),maths);
    addWidget(new QLabel(tr("<i>in physiological units</i>")));

    QStringList errs;
    td->time_derivative->maths->validateMathInLine(root->al.data(), &errs);

    // sort out errors
    QSettings settings;
    int num_errs = settings.beginReadArray("warnings");
    settings.endArray();

    if (num_errs != 0) {

        // show errors by changing lineedit colour
        QPalette p = maths->palette();
        p.setColor( QPalette::Normal, QPalette::Base, QColor(255, 200, 200) );
        maths->setPalette(p);

        // clear errors
        settings.remove("warnings");

    }
    if (num_errs == 0) {

        // show no errors by changing lineedit colour
        QPalette p = maths->palette();
        p.setColor( QPalette::Normal, QPalette::Base, QColor(255, 255, 255) );
        maths->setPalette(p);

        // clear errors
        settings.remove("warnings");
    }
}






void PropertiesManager::createParameterListProperties()
{
    root->addItemsToolbar->addAction(root->actionAddParameter);
    root->addItemsToolbar->addAction(root->actionAddStateVariable);
    root->addItemsToolbar->addAction(root->actionAddAlias);
    root->actionDeleteItems->setEnabled(false);
}

void PropertiesManager::createParameterProperties(ParameterTextItem *pti)
{
    //move up down
    updateReorderingIcons(pti);
    root->actionDeleteItems->setEnabled(true);

    QLabel *label = new QLabel("<b>Edit Parameter</b>");
    addRow(label);

    QLineEdit *name = new QLineEdit();
    name->installEventFilter(&eventFilterObject);
    name->setText(pti->getName());
    name->setValidator(validator);
    connect(name, SIGNAL(textChanged(QString)), pti, SLOT(setName(QString)));
    addRow(tr("&Name:"),name);

    QComboBox *dims_prefix = getPrefixCombo(pti->parameter->dims->getPrefix());
    connect(dims_prefix, SIGNAL(currentIndexChanged(QString)), pti, SLOT(setDimsPrefix(QString)));
    addRow(tr("Dimensionality &Prefix:"),dims_prefix);

    QComboBox *dims_unit = getUnitCombo(pti->parameter->dims->getUnit());
    connect(dims_unit, SIGNAL(currentIndexChanged(QString)), pti, SLOT(setDimsUnit(QString)));
    addRow(tr("Dimensionality &Unit:"),dims_unit);
}


void PropertiesManager::createStateVariableProperties(StateVariableTextItem *svti)
{
    //move up down
    root->actionMove_Up->setEnabled(true);
    root->actionMove_Down->setEnabled(true);
    root->actionDeleteItems->setEnabled(true);

    QLabel *label = new QLabel("<b>Edit State Variable</b>");
    addRow(label);

    QLineEdit *name = new QLineEdit();
    name->installEventFilter(&eventFilterObject);
    name->setText(svti->getName());
    name->setValidator(validator);
    connect(name, SIGNAL(textChanged(QString)), svti, SLOT(setName(QString)));
    addRow(tr("&Name:"), name);

    QComboBox *dims_prefix = getPrefixCombo(svti->state_variable->dims->getPrefix());
    connect(dims_prefix, SIGNAL(currentIndexChanged(QString)), svti, SLOT(setDimsPrefix(QString)));
    addRow(tr("Dimensionality &Prefix:"),dims_prefix);

    QComboBox *dims_unit = getUnitCombo(svti->state_variable->dims->getUnit());
    connect(dims_unit, SIGNAL(currentIndexChanged(QString)), svti, SLOT(setDimsUnit(QString)));
    addRow(tr("Dimensionality &Unit:"),dims_unit);
}

void PropertiesManager::createAliasProperties(AliasTextItem *ati)
{
    //move up down
    root->actionMove_Up->setEnabled(true);
    root->actionMove_Down->setEnabled(true);
    root->actionDeleteItems->setEnabled(true);

    QLabel *label = new QLabel("<b>Edit Alias</b>");
    addRow(label);

    QLineEdit *name = new QLineEdit();
    name->installEventFilter(&eventFilterObject);
    name->setText(ati->getName());
    name->setValidator(validator);
    connect(name, SIGNAL(textChanged(QString)), ati, SLOT(setName(QString)));
    addRow(tr("&Name:"), name);

    QLineEdit *maths = new QLineEdit();
    maths->installEventFilter(&eventFilterObject);
    maths->setText(ati->getMaths()->equation);
    connect(maths, SIGNAL(textChanged(QString)), ati, SLOT(setMaths(QString)));
    addRow(tr("&Maths:"), maths);
    addWidget(new QLabel(tr("<i>in physiological units</i>")));

    QStringList errs;
    ati->getMaths()->validateMathInLine(root->al.data(), &errs);

    // sort out errors
    QSettings settings;
    int num_errs = settings.beginReadArray("warnings");
    settings.endArray();

    if (num_errs != 0) {

        // show errors by changing lineedit colour
        QPalette p = maths->palette();
        p.setColor( QPalette::Normal, QPalette::Base, QColor(255, 200, 200) );
        maths->setPalette(p);

        // clear errors
        settings.remove("warnings");

    }
    if (num_errs == 0) {

        // show no errors by changing lineedit colour
        QPalette p = maths->palette();
        p.setColor( QPalette::Normal, QPalette::Base, QColor(255, 255, 255) );
        maths->setPalette(p);

        // clear errors
        settings.remove("warnings");
    }
}


void PropertiesManager::createPortListProperties()
{
    root->addItemsToolbar->addAction(root->actionAddAnalogePort);
    root->addItemsToolbar->addAction(root->actionAddEventPort);
    root->addItemsToolbar->addAction(root->actionAddImpulsePort);
    root->actionDeleteItems->setEnabled(false);
}





void PropertiesManager::createOnConditionProperties(OnConditionGraphicsItem *oci)
{
    QLabel *label = new QLabel("<b>Edit On Condition</b>");
    addRow(label);

    QLineEdit *maths = new QLineEdit();
    maths->installEventFilter(&eventFilterObject);
    maths->setText(oci->getTriggerMaths()->equation);
    connect(maths, SIGNAL(textChanged(QString)), oci, SLOT(setTriggerMaths(QString)));
    addRow(tr("&Maths:"), maths);
    addWidget(new QLabel(tr("<i>in physiological units</i>")));

    QStringList errs;
    oci->getTriggerMaths()->validateMathInLine(root->al.data(), &errs);

    // sort out errors
    QSettings settings;
    int num_errs = settings.beginReadArray("warnings");
    settings.endArray();

    if (num_errs != 0) {

        // show errors by changing lineedit colour
        QPalette p = maths->palette();
        p.setColor( QPalette::Normal, QPalette::Base, QColor(255, 200, 200) );
        maths->setPalette(p);

        // clear errors
        settings.remove("warnings");

    }
    if (num_errs == 0) {

        // show no errors by changing lineedit colour
        QPalette p = maths->palette();
        p.setColor( QPalette::Normal, QPalette::Base, QColor(255, 255, 255) );
        maths->setPalette(p);

        // clear errors
        settings.remove("warnings");
    }

    //edit Synapse regime??

    root->addItemsToolbar->addAction(root->actionAddStateAssignment);
    root->addItemsToolbar->addAction(root->actionAddEventOut);
    root->addItemsToolbar->addAction(root->actionAddImpulseOut);
    root->actionDeleteItems->setEnabled(true);
}

void PropertiesManager::createOnEventProperties(OnEventGraphicsItem *oei)
{
    bool match = false;
    QLabel *label = new QLabel("<b>Edit On Event</b>");
    addRow(label);

    QComboBox *event = new QComboBox();
    for (int i=0; i< root->al->EventPortList.size(); i++)
    {
        if (root->al->EventPortList[i]->mode == EventRecvPort){
            event->addItem(root->al->EventPortList[i]->getName());
            if (oei->getEventPort() != NULL){
                if (root->al->EventPortList[i]->getName().compare(oei->getEventPort()->getName()) == 0)
                {
                    event->setCurrentIndex(i);
                    match = true;
                }
            }
        }
    }
    if (!match)
        event->setCurrentIndex(-1);

    connect(event, SIGNAL(currentIndexChanged(QString)), oei, SLOT(setEventPort(QString)));
    addRow(tr("&Event Port:"),event);

    root->addItemsToolbar->addAction(root->actionAddStateAssignment);
    root->addItemsToolbar->addAction(root->actionAddEventOut);
    root->addItemsToolbar->addAction(root->actionAddImpulseOut);
    root->actionDeleteItems->setEnabled(true);
}

void PropertiesManager::createOnImpulseProperties(OnImpulseGraphicsItem *oii)
{
    QLabel *label = new QLabel("<b>Edit On Impulse</b>");
    addRow(label);

    QComboBox *impulse = new QComboBox();
    bool impulse_match = false;
    for (int i=0; i< root->al->ImpulsePortList.size(); i++)
    {
        if (root->al->ImpulsePortList[i]->mode == ImpulseRecvPort){
            impulse->addItem(root->al->ImpulsePortList[i]->getName());
            if (oii->getImpulsePort() != NULL){
                if (root->al->ImpulsePortList[i]->getName().compare(oii->getImpulsePort()->getName()) == 0)
                {
                    impulse->setCurrentIndex(i);
                    impulse_match = true;
                }
            }
        }
    }
    if (!impulse_match)
        impulse->setCurrentIndex(-1);

    connect(impulse, SIGNAL(currentIndexChanged(QString)), oii, SLOT(setImpulsePort(QString)));
    addRow(tr("&Impulse Port:"),impulse);

    root->addItemsToolbar->addAction(root->actionAddStateAssignment);
    root->addItemsToolbar->addAction(root->actionAddEventOut);
    root->addItemsToolbar->addAction(root->actionAddImpulseOut);
    root->actionDeleteItems->setEnabled(true);
}



void PropertiesManager::createStateAssignmentProperties(StateAssignmentTextItem *sa)
{
    //move up down
    root->actionMove_Up->setEnabled(true);
    root->actionMove_Down->setEnabled(true);
    root->actionDeleteItems->setEnabled(true);

    QLabel *label = new QLabel("<b>Edit State Assigment</b>");
    addRow(label);
    QComboBox *var = new QComboBox();
    for (int i=0; i< root->al->StateVariableList.size(); i++)
    {
        var->addItem(root->al->StateVariableList[i]->getName());
        if (sa->getVariable() != NULL){
            if (root->al->StateVariableList[i]->getName().compare(sa->getVariable()->getName()) == 0)
                var->setCurrentIndex(i);
        }
        else
            var->setCurrentIndex(-1);
    }
    connect(var, SIGNAL(currentIndexChanged(QString)), sa, SLOT(setVariable(QString)));
    addRow(tr("&Variable:"),var);

    QLineEdit *maths = new QLineEdit();
    maths->installEventFilter(&eventFilterObject);
    maths->setText(sa->getMaths()->equation);
    connect(maths, SIGNAL(textEdited(QString)), sa, SLOT(setMaths(QString)));
    addRow(tr("&Maths:"),maths);
    addWidget(new QLabel(tr("<i>in physiological units</i>")));

    QStringList errs;
    sa->getMaths()->validateMathInLine(root->al.data(), &errs);

    // sort out errors
    QSettings settings;
    int num_errs = settings.beginReadArray("warnings");
    settings.endArray();

    if (num_errs != 0) {

        // show errors by changing lineedit colour
        QPalette p = maths->palette();
        p.setColor( QPalette::Normal, QPalette::Base, QColor(255, 200, 200) );
        maths->setPalette(p);

        // clear errors
        settings.remove("warnings");

    }
    if (num_errs == 0) {

        // show no errors by changing lineedit colour
        QPalette p = maths->palette();
        p.setColor( QPalette::Normal, QPalette::Base, QColor(255, 255, 255) );
        maths->setPalette(p);

        // clear errors
        settings.remove("warnings");
    }
}

void PropertiesManager::createEventOutProperties(EventOutTextItem *eo)
{
    //move up down
    root->actionMove_Up->setEnabled(true);
    root->actionMove_Down->setEnabled(true);
    root->actionDeleteItems->setEnabled(true);

    QLabel *label = new QLabel("<b>Edit Event Output</b>");
    addRow(label);
    QComboBox *var = new QComboBox();
    for (int i=0; i< root->al->EventPortList.size(); i++)
    {
        if (root->al->EventPortList[i]->mode == EventSendPort){
            var->addItem(root->al->EventPortList[i]->getName());
            if (eo->getEventPort() != NULL){
                if (root->al->EventPortList[i]->getName().compare(eo->getEventPort()->getName()) == 0)
                    var->setCurrentIndex(i);
            }
            else
                var->setCurrentIndex(-1);
        }
    }
    connect(var, SIGNAL(currentIndexChanged(QString)), eo, SLOT(setEventPort(QString)));
    addRow(tr("&Event:"), var);
}

void PropertiesManager::createImpulseOutProperties(ImpulseOutTextItem *io)
{
    //move up down
    root->actionMove_Up->setEnabled(true);
    root->actionMove_Down->setEnabled(true);
    root->actionDeleteItems->setEnabled(true);

    QLabel *label = new QLabel("<b>Edit Impulse Output</b>");
    addRow(label);
    QComboBox *var = new QComboBox();
    for (int i=0; i< root->al->ImpulsePortList.size(); i++)
    {
        if (root->al->ImpulsePortList[i]->mode == ImpulseSendPort){
            if (root->al->ImpulsePortList[i]->parameter != NULL){
                var->addItem(root->al->ImpulsePortList[i]->parameter->getName());
                if (io->getImpulsePort() != NULL){
                    if (io->getImpulsePort()->parameter != NULL)
                        if (root->al->ImpulsePortList[i]->parameter->getName().compare(io->getImpulsePort()->parameter->getName()) == 0)
                            var->setCurrentIndex(i);
                }
                else
                    var->setCurrentIndex(-1);
            }else{
                qDebug() << "Impulse send port has port with no parameter!";
            }
        }
    }
    connect(var, SIGNAL(currentIndexChanged(QString)), io, SLOT(setImpulsePort(QString)));
    addRow(tr("&Impulse:"), var);
}

void PropertiesManager::createAnalogPortProperties(AnalogPortTextItem *ap)
{
    //move up down
    root->actionMove_Up->setEnabled(true);
    root->actionMove_Down->setEnabled(true);
    root->actionDeleteItems->setEnabled(true);

    QLabel *label = new QLabel("<b>Edit Analog Port</b>");
    addRow(label);

    //if mode == send
    if (ap->getPortMode() == AnalogSendPort){
        QComboBox *var = new QComboBox();
        for (int i=0; i< root->al->StateVariableList.size(); i++)
        {
            var->addItem(root->al->StateVariableList[i]->getName());
            if (ap->getVariable() != NULL){
                if (root->al->StateVariableList[i]->getName().compare(ap->getVariable()->getName()) == 0)
                    var->setCurrentIndex(i);
            }
        }
        for (int i=0; i< root->al->AliasList.size(); i++)
        {
            var->addItem(root->al->AliasList[i]->getName());
            if (ap->getVariable() != NULL){
                if (root->al->AliasList[i]->getName().compare(ap->getVariable()->getName()) == 0)
                    var->setCurrentIndex(i+root->al->StateVariableList.size());
            }
        }
        if (ap->getVariable() == NULL)
            var->setCurrentIndex(-1);
        connect(var, SIGNAL(currentIndexChanged(QString)), ap, SLOT(setVariable(QString)));
        addRow(tr("&Variable:"),var);
    }
    //otherwise
    else
    {

        QLineEdit *name = new QLineEdit();
        name->installEventFilter(&eventFilterObject);
        name->setText(ap->getName());
        name->setValidator(validator);
        connect(name, SIGNAL(textEdited(QString)), ap, SLOT(setName(QString)));
        addRow(tr("&Name:"),name);
    }

    QComboBox *mode = new QComboBox();
    mode->addItem("Send");
    mode->addItem("Receive");
    mode->addItem("Reduce");
    AnalogPortMode m = ap->getPortMode();
    switch(m){
    case(AnalogSendPort):
        mode->setCurrentIndex(0);
        break;
    case(AnalogRecvPort):
        mode->setCurrentIndex(1);
        break;
    case(AnalogReducePort):
        mode->setCurrentIndex(2);
        break;
    }
    connect(mode, SIGNAL(currentIndexChanged(QString)), ap, SLOT(setPortMode(QString)));
    addRow(tr("Analog &Mode"), mode);

    if(m == AnalogReducePort){
        QComboBox *reduce = new QComboBox();
        reduce->addItem("None");
        reduce->addItem("Addition");
        ReduceOperation r = ap->getPortReduceOp();
        switch(r){
        case(ReduceOperationNone):
            reduce->setCurrentIndex(0);
            break;
        case(ReduceOperationAddition):
            reduce->setCurrentIndex(1);
            break;
        }
        connect(reduce, SIGNAL(currentIndexChanged(QString)), ap, SLOT(setPortReduceOp(QString)));
        addRow(tr("&Reduce Operation:"), reduce);
    }

    if ((m == AnalogRecvPort)||(m == AnalogReducePort)){
        QComboBox *dims_prefix = getPrefixCombo(ap->port->dims->getPrefix());
        connect(dims_prefix, SIGNAL(currentIndexChanged(QString)), ap, SLOT(setDimsPrefix(QString)));
        addRow(tr("Dimensionality &Prefix:"),dims_prefix);

        QComboBox *dims_unit = getUnitCombo(ap->port->dims->getUnit());
        connect(dims_unit, SIGNAL(currentIndexChanged(QString)), ap, SLOT(setDimsUnit(QString)));
        addRow(tr("Dimensionality &Unit:"),dims_unit);
    }
}

void PropertiesManager::createEventPortProperties(EventPortTextItem *ep)
{
    //move up down
    root->actionMove_Up->setEnabled(true);
    root->actionMove_Down->setEnabled(true);
    root->actionDeleteItems->setEnabled(true);

    QLabel *label = new QLabel("<b>Edit Event Port</b>");
    addRow(label);

    QComboBox *mode = new QComboBox();
    mode->addItem("Send");
    mode->addItem("Receive");
    EventPortMode m = ep->getPortMode();
    switch(m){
    case(EventSendPort):{
        mode->setCurrentIndex(0);
        break;
    }
    case(EventRecvPort):{
        mode->setCurrentIndex(1);
        break;
    }
    default:{
        mode->setCurrentIndex(-1);
    }
    }
    connect(mode, SIGNAL(currentIndexChanged(QString)), ep, SLOT(setPortMode(QString)));
    addRow(tr("Event &Mode:"), mode);

    QLineEdit *name = new QLineEdit();
    name->installEventFilter(&eventFilterObject);
    name->setText(ep->getName());
    name->setValidator(validator);
    connect(name, SIGNAL(textEdited(QString)), ep, SLOT(setName(QString)));
    addRow(tr("&Name:"),name);
}

void PropertiesManager::createImpulsePortProperties(ImpulsePortTextItem *ip)
{
    //move up down
    root->actionMove_Up->setEnabled(true);
    root->actionMove_Down->setEnabled(true);
    root->actionDeleteItems->setEnabled(true);

    QLabel *label = new QLabel("<b>Edit Impulse Port</b>");
    addRow(label);

    ImpulsePortMode m = ip->getPortMode();

    //if mode == send
    if (m == ImpulseSendPort){
        QComboBox *param = new QComboBox();
        for (int i=0; i< root->al->ParameterList.size(); i++)
        {
            param->addItem(root->al->ParameterList[i]->getName());
            if (ip->getParameter() != NULL){
                if (root->al->ParameterList[i]->getName().compare(ip->getParameter()->getName()) == 0)
                    param->setCurrentIndex(i);
            }
        }
        for (int i=0; i< root->al->StateVariableList.size(); i++)
        {
            param->addItem(root->al->StateVariableList[i]->getName());
            if (ip->getParameter() != NULL){
                if (root->al->StateVariableList[i]->getName().compare(ip->getParameter()->getName()) == 0)
                    param->setCurrentIndex(i);
            }
        }
        for (int i=0; i< root->al->AliasList.size(); i++)
        {
            param->addItem(root->al->AliasList[i]->getName());
            if (ip->getParameter() != NULL){
                if (root->al->AliasList[i]->getName().compare(ip->getParameter()->getName()) == 0)
                    param->setCurrentIndex(i);
            }
        }
        if (ip->getParameter() == NULL)
            param->setCurrentIndex(-1);
        connect(param, SIGNAL(currentIndexChanged(QString)), ip, SLOT(setParameter(QString)));
        addRow(tr("&Par/Var:"),param);
    }
    //otherwise m == Recv
    else
    {
        QLineEdit *name = new QLineEdit();
        name->installEventFilter(&eventFilterObject);
        name->setText(ip->getName());
        name->setValidator(validator);
        connect(name, SIGNAL(textEdited(QString)), ip, SLOT(setName(QString)));
        addRow(tr("&Name:"),name);
    }

    QComboBox *mode = new QComboBox();
    mode->addItem("Send");
    mode->addItem("Receive");
    switch(m){
    case(ImpulseSendPort):
        mode->setCurrentIndex(0);
        break;
    case(ImpulseRecvPort):
        mode->setCurrentIndex(1);
        break;
    }
    connect(mode, SIGNAL(currentIndexChanged(QString)), ip, SLOT(setPortMode(QString)));
    addRow(tr("Impulse &Mode"), mode);

    if (m == ImpulseRecvPort){
        QComboBox *dims_prefix = getPrefixCombo(ip->port->dims->getPrefix());
        connect(dims_prefix, SIGNAL(currentIndexChanged(QString)), ip, SLOT(setDimsPrefix(QString)));
        addRow(tr("Dimensionality &Prefix:"),dims_prefix);

        QComboBox *dims_unit = getUnitCombo(ip->port->dims->getUnit());
        connect(dims_unit, SIGNAL(currentIndexChanged(QString)), ip, SLOT(setDimsUnit(QString)));
        addRow(tr("Dimensionality &Unit:"),dims_unit);
    }

}
