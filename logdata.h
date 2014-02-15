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
#include "qcustomplot.h"
#include <globalHeader.h>

enum fileFormat {
    BINARY,
    CSVFormat,
    SSVFormat
};

enum dataClasses {
    ANALOGDATA,
    EVENTDATA
};


enum dataType {
    TYPE_DOUBLE,
    TYPE_FLOAT,
    TYPE_INT32,
    TYPE_INT64,
    TYPE_STRING
};


struct column {
    int index;
    QString heading;
    QString dims;
    dataType type;
};

class logData : public QObject
{
    Q_OBJECT
public:
    explicit logData(QObject *parent = 0);
    QCustomPlot * plot;
    QFile logFile;
    QString logFileXMLname;
    fileFormat dataFormat;
    vector < column > columns;
    double endTime;
    vector < int > eventIndices;
    int binaryDataStride;
    QVector < QVector < double > > colData;
    double timeStep;
    dataClasses dataClass;
    QString eventPortName;
    QString logName;
    bool allLogged;
    double min;
    double max;

    bool setupFromXML();
    double getMax();
    double getMin();
    vector < double > getRow(int rowNum);
    bool plotLine(QCustomPlot * plot, int colNum, int update = -1);
    bool plotRaster(QCustomPlot * plot, QList < QVariant > indices, int update = -1);
    bool calculateBinaryDataStride();
    int calculateBinaryDataOffset(int);

signals:
    
public slots:

    
};

#endif // LOGDATA_H
