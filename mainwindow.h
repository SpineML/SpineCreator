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
**          Authors: Alex Cope, Seb James                                 **
**  Website/Contact: http://bimpa.group.shef.ac.uk/                       **
****************************************************************************/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "globalHeader.h"

#include "CL_classes.h"
#include "SC_connectionmodel.h"
#include "SC_network_3d_visualiser_panel.h"
#include "SC_network_layer_rootdata.h"
#include "SC_network_layer_rootlayout.h"
#include "SC_layout_aliaseditdialog.h"
#include "SC_layout_editpreviewdialog.h"
#include "SC_viewELexptpanelhandler.h"
#include "SC_viewVZlayoutedithandler.h"
#include "SC_viewGVpropertieslayout.h"
#include "SC_component_view.h"
#include "SC_versioncontrol.h"

/*!
 * paths used to store the last used directory for file open/save
 * dialogs in QSettings.
 */
//@{
#define MAINWINDOW_LASTPROJECTDIR   "mainwindow/lastProjectDirectory"
#define MAINWINDOW_LASTNETWORKDIR   "mainwindow/lastNetworkDirectory"
#define MAINWINDOW_LASTCOMPONENTDIR "mainwindow/lastComponentDirectory"
#define MAINWINDOW_LASTLAYOUTDIR    "mainwindow/lastLayoutDirectory"
//@}

namespace Ui {
    class MainWindow;
}

struct viewGVstruct {
    QMainWindow * subWin;
    QMdiArea * mdiarea;
    QDockWidget * dock;
    QToolBar * toolbar;
    MainWindow * mainwindow;
    viewGVpropertieslayout * properties;
};

struct viewELstruct {
    QFrame * view;
    QWidget * selectionPanel;
    QWidget * panel;
    QWidget * expt;
    QScrollArea * propertiesScrollArea;
};

struct viewNLstruct {
    nl_rootlayout * layout;
};

struct viewVZstruct {
    QSplitter * view;
    csv_connectionModel * connMod;
    QTableView * connTable;
    QWidget * panel;
    glConnectionWidget * OpenGLWidget;
    QTreeView * treeView;
    QLabel * mathOut;
    QSharedPointer<NineMLLayout> layout;
    QSharedPointer<systemObject> currObject;
    QSharedPointer<NineMLLayout> editLayout;
    QLabel * errors;
    systemmodel * sysModel;
    QFrame * toolbar;
};

struct viewCLstruct {
     RootComponentItem * root;
     ALView * display;
     QToolBar * toolBar;
     QDockWidget * dock;
     QFrame * frame;
     QHBoxLayout * layout;
     QWidget * propertiesContents;
     QListWidget * fileList;
     QVBoxLayout * fileListLayout;
     QPushButton * toggleFileBox;
     MainWindow * mainWindow;
     QMainWindow * subWin;
};


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    nl_rootdata data;
    QUndoGroup * undoStacks;
    viewGVstruct viewGV;
    viewELstruct viewEL;
    viewNLstruct viewNL;
    viewVZstruct viewVZ;
    viewCLstruct viewCL;
    viewELExptPanelHandler * viewELhandler;
    viewVZLayoutEditHandler * viewVZhandler;
    QShortcut * deleteShortcut;
    void addComponentsToFileList();
    QString toolbarStyleSheet;
    void setProjectMenu();

private:
    Ui::MainWindow *ui;
    GLWidget *glWidget;
    void createActions();
    bool unsaved_changes;
#ifdef Q_OS_MAC111
    QTimer fix;
#endif
    /*!
     * If an error message needs to be displayed, do so with this.
     */
    QErrorMessage* emsg;

    QAction *undoAction;
    QAction *redoAction;
    QDomDocument tempDoc;
    void initViewEL();
    void connectViewEL();
    void initViewGV();
    void connectViewGV();
    void initViewCL();
    void connectViewCL();
    void initViewVZ();
    void connectViewVZ();
    void configureVCSMenu();
    bool isChanged();
    bool promptToSave();
    void clearComponents();

public slots:
    void import_project();
    void import_recent_project();
    void clear_recent_projects();
    void export_project();
    void export_project_as();
    void close_project();
    void import_network();
    void export_network();
    void import_component();
    void export_component();
    void duplicate_component();
    void delete_component();
    void import_layout();
    void export_layout();
    void import_csv();
    void viewGVshow();
    void viewELshow();
    void viewNLshow();
    void viewVZshow();
    void viewCLshow();
    void setCaption(QString);
    void new_project();
    void saveImageAction();
    void launchSimulatorEditor();
    void initialiseModel(QSharedPointer<Component>);
    void updateNetworkButtons(nl_rootdata *);
    void undoOrRedoPerformed(int);

    // AL editor slots
    void actionAs_Image_triggered();
    void actionAs_Dotty_Graph_triggered();
    void actionNew_triggered();

    void actionSelectMode_triggered();
    void actionShowHideParams_triggered(bool checked);
    void actionShowHidePorts_triggered(bool checked);
    void actionMove_Up_triggered();
    void actionMove_Down_triggered();
    void actionDeleteItems_triggered();
    void actionZoomIn_triggered();
    void actionZoomOut_triggered();

    void actionAddParamater_triggered();
    void actionAddRegime_triggered();
    void actionAddOnCondition_triggered();
    void actionAddTimeDerivative_triggered();
    void actionAddAnalogePort_triggered();
    void actionAddEventPort_triggered();
    void actionAddImpulsePort_triggered();
    void actionAddStateVariable_triggered();
    void actionAddAlias_triggered();
    void actionAddStateAssignment_triggered();
    void actionAddEventOut_triggered();
    void actionAddImpulseOut_triggered();
    void actionAddOnEvent_triggered();
    void actionAddOnImpulse_triggered();
    void actionDuplicate_experiment_triggered();
    void actionDelete_experiment_triggered();
    void about();

    // versioning
    void actionCommitModel_triggered();
    void actionUpdateModel_triggered();
    void actionRevertModel_triggered();
    void actionRepStatus_triggered();
    void actionRepLog_triggered();
    void actionRescanVCS_triggered();

    void updateTitle(bool);
    void updateTitle();

    void fileListToggle();
    void fileListItemChanged(QListWidgetItem *current, QListWidgetItem *);
#ifdef Q_OS_MAC111
    // init gl for osx
    void osxHack();
#endif

signals:
    void import_component_xml(QStringList fileNames);
    void import_layout_xml(QStringList fileNames);
    void import_model_xml(QString fileName);
    void export_component_xml(QString fileName, QSharedPointer<Component> component);
    void export_layout_xml(QString fileName, QSharedPointer<NineMLLayout> component);
    void export_model_xml(QString fileName);
    void import_csv_signal(QString fileName);
    void launchComponentSorter();
    void mathText(QString);
    void statusBarUpdate(QString, int);
    void updatePanel(nl_rootdata *);
    void saveImage();

protected:
    void closeEvent(QCloseEvent *event);

    /*!
     * Update the recent projects list, storing it with QSettings,
     * then re-generate the menu with setupRecentProjectsMenu.
     *
     * This also saves the mainwindow/lastDirectory QSetting, which is
     * used as the starting point for load/save project dialogs.
     */
    void updateRecentProjects(const QString& filePath);

    /*!
     * Generate the recent projects menu.
     */
    void setupRecentProjectsMenu(QSettings* settings);

    /*!
     * The maximum number of recent files to show in the recent files menu.
     */
    const int maxRecentFiles;

    /*!
     * The guts of the import_project slot. TODO Make this an overload of import_project.
     */
    void import_project(const QString& filePath);

    /*!
     * Export the project at filePath. Shared code between
     * export_project and export_project_as slots.
     */
    void export_project(const QString& filePath);

    /*!
     * Get the last directory for the passed-in settings path
     * (e.g. mainwindow/lastProjectDirectory). If this is empty,
     * return the current user's HOME.
     */
    QString getLastDirectory (const QString& settingsPath);
};

#endif // MAINWINDOW_H
