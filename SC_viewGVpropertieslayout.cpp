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
    this->logList = new QListWidget;
    // set width to stop dock being resized too small
    this->logList->setMinimumWidth(200);
    this->logList->setSelectionMode(QAbstractItemView::SingleSelection);
    this->layout()->addWidget(new QLabel("Loaded logs"));
    this->layout()->addWidget(this->logList);
    this->dataIndexList = new QListWidget;
    this->dataIndexList->setSelectionMode(QAbstractItemView::MultiSelection);
    this->layout()->addWidget(new QLabel("Select log indices to plot"));
    this->layout()->addWidget(this->dataIndexList);
    this->typeList = new QListWidget;
    this->typeList->setSelectionMode(QAbstractItemView::SingleSelection);
    this->typeList->setMinimumHeight(40);
    this->typeList->setMaximumHeight(50);
    this->layout()->addWidget(new QLabel("Plot type"));
    this->layout()->addWidget(this->typeList);
    QPushButton * agb = new QPushButton("Add plot");
    this->layout()->addWidget(agb);
    this->addGraphButton = agb;
    QPushButton * dlb = new QPushButton("Delete log file (PERMANENTLY)");
    this->layout()->addWidget(dlb);
    this->unifyTime = true;
    QCheckBox* unifyTimeButton = new QCheckBox("Unify time axes");
    unifyTimeButton->setCheckState (Qt::Checked);
    this->layout()->addWidget (unifyTimeButton);
    ((QVBoxLayout *)this->layout())->addStretch();

    // connect
    connect(this->viewGV->mdiarea, SIGNAL(subWindowActivated(QMdiSubWindow*)), this, SLOT(windowSelected(QMdiSubWindow*)));
    connect(this->logList, SIGNAL(currentRowChanged(int)), this, SLOT(logListSelectionChanged(int)));
    connect(agb, SIGNAL(clicked()), this, SLOT(addGraphsToCurrent()));
    connect(dlb, SIGNAL(clicked()), this, SLOT(deleteCurrentLog()));
    connect(unifyTimeButton, SIGNAL(clicked()), this, SLOT(toggleUnifyTime()));
}

viewGVpropertieslayout::~viewGVpropertieslayout()
{
    for (int i = 0; i < this->vLogData.size(); ++i) {
        delete this->vLogData[i];
    }

    delete actionAddGraphSubWin;
    actionAddGraphSubWin = NULL;

    // delete actionToGrid;
    this->actionToGrid = NULL;
}

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

    // Connect signal to unify the time axis range
    connect(plot->xAxis, SIGNAL(rangeChanged(QCPRange)), this, SLOT(unifyRangeForAllPlots(QCPRange)));
}

void viewGVpropertieslayout::unifyRangeForAllPlots (const QCPRange& r)
{
    if (this->unifyTime) {
        // Need sender, so we don't try to set range for that one.
        QCustomPlot* senderPlot = qobject_cast<QCustomPlot*>(sender());

        QList<QMdiSubWindow*> subWins = this->viewGV->mdiarea->subWindowList();
        QList<QMdiSubWindow*>::iterator sw = subWins.begin();
        while (sw != subWins.end()) {
            if ((*sw) != (QMdiSubWindow*)0) {
                QCustomPlot* p = (QCustomPlot*)(*sw)->widget();
                if (p != senderPlot) {
                    // Disconnect handler, change range, then re-connect the handler:
                    bool disconnected = disconnect (p->xAxis, SIGNAL(rangeChanged(QCPRange)),
                                                    this, SLOT(unifyRangeForAllPlots(QCPRange)));
                    if (disconnected) {
                        p->xAxis->setRange(r);
                        p->replot();
                        bool connected = connect(p->xAxis, SIGNAL(rangeChanged(QCPRange)),
                                                 this, SLOT(unifyRangeForAllPlots(QCPRange)));
                        if (!connected) {
                            DBG() << "WARNING: Failed to re-connect unifyRangeForAllPlots signal";
                        }
                    }
                }
            }
            sw++;
        }
    } // else not configured to unify time axes
}

void viewGVpropertieslayout::updateLogList()
{
    DBG() << "re-populating this->logList";
    disconnect(this->logList);
    this->logList->clear();
    for (int i = 0; i < this->vLogData.size(); ++i) {
        this->logList->addItem(this->vLogData[i]->logName);
    }
    connect(this->logList, SIGNAL(currentRowChanged(int)), this, SLOT(logListSelectionChanged(int)));
}

void viewGVpropertieslayout::clearVLogData()
{
    // If logData's deconstructor worked, then this would be just vLogData.clear():
    QVector<logData*>::iterator j = this->vLogData.begin();
    while (j != this->vLogData.end()) {
        if ((*j) != (logData*)0) {
            delete (*j);
        }
        j = this->vLogData.erase(j);
    }

    this->updateLogList();
}

void viewGVpropertieslayout::populateVLogData (QStringList fileNames, QDir* path)
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
                this->refreshLog(this->vLogData[i]);
                exists = true;
            }
        }

        // otherwise...
        if (!exists) {
            //DBG() << "Create a new logData object";
            logData * log = new logData();
            log->logFileXMLname = logXMLname;
            if (!log->setupFromXML()) {
                qDebug() << "Failed to read XML";
                delete log;
                continue;
            }
            //DBG() << "Push back " << logXMLname << " onto logsForGraphs";
            this->vLogData.push_back(log);
        }
    }

    this->updateLogList();
}

void viewGVpropertieslayout::storePlotsToExpt (void)
{
    if (this->currentExperiment == (experiment*)0) {
        DBG() << "No currentExperiment; can't store plots";
        return;
    }

    this->currentExperiment->clearVisiblePlots();

    QList<QMdiSubWindow*> subWins = this->viewGV->mdiarea->subWindowList();
    QList<QMdiSubWindow*>::iterator sw = subWins.begin();
    while (sw != subWins.end()) {
        if ((*sw) != (QMdiSubWindow*)0) {
            // Make a copy of the QCustomPlot information
            PlotInfo p((QCustomPlot*)(*sw)->widget());
            DBG() << "Stored PlotInfo for plot '" << p.getTitle() << "'";
            this->currentExperiment->visiblePlots.append(p);
        }
        sw++;
    }
}

void viewGVpropertieslayout::clearPlots (void)
{
    QList<QMdiSubWindow*> subWins = this->viewGV->mdiarea->subWindowList();
    QList<QMdiSubWindow*>::iterator sw = subWins.begin();
    while (sw != subWins.end()) {
        if ((*sw) != (QMdiSubWindow*)0) {
            (*sw)->close();
        }
        sw++;
    }
}

void viewGVpropertieslayout::addEmptyPlot (void)
{
    DBG() << "INFO: ADD EMPTY PLOT";
    this->actionAddGraphSubWin_triggered();
    this->actionToGrid_triggered();
}

int viewGVpropertieslayout::restorePlotsFromExpt (experiment* e)
{
    int numrestored = 0;

    if (e == (experiment*)0) {
        DBG() << "ERROR: need to be passed a non-null experiment*";
        return numrestored;
    }

    this->currentExperiment = e;

    if (this->currentExperiment->visiblePlots.isEmpty()) {
        DBG() << "INFO: The new experiment has no plots to render";
        return numrestored;
    }

    DBG() << "Restore plots for experiment " << this->currentExperiment->name;
    int pi = 0;
    while (pi < this->currentExperiment->visiblePlots.size()) {
        // Now re-create the graph(s)
        QCustomPlot* cplot = new QCustomPlot();
        this->setupPlot (cplot);

        // setup and restore from pi.
        DBG() << "Calling addGraphsToPlot";
        // .at(pi) would be better but can't pass const PlotInfo& to addGraphsToPlot
        this->addGraphsToPlot (this->currentExperiment->visiblePlots[pi], cplot);

        QMdiSubWindow* mdiSubWin = this->viewGV->mdiarea->addSubWindow(cplot);
        if (mdiSubWin != NULL) {
            ++numrestored;
            mdiSubWin->setVisible(true);
        } else {
            DBG() << "ERROR: Couldn't add sub window to mdiarea (viewGVpropertieslayout::restoreLogDataFromExpt)";
        }
        ++pi;
    }

    if (numrestored > 0) {
        this->actionToGrid_triggered();
    }

    return numrestored;
}

void
viewGVpropertieslayout::addRasterGraphToPlot (PlotInfo& pi, QCustomPlot* plot, int i)
{
    this->addGraphSetDataCommon (pi, plot, i);
    plot->graph(plot->graphCount()-1)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 1));
    plot->graph(plot->graphCount()-1)->setLineStyle(QCPGraph::lsNone);
#if 0
    // Construct indices.
    QMap<double, QCPData>::const_iterator j = pi.graphs(i).data.constBegin();
    QList<QVariant> indices;
    while (j != pi.graphs(i).data.constEnd()) {
        // (int)j.key() adds to indices
        indices.insert ((int)j.key());
        ++j;
    }
    QVariant var(indices); // Found in pi.graphs(i).data
    plot->graph(plot->graphCount()-1)->setProperty("indices", var); // Perhaps will have to collect indices from out of the old plot?
#endif
    this->setGraphSettingsCommon (pi, plot, i);
}

void
viewGVpropertieslayout::addLineGraphToPlot (PlotInfo& pi, QCustomPlot* plot, int i)
{
    this->addGraphSetDataCommon (pi, plot, i);
    plot->graph(plot->graphCount()-1)->setName ("Index " + QString::number(i));
    plot->graph(plot->graphCount()-1)->setProperty("index", pi.graphs[i].index);
    this->setGraphSettingsCommon (pi, plot, i);
}

void
viewGVpropertieslayout::alternatePlotColours (QCustomPlot* plot)
{
    QPen pen;
    pen.setColor((Qt::GlobalColor) (7+(plot->graphCount()-1)%11));
    plot->graph(plot->graphCount()-1)->setPen(pen);
}

void
viewGVpropertieslayout::addGraphSetDataCommon (PlotInfo& pi, QCustomPlot* plot, int i)
{
    plot->addGraph();

    // Create a new QCPDataMap from pi.graphs[i].data and add it to
    // the QCustomPlot.
    QCPDataMap* dmptr = new QCPDataMap;
    QMap<double, double>::const_iterator it = pi.graphs[i].data.constBegin();
    while (it != pi.graphs[i].data.constEnd()) {
        QCPData dat(it.key(),it.value());
        dmptr->insert(it.key(), dat);
        ++it;
    }
    plot->graph(plot->graphCount()-1)->setData (dmptr);
}

void
viewGVpropertieslayout::setGraphSettingsCommon (PlotInfo& pi, QCustomPlot* plot, int i)
{
    plot->graph(plot->graphCount()-1)->setProperty("type", pi.graphs[i].type);
    plot->graph(plot->graphCount()-1)->setProperty("source", pi.graphs[i].source);
    plot->xAxis->setLabel(pi.xlabel);
    plot->yAxis->setLabel(pi.ylabel);
    this->alternatePlotColours (plot);
    plot->xAxis->setTickStep(pi.xtickstep);
    plot->yAxis->setTickStep(pi.ytickstep);
    plot->xAxis->setRange(pi.xrangelower, pi.xrangeupper);
    plot->yAxis->setRange(pi.yrangelower, pi.yrangeupper);
    // Setting rangezoom, rangedrag, etc not strictly necessary, but
    // keeps the plots in same state for the user:
    plot->xAxis->axisRect()->setRangeZoom(pi.xrangezoom);
    plot->yAxis->axisRect()->setRangeZoom(pi.yrangezoom);
    plot->xAxis->axisRect()->setRangeZoomFactor(pi.xrangezoomfactor);
    plot->yAxis->axisRect()->setRangeZoomFactor(pi.yrangezoomfactor);
    plot->xAxis->axisRect()->setRangeDrag(pi.xrangedrag);
    plot->yAxis->axisRect()->setRangeDrag(pi.yrangedrag);

    // Probably need to check layout element exists:
    QCPPlotTitle* t = (QCPPlotTitle*)plot->plotLayout()->element (0, 0);
    t->setText (pi.getTitle());
}

void
viewGVpropertieslayout::addGraphsToPlot (PlotInfo& pi, QCustomPlot* plot)
{
    // add graph and setup data and name
    DBG() << "called for plot title" << pi.getTitle();

    for (int i = 0; i < pi.graphs.size(); ++i) {
        if (pi.graphs[i].type == "rasterPlot") {
            this->addRasterGraphToPlot (pi, plot, i);
        } else {
            this->addLineGraphToPlot (pi, plot, i);
        }
    }

    // Removed a call here to rescale the plot.

    plot->legend->setVisible(false); // fixme: Make this an option
    // title
    if (plot->plotLayout()->rowCount() == 1) {
        plot->plotLayout()->insertRow(0); // inserts an empty row above the default axis rect
        plot->plotLayout()->addElement(0, 0, new QCPPlotTitle(plot, pi.getTitle()));
    }

    plot->replot();
}

void viewGVpropertieslayout::addLinesRasters (logData* log, QMdiSubWindow* subWin)
{
    QCustomPlot* currPlot = (QCustomPlot*)subWin->widget();

    // loop through graphs in the plot
    for (int j = 0; j < currPlot->graphCount(); ++j) {

        // if "the graph is from this log"
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
        } // else graph not from this log

    } // end loop through graphs in plot.
}

void viewGVpropertieslayout::deleteCurrentLog()
{
    // get the log index
    int listIdx = this->logList->currentRow();

    // check that we have a selection
    if (listIdx < 0) {
        return;
    }

    // delete the log file
    this->vLogData[listIdx]->deleteLogFile();

    // remove the log
    logData* log = this->vLogData[listIdx];
    this->vLogData.erase(this->vLogData.begin()+listIdx);
    delete log;

    // refresh display
    this->updateLogList();
}

void viewGVpropertieslayout::toggleUnifyTime (void)
{
    QCheckBox* sndr = qobject_cast<QCheckBox*>(sender());
    if (sndr->checkState() == Qt::Checked) {
        this->unifyTime = true;
    } else {
        this->unifyTime = false;
    }
}

void viewGVpropertieslayout::removeSelectedGraph()
{
    QCustomPlot * currPlot = (QCustomPlot*)this->currentSubWindow->widget();
    if (currPlot->selectedGraphs().size() > 0) {
        currPlot->removeGraph(currPlot->selectedGraphs().first());
        currPlot->replot();
    }
}

void viewGVpropertieslayout::removeAllGraphs()
{
    QCustomPlot * currPlot = (QCustomPlot*)this->currentSubWindow->widget();
    currPlot->clearGraphs();
    currPlot->replot();
}

void viewGVpropertieslayout::toggleHorizontalZoom()
{
    QCustomPlot* currPlot = (QCustomPlot*)this->currentSubWindow->widget();
    if (currPlot->axisRect()->rangeZoom() & Qt::Horizontal) {
        currPlot->axisRect()->setRangeZoom(currPlot->axisRect()->rangeZoom() & ~Qt::Horizontal);
    } else {
        currPlot->axisRect()->setRangeZoom(currPlot->axisRect()->rangeZoom() | Qt::Horizontal);
    }
}

void viewGVpropertieslayout::toggleHorizontalDrag()
{
    QCustomPlot* currPlot = (QCustomPlot*) this->currentSubWindow->widget();
    if (currPlot->axisRect()->rangeDrag() & Qt::Horizontal) {
        currPlot->axisRect()->setRangeDrag(currPlot->axisRect()->rangeDrag() & ~Qt::Horizontal);
    } else {
        currPlot->axisRect()->setRangeDrag(currPlot->axisRect()->rangeDrag() | Qt::Horizontal);
    }
}

void viewGVpropertieslayout::toggleVerticalZoom()
{
    QCustomPlot* currPlot = (QCustomPlot*)this->currentSubWindow->widget();
    if (currPlot->axisRect()->rangeZoom() & Qt::Vertical) {
        currPlot->axisRect()->setRangeZoom(currPlot->axisRect()->rangeZoom() & ~Qt::Vertical);
    } else {
        currPlot->axisRect()->setRangeZoom(currPlot->axisRect()->rangeZoom() | Qt::Vertical);
    }
}

void viewGVpropertieslayout::toggleVerticalDrag()
{
    QCustomPlot* currPlot = (QCustomPlot*)this->currentSubWindow->widget();
    if (currPlot->axisRect()->rangeDrag() & Qt::Vertical) {
        currPlot->axisRect()->setRangeDrag(currPlot->axisRect()->rangeDrag() & ~Qt::Vertical);
    } else {
        currPlot->axisRect()->setRangeDrag(currPlot->axisRect()->rangeDrag() | Qt::Vertical);
    }
}

void viewGVpropertieslayout::rescaleAxes()
{
    QCustomPlot* currPlot = (QCustomPlot*)this->currentSubWindow->widget();
    currPlot->rescaleAxes();
    currPlot->replot();
}

void viewGVpropertieslayout::contextMenuRequest(QPoint pos)
{
    QCustomPlot* currPlot = (QCustomPlot*)this->currentSubWindow->widget();
    QMenu* menu = new QMenu(this);
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
    if (this->vLogData[index]->dataClass == ANALOGDATA) {
        for (int i = 0; i < this->vLogData[index]->columns.size(); ++i) {
            this->dataIndexList->addItem("Index " + QString::number(this->vLogData[index]->columns[i].index));
        }
    } else if (this->vLogData[index]->dataClass == EVENTDATA) {
        for (int i = 0; i < this->vLogData[index]->eventIndices.size(); ++i) {
            this->dataIndexList->addItem("Index " + QString::number(this->vLogData[index]->eventIndices[i]));
        }
    }

    // setup plot types
    if (this->vLogData[index]->dataClass == ANALOGDATA) {

        // populate types with analog plot forms:
        this->typeList->addItem("Line plot");

        // Pre-select Line plot, as there is only one option.
        this->typeList->setCurrentItem(this->typeList->item(0));

        // For line plots, it's common to select one or a few traces
        // so don't select all items in this->indices.

    } else if (this->vLogData[index]->dataClass == EVENTDATA) {

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
    this->addGraphButton->setDisabled(false);
}

void viewGVpropertieslayout::addGraphsToCurrent()
{
    QCustomPlot* currPlot = (QCustomPlot*)this->currentSubWindow->widget();

    if (this->unifyTime) {
        // In this case, if there are other plots, then scale to those plots.
        // WRITEME NEXT
    }

    // get the log index
    int dataIndex = this->logList->currentRow();

    if (dataIndex < 0) {
        return;
    }

    // ok - if then time
    if (this->vLogData[dataIndex]->dataClass == ANALOGDATA) {
        if (this->typeList->currentRow() == 0) { // Line Plot
            QList < QListWidgetItem * > selectedItems = this->dataIndexList->selectedItems();
            for (int i = 0; i < selectedItems.size(); ++i) {
                int index = this->dataIndexList->row(selectedItems[i]);
                // now we have the row, draw the graph...
                DBG() << "Draw graph for dataIndex=" << dataIndex << " indices index=" << index;
                if (!this->vLogData[dataIndex]->plotLine(currPlot, this->currentSubWindow, index)) {
                    DBG() << "Oops, failed to plot";
                }
            }
        }
    }
    if (this->vLogData[dataIndex]->dataClass == EVENTDATA) {
        if (this->typeList->currentRow() == 0) { // Raster Plot
            QList < QListWidgetItem * > selectedItems = this->dataIndexList->selectedItems();
            QList < QVariant > indexList;
            for (int i = 0; i < selectedItems.size(); ++i) {
                indexList.push_back(this->dataIndexList->row(selectedItems[i]));
            }
            if (!this->vLogData[dataIndex]->plotRaster(currPlot, this->currentSubWindow, indexList)) {
                DBG() << "Oops, failed to plot";
            }
        }
    }
}

void viewGVpropertieslayout::windowSelected (QMdiSubWindow* window)
{
    if (window == NULL) {
        this->actionToGrid->setDisabled(true);
        this->actionSavePdf->setDisabled(true);
        this->actionSavePng->setDisabled(true);
        this->addGraphButton->setDisabled(true);
    } else {
        this->actionToGrid->setDisabled(false);
        this->actionSavePdf->setDisabled(false);
        this->actionSavePng->setDisabled(false);
        this->addGraphButton->setDisabled(false);
    }

    // get the window list
    QList<QMdiSubWindow*> windowList = this->viewGV->mdiarea->subWindowList();

    // Set currentSubWindow for the selected window
    for (int i = 0; i < windowList.count(); ++i) {
        if (windowList[i] == window) {
            this->currentSubWindow = windowList[i];
            /*QLabel * label = qobject_cast <QLabel *> (this->layout()->itemAt(0)->widget());
              if (label) { label->setText(QString::number(i)); } */
        }
    }
}

void viewGVpropertieslayout::createToolbar()
{
    QCommonStyle style;

    // create actions
    actionAddGraphSubWin = new QAction(QIcon(":/icons/toolbar/images/new_file.png"),"",this->viewGV->mainwindow);
    actionAddGraphSubWin->setToolTip("Add a new plot");
    this->actionToGrid = new QAction(QIcon(":/icons/toolbar/images/tile_windows.png"),"",this->viewGV->mainwindow);
    this->actionToGrid->setToolTip("Tile plots");
    this->actionLoadLogData = new QAction(QIcon(":/icons/toolbar/images/open_file.png"),"",this->viewGV->mainwindow);
    this->actionLoadLogData->setToolTip("Load data");
    this->actionRefreshLogData = new QAction(QIcon(":/icons/toolbar/images/refresh.png"),"",this->viewGV->mainwindow);
    this->actionRefreshLogData->setToolTip("Refresh data manually");
    this->actionSavePdf = new QAction(QIcon(":/icons/toolbar/images/pdf_save.png"),"",this->viewGV->mainwindow);
    this->actionSavePdf->setToolTip("Save plot as pdf");
    this->actionSavePng = new QAction(QIcon(":/icons/toolbar/images/png_save.png"),"",this->viewGV->mainwindow);
    this->actionSavePng->setToolTip("save plot as png");

    // connect actions
    connect(actionAddGraphSubWin, SIGNAL(triggered()), this, SLOT(actionAddGraphSubWin_triggered()));
    connect(this->actionToGrid, SIGNAL(triggered()), this, SLOT(actionToGrid_triggered()));
    connect(this->actionLoadLogData, SIGNAL(triggered()), this, SLOT(actionLoadLogData_triggered()));
    connect(this->actionRefreshLogData, SIGNAL(triggered()), this, SLOT(actionRefreshLogData_triggered()));
    connect(this->actionSavePdf, SIGNAL(triggered()), this, SLOT(actionSavePdf_triggered()));
    connect(this->actionSavePng, SIGNAL(triggered()), this, SLOT(actionSavePng_triggered()));

    // add actions to toolbar
    this->viewGV->toolbar->addAction(actionAddGraphSubWin);
    this->viewGV->toolbar->addAction(this->actionSavePdf);
    this->viewGV->toolbar->addAction(this->actionSavePng);
    this->viewGV->toolbar->addAction(this->actionToGrid);
    this->viewGV->toolbar->addAction(this->actionLoadLogData);
    this->viewGV->toolbar->addAction(this->actionRefreshLogData);

    // make the toolbar the right size
    this->viewGV->toolbar->setFixedHeight(28);
    this->viewGV->toolbar->layout()->setMargin(0);
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
    this->viewGV->mdiarea->tileSubWindows();
}

void viewGVpropertieslayout::actionRefreshLogData_triggered()
{
    // refresh all logs:
    for (int i = 0; i < this->vLogData.size(); ++i) {
        this->refreshLog (this->vLogData[i]);
    }
}

void viewGVpropertieslayout::actionSavePdf_triggered()
{
    QCustomPlot* currPlot = (QCustomPlot*)this->currentSubWindow->widget();
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save graph as pdf"), "", tr("Pdf (*.pdf)"));

    if (!fileName.isEmpty()) {
        currPlot->savePdf(fileName);
    }
}

void viewGVpropertieslayout::actionSavePng_triggered()
{
    QCustomPlot* currPlot = (QCustomPlot*)this->currentSubWindow->widget();
    QString fileName = QFileDialog::getSaveFileName (this, tr("Save graph as png"), "", tr("Png (*.png)"));

    if (!fileName.isEmpty()) {
        currPlot->savePng (fileName);
    }
}

// Originally intended to refresh the graphs based on there being new data in the
// backend. Calls logData::setupFromXML to re-populate log, then
void viewGVpropertieslayout::refreshLog (logData* log)
{
    if (log->setupFromXML()) {
        // find graphs from this log

        // get a list of the MDI windows which are visible
        QList<QMdiSubWindow*> subWins = this->viewGV->mdiarea->subWindowList();

        // loop and extract the plot
        for (int i = 0; i < subWins.size(); ++i) {
            // Adds lines and rasters from log to subWins[i] ONLY if there's a match.
            this->addLinesRasters (log, subWins[i]);
        }

    } // else return
}

void viewGVpropertieslayout::actionLoadLogData_triggered()
{
    QStringList fileNames = QFileDialog::getOpenFileNames (this, tr("Load log"), qgetenv("HOME"),
                                                           tr("XML files (*.xml);; All files (*)"));
    this->populateVLogData (fileNames);
}
