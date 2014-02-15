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
#include "logdata.h"

struct viewGVstruct;

class viewGVpropertieslayout : public QWidget
{
    Q_OBJECT
public:
    explicit viewGVpropertieslayout(viewGVstruct * viewGVin, QWidget *parent = 0);
    ~viewGVpropertieslayout();
    void setupPlot(QCustomPlot * plot);
    void loadDataFiles(QStringList, QDir * path = 0);
    viewGVstruct * viewGV;
    QAction * actionAddGraph;
    QAction * actionToGrid;
    QAction * actionLoadData;
    QAction * actionRefresh;
    QAction * actionSavePdf;
    QAction * actionSavePng;
    QVector < logData * > logs;
    QMdiSubWindow * currentSubWindow;
    QListWidget * datas;
    QListWidget * indices;
    QListWidget * types;
    QPushButton * addButton;

private:
    void createToolbar();
    void updateLogs();
    void refreshLog(logData * log);
    
signals:
    
public slots:
    // property slots
    void windowSelected(QMdiSubWindow *);
    void dataSelectionChanged(int);
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
    void actionAddGraph_triggered();
    void actionToGrid_triggered();
    void actionLoadData_triggered();
    void actionRefresh_triggered();
    void actionSavePdf_triggered();
    void actionSavePng_triggered();
};

#endif // VIEWGVPROPERTIESLAYOUT_H
