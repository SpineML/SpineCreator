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
**          Authors: Alex Cope, Paul Richmond, Seb James                  **
**  Website/Contact: http://bimpa.group.shef.ac.uk/                       **
****************************************************************************/

#ifdef _DEBUG
  #undef _DEBUG
  #include <Python.h>
  #define _DEBUG
#else
  #include <Python.h>
#endif

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "SC_settings.h"
#include "SC_component_rootcomponentitem.h"
#include "SC_component_propertiesmanager.h"
#include "SC_component_scene.h"
#include "SC_export_component_image.h"
#include "SC_dotwriter.h"
#include "SC_systemmodel.h"
#include "SC_export_network_image.h"
#include "EL_experiment.h"
#include <QCryptographicHash>
#include "SC_undocommands.h"
#include "SC_versioncontrol.h"
#include "qcustomplot.h"
#include "SC_projectobject.h"
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#include <QStandardPaths>
#endif

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
  #define RETINA_SUPPORT 1.0
#else
  #ifdef WIN_HIDPI_FIX
  #define RETINA_SUPPORT WIN_DPI_SCALING
  #else
  #define RETINA_SUPPORT 1.0
  #endif
#endif

#include "qdebug.h"
#include "SC_aboutdialog.h"


MainWindow::
MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    maxRecentFiles(12) // Number of files which show in the recent projects menu
{

    data.main = this;
    this->setWindowTitle("SpineCreator - Graphical SNN creation");

    QCoreApplication::setOrganizationName("SpineML");
    QCoreApplication::setOrganizationDomain("sheffield.ac.uk");
    QCoreApplication::setApplicationName("SpineCreator");

    // initialise GUI
    ui->setupUi(this);

    // Initialise Python. To run numba cuda code, it's going to be
    // necessary to set this up with a user-specifiable path to python
    // and/or modules.
    //
    // The path determined from anaconda/ipython:
    // ['', '/home/seb/anaconda3/bin', '/home/seb/anaconda3/lib/python37.zip', '/home/seb/anaconda3/lib/python3.7', '/home/seb/anaconda3/lib/python3.7/lib-dynload', '/home/seb/anaconda3/lib/python3.7/site-packages', '/home/seb/anaconda3/lib/python3.7/site-packages/IPython/extensions', '/home/seb/.ipython']
    Py_SetProgramName(L"/home/seb/anaconda3/bin/python");
    Py_SetPythonHome(L"/home/seb/anaconda3:/home/seb/anaconda3/bin:/home/seb/anaconda3/lib:/home/seb/anaconda3/lib/python3.7:/home/seb/anaconda3/lib/python3.7/lib-dynload:/home/seb/anaconda3/lib/python3.7/site-packages:/home/seb/anaconda3/lib/python3.7/site-packages/numba:/home/seb/anaconda3/lib/python3.7/site-packages/numba/cuda");
    Py_Initialize();

#ifdef DEBUG
    DBG() << "Python interpreter: " << (wchar_t*)Py_GetProgramName();
    DBG() << "Py_GetPrefix(): " << (wchar_t*)Py_GetPrefix();
    DBG() << "Py_GetExecPrefix(): " << (wchar_t*)Py_GetExecPrefix();
    DBG() << "Py_GetPath(): " << (wchar_t*)Py_GetPath();
    DBG() << "Py_GetProgramFullPath(): " << (wchar_t*)Py_GetProgramFullPath();
#endif

    QSettings settings; // probably get Python path from this and
                        // Py_SetProgramName therefrom. That will mean
                        // de-initialising and then re-initialising
                        // the python system when the setting is
                        // changed.

#if 0
    // clear all QSettings keys (for testing initial setup)
    QStringList keys = settings.allKeys();
    foreach (QString key, keys) {
        qDebug() << key;
        settings.remove(key);
    }
    exit(0);
#endif

#if QT_VERSION > QT_VERSION_CHECK(5, 0, 0)
  #ifdef Q_OS_MAC2

   settings.setValue("dpi",this->windowHandle()->devicePixelRatio());

  #endif
#endif

    this->emsg = (QErrorMessage*)0;

    // Copy the main toolbar stylesheet, as set in the QT UI form:
    this->toolbarStyleSheet = this->ui->toolbar_3->styleSheet();

    resize(settings.value("mainwindow/size", QSize(1060, 622)).toSize());
    move(settings.value("mainwindow/pos", QPoint(0, 0)).toPoint());
    QString currFileName = settings.value("files/currentFileName", "").toString();

    if (currFileName.size() != 0) {
        DBG() << "oops - a previous instance of SpineCreator seems to have crashed (currFileName: " << currFileName << ")";
    }

    // This really means "is *SpineML_2_BRAHMS* present?".
    if (!settings.value("simulators/BRAHMS/present", false).toBool()) {

#ifdef Q_OS_LINUX

        // On Linux, search for an installed version of
        // spineml-2-brahms and set corresponding defaults for
        // working_dir and path.

        // NB: These settings paths are case insensitive. Do CI
        // searches in the code to find where they're used.

        // See viewELexptpanelhandler.cpp for the use of these settings.

        // I'm simply going to assume that SpineML_2_BRAHMS will be
        // present, as for the Mac case, below.
        settings.setValue("simulators/BRAHMS/present", true);

        // spineml-2-brahms settings
        //
        // It may be that this directory will be used - the convert_script_s2b may create this in the home dir.
        QFile convert_script_packaged("/usr/bin/convert_script_s2b");
        QFile convert_script_home(QDir::toNativeSeparators(qgetenv("HOME") + "/SpineML_2_BRAHMS/convert_script_s2b"));

        if (convert_script_packaged.exists()) {
            // We have the _packaged_ version of the convert script
            if (settings.value("simulators/BRAHMS/working_dir").toString().compare("") == 0) {
                // Then there was no value set for working_dir; we can change it.
                settings.setValue("simulators/BRAHMS/working_dir", QDir::toNativeSeparators(qgetenv("HOME") + "/spineml-2-brahms"));
            }
            if (settings.value("simulators/BRAHMS/path").toString().compare("") == 0) {
                settings.setValue("simulators/BRAHMS/path", QDir::toNativeSeparators("/usr/bin/convert_script_s2b"));
            }

        } else if (convert_script_home.exists()) {
            if (settings.value("simulators/BRAHMS/working_dir").toString().compare("") == 0) {
                settings.setValue("simulators/BRAHMS/working_dir", QDir::toNativeSeparators(qgetenv("HOME") + "/SpineML_2_BRAHMS"));
            }
            if (settings.value("simulators/BRAHMS/path").toString().compare("") == 0) {
                settings.setValue("simulators/BRAHMS/path", QDir::toNativeSeparators(qgetenv("HOME") + "/SpineML_2_BRAHMS/convert_script_s2b"));
            }

        } else {
            // No SpineML to BRAHMS script in the usual locations. Set default values unless already set.
            if (settings.value("simulators/BRAHMS/path").toString().compare("") == 0) {
                settings.setValue("simulators/BRAHMS/path", QDir::toNativeSeparators(qgetenv("HOME") + "/SpineML_2_BRAHMS/convert_script_s2b"));
            }
            if (settings.value("simulators/BRAHMS/working_dir").toString().compare("") == 0) {
                settings.setValue("simulators/BRAHMS/working_dir", QDir::toNativeSeparators(qgetenv("HOME") + "/SpineML_2_BRAHMS"));
            }
        }

        // set to use binaries
        settings.setValue("simulators/BRAHMS/binary", true);

        // Check if we have the pre-compiled/packaged *SpineML* Namespace.
        QFile spineml_2_brahms_precompiled_namespace("/usr/lib/spineml-2-brahms/dev/SpineML/tools/allToAll/brahms/0/component.so");
        if (spineml_2_brahms_precompiled_namespace.exists()) {
            settings.setValue("simulators/BRAHMS/envVar/BRAHMS_NS", "/usr/lib/spineml-2-brahms/");
        } else {
            // Set the BRAHMS Namespace to the one we just set in simulators/BRAHMS/path
            settings.setValue("simulators/BRAHMS/envVar/BRAHMS_NS",
                              settings.value ("simulators/BRAHMS/working_dir").toString() + "/temp/Namespace/");
        }

#else
        // BRAHMS should be packaged with the app by default

        // assume SpineML_2_BRAHMS is present
        settings.setValue("simulators/BRAHMS/present", true);

        // Is this used? Yes, in viewELexptpanelhandler.cpp
        settings.setValue("simulators/BRAHMS/path", QDir::toNativeSeparators(qgetenv("HOME") + "/SpineML_2_BRAHMS/convert_script_s2b"));

        QDir brahmspath = qApp->applicationDirPath();
        brahmspath.cd("SystemML");
        QDir S2Bpath = qApp->applicationDirPath();
        S2Bpath.cd("SpineML_2_BRAHMS");

        // Initial code to try and find SpineML_2_BRAHMS - looks in
        // the HOME and one level of sub-directories, should be
        // expanded to be recursive
        QDir simdir(qgetenv("HOME"));
        QStringList correct_dir = simdir.entryList(QStringList("SpineML_2_BRAHMS"), QDir::Dirs);
        if (correct_dir.isEmpty()) {
            QStringList dirs = simdir.entryList(QDir::Dirs);
            for (int i = 0; i < dirs.size(); ++i) {
                simdir.cd(dirs[i]);
                correct_dir = simdir.entryList(QStringList("SpineML_2_BRAHMS"), QDir::Dirs);
                if (!correct_dir.isEmpty()) {
                    break;
                }
                simdir.cdUp();
            }
        }

        if (correct_dir.isEmpty()) {
            correct_dir.push_back("SpineML_2_BRAHMS");
        }

        settings.setValue("simulators/BRAHMS/path", S2Bpath.absolutePath() + "/convert_script_s2b");

        settings.setValue("simulators/BRAHMS/envVar/SYSTEMML_INSTALL_PATH", brahmspath.absolutePath());
        settings.setValue("simulators/BRAHMS/envVar/PATH", QDir::toNativeSeparators(qgetenv("PATH") + ":" + brahmspath.absolutePath() + QDir::toNativeSeparators("/SystemML/BRAHMS/bin/")));
        settings.setValue("simulators/BRAHMS/envVar/BRAHMS_NS", S2Bpath.absolutePath() + "/temp/Namespace/");
        settings.setValue("simulators/BRAHMS/working_dir", QDir::toNativeSeparators(qgetenv("HOME") + "/SpineML_2_BRAHMS_out"));
#endif
        settings.setValue("simulators/BRAHMS/envVar/REBUILD", "false");
    }

    // some default settings
    if (settings.value("fileOptions/saveBinaryConnections", -30).toInt() == -30) {
        settings.setValue("fileOptions/saveBinaryConnections", QString::number(float(true)));
    }
    if (settings.value("glOptions/detail", -30).toInt() == -30) {
        settings.setValue("glOptions/detail", 5);
    }

    // setup undo / redo
    undoStacks = new QUndoGroup(this);

    undoAction = undoStacks->createUndoAction(this,tr("&Undo"));
    undoAction->setShortcuts(QKeySequence::Undo);
    redoAction = undoStacks->createRedoAction(this,tr("&Redo"));
    redoAction->setShortcuts(QKeySequence::Redo);

    // add undo redo to menu
    ui->menuEdit->addAction(undoAction);
    ui->menuEdit->addAction(redoAction);

    projectObject * newProject = new projectObject();

    data.currProject = newProject;
    data.projects.push_back(newProject);
    newProject->name = "Untitled Project";
    undoStacks->addStack(newProject->undoStack);
    undoStacks->setActiveStack(newProject->undoStack);

    connect(undoStacks, SIGNAL(indexChanged(int)), this, SLOT(undoOrRedoPerformed(int)));
    connect(undoStacks, SIGNAL(cleanChanged(bool)), this, SLOT(updateTitle()));

    // Configure two QActions which will be presented in the Experiment menu.
    this->data.dupExpAction = new QAction("Duplicate experiment", this);
    connect(data.dupExpAction, SIGNAL(triggered()), this, SLOT(actionDuplicate_experiment_triggered()));
#if 0
    this->data.runExpAction = new QAction("Run experiment", this);
#endif
    // check for version control
    configureVCSMenu();

    // EXPERIMENT EDITOR initialisation
    initViewEL();

    // The empty graph view used when there is no open experiment for a project
    this->initEmptyGV();

    // NETWORK LAYER initialisation (there isn't much!)
    ui->butB->setEnabled(false);
    ui->butC->setEnabled(false);

    // VISUALISATION VIEWER initialisation
    this->viewVZ.OpenGLWidget = NULL;

    // COMPONENT EDITOR initialisation
    initViewCL();
    connectViewCL();

    // select first view button
    this->ui->tab1->setStyleSheet("border: 0px; color:white; background:rgba(255,255,255,40%)");

    // add viewNL properties panel
    nl_rootlayout * layoutRoot = new nl_rootlayout(&data, ui->parsPanel);
    this->viewNL.layout = layoutRoot;

#ifdef Q_OS_WIN
    // apply additional HiDPI scaling to the main UI (as Windows has incomplete HiDPI support)
    ui->viewSelector->setMinimumSize(ui->viewSelector->size()*RETINA_SUPPORT/2.0);
    ui->viewSelector->move(ui->viewSelector->pos()*RETINA_SUPPORT);

    ui->frame->setMinimumSize(ui->frame->size()*RETINA_SUPPORT);
    ui->frame->move(ui->frame->pos()*RETINA_SUPPORT);

    ui->frame_2->setMinimumSize(ui->frame_2->size()*RETINA_SUPPORT);
    ui->frame_2->move(ui->frame_2->pos()*RETINA_SUPPORT);

    ui->toolbar_3->setMinimumSize(ui->toolbar_3->size()*RETINA_SUPPORT);
    ui->toolbar_3->move(ui->toolbar_3->pos()*RETINA_SUPPORT);

    ui->topleft->setMinimumHeight(ui->topleft->height()*RETINA_SUPPORT);
    ui->topleft->move(ui->topleft->pos()*RETINA_SUPPORT);

    ui->butA->setMinimumWidth(ui->butA->width()*RETINA_SUPPORT);
    ui->butA->setMinimumHeight(ui->butA->height()*RETINA_SUPPORT);
    ui->butA->setIconSize(ui->butA->iconSize()*RETINA_SUPPORT);

    ui->butB->setMinimumWidth(ui->butB->width()*RETINA_SUPPORT);
    ui->butB->setMinimumHeight(ui->butB->height()*RETINA_SUPPORT);
    ui->butB->move(ui->butB->pos()*RETINA_SUPPORT);
    ui->butB->setIconSize(ui->butB->iconSize()*RETINA_SUPPORT);

    ui->butSS->setMinimumWidth(ui->butSS->width()*RETINA_SUPPORT);
    ui->butSS->setMinimumHeight(ui->butSS->height()*RETINA_SUPPORT);
    ui->butSS->move(ui->butSS->pos()*RETINA_SUPPORT);
    ui->butSS->setIconSize(ui->butSS->iconSize()*RETINA_SUPPORT);

    ui->butC->setMinimumWidth(ui->butC->width()*RETINA_SUPPORT);
    ui->butC->setMinimumHeight(ui->butC->height()*RETINA_SUPPORT);
    ui->butC->move(ui->butC->pos()*RETINA_SUPPORT);
    ui->butC->setIconSize(ui->butC->iconSize()*RETINA_SUPPORT);

    ui->tab0->setMinimumWidth(ui->tab0->width()*RETINA_SUPPORT);
    ui->tab0->setMinimumHeight(ui->tab0->height()*RETINA_SUPPORT);
    ui->tab0->move(ui->tab0->pos()*RETINA_SUPPORT);
    ui->tab0->setIconSize(ui->tab0->iconSize()*RETINA_SUPPORT);

    ui->tab1->setMinimumWidth(ui->tab1->width()*RETINA_SUPPORT);
    ui->tab1->setMinimumHeight(ui->tab1->height()*RETINA_SUPPORT);
    ui->tab1->move(ui->tab1->pos()*RETINA_SUPPORT);
    ui->tab1->setIconSize(ui->tab1->iconSize()*RETINA_SUPPORT);

    ui->tab2->setMinimumWidth(ui->tab2->width()*RETINA_SUPPORT);
    ui->tab2->setMinimumHeight(ui->tab2->height()*RETINA_SUPPORT);
    ui->tab2->move(ui->tab2->pos()*RETINA_SUPPORT);
    ui->tab2->setIconSize(ui->tab2->iconSize()*RETINA_SUPPORT);

    ui->tab3->setMinimumWidth(ui->tab3->width()*RETINA_SUPPORT);
    ui->tab3->setMinimumHeight(ui->tab3->height()*RETINA_SUPPORT);
    ui->tab3->move(ui->tab3->pos()*RETINA_SUPPORT);
    ui->tab3->setIconSize(ui->tab3->iconSize()*RETINA_SUPPORT);

    ui->tab4->setMinimumWidth(ui->tab4->width()*RETINA_SUPPORT);
    ui->tab4->setMinimumHeight(ui->tab4->height()*RETINA_SUPPORT);
    ui->tab4->move(ui->tab4->pos()*RETINA_SUPPORT);
    ui->tab4->setIconSize(ui->tab4->iconSize()*RETINA_SUPPORT);

    ui->caption->setMinimumSize(ui->caption->size()*RETINA_SUPPORT);
    ui->caption->move(ui->caption->pos()*RETINA_SUPPORT);


#endif

    // join up the components of the program
    QObject::connect(ui->viewport, SIGNAL(reDraw(QPainter*, float, float, float, int, int, drawStyle)), &(data), SLOT(reDrawAll(QPainter*, float, float, float, int, int, drawStyle)));
    QObject::connect(ui->viewport, SIGNAL(onLeftMouseDown(float,float,float,bool)), &(data), SLOT(onLeftMouseDown(float,float,float,bool)));
#ifdef NEED_MOUSE_UP_LOGIC
    QObject::connect(ui->viewport, SIGNAL(onLeftMouseUp(float,float,float)), &(data), SLOT(onLeftMouseUp(float,float,float)));
#endif
    QObject::connect(ui->viewport, SIGNAL(itemWasMoved()), &(data), SLOT(itemWasMoved()));
    QObject::connect(ui->viewport, SIGNAL(onRightMouseDown(float,float,float)), &(data), SLOT(onRightMouseDown(float,float,float)));
    QObject::connect(ui->viewport, SIGNAL(mouseMove(float,float)), &(data), SLOT(mouseMoveGL(float,float)));
    QObject::connect(ui->viewport, SIGNAL(drawSynapse(float,float)), &(data), SLOT(startAddBezier(float,float)));
    QObject::connect(ui->viewport, SIGNAL(addBezierOrProjection(float,float)), &(data), SLOT(addBezierOrProjection(float,float)));
    QObject::connect(ui->viewport, SIGNAL(abortProjection()), &(data), SLOT(abortProjection()));

    // connect up drag selection for network layer view
    QObject::connect(ui->viewport, SIGNAL(dragSelect(float,float)), &(data), SLOT(dragSelect(float,float)));
    QObject::connect(ui->viewport, SIGNAL(endDragSelect()), &(data), SLOT(endDragSelection()));

    QObject::connect(&(data), SIGNAL(finishDrawingSynapse()), ui->viewport, SLOT(finishConnect()));
    QObject::connect(&(data), SIGNAL(redrawGLview()), ui->viewport, SLOT(redrawGLview()));
    //QObject::connect(&(data), SIGNAL(redrawGLview()), viewVZ.OpenGLWidget, SLOT(redraw()));
    QObject::connect(&(data), SIGNAL(setCaption(QString)), this, SLOT(setCaption(QString)));
    QObject::connect(&(data), SIGNAL(setWindowTitle()), this, SLOT(updateTitle()));

    QObject::connect(ui->butA, SIGNAL(clicked()), &(data), SLOT(addPopulation()));
    QObject::connect(ui->butB, SIGNAL(clicked()), ui->viewport, SLOT(startConnect()));
    QObject::connect(ui->butSS, SIGNAL(clicked()), &(data), SLOT(addSpikeSource()));
    QObject::connect(ui->butC, SIGNAL(clicked()), &(data), SLOT(deleteCurrentSelection()));

    QObject::connect(ui->tab0, SIGNAL(clicked()), this, SLOT(viewELshow()));
    QObject::connect(ui->tab1, SIGNAL(clicked()), this, SLOT(viewNLshow()));
    QObject::connect(ui->tab2, SIGNAL(clicked()), this, SLOT(viewVZshow()));
    QObject::connect(ui->tab3, SIGNAL(clicked()), this, SLOT(viewCLshow()));
    QObject::connect(ui->tab4, SIGNAL(clicked()), this, SLOT(viewGVshow()));

    // for animation
    QTimer *timer = new QTimer( this );
    // this creates a Qt timer event
    connect( timer, SIGNAL(timeout()), ui->viewport, SLOT(animate()) );

    QObject::connect(&(data), SIGNAL(updatePanel(nl_rootdata*)), layoutRoot, SLOT(updatePanel(nl_rootdata*)));
    QObject::connect(&(data), SIGNAL(updatePanel(nl_rootdata*)), this, SLOT(updateNetworkButtons(nl_rootdata*)));
    QObject::connect(this, SIGNAL(updatePanel(nl_rootdata*)), layoutRoot, SLOT(updatePanel(nl_rootdata*)));

    QObject::connect(layoutRoot, SIGNAL(setCaption(QString)), &(data), SLOT(setModelTitle(QString)));
    QObject::connect(&(data), SIGNAL(statusBarUpdate(QString, int)), ui->statusBar, SLOT(showMessage(QString, int)));
    QObject::connect(this, SIGNAL(statusBarUpdate(QString, int)), ui->statusBar, SLOT(showMessage(QString, int)));

    // add the library to the component file list
    addComponentsToFileList();

    this->createActions();

    timer->start(30);

    // force nice startup
    // Construct the menus
    ui->menuBar->clear();
    ui->menuBar->addMenu(ui->menuFile);
    ui->menuBar->addMenu(ui->menuEdit);
    ui->menuBar->addMenu(ui->menuProject);
    ui->menuBar->addMenu(ui->menuModel); // actually version control
    ui->menuBar->addMenu(ui->menuHelp);

    // Recent projects
    this->setupRecentProjectsMenu(&settings);

    // default
    ui->action_Close_project->setEnabled(false);

    // setup projects
    this->setProjectMenu();
    this->setExperimentMenu(); // ?

    // show current view
    this->ui->view1->show();

    // titlebar
    updateTitle();

    // redraw
    this->ui->viewport->changed = 1;
    QApplication::processEvents( QEventLoop::ExcludeUserInputEvents );
    layoutRoot->updatePanel(&data);
    data.setCaptionOut("Untitled Project");
    updateTitle();

    this->setWindowTitle("SpineCreator: Network Editor");

    // update layout with libraries
    viewNL.layout->updatePanel(&data);

    // for now
    ui->actionE_xport_network->setEnabled(false);

#ifdef Q_OS_MAC111
    fix.setSingleShot(true);
    connect(&fix, SIGNAL(timeout()), this, SLOT(osxHack()));
    fix.start(200);
#endif
}

void
MainWindow::updateRecentProjects(const QString& filePath)
{
    // Save the last-used directory so the dialog box can conveniently
    // open with that location:
    QDir lastDirectory (filePath);
    lastDirectory.cdUp();
    QSettings settings;
    settings.setValue (MAINWINDOW_LASTPROJECTDIR, lastDirectory.absolutePath());

    // To hold a list of the recent filePaths used.
    QList <QString> recents;
    // First we load existing recent files list:
    int size = settings.beginReadArray("mainwindow/recentprojects");
    int i = 0;
    for (i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        recents.push_back(settings.value("filePath").toString());
    }
    settings.endArray();

    // Now we modify the recents list to remove any existing entry for filePath
    QList<QString>::iterator iter = recents.begin();
    while (iter != recents.end()) {
        if (*iter == filePath) {
            recents.erase (iter);
            break;
        }
        ++iter;
    }
    // and then finally stick filePath in at the front of the list.
    recents.push_front(filePath);

    // Now write it back to QSettings.
    settings.beginWriteArray("mainwindow/recentprojects");
    // Hmm, QHash seems to be disordered, and not by "when inserted".
    QList<QString>::const_iterator citer = recents.constBegin();
    i = 0;
    while (i < MainWindow::maxRecentFiles && citer != recents.constEnd()) {
        settings.setArrayIndex(i);
        settings.setValue("filePath", *citer);
        ++citer; ++i;
    }
    settings.endArray();

    // Lastly update the recent projects menu:
    this->setupRecentProjectsMenu (&settings);
}

void MainWindow::setupRecentProjectsMenu(QSettings* settings)
{
    QMenu* recents = ui->menuFile->findChild<QMenu*>("menuRecent_projects");
    if (recents != NULL) {
        // Clear the contents set up in the UI editor.
        recents->clear();

        // For each member of the recent files list:
        int size = settings->beginReadArray("mainwindow/recentprojects");
        for (int i = 0; i < size; ++i) {
            settings->setArrayIndex(i);
            QAction* actionTest;
            actionTest = new QAction(settings->value("filePath").toString(), this);
            recents->addAction(actionTest);
            // connect this action to the slot someslot():
            connect(actionTest, SIGNAL(triggered()), this, SLOT(import_recent_project()));
        }
        settings->endArray();

        // Finally add separator and the Clear list action
        recents->addSeparator();
        QAction* actionClear;
        actionClear = new QAction("&Clear list", this);
        //actionClear->setShortcut(Qt::Key_C); // shortcuts not used within this app
        recents->addAction(actionClear);
        connect(actionClear, SIGNAL(triggered()), this, SLOT(clear_recent_projects()));
    }
}

#ifdef Q_OS_MAC111
void MainWindow::osxHack()
{
    // make sure gl context is initialised correctly
    viewVZshow();
    viewNLshow();
}
#endif

bool MainWindow::isChanged()
{
    for (int i = 0; i < data.projects.size(); ++i) {
        if (data.projects[i]->isChanged(&data))
            return true;
    }

    return false;
}

void MainWindow::setProjectMenu()
{
    // clear existing
    ui->menuProject->clear();
    // if we have the action group
    if (data.projectActions != NULL) {
        delete data.projectActions;
    }
    // create a new action group
    data.projectActions = new QActionGroup(this);
    // add actions to select projects to the action group
    for (int i = 0; i < data.projects.size(); ++i) {
        data.projectActions->addAction(data.projects[i]->action(i));
    }
    // select the current project
    data.currProject->menuAction->setChecked(true);
    // add the action group to the menu
    ui->menuProject->addActions(data.projectActions->actions());
    connect(data.projectActions, SIGNAL(triggered(QAction*)), &data, SLOT(selectProject(QAction*)));
}

void MainWindow::setExperimentMenu()
{
    ui->menuExperiment->clear(); // existing menu actions.

    // Add entries for each experiment to the menu and connect up a
    // suitable function to switch between experiments.
    if (this->data.experimentActions != NULL) {
        delete this->data.experimentActions;
    }

    // Add Duplicate experiment and Run experiment actions.
    this->ui->menuExperiment->addAction (this->data.dupExpAction);
#if 0
    this->ui->menuExperiment->addAction (this->data.runExpAction);
#endif
    // Add a list of available experiments
    this->data.experimentActions = new QActionGroup(this);
    // Each experiment is going to have a QAction stored for it.
    for (int i = 0; i < this->data.experiments.size(); ++i) {
        QAction* a = this->data.experiments[i]->action(i);
        a->setChecked (this->data.experiments[i]->selected);
        this->data.experimentActions->addAction (a);
    }
    this->ui->menuExperiment->addActions (this->data.experimentActions->actions());
    connect (data.experimentActions, SIGNAL(triggered(QAction*)), &data, SLOT(selectExperiment(QAction*)));
}

void MainWindow::selectExperiment (int exptNum)
{
    DBG() << "MainWindow::selectExperiment called; experiments.size: " << data.experiments.size();

    for (int i = 0; i < data.experiments.size(); ++i) {
        data.experiments[i]->selected = ((i == exptNum) ? true : false);
        DBG() << "experiment " << i << " is selected?: " << data.experiments[i]->selected;
        if (data.experiments[i]->selected == true) {
            // Then make sure there's a viewGV for the experiment.
            if (!this->existsViewGV(data.experiments[i])) {
                if (data.experiments[i] == (experiment*)0) {
                    DBG() << "Error: null experiment*! continuing...";
                    continue;
                }
                this->initViewGV (data.experiments[i]);
            } else {
                this->viewGVreshow();
            }
        }
    }
}

int MainWindow::getCurrentExptNum (void)
{
    int currentExptNum = -1;
    for (int i = 0; i < data.experiments.size(); ++i) {
        if (data.experiments[i]->selected) {
            currentExptNum = i;
            break;
        }
    }
    return currentExptNum;
}

experiment* MainWindow::getCurrentExpt (void)
{
    experiment* currentExperiment = (experiment*)0;
    for (int i = 0; i < data.experiments.size(); ++i) {
        if (data.experiments[i]->selected) {
            currentExperiment = data.experiments[i];
            break;
        }
    }
    return currentExperiment;
}

void MainWindow::updateDatas (void)
{
    // fetch current experiment sim engine
    int currentExptNum = this->getCurrentExptNum();
    experiment* currentExperiment = this->getCurrentExpt();

    DBG() << "Current expt num is " << currentExptNum;

    // Build up the correct log path (compare with code in
    // SC_viewELexptpanelhander.cpp)
    if (currentExptNum != -1) {
        QSettings settings;
        QString simName = currentExperiment->setup.simType;
        settings.beginGroup("simulators/" + simName);
        QString wk_dir_string = settings.value("working_dir").toString();
        settings.endGroup();
        wk_dir_string = QDir::toNativeSeparators(wk_dir_string);
        QDir wk_dir(wk_dir_string);
        QString out_dir_name = wk_dir.absolutePath() + QDir::separator() + "temp"
            + QDir::separator() + data.currProject->getFilenameFriendlyName()
            + "_e" + QString::number(currentExptNum);
        QString perexpt_logpath = out_dir_name + QDir::separator() + "log";
        QDir logs(perexpt_logpath);
        QStringList filter;
        filter << "*.xml";
        logs.setNameFilters(filter);

        if (this->viewGV[currentExperiment]->properties->currentLogDataDir != perexpt_logpath) {
            DBG() << "Log dir changed. CurrentDatasDir:" << this->viewGV[currentExperiment]->properties->currentLogDataDir;
            DBG() << "perexpt_logpath for currentExptNum "<< currentExptNum << ": " << perexpt_logpath;

            this->viewGV[currentExperiment]->properties->currentLogDataDir = perexpt_logpath;

            // Clear plots and add an empty one.
            this->viewGV[currentExperiment]->properties->clearPlots();
            this->viewGV[currentExperiment]->properties->addEmptyPlot();
            // Clear out vLogData as we'll re-read from the new expt log directory
            this->viewGV[currentExperiment]->properties->clearVLogData();
            this->viewGV[currentExperiment]->properties->populateVLogData (logs.entryList(), &logs);
            // and insert logs into visualiser
            if (this->viewVZ.OpenGLWidget != NULL) {
                this->viewVZ.OpenGLWidget->addLogs(&this->viewGV[currentExperiment]->properties->vLogData);
            }
        }
    } else {
        DBG() << "Current expt num is -1, no current experiment exists, nothing to do.";
    }
}

bool MainWindow::promptToSave()
{
    // check what the user wants to do
    QMessageBox msgBox(this);
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setText("<b>Unsaved changes!                </b>");
    msgBox.setInformativeText("There are unsaved changes in project '" + data.currProject->name + "'. Do you want to save your changes?");
    msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Save);
    msgBox.setModal(true);
    switch (msgBox.exec()) {
       case QMessageBox::Save:
           export_project();
           break;
       case QMessageBox::Discard:
           break;
       case QMessageBox::Cancel:
           return false;
       default:
           // should never be reached
           break;
    }

    return true;
}

void MainWindow::closeEvent(QCloseEvent * event)
{
    // if there are changes
    for (int i = 0; i < data.projects.size(); ++i) {
        if (data.projects[i]->isChanged(&data)) {
            data.currProject->deselect_project(&data);
            data.projects[i]->select_project(&data);
            if (!promptToSave()) {
                // abort closing program
                event->ignore();
                return;
            }
        }
    }

    // clear some pointers
    if (this->viewVZ.OpenGLWidget != NULL) {
        this->viewVZ.OpenGLWidget->clear();
    }
    // disconnect the undo signal
    undoStacks->disconnect();
    //data.undoStack->clear();
    event->accept();
}

void MainWindow::clearComponents()
{
    // delete catalog components
    for (int i = 0; i < this->data.catalogLayout.size(); ++i) {
        this->data.catalogLayout[i].clear();
    }
    for (int i = 0; i < this->data.catalogNrn.size(); ++i) {
        this->data.catalogNrn[i].clear();
    }
    for (int i = 0; i < this->data.catalogPS.size(); ++i) {
        this->data.catalogPS[i].clear();
    }
    for (int i = 0; i < this->data.catalogWU.size(); ++i) {
        this->data.catalogWU[i].clear();
    }
    for (int i = 0; i < this->data.catalogUnsorted.size(); ++i) {
        this->data.catalogUnsorted[i].clear();
    }

    // clear catalog vectors
    data.catalogLayout.clear();
    data.catalogNrn.clear();
    data.catalogPS.clear();
    data.catalogWU.clear();
    data.catalogUnsorted.clear();

}


MainWindow::~MainWindow()
{
    QSettings settings;

    settings.setValue("mainwindow/size", size());
    settings.setValue("mainwindow/pos", pos());
    settings.remove("files/currentFileName");

    // start investigating the library
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    #if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    QDir lib_dir = QDir(QDesktopServices::storageLocation(QDesktopServices::DataLocation));
#else
    QDir lib_dir = QDir(QStandardPaths::writableLocation(QStandardPaths::DataLocation));
#endif
#else
    QDir lib_dir = QDir(QStandardPaths::writableLocation(QStandardPaths::DataLocation));
#endif
    if (!lib_dir.exists()) {
        if (!lib_dir.mkpath(lib_dir.absolutePath()))
            qDebug() << "error creating library";
    }

    // remove any stale temporary connection list files once at program start.
    lib_dir.setFilter(QDir::Files);
    foreach(QString dirFile, lib_dir.entryList()) {
        lib_dir.remove(dirFile);
    }

    if (this->emsg != (QErrorMessage*)0) {
        delete this->emsg;
    }

    if (viewCL.root != NULL)
        delete viewCL.root;

    // delete catalogs:
    clearComponents();

    if (viewVZ.OpenGLWidget != NULL) {
        //delete this->viewVZ.errors;
        this->viewVZ.layout.clear();
    }

    // clear up python
    Py_Finalize();

    // Ensure viewELhandler's destructor is called to clean up temporary model directory
    delete this->viewELhandler;

    // FIXME - there's probably more cleanup to do here.

    delete ui;
}

void MainWindow::initEmptyGV (void)
{
    this->setupViewGV (&this->emptyGV);
    this->emptyGV.properties->clearPlots();
    this->emptyGV.properties->addEmptyPlot();
}

void MainWindow::setupViewGV (viewGVstruct* vgv)
{
    // add ref to this
    vgv->mainwindow = this;

    // add a sub-window. This should be the only thing we need to explicitly delete. With luck...
    vgv->subWin = new QMainWindow(this);
    vgv->subWin->setWindowFlags(Qt::Widget);

    // add toolbar
    vgv->toolbar = new QToolBar("Graphing Toolbar");
    vgv->toolbar->setAllowedAreas(Qt::TopToolBarArea);
#ifdef Q_OS_MAC
    // Don't set toolbar stylesheet for Mac, current QT implementation ignores it.
#else
    vgv->toolbar->setStyleSheet(this->toolbarStyleSheet);
#endif
    vgv->subWin->addToolBar(Qt::TopToolBarArea,vgv->toolbar);

    // make a central widget
    vgv->subWin->setCentralWidget(new QFrame);

    // assign a layout to the central widget
    vgv->subWin->centralWidget()->setLayout(new QHBoxLayout);

    // setup the spacing
    vgv->subWin->centralWidget()->layout()->setSpacing(0);
    vgv->subWin->centralWidget()->layout()->setContentsMargins(0,0,0,0);

    // create a new mdi area to hold the graph windows
    vgv->mdiarea = new QMdiArea(vgv->subWin);
    vgv->subWin->centralWidget()->layout()->addWidget(vgv->mdiarea); // mdiarea owned by subWin.

    // add a dock widget
    vgv->dock = new QDockWidget("Log data");
    vgv->dock->setFeatures(QDockWidget::DockWidgetMovable);
    // The dock becomes owned by the subWin, so shouldn't need to
    // explicitly delete the subWin:
    vgv->subWin->addDockWidget(Qt::RightDockWidgetArea,vgv->dock);

    // add the properties editor
    vgv->properties = new viewGVpropertieslayout(vgv);
    vgv->dock->setWidget(vgv->properties); // the properties should be
                                           // parented to the dock, so
                                           // when the QDockWidget is
                                           // deallocated, then the
                                           // properties should be
                                           // automatically.

    // add sub window area to layout
    ((QGridLayout*)this->ui->centralWidget->layout())->addWidget(vgv->subWin, 0,
                                                                 ((QGridLayout*)this->ui->centralWidget->layout())->columnCount(), 4, 1);

    // hide to start with
    vgv->subWin->hide();
}

void MainWindow::cleanupViewGV (viewGVstruct* vgv)
{
    vgv->subWin->hide();
    delete vgv->subWin;
}

bool MainWindow::existsViewGV (experiment* e)
{
    bool rtn(false);
    if (e == (experiment*)0) {
        return rtn;
    }
    if (this->viewGV.find(e) != this->viewGV.end()) {
        rtn = true;
    }
    return rtn;
}

void MainWindow::initViewGV(experiment* e)
{
    DBG() << "Called";

    if (this->existsViewGV (e)) {
        DBG() << "A viewGV for the given experiment already exists.";
        return;
    }

    // viewGV becomes map of viewGVstructs, keyed by experiment pointer.
    viewGVstruct* vgv = new viewGVstruct;

    // add ref to expt
    vgv->e = e;

    this->setupViewGV(vgv);

    // Now add it to the QMap:
    this->viewGV[e] = vgv;

    // This populates the list of available data for the experiment:
    this->updateDatas();
}

void MainWindow::initViewEL()
{
    // adding viewEL (experiments) ####################

    // add horizontal layout
    QFrame * frame = new QFrame;
    QHBoxLayout * splitter = new QHBoxLayout;
    frame->setLayout(splitter);
    viewEL.view = frame;

    // add the splitter to the main layout, spanning 2 rows and set margins
    ((QGridLayout *) this->ui->centralWidget->layout())->addWidget(frame, 0, ((QGridLayout *) this->ui->centralWidget->layout())->columnCount(),4,1);
    frame->resize(this->ui->view1->size());
    frame->setContentsMargins(0,0,0,0);
    splitter->setContentsMargins(0,0,0,0);

    // hide, since we will default to view 1
    frame->hide();

    // add the panel
    QScrollArea *expt = new QScrollArea();
    expt->setLineWidth(1);
    expt->setFrameStyle(1);

    QWidget * exptContent = new QWidget(expt);
    expt->setWidget(exptContent);
    expt->setWidgetResizable(true);

    exptContent->setLayout(new QHBoxLayout());
    exptContent->layout()->setSpacing(5);

    splitter->addWidget(expt);

    this->viewEL.expt = exptContent;
    this->viewEL.propertiesScrollArea = expt;

    // add the view
    QScrollArea *selection = new QScrollArea();
    selection->setLineWidth(0);
    selection->setFrameStyle(1);

    QWidget *selContent0 = new QWidget;
    selection->setWidget(selContent0);
    selection->setWidgetResizable(true);
    selection->setMinimumWidth(300*RETINA_SUPPORT);
    selection->setMaximumWidth(300*RETINA_SUPPORT);

    selContent0->setLayout(new QVBoxLayout());

    this->viewEL.panel = selContent0;

    ((QVBoxLayout *) selContent0->layout())->addStretch();
    ((QVBoxLayout *) selContent0->layout())->setContentsMargins(0,0,0,0);
    ((QVBoxLayout *) selContent0->layout())->setSpacing(0);

    splitter->insertWidget(0,selection);

    // add and set up the toolbar
    QFrame * toolbar0 = new QFrame(this->ui->toolbar_3);
    toolbar0->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    toolbar0->setMinimumSize(this->ui->topleft->size());
    toolbar0->setMinimumHeight(27*RETINA_SUPPORT);
    toolbar0->setMaximumHeight(27*RETINA_SUPPORT);
    toolbar0->setStyleSheet(this->toolbarStyleSheet);
    if (toolbar0->layout())
        delete toolbar0->layout();
    toolbar0->setLayout(new QHBoxLayout);
    toolbar0->layout()->setContentsMargins(0,0,0,0);
    toolbar0->layout()->setSpacing(0);
    toolbar0->setLineWidth(0);
    toolbar0->setFrameStyle(0);

    // add toolbar to the frame
    ((QVBoxLayout *) selContent0->layout())->insertWidget(0, toolbar0);
    ((QHBoxLayout *) toolbar0->layout())->addStretch();

    QFrame* line0b = new QFrame();
    line0b->setMaximumHeight(1);
    line0b->setFrameShape(QFrame::HLine);
    line0b->setFrameShadow(QFrame::Plain);

    ((QVBoxLayout *) selContent0->layout())->insertWidget(1,line0b);

    this->viewELhandler = new viewELExptPanelHandler(&(this->viewEL), &(this->data));
    this->viewELhandler->main = this;
}

void MainWindow::connectViewEL()
{
}

void MainWindow::initViewCL()
{
    // store a pointer to the main window
    viewCL.mainWindow = this;

    // no unsaved changes at startup!
    unsaved_changes = false;

    // set to NULL so we can detect it isn't initialised
    viewCL.root = NULL;

    // create the main QFrame to contain the tab's QWidgets
    viewCL.frame = new QFrame();
    // add the QFrame to the centralWidget of the window
    ((QGridLayout *) this->ui->centralWidget->layout())->addWidget(viewCL.frame, 0, ((QGridLayout *) this->ui->centralWidget->layout())->columnCount(),4,1);

    // add a root layout
    viewCL.layout = new QHBoxLayout();
    // delete any existing layout and replace with this one
    if (viewCL.frame->layout()) {
        delete viewCL.frame->layout();
    }
    viewCL.frame->setLayout(viewCL.layout);

    // get rid of the margins and padding
    viewCL.frame->setContentsMargins(0,0,0,0);
    viewCL.layout->setContentsMargins(0,0,0,0);
    viewCL.layout->setSpacing(0);

    QMainWindow * subWin = new QMainWindow(this);
    subWin->setWindowFlags(Qt::Widget);
    viewCL.subWin = subWin;

    viewCL.layout->addWidget(subWin);

    subWin->setCentralWidget(new QFrame);

    subWin->centralWidget()->setLayout(new QHBoxLayout());

    // get rid of the margins and padding
    subWin->setContentsMargins(0,0,0,0);
    subWin->centralWidget()->layout()->setContentsMargins(0,0,0,0);
    subWin->centralWidget()->layout()->setSpacing(0);

    // add the Properties dock
    viewCL.dock = new QDockWidget("Properties", subWin);
    subWin->addDockWidget(Qt::RightDockWidgetArea, viewCL.dock);
    // stop the dock being movable / closable etc....
    viewCL.dock->setFeatures(QDockWidget::DockWidgetMovable);

    // add a QWidget to the Properties dock to contain the properties widgets and hide the dock
    viewCL.propertiesContents = new QWidget;
    viewCL.dock->setWidget(viewCL.propertiesContents);
    viewCL.dock->hide();

    // add the visualisation pane
    viewCL.display = new ALView(ui->centralWidget);
    subWin->centralWidget()->layout()->addWidget(viewCL.display);

    // add button to hide / show the file box
    viewCL.toggleFileBox = new QPushButton;
    viewCL.toggleFileBox->setMaximumWidth(10*RETINA_SUPPORT);
    viewCL.toggleFileBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    viewCL.toggleFileBox->setText("<");
    viewCL.toggleFileBox->setFlat(true);
    viewCL.layout->insertWidget(0,viewCL.toggleFileBox);

    // add the file box layout
    QVBoxLayout * files = new QVBoxLayout;
    viewCL.layout->insertLayout(0,files);
    viewCL.fileListLayout = files;

    // add file box
    viewCL.fileList = new QListWidget;
    viewCL.fileList->setMinimumWidth(250*RETINA_SUPPORT);
    viewCL.fileList->setMaximumWidth(250*RETINA_SUPPORT);
    viewCL.fileList->setSortingEnabled(true);
    QListWidgetItem *newItem = new QListWidgetItem;
    newItem->setText("No components loaded...");
    viewCL.fileList->addItem(newItem);
    files->insertWidget(0,viewCL.fileList);

    // add and set up the file toolbar
    QFrame * toolbar0 = new QFrame(this->ui->toolbar_3);
    toolbar0->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    toolbar0->setMinimumWidth(250*RETINA_SUPPORT);
    toolbar0->setMaximumWidth(250*RETINA_SUPPORT);
    toolbar0->setMinimumHeight(27*RETINA_SUPPORT);
    toolbar0->setMaximumHeight(27*RETINA_SUPPORT);
    toolbar0->setStyleSheet(this->toolbarStyleSheet);
    if (toolbar0->layout())
        delete toolbar0->layout();
    toolbar0->setLayout(new QHBoxLayout);
    toolbar0->layout()->setContentsMargins(0,0,0,0);
    toolbar0->layout()->setSpacing(0);
    toolbar0->setLineWidth(0);
    toolbar0->setFrameStyle(0);

    // add file toolbar to the frame
    files->insertWidget(0, toolbar0);

    // a nice black line
    QFrame* line = new QFrame();
    line->setMaximumHeight(1);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Plain);
    files->insertWidget(1, line);


    QCommonStyle style;

    // add a new file button to the toolbar
    QPushButton * newFile = new QPushButton();
    newFile->setFlat(true);
    newFile->setText("");
    newFile->setToolTip("New component");
    newFile->setIcon(style.standardIcon(QStyle::SP_FileIcon));
    toolbar0->layout()->addWidget(newFile);

    connect(newFile, SIGNAL(clicked()), this, SLOT(actionNew_triggered()));

    // add a load file button to the toolbar
    QPushButton * openFile = new QPushButton();
    openFile->setFlat(true);
    openFile->setText("");
    openFile->setToolTip("Load components");
    openFile->setIcon(style.standardIcon(QStyle::SP_DirOpenIcon));
    toolbar0->layout()->addWidget(openFile);

    connect(openFile, SIGNAL(clicked()), this, SLOT(import_component()));

    // add a close file button to the toolbar
    QPushButton * closeFile = new QPushButton();
    closeFile->setFlat(true);
    closeFile->setText("");
    closeFile->setToolTip("Close/delete selected component");
    closeFile->setIcon(style.standardIcon(QStyle::SP_DirClosedIcon));
    toolbar0->layout()->addWidget(closeFile);

    connect(closeFile, SIGNAL(clicked()), this, SLOT(delete_component()));

    ((QHBoxLayout *) toolbar0->layout())->addStretch();

    // finish off
    viewCL.frame->hide();
    // new model
    //initialiseModel((QFile *) NULL);
    updateTitle(false);

    // SHORTCUTS ################################
    // delete mapped to delete button
    deleteShortcut = new QShortcut(QKeySequence(tr("Del", "Edit|Delete")), this);
    connect(deleteShortcut, SIGNAL(activated()), this, SLOT(actionDeleteItems_triggered()));
}

void MainWindow::connectViewCL()
{
    // connect file list toggle
    connect(viewCL.toggleFileBox, SIGNAL(clicked()), this, SLOT(fileListToggle()));

    // connect file list changed
    connect(viewCL.fileList, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), this, SLOT(fileListItemChanged(QListWidgetItem*,QListWidgetItem*)));
}

void MainWindow::initViewVZ()
{
    // adding VIEWVZ (populations and projections) ####################################################################################
    this->viewVZ.layout = QSharedPointer<NineMLLayout> (new NineMLLayout(this->data.catalogLayout[0]));

    this->viewVZ.sysModel = NULL;

    // add splitter
    QSplitter * splitter = new QSplitter();
    // add a frame
    QFrame * frame = new QFrame();

    this->viewVZ.errors = new QLabel;

    // add the splitter to the main layout, spanning 2 rows and set margins
    ((QGridLayout *) this->ui->centralWidget->layout())->addWidget(splitter, 0, ((QGridLayout *) this->ui->centralWidget->layout())->columnCount(),4,1);
    splitter->resize(this->ui->view1->size());
    splitter->setContentsMargins(0,0,0,0);
    splitter->setHandleWidth(0);

    // hide, since we will default to view 1
    splitter->hide();

    // add the frame for the body of the view
    splitter->addWidget(frame);

    // add the panel
    QScrollArea *panel = new QScrollArea();
    panel->setLineWidth(1);
    panel->setFrameStyle(1);

    QWidget * panelContent = new QWidget;
    panel->setWidget(panelContent);
    panel->setWidgetResizable(true);
    panel->setMaximumWidth(600*RETINA_SUPPORT);

    panelContent->setLayout(new QVBoxLayout());

    this->viewVZ.panel = panelContent;

    // add the panel to the splitter
    splitter->addWidget(panel);

    // add some more stuff
    QGridLayout * v2lay = new QGridLayout;
    if (frame->layout())
        delete frame->layout();
    frame->setLayout(v2lay);

    frame->layout()->setContentsMargins(0,0,0,0);
    frame->layout()->setSpacing(0);
    frame->setContentsMargins(0,0,0,0);

    // add and set up the toolbar
    QFrame * toolbar = new QFrame(this->ui->toolbar_3);
    toolbar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    toolbar->setMinimumSize(this->ui->topleft->size());
    toolbar->setMinimumHeight(27*RETINA_SUPPORT);
    toolbar->setMaximumHeight(27*RETINA_SUPPORT);
    toolbar->setStyleSheet(this->toolbarStyleSheet);
    if (toolbar->layout())
        delete toolbar->layout();
    toolbar->setLayout(new QHBoxLayout);
    toolbar->layout()->setContentsMargins(0,0,0,0);
    toolbar->layout()->setSpacing(0);
    toolbar->setLineWidth(0);
    toolbar->setFrameStyle(0);

    // add toolbar to the frame and struct
    frame->layout()->addWidget(toolbar);
    viewVZ.toolbar = toolbar;

    QFrame* line2 = new QFrame();
    //line->setObjectName(QString::fromUtf8("line"));
    line2->setMaximumHeight(1);
    line2->setFrameShape(QFrame::HLine);
    line2->setFrameShadow(QFrame::Plain);

    frame->layout()->addWidget(line2);

    // this is a placeholder for the main widget which will show the 3D view of the populations and connections
    this->viewVZ.OpenGLWidget = new glConnectionWidget(&this->data);
    frame->layout()->addWidget(this->viewVZ.OpenGLWidget);

    this->viewVZhandler = new viewVZLayoutEditHandler(&data, &this->viewNL, &this->viewVZ, this);

    // try to divide the splitter up nicely
    this->viewVZ.view = splitter;
    QList <int> sizes;
    sizes.push_back(3);
    sizes.push_back(1);
    sizes.push_back(1);
    splitter->setSizes(sizes);
}

void MainWindow::connectViewVZ()
{
    // viewVZ
    QObject::connect(this->viewVZ.OpenGLWidget, SIGNAL(setSelectionbyName(QString)), &(data), SLOT(setSelectionbyName(QString)));
    QObject::connect(this->viewVZ.OpenGLWidget, SIGNAL(getNeuronLocationsSrc(QVector < QVector <loc> >*,QVector <QColor>*, QString)), &(data), SLOT(getNeuronLocationsSrc(QVector < QVector <loc> >*,QVector <QColor>*, QString)));
    QObject::connect(this->viewVZ.OpenGLWidget, SIGNAL(updatePanel(QString)), this->viewVZhandler, SLOT(redrawFromObject(QString)));
    QObject::connect(&(data), SIGNAL(updatePanelView2(QString)), this->viewVZhandler, SLOT(redrawFromObject(QString)));
}

void MainWindow::fileListToggle()
{
    if (viewCL.fileList->isHidden()) {
        QLayoutItem * item;
        for (int i = 0; i < viewCL.fileListLayout->count(); ++i) {
            item = viewCL.fileListLayout->itemAt(i);
            if (item->widget())
                ((QWidget *) item->widget())->show();
        }
        //viewCL.fileList->show();
        viewCL.toggleFileBox->setText("<");
    }
    else {
        QLayoutItem * item;
        for (int i = 0; i < viewCL.fileListLayout->count(); ++i) {
            item = viewCL.fileListLayout->itemAt(i);
            if (item->widget())
                ((QWidget *) item->widget())->hide();
        }
        viewCL.fileList->hide();
        viewCL.toggleFileBox->setText(">");
    }
}

void MainWindow::fileListItemChanged(QListWidgetItem * current, QListWidgetItem *)
{
    // ok - need to be a bit careful here...

    if (current == NULL) {
        return;
    }

    // extract the current item component:
    // select catalog
    QVector < QSharedPointer<Component> > currCatalog;
    QString catalogString;

    QSharedPointer<Component> selectedComponent;

    for (int catNum = 0; catNum < 3; ++catNum) {

        switch (catNum) {
        case 0:
            currCatalog = data.catalogWU;
            catalogString = "WU: ";
            break;
        case 1:
            currCatalog = data.catalogPS;
            catalogString = "PS: ";
            break;
        case 2:
            currCatalog = data.catalogNrn;
            catalogString = "NB: ";
            break;
        }

        // add components
        for (int i = 0; i < currCatalog.size(); ++i) {

            // get reference for component
            QSharedPointer<Component> component = currCatalog[i];

            // create the text for this component
            QString title = catalogString;
            title += component->name;

            // see if the component matches...
            if (title == current->text())
                selectedComponent = component;

            // break out if we have a match
            if (!selectedComponent.isNull())
                break;
        }

        // break out if we have a match
        if (!selectedComponent.isNull())
            break;
    }

    // migrate the system to the new component!
    if (viewCL.root != NULL) {
        viewCL.root->alPtr->updateFrom(viewCL.root->al);
        data.replaceComponent(viewCL.root->alPtr, viewCL.root->alPtr);
    }


    /*if (selectedComponent->editedVersion != NULL) {
        // in the process of being edited so load the edited version
        initialiseModel(selectedComponent->editedVersion);
        // but still point to the original
        viewCL.root->alPtr = selectedComponent;
        // activate the undostack
        selectedComponent->undoStack.setActive(true);
    } else {*/
        // not currently edited
        initialiseModel(selectedComponent);
        viewCL.root->alPtr = selectedComponent;
        // add undo to undoGroup
        this->undoStacks->addStack(&selectedComponent->undoStack);
        this->undoStacks->setActiveStack(&selectedComponent->undoStack);
    //}

    // update fileList
    viewCL.fileList->disconnect();
    addComponentsToFileList();
    connect(viewCL.fileList, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), this, SLOT(fileListItemChanged(QListWidgetItem*,QListWidgetItem*)));

    //center display
    viewCL.display->centerOn(viewCL.display->scene()->itemsBoundingRect().center());

    // update title
    updateTitle(true);
}

void MainWindow::addComponentsToFileList()
{
    // remove existing components in the list:
    QListWidgetItem * item;
    item = viewCL.fileList->takeItem(0);
    while (item) {
        delete item;
        item = viewCL.fileList->takeItem(0);
    }

    // select catalog
    QVector < QSharedPointer<Component> > currCatalog;
    QString catalogString;
    for (int catNum = 0; catNum < 3; ++catNum) {

        switch (catNum) {
        case 0:
            currCatalog = data.catalogWU;
            catalogString = "WU: ";
            break;
        case 1:
            currCatalog = data.catalogPS;
            catalogString = "PS: ";
            break;
        case 2:
            currCatalog = data.catalogNrn;
            catalogString = "NB: ";
            break;
        }

        // add components
        for (int i = 1; i < currCatalog.size(); ++i) {

            // get reference for component
            QSharedPointer<Component> component = currCatalog[i];

            // create the text for this component
            QString title = catalogString;
            title += component->name;

            // create the item
            QListWidgetItem * newItem = new QListWidgetItem;
            newItem->setText(title);
            viewCL.fileList->addItem(newItem);

            // are we the currently edited component?
            if (viewCL.root != NULL)
                if (viewCL.root->alPtr == component)
                    viewCL.fileList->setCurrentItem(newItem);

            // set the background colour depending on component state
            if (data.isComponentInUse(component)) {

                // component is used in the model - color blue
                newItem->setBackgroundColor(QColor(200,200,255,255));

            } else {

                // check if the original (or if modified edited) component validates
                if (component->editedVersion == NULL)
                    component->validateComponent();
                else
                    component->editedVersion->validateComponent();
                QSettings settings;
                int num_errs = settings.beginReadArray("errors");
                settings.endArray();
                num_errs += settings.beginReadArray("warnings");
                settings.endArray();

                settings.remove("errors");
                settings.remove("warnings");

                // red for not valid / green for valid / orange for edited
                if (num_errs != 0)
                    newItem->setBackgroundColor(QColor(255,200,200,255));
                else {
                    if (component->undoStack.isClean())
                        newItem->setBackgroundColor(QColor(200,255,200,255));
                    else
                        newItem->setBackgroundColor(QColor(255,255,200,255));
                }
            }

            // are we the currently edited component?
            if (viewCL.root != NULL)
                if (viewCL.root->alPtr == component)
                    newItem->setBackgroundColor(QColor(255,255,255,255));
        }

    }
}

void MainWindow::undoOrRedoPerformed(int)
{
    updateTitle();
}

// the actions for the menu
void MainWindow::createActions()
{
    connect(ui->actionImport_network, SIGNAL(triggered()), this, SLOT(import_network()));
    connect(ui->actionImport_neuron, SIGNAL(triggered()), this, SLOT(import_component()));
    connect(ui->actionExport_Component, SIGNAL(triggered()), this, SLOT(export_component()));
    connect(ui->actionSave_component_as, SIGNAL(triggered()), this, SLOT(export_component()));
    connect(ui->actionExport_layout, SIGNAL(triggered()), this, SLOT(export_layout()));
    connect(ui->actionImport_layouts, SIGNAL(triggered()), this, SLOT(import_layout()));
    connect(ui->actionImport_model, SIGNAL(triggered()), this, SLOT(import_project()));
    connect(ui->actionExport_model, SIGNAL(triggered()), this, SLOT(export_project()));
    connect(ui->actionSave_project_as, SIGNAL(triggered()), this, SLOT(export_project_as()));
    connect(ui->actionDelete_current_selection, SIGNAL(triggered()), this, SLOT(actionDeleteItems_triggered()));
    connect(ui->actionImport_CSV, SIGNAL(triggered()), this, SLOT(import_csv()));
    connect(ui->actionExit, SIGNAL(triggered()), this, SLOT(close()));
    connect(ui->actionNew_model, SIGNAL(triggered()), this, SLOT(new_project()));
    connect(ui->actionSave_Image, SIGNAL(triggered()), this, SLOT(saveImageAction()));
    connect(ui->actionEdit_Simulators, SIGNAL(triggered()), this, SLOT(launchSimulatorEditor()));
    connect(ui->actionNew, SIGNAL(triggered()), this, SLOT(actionNew_triggered()));
    connect(ui->action_Close_project, SIGNAL(triggered()), this, SLOT(close_project()));

    connect(ui->actionCommit_model, SIGNAL(triggered()), this, SLOT(actionCommitModel_triggered()));
    connect(ui->action_Update_model, SIGNAL(triggered()), this, SLOT(actionUpdateModel_triggered()));
    connect(ui->action_Revert_model, SIGNAL(triggered()), this, SLOT(actionRevertModel_triggered()));
    connect(ui->actionRepository_status, SIGNAL(triggered()), this, SLOT(actionRepStatus_triggered()));
    connect(ui->actionRepository_log, SIGNAL(triggered()), this, SLOT(actionRepLog_triggered()));
    connect(ui->actionRe_scan_for_VCS, SIGNAL(triggered()), this, SLOT(actionRescanVCS_triggered()));

    connect(ui->actionDuplicate_experiment, SIGNAL(triggered()), this, SLOT(actionDuplicate_experiment_triggered()));
    // actionRun_experiment is connected up when it is available.

    connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(about()));

    connect(ui->action_Copy_objects, SIGNAL(triggered()), &data, SLOT(copySelectionToClipboard()));
    connect(ui->actionPaste_objects, SIGNAL(triggered()), &data, SLOT(pasteSelectionFromClipboard()));

}

void MainWindow::new_project()
{

    // check for unsaved changes?: if (isChanged()) { promptToSave(); }

    // create new project
    projectObject * newProject = new projectObject();

    newProject->name = "Untitled Project";

    // deselect current project
    data.currProject->deselect_project(&data);
    if (this->viewVZ.OpenGLWidget != NULL) {
        this->viewVZ.OpenGLWidget->clear();
    }

    // add & select new project
    data.projects.push_back(newProject);
    newProject->select_project(&data);

    emit updatePanel(&data);

    // redraw
    this->ui->viewport->changed = 1;

    data.cursor.x = 0;
    data.cursor.y = 0;

    // add to undostacks
    undoStacks->addStack(newProject->undoStack);

    // move to viewNL
    updateNetworkButtons(&data);

    this->setProjectMenu();
    // No need to setExperimentMenu; there are no experiments in a new project.

    if (data.projects.size() > 1)
        ui->action_Close_project->setEnabled(true);
    else
        ui->action_Close_project->setEnabled(false);

    this->updateTitle();
}

void MainWindow::clear_recent_projects()
{
    QSettings settings;
    settings.beginGroup("mainwindow/recentprojects");
    settings.remove("");
    settings.endGroup();

    this->setupRecentProjectsMenu (&settings);
}

void MainWindow::import_recent_project()
{
    // Get filename from the sender action:
    QAction* sndr = (QAction*)QObject::sender();
    // And import:
    this->import_project (sndr->text());
}

QString MainWindow::getLastDirectory(const QString& settingsPath)
{
    QSettings settings;
    QString lastDirectory = settings.value (settingsPath).toString();
    if (lastDirectory.length() == 0) { lastDirectory = qgetenv("HOME"); }
    return lastDirectory;
}

void MainWindow::import_project()
{
    // check for unsaved changes?: if (isChanged()) { promptToSave(); }
    QString lastDirectory = this->getLastDirectory(MAINWINDOW_LASTPROJECTDIR);
    QString filePath = QFileDialog::getOpenFileName(this,
                                                    tr("Choose the Project to load"),
                                                    lastDirectory,
                                                    tr("Project files (*.proj);; All files (*)"));
    this->import_project (filePath);
}

void MainWindow::import_project(const QString& filePath)
{
    DBG() << "called";

    if (filePath.isEmpty()) {
        return;
    }

    projectObject * newProject = new projectObject();

    if (newProject->open_project(filePath)) {

        // success!
        data.projects.push_back(newProject);
        data.currProject->deselect_project(&data);
        newProject->select_project(&data);

        // check for version control
        data.currProject->version.setupVersion();

        // add to undostacks
        undoStacks->addStack(newProject->undoStack);

    } else {
        // failure - delete
        delete newProject;
        // go straight to jail, do not pass GO, do not collect £200
        return;
    }

    // clear viewVZ
    if (this->viewVZ.OpenGLWidget != NULL) {
        viewVZ.OpenGLWidget->clear();
    }

    // redraw
    this->ui->viewport->changed = 1;
    configureVCSMenu();

    updateNetworkButtons(&data);

    this->setProjectMenu();
    this->setExperimentMenu();

    if (data.projects.size() > 1) {
        ui->action_Close_project->setEnabled(true);
    } else {
        ui->action_Close_project->setEnabled(false);
    }

    // Update the recent projects menu
    this->updateRecentProjects(filePath);
    this->updateTitle();

    // Initialise the graph view for the default experiment of the
    // newly-imported project.
    experiment* e = this->getCurrentExpt();
    if (e != (experiment*)0) {
        this->initViewGV (e);
    } else {
        DBG() << "Can't init viewGV; no current experiment";
    }

    // This will re-paint the graph window if necessary.
    this->viewGVreshow();
}

void MainWindow::export_project(const QString& filePath)
{
    if (filePath.isEmpty()) {
        return;
    }

    // Check here to see if there is already a project file in this
    // directory, and warn the user that you can't save two projects
    // in one directory.
    QDir d(filePath);
    d.cdUp();
    // Get all files in QDir
    QStringList nf;
    nf << "*.proj";
    QStringList l = d.entryList(nf, QDir::Files);

    QStringList::const_iterator iter = l.constBegin();
    while (iter != l.constEnd()) {
        QString testPath = d.path() + QDir::separator() + *iter;
        if (testPath == filePath) {
            // Skip this
            // qDebug() << "This file is ok - it's going to be over-written: " << testPath;
        } else if (testPath == (filePath + ".proj")) {
            // Skip this too, because if user omitted the .proj, then
            // projectObject::save_project_file will append .proj. So
            // this is going to be overwritten, too.
        } else {
            // We have an alien project file here!
            if (this->emsg == (QErrorMessage*)0) {
                this->emsg = new QErrorMessage(this);
                QSize sz(250,200);
                this->emsg->setMinimumSize(sz);
            }
            this->emsg->setWindowTitle("Can't save two projects together");
            this->emsg->showMessage(
                tr("SpineCreator can't save more than one project in the each directory. A project consists of one .proj file and several .xml files. Two projects may have conflicting xml files, and hence we require that you save each project in a different directory.")
                );
            return;
        }
        ++iter;
    }

    QDir project_dir(filePath);
    QSettings settings;
    settings.setValue("files/currentFileName", project_dir.absolutePath());
    this->data.currProject->save_project(filePath, &this->data);

    // Clean up the component undostack (the project itself doesn't
    // have access to this as far as I can see (Seb).
    if (viewCL.root != NULL && viewCL.root->alPtr != NULL) {
        viewCL.root->alPtr->undoStack.setClean();
    }

    // enable / disable menus
    this->configureVCSMenu();
    // Update the recent projects menu
    this->updateRecentProjects(filePath);
}

void MainWindow::export_project()
{
    QString path = data.currProject->filePath;
    QString fileName = path;
    if (path.size() == 0) {
        path = this->getLastDirectory(MAINWINDOW_LASTPROJECTDIR); // defaults to HOME.
        fileName = QFileDialog::getSaveFileName(this, "Choose the Directory to save project in", path, tr("Project files (*.proj);; All files (*)"));
        if (fileName.isEmpty()) {
            return;
        }
    }
    if (!fileName.contains(".")) {
        fileName.append(".proj");
    }
    this->export_project (fileName);
}

void MainWindow::export_project_as()
{
    QString path = data.currProject->filePath;
    QString fileName = path;
    if (path.size() == 0) {
        path = this->getLastDirectory(MAINWINDOW_LASTPROJECTDIR); // defaults to HOME.
    }
    fileName = QFileDialog::getSaveFileName(this, "Choose the Directory to save project in", path, tr("Project files (*.proj);; All files (*)"));
    if (fileName.isEmpty()) {
        return;
    }
    if (!fileName.contains(".")) {
        fileName.append(".proj");
    }
    this->export_project (fileName);
}

void MainWindow::close_project()
{
    // check
    if (data.currProject->isChanged(&data)) {
        if (!promptToSave()) {
            // abort!
            return;
        }
    }

    // close the current project
    // find current project
    for (int i = 0; i < data.projects.size(); ++i) {
        if (data.projects[i] == data.currProject) {

            // find another project to switch to:
            data.currProject->deselect_project(&data);
            if (i == 0) {
                data.projects[1]->select_project(&data);
            } else {
                data.projects[0]->select_project(&data);
            }

            DBG() << "Before cleanup, viewGV has size " << this->viewGV.size();
            // Before deleting the project, remove all the viewGV
            // entries which point to the experiments in the project.
            QVector<experiment*>::iterator ie = data.projects[i]->experimentList.begin();
            int expnum = 0;
            while (ie != data.projects[i]->experimentList.end()) {
                // remove from viewGV
                if (this->viewGV[*ie] != (viewGVstruct*)0) {
                    // Cleanup memory which is the responsibility of the viewGVstruct in this->viewGV:
                    viewGVstruct* vgv = this->viewGV[*ie];
                    this->cleanupViewGV (vgv);
                    // delete the viewGVstruct object:
                    delete vgv;
                    // then remove the pointer from the QMap:
                    this->viewGV.remove(*ie);
                    DBG() << "Removed viewGV for expt " << expnum << " in this project";
                } else {
                    DBG() << "There is no viewGV for expt " << expnum << " in this project";
                }
                ++ie;
                ++expnum;
            }
            DBG() << "After cleanup, viewGV has size " << this->viewGV.size();

            // now delete the project. First delete the thing pointed
            // to by the pointer in the QVector<projectObject*>
            // data.projects:
            delete data.projects[i];
            // then remove the pointer itself from data.projects:
            data.projects.erase(data.projects.begin()+i);
        }
    }

    // Update the project menu as we may have removed a project from
    // within the list:
    this->setProjectMenu();
    this->setExperimentMenu();

    if (data.projects.size() > 1) {
        ui->action_Close_project->setEnabled(true);
    } else {
        ui->action_Close_project->setEnabled(false);
    }

    this->data.redrawViews();

    this->updateTitle();
}

void MainWindow::import_network()
{
    QString lastDir = this->getLastDirectory (MAINWINDOW_LASTNETWORKDIR);
    QString fileName = QFileDialog::getOpenFileName(this, tr("Choose the network file"), lastDir, tr("XML files (*.xml);; All files (*)"));

    if (fileName.isEmpty()) {
        return;
    }

    projectObject * newProject = data.currProject;

    // make sure we update the latest data
    newProject->copy_back_data(&data);

    // store old name
    QString name = newProject->name;

    if (newProject->import_network(fileName, data.cursor)) {
        // success!
        // put new data back
        newProject->copy_out_data(&data);
        // put back old name
        newProject->name = name;
        // check for version control
        data.currProject->version.setupVersion();
        // clear viewVZ
        if (this->viewVZ.OpenGLWidget != NULL) {
            viewVZ.OpenGLWidget->clear();
        }
        // redraw
        this->ui->viewport->changed = 1;
        configureVCSMenu();
        updateNetworkButtons(&data);
        this->setProjectMenu();
        this->setExperimentMenu();

    } else {
        // failure - delete newProject was commented out here.
    }

    // Regardless of success or failure, save the directory in which
    // the user chose the file.
    QDir lastDirectory (fileName);
    lastDirectory.cdUp();
    QSettings settings;
    settings.setValue (MAINWINDOW_LASTNETWORKDIR, lastDirectory.absolutePath());
}

void MainWindow::export_network()
{
    QSettings settings;
    QString lastDir = this->getLastDirectory (MAINWINDOW_LASTNETWORKDIR);
    QString path = settings.value("files/currentFileName", lastDir).toString();
    QString fileName = path;
    if (path == lastDir) {
        fileName = QFileDialog::getExistingDirectory(this, "Choose the Directory to save network in", path);
    }

    if (fileName.isEmpty()) {
        return;
    }

    emit export_model_xml(fileName);

    // enable / disable menus
    configureVCSMenu();

    // Save lastDir
    QDir lastDirectory (fileName);
    lastDirectory.cdUp();
    settings.setValue (MAINWINDOW_LASTNETWORKDIR, lastDirectory.absolutePath());
}


void MainWindow::import_csv()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open CSV file for import"), qgetenv("HOME"), tr("CSV files (*.csv *.txt);; All files (*)"));
    emit import_csv_signal(fileName);
}

void MainWindow::duplicate_component()
{
    // find which catalog we are saving to
    QVector<QSharedPointer<Component> >* curr_lib = (QVector<QSharedPointer<Component> >*)0;
    if (viewCL.root->al->type == "neuron_body")
        curr_lib = &data.catalogNrn;
    if (viewCL.root->al->type == "weight_update")
        curr_lib = &data.catalogWU;
    if (viewCL.root->al->type == "postsynapse")
        curr_lib = &data.catalogPS;
    if (viewCL.root->al->type == "generic_component")
        curr_lib = &data.catalogUnsorted;

    // find unique name
    int val = 1;
    bool unique = false;
    // see if there is a component with the same name
    while (!unique) {
        unique = true;
        for (int i = 0; i < curr_lib->size(); ++i) {
            if ((*curr_lib)[i]->name == viewCL.root->al->name + QString::number(float(val))) {
                unique = false;
            }
        }
        if (!unique) {
            ++val;
        }
    }

    // duplicate
    curr_lib->push_back(QSharedPointer<Component> (new Component(viewCL.root->al)));
    curr_lib->back()->name += QString::number(float(val));


    // update the file list
    viewCL.fileList->disconnect();
    addComponentsToFileList();
    connect(viewCL.fileList, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), this, SLOT(fileListItemChanged(QListWidgetItem*,QListWidgetItem*)));

    // update other lists
    viewNL.layout->updatePanel(&data);
}


void MainWindow::import_component()
{
    // sync everything so we don't lose changes
    data.currProject->copy_back_data(&data);

    QString lastDir = this->getLastDirectory (MAINWINDOW_LASTCOMPONENTDIR);
    QStringList fileNames = QFileDialog::getOpenFileNames(this, tr("Open SpineML"), lastDir, tr("XML files (*.xml *.9ml);; All files (*)"));

    if (fileNames.isEmpty()) {
        return;
    }

    for (int i = 0; i < fileNames.size(); ++i) {
        data.currProject->import_component(fileNames[i]);
    }

    data.currProject->printIssues("Errors found in components");

    // make sure components appear
    data.currProject->copy_out_data(&data);

    // update the file list
    viewCL.fileList->disconnect();
    addComponentsToFileList();
    connect(viewCL.fileList, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), this, SLOT(fileListItemChanged(QListWidgetItem*,QListWidgetItem*)));

    // update other lists
    viewNL.layout->updatePanel(&data);

    // Save directory from the first of the fileNames.
    QDir lastDirectory (fileNames[0]);
    lastDirectory.cdUp();
    QSettings settings;
    settings.setValue (MAINWINDOW_LASTCOMPONENTDIR, lastDirectory.absolutePath());
}

void MainWindow::delete_component()
{
    if (viewCL.root != NULL) {
        viewCL.root->deleteComponent();
    }
}

void MainWindow::export_component()
{
    if (viewCL.root== NULL) {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setText("<P><b>No component selected for Export                  </b></P>");
        msgBox.setInformativeText("Select the component to Export in the Component Editor");
        msgBox.exec();
        return;
    }

    QString lastDir = this->getLastDirectory (MAINWINDOW_LASTCOMPONENTDIR);
    QString fileName = QFileDialog::getSaveFileName(this, "Save component " + viewCL.root->al->name + " as SpineML", lastDir, tr("XML files (*.xml *.9ml);; All files (*)"));

    if (fileName.isEmpty()) {
        return;
    }

    // Save lastDir, regardless of whether the file is successfully imported
    QDir lastDirectory (fileName);
    lastDirectory.cdUp();
    QSettings settings;
    settings.setValue (MAINWINDOW_LASTCOMPONENTDIR, lastDirectory.absolutePath());

    if (!fileName.contains(".")) {
        fileName.append(".xml");
    }

    QFile file(fileName);
    if (!file.open( QIODevice::WriteOnly)) {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setText("Error creating file!           ");
        msgBox.exec();
        return;
    }

    // get the 9ML description
    QDomDocument * doc = new QDomDocument( "SpineML" );
    viewCL.root->al->write(doc);

    // write out to files
    QTextStream stream( &file );
    stream << doc->toString();
    delete doc;
    doc = NULL;

    updateTitle(false);
}

void MainWindow::import_layout()
{
    // sync everything so we don't lose changes
    data.currProject->copy_back_data(&data);

    QString lastDir = this->getLastDirectory (MAINWINDOW_LASTLAYOUTDIR);
    QStringList fileNames = QFileDialog::getOpenFileNames(this, tr("Open SpineML layout"), lastDir, tr("XML files (*.xml *.9ml);; All files (*)"));

    if (fileNames.isEmpty()) {
        return;
    }

    for (int i = 0; i < fileNames.size(); ++i) {
        data.currProject->import_layout(fileNames[i]);
    }

    data.currProject->printIssues("Errors found in layouts");

    // make sure layouts appear
    data.currProject->copy_out_data(&data);

    if (this->viewVZ.OpenGLWidget) {
        viewVZhandler->updateLayoutList(&data);
    }

    // Save the directory from the first of the fileNames
    QDir lastDirectory (fileNames[0]);
    lastDirectory.cdUp();
    QSettings settings;
    settings.setValue (MAINWINDOW_LASTLAYOUTDIR, lastDirectory.absolutePath());
}

void MainWindow::export_layout()
{
    // get layout to export

    // dialog
    QInputDialog * dialog = new QInputDialog(this);
    dialog->setOption(QInputDialog::UseListViewForComboBoxItems, true);
    dialog->setLabelText("Choose a layout to output");

    // add layouts to list and add to dialog
    QStringList options;
    for (int i = 1; i < data.catalogLayout.size(); ++i) {
        options <<  data.catalogLayout[i]->name;
    }
    dialog->setComboBoxItems(options);

    int index = -1;

    // launch
    dialog->setModal(true);

    if (dialog->exec()) {
        for (int i = 1; i < data.catalogLayout.size(); ++i) {
            if (dialog->textValue() == data.catalogLayout[i]->name) {
                index = i;
            }
        }
    } else {
        return;
    }

    if (index == -1) {
        return;
    }

    QString lastDir = this->getLastDirectory (MAINWINDOW_LASTLAYOUTDIR);
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save layout as NineML"), lastDir, tr("XML files (*.xml *.9ml);; All files (*)"));

    if (fileName.isEmpty()) {
        return;
    }

    // Save lastDir
    QDir lastDirectory (fileName);
    lastDirectory.cdUp();
    QSettings settings;
    settings.setValue (MAINWINDOW_LASTLAYOUTDIR, lastDirectory.absolutePath());

    if (!fileName.contains(".")) {
        fileName.append(".xml");
    }

    QFile file(fileName);
    if (!file.open( QIODevice::WriteOnly)) {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setText("Error creating file!           ");
        msgBox.exec();
        return;
    }

    // get the 9ML description
    QDomDocument * doc = new QDomDocument( "SpineML" );
    data.catalogLayout[index]->write(doc);

    // write out to files
    QTextStream stream( &file );
    stream << doc->toString();
    delete doc;
    doc = NULL;
}


void MainWindow::saveImageAction()
{
    if (viewCL.frame->isVisible()) {
        // components
        actionAs_Image_triggered();
    } else if (viewVZ.OpenGLWidget != NULL && viewVZ.view->isVisible()) {
        // visualisation
        //if (!fileName.isEmpty()) {
            saveNetworkImageDialog svImDiag(viewVZ.OpenGLWidget, "");
            svImDiag.exec();
        //}
    } else {
        // network
        //QString fileName = QFileDialog::getSaveFileName(this, tr("Export As Image"), "", tr("Png (*.png)"));
        data.saveImage("");
    }
}

bool MainWindow::viewGVvisible (void)
{
    DBG() << "Called to see if the Graph View (GV) is currently visible.";
    bool rtn(false);
    DBG() << "this->viewGV has size " << this->viewGV.size();
    QMap<experiment*, viewGVstruct*>::iterator vgvi = this->viewGV.begin();
    while (vgvi != this->viewGV.end()) {
        if (vgvi.value() != (viewGVstruct*)0
            && vgvi.value()->subWin->isVisible() == true) {
            rtn = true;
            break;
        }
        ++vgvi;
    }

    if (this->emptyGV.subWin->isVisible() == true) {
        rtn = true;
    }

    DBG() << "returning " << rtn;
    return rtn;
}

void MainWindow::viewGVreshow (void)
{
    // IF we're in the viewGV, then remove the old and show the
    // new. This is required when switching projects or experiments
    // whilst in the viewGV.
    if (this->viewGVvisible() == true) {
        this->hideViewGV();
        this->viewGVshow();
    }
}

void MainWindow::viewGVshow()
{
    DBG() << "called";

    experiment* e = this->getCurrentExpt();
    if (e != (experiment*)0
        && this->existsViewGV(e) == false) {
        DBG() << "This experiment needs a viewGV to be initialised...";
        this->initViewGV(e);
    }

    // reset all view buttons to 'inactive' look
    this->ui->tab0->setStyleSheet("QToolButton { border: 0px; color:white; background:QColor(0,0,0,0); }");
    this->ui->tab1->setStyleSheet("QToolButton { border: 0px; color:white; background:QColor(0,0,0,0); }");
    this->ui->tab2->setStyleSheet("QToolButton { border: 0px; color:white; background:QColor(0,0,0,0); }");
    this->ui->tab3->setStyleSheet("QToolButton { border: 0px; color:white; background:QColor(0,0,0,0); }");
    this->ui->tab4->setStyleSheet("QToolButton { border: 0px; color:white; background:QColor(0,0,0,0); }");

    // set the current view button to the 'active' look
    this->ui->tab4->setStyleSheet("QToolButton { border: 0px; color:white; background:rgba(255,255,255,40%); }");

    // make sure open component is replaced in the network
    if (viewCL.root != NULL) {
        viewCL.root->alPtr->updateFrom(viewCL.root->al);
        data.replaceComponent(viewCL.root->alPtr, viewCL.root->alPtr);
    }

    // hide the other views
    this->ui->view1->hide();
    this->viewEL.view->hide();
    if (this->viewVZ.OpenGLWidget != NULL) {
        this->viewVZ.view->hide();
        // save the tree state
        viewVZhandler->saveTreeState();
    }
    this->viewCL.frame->hide();
    if (viewCL.root != NULL) {
        this->viewCL.root->toolbar->hide();
        this->viewCL.root->addItemsToolbar->hide();
    }
    this->viewCL.dock->hide();

    // menus
    ui->menuBar->clear();
    ui->menuBar->addMenu(ui->menuFile);
    ui->menuBar->addMenu(ui->menuEdit);
    ui->menuBar->addMenu(ui->menuProject);

    ui->menuBar->addMenu(ui->menuExperiment);
    // Now add each experiment in the project...

    ui->menuBar->addMenu(ui->menuModel); // actually version control
    ui->menuBar->addMenu(ui->menuHelp);

    DBG() << "Current expt num is " << this->getCurrentExptNum();

    // show the relevant view for the selected experiment
    if (e != (experiment*)0) {
        this->viewGV[e]->subWin->show();
    } else {
        // Do a standard, empty setup?
        DBG() << "make graph area look empty in a nice way.";
        this->emptyGV.subWin->show();
    }

    QApplication::processEvents( QEventLoop::ExcludeUserInputEvents );
}

void MainWindow::viewELshow()
{
    DBG() << "called";

    // reset all view buttons to 'inactive' look
    this->ui->tab0->setStyleSheet("QToolButton { border: 0px; color:white; background:QColor(0,0,0,0); }");
    this->ui->tab1->setStyleSheet("QToolButton { border: 0px; color:white; background:QColor(0,0,0,0); }");
    this->ui->tab2->setStyleSheet("QToolButton { border: 0px; color:white; background:QColor(0,0,0,0); }");
    this->ui->tab3->setStyleSheet("QToolButton { border: 0px; color:white; background:QColor(0,0,0,0); }");
    this->ui->tab4->setStyleSheet("QToolButton { border: 0px; color:white; background:QColor(0,0,0,0); }");

    // make sure open component is replaced in the network
    if (viewCL.root != NULL) {
        viewCL.root->alPtr->updateFrom(viewCL.root->al);
        data.replaceComponent(viewCL.root->alPtr, viewCL.root->alPtr);
    }

    // set the current view button to the 'active' look
    this->ui->tab0->setStyleSheet("QToolButton { border: 0px; color:white; background:rgba(255,255,255,40%); }");

    // refresh it all
    this->viewELhandler->redraw();

    // hide the other views
    this->hideViewGV();

    this->ui->view1->hide();
    if (this->viewVZ.OpenGLWidget != NULL) {
        this->viewVZ.view->hide();
        // save the tree state
        viewVZhandler->saveTreeState();
    }
    this->viewCL.frame->hide();
    if (viewCL.root != NULL) {
        this->viewCL.root->toolbar->hide();
        this->viewCL.root->addItemsToolbar->hide();
    }
    this->viewCL.dock->hide();

    // menus
    ui->menuBar->clear();
    ui->menuBar->addMenu(ui->menuFile);
    ui->menuBar->addMenu(ui->menuEdit);
    ui->menuBar->addMenu(ui->menuProject);
    ui->menuBar->addMenu(ui->menuExperiment);
    ui->menuBar->addMenu(ui->menuModel); // actually version control
    ui->menuBar->addMenu(ui->menuHelp);

    // show current view
    this->viewEL.view->show();

    // titlebar
    updateTitle();

    // Data logs, if experiment has changed.
    //DBG() << "update logs as experiment layer has been selected.";
    DBG() << "updateDatas used to be called from here.";
    //this->updateDatas();

    QApplication::processEvents( QEventLoop::ExcludeUserInputEvents );
}

void MainWindow::hideViewGV (void)
{
    QMap<experiment*, viewGVstruct*>::iterator vgvi = this->viewGV.begin();
    while (vgvi != this->viewGV.end()) {
        if (vgvi.value() != (viewGVstruct*)0) {
            vgvi.value()->subWin->hide();
        }
        ++vgvi;
    }
    // Hide the emptyViewGV also:
    this->emptyGV.subWin->hide();
}

void MainWindow::viewNLshow()
{
    // reset all view buttons to 'inactive' look
    this->ui->tab0->setStyleSheet("QToolButton { border: 0px; color:white; background:QColor(0,0,0,0); }");
    this->ui->tab1->setStyleSheet("QToolButton { border: 0px; color:white; background:QColor(0,0,0,0); }");
    this->ui->tab2->setStyleSheet("QToolButton { border: 0px; color:white; background:QColor(0,0,0,0); }");
    this->ui->tab3->setStyleSheet("QToolButton { border: 0px; color:white; background:QColor(0,0,0,0); }");
    this->ui->tab4->setStyleSheet("QToolButton { border: 0px; color:white; background:QColor(0,0,0,0); }");

    // make sure open component is replaced in the network
    if (viewCL.root != NULL) {
        viewCL.root->alPtr->updateFrom(viewCL.root->al);
        data.replaceComponent(viewCL.root->alPtr, viewCL.root->alPtr);
    }

    // update the panel:
    this->data.reDrawPanel();

    // set the current view button to the 'active' look
    this->ui->tab1->setStyleSheet("QToolButton { border: 0px; color:white; background:rgba(255,255,255,40%); }");

    this->hideViewGV();

    this->viewEL.view->hide();
    if (this->viewVZ.OpenGLWidget != NULL) {
        this->viewVZ.view->hide();
        // save the tree state
        viewVZhandler->saveTreeState();
    }
    this->viewCL.frame->hide();
    if (viewCL.root != NULL) {
        this->viewCL.root->toolbar->hide();
        this->viewCL.root->addItemsToolbar->hide();
    }
    this->viewCL.dock->hide();

    // menus
    ui->menuBar->clear();
    ui->menuBar->addMenu(ui->menuFile);
    ui->menuBar->addMenu(ui->menuEdit);
    ui->menuBar->addMenu(ui->menuProject);
    ui->menuBar->addMenu(ui->menuModel); // actually version control
    ui->menuBar->addMenu(ui->menuHelp);

    // show current view
    this->ui->view1->show();

    // titlebar
    updateTitle();

    // redraw panel
    viewNL.layout->updatePanel(&data);

    //data.currProject->undoStack->setActive(true);
    this->undoStacks->setActiveStack(data.currProject->undoStack);

    // redraw
    this->ui->viewport->changed = 1;
    QApplication::processEvents( QEventLoop::ExcludeUserInputEvents );
}

void MainWindow::viewVZshow()
{

    if (viewVZ.OpenGLWidget == NULL) {
        initViewVZ();
        connectViewVZ();
    }

    // correct issues due to editing
    viewVZ.OpenGLWidget->refreshAll();

    // reset all view buttons to 'inactive' look
    this->ui->tab0->setStyleSheet("QToolButton { border: 0px; color:white; background:QColor(0,0,0,0); }");
    this->ui->tab1->setStyleSheet("QToolButton { border: 0px; color:white; background:QColor(0,0,0,0); }");
    this->ui->tab2->setStyleSheet("QToolButton { border: 0px; color:white; background:QColor(0,0,0,0); }");
    this->ui->tab3->setStyleSheet("QToolButton { border: 0px; color:white; background:QColor(0,0,0,0); }");
    this->ui->tab4->setStyleSheet("QToolButton { border: 0px; color:white; background:QColor(0,0,0,0); }");

    // make sure open component is replaced in the network
    if (viewCL.root != NULL) {
        viewCL.root->alPtr->updateFrom(viewCL.root->al);
        data.replaceComponent(viewCL.root->alPtr, viewCL.root->alPtr);
    }

    // set the current view button to the 'active' look
    this->ui->tab2->setStyleSheet("QToolButton { border: 0px; color:white; background:rgba(255,255,255,40%); }");

    // hide the other views
    this->hideViewGV();
    this->ui->view1->hide();
    this->viewEL.view->hide();
    this->viewCL.frame->hide();
    if (viewCL.root != NULL) {
        this->viewCL.root->toolbar->hide();
        this->viewCL.root->addItemsToolbar->hide();
    }
    this->viewCL.dock->hide();

    // clear away old stuff
    this->viewVZ.currObject = (QSharedPointer<systemObject>)0;
    this->viewVZhandler->clearAll();
    this->viewVZ.OpenGLWidget->clear();

    // configure TreeView
    if (!(viewVZ.sysModel == NULL)) {
        delete viewVZ.sysModel;
    }
    viewVZ.sysModel = new systemmodel(&data);
    viewVZ.treeView->setModel(viewVZ.sysModel);
    // connect for function
    connect(viewVZ.treeView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), viewVZhandler, SLOT(selectionChanged(QItemSelection,QItemSelection)));
    connect(viewVZ.treeView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this->viewVZ.OpenGLWidget, SLOT(selectionChanged(QItemSelection,QItemSelection)));


    connect(viewVZ.sysModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this->viewVZ.OpenGLWidget, SLOT(sysSelectionChanged(QModelIndex,QModelIndex)));

    // ok, there are three possibilities. Current base selection is a population, or a projection, or nothing is selected / there is nothing to select

    // menus
    ui->menuBar->clear();
    ui->menuBar->addMenu(ui->menuFile);
    ui->menuBar->addMenu(ui->menuEdit);
    ui->menuBar->addMenu(ui->menuProject);
    ui->menuBar->addMenu(ui->menuModel); // actually version control
    ui->menuBar->addMenu(ui->menuHelp);

    // unhide the current view
    this->viewVZ.view->show();
    this->viewVZhandler->redrawHeaders();

    // titlebar
    updateTitle();

    // put the tree state back
    viewVZhandler->restoreTreeState();

    viewVZ.OpenGLWidget->sysSelectionChanged(QModelIndex(), QModelIndex());

    QApplication::processEvents( QEventLoop::ExcludeUserInputEvents );

    // fix annoying OSX bug where opengl context hangs (hopefully!)
#ifdef Q_OS_MAC
    QList<int> sizes = viewVZ.view->sizes();
    sizes[0] -= 1;
    sizes[1] += 1;
    viewVZ.view->setSizes(sizes);
#endif
}

void MainWindow::viewCLshow()
{
    // reset all view buttons to 'inactive' look
    this->ui->tab0->setStyleSheet("QToolButton { border: 0px; color:white; background:QColor(0,0,0,0); }");
    this->ui->tab1->setStyleSheet("QToolButton { border: 0px; color:white; background:QColor(0,0,0,0); }");
    this->ui->tab2->setStyleSheet("QToolButton { border: 0px; color:white; background:QColor(0,0,0,0); }");
    this->ui->tab3->setStyleSheet("QToolButton { border: 0px; color:white; background:QColor(0,0,0,0); }");
    this->ui->tab4->setStyleSheet("QToolButton { border: 0px; color:white; background:QColor(0,0,0,0); }");

    // get source button handle
    //QToolButton * srcBut =  (QToolButton *) sender();

    // set the current view button to the 'active' look
    this->ui->tab3->setStyleSheet("QToolButton { border: 0px; color:white; background:rgba(255,255,255,40%); }");

    // hide the other views
    this->hideViewGV();
    this->ui->view1->hide();
    this->viewEL.view->hide();
    if (this->viewVZ.OpenGLWidget != NULL) {
        this->viewVZ.view->hide();
        // save the tree state
        viewVZhandler->saveTreeState();
    }

    // menus
    ui->menuBar->clear();
    ui->menuBar->addMenu(ui->menuFile);
    ui->menuBar->addMenu(ui->menuEdit);
    ui->menuBar->addMenu(ui->menuProject);
    ui->menuBar->addMenu(ui->menuNeuron);
    ui->menuBar->addMenu(ui->menuModel); // actually version control
    ui->menuBar->addMenu(ui->menuHelp);

    // show the current view
    this->viewCL.frame->show();
    if (viewCL.root != NULL) {
        this->viewCL.root->toolbar->show();
        this->viewCL.root->addItemsToolbar->show();
    }
    this->viewCL.dock->show();

    // titlebar
    updateTitle(unsaved_changes);

    // make sure we relect the changes in the network layer component usage
    viewCL.fileList->disconnect();
    addComponentsToFileList();
    connect(viewCL.fileList, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), this, SLOT(fileListItemChanged(QListWidgetItem*,QListWidgetItem*)));


    // set the current component's undoStack to active
    if (this->viewCL.root != NULL && this->viewCL.root->alPtr != NULL) {
        undoStacks->setActiveStack(&this->viewCL.root->alPtr->undoStack);
    }

    QApplication::processEvents( QEventLoop::ExcludeUserInputEvents );
}

void MainWindow::setCaption(QString caption)
{
    ui->caption->setText(caption);
}

void MainWindow::launchSimulatorEditor()
{
    settings_window * dialog  = new settings_window(this);
    dialog->show();

    // we may have changed the python scripts, so we should redraw NL and VZ
    if (this->viewVZ.OpenGLWidget) {
        this->viewVZhandler->redrawHeaders();
    }
    this->viewNL.layout->updatePanel(&data);
}


////////////////////////////////////////////// AL editor slots

void MainWindow::initialiseModel(QSharedPointer<Component> component)
{
    //todo: cleanup any old scene and root stuff
    if (viewCL.root != NULL) {
        delete viewCL.root;
        viewCL.root = NULL;
    }

    viewCL.root = new RootComponentItem(this, ui, component);
    viewCL.root->rootDataPtr = &data;

    if (!viewCL.frame->isVisible()) {
        viewCL.root->toolbar->hide();
        viewCL.root->addItemsToolbar->hide();
    }

    viewCL.display->setScene(viewCL.root->scene);
    viewCL.display->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::HighQualityAntialiasing);
    if (viewCL.propertiesContents->layout()) {
        delete viewCL.propertiesContents->layout();
    }
    viewCL.propertiesContents->setLayout(viewCL.root->properties);
    ui->actionShowHideParams->setChecked(true);
    ui->actionShowHidePorts->setChecked(true);

    //update layout
    viewCL.root->gvlayout->updateLayout();

    //set initial toolbar buttons for empty selection
    viewCL.root->properties->createEmptySelectionProperties();

    //connect to update title to notify of any unsaved changes
    connect(viewCL.root, SIGNAL(unsavedChange(bool)), this, SLOT(updateTitle(bool)));
}

void MainWindow::actionAddParamater_triggered()
{
    if (viewCL.root != NULL) {
        QSharedPointer<Component> oldComponent = QSharedPointer<Component> (new Component(viewCL.root->al));
        Parameter *p = new Parameter();
        p->name = "New_Parameter_";
        int n = 1;
        for (int i=0; i< viewCL.root->al->ParameterList.size(); i++) {
            QString name = viewCL.root->al->ParameterList[i]->getName();
            if (name.startsWith("New_Parameter_")) {
                QString temp = name.remove(0,14);
                int t = temp.toInt();
                if (t >= n) {
                    n = t+1;
                }
            }
        }
        char num[4];
        sprintf(num, "%i", n);
        p->name.append(num);
        viewCL.root->al->ParameterList.push_back(p);
        viewCL.root->scene->pl_item->addParameterItem(p);
        viewCL.root->gvlayout->updateLayout();
        viewCL.root->alPtr->undoStack.push(new changeComponent(this->viewCL.root, oldComponent, "Add Parameter"));
        updateTitle(true);
    }
}

void MainWindow::actionAddRegime_triggered()
{
    if (viewCL.root != NULL) {
        // store previous iteration for undo / redo
        QSharedPointer<Component> oldComponent = QSharedPointer<Component> (new Component(viewCL.root->al));
        Regime *r = new Regime();
        r->name = "New_Regime_";
        int n = 1;
        for (int i=0; i< viewCL.root->al->RegimeList.size(); i++)
        {
            QString name = viewCL.root->al->RegimeList[i]->name;
            if (name.startsWith("New_Regime_"))
            {
                QString temp = name.remove(0,11);
                int t = temp.toInt();
                if (t >= n)
                    n = t+1;
            }
        }
        //char num[4];
        //sprintf(num, "%i", n);
        r->name.append(QString::number(n));
        viewCL.root->al->RegimeList.push_back(r);
        RegimeGraphicsItem *rg = viewCL.root->scene->addRegimeItem(r);
        viewCL.root->gvlayout->updateLayout();
        viewCL.root->clearSelection();
        rg->setSelected(true);
        // do undo / redo
        viewCL.root->alPtr->undoStack.push(new changeComponent(this->viewCL.root, oldComponent, "Add Regime"));

        updateTitle(true);
    }
}

void MainWindow::actionAddOnCondition_triggered()
{
    if(viewCL.root) {
        viewCL.root->setSelectionMode(ModeInsertOnCondition);
    }
}

void MainWindow::actionAddOnEvent_triggered()
{
    if (viewCL.root) {
        viewCL.root->setSelectionMode(ModeInsertOnEvent);
    }
}

void MainWindow::actionAddOnImpulse_triggered()
{
    if (viewCL.root) {
        viewCL.root->setSelectionMode(ModeInsertOnImpulse);
    }
}


void MainWindow::actionSelectMode_triggered()
{
    if (viewCL.root) {
        viewCL.root->setSelectionMode(ModeSelect);
    }
}

void MainWindow::actionDeleteItems_triggered()
{
    if (viewCL.frame->isVisible()) {
        QSharedPointer<Component> oldComponent = QSharedPointer<Component> (new Component(viewCL.root->al));
        viewCL.root->scene->deleteSelectedItem();
        viewCL.root->alPtr->undoStack.push(new changeComponent(this->viewCL.root, oldComponent, "Delete selected"));
    }
    if (ui->view1->isVisible()) {
        data.deleteCurrentSelection();
    }

    experiment* e = this->getCurrentExpt();
    if (e != (experiment*)0) {
        if (this->viewGV[e]->subWin->isVisible()) {
            this->viewGV[e]->properties->deleteCurrentLog();
        }
    }
}

void MainWindow::actionAddTimeDerivative_triggered()
{
    if (viewCL.root) {
        QList <QGraphicsItem*> selected = viewCL.root->scene->selectedItems();
        if (selected.size() > 0) {
            QGraphicsItem *g = selected.first();
            if (g->type() == RegimeGraphicsItem::Type) {
                QSharedPointer<Component> oldComponent = QSharedPointer<Component> (new Component(viewCL.root->al));
                RegimeGraphicsItem *rgi = ((RegimeGraphicsItem*)g);
                TimeDerivative * td = new TimeDerivative();
                // shouldn't be validating a brand new td...
                // QStringList errs;
                // td->validateTimeDerivative(viewCL.root->al, &errs);
                rgi->regime->TimeDerivativeList.push_back(td);
                rgi->addTimeDerivativeItem(td);
                viewCL.root->gvlayout->updateLayout();
                viewCL.root->alPtr->undoStack.push(new changeComponent(this->viewCL.root, oldComponent, "Add TimeDerivative"));
                updateTitle(true);
            }
        } else {
            qDebug() << "Regime item should be selected during add time derivative!";
        }
    }
}

void MainWindow::actionAddAnalogePort_triggered()
{
    if (viewCL.root!= NULL) {
        QSharedPointer<Component> oldComponent = QSharedPointer<Component> (new Component(viewCL.root->al));
        PortListGraphicsItem *pli = viewCL.root->scene->portl_item;
        AnalogPort *ap = new AnalogPort();
        ap->mode = AnalogSendPort;
        // validate to set up pointers
        QStringList errs;
        ap->validateAnalogPort(viewCL.root->al.data(), &errs);
        // clear errors
        QSettings settings;
        settings.remove("errors");
        settings.remove("warnings");
        viewCL.root->al->AnalogPortList.push_back(ap);
        pli->addAnalogePortItem(ap);
        viewCL.root->gvlayout->updateLayout();
        viewCL.root->alPtr->undoStack.push(new changeComponent(this->viewCL.root, oldComponent, "Add Analog Port"));
    }
}

void MainWindow::actionAddEventPort_triggered()
{
    if (viewCL.root!= NULL) {
        QSharedPointer<Component> oldComponent = QSharedPointer<Component> (new Component(viewCL.root->al));
        PortListGraphicsItem *pli = viewCL.root->scene->portl_item;
        EventPort *ep = new EventPort();
        viewCL.root->al->EventPortList.push_back(ep);
        pli->addEventPortItem(ep);
        viewCL.root->gvlayout->updateLayout();
        viewCL.root->alPtr->undoStack.push(new changeComponent(this->viewCL.root, oldComponent, "Add Event Port"));
    }

}

void MainWindow::actionAddImpulsePort_triggered()
{
    if (viewCL.root!= NULL) {
        QSharedPointer<Component> oldComponent = QSharedPointer<Component> (new Component(viewCL.root->al));
        PortListGraphicsItem *pli = viewCL.root->scene->portl_item;
        ImpulsePort *ip = new ImpulsePort();
        viewCL.root->al->ImpulsePortList.push_back(ip);
        pli->addImpulsePortItem(ip);
        viewCL.root->gvlayout->updateLayout();
        viewCL.root->alPtr->undoStack.push(new changeComponent(this->viewCL.root, oldComponent, "Add Impulse Port"));
    }
}

void MainWindow::actionAddStateVariable_triggered()
{
    if (viewCL.root != NULL) {
        QSharedPointer<Component> oldComponent = QSharedPointer<Component> (new Component(viewCL.root->al));
        StateVariable * sv = new StateVariable();
        sv->name = "New_State_Var_";
        int n = 1;
        for (int i=0; i< viewCL.root->al->StateVariableList.size(); i++) {
            QString name = viewCL.root->al->StateVariableList[i]->getName();
            if (name.startsWith("New_State_Var_")) {
                QString temp = name.remove(0,14);
                int t = temp.toInt();
                if (t >= n) {
                    n = t+1;
                }
            }
        }
        char num[4];
        sprintf(num, "%i", n);
        sv->name.append(num);
        viewCL.root->al->StateVariableList.push_back(sv);
        viewCL.root->scene->pl_item->addStateVariableItem(sv);
        viewCL.root->gvlayout->updateLayout();
        viewCL.root->alPtr->undoStack.push(new changeComponent(this->viewCL.root, oldComponent, "Add State Variable"));
        updateTitle(true);
    }
}

void MainWindow::actionAddAlias_triggered()
{
    if (viewCL.root != NULL) {
        QSharedPointer<Component> oldComponent = QSharedPointer<Component> (new Component(viewCL.root->al));
        Alias * a = new Alias();
        a->name = "New_Alias_";
        int n = 1;
        for (int i=0; i< viewCL.root->al->AliasList.size(); i++) {
            QString name = viewCL.root->al->AliasList[i]->name;
            if (name.startsWith("New_Alias_")) {
                QString temp = name.remove(0,10);
                int t = temp.toInt();
                if (t >= n)
                    n = t+1;
            }
        }
        char num[4];
        sprintf(num, "%i", n);
        a->name.append(num);
        viewCL.root->al->AliasList.push_back(a);
        viewCL.root->scene->pl_item->addAliasItem(a);
        viewCL.root->gvlayout->updateLayout();
        viewCL.root->alPtr->undoStack.push(new changeComponent(this->viewCL.root, oldComponent, "Add Alias"));
        updateTitle(true);
    }
}

void MainWindow::actionAddStateAssignment_triggered()
{
    if (viewCL.root!= NULL) {
        QList <QGraphicsItem*> selected = viewCL.root->scene->selectedItems();
        if (selected.size() > 0) {
            QGraphicsItem *g = selected.first();
            StateAssignment *sa = new StateAssignment();
            QSharedPointer<Component> oldComponent = QSharedPointer<Component> (new Component(viewCL.root->al));

            if (g->type() == OnConditionGraphicsItem::Type) {
                OnConditionGraphicsItem *oci = ((OnConditionGraphicsItem*)g);
                oci->on_condition->StateAssignList.push_back(sa);
                oci->addStateAssignment(sa);

            } else if (g->type() == OnEventGraphicsItem::Type) {
                OnEventGraphicsItem *oei = ((OnEventGraphicsItem*)g);
                oei->on_event->StateAssignList.push_back(sa);
                oei->addStateAssignment(sa);
            } else if (g->type() == OnImpulseGraphicsItem::Type) {
                OnImpulseGraphicsItem *oii = ((OnImpulseGraphicsItem*)g);
                oii->on_impulse->StateAssignList.push_back(sa);
                oii->addStateAssignment(sa);
            }
            viewCL.root->gvlayout->updateLayout();
            viewCL.root->alPtr->undoStack.push(new changeComponent(this->viewCL.root, oldComponent, "Add State Assignment"));
            updateTitle(true);
        } else {
            qDebug() << "Transition item should be selected during add state assignment!";
        }
    }
}

void MainWindow::actionAddEventOut_triggered()
{
    if (viewCL.root!= NULL) {
        QList <QGraphicsItem*> selected = viewCL.root->scene->selectedItems();
        if (selected.size() > 0) {
            QGraphicsItem *g = selected.first();
            EventOut *eo = new EventOut();
            QSharedPointer<Component> oldComponent = QSharedPointer<Component> (new Component(viewCL.root->al));

            if (g->type() == OnConditionGraphicsItem::Type) {
                OnConditionGraphicsItem *oci = ((OnConditionGraphicsItem*)g);
                oci->on_condition->eventOutList.push_back(eo);
                oci->addEventOut(eo);
            } else if (g->type() == OnEventGraphicsItem::Type) {
                OnEventGraphicsItem *oei = ((OnEventGraphicsItem*)g);
                oei->on_event->eventOutList.push_back(eo);
                oei->addEventOut(eo);
            } else if (g->type() == OnImpulseGraphicsItem::Type) {
                OnImpulseGraphicsItem *oii = ((OnImpulseGraphicsItem*)g);
                oii->on_impulse->eventOutList.push_back(eo);
                oii->addEventOut(eo);
            }
            viewCL.root->gvlayout->updateLayout();
            viewCL.root->alPtr->undoStack.push(new changeComponent(this->viewCL.root, oldComponent, "Add EventOut"));
            updateTitle(true);
        } else {
            qDebug() << "Transition item should be selected during add event out!";
        }
    }
}

void MainWindow::actionAddImpulseOut_triggered()
{
    if (viewCL.root!= NULL) {
        QList <QGraphicsItem*> selected = viewCL.root->scene->selectedItems();
        if (selected.size() > 0) {
            QGraphicsItem *g = selected.first();
            ImpulseOut *io = new ImpulseOut();
            QSharedPointer<Component> oldComponent = QSharedPointer<Component> (new Component(viewCL.root->al));

            if (g->type() == OnConditionGraphicsItem::Type) {
                OnConditionGraphicsItem *oci = ((OnConditionGraphicsItem*)g);
                oci->on_condition->impulseOutList.push_back(io);
                oci->addImpulseOut(io);
            } else if (g->type() == OnEventGraphicsItem::Type) {
                OnEventGraphicsItem *oei = ((OnEventGraphicsItem*)g);
                oei->on_event->impulseOutList.push_back(io);
                oei->addImpulseOut(io);
            } else if (g->type() == OnImpulseGraphicsItem::Type) {
                OnImpulseGraphicsItem *oii = ((OnImpulseGraphicsItem*)g);
                oii->on_impulse->impulseOutList.push_back(io);
                oii->addImpulseOut(io);
            }
            viewCL.root->gvlayout->updateLayout();
            viewCL.root->alPtr->undoStack.push(new changeComponent(this->viewCL.root, oldComponent, "Add Impulse Out"));
            updateTitle(true);
        } else {
            qDebug() << "Transition item should be selected during add Impulse out!";
        }
    }
}

void MainWindow::actionZoomIn_triggered()
{
    viewCL.display->zoomIn();
}

void MainWindow::actionZoomOut_triggered()
{
    viewCL.display->zoomOut();
}

void MainWindow::actionAs_Image_triggered()
{
    if (viewCL.root != NULL) {
        QString fileName = QFileDialog::getSaveFileName(this, tr("Export As Image"), "", tr("Png (*.png)"));

        if (!fileName.isEmpty()) {
            QRectF view = viewCL.root->scene->itemsBoundingRect();
            ExportImageDialog image_dialog(view.width(), view.height(), this);
            int result = image_dialog.exec();
            if (result) {
                QImage image = QImage(image_dialog.getWidth(), image_dialog.getHeight(), QImage::Format_ARGB32_Premultiplied);
                QPainter p(&image);
                if (image_dialog.getAliasing())
                    p.setRenderHint(QPainter::Antialiasing);
                image.fill(Qt::white);
                viewCL.root->scene->render(&p, image.rect(), view, Qt::KeepAspectRatio);
                p.end();
                image.save(fileName, "png");
            }
        }
    } else {
        QMessageBox::critical( 0, "Export As Image Error:", QString("No File Open!"));
    }
}

void MainWindow::actionAs_Dotty_Graph_triggered()
{
    if (viewCL.root != NULL) {
        QString fileName = QFileDialog::getSaveFileName(this, tr("Export Dot Graph"), "", tr("Dot (*.dot)"));

        if (!fileName.isEmpty()) {
            DotWriter dot(viewCL.root);
            dot.writeDotFile(fileName);
        }
    } else {
        QMessageBox::critical( 0, "Export As Dot Graph Error:", QString("No File Open!"));
    }
}


void MainWindow::actionNew_triggered()
{
    //if (warnSave()) { // warnsave isn't needed as multiple files can be edited
    int val = 1;
    bool unique = false;
    // see if there is a component with the same name
    while (!unique) {
        unique = true;
        for (int i = 0; i < data.catalogNrn.size(); ++i) {
            if (data.catalogNrn[i]->name == "New Component " + QString::number(float(val)))
                unique = false;
        }
        if (!unique)
            ++val;
    }

    // add the new component
    data.catalogNrn.push_back(QSharedPointer<Component> (new Component()));
    data.catalogNrn.back()-> name = "New Component " + QString::number(float(val));
    initialiseModel(data.catalogNrn.back());
    viewCL.root->alPtr = data.catalogNrn.back();

    // redraw the file list
    viewCL.fileList->disconnect();
    addComponentsToFileList();
    connect(viewCL.fileList, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), this, SLOT(fileListItemChanged(QListWidgetItem*,QListWidgetItem*)));
}

void MainWindow::actionShowHideParams_triggered(bool checked)
{
    if (viewCL.root) {
        viewCL.root->scene->setParamsVisibility(!checked);
    }
}


void MainWindow::actionShowHidePorts_triggered(bool checked)
{
    if (viewCL.root) {
        viewCL.root->scene->setPortsVisibility(!checked);
    }
}

void MainWindow::actionMove_Up_triggered()
{
    if (viewCL.root) {
        QSharedPointer<Component> oldComponent = QSharedPointer<Component> (new Component(viewCL.root->al));
        viewCL.root->scene->moveItemUp();
        viewCL.root->alPtr->undoStack.push(new changeComponent(this->viewCL.root, oldComponent, "Change order"));
        updateTitle(true);
    }
}

void MainWindow::actionMove_Down_triggered()
{
    if (viewCL.root) {
        QSharedPointer<Component> oldComponent = QSharedPointer<Component> (new Component(viewCL.root->al));
        viewCL.root->scene->moveItemDown();
        viewCL.root->alPtr->undoStack.push(new changeComponent(this->viewCL.root, oldComponent, "Change order"));
        updateTitle(true);
    }
}

void MainWindow::actionCommitModel_triggered()
{
    data.currProject->version.commitVersion();
}

void MainWindow::actionUpdateModel_triggered()
{
    data.currProject->version.updateVersion();
}

void MainWindow::actionRevertModel_triggered()
{
    data.currProject->version.revertVersion();
}

void MainWindow::actionRepStatus_triggered()
{
    data.currProject->version.showVersionStatus();
}

void MainWindow::actionRepLog_triggered()
{
    data.currProject->version.showVersionLog();
}

void MainWindow::actionRescanVCS_triggered()
{
    data.currProject->version.detectVCSes();
    configureVCSMenu();
}

void MainWindow::actionDuplicate_experiment_triggered()
{
        // find the selected experiment
    for (int i = 0; i < this->data.experiments.size(); ++i)
    {
        if (this->data.experiments[i]->selected) {
            this->data.experiments.push_back(new experiment(this->data.experiments[i]));
            // select new experiment
            this->data.experiments.back()->select(&this->data.experiments);
            if (this->viewELhandler) {
                this->viewELhandler->redraw();
            }
            return;
        }
    }
}

void MainWindow::about()
{
    AboutDialog win;
    win.exec();
}

void MainWindow::configureVCSMenu()
{
    // show or hide menus
    if (data.currProject->version.haveVersion()) {
        ui->menuVersion_control->setEnabled(true);
    } else {
        ui->menuVersion_control->setEnabled(false);
    }
    // enable or disable menu items
    if (data.currProject->version.isModelUnderVersion()) {
        QList < QAction * > actions = ui->menuVersion_control->actions();
        for (int i = 0; i < actions.count(); ++i) {
            actions[i]->setEnabled(true);
        }
    } else {
        QList < QAction * > actions = ui->menuVersion_control->actions();
        for (int i = 0; i < actions.count(); ++i) {
            actions[i]->setEnabled(false);
        }
    }
}

void MainWindow::updateTitle(bool unsaved)
{
    // TITLE UPDATE FOR COMPONENT LAYER
    unsaved_changes = unsaved;
    if (viewCL.frame->isVisible()) {
        QString title = "SpineCreator: Component Editor";
        if(viewCL.root) {
            if (viewCL.root->alPtr != NULL) {
                title = title.prepend(" - ");
                if (viewCL.root->alPtr->path == "temp") {
                    title = title.prepend(" (" + viewCL.root->alPtr->filePath + ")");
                }
                title = title.prepend(viewCL.root->alPtr->name);
                if (!viewCL.root->alPtr->undoStack.isClean()) {
                    title = title.append("*");
                }
            }
        }
        this->setWindowTitle(title);
    }
}

void MainWindow::updateTitle()
{

    // TITLE UPDATE FOR NETWORK LAYER
    QString title;
    if (ui->view1->isVisible()) {
        title = "SpineCreator: Network Editor";
        title = title.prepend(" - ");
        title = title.prepend(data.currProject->name);
        if (!data.currProject->undoStack->isClean()) {
            title = title.append("*");
        }
        this->setWindowTitle(title);
    }
    if (this->viewVZ.OpenGLWidget != NULL) {
        if (viewVZ.view->isVisible()) {
            title = "SpineCreator: Visualiser";
            title = title.prepend(" - ");
            title = title.prepend(data.currProject->name);
            if (!data.currProject->undoStack->isClean()) {
                title = title.append("*");
            }
            this->setWindowTitle(title);
        }
    }
    if (viewEL.view->isVisible()) {
        title = "SpineCreator: Experiment Editor";
        this->setWindowTitle(title);
    }
    if (viewCL.frame->isVisible()) {
        QString title = "SpineCreator: Component Editor";
        if(viewCL.root) {
            if (viewCL.root->alPtr != NULL) {
                title = title.prepend(" - ");
                title = title.prepend(viewCL.root->alPtr->name);
                if (!viewCL.root->alPtr->undoStack.isClean()) {
                    title = title.append("*");
                }
            }
        }
        this->setWindowTitle(title);
    }
}

void MainWindow::updateNetworkButtons(nl_rootdata * data)
{
    if (data->selList.size() == 0) {
        ui->butA->setDisabled(false);
        ui->butB->setDisabled(true);
        ui->butSS->setDisabled(false);
        ui->butC->setDisabled(true);
    } else if (data->selList.size() == 1) {
        if (data->selList[0]->type == populationObject) {
            ui->butA->setDisabled(true);
            ui->butB->setDisabled(false);
            ui->butSS->setDisabled(true);
            ui->butC->setDisabled(false);
        } else {
            ui->butA->setDisabled(true);
            ui->butB->setDisabled(true);
            ui->butSS->setDisabled(true);
            ui->butC->setDisabled(false);
        }
    } else if (data->selList.size() > 1) {
        ui->butA->setDisabled(true);
        ui->butB->setDisabled(true);
        ui->butSS->setDisabled(true);
        ui->butC->setDisabled(false);
    }
}
