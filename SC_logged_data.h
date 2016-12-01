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

#ifndef LOGDATA_H
#define LOGDATA_H

#include <QObject>
#include <QMdiArea>
#include "qcustomplot.h"
#include "globalHeader.h"

enum fileFormat
{
    BINARY,
    CSVFormat,
    SSVFormat
};

enum dataClasses
{
    ANALOGDATA,
    EVENTDATA
};

enum dataType
{
    TYPE_DOUBLE,
    TYPE_FLOAT,
    TYPE_INT32,
    TYPE_INT64,
    TYPE_STRING
};

struct column
{
    int index;
    QString heading;
    QString dims;
    dataType type;
};

/*!
 * \brief The logData class provides an interface to logged data from simulations stored on disk
 */
class logData : public QObject
{
    Q_OBJECT
public:
    explicit logData(QObject *parent = 0);

    /*!
     * A map of the plots associated with this logData object, and the QMdiSubWindows as well.
     */
    QMap<QCustomPlot*, QMdiSubWindow*> plots;
    QMap<QCustomPlot*, QMdiSubWindow*> getPlots (void) { return this->plots; }
    void addPlots (QMap<QCustomPlot*, QMdiSubWindow*>& theplots) {
        // CHECKME: Sensible?
        this->plots.swap (theplots);
    }

    QString logFileXMLname;
    double timeStep;
    QString logName;
    dataClasses dataClass;
    QVector < column > columns;
    QVector < int > eventIndices;

    /*!
     * Return the number of QCustomPlots associated with this logData object.
     */
    bool numPlots (void) {
        return this->plots.size();
    }

public: // but really private
    QFile logFile;
public: // but really private 2.
    fileFormat dataFormat;
    double endTime;
    int binaryDataStride;
    QVector < QVector < double > > colData;
    QString eventPortName;
    bool allLogged;
    double min;
    double max;

public:
    /*!
     * Delete the log file associated with this logData
     */
    void deleteLogFile (void);
    bool setupFromXML();
    double getMax();
    double getMin();
    QVector < double > getRow(int rowNum);
    bool plotLine(QCustomPlot* plot, QMdiSubWindow* msw, int colNum, int update = -1);
    bool plotRaster(QCustomPlot* plot, QMdiSubWindow* msw, QList < QVariant > indices, int update = -1);
    bool calculateBinaryDataStride();
    int calculateBinaryDataOffset(int);

    /*!
     * Close all the plots associated with this logData.
     */
    void closePlots (QMdiArea* mdiarea);

public slots:
    /*!
     * Called when the associated QCustomPlot is destroyed. Should remove entry from this->plots
     */
    void onPlotDestroyed (void);
};

#endif // LOGDATA_H
