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

#ifndef VIEWGVPROPERTIESLAYOUT_H
#define VIEWGVPROPERTIESLAYOUT_H

#include <QtGui>
#include "SC_logged_data.h"
#include "EL_experiment.h"

struct viewGVstruct; // Defined in mainwindow.h

class viewGVpropertieslayout : public QWidget
{
    Q_OBJECT
public:
    explicit viewGVpropertieslayout(viewGVstruct* viewGVin, QWidget* parent = 0);
    ~viewGVpropertieslayout();
    void setupPlot(QCustomPlot* plot);
    void loadLogDataFiles(QStringList, QDir* path = 0);

    /*!
     * Clears this->datas
     */
    void clearLogs();

    /*!
     * Store the logDatas for the currently rendered graphs into a
     * list in this->currentExperiment.
     *
     * For each graph, save the logData to
     * this->currentExperiment->graphedLogs (QList<logData>)
     */
    void storeGraphs (void);

    /*!
     * Restore the list of logDatas for rendering from experiment* e,
     * plotting all the plots for each one. Set this->currentExperiment = e.
     */
    void restoreGraphs (experiment* e);

    viewGVstruct* viewGV;

    /*!
     * Called when the user presses a button to create a new, empty
     * graph window.
     */
    QAction* actionAddGraphSubWin;

    QAction* actionToGrid;

    /*!
     * \brief actionLoadData Open a dialog to load some log data
     */
    QAction* actionLoadLogData;

    QAction* actionRefreshLogData;
    QAction* actionSavePdf;
    QAction* actionSavePng;

    /*!
     * \brief vLogData A vector of pointers to the logData objects
     * which are being shown for the current experiment.
     */
    QVector<logData*> vLogData;

    /*!
     * The currently selected graph sub-window.
     */
    QMdiSubWindow* currentSubWindow;

    /*!
     * The experiment which is currently "in view"
     */
    experiment* currentExperiment;

    /*!
     * The list widgets which appear on the right hand side of the graph view.
     */
    //@{
    QListWidget* logList;
    QListWidget* dataIndexList;
    QListWidget* typeList;
    //@}

    /*!
     * This is "add a graph of the data to the current, existent plot
     * window".
     */
    QPushButton * addButton;

    /*!
     * The log directory which is currently being viewed.
     */
    QString currentLogDataDir;

private:
    void createToolbar();
    void updateLogList();
    void refreshLog(logData* log);

    /*!
     * Run through the logData and plot all data on subWin.
     */
    void addLinesRasters (logData* log, QMdiSubWindow* subWin);

signals:

public slots:
    // property slots
    void windowSelected(QMdiSubWindow*);
    void logListSelectionChanged(int);
    void addPlotToCurrent();
    void deleteCurrentLog();

    // plot slots
    void removeSelectedGraph();
    void removeAllGraphs();
    void toggleHorizontalZoom();
    void toggleHorizontalDrag();
    void toggleVerticalZoom();
    void toggleVerticalDrag();
    void rescaleAxes();
    void contextMenuRequest(QPoint pos);

    // toolbar slots
    void actionAddGraphSubWin_triggered();
    void actionToGrid_triggered();
    void actionLoadLogData_triggered();
    void actionRefreshLogData_triggered();
    void actionSavePdf_triggered();
    void actionSavePng_triggered();
};

#endif // VIEWGVPROPERTIESLAYOUT_H
