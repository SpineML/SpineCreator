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

#ifndef CONNECTION_H
#define CONNECTION_H

#include "globalHeader.h"

#include "rootdata.h"
#include "nineML_classes.h"
#include "population.h"

#define NO_DELAY -1 // used to determine if Python Scripts have delay data

struct change {
    int row;
    int col;
    float value;
};

class connection: public QObject
{
    Q_OBJECT

public:
    connection();
    virtual ~connection();

    QString name;
    connectionType type;

    virtual void write_node_xml(QXmlStreamWriter &){}
    virtual void import_parameters_from_xml(QDomNode &){}
    virtual void writeDelay(QXmlStreamWriter &xmlOut);
    ParameterData * delay;

private:
    QString filename;
    QFile file;

};

class alltoAll_connection : public connection
{
        Q_OBJECT
public:
    alltoAll_connection();
    ~alltoAll_connection();

    void write_node_xml(QXmlStreamWriter &xmlOut);
    void import_parameters_from_xml(QDomNode &);

private:
};

class onetoOne_connection : public connection
{
        Q_OBJECT
public:
    onetoOne_connection();
    ~onetoOne_connection();

    void write_node_xml(QXmlStreamWriter &xmlOut);
    void import_parameters_from_xml(QDomNode &);

private:
};

class fixedProb_connection : public connection
{
        Q_OBJECT
public:
    fixedProb_connection();
    ~fixedProb_connection();

    QStringList values;
    void write_node_xml(QXmlStreamWriter &xmlOut);
    void import_parameters_from_xml(QDomNode &);

    // the probability of a connection
    float p;
    int seed;

private:
};

class csv_connection : public connection
{
        Q_OBJECT
public:

    csv_connection(QString fileName);
    csv_connection();
    ~csv_connection();

    QStringList values;
    void import_csv(QString filename);
    vector <float> fetchData(int index);
    void getAllData(vector < conn > &conns);
    float getData(int, int);
    float getData(QModelIndex &index);
    QString getHeader(int section);
    int getNumRows();
    void setNumRows(int);
    int getNumCols();
    void setNumCols(int);
    void setData(const QModelIndex & index, float value);
    void setData(int, int, float);
    void flushChangesToDisk();
    void abortChanges();
    void write_node_xml(QXmlStreamWriter &xmlOut);
    void import_parameters_from_xml(QDomNode &);
    void setFileName(QString name);
    QString getFileName();
    void fetch_headings();

private:
    QString filename;
    QFile file;
    QXmlStreamWriter xmlOut;
    QXmlStreamReader xmlIn;
    int numRows;
    vector < change > changes;
    void setUniqueName();

};

class distanceBased_connection : public connection
{
        Q_OBJECT
public:
    distanceBased_connection();
    ~distanceBased_connection();

    void write_node_xml(QXmlStreamWriter &xmlOut);
    void import_parameters_from_xml(QDomNode &);

    QString equation;
    QString delayEquation;
    QString errorLog;

    population * src;
    population * dst;
    vector < conn > *conns;
    QMutex * mutex;
    bool isList();
    bool selfConnections;
    vector <conn> connections;
    bool changed();
    bool changed(QString);
    void setUnchanged(bool);


private:
    csv_connection * explicitList;
    bool isAList;
    bool hasChanged;
    int srcSize;
    int dstSize;


public slots:
    //void generate_connections(population * src, population * dst, vector < conn > &conns);
    void generate_connections();
    void convertToList(bool);

signals:
    void progress(int);
    void connectionsDone();

};

class kernel_connection : public connection
{
        Q_OBJECT
public:
    kernel_connection();
    ~kernel_connection();

    void write_node_xml(QXmlStreamWriter &xmlOut);
    void import_parameters_from_xml(QDomNode &);

    float kernel[11][11];
    int kernel_size;
    float kernel_scale;
    float rotation;
    QString errorLog;

    population * src;
    population * dst;
    vector < conn > *conns;
    QMutex * mutex;
    bool isList();
    bool selfConnections;
    bool changed();
    void setUnchanged(bool);
    vector <conn> connections;

private:
    csv_connection * explicitList;
    bool isAList;
    bool hasChanged;
    int srcSize;
    int dstSize;

public slots:
    //void generate_connections(population * src, population * dst, vector < conn > &conns);
    void generate_connections();
    void convertToList(bool);
    void setKernelSize(int);
    void setKernelScale(float);
    void setKernel(int,int,float);

signals:
    void progress(int);
    void connectionsDone();

};

class pythonscript_connection : public connection
{
        Q_OBJECT
public:
    pythonscript_connection();
    ~pythonscript_connection();

    void write_node_xml(QXmlStreamWriter &xmlOut);
    void import_parameters_from_xml(QDomNode &);

    float rotation;
    QString errorLog;

    population * src;
    population * dst;
    vector < conn > *conns;
    QMutex * mutex;
    bool isList();
    bool selfConnections;
    bool changed();

    vector <conn> connections;

    QString scriptText;
    QStringList parNames;
    QVector <float> parValues;
    QVector < QPoint > parPos;

    QString weightProp;
    vector <float> weights;

    QString pythonErrors;

    bool scriptValidates;
    bool hasWeight;
    bool hasDelay;

    ParameterData *getPropPointer();
    QStringList getPropList();

private:
    void regenerateConnections();
    csv_connection * explicitList;
    bool isAList;
    bool hasChanged;
    int srcSize;
    int dstSize;


public slots:
    void generate_connections();
    void convertToList(bool);
    /*!
     * \brief configureFromScript
     * Get a Python script as a string and parse it to set up the connection
     * parameters. Is a slot so it can be triggered by events.
     */
    void configureFromScript(QString);

    void configureFromTextEdit();

    void configureAfterLoad();

    void setUnchanged(bool);

signals:
    void progress(int);
    void connectionsDone();

};

#endif // CONNECTION_H
