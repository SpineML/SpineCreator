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

    /*!
     * \brief configure a new QCustomPlot for use.
     */
    void setupPlot (QCustomPlot* plot);

    /*!
     * \brief populateVLogData Populate @see vLogData from the given list of files.
     * \param filelist The list of log file names or absolute file paths.
     * \param path A directory whose path is prepended to the members of filelist.
     */
    void populateVLogData (QStringList filelist, QDir* path = 0);

    /*!
     * Clears out this->vLogData
     */
    void clearVLogData (void);

    /*!
     * \brief Store the visible QCustomPlots to the current experiment.
     */    void storePlotsToExpt (void);

    /*!
     * \brief Restore plots from an experiment into the graph view area.
     * \param e Pointer to the experiment which holds the list of plots to restore.
     */
    void restorePlotsFromExpt (experiment* e);

    /*!
     * \brief viewGV - a structure defined in mainwindow.h holding
     * information about the graphing interface of SpineCreator.
     */
    viewGVstruct* viewGV;

    /*!
     * Called when the user presses a button to create a new, empty
     * graph window.
     */
    QAction* actionAddGraphSubWin;

    /*!
     * \brief actionToGrid Tile all the subwindows.
     */
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
    QListWidget* logList;

    /*!
     * \brief dataIndexList is a list of the individual data sets within each
     * log file. Corresponds to the elements in a population.
     */
    QListWidget* dataIndexList;

    /*!
     * \brief A list of the plot types for a given logData. Currently there's
     * only one option, depending on the data type; line graphs or raster plots.
     */
    QListWidget* typeList;

    /*!
     * This is "add a graph of the data to the current, existent plot
     * window". In this class, we'll use "graph" to mean the graph of
     * a single set of data, which may be one of many existing in the
     * file backend of a given logData object
     */
    QPushButton* addGraphButton;

    /*!
     * The log directory which is currently being viewed.
     */
    QString currentLogDataDir;

private:
    /*!
     * \brief Create the tool bar that sits above the graphing area. Called once.
     */
    void createToolbar (void);

    /*!
     * \brief Update @see logList
     */
    void updateLogList (void);

    /*!
     * \brief refreshLog
     * \param log
     */
    void refreshLog (logData* log);

    /*!
     * Run through the logData object and graph all data on @see subWin.
     */
    void addLinesRasters (logData* log, QMdiSubWindow* subWin);

signals:

public slots:
    // property slots
    /*!
     * \brief Actions to carry out when a sub window is selected by the user.
     */
    void windowSelected (QMdiSubWindow* window);

    /*!
     * \brief Called when the list of selected logs is changed by the user.
     */
    void logListSelectionChanged (int);

    /*!
     * \brief Add a graph to the current sub window.
     */
    void addGraphsToCurrent();

    /*!
     * Add the graphs defined in the give PlotInfo object to the
     * QCustomPlot plot.
     */
    void addGraphsToPlot (const PlotInfo& pi, QCustomPlot* plot);

    /*!
     * \brief Delete the currently selected log file.
     *
     * This looks at the currently selected log in @see logList, deletes
     * the log files behind it (the .xml and .bin files) and then removes
     * the corresponding entry from @see vLogData and @see logList
     */
    void deleteCurrentLog();

    // graph slots
    /*!
     * \brief Remove the selected graph from the currently selected subwindow.
     */
    void removeSelectedGraph();

    /*!
     * \brief Remove all graphs from the currently selected subwindow.
     */
    void removeAllGraphs();

    void toggleHorizontalZoom();
    void toggleHorizontalDrag();
    void toggleVerticalZoom();
    void toggleVerticalDrag();
    void rescaleAxes();
    void contextMenuRequest (QPoint pos);

    // toolbar slots
    void actionAddGraphSubWin_triggered();
    void actionToGrid_triggered();
    void actionLoadLogData_triggered();
    void actionRefreshLogData_triggered();
    void actionSavePdf_triggered();
    void actionSavePng_triggered();
};

#endif // VIEWGVPROPERTIESLAYOUT_H
