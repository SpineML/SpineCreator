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

#include "nineml_rootcomponentitem.h"
#include "nineml_graphicsitems.h"
#include "propertiesmanager.h"
#include "nineml_alscene.h"
#include "rootdata.h"
#include "undocommands.h"

RootComponentItem::RootComponentItem(MainWindow *main,Ui::MainWindow *ui, QFile *file)
{
    this->ui =ui;
    this->main = main;

    init();

    al = QSharedPointer<NineMLComponent>(new NineMLComponent());
    //load file if one is provided (otherwise create blank doc)
    if (file != NULL)
    {
        QDomDocument doc("SpineML");
        doc.setContent(file);
        al->load(&doc);
    }
    else
    {
        al->name = "New Component";
        al->type = "neuron_body";
    }
    alPtr.clear();

    //initialise the scene
    scene->initialiseScene(al);
}

RootComponentItem::RootComponentItem(MainWindow *main,Ui::MainWindow *ui, QSharedPointer<NineMLComponent>component)
{
    this->ui =ui;
    this->main = main;

    init();

    al = QSharedPointer<NineMLComponent>(new NineMLComponent(component));
    alPtr.clear();

    //initialise the scene
    scene->initialiseScene(al);
}


RootComponentItem::~RootComponentItem()
{
    //any left over data
    delete properties;
    properties = NULL;
    delete scene;
    scene = NULL;
    delete gvlayout;
    gvlayout = NULL;
    al.clear();
    alPtr.clear();
    delete actionSelectMode;
    actionSelectMode = NULL;
    delete actionAddOnCondition;
    actionAddOnCondition = NULL;
    delete actionAddOnEvent;
    actionAddOnEvent = NULL;
    delete actionLayout;
    actionAddOnImpulse = NULL;
    delete actionAddOnImpulse;
    actionLayout = NULL;
    delete actionDeleteItems;
    actionDeleteItems = NULL;
    delete actionMove_Up;
    actionMove_Up = NULL;
    delete actionMove_Down;
    actionMove_Down = NULL;
    delete actionShowHidePorts;
    actionShowHidePorts = NULL;
    delete actionShowHideParams;
    actionShowHideParams = NULL;
    delete actionZoomIn;
    actionZoomIn = NULL;
    delete actionZoonOut;
    actionZoonOut = NULL;

    delete actionAddTimeDerivative;
    actionAddTimeDerivative = NULL;
    delete actionAddParameter;
    actionAddParameter = NULL;
    delete actionAddStateVariable;
    actionAddStateVariable = NULL;
    delete actionAddAlias;
    actionAddAlias = NULL;
    delete actionAddAnalogePort;
    actionAddAnalogePort = NULL;
    delete actionAddEventPort;
    actionAddEventPort = NULL;
    delete actionAddEventOut;
    actionAddEventOut = NULL;
    delete actionAddStateAssignment;
    actionAddStateAssignment = NULL;
    delete actionAddRegime;
    actionAddRegime = NULL;

    toolbar->deleteLater();
    addItemsToolbar->deleteLater();
}



void RootComponentItem::setSelectionMode(ALSceneMode mode)
{
    scene->clearSelection();
    switch(mode){
    case(ModeSelect):{
        clearSelection();
        actionSelectMode->setChecked(true);
        actionDeleteItems->setChecked(false);
        actionAddOnCondition->setChecked(false);
        actionAddOnEvent->setChecked(false);
        actionAddOnImpulse->setChecked(false);
        main->viewCL.display->setCursor(Qt::ArrowCursor);
        break;
    }
    case(ModeInsertOnCondition):{
        actionAddOnCondition->setChecked(true);
        actionAddOnImpulse->setChecked(false);
        actionDeleteItems->setChecked(false);
        actionSelectMode->setChecked(false);
        actionAddOnEvent->setChecked(false);
        main->viewCL.display->setCursor(Qt::ArrowCursor);
        break;
    }
    case(ModeInsertOnEvent):{
        actionAddOnEvent->setChecked(true);
        actionAddOnImpulse->setChecked(false);
        actionDeleteItems->setChecked(false);
        actionSelectMode->setChecked(false);
        actionAddOnCondition->setChecked(false);
        main->viewCL.display->setCursor(Qt::ArrowCursor);
        break;
    }
    case(ModeInsertOnImpulse):{
        actionAddOnImpulse->setChecked(true);
        actionAddOnEvent->setChecked(false);
        actionDeleteItems->setChecked(false);
        actionSelectMode->setChecked(false);
        actionAddOnCondition->setChecked(false);
        main->viewCL.display->setCursor(Qt::ArrowCursor);
        break;
    }
    }
    scene->setMode(mode);
}

void RootComponentItem::clearSelection()
{
    properties->clear();
    scene->clearSelection();
}

void RootComponentItem::notifyDataChange()
{
    // Update the component in the component library in root data with this line:
    if (this->alPtr != NULL && this->al != NULL) {
        this->alPtr->updateFrom (this->al);
    }
    emit unsavedChange(true);
}


void RootComponentItem::requestLayoutUpdate()
{
    gvlayout->updateLayout();
}

void RootComponentItem::setComponentClassName(QString name)
{
    QSharedPointer<NineMLComponent> oldComponent = QSharedPointer<NineMLComponent>(new NineMLComponent(al));
    // find which catalog we are saving to
    vector < QSharedPointer<NineMLComponent> > * curr_lib;
    if (al->type == "neuron_body")
        curr_lib = &main->data.catalogNrn;
    if (al->type == "weight_update")
        curr_lib = &main->data.catalogWU;
    if (al->type == "postsynapse")
        curr_lib = &main->data.catalogPS;
    if (al->type == "generic_component")
        curr_lib = &main->data.catalogUnsorted;

    bool unique = true;
    // check if name is unique
    for (uint i = 0; i < curr_lib->size(); ++i) {
        if ((*curr_lib)[i]->name == name) {
            qDebug() << name << " is not unique in the component catalog.";
            unique = false;
        }
    }

    // find unique name
    int val = -1;
    if (!unique) {
        val = 1;
        // see if there is a component with the same name
        while (!unique) {
            unique = true;
            for (uint i = 0; i < curr_lib->size(); ++i) {
                if ((*curr_lib)[i]->name == name + QString::number(float(val))) {
                    unique = false;
                }
            }
            if (!unique) {
                ++val;
            }
        }
    }

    if (val != -1) {
        qDebug() << "Adding extra suffix to name to make it unique";
        name += " " + QString::number(val);
    }

    // Update the name in the current component.
    al->name = name;

    notifyDataChange();

    if (qobject_cast < QLineEdit *> (sender())) {
        alPtr->undoStack.push(new changeComponent(this, oldComponent, "Set Component name"));
    } else {
        oldComponent.clear();
    }

    main->updateTitle();
}

void RootComponentItem::setComponentClassType(QString type)
{
    QSharedPointer<NineMLComponent> oldComponent = QSharedPointer<NineMLComponent>(new NineMLComponent(al));
    QString oldType = al->type;
    al->type = type;

    vector <QSharedPointer<NineMLComponent> > * old_lib;
    vector <QSharedPointer<NineMLComponent> > * new_lib;

    // pointer to source catalog
    if (oldType == "neuron_body")
        old_lib = &main->data.catalogNrn;
    if (oldType == "weight_update")
        old_lib = &main->data.catalogWU;
    if (oldType == "postsynapse")
        old_lib = &main->data.catalogPS;
    if (oldType == "generic_component")
        old_lib = &main->data.catalogUnsorted;

    // pointer to dest catalog
    if (type == "neuron_body")
        new_lib = &main->data.catalogNrn;
    if (type == "weight_update")
        new_lib = &main->data.catalogWU;
    if (type == "postsynapse")
        new_lib = &main->data.catalogPS;
    if (type == "generic_component")
        new_lib = &main->data.catalogUnsorted;

    notifyDataChange();
    if (qobject_cast < QComboBox *> (sender()))
        alPtr->undoStack.push(new changeComponentType(this, old_lib, new_lib, alPtr, "Set Component class type"));
    else
        oldComponent.clear();
}

void RootComponentItem::setInitialRegime(QString regime)
{
    QSharedPointer<NineMLComponent> oldComponent = QSharedPointer<NineMLComponent> (new NineMLComponent(al));
    for (uint i=0; i< al->RegimeList.size(); i++){
        if (al->RegimeList[i]->name == regime){
            al->initial_regime = al->RegimeList[i];
            al->initial_regime_name = regime;
        }
    }
    if (al->initial_regime) {
        emit initialRegimeChanged();
    }
    notifyDataChange();
    if (qobject_cast < QComboBox *> (sender())) {
        alPtr->undoStack.push(new changeComponent(this, oldComponent, "Set Component Initial Regime"));
    } else {
        oldComponent.clear();
    }
}

void RootComponentItem::setPath(QString component_path)
{
    QSharedPointer<NineMLComponent> oldComponent = QSharedPointer<NineMLComponent> (new NineMLComponent(al));
    al->path = component_path;
    if (qobject_cast < QComboBox *> (sender()))
        alPtr->undoStack.push(new changeComponent(this, oldComponent, "Set Component path"));
    else
        oldComponent.clear();
}

void RootComponentItem::validateAndStore()
{
    emit validateAndStoreSignal();
    properties->clear();
    properties->createEmptySelectionProperties();
}

void RootComponentItem::deleteComponent()
{
    if (alPtr == NULL)
        return;

    // check if component is referenced:
    if (rootDataPtr->isComponentInUse(alPtr)) {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Warning);
        if (al->path == "temp" || al->path == "model")
            msgBox.setText("Cannot close: component is in use in network");
        else
            msgBox.setText("Cannot delete: component is in use in network");
        msgBox.exec();
    } else {
        if (alPtr->type == "neuron_body") {
            for (uint i = 0; i < rootDataPtr->catalogNrn.size(); ++i)
                if (rootDataPtr->catalogNrn[i] == alPtr)
                    rootDataPtr->catalogNrn.erase(rootDataPtr->catalogNrn.begin()+i);
        } else if (alPtr->type == "weight_update") {
            for (uint i = 0; i < rootDataPtr->catalogWU.size(); ++i)
                if (rootDataPtr->catalogWU[i] == alPtr)
                    rootDataPtr->catalogWU.erase(rootDataPtr->catalogWU.begin()+i);
        } else if (alPtr->type == "postsynapse") {
            for (uint i = 0; i < rootDataPtr->catalogPS.size(); ++i)
                if (rootDataPtr->catalogPS[i] == alPtr)
                    rootDataPtr->catalogPS.erase(rootDataPtr->catalogPS.begin()+i);
        } else //generic_component
            for (uint i = 0; i < rootDataPtr->catalogUnsorted.size(); ++i)
                if (rootDataPtr->catalogUnsorted[i] == alPtr)
                    rootDataPtr->catalogUnsorted.erase(rootDataPtr->catalogUnsorted.begin()+i);


        /*delete scene;
        delete gvlayout;
        delete al;
        alPtr = NULL;

        gvlayout = new GVLayout();
        scene = new NineMLALScene(this);*/

        //initialise the scene
        /*scene->initialiseScene();
        this->main->viewCL.display->setScene(scene);
        gvlayout->updateLayout();
        properties->clear();
        properties->createEmptySelectionProperties();*/
    }

    // update the file list
    main->viewCL.fileList->disconnect();
    main->addComponentsToFileList();
    connect(main->viewCL.fileList, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), main, SLOT(fileListItemChanged(QListWidgetItem*,QListWidgetItem*)));

    // update list in NL editor
    main->viewNL.layout->updatePanel(&main->data);

    // go down
    main->viewCL.root = NULL;
    delete this;

}

void RootComponentItem::init()
{

    // add actions and toolbars:
    toolbar = new QToolBar("Component main toolbar");
    main->viewCL.subWin->addToolBar(Qt::TopToolBarArea,toolbar);
#ifndef Q_OS_MAC
    toolbar->setStyleSheet(main->toolbarStyleSheet);
#endif
    toolbar->setMaximumHeight(32);
    toolbar->layout()->setMargin(0);

    addItemsToolbar = new QToolBar("Component additional toolbar");
    main->viewCL.subWin->addToolBar( Qt::TopToolBarArea, addItemsToolbar);
#ifndef Q_OS_MAC
    addItemsToolbar->setStyleSheet(main->toolbarStyleSheet);
#endif
    addItemsToolbar->setMaximumHeight(32);
    addItemsToolbar->layout()->setMargin(0);

    properties = new PropertiesManager(this);
    gvlayout = new GVLayout();
    scene = new NineMLALScene(this);

    actionSelectMode = new QAction(QIcon(":/icons/toolbar/images/cursor.png"),"",main);
    actionSelectMode->setCheckable(true);
    actionSelectMode->setChecked(true);
    actionSelectMode->setToolTip("Select Mode: Select items in the viewport");
    actionAddOnCondition = new QAction(QIcon(":/icons/toolbar/images/add_oncond.png"), "",main);
    actionAddOnCondition->setCheckable(true);
    actionAddOnCondition->setChecked(false);
    actionAddOnCondition->setToolTip("OnCondition Add Mode: Drag between Regimes to add OnCondition");
    actionAddOnEvent = new QAction(QIcon(":/icons/toolbar/images/add_onev.png"), "",main);
    actionAddOnEvent->setCheckable(true);
    actionAddOnEvent->setChecked(false);
    actionAddOnEvent->setToolTip("OnEvent Add Mode: Drag between Regimes to add OnEvent");
    actionAddOnImpulse = new QAction(QIcon(":/icons/toolbar/images/add_onimp.png"), "",main);
    actionAddOnImpulse->setCheckable(true);
    actionAddOnImpulse->setChecked(false);
    actionAddOnImpulse->setToolTip("OnImpulse Add Mode: Drag between Regimes to add OnImpulse");
    actionLayout = new QAction(QIcon(":/icons/toolbar/images/layout-icon.png"), "",main);
    actionDeleteItems = new QAction(QIcon(":/icons/toolbar/delShad.png"), "",main);
    actionDeleteItems->setToolTip("Delete selected items");
    actionMove_Up = new QAction(QIcon(":/icons/toolbar/up.png"),"",main);
    actionMove_Up->setToolTip("Move selected item up in list");
    actionMove_Down = new QAction(QIcon(":/icons/toolbar/down.png"),"",main);
    actionMove_Down->setToolTip("Move selected item down in list");
    actionShowHidePorts = new QAction(QIcon(":/icons/toolbar/images/sh-ports-icon.png"),"",main);
    actionShowHidePorts->setCheckable(true);
    actionShowHidePorts->setChecked(true);
    actionShowHidePorts->setToolTip("Show/hide Ports box");
    actionShowHideParams = new QAction(QIcon(":/icons/toolbar/images/sh-param-icon.png"),"",main);
    actionShowHideParams->setCheckable(true);
    actionShowHideParams->setChecked(true);
    actionShowHideParams->setToolTip("Show/hide Params, Vars & Alias box");
    actionZoomIn = new QAction(QIcon(":/icons/toolbar/images/zoom_in.png"),"",main);
    actionZoomIn->setToolTip("Zoom viewport in");
    actionZoonOut = new QAction(QIcon(":/icons/toolbar/images/zoom_out.png"),"",main);
    actionZoonOut->setToolTip("Zoom viewport out");

    connect(actionSelectMode, SIGNAL(triggered()), main, SLOT(actionSelectMode_triggered()));
    connect(actionAddOnCondition, SIGNAL(triggered()), main, SLOT(actionAddOnCondition_triggered()));
    connect(actionAddOnEvent, SIGNAL(triggered()), main, SLOT(actionAddOnEvent_triggered()));
    connect(actionAddOnImpulse, SIGNAL(triggered()), main, SLOT(actionAddOnImpulse_triggered()));
    //connect(actionLayout, SIGNAL(triggered()), main, SLOT(requestLayoutUpdate()));
    connect(actionDeleteItems, SIGNAL(triggered()), main, SLOT(actionDeleteItems_triggered()));
    connect(actionMove_Up, SIGNAL(triggered()), main, SLOT(actionMove_Up_triggered()));
    connect(actionMove_Down, SIGNAL(triggered()), main, SLOT(actionMove_Down_triggered()));
    connect(actionShowHidePorts, SIGNAL(toggled(bool)), main, SLOT(actionShowHidePorts_triggered(bool)));
    connect(actionShowHideParams, SIGNAL(toggled(bool)), main, SLOT(actionShowHideParams_triggered(bool)));
    connect(actionZoomIn, SIGNAL(triggered()), main, SLOT(actionZoomIn_triggered()));
    connect(actionZoonOut, SIGNAL(triggered()), main, SLOT(actionZoomOut_triggered()));

    toolbar->addAction(actionSelectMode);
    toolbar->addAction(actionAddOnCondition);
    toolbar->addAction(actionAddOnEvent);
    toolbar->addAction(actionAddOnImpulse);
    toolbar->addSeparator();
    //toolbar->addAction(actionLayout);
    toolbar->addAction(actionDeleteItems);
    toolbar->addAction(actionMove_Up);
    toolbar->addAction(actionMove_Down);
    toolbar->addSeparator();
    toolbar->addAction(actionShowHidePorts);
    toolbar->addAction(actionShowHideParams);
    toolbar->addSeparator();
    toolbar->addAction(actionZoomIn);
    toolbar->addAction(actionZoonOut);
    toolbar->addSeparator();

    actionAddTimeDerivative = new QAction(QIcon(":/icons/toolbar/images/derivative-icon.png"),"",main);
    actionAddTimeDerivative->setToolTip("Add a TimeDerivative to the selected Regime");
    actionAddParameter = new QAction(QIcon(":/icons/toolbar/images/param-icon.png"),"",main);
    actionAddParameter->setToolTip("Add a Parameter");
    actionAddStateVariable = new QAction(QIcon(":/icons/toolbar/images/statevar-icon.png"),"",main);
    actionAddStateVariable->setToolTip("Add a State Variable");
    actionAddAlias = new QAction(QIcon(":/icons/toolbar/images/alias-icon.png"),"",main);
    actionAddAlias->setToolTip("Add an Alias");
    actionAddAnalogePort = new QAction(QIcon(":/icons/toolbar/images/analogport-icon.png"),"",main);
    actionAddAnalogePort->setToolTip("Add an Analog Port");
    actionAddEventPort = new QAction(QIcon(":/icons/toolbar/images/eventport-icon.png"),"",main);
    actionAddEventPort->setToolTip("Add an Event Port");
    actionAddImpulsePort = new QAction(QIcon(":/icons/toolbar/images/impulseport-icon.png"),"",main);
    actionAddImpulsePort->setToolTip("Add an Impulse Port");
    actionAddEventOut = new QAction(QIcon(":/icons/toolbar/images/emitevent-icon.png"),"",main);
    actionAddEventOut->setToolTip("Add an EventOut to the current Transition");
    actionAddImpulseOut = new QAction(QIcon(":/icons/toolbar/images/emitimpulse-icon.png"),"",main);
    actionAddImpulseOut->setToolTip("Add an ImpulseOut to the current Transition");
    actionAddStateAssignment = new QAction(QIcon(":/icons/toolbar/images/stateassignment-icon.png"),"",main);
    actionAddStateAssignment->setToolTip("Add an State Assignment to the current Transition");
    actionAddRegime = new QAction(QIcon(":/icons/toolbar/images/regime-icon.png"),"",main);
    actionAddRegime->setToolTip("Add a Regime to the Component");

    connect(actionAddTimeDerivative, SIGNAL(triggered()), main, SLOT(actionAddTimeDerivative_triggered()));
    connect(actionAddParameter, SIGNAL(triggered()), main, SLOT(actionAddParamater_triggered()));
    connect(actionAddStateVariable, SIGNAL(triggered()), main, SLOT(actionAddStateVariable_triggered()));
    connect(actionAddAlias, SIGNAL(triggered()), main, SLOT(actionAddAlias_triggered()));
    connect(actionAddAnalogePort, SIGNAL(triggered()), main, SLOT(actionAddAnalogePort_triggered()));
    connect(actionAddEventPort, SIGNAL(triggered()), main, SLOT(actionAddEventPort_triggered()));
    connect(actionAddImpulsePort, SIGNAL(triggered()), main, SLOT(actionAddImpulsePort_triggered()));
    connect(actionAddEventOut, SIGNAL(triggered()), main, SLOT(actionAddEventOut_triggered()));
    connect(actionAddImpulseOut, SIGNAL(triggered()), main, SLOT(actionAddImpulseOut_triggered()));
    connect(actionAddStateAssignment, SIGNAL(triggered()), main, SLOT(actionAddStateAssignment_triggered()));
    connect(actionAddRegime, SIGNAL(triggered()), main, SLOT(actionAddRegime_triggered()));
}







