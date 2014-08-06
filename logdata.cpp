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

// this file includes a class for accessing logs and communicating with the
// qcustomplot widget

#include "logdata.h"
#include <QXmlStreamReader>

logData::logData(QObject *parent) :
    QObject(parent)
{
    plot = NULL;
    timeStep = 0.1;
}

double logData::getMax() {

    if (max != Q_INFINITY)
        return max;

    // no max, must calculate
    double tempMax = -Q_INFINITY;
    QVector < double > rowData;
    rowData.push_back(-Q_INFINITY);
    int i = 0;
    while (rowData.size() > 0) {
        for (int j = 0; j < rowData.size(); ++j)
            if (rowData[j] > tempMax && rowData[j] < Q_INFINITY)
                tempMax = rowData[j];
                //qDebug() << rowData[0] << rowData[1]<< rowData[2]<< rowData[3]<< rowData[4]<< rowData[5]<< rowData[6]<< rowData[7]<< rowData[8]<< rowData[9];}
        rowData = getRow(i);
        ++i;
    }
    max = tempMax;
    return max;

}

double logData::getMin() {

    if (min != Q_INFINITY)
        return min;

    // no min, must calculate
    double tempMin = Q_INFINITY;
    QVector < double > rowData;
    rowData.push_back(Q_INFINITY);
    int i = 0;
    while (rowData.size() > 0) {
        for (int j = 0; j < rowData.size(); ++j)
            if (rowData[j] < tempMin)
                tempMin = rowData[j];
        rowData = getRow(i);
        ++i;
    }
    min = tempMin;
    return min;
}

QVector < double > logData::getRow(int rowNum) {


    QVector < double > rowData;

    // is not analog return empty
    if (this->dataClass != ANALOGDATA)
        return rowData;

    // get data
    switch (dataFormat) {
    case BINARY:
    {
        if (!calculateBinaryDataStride())
            return rowData;

        // stream data from file
        QDataStream data(&logFile);
        data.device()->seek(0);
        // offset into file
        data.skipRawData(binaryDataStride*rowNum);

        // if we skip to the end of the file
        if (data.atEnd())
            return rowData;

        // check that all are same type
        dataType mainType;
        mainType = columns[0].type;
        for (int i = 0; i < columns.size(); ++i) {
            if (columns[i].type != mainType)
                return rowData;
        }

        switch (columns[0].type) {
        case TYPE_DOUBLE:
        {
            if (allLogged) {
                rowData.resize(columns.size());
                data.readRawData((char *) &rowData[0], sizeof(double)*rowData.size());
            } else {
                QVector < double > tempDbl;
                tempDbl.resize(columns.size());
                data.readRawData((char *) &tempDbl[0], sizeof(double)*tempDbl.size());
                // a good first guess
                vector <double> temp = rowData.toStdVector();
                temp.resize(columns.back().index+1, Q_INFINITY);
                rowData = QVector <double>::fromStdVector(temp);
                for (int i = 0; i < columns.size(); ++i) {
                    if (static_cast<int>(columns[i].index) > rowData.size()) {
                        vector <double> temp = rowData.toStdVector();
                        temp.resize(columns[i].index+1, Q_INFINITY);
                        rowData = QVector <double>::fromStdVector(temp);
                    }
                    rowData[columns[i].index] = tempDbl[i];
                }
            }
            return rowData;
        }
        break;
        case TYPE_FLOAT:
        {
            QVector < float > tempFloat;
            tempFloat.resize(columns.size());
            data.setFloatingPointPrecision(QDataStream::SinglePrecision);
            data.readRawData((char *) &tempFloat[0], sizeof(float)*tempFloat.size());
            // a good first guess
            vector <double> temp = rowData.toStdVector();
            temp.resize(columns.back().index+1, Q_INFINITY);
            rowData = QVector <double>::fromStdVector(temp);
            for (int i = 0; i < columns.size(); ++i) {
                if (static_cast<int>(columns[i].index) > rowData.size()) {
                    vector <double> temp = rowData.toStdVector();
                    temp.resize(columns[i].index+1, Q_INFINITY);
                    rowData = QVector <double>::fromStdVector(temp);
                }
                rowData[columns[i].index] = tempFloat[i];
            }
            return rowData;
        }
        break;
        case TYPE_INT64:
        {
            // not supported currently
            return rowData;

        }
        break;
        case TYPE_INT32:
        {
            QVector < int > tempInt;
            tempInt.resize(columns.size());
            data.readRawData((char *) &tempInt[0], sizeof(int)*tempInt.size());
            // a good first guess
            vector <double> temp = rowData.toStdVector();
            temp.resize(columns.back().index+1, Q_INFINITY);
            rowData = QVector <double>::fromStdVector(temp);
            for (int i = 0; i < columns.size(); ++i) {
                if (static_cast<int>(columns[i].index) > rowData.size()) {
                    vector <double> temp = rowData.toStdVector();
                    temp.resize(columns[i].index+1, Q_INFINITY);
                    rowData = QVector <double>::fromStdVector(temp);
                }
                rowData[columns[i].index] = tempInt[i];
            }
            return rowData;
        }
        break;
        case TYPE_STRING:
            return rowData;
        } // end switch (columns[0].type)
    }
    case CSVFormat:
    case SSVFormat:
    default:
        // do nothing in these cases.
        break;
    } // end switch (dataFormat)

    return rowData;
}

bool logData::plotLine(QCustomPlot *plot, int colNum, int update) {

    // if no plot give up
    if (plot == NULL)
        return false;

    // if out of range give up (should also catch no file)
    if (colNum >= (int) columns.size())
        return false;

    // clear existing data;
    colData[colNum].clear();

    // get data
    switch (dataFormat) {
    case BINARY:
    {
        if (!calculateBinaryDataStride())
            return false;
        int offset = calculateBinaryDataOffset(colNum);
        if (offset == -1)
            return false;

        // stream data from file
        QDataStream data(&logFile);
        data.device()->seek(0);
        // offset into file
        data.skipRawData(offset);
        switch (columns[colNum].type) {
        case TYPE_DOUBLE:
        {
            double tempDbl;
            while (data.readRawData((char *) &tempDbl, sizeof(tempDbl)) != -1) {
                // add to data
                colData[colNum].push_back(tempDbl);
                // skip to next row
                data.skipRawData(binaryDataStride-sizeof(double));
                if (data.atEnd())
                    break;
            }
        }
            break;
        case TYPE_FLOAT:
        {
            float tempFloat;
            data.setFloatingPointPrecision(QDataStream::SinglePrecision);
            while (data.readRawData((char *) &tempFloat, sizeof(tempFloat)) != -1) {
                // add to data
                colData[colNum].push_back((double) tempFloat);
                // skip to next row
                data.skipRawData(binaryDataStride-sizeof(float));
                if (data.atEnd())
                    break;
            }
        }
            break;
        case TYPE_INT64:
        {
            // not supported currently
                return false;
            int tempInt;
            while (data.readRawData((char *) &tempInt, sizeof(tempInt)) != -1) {
                // add to data
                colData[colNum].push_back((double) tempInt);
                // skip to next row
                data.skipRawData(binaryDataStride-sizeof(long int));
                if (data.atEnd())
                    break;
            }
        }
            break;
        case TYPE_INT32:
        {
            int tempInt;
            while (data.readRawData((char *) &tempInt, sizeof(tempInt)) != -1) {
                // add to data
                colData[colNum].push_back((double) tempInt);
                // skip to next row
                data.skipRawData(binaryDataStride-sizeof(int));
                if (data.atEnd())
                    break;
            }
        }
            break;
        case TYPE_STRING:
            return false;
        }
    }
        break;
    case CSVFormat:
    case SSVFormat:

        break;
    default:
        // oops, bad dataType
        return false;

    }

    //
    QVector < double > times;
    for (int i = 0; i < colData[colNum].size(); ++i) {
        times.push_back(((double) i)*timeStep);
    }

    if (update == -1) {
        // add graph and setup data and name
        plot->addGraph();
        plot->graph(plot->graphCount()-1)->setData(times, colData[colNum]);
        plot->graph(plot->graphCount()-1)->setName("Index " + QString::number(columns[colNum].index));

        // add properties to graph so we know what it came from
        plot->graph(plot->graphCount()-1)->setProperty("type", "linePlot");
        plot->graph(plot->graphCount()-1)->setProperty("source", logFileXMLname);
        plot->graph(plot->graphCount()-1)->setProperty("index", colNum);

        // axis labels
        plot->xAxis->setLabel("Time (ms)");
        plot->yAxis->setLabel(columns[colNum].heading + " (" + columns[colNum].dims + ")");

        // alternate colours
        QPen pen;
        pen.setColor((Qt::GlobalColor) (7+(plot->graphCount()-1)%11));
        plot->graph(plot->graphCount()-1)->setPen(pen);

        // fit all if not an update
        plot->rescaleAxes();


    } else {
        plot->graph(update)->setData(times, colData[colNum]);
    }

    plot->legend->setVisible(true);

    // title
    if (plot->plotLayout()->rowCount() == 1) {
        plot->plotLayout()->insertRow(0); // inserts an empty row above the default axis rect
        plot->plotLayout()->addElement(0, 0, new QCPPlotTitle(plot, logName));
    }

    // redraw
    plot->replot();

    return true;
}

bool logData::plotRaster(QCustomPlot * plot, QList < QVariant > indices, int update) {

    // if no plot give up
    if (plot == NULL)
        return false;

    // clear existing data;
    colData[0].clear();
    colData[1].clear();

    // get data
    for (int colNum = 0; colNum < 2; ++colNum) {
        switch (dataFormat) {
        case BINARY:
        {
            if (!calculateBinaryDataStride())
                return false;
            int offset = calculateBinaryDataOffset(colNum);
            if (offset == -1)
                return false;
            // stream data from file
            QDataStream data(&logFile);
            data.device()->seek(0);
            // offset into file
            data.skipRawData(offset);
            switch (columns[colNum].type) {
            case TYPE_DOUBLE:
            {
                double tempDbl;
                while (data.readRawData((char *) &tempDbl, sizeof(tempDbl)) != -1) {
                    // add to data
                    colData[colNum].push_back(tempDbl);
                    // skip to next row
                    data.skipRawData(binaryDataStride-sizeof(double));
                    if (data.atEnd())
                        break;
                }
            }
                break;
            case TYPE_FLOAT:
            {
                float tempFloat;
                data.setFloatingPointPrecision(QDataStream::SinglePrecision);
                while (data.readRawData((char *) &tempFloat, sizeof(tempFloat)) != -1) {
                    // add to data
                    colData[colNum].push_back((double) tempFloat);
                    // skip to next row
                    data.skipRawData(binaryDataStride-sizeof(float));
                    if (data.atEnd())
                        break;
                }
            }
                break;
            case TYPE_INT64:
            {
                // not supported currently
                    return false;
                int tempInt;
                while (data.readRawData((char *) &tempInt, sizeof(tempInt)) != -1) {
                    // add to data
                    colData[colNum].push_back((double) tempInt);
                    // skip to next row
                    data.skipRawData(binaryDataStride-sizeof(long int));
                    if (data.atEnd())
                        break;
                }
            }
                break;
            case TYPE_INT32:
            {
                int tempInt;
                while (data.readRawData((char *) &tempInt, sizeof(tempInt)) != -1) {
                    // add to data
                    colData[colNum].push_back((double) tempInt);
                    // skip to next row
                    data.skipRawData(binaryDataStride-sizeof(int));
                    if (data.atEnd())
                        break;
                }
            }
                break;
            case TYPE_STRING:
                return false;
            }
        }
            break;
        case CSVFormat:
        case SSVFormat:
        {
            // read line by line
            QTextStream data(&logFile);
            data.device()->seek(0);

            // read a line
            while (!data.atEnd()) {

                // get line
                QString line = data.readLine();

                // divide up
                QStringList cols;
                if (dataFormat == CSVFormat) {
                    line.remove(" ");
                    cols = line.split(",");
                }
                else if (dataFormat == SSVFormat) {
                    line = line.simplified();
                    cols = line.split(" ");
                }

                // parse
                if (cols.size() != (int) columns.size()) {
                    qDebug() << "Col size incorrect on import";
                    return false;
                }

                for (int i = 0; i < indices.size(); ++i) {
                    if (cols[1].toInt() == indices[i].toInt()) {
                        colData[0].push_back(cols[0].toDouble());
                        colData[1].push_back(cols[1].toDouble());
                    }
                }

            }
        }

            break;
        default:
            // oops, bad dataType
            qDebug() << "Bad dataType";
            return false;

        }
    }

    if (colData.size() != 2) {
        qDebug() << "Not 2 cols";
        return false;
    }

    // add graph and setup data and name, or update existing
    if (update == -1) {

        plot->addGraph();
        plot->graph(plot->graphCount()-1)->setData(colData[0], colData[1]);
        plot->graph(plot->graphCount()-1)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 1));
        plot->graph(plot->graphCount()-1)->setLineStyle(QCPGraph::lsNone);

        // add properties to graph so we know what it came from
        plot->graph(plot->graphCount()-1)->setProperty("type", "rasterPlot");
        plot->graph(plot->graphCount()-1)->setProperty("source", logFileXMLname);
        QVariant var(indices);
        plot->graph(plot->graphCount()-1)->setProperty("indices", var);

        // axis labels
        plot->xAxis->setLabel("Time (ms)");
        plot->yAxis->setLabel("Index");

        // alternate colours
        QPen pen;
        pen.setColor((Qt::GlobalColor) (7+(plot->graphCount()-1)%11));
        plot->graph(plot->graphCount()-1)->setPen(pen);

        plot->xAxis->setRange(0, endTime);
        plot->yAxis->setRange(-0.5, eventIndices.size()-0.5);
        plot->yAxis->setTickStep(1.0);

    } else {
        plot->graph(update)->setData(colData[0], colData[1]);
    }

    // title
    if (plot->plotLayout()->rowCount() == 1) {
        plot->plotLayout()->insertRow(0); // inserts an empty row above the default axis rect
        plot->plotLayout()->addElement(0, 0, new QCPPlotTitle(plot, logFile.fileName()));
    }

    // redraw
    plot->replot();

    return true;
}

bool logData::calculateBinaryDataStride() {

    binaryDataStride = 0;

    for (int i = 0; i < (int) columns.size(); ++i) {
        switch (columns[i].type) {
        case TYPE_DOUBLE:
            binaryDataStride += sizeof(double);
            break;
        case TYPE_FLOAT:
            binaryDataStride += sizeof(float);
            break;
        case TYPE_INT64:
            binaryDataStride += sizeof(long int);
            break;
        case TYPE_INT32:
            binaryDataStride += sizeof(int);
            break;
        case TYPE_STRING:
            binaryDataStride = 0;
            return false;
        }
    }
    return true;
}

int logData::calculateBinaryDataOffset(int colNum) {

    int tempInt = 0;

    for (int i = 0; i < colNum; ++i) {
        switch (columns[i].type) {
        case TYPE_DOUBLE:
            tempInt += sizeof(double);
            break;
        case TYPE_FLOAT:
            tempInt += sizeof(float);
            break;
        case TYPE_INT64:
            tempInt += sizeof(long int);
            break;
        case TYPE_INT32:
            tempInt += sizeof(int);
            break;
        case TYPE_STRING:
            return -1;
        }
    }
    return tempInt;
}

bool logData::setupFromXML() {

    // no log specified
    if (logFileXMLname.isEmpty()) {
        qDebug() << "XML filename not set";
        return false;
    }

    // open XML for reading
    QFile xmlfile( logFileXMLname );
    if( !xmlfile.open( QIODevice::ReadOnly ) ) {
        // could not open
        qDebug() << "Couldn't open XML file";
        return false;}


    // setup XML reader
    QXmlStreamReader * reader = new QXmlStreamReader;
    reader->setDevice( &xmlfile );

    // clear up
    columns.clear();
    eventIndices.clear();
    allLogged = false;
    min = Q_INFINITY;
    max = Q_INFINITY;

    // temp config data
    QString logFileName;

    // parse XML
    while (reader->readNextStartElement()) {

        if (reader->name() == "LogReport") {

            while (reader->readNextStartElement()) {

                if (reader->name() == "AnalogLog") {

                    dataClass = ANALOGDATA;

                    while (reader->readNextStartElement()) {

                        if (reader->name() == "LogFile") {

                            // store logfile name
                            logFileName = reader->readElementText();

                        } else if (reader->name() == "LogFileType") {

                            QString tempStr = reader->readElementText();
                            if (tempStr == "binary") {
                                dataFormat = BINARY;
                            } else if (tempStr == "csv") {
                                dataFormat = CSVFormat;
                            } else if (tempStr == "ssv") {
                                dataFormat = SSVFormat;
                            } else {
                                // invalid file format
                                qDebug() << "Invalid file format";
                                delete reader;
                                return false;
                            }

                        } else if (reader->name() == "LogEndTime") {

                            QString tempStr = reader->readElementText();
                            endTime = tempStr.toDouble();

                        } else if (reader->name() == "LogCol") {

                            column newCol;

                            if (reader->attributes().hasAttribute("index")) {
                                newCol.index = reader->attributes().value("index").toString().toInt();
                            } else {
                                // required attribute
                                qDebug() << "Index attr missing";
                                delete reader;
                                return false;
                            }

                            if (reader->attributes().hasAttribute("heading")) {
                                newCol.heading = reader->attributes().value("heading").toString();
                            } else {
                                // required attribute
                                qDebug() << "Heading attr missing";
                                delete reader;
                                return false;
                            }

                            if (reader->attributes().hasAttribute("dims")) {
                                newCol.dims = reader->attributes().value("dims").toString();
                            } else {
                                // required attribute
                                qDebug() << "Dims attr missing";
                                delete reader;
                                return false;
                            }

                            if (reader->attributes().hasAttribute("type")) {
                                QString tempStr = reader->attributes().value("type").toString();

                                if (tempStr == "double") {
                                    newCol.type = TYPE_DOUBLE;
                                } else if (tempStr == "float") {
                                    newCol.type = TYPE_FLOAT;
                                } else if (tempStr == "int") {
                                    newCol.type = TYPE_INT32;
                                } else if (tempStr == "longint") {
                                    newCol.type = TYPE_INT64;
                                } else if (tempStr == "string") {
                                    newCol.type = TYPE_STRING;
                                } else {
                                    // invalid data type
                                    qDebug() << "Wrong data type label";
                                    delete reader;
                                    return false;
                                }


                            } else {
                                // required attribute
                                qDebug() << "Type attr missing";
                                delete reader;
                                return false;
                            }

                            // as unclosed
                            reader->readNextStartElement();

                            // add column
                            columns.push_back(newCol);

                        } else if (reader->name() == "LogAll") {

                            allLogged = true;
                            column newCol;
                            int size;

                            if (reader->attributes().hasAttribute("size")) {
                                size = reader->attributes().value("size").toString().toInt();
                            } else {
                                // required attribute
                                qDebug() << "Size attr missing";
                                delete reader;
                                return false;
                            }

                            if (reader->attributes().hasAttribute("headings")) {
                                newCol.heading = reader->attributes().value("headings").toString();
                            } else {
                                // required attribute
                                qDebug() << "Headings attr missing";
                                delete reader;
                                return false;
                            }

                            if (reader->attributes().hasAttribute("dims")) {
                                newCol.dims = reader->attributes().value("dims").toString();
                            } else {
                                // required attribute
                                qDebug() << "Dims attr missing";
                                delete reader;
                                return false;
                            }

                            if (reader->attributes().hasAttribute("type")) {
                                QString tempStr = reader->attributes().value("type").toString();

                                if (tempStr == "double") {
                                    newCol.type = TYPE_DOUBLE;
                                } else if (tempStr == "float") {
                                    newCol.type = TYPE_FLOAT;
                                } else if (tempStr == "int") {
                                    newCol.type = TYPE_INT32;
                                } else if (tempStr == "longint") {
                                    newCol.type = TYPE_INT64;
                                } else if (tempStr == "string") {
                                    newCol.type = TYPE_STRING;
                                } else {
                                    // invalid data type
                                    qDebug() << "Wrong data type name";
                                    delete reader;
                                    return false;
                                }

                            } else {
                                // required attribute
                                qDebug() << "Type attr missing";
                                delete reader;
                                return false;
                            }

                            // as unclosed
                            reader->readNextStartElement();

                            // add columns
                            vector <column> temp = columns.toStdVector();
                            temp.resize(size, newCol);
                            columns = QVector<column>::fromStdVector(temp);

                            // setup indices
                            for (int i = 0; i < columns.size(); ++i)
                                columns[i].index = i;

                        } else if (reader->name() == "TimeStep") {

                            if (reader->attributes().hasAttribute("dt")) {
                                timeStep = reader->attributes().value("dt").toString().toDouble();
                            } else {
                                // required attribute
                                qDebug() << "Timestep attr missing";
                                delete reader;
                                return false;
                            }

                        } else {
                            // XML tag not recognised
                            qDebug() << "Unknown tag name " << reader->name();
                            delete reader;
                            return false;
                        }

                    }

                } else if (reader->name() == "EventLog") {

                    dataClass = EVENTDATA;

                    while (reader->readNextStartElement()) {

                        if (reader->name() == "LogFile") {

                            // store logfile name
                            logFileName = reader->readElementText();

                        } else if (reader->name() == "LogFileType") {

                            QString tempStr = reader->readElementText();
                            if (tempStr == "binary") {
                                dataFormat = BINARY;
                            } else if (tempStr == "csv") {
                                dataFormat = CSVFormat;
                            } else if (tempStr == "ssv") {
                                dataFormat = SSVFormat;
                            } else {
                                // invalid file format
                                qDebug() << "File format unkown";
                                delete reader;
                                return false;
                            }

                        } else if (reader->name() == "LogEndTime") {

                            QString tempStr = reader->readElementText();
                            endTime = tempStr.toDouble();

                        } else if (reader->name() == "LogCol") {

                            column newCol;

                            // event port logs don't use the index
                            newCol.index = -1;

                            if (reader->attributes().hasAttribute("heading")) {
                                newCol.heading = reader->attributes().value("heading").toString();
                            } else {
                                // required attribute
                                qDebug() << "Heading attr missing";
                                delete reader;
                                return false;
                            }

                            if (reader->attributes().hasAttribute("dims")) {
                                newCol.dims = reader->attributes().value("dims").toString();
                            } else {
                                // required attribute
                                qDebug() << "Dims attr missing";
                                delete reader;
                                return false;
                            }

                            if (reader->attributes().hasAttribute("type")) {
                                QString tempStr = reader->attributes().value("type").toString();

                                if (tempStr == "double") {
                                    newCol.type = TYPE_DOUBLE;
                                } else if (tempStr == "float") {
                                    newCol.type = TYPE_FLOAT;
                                } else if (tempStr == "int") {
                                    newCol.type = TYPE_INT32;
                                } else if (tempStr == "longint") {
                                    newCol.type = TYPE_INT64;
                                } else if (tempStr == "string") {
                                    newCol.type = TYPE_STRING;
                                } else {
                                    // invalid data type
                                    qDebug() << "Data type unknown";
                                    delete reader;
                                    return false;
                                }

                            } else {
                                // required attribute
                                qDebug() << "Type attr missing";
                                delete reader;
                                return false;
                            }

                            // as unclosed
                            reader->readNextStartElement();

                            // add column
                            columns.push_back(newCol);

                        } else if (reader->name() == "LogAll") {

                            allLogged = true;
                            int size;

                            if (reader->attributes().hasAttribute("size")) {
                                size = reader->attributes().value("size").toString().toInt();
                            } else {
                                // required attribute
                                qDebug() << "Size attr missing";
                                delete reader;
                                return false;
                            }

                            // add indices
                            for (int i = 0; i < size; ++i)
                                eventIndices.push_back(i);

                            // as unclosed
                            reader->readNextStartElement();

                        } else if (reader->name() == "LogIndex") {

                            eventIndices.push_back(reader->readElementText().toInt());


                        } else if (reader->name() == "LogPort") {

                            eventPortName = reader->readElementText();

                        } else if (reader->name() == "TimeStep") {

                            if (reader->attributes().hasAttribute("dt")) {
                                timeStep = reader->attributes().value("dt").toString().toDouble();
                            } else {
                                // required attribute
                                qDebug() << "Timestep attr missing";
                                delete reader;
                                return false;
                            }

                        } else {
                            // XML tag not recognised
                            qDebug() << "XML tag not recognised";
                            delete reader;
                            return false;
                        }

                    }

                } else {
                    // log type not recognised
                    qDebug() << "Log type unknown";
                    delete reader;
                    return false;
                }
            }

        } else {
            // not a LogReport
            qDebug() << "Not a logreport";
            delete reader;
            return false;
        }

    }

    // load the log file
    // get local dir
    QString dirPath = logFileXMLname;
    dirPath.resize(dirPath.lastIndexOf(QDir::separator()));
    QDir localDir(dirPath);

    if (!logFile.isOpen()) {
        logFile.setFileName(localDir.absoluteFilePath(logFileName));
        if( !logFile.open( QIODevice::ReadOnly ) ) {
            // could not open
            qDebug() << "Couldn't open log file " << localDir.absoluteFilePath(logFileName);
            delete reader;
            return false;}
    }

    // resize data carriers
    colData.resize(columns.size());

    // log name
    logName = logFileName;

    // all successful!
    delete reader;
    return true;
}
