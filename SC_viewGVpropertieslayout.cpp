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

    // add widgets
    datas = new QListWidget;
    // set width to stop dock being resized too small
    datas->setMinimumWidth(200);
    datas->setSelectionMode(QAbstractItemView::SingleSelection);
    this->layout()->addWidget(new QLabel("Loaded logs"));
    this->layout()->addWidget(datas);
    indices = new QListWidget;
    indices->setSelectionMode(QAbstractItemView::MultiSelection);
    this->layout()->addWidget(new QLabel("Select log indices to plot"));
    this->layout()->addWidget(indices);
    types = new QListWidget;
    types->setSelectionMode(QAbstractItemView::SingleSelection);
    types->setMinimumHeight(40);
    types->setMaximumHeight(50);
    this->layout()->addWidget(new QLabel("Plot type"));
    this->layout()->addWidget(types);
    QPushButton * addPlot = new QPushButton("Add plot");
    this->layout()->addWidget(addPlot);
    addButton = addPlot;
    QPushButton * delLog = new QPushButton("Delete log file (PERMANENTLY)");
    this->layout()->addWidget(delLog);

    ((QVBoxLayout *)this->layout())->addStretch();

    // connect
    connect(viewGV->mdiarea, SIGNAL(subWindowActivated(QMdiSubWindow*)), this, SLOT(windowSelected(QMdiSubWindow*)));
    connect(datas, SIGNAL(currentRowChanged(int)), this, SLOT(dataSelectionChanged(int)));
    connect(addPlot, SIGNAL(clicked()), this, SLOT(addPlotToCurrent()));
    connect(delLog, SIGNAL(clicked()), this, SLOT(deleteCurrentLog()));
}

viewGVpropertieslayout::~viewGVpropertieslayout()
{
    for (int i = 0; i < logsForGraphs.size(); ++i) {
        delete logsForGraphs[i];
    }

    delete actionAddGraph;
    actionAddGraph = NULL;

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

void viewGVpropertieslayout::updateLogs()
{
    DBG() << "re-populating this->datas";
    disconnect(datas);
    datas->clear();
    for (int i = 0; i < logsForGraphs.size(); ++i) {
        datas->addItem(logsForGraphs[i]->logName);
    }
    connect(datas, SIGNAL(currentRowChanged(int)), this, SLOT(dataSelectionChanged(int)));
}

void viewGVpropertieslayout::clearLogs()
{
    DBG() << "Clearing out logsForGraphs and datas (UI element)";
    // disconnect(this->datas);
    // FIXME: Need to do the right thing here. What if a log has a plot associated with it?
    QVector<logData*>::iterator j = this->logsForGraphs.begin();
    while (j != this->logsForGraphs.end()) {
        if ((*j)->plot == NULL) {
            DBG() << "Null plot, deleting from logsForGraphs";
            delete *j;
            // this->datas[i].remove();
            this->logsForGraphs.erase(j);
        } else {
            DBG() << "Non-null plot, keep this one.";
        }
        ++j;
    }
    //connect(this->datas, SIGNAL(currentRowChanged(int)), this, SLOT(dataSelectionChanged(int)));
}

void viewGVpropertieslayout::deleteCurrentLog()
{
    // get the log index
    int dataIndex = datas->currentRow();

    // check that we have a selection
    if (dataIndex < 0) {
        return;
    }

    // delete the log file
    QDir dir;
    dir.remove(logsForGraphs[dataIndex]->logFileXMLname);
    dir.remove(logsForGraphs[dataIndex]->logFile.fileName());

    // remove the log
    logData * log = logsForGraphs[dataIndex];
    logsForGraphs.erase(logsForGraphs.begin()+dataIndex);
    delete log;

    // refresh display
    updateLogs();
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

void viewGVpropertieslayout::dataSelectionChanged(int index)
{
    // use index to get log
    this->types->clear();
    this->indices->clear();

    // no s
    if (index == -1) {
        return;
    }

    // setup log indices
    if (logsForGraphs[index]->dataClass == ANALOGDATA) {
        for (int i = 0; i < logsForGraphs[index]->columns.size(); ++i) {
            this->indices->addItem("Index " + QString::number(logsForGraphs[index]->columns[i].index));
        }
    } else if (logsForGraphs[index]->dataClass == EVENTDATA) {
        for (int i = 0; i < logsForGraphs[index]->eventIndices.size(); ++i) {
            this->indices->addItem("Index " + QString::number(logsForGraphs[index]->eventIndices[i]));
        }
    }
    // start with all indices selected
    //indices->selectedItems();

    // setup plot types
    if (logsForGraphs[index]->dataClass == ANALOGDATA) {
        // populate types with analog plot forms
        this->types->addItem("Line plot");
        this->types->setCurrentItem(this->types->item(0));
    } else if (logsForGraphs[index]->dataClass == EVENTDATA) {
        // populate types with event plot forms
        this->types->addItem("Raster plot");
        //this->types->addItem("Histogram");
        this->types->setCurrentItem(this->types->item(0));
    }
}

void viewGVpropertieslayout::addPlotToCurrent()
{
    // first get a pointer to the current plot!
    QCustomPlot * currPlot = (QCustomPlot *) currentSubWindow->widget();

    // get the log index
    int dataIndex = datas->currentRow();

    if (dataIndex < 0) {
        return;
    }

    // ok - if then time
    if (logsForGraphs[dataIndex]->dataClass == ANALOGDATA) {
        if (types->currentRow() == 0) { // Line Plot
            QList < QListWidgetItem * > selectedItems = indices->selectedItems();
            for (int i = 0; i < selectedItems.size(); ++i) {
                int index = indices->row(selectedItems[i]);
                // now we have the row, draw the graph...
                if (!logsForGraphs[dataIndex]->plotLine(currPlot, index)) {
                    qDebug() << "Oops, failed to plot";
                }
            }
        }
    }
    if (logsForGraphs[dataIndex]->dataClass == EVENTDATA) {
        if (types->currentRow() == 0) { // Raster Plot
            QList < QListWidgetItem * > selectedItems = indices->selectedItems();
            QList < QVariant > indexList;
            for (int i = 0; i < selectedItems.size(); ++i) {
                indexList.push_back( indices->row(selectedItems[i]));
            }
            if (!logsForGraphs[dataIndex]->plotRaster(currPlot, indexList)) {
                qDebug() << "Oops, failed to plot";
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
    actionAddGraph = new QAction(QIcon(":/icons/toolbar/images/new_file.png"),"",viewGV->mainwindow);
    actionAddGraph->setToolTip("Add a new plot");
    actionToGrid = new QAction(QIcon(":/icons/toolbar/images/tile_windows.png"),"",viewGV->mainwindow);
    actionToGrid->setToolTip("Tile plots");
    actionLoadData = new QAction(QIcon(":/icons/toolbar/images/open_file.png"),"",viewGV->mainwindow);
    actionLoadData->setToolTip("Load data");
    actionRefresh = new QAction(QIcon(":/icons/toolbar/images/refresh.png"),"",viewGV->mainwindow);
    actionRefresh->setToolTip("Refresh data manually");
    actionSavePdf = new QAction(QIcon(":/icons/toolbar/images/pdf_save.png"),"",viewGV->mainwindow);
    actionSavePdf->setToolTip("Save plot as pdf");
    actionSavePng = new QAction(QIcon(":/icons/toolbar/images/png_save.png"),"",viewGV->mainwindow);
    actionSavePng->setToolTip("save plot as png");

    // connect actions
    connect(actionAddGraph, SIGNAL(triggered()), this, SLOT(actionAddGraph_triggered()));
    connect(actionToGrid, SIGNAL(triggered()), this, SLOT(actionToGrid_triggered()));
    connect(actionLoadData, SIGNAL(triggered()), this, SLOT(actionLoadData_triggered()));
    connect(actionRefresh, SIGNAL(triggered()), this, SLOT(actionRefresh_triggered()));
    connect(actionSavePdf, SIGNAL(triggered()), this, SLOT(actionSavePdf_triggered()));
    connect(actionSavePng, SIGNAL(triggered()), this, SLOT(actionSavePng_triggered()));

    // add actions to toolbar
    viewGV->toolbar->addAction(actionAddGraph);
    viewGV->toolbar->addAction(actionSavePdf);
    viewGV->toolbar->addAction(actionSavePng);
    viewGV->toolbar->addAction(actionToGrid);
    viewGV->toolbar->addAction(actionLoadData);
    viewGV->toolbar->addAction(actionRefresh);

    // make the toolbar the right size
    viewGV->toolbar->setFixedHeight(28);
    viewGV->toolbar->layout()->setMargin(0);
}

void viewGVpropertieslayout::actionAddGraph_triggered()
{
    QCustomPlot * plot = new QCustomPlot;
    setupPlot(plot);

    QMdiSubWindow * mdiSubWin = viewGV->mdiarea->addSubWindow(plot);

    if (mdiSubWin != NULL) {
        mdiSubWin->setVisible(true);
    }
}

void viewGVpropertieslayout::actionToGrid_triggered()
{
    viewGV->mdiarea->tileSubWindows();
}

void viewGVpropertieslayout::actionRefresh_triggered()
{
    // refresh all logs:
    for (int i = 0; i < logsForGraphs.size(); ++i) {
        refreshLog(logsForGraphs[i]);
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

void viewGVpropertieslayout::refreshLog(logData * log)
{
    if (log->setupFromXML()) {
        // find graphs from this log

        // get a list of the MDI windows
        QList<QMdiSubWindow *> subWins = viewGV->mdiarea->subWindowList();

        // loop and extract the plot
        for (int i = 0; i < subWins.size(); ++i) {
            QCustomPlot * currPlot = (QCustomPlot *) subWins[i]->widget();

            // loop through graphs in the plot
            for (int j = 0; j < currPlot->graphCount(); ++j) {

                // if the graph is from this log
                if (currPlot->graph(j)->property("source").toString() == log->logFileXMLname) {

                    // extract remaining graph data
                    QString type = currPlot->graph(j)->property("type").toString();

                    if (type == "linePlot") {
                        // get index
                        int index = currPlot->graph(j)->property("index").toInt();
                        log->plotLine(currPlot, index, j);

                    } else if (type == "rasterPlot") {
                        // get indices
                        QList < QVariant > indices = currPlot->graph(j)->property("indices").toList();
                        log->plotRaster(currPlot, indices, j);
                    }
                }
            }
        }
    } // else return
}

void viewGVpropertieslayout::loadDataFiles(QStringList fileNames, QDir * path)
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
        for (int i = 0; i < logsForGraphs.size(); ++i) {
            if (logsForGraphs[i]->logFileXMLname == logXMLname) {
                refreshLog(logsForGraphs[i]);
                exists = true;
            }
        }

        // otherwise...
        if (!exists) {
            logData * log = new logData();
            log->logFileXMLname = logXMLname;
            if (!log->setupFromXML()) {
                qDebug() << "Failed to read XML";
                delete log;
                continue;
            }
            logsForGraphs.push_back(log);
        }
    }
    this->updateLogs();
}

void viewGVpropertieslayout::actionLoadData_triggered()
{
    // file dialog:
    QStringList fileNames = QFileDialog::getOpenFileNames(this, tr("Load log"), qgetenv("HOME"),
                                                          tr("XML files (*.xml);; All files (*)"));
    this->loadDataFiles(fileNames);
}
