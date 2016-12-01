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
**           Author: Alex Cope                                            **
**  Website/Contact: http://bimpa.group.shef.ac.uk/                       **
****************************************************************************/

#include "SC_viewGVpropertieslayout.h"
#include "mainwindow.h"
#include "qcustomplot.h"
#include "globalHeader.h"

viewGVpropertieslayout::viewGVpropertieslayout(viewGVstruct * viewGVin, QWidget *parent) :
    QWidget(parent)
{
    // add pointer to viewGVstruct
    this->viewGV = viewGVin;

    // create toolbar
    this->createToolbar();

    // add layout
    this->setLayout(new QVBoxLayout);

    this->currentExperiment = (experiment*)0;

    this->vLogData.clear();

    // add widgets
    logList = new QListWidget;
    // set width to stop dock being resized too small
    logList->setMinimumWidth(200);
    logList->setSelectionMode(QAbstractItemView::SingleSelection);
    this->layout()->addWidget(new QLabel("Loaded logs"));
    this->layout()->addWidget(logList);
    dataIndexList = new QListWidget;
    dataIndexList->setSelectionMode(QAbstractItemView::MultiSelection);
    this->layout()->addWidget(new QLabel("Select log indices to plot"));
    this->layout()->addWidget(dataIndexList);
    typeList = new QListWidget;
    typeList->setSelectionMode(QAbstractItemView::SingleSelection);
    typeList->setMinimumHeight(40);
    typeList->setMaximumHeight(50);
    this->layout()->addWidget(new QLabel("Plot type"));
    this->layout()->addWidget(typeList);
    QPushButton * addPlot = new QPushButton("Add plot");
    this->layout()->addWidget(addPlot);
    addButton = addPlot;
    QPushButton * delLog = new QPushButton("Delete log file (PERMANENTLY)");
    this->layout()->addWidget(delLog);

    ((QVBoxLayout *)this->layout())->addStretch();

    // connect
    connect(viewGV->mdiarea, SIGNAL(subWindowActivated(QMdiSubWindow*)), this, SLOT(windowSelected(QMdiSubWindow*)));
    connect(logList, SIGNAL(currentRowChanged(int)), this, SLOT(logListSelectionChanged(int)));
    connect(addPlot, SIGNAL(clicked()), this, SLOT(addPlotToCurrent()));
    connect(delLog, SIGNAL(clicked()), this, SLOT(deleteCurrentLog()));
}

viewGVpropertieslayout::~viewGVpropertieslayout()
{
    for (int i = 0; i < vLogData.size(); ++i) {
        delete vLogData[i];
    }

    delete actionAddGraphSubWin;
    actionAddGraphSubWin = NULL;

    // delete actionToGrid;
    actionToGrid = NULL;
}

// configure the plot for use
void viewGVpropertieslayout::setupPlot(QCustomPlot * plot)
{
    // range
    plot->setInteraction(QCP::iRangeDrag, true);
    plot->setInteraction(QCP::iRangeZoom, true);

    // default to x-axis drag / zoom only
    plot->axisRect()->setRangeDrag(Qt::Horizontal);
    plot->axisRect()->setRangeZoom(Qt::Horizontal);

    // selections allowed
    plot->setInteraction(QCP::iSelectPlottables, true);
    plot->setInteraction(QCP::iSelectAxes, true);
    plot->setInteraction(QCP::iSelectLegend, true);

    // context menus for deleting graphs / reseting axes etc
    plot->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(plot, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenuRequest(QPoint)));

    // make bottom and left axes transfer their ranges to top and right axes:
    connect(plot->xAxis, SIGNAL(rangeChanged(QCPRange)), plot->xAxis2, SLOT(setRange(QCPRange)));
    connect(plot->yAxis, SIGNAL(rangeChanged(QCPRange)), plot->yAxis2, SLOT(setRange(QCPRange)));
}

void viewGVpropertieslayout::updateLogList()
{
    DBG() << "re-populating this->logList";
    disconnect(logList);
    logList->clear();
    for (int i = 0; i < vLogData.size(); ++i) {
        logList->addItem(vLogData[i]->logName);
    }
    connect(logList, SIGNAL(currentRowChanged(int)), this, SLOT(logListSelectionChanged(int)));
}

void viewGVpropertieslayout::clearLogs()
{
#ifdef __CLEAR_LOGS_EXCEPT_RENDERED_ONES__
    DBG() << "Clearing out logsForGraphs and datas (UI element). logsForGraphs size" << logsForGraphs.size();
    // Remove logs unless the log has a plot associated with it.
    QVector<logData*>::iterator j = this->logsForGraphs.begin();
    while (j != this->logsForGraphs.end()) {
        if ((*j) == (logData*)0) {
            //DBG() << "Null logData*, just erase it from logsForGraphs.";
            j = this->logsForGraphs.erase(j);
        } else {
            // Non-null logData*
            if ((*j)->hasPlot == false) {
                //DBG() << "logData has no plot, deleting from logsForGraphs...";
                delete (*j);
                //DBG() << "Erase from logsForGraphs...";
                j = this->logsForGraphs.erase(j);
                //DBG() << "After erase, logsForGraphs size" << logsForGraphs.size();
            } else {
                //DBG() << "This logData has a plot, keep it in the logsForGraphs list..";
                j++;
            }
        }
    }
#else // clear all/most logs
    QVector<logData*>::iterator j = this->vLogData.begin();
    while (j != this->vLogData.end()) {
        if ((*j) != (logData*)0) {
            if ((*j)->numPlots() == 0) {
                delete (*j);
            } // else don't delete the logData; it's memory is
              // maintained, and a pointer should exist inside one of
              // the experiment*s in the projectobject.
        }
        j = this->vLogData.erase(j);
    }
#endif
    //DBG() << "At end, logsForGraphs size" << logsForGraphs.size();
    this->updateLogList();
}

void viewGVpropertieslayout::loadLogDataFiles(QStringList fileNames, QDir * path)
{
    if (path) {
        DBG() << "called to load a list of files of size "
              << fileNames.size() << " with path " << path->absolutePath();
    } else {
        DBG() << "called to load a list of files of size "
              << fileNames.size();
    }

    // load the files
    for (int i = 0; i < fileNames.size(); ++i) {

        bool exists = false;

        QString logXMLname;
        if (path) {
            logXMLname = path->absoluteFilePath(fileNames[i]);
        } else {
            logXMLname = fileNames[i];
        }

        // check if we have the log
        //DBG() << "logsForGraphs size is " << this->logsForGraphs.size();
        for (int i = 0; i < this->vLogData.size(); ++i) {
            if (this->vLogData[i]->logFileXMLname == logXMLname) {
                DBG() "Refreshing existing log " << logXMLname << "...";
                this->refreshLog(vLogData[i]);
                exists = true;
            }
        }

        // otherwise...
        if (!exists) {
            DBG() << "Create a new logData object";
            logData * log = new logData();
            log->logFileXMLname = logXMLname;
            if (!log->setupFromXML()) {
                qDebug() << "Failed to read XML";
                delete log;
                continue;
            }
            DBG() << "Push back " << logXMLname << " onto logsForGraphs";
            vLogData.push_back(log);
        }
    }
    this->updateLogList();
}

void viewGVpropertieslayout::storeGraphs (void)
{
    DBG() << "Called";

    if (this->currentExperiment == (experiment*)0) {
        DBG() << "No currentExperiment; can't storeGraphs";
        return;
    }

    this->currentExperiment->clearGraphedLogs();

    QVector<logData*>::iterator j = this->vLogData.begin();
    while (j != this->vLogData.end()) {
        if ((*j) != (logData*)0 && (*j)->numPlots() > 0) {
            DBG() << "This logData has a plot or plots, add it to currentExperiment->graphedLogs.";
            // Close associated plot sub-windows
            this->currentExperiment->graphedLogs.append(*j);
            (*j)->closePlots(this->viewGV->mdiarea);
        }
        j++;
    }
}

void viewGVpropertieslayout::restoreGraphs (experiment* e)
{
    DBG() << "Called";

    if (!this->vLogData.isEmpty()) {
        DBG() << "ERROR: logsForGraphs needs to be emptied with clearLogs()";
        return;
    }

    if (e == (experiment*)0) {
        DBG() << "ERROR: need to be passed a non-null experiment*";
        return;
    }

    this->currentExperiment = e;

    if (this->currentExperiment->graphedLogs.isEmpty()) {
        DBG() << "INFO: graphedLogs is empty";
        return;
    }

    DBG() << "Restore graphedLogs for experiment " << this->currentExperiment->name;
    QVector<logData*>::iterator j = this->currentExperiment->graphedLogs.begin();
    while (j != this->currentExperiment->graphedLogs.end()) {
        DBG() << "Append pointer to existing logData onto logsForGraphs";
        this->vLogData.append (*j);

        // Now re-create the graph(s)
        QMap<QCustomPlot*, QMdiSubWindow*> mPlots = (*j)->getPlots();
        QMap<QCustomPlot*, QMdiSubWindow*>::iterator i = mPlots.begin();
        bool added = false;
        while (i != mPlots.end()) {
            DBG() << "addSubWindow for log " << (*j)->logName;
            QMdiSubWindow* mdiSubWin = this->viewGV->mdiarea->addSubWindow(i.key());
            if (mdiSubWin != NULL) {
                mdiSubWin->setVisible(true);
                // Add the mdiSubWin back into mPlots:
                mPlots[i.key()] = mdiSubWin;
                this->addLinesRasters (*j, mdiSubWin);
                added = true;
            }
            ++i;
        }
        if (added) {
            (*j)->addPlots (mPlots);
        }
        ++j;
    }
}

void viewGVpropertieslayout::addLinesRasters (logData* log, QMdiSubWindow* subWin)
{
    QCustomPlot* currPlot = (QCustomPlot*)subWin->widget();

    // loop through graphs in the plot
    for (int j = 0; j < currPlot->graphCount(); ++j) {
        // if the graph is from this log
        if (currPlot->graph(j)->property("source").toString() == log->logFileXMLname) {
            // extract remaining graph data
            QString type = currPlot->graph(j)->property("type").toString();
            if (type == "linePlot") {
                // get index
                int index = currPlot->graph(j)->property("index").toInt();
                log->plotLine(currPlot, subWin, index, j);

            } else if (type == "rasterPlot") {
                // get indices
                QList < QVariant > indices = currPlot->graph(j)->property("indices").toList();
                log->plotRaster(currPlot, subWin, indices, j);
            }
        }
    }
}

void viewGVpropertieslayout::deleteCurrentLog()
{
    // get the log index
    int dataIndex = logList->currentRow();

    // check that we have a selection
    if (dataIndex < 0) {
        return;
    }

    // delete the log file
    vLogData[dataIndex]->deleteLogFile();

    // remove the log
    logData * log = vLogData[dataIndex];
    vLogData.erase(vLogData.begin()+dataIndex);
    delete log;

    // refresh display
    updateLogList();
}

void viewGVpropertieslayout::removeSelectedGraph()
{
    // first get a pointer to the current plot!
    QCustomPlot * currPlot = (QCustomPlot *) currentSubWindow->widget();
    if (currPlot->selectedGraphs().size() > 0) {
        currPlot->removeGraph(currPlot->selectedGraphs().first());
        currPlot->replot();
    }
}

void viewGVpropertieslayout::removeAllGraphs()
{
    // first get a pointer to the current plot!
    QCustomPlot * currPlot = (QCustomPlot *) currentSubWindow->widget();
    currPlot->clearGraphs();
    currPlot->replot();
}

void viewGVpropertieslayout::toggleHorizontalZoom()
{
    // first get a pointer to the current plot!
    QCustomPlot * currPlot = (QCustomPlot *) currentSubWindow->widget();
    if (currPlot->axisRect()->rangeZoom() & Qt::Horizontal) {
        currPlot->axisRect()->setRangeZoom(currPlot->axisRect()->rangeZoom() & ~Qt::Horizontal);
    } else {
        currPlot->axisRect()->setRangeZoom(currPlot->axisRect()->rangeZoom() | Qt::Horizontal);
    }
}

void viewGVpropertieslayout::toggleHorizontalDrag()
{
    // first get a pointer to the current plot!
    QCustomPlot * currPlot = (QCustomPlot *) currentSubWindow->widget();
    if (currPlot->axisRect()->rangeDrag() & Qt::Horizontal) {
        currPlot->axisRect()->setRangeDrag(currPlot->axisRect()->rangeDrag() & ~Qt::Horizontal);
    } else {
        currPlot->axisRect()->setRangeDrag(currPlot->axisRect()->rangeDrag() | Qt::Horizontal);
    }
}

void viewGVpropertieslayout::toggleVerticalZoom()
{
    // first get a pointer to the current plot!
    QCustomPlot * currPlot = (QCustomPlot *) currentSubWindow->widget();
    if (currPlot->axisRect()->rangeZoom() & Qt::Vertical) {
        currPlot->axisRect()->setRangeZoom(currPlot->axisRect()->rangeZoom() & ~Qt::Vertical);
    } else {
        currPlot->axisRect()->setRangeZoom(currPlot->axisRect()->rangeZoom() | Qt::Vertical);
    }
}

void viewGVpropertieslayout::toggleVerticalDrag()
{
    // first get a pointer to the current plot!
    QCustomPlot * currPlot = (QCustomPlot *) currentSubWindow->widget();
    if (currPlot->axisRect()->rangeDrag() & Qt::Vertical) {
        currPlot->axisRect()->setRangeDrag(currPlot->axisRect()->rangeDrag() & ~Qt::Vertical);
    } else {
        currPlot->axisRect()->setRangeDrag(currPlot->axisRect()->rangeDrag() | Qt::Vertical);
    }
}

void viewGVpropertieslayout::rescaleAxes()
{
    // first get a pointer to the current plot!
    QCustomPlot * currPlot = (QCustomPlot *) currentSubWindow->widget();
    currPlot->rescaleAxes();
    currPlot->replot();
}

void viewGVpropertieslayout::contextMenuRequest(QPoint pos)
{
    // first get a pointer to the current plot!
    QCustomPlot * currPlot = (QCustomPlot *) currentSubWindow->widget();
    QMenu *menu = new QMenu(this);
    menu->setAttribute(Qt::WA_DeleteOnClose);

    if (currPlot->legend->selectTest(pos, false) >= 0) { // context menu on legend requested
#if 0
        menu->addAction("Move to top left", this, SLOT(moveLegend()))->setData((int)(Qt::AlignTop|Qt::AlignLeft));
        menu->addAction("Move to top center", this, SLOT(moveLegend()))->setData((int)(Qt::AlignTop|Qt::AlignHCenter));
        menu->addAction("Move to top right", this, SLOT(moveLegend()))->setData((int)(Qt::AlignTop|Qt::AlignRight));
        menu->addAction("Move to bottom right", this, SLOT(moveLegend()))->setData((int)(Qt::AlignBottom|Qt::AlignRight));
        menu->addAction("Move to bottom left", this, SLOT(moveLegend()))->setData((int)(Qt::AlignBottom|Qt::AlignLeft));
#endif
    } else if (currPlot->xAxis->selectTest(pos, false) >= 0
               || currPlot->xAxis2->selectTest(pos, false) >= 0) {
        // enable / disable zoom
        if (currPlot->axisRect()->rangeZoom() & Qt::Horizontal) {
            menu->addAction("Disable zoom on axis", this, SLOT(toggleHorizontalZoom()));
        } else {
            menu->addAction("Enable zoom on axis", this, SLOT(toggleHorizontalZoom()));
        }

        // enable / diable drag
        if (currPlot->axisRect()->rangeDrag() & Qt::Horizontal) {
            menu->addAction("Disable drag on axis", this, SLOT(toggleHorizontalDrag()));
        } else {
            menu->addAction("Enable drag on axis", this, SLOT(toggleHorizontalDrag()));
        }

    } else if (currPlot->yAxis->selectTest(pos, false) >= 0
               || currPlot->yAxis2->selectTest(pos, false) >= 0) {
        // enable / disable zoom
        if (currPlot->axisRect()->rangeZoom() & Qt::Vertical) {
            menu->addAction("Disable zoom on axis", this, SLOT(toggleVerticalZoom()));
        } else {
            menu->addAction("Enable zoom on axis", this, SLOT(toggleVerticalZoom()));
        }

        // enable / diable drag
        if (currPlot->axisRect()->rangeDrag() & Qt::Vertical) {
            menu->addAction("Disable drag on axis", this, SLOT(toggleVerticalDrag()));
        } else {
            menu->addAction("Enable drag on axis", this, SLOT(toggleVerticalDrag()));
        }
    } else {
        if (currPlot->graphCount() > 0) {
          menu->addAction("Scale axes to fit", this, SLOT(rescaleAxes()));
        }
        if (currPlot->selectedGraphs().size() > 0) {
          menu->addAction("Remove selected graph", this, SLOT(removeSelectedGraph()));
        }
        if (currPlot->graphCount() > 0) {
          menu->addAction("Remove all graphs", this, SLOT(removeAllGraphs()));
        }
    }

    menu->popup(currPlot->mapToGlobal(pos));
}

void viewGVpropertieslayout::logListSelectionChanged(int index)
{
    // use index to get log
    this->typeList->clear();
    this->dataIndexList->clear();

    // no s
    if (index == -1) {
        return;
    }

    // setup log indices
    if (vLogData[index]->dataClass == ANALOGDATA) {
        for (int i = 0; i < vLogData[index]->columns.size(); ++i) {
            this->dataIndexList->addItem("Index " + QString::number(vLogData[index]->columns[i].index));
        }
    } else if (vLogData[index]->dataClass == EVENTDATA) {
        for (int i = 0; i < vLogData[index]->eventIndices.size(); ++i) {
            this->dataIndexList->addItem("Index " + QString::number(vLogData[index]->eventIndices[i]));
        }
    }

    // setup plot types
    if (vLogData[index]->dataClass == ANALOGDATA) {

        // populate types with analog plot forms:
        this->typeList->addItem("Line plot");

        // Pre-select Line plot, as there is only one option.
        this->typeList->setCurrentItem(this->typeList->item(0));

        // For line plots, it's common to select one or a few traces
        // so don't select all items in this->indices.

    } else if (vLogData[index]->dataClass == EVENTDATA) {

        // populate types with event plot forms:
        this->typeList->addItem("Raster plot");
        // An alternative plot type is: this->types->addItem("Histogram");

        // Pre-select Raster plot, as there is only one option.
        this->typeList->setCurrentItem(this->typeList->item(0));

        // Typically we want to select all items for a raster plot, so do this here:
        this->dataIndexList->setSelectionMode(QAbstractItemView::ExtendedSelection); // or QAbstractItemView::MultiSelection?
        for (int i = 0; i < this->dataIndexList->count(); ++i) {
            this->dataIndexList->item(i)->setSelected(true);
        }
    }

    // Ensure the add-to-plot button is correctly enabled:
    addButton->setDisabled(false);
}

void viewGVpropertieslayout::addPlotToCurrent()
{
    // first get a pointer to the current plot!
    QCustomPlot* currPlot = (QCustomPlot*)currentSubWindow->widget();

    // get the log index
    int dataIndex = this->logList->currentRow();

    if (dataIndex < 0) {
        return;
    }

    // ok - if then time
    if (this->vLogData[dataIndex]->dataClass == ANALOGDATA) {
        if (typeList->currentRow() == 0) { // Line Plot
            QList < QListWidgetItem * > selectedItems = this->dataIndexList->selectedItems();
            for (int i = 0; i < selectedItems.size(); ++i) {
                int index = dataIndexList->row(selectedItems[i]);
                // now we have the row, draw the graph...
                DBG() << "Draw graph for dataIndex=" << dataIndex << " indices index=" << index;
                if (!this->vLogData[dataIndex]->plotLine(currPlot, currentSubWindow, index)) {
                    DBG() << "Oops, failed to plot";
                //} else {
                    //DBG() << "logsForGraphs[" << dataIndex << "]->plot=" << this->logsForGraphs[dataIndex]->plot;
                }
            }
        }
    }
    if (this->vLogData[dataIndex]->dataClass == EVENTDATA) {
        if (typeList->currentRow() == 0) { // Raster Plot
            QList < QListWidgetItem * > selectedItems = dataIndexList->selectedItems();
            QList < QVariant > indexList;
            for (int i = 0; i < selectedItems.size(); ++i) {
                indexList.push_back( dataIndexList->row(selectedItems[i]));
            }
            if (!this->vLogData[dataIndex]->plotRaster(currPlot, currentSubWindow, indexList)) {
                DBG() << "Oops, failed to plot";
            }
        }
    }
}

void viewGVpropertieslayout::windowSelected(QMdiSubWindow * window)
{
    if (window == NULL) {
        actionToGrid->setDisabled(true);
        actionSavePdf->setDisabled(true);
        actionSavePng->setDisabled(true);
        addButton->setDisabled(true);
    } else {
        actionToGrid->setDisabled(false);
        actionSavePdf->setDisabled(false);
        actionSavePng->setDisabled(false);
        addButton->setDisabled(false);
    }

    // get window list
    QList < QMdiSubWindow * > windowList = viewGV->mdiarea->subWindowList();

    for (int i = 0; i < windowList.count(); ++i) {
        if (windowList[i] == window) {
            currentSubWindow = windowList[i];
            /*QLabel * label = qobject_cast <QLabel *> (this->layout()->itemAt(0)->widget());
              if (label) { label->setText(QString::number(i)); } */
        }
    }
}

//////////////////////////// TOOLBAR ////////////////////////////////
void viewGVpropertieslayout::createToolbar()
{
    QCommonStyle style;

    // create actions
    actionAddGraphSubWin = new QAction(QIcon(":/icons/toolbar/images/new_file.png"),"",viewGV->mainwindow);
    actionAddGraphSubWin->setToolTip("Add a new plot");
    actionToGrid = new QAction(QIcon(":/icons/toolbar/images/tile_windows.png"),"",viewGV->mainwindow);
    actionToGrid->setToolTip("Tile plots");
    actionLoadLogData = new QAction(QIcon(":/icons/toolbar/images/open_file.png"),"",viewGV->mainwindow);
    actionLoadLogData->setToolTip("Load data");
    actionRefreshLogData = new QAction(QIcon(":/icons/toolbar/images/refresh.png"),"",viewGV->mainwindow);
    actionRefreshLogData->setToolTip("Refresh data manually");
    actionSavePdf = new QAction(QIcon(":/icons/toolbar/images/pdf_save.png"),"",viewGV->mainwindow);
    actionSavePdf->setToolTip("Save plot as pdf");
    actionSavePng = new QAction(QIcon(":/icons/toolbar/images/png_save.png"),"",viewGV->mainwindow);
    actionSavePng->setToolTip("save plot as png");

    // connect actions
    connect(actionAddGraphSubWin, SIGNAL(triggered()), this, SLOT(actionAddGraphSubWin_triggered()));
    connect(actionToGrid, SIGNAL(triggered()), this, SLOT(actionToGrid_triggered()));
    connect(actionLoadLogData, SIGNAL(triggered()), this, SLOT(actionLoadLogData_triggered()));
    connect(actionRefreshLogData, SIGNAL(triggered()), this, SLOT(actionRefreshLogData_triggered()));
    connect(actionSavePdf, SIGNAL(triggered()), this, SLOT(actionSavePdf_triggered()));
    connect(actionSavePng, SIGNAL(triggered()), this, SLOT(actionSavePng_triggered()));

    // add actions to toolbar
    viewGV->toolbar->addAction(actionAddGraphSubWin);
    viewGV->toolbar->addAction(actionSavePdf);
    viewGV->toolbar->addAction(actionSavePng);
    viewGV->toolbar->addAction(actionToGrid);
    viewGV->toolbar->addAction(actionLoadLogData);
    viewGV->toolbar->addAction(actionRefreshLogData);

    // make the toolbar the right size
    viewGV->toolbar->setFixedHeight(28);
    viewGV->toolbar->layout()->setMargin(0);
}

void viewGVpropertieslayout::actionAddGraphSubWin_triggered()
{
    DBG() << "Create a new QCustomPlot object";
    QCustomPlot* plot = new QCustomPlot;
    this->setupPlot (plot);

    QMdiSubWindow* mdiSubWin = this->viewGV->mdiarea->addSubWindow(plot);
    if (mdiSubWin != NULL) {
        mdiSubWin->setVisible(true);
        // Add the mdiSubWin somewhere so it can be removed programatically.
    }
}

void viewGVpropertieslayout::actionToGrid_triggered()
{
    viewGV->mdiarea->tileSubWindows();
}

void viewGVpropertieslayout::actionRefreshLogData_triggered()
{
    // refresh all logs:
    for (int i = 0; i < vLogData.size(); ++i) {
        this->refreshLog (vLogData[i]);
    }
}

void viewGVpropertieslayout::actionSavePdf_triggered()
{
    // first get a pointer to the current plot!
    QCustomPlot * currPlot = (QCustomPlot *) currentSubWindow->widget();
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save graph as pdf"), "", tr("Pdf (*.pdf)"));

    if (!fileName.isEmpty()) {
        currPlot->savePdf(fileName);
    }
}

void viewGVpropertieslayout::actionSavePng_triggered()
{
    // first get a pointer to the current plot!
    QCustomPlot * currPlot = (QCustomPlot *) currentSubWindow->widget();
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save graph as png"), "", tr("Png (*.png)"));

    if (!fileName.isEmpty()) {
        currPlot->savePng(fileName);
    }
}

// Originally intended to refresh the graphs based on there being new data in the backend.
void viewGVpropertieslayout::refreshLog(logData * log)
{
    if (log->setupFromXML()) {
        // find graphs from this log

        // get a list of the MDI windows which are visible
        QList<QMdiSubWindow *> subWins = viewGV->mdiarea->subWindowList();

        // loop and extract the plot
        for (int i = 0; i < subWins.size(); ++i) {
            this->addLinesRasters (log, subWins[i]);
        }

    } // else return
}

void viewGVpropertieslayout::actionLoadLogData_triggered()
{
    // file dialog:
    QStringList fileNames = QFileDialog::getOpenFileNames(this, tr("Load log"), qgetenv("HOME"),
                                                          tr("XML files (*.xml);; All files (*)"));
    this->loadLogDataFiles(fileNames);
}
