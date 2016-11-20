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

#include "SC_network_layer_rootdata.h"
#include "CL_classes.h"
#include "NL_population.h"
#include "NL_systemobject.h"


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
    virtual void write_metadata_xml(QDomDocument &, QDomNode &) {}
    virtual void read_metadata_xml(QDomNode &) {}
    virtual void writeDelay(QXmlStreamWriter &xmlOut);
    virtual QLayout * drawLayout(nl_rootdata * , viewVZLayoutEditHandler * , nl_rootlayout * ) {return new QHBoxLayout();}

    virtual connection * newFromExisting() {return new connection;}
    virtual int getIndex();

    virtual QString getTypeStr(void);

    /*!
     * The parent object which contains this connection. Will be
     * either a GenericInput or a Synapse. The introduction of this
     * makes this->synapseIndex possibly unnecessary. This was
     * introduced so that csv_connections could contain the parent
     * that they relate to, so that a checkbox in the panel could be
     * used to change the connection features.
     */
    QSharedPointer<systemObject> parent;

    /*!
     * Setter for @see parent.
     */
    void setParent (QSharedPointer<systemObject> ptr);

    /*!
     * A ParameterInstance is all about a list of parameter values,
     * but can also be FixedValue.
     *
     * A csv_connection or fixedprob_connection in a generic input
     * needs a fixedvalue delay for our canonical simulator,
     * SpineML_2_BRAHMS. It's necessary to be able to define a
     * fixedvalue delay for a connection list in a generic input.
     */
    ParameterInstance * delay;

    /*!
     * The source population for this connection.
     */
    QSharedPointer <population> src;

    /*!
     * The destination population for this connection.
     */
    QSharedPointer <population> dst;

    /*!
     * Setter for srcName.
     */
    void setSrcName (QString& s);

    /*!
     * Setter for dstName.
     */
    void setDstName (QString& d);

    /*!
     * Obtain the index number of the synapse in which this connection
     * exists.
     */
    int getSynapseIndex();

    /*!
     * Set the index number of the synapse in which this connection
     * exists.
     */
    void setSynapseIndex(int synidx);

private:
    QString filename;

protected:
    /*!
     * The name of the source population for this connection. This may
     * be used where it is inconvenient to set the
     * QSharedPointer<population> src.
     */
    QString srcName;

    /*!
     * The name of the dest population for this connection. This may
     * be used where it is inconvenient to set the
     * QSharedPointer<population> dst.
     */
    QString dstName;

    /*!
     * The synapse index for this connection (a projection between two
     * populations can have many synapses, which count from
     * 0). Initialised in constructors to a negative number, which
     * must be changed later to a valid number 0 or above.
     */
    int synapseIndex;
};

class alltoAll_connection : public connection
{
        Q_OBJECT
public:
    alltoAll_connection();
    ~alltoAll_connection();

    void write_node_xml(QXmlStreamWriter &xmlOut);
    void import_parameters_from_xml(QDomNode &);
    QLayout * drawLayout(nl_rootdata * data, viewVZLayoutEditHandler * viewVZhandler, nl_rootlayout * rootLay);
    connection * newFromExisting() {alltoAll_connection * c = new alltoAll_connection; c->delay = new ParameterInstance(this->delay); return c;}

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
    QLayout * drawLayout(nl_rootdata * data, viewVZLayoutEditHandler * viewVZhandler, nl_rootlayout * rootLay);
    connection * newFromExisting() {onetoOne_connection * c = new onetoOne_connection; c->delay = new ParameterInstance(this->delay); return c;}

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
    QLayout * drawLayout(nl_rootdata * data, viewVZLayoutEditHandler * viewVZhandler, nl_rootlayout * rootLay);
    connection * newFromExisting() {
        fixedProb_connection * c = new fixedProb_connection;
        c->p = this->p;
        c->seed = this->seed;
        c->delay = new ParameterInstance(this->delay);
        return c;
    }


    // the probability of a connection
    float p;
    int seed;

private:
};

/*!
 * \brief The csv_connection class
 * This class is a subclass of connection. It allows the use of explicit connection lists
 * in the form of source-destination pairs, with an optional individual or global delay.
 */
class csv_connection : public connection
{
    Q_OBJECT
public:
    csv_connection();
    ~csv_connection();

    /*!
     * Column headers really. A list of 2 or 3 strings as headers.
     */
    QStringList values;

    connection* generator;

    /*!
     * \brief import_csv
     * \param filename
     * Import data into the connection from a file written in Comma Seperated Variable format.
     * This format consists of ASCII text data written as S,D,L/n where S is the source index,
     * D is the destination index and L (optional) is the delay.
     *
     * Returns false if the import failed for any reason, otherwise returns true.
     */
    bool import_csv (QString filename);

    /*!
     * \brief import_packed_binary
     * \param fileIn The source file from which to import
     * \param fileOut The connection's storage file
     * Import data into the connection from a file written in the packed binary format. This
     * format consists of close packed binary data with the structure (int S)(int D)(opt float L)
     * where S is the source index, D is the dest index, and optionally L is the delay.
     */
    void import_packed_binary (QFile &fileIn, QFile& fileOut);

    void getAllData (QVector<conn>& conns);
    float getData (int, int) const;
    float getData (QModelIndex &index) const;
    QString getHeader (int section);
    int getNumRows (void) const;
    void setNumRows (int);

    /*!
     * Get the number of columns of data - the size of this->values.
     */
    int getNumCols (void) const;

    /*!
     * Set the number of cols required in values. Destructively
     * resizes this->values.
     */
    void setNumCols (int n);

    /*!
     * Updates the data based on the current numcols, in case that has
     * changed.
     */
    void updateDataForNumCols (int num);

    void setData (const QModelIndex& index, float value);
    void setData (int, int, float);

    /*!
     * Write out the connection data. If numCols is 3, then write out
     * the fixedValue delay if it exists, other wise 0 for the delay
     * column.
     */
    void setAllData (QVector<conn>& conns);

    void clearData (void);
    void flushChangesToDisk (void);
    void abortChanges (void);
    /*!
     * Write out the node xml to the XML files, using the final
     * version of the filename for any file-based explicit connection
     * list.
     */
    void write_node_xml (QXmlStreamWriter& xmlOut);
    void write_metadata_xml (QDomDocument&, QDomNode&);
    void import_parameters_from_xml (QDomNode&);
    void read_metadata_xml (QDomNode&);
    void setFileName (QString name);
    QString getFileName (void);
    QString getUUIDFileName (void);
    void fetch_headings (void);
    QLayout* drawLayout (nl_rootdata*, viewVZLayoutEditHandler* viewVZhandler, nl_rootlayout* rootLay);
    int getIndex (void);
    connection* newFromExisting (void);

    /*!
     * Copy the values from other into this. If both values have same
     * number of cols, then direct copy the data and values, otherwise
     * only copy src/dst values.
     */
    void copyDataValues (const csv_connection* other);

private:

    /*!
     * If the connection has explicit data which needs to be stored in
     * a binary file, then it needs a filename into which that data is
     * stored. This is the "final, human-readable" version of that
     * file, which is written when a model is saved out of
     * SpineCreator. While a model is being edited/created, the
     * temporary, uuidFilename is used instead.
     */
    QString filename;

    /*!
     * A temporary filename. This is used only if the connection has
     * explicit data which needs to be stored in a binary file. It is
     * used while the model is being worked on. It is generated by
     * generateUUIDFilename(). When the model is written out,
     * generateFilename() is used to generate the final, src & dst
     * based filename which is then written out into the model
     * directory.
     */
    QString uuidFilename;

    QXmlStreamWriter xmlOut;
    QXmlStreamReader xmlIn;

    /*!
     * The number of rows expected in the data file.
     */
    int numRows;

    /*!
     * The number of columns in the values QStringList and in the data
     * file. Should be 2 or 3.
     */
    int numCols;

    QVector<change> changes;
    csv_connection* copiedFrom;

    /*!
     * Generate a filename based on the source and destination
     * population names, throwing an exception if either of these is
     * not set.
     */
    void generateFilename (void);

    /*!
     * Generate a unique filename for temporary use. This is used up
     * until the point at which the network file is written out.
     */
    void generateUUIDFilename (void);

    /*!
     * Get the directory used for storage of the connection binary
     * files - i.e. the model directory.
     */
    QDir getLibDir (void) const;

    /*!
     * Replace chars in str which are not in the string allowed with
     * replaceChar.
     */
    void sanitizeReplace (QString& str,
                          const QString& allowed,
                          const char replaceChar);

    /*!
     * Sort connection data by src and dst indices.
     */
    void sortData (void);

    /*!
     * Function to be used with std::sort to sort connections.
     */
    static bool sorttwo (conn a, conn b);

public slots:
    /*!
     * Called when the "Global delay" checkbox is changed.
     */
    void updateGlobalDelay (void);
};

class pythonscript_connection : public connection
{
        Q_OBJECT
public:
    pythonscript_connection(QSharedPointer <population> src, QSharedPointer <population> dst, csv_connection *conn_targ);
    pythonscript_connection() {
        type = Python;
        this->isAList = false;
        selfConnections = false;
        rotation = 0;
        hasChanged = true;
        this->scriptValidates = false;
        this->hasWeight = false;
        this->hasDelay = false;
    }

    ~pythonscript_connection();

    void write_node_xml(QXmlStreamWriter &xmlOut);
    void import_parameters_from_xml(QDomNode &);
    void write_metadata_xml(QDomDocument &, QDomNode &);
    void read_metadata_xml(QDomNode &);
    int getIndex();
    QString getTypeStr(void);

    float rotation;
    QString errorLog;

    QSharedPointer <population> src;
    QSharedPointer <population> dst;
    QVector < conn > *conns;
    QMutex * mutex;
    bool isList();
    bool selfConnections;
    bool changed();

    QVector <conn> connections;

    QString scriptText;
    QString lastGeneratedScriptText;
    QString scriptName;
    QStringList parNames;
    QVector <double> parValues;
    QVector <double> lastGeneratedParValues;
    QVector < QPoint > parPos;

    QString weightProp;
    QString lastGeneratedWeightProp;
    QVector <double> weights;

    QString pythonErrors;

    bool scriptValidates;
    bool hasWeight;
    bool hasDelay;

    ParameterInstance *getPropPointer();
    QStringList getPropList();
    QLayout * drawLayout(nl_rootdata * data, viewVZLayoutEditHandler * viewVZhandler, nl_rootlayout * rootLay);

    // the explicit connection list to copy the generated weights to
    csv_connection * connection_target;

    connection * newFromExisting();

private:

    csv_connection * explicitList;
    bool isAList;
    bool hasChanged;
    int srcSize;
    int dstSize;


public slots:
    void generate_connections();
    //void convertToList(bool);
    /*!
     * \brief configureFromScript
     * Get a Python script as a string and parse it to set up the connection
     * parameters. Is a slot so it can be triggered by events.
     */
    void configureFromScript(QString);

    void regenerateConnections();

    void setUnchanged(bool);

    /*!
     * \brief enableGen
     * A simple slot to allow the QDoubleSpinBoxes with the pars to re-enable the Generator button
     */
    void enableGen(double);

    /*!
     * \brief enableGen
     * A simple slot to allow the QComboBox with the Propoerty for the weight to re-enable the Generator button
     */
    void enableGen(int);

signals:
    void progress(int);
    void connectionsDone();
    void setGenEnabled(bool);

};

#endif // CONNECTION_H
