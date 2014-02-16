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

#ifndef NINEML_ROOTCOMPONENTITEM_H
#define NINEML_ROOTCOMPONENTITEM_H

#include "gvitems.h"
#include "nineML_classes.h"
#include "nineml_graphicsitems.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

enum ALSceneMode { ModeSelect, ModeInsertOnCondition, ModeInsertOnEvent, ModeInsertOnImpulse };


class PropertiesManager;
class NineMLALScene;
class rootData;

class RootComponentItem: public QObject
{
    Q_OBJECT
public:
    RootComponentItem(MainWindow *main,Ui::MainWindow *ui, QFile *file=0);
    RootComponentItem(MainWindow *main,Ui::MainWindow *ui, NineMLComponent *);
    ~RootComponentItem();

    void setSelectionMode(ALSceneMode mode);
    void clearSelection();
    void notifyDataChange();
    QAction * actionSelectMode;
    QToolBar * addItemsToolbar;
    QToolBar * toolbar;
    QAction * actionAddOnCondition;
    QAction * actionAddOnEvent;
    QAction * actionAddOnImpulse;
    QAction * actionLayout;
    QAction * actionDeleteItems;
    QAction * actionMove_Up;
    QAction * actionMove_Down;
    QAction * actionShowHidePorts;
    QAction * actionShowHideParams;
    QAction * actionZoomIn;
    QAction * actionZoonOut;
    QAction * actionAddRegime;

    QAction * actionAddTimeDerivative;
    QAction * actionAddParameter;
    QAction * actionAddStateVariable;
    QAction * actionAddAlias;
    QAction * actionAddAnalogePort;
    QAction * actionAddEventPort;
    QAction * actionAddImpulsePort;
    QAction * actionAddEventOut;
    QAction * actionAddImpulseOut;
    QAction * actionAddStateAssignment;
    rootData * rootDataPtr;
    NineMLComponent * alPtr;


public slots:
    void requestLayoutUpdate();
    void setComponentClassName(QString name);
    void setComponentClassType(QString type);
    void setInitialRegime(QString regime);
    void setPath(QString component_path);
    void validateAndStore();
    void deleteComponent();

public:
    NineMLComponent *al;
    PropertiesManager *properties;
    NineMLALScene *scene;
    Ui::MainWindow *ui;
    MainWindow *main;
    GVLayout *gvlayout;

private:
    void init();

signals:
    void unsavedChange(bool unsaved);
    void initialRegimeChanged();
    void validateAndStoreSignal();
};

#endif // NINEML_ROOTCOMPONENTITEM_H
