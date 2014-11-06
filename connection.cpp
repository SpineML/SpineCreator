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

#include <Python.h>

#include "connection.h"
#include "cinterpreter.h"
#include "generate_dialog.h"
#include "viewVZlayoutedithandler.h"
#include "filteroutundoredoevents.h"

connection::connection()
{
    type = none;
    delay = new ParameterData("ms");
    delay->name = "delay";
    delay->currType = Undefined;
}

connection::~connection()
{
    delete delay;
}

int connection::getIndex()
{
    return (int) this->type;
}

void connection::writeDelay(QXmlStreamWriter &xmlOut)
{
    xmlOut.writeStartElement("Delay");

    xmlOut.writeAttribute("Dimension", this->delay->dims->toString());

    if (this->delay->currType == FixedValue) {
        xmlOut.writeEmptyElement("FixedValue");
        xmlOut.writeAttribute("value", QString::number(this->delay->value[0]));
    }
    else if (this->delay->currType == Statistical) {
        switch (int(round(this->delay->value[0]))) {
        case 0:
            break;
        case 1:
        {
            xmlOut.writeEmptyElement("UniformDistribution");
            xmlOut.writeAttribute("minimum", QString::number(this->delay->value[1]));
            xmlOut.writeAttribute("maximum", QString::number(this->delay->value[2]));
            xmlOut.writeAttribute("seed", QString::number(this->delay->value[3]));
        }
            break;
        case 2:
        {
            xmlOut.writeEmptyElement("NormalDistribution");
            xmlOut.writeAttribute("mean", QString::number(this->delay->value[1]));
            xmlOut.writeAttribute("variance", QString::number(this->delay->value[2]));
            xmlOut.writeAttribute("seed", QString::number(this->delay->value[3]));
         }
            break;
        }
    }
    else if (this->delay->currType == ExplicitList) {
        xmlOut.writeStartElement("ValueList");
        for (int ind = 0; ind < this->delay->value.size(); ++ind) {
            xmlOut.writeEmptyElement("ValueInstance");
            xmlOut.writeAttribute("index", QString::number(float(this->delay->indices[ind])));
            xmlOut.writeAttribute("value", QString::number(float(this->delay->value[ind])));
        }
       xmlOut.writeEndElement(); // valueList
    }
    else {
        xmlOut.writeEmptyElement("FixedValue");
        xmlOut.writeAttribute("value", QString::number(0.0)); // default to fixed zero delay
    }

    xmlOut.writeEndElement(); // delay
}

/////////////////////////////// ALL TO ALL

alltoAll_connection::alltoAll_connection()
{
    type = AlltoAll;
}

alltoAll_connection::~alltoAll_connection()
{
}

QLayout * alltoAll_connection::drawLayout(rootData *, viewVZLayoutEditHandler * viewVZhandler, rootLayout * rootLay)
{
    QHBoxLayout * hlay = new QHBoxLayout();
    if (viewVZhandler) {
        connect(viewVZhandler, SIGNAL(deleteProperties()), hlay, SLOT(deleteLater()));
    }
    if (rootLay) {
        connect(rootLay, SIGNAL(deleteProperties()), hlay, SLOT(deleteLater()));
    }
    return hlay;
}

void alltoAll_connection::write_node_xml(QXmlStreamWriter &xmlOut)
{
    xmlOut.writeStartElement("AllToAllConnection");
    this->writeDelay(xmlOut);
    xmlOut.writeEndElement(); // allToAllConnection
}

void alltoAll_connection::import_parameters_from_xml(QDomNode &e)
{
    QDomNodeList delayProp = e.toElement().elementsByTagName("Delay");
    if (delayProp.size() == 1) {

        QDomNode n = delayProp.item(0);

        QDomNodeList propVal = n.toElement().elementsByTagName("FixedValue");
        if (propVal.size() == 1) {
            this->delay->currType = FixedValue;
            this->delay->value.resize(1);
            this->delay->value.fill(0);
            this->delay->value[0] = propVal.item(0).toElement().attribute("value").toFloat();
        }
        propVal = n.toElement().elementsByTagName("UniformDistribution");
        if (propVal.size() == 1) {
            this->delay->currType = Statistical;
            this->delay->value.resize(4);
            this->delay->value.fill(0);
            this->delay->value[0] = 1;
            this->delay->value[1] = propVal.item(0).toElement().attribute("minimum").toFloat();
            this->delay->value[2] = propVal.item(0).toElement().attribute("maximum").toFloat();
            this->delay->value[3] = propVal.item(0).toElement().attribute("seed").toFloat();
        }
        propVal = n.toElement().elementsByTagName("NormalDistribution");
        if (propVal.size() == 1) {
            this->delay->currType = Statistical;
            this->delay->value.resize(4);
            this->delay->value.fill(0);
            this->delay->value[0] = 2;
            this->delay->value[1] = propVal.item(0).toElement().attribute("mean").toFloat();
            this->delay->value[2] = propVal.item(0).toElement().attribute("variance").toFloat();
            this->delay->value[3] = propVal.item(0).toElement().attribute("seed").toFloat();
        }
    }
}

/////////////////////////////// ONE TO ONE

onetoOne_connection::onetoOne_connection()
{
    type = OnetoOne;
}

onetoOne_connection::~onetoOne_connection()
{
}

QLayout * onetoOne_connection::drawLayout(rootData *, viewVZLayoutEditHandler * viewVZhandler, rootLayout * rootLay)
{
    QHBoxLayout * hlay = new QHBoxLayout();
    if (viewVZhandler) {
        connect(viewVZhandler, SIGNAL(deleteProperties()), hlay, SLOT(deleteLater()));
    }
    if (rootLay) {
        connect(rootLay, SIGNAL(deleteProperties()), hlay, SLOT(deleteLater()));
    }
    return hlay;
}

void onetoOne_connection::write_node_xml(QXmlStreamWriter &xmlOut)
{
    xmlOut.writeStartElement("OneToOneConnection");
    this->writeDelay(xmlOut);
    xmlOut.writeEndElement(); // oneToOneConnection
}

void onetoOne_connection::import_parameters_from_xml(QDomNode &e)
{
    QDomNodeList delayProp = e.toElement().elementsByTagName("Delay");
    if (delayProp.size() == 1) {

        QDomNode n = delayProp.item(0);

        QDomNodeList propVal = n.toElement().elementsByTagName("FixedValue");
        if (propVal.size() == 1) {
            this->delay->currType = FixedValue;
            this->delay->value.resize(1);
            this->delay->value.fill(0);
            this->delay->value[0] = propVal.item(0).toElement().attribute("value").toFloat();
        }
        propVal = n.toElement().elementsByTagName("UniformDistribution");
        if (propVal.size() == 1) {
            this->delay->currType = Statistical;
            this->delay->value.resize(4);
            this->delay->value.fill(0);
            this->delay->value[0] = 1;
            this->delay->value[1] = propVal.item(0).toElement().attribute("minimum").toFloat();
            this->delay->value[2] = propVal.item(0).toElement().attribute("maximum").toFloat();
            this->delay->value[3] = propVal.item(0).toElement().attribute("seed").toFloat();
        }
        propVal = n.toElement().elementsByTagName("NormalDistribution");
        if (propVal.size() == 1) {
            this->delay->currType = Statistical;
            this->delay->value.resize(4);
            this->delay->value.fill(0);
            this->delay->value[0] = 2;
            this->delay->value[1] = propVal.item(0).toElement().attribute("mean").toFloat();
            this->delay->value[2] = propVal.item(0).toElement().attribute("variance").toFloat();
            this->delay->value[3] = propVal.item(0).toElement().attribute("seed").toFloat();
        }
    }
}

//////////////////////////////////////// FIXED PROBABILITY

fixedProb_connection::fixedProb_connection()
{
    type = FixedProb;
    p = 0.01;
    seed = 123;
}


fixedProb_connection::~fixedProb_connection()
{
}

/*!
 * \brief fixedProb_connection::drawLayout
 * \param data
 * \param viewVZhandler
 * \param rootLay
 * \return
 *
 * Draw the UI for the Fixed Probability connection - caller can be either the Visualiser layout handler, or the Network layout
 * handler, so we allow both to be passed and ignore the one set to NULL
 */
QLayout * fixedProb_connection::drawLayout(rootData * data, viewVZLayoutEditHandler * viewVZhandler, rootLayout * rootLay)
{
    // draw up probability changer
    QHBoxLayout * hlay = new QHBoxLayout;

    QDoubleSpinBox *pSpin = new QDoubleSpinBox;
    pSpin->setRange(0, 1);
    pSpin->setSingleStep(0.1);
    pSpin->setMaximumWidth(60);
    pSpin->setDecimals(3);
    pSpin->setValue(this->p);
    pSpin->setProperty("valToChange", "1");
    pSpin->setProperty("conn", "true");
    pSpin->setToolTip("connection probability");
    pSpin->setProperty("ptr", qVariantFromValue((void *) this));
    pSpin->setProperty("action","changeConnProb");
    pSpin->setFocusPolicy(Qt::StrongFocus);
    pSpin->installEventFilter(new FilterOutUndoRedoEvents);
    connect(pSpin, SIGNAL(editingFinished()), data, SLOT (updatePar()));
    hlay->addWidget(new QLabel("Probability: "));
    if (data->main->viewVZ.OpenGLWidget) {
        connect(pSpin, SIGNAL(editingFinished()), data->main->viewVZ.OpenGLWidget, SLOT (parsChangedProjection()));
    }
    // do connections to other classes
    if (viewVZhandler) {
        connect(viewVZhandler, SIGNAL(deleteProperties()), pSpin, SLOT(deleteLater()));
        connect(viewVZhandler, SIGNAL(deleteProperties()), hlay, SLOT(deleteLater()));
        connect(viewVZhandler, SIGNAL(deleteProperties()), hlay->itemAt(hlay->count()-1)->widget(), SLOT(deleteLater()));
    }
    if (rootLay) {
        connect(rootLay, SIGNAL(deleteProperties()), pSpin, SLOT(deleteLater()));
        connect(rootLay, SIGNAL(deleteProperties()), hlay, SLOT(deleteLater()));
        connect(rootLay, SIGNAL(deleteProperties()), hlay->itemAt(hlay->count()-1)->widget(), SLOT(deleteLater()));
    }
    hlay->addWidget(pSpin);

    return hlay;
}

void fixedProb_connection::write_node_xml(QXmlStreamWriter &xmlOut)
{
    xmlOut.writeStartElement("FixedProbabilityConnection");
    xmlOut.writeAttribute("probability", QString::number(this->p));
    xmlOut.writeAttribute("seed", QString::number(this->seed));
    this->writeDelay(xmlOut);
    xmlOut.writeEndElement(); // fixedProbabilityConnection
}

void fixedProb_connection::import_parameters_from_xml(QDomNode &e)
{
    this->p = e.toElement().attribute("probability").toFloat();
    this->seed = e.toElement().attribute("seed").toInt();

    QDomNodeList delayProp = e.toElement().elementsByTagName("Delay");
    if (delayProp.size() == 1) {

        QDomNode n = delayProp.item(0);

        QDomNodeList propVal = n.toElement().elementsByTagName("FixedValue");
        if (propVal.size() == 1) {
            this->delay->currType = FixedValue;
            this->delay->value.resize(1);
            this->delay->value.fill(0);
            this->delay->value[0] = propVal.item(0).toElement().attribute("value").toFloat();
        }
        propVal = n.toElement().elementsByTagName("UniformDistribution");
        if (propVal.size() == 1) {
            this->delay->currType = Statistical;
            this->delay->value.resize(4);
            this->delay->value.fill(0);
            this->delay->value[0] = 1;
            this->delay->value[1] = propVal.item(0).toElement().attribute("minimum").toFloat();
            this->delay->value[2] = propVal.item(0).toElement().attribute("maximum").toFloat();
            this->delay->value[3] = propVal.item(0).toElement().attribute("seed").toFloat();
        }
        propVal = n.toElement().elementsByTagName("NormalDistribution");
        if (propVal.size() == 1) {
            this->delay->currType = Statistical;
            this->delay->value.resize(4);
            this->delay->value.fill(0);
            this->delay->value[0] = 2;
            this->delay->value[1] = propVal.item(0).toElement().attribute("mean").toFloat();
            this->delay->value[2] = propVal.item(0).toElement().attribute("variance").toFloat();
            this->delay->value[3] = propVal.item(0).toElement().attribute("seed").toFloat();
        }
    }
}

/////////////////////////////////// EXPLICIT LIST

csv_connection::csv_connection()
{
    type = CSV;
    numRows = 0;
    setUniqueName();
    // no connectivity generator in constructor
    generator = NULL;

    this->values.push_back("src");
    this->values.push_back("dst");
    this->values.push_back("delay");

    // create the file:

    // start investigating the library
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    QDir lib_dir = QDir(QDesktopServices::storageLocation(QDesktopServices::DataLocation));
#else
    QDir lib_dir = QDir(QStandardPaths::writableLocation(QStandardPaths::DataLocation));
#endif
    if (!lib_dir.exists()) {
        if (!lib_dir.mkpath(lib_dir.absolutePath())) {
            qDebug() << "error creating library";
        }
    }

    qDebug() << lib_dir;

    this->file.setFileName(lib_dir.absoluteFilePath(this->filename));

    // open the storage file
    if( !this->file.open( QIODevice::ReadWrite ) ) {
        QMessageBox msgBox;
        msgBox.setText("Could not open output file for conversion");
        msgBox.exec();
        return;
    }
}

csv_connection::~csv_connection()
{
    // remove generator
    if (this->generator) {
        delete this->generator;
        this->generator = NULL;
    }

    // remove memory usage
    if (this->file.isOpen()) {
        this->file.close();
    }
}

int csv_connection::getIndex()
{
    if (!this->generator) {
        return (int) this->type;
    } else {
        return this->generator->getIndex();
    }
}

QLayout * csv_connection::drawLayout(rootData * data, viewVZLayoutEditHandler * viewVZhandler, rootLayout * rootLay)
{
    // if we do not have a generator...
    if (this->generator == NULL) {

        QHBoxLayout * hlay = new QHBoxLayout();

        QTableView *tableView = new QTableView();

        csv_connectionModel *connMod = new csv_connectionModel();
        connMod->setConnection(this);
        tableView->setModel(connMod);

        hlay->addWidget(tableView);

        QPushButton *import = new QPushButton("Import");
        import->setMaximumWidth(70);
        import->setMaximumHeight(28);
        import->setToolTip("Import the explicit value list");
        import->setProperty("ptr", qVariantFromValue((void *) this));

        hlay->addWidget(import);

        // add connection:
        connect(import, SIGNAL(clicked()), data, SLOT(editConnections()));

       // set up GL:
        if (viewVZhandler) {
            connect(viewVZhandler, SIGNAL(deleteProperties()), tableView, SLOT(deleteLater()));
            if (viewVZhandler->viewVZ->OpenGLWidget->getConnectionsModel() != (QAbstractTableModel *)0)
            {
                // don't fetch data if we already have for this connection
                csv_connectionModel * connModel = dynamic_cast<csv_connectionModel *> (viewVZhandler->viewVZ->OpenGLWidget->getConnectionsModel());
                CHECK_CAST(connModel);
                if (connModel->getConnection() == this  && (int) viewVZhandler->viewVZ->OpenGLWidget->connections.size() == this->getNumRows()) {
                    viewVZhandler->viewVZ->OpenGLWidget->setConnectionsModel(connMod);
                } else {
                    viewVZhandler->viewVZ->OpenGLWidget->setConnectionsModel(connMod);
                    viewVZhandler->viewVZ->OpenGLWidget->getConnections();
                }
            } else {
                viewVZhandler->viewVZ->OpenGLWidget->setConnectionsModel(connMod);
                viewVZhandler->viewVZ->OpenGLWidget->getConnections();
            }

            connect(connMod, SIGNAL(dataChanged(QModelIndex,QModelIndex)), viewVZhandler->viewVZ->OpenGLWidget, SLOT(connectionDataChanged(QModelIndex,QModelIndex)));
            connect(tableView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), viewVZhandler->viewVZ->OpenGLWidget, SLOT(connectionSelectionChanged(QItemSelection,QItemSelection)));
            connect(viewVZhandler, SIGNAL(deleteProperties()), import, SLOT(deleteLater()));
            connect(viewVZhandler, SIGNAL(deleteProperties()), hlay, SLOT(deleteLater()));
        }
        if (rootLay) {
            //
            connect(rootLay, SIGNAL(deleteProperties()), import, SLOT(deleteLater()));
            connect(rootLay, SIGNAL(deleteProperties()), tableView, SLOT(deleteLater()));
            connect(rootLay, SIGNAL(deleteProperties()), hlay, SLOT(deleteLater()));
        }

        return hlay;

    } else {

        // we have a generator, so pass on the task of drawing the layout to it!
        return generator->drawLayout(data, viewVZhandler, rootLay);

    }
}

csv_connection::csv_connection(QString fileName)
{
    type = CSV;
    numRows = 0;
    setUniqueName();
    // no connectivity generator in constructor
    generator = NULL;

    this->values.push_back("src");
    this->values.push_back("dst");
    this->values.push_back("delay");

    // create the file:

    // set the filepath
    // start investigating the library
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    QDir lib_dir = QDir(QDesktopServices::storageLocation(QDesktopServices::DataLocation));
#else
    QDir lib_dir = QDir(QStandardPaths::writableLocation(QStandardPaths::DataLocation));
#endif
    if (!lib_dir.exists()) {
        if (!lib_dir.mkpath(lib_dir.absolutePath())) {
            qDebug() << "error creating library";
        }
    }

    this->file.setFileName(lib_dir.absoluteFilePath(this->filename));

    // open the storage file
    if (!this->file.open(QIODevice::ReadWrite)) {
        QMessageBox msgBox;
        msgBox.setText("Could not open output file for conversion");
        msgBox.exec();
        return;
    }

    QStringList list;
    list = fileName.split("/", QString::SkipEmptyParts);
    list = list.back().split("\\", QString::SkipEmptyParts);

    this->name = list.back();
    this->import_csv(fileName);
}

void csv_connection::setFileName(QString name)
{
    filename = name;
}

QString csv_connection::getFileName()
{
    return filename;
}

void csv_connection::write_node_xml(QXmlStreamWriter &xmlOut)
{
    file.seek(0);
    QDataStream access(&file);

    // ok, check if we have a generator, and if it is up-to-date
    if (this->generator) {
        pythonscript_connection * pyConn = dynamic_cast<pythonscript_connection *> (this->generator);
        CHECK_CAST(pyConn);
        // if we have changes then...
        if (pyConn->changed()) {
            // ... regenerate the connectivity!
            pyConn->regenerateConnections();
        }
    }

    // get a handle to the saved file
    QSettings settings;
    QString filePathString = settings.value("files/currentFileName", "error").toString();

    if (filePathString == "error") {
        qDebug() << "Error getting current project path - THIS SHOULD NEVER HAPPEN!";
        return;
    }

    QDir saveDir(filePathString);

    bool saveBinaryConnections = settings.value("fileOptions/saveBinaryConnections", "error").toBool();

    // write containing tag
    xmlOut.writeStartElement("ConnectionList");

    // if we have more than 30 rows and we want to write to binary then write binary file
    // (less than 30 rows is sufficiently compact that it should go in the XML)
    QString saveFullFileName;
    if (saveBinaryConnections && this->getNumRows() > MIN_CONNS_TO_FORCE_BINARY) {

        saveFullFileName = QDir::toNativeSeparators(settings.value("files/currentFileName").toString() + "/" + this->filename + ".bin");

        // extract the filename without the path...
        QString saveFileName;
        QStringList fileBits = saveFullFileName.split(QDir::toNativeSeparators("/"));
        saveFileName = fileBits.last();

        // add a tag to the binary file
        xmlOut.writeEmptyElement("BinaryFile");
        xmlOut.writeAttribute("file_name", saveFileName);
        xmlOut.writeAttribute("num_connections", QString::number(getNumRows()));
        xmlOut.writeAttribute("explicit_delay_flag", QString::number(float(getNumCols()==3)));
        xmlOut.writeAttribute("packed_data", "true");

        // re-write the data
        if (getNumCols()==3) {
            QVector <conn> conns;
            this->getAllData(conns);

            // write out
            QFile export_file(saveFullFileName);

            if (!export_file.open( QIODevice::WriteOnly)) {
                QMessageBox msgBox;
                msgBox.setText("Error creating exported binary connection file '" + saveFullFileName
                               + "' (Check disk space; permissions)");
                msgBox.exec();
                return;
            }

            QDataStream access(&export_file);
            for (int i = 0; i < conns.size(); ++i) {
                access.writeRawData((char*) &conns[i].src, sizeof(int));
                access.writeRawData((char*) &conns[i].dst, sizeof(int));
                access.writeRawData((char*) &conns[i].metric, sizeof(float));
            }
        }
        if (getNumCols()==2) {
            QVector <conn> conns;
            this->getAllData(conns);

            // write out
            QFile export_file(saveFullFileName);

            if (!export_file.open( QIODevice::WriteOnly)) {
                QMessageBox msgBox;
                msgBox.setText("Error creating exported binary connection file '" + saveFullFileName
                               + "' (Check disk space; permissions)");
                msgBox.exec();
                return;
            }

            QDataStream access(&export_file);
            for (int i = 0; i < conns.size(); ++i) {
                access.writeRawData((char*) &conns[i].src, sizeof(int));
                access.writeRawData((char*) &conns[i].dst, sizeof(int));
            }
        }


    } else {

        // loop through connections writing them out in XML format.
        for (int i=0; i < this->getNumRows(); ++i) {

            xmlOut.writeEmptyElement("Connection");

            qint32 val;
            access >> val;
            xmlOut.writeAttribute("src_neuron", QString::number(float(val)));

            access >> val;
            xmlOut.writeAttribute("dst_neuron", QString::number(float(val)));

            if (this->getNumCols() == 3) {
                float valf;
                access >> valf;
                xmlOut.writeAttribute("delay", QString::number(float(valf)));
            }
        }
    }

    if (this->getNumCols() == 2) {
        writeDelay(xmlOut);
    }

    xmlOut.writeEndElement(); // connectionList
}

/*!
 * \brief csv_connection::write_metadata_xml
 * \param meta
 * \param e
 *
 * Accessor for getting the connectivity generator to write its description into the Project
 * metaData.xml file.
 */
void csv_connection::write_metadata_xml(QDomDocument &meta, QDomNode &e)
{
    // pass this task on to the generator connection
    if (this->generator != NULL) {
        this->generator->write_metadata_xml(meta, e);
    }
}

/*!
 * \brief csv_connection::read_metadata_xml
 * \param e
 *
 * Read the metaData for a connection generator - may be blank if there is not one
 */
void csv_connection::read_metadata_xml(QDomNode &e)
{
    // pass this task on to the generator connection
    if (this->generator != NULL) {
        this->generator->read_metadata_xml(e);
    }
}

void csv_connection::import_parameters_from_xml(QDomNode &e)
{
    QDomNodeList BinaryFileList = e.toElement().elementsByTagName("BinaryFile");

    if (BinaryFileList.count() == 1) {

        // is a binary file so load accordingly

        // set number of connections
        this->setNumRows(BinaryFileList.at(0).toElement().attribute("num_connections").toUInt());
        DBG() << "Conn text = " << BinaryFileList.at(0).toElement().attribute("num_connections");
        DBG() << "Conn value = " << BinaryFileList.at(0).toElement().attribute("num_connections").toUInt();

        // do we have explicit delays
        bool explicit_delay = BinaryFileList.at(0).toElement().attribute("explicit_delay_flag").toInt();
        if (explicit_delay)
            this->setNumCols(3);
        else
            this->setNumCols(2);

        // check what the file type is, since we changed from using QStreamData written binary to
        // packed binary we need to check for the packed_binary flag

        QString isPacked = BinaryFileList.at(0).toElement().attribute("packed_data", "false");

        if (isPacked == "true") {

            // first remove existing file
            this->file.remove();

            // get a handle to the saved file
            QSettings settings;
            QString filePathString = settings.value("files/currentFileName", "error").toString();

            if (filePathString == "error") {
                qDebug() << "Error getting current project path - THIS SHOULD NEVER HAPPEN!";
                return;
            }

            QDir filePath(filePathString);

            // get file name and path
            QString fileName = BinaryFileList.at(0).toElement().attribute("file_name");

            #if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
            QDir lib_dir = QDir(QDesktopServices::storageLocation(QDesktopServices::DataLocation));
            #else
            QDir lib_dir = QDir(QStandardPaths::writableLocation(QStandardPaths::DataLocation));
            #endif
            if (!lib_dir.exists()) {
                if (!lib_dir.mkpath(lib_dir.absolutePath())) {
                    qDebug() << "error creating library";
                }
            }

            // copy the file across to the temporary file
            QFile savedData(filePath.absoluteFilePath(fileName));

            // check that the data file exists!
            if (!savedData.open(QIODevice::ReadOnly)) {
                QSettings settings;
                int num_errs = settings.beginReadArray("errors");
                settings.endArray();
                settings.beginWriteArray("errors");
                    settings.setArrayIndex(num_errs + 1);
                    settings.setValue("errorText",  "Error: Binary file referenced in network not found: " + fileName);
                settings.endArray();
                return;
            }

            // restart the file
            this->file.setFileName(lib_dir.absoluteFilePath(this->filename));

            // open the storage file
            if( !this->file.open( QIODevice::ReadWrite ) ) {
                QMessageBox msgBox;
                msgBox.setText("Could not open temporary file for Explicit Connection");
                msgBox.exec();
                return;
            }

            // now we need to read from the savedData file and put this into a QDataStream...
            this->import_packed_binary(savedData);

        }
        // this is the old way of loading binary data - it is obselete as new projects should not store data this way,
        // however it is left in for compatibility with old projects - these will be converted to the new method once
        // they are re-saved
        else {

            // copy across file and set file name
            // first remove existing file
            this->file.remove();

            // get a handle to the saved file
            QSettings settings;
            QString filePathString = settings.value("files/currentFileName", "error").toString();

            if (filePathString == "error") {
                qDebug() << "Error getting current project path - THIS SHOULD NEVER HAPPEN!";
                return;
            }

            QDir filePath(filePathString);

            // get file name and path
            QString fileName = BinaryFileList.at(0).toElement().attribute("file_name");

            #if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
            QDir lib_dir = QDir(QDesktopServices::storageLocation(QDesktopServices::DataLocation));
            #else
            QDir lib_dir = QDir(QStandardPaths::writableLocation(QStandardPaths::DataLocation));
            #endif
            if (!lib_dir.exists()) {
                if (!lib_dir.mkpath(lib_dir.absolutePath())) {
                    qDebug() << "error creating library";
                }
            }

            // copy the file across to the temporary file
            QFile savedData(filePath.absoluteFilePath(fileName));

            // check that the data file exists!
            if (!savedData.open(QIODevice::ReadOnly)) {
                QSettings settings;
                int num_errs = settings.beginReadArray("errors");
                settings.endArray();
                settings.beginWriteArray("errors");
                    settings.setArrayIndex(num_errs + 1);
                    settings.setValue("errorText",  "Error: Binary file referenced in network not found: " + fileName);
                settings.endArray();
                return;
            }
            savedData.close();

            savedData.copy(lib_dir.absoluteFilePath(this->filename));

            // restart the file
            this->file.setFileName(lib_dir.absoluteFilePath(this->filename));

            // open the storage file
            if( !this->file.open( QIODevice::ReadWrite ) ) {
                QMessageBox msgBox;
                msgBox.setText("Could not open temporary file for Explicit Connection");
                msgBox.exec();
                return;
            }
        }

    }

    if (BinaryFileList.count() != 1) {

        // load connections from xml
        file.seek(0);
        QDataStream access(&file);

        QDomNodeList connInstList = e.toElement().elementsByTagName("Connection");

        this->setNumRows(connInstList.size());

        for (int i=0; i < (int)connInstList.size(); ++i) {

            qint32 val = connInstList.at(i).toElement().attribute("src_neuron").toUInt();
            access << val;

            val = connInstList.at(i).toElement().attribute("dst_neuron").toUInt();
            access << val;

            QString delayStr = connInstList.at(i).toElement().attribute("delay", "noDelay");
            if (delayStr != "noDelay") {
                float val_f = delayStr.toFloat();
                access << val_f;
            } else {
                if (this->values.size()> 2)
                    this->values.removeLast();
            }
        }
    }

    //// LOAD DELAY

    QDomNodeList delayProp = e.toElement().elementsByTagName("Delay");

    QDomNode n = delayProp.item(0);

    if (delayProp.size() == 1) {

        QDomNodeList propVal = n.toElement().elementsByTagName("FixedValue");
        if (propVal.size() == 1) {
            this->delay->currType = FixedValue;
            this->delay->value.resize(1);
            this->delay->value.fill(0);
            this->delay->value[0] = propVal.item(0).toElement().attribute("value").toFloat();
        }
        propVal = n.toElement().elementsByTagName("UniformDistribution");
        if (propVal.size() == 1) {
            this->delay->currType = Statistical;
            this->delay->value.resize(4);
            this->delay->value.fill(0);
            this->delay->value[0] = 1;
            this->delay->value[1] = propVal.item(0).toElement().attribute("minimum").toFloat();
            this->delay->value[2] = propVal.item(0).toElement().attribute("maximum").toFloat();
            this->delay->value[3] = propVal.item(0).toElement().attribute("seed").toFloat();
        }
        propVal = n.toElement().elementsByTagName("NormalDistribution");
        if (propVal.size() == 1) {
            this->delay->currType = Statistical;
            this->delay->value.resize(4);
            this->delay->value.fill(0);
            this->delay->value[0] = 2;
            this->delay->value[1] = propVal.item(0).toElement().attribute("mean").toFloat();
            this->delay->value[2] = propVal.item(0).toElement().attribute("variance").toFloat();
            this->delay->value[3] = propVal.item(0).toElement().attribute("seed").toFloat();
        }

    }


    // flush out the output...
    file.flush();
}

void csv_connection::fetch_headings()
{
}

void csv_connection::import_csv(QString fileName)
{
    this->numRows = 0;

    this->changes.clear();

    //wipe file;
    file.resize(0);

    // open the input csv file for reading
    QFile fileIn(fileName);

    if (!fileIn.open(QIODevice::ReadOnly)) {
        QMessageBox msgBox;
        msgBox.setText("Could not open the selected file");
        msgBox.exec();
        return;
    }

    // if no filename already
    if (this->filename.size() < 1) {
        setUniqueName();
    }

    // use textstream so we can read lines into a QString
    QTextStream stream(&fileIn);

    file.seek(0);

    QDataStream access(&file);

    // test for consistency:
    int numFields = -1;

    // load in the csv line by line
    while (!(stream.atEnd())) {

        // get a line
        QString line = stream.readLine();

        // see if it is a comment:
        if (line[0] == '#') {
            continue;
        }

        // or a blank line
        QString line2 = line;
        line2.replace(" ", "");
        if (line2 == "\n") {
            continue;
        }

        // not a comment - so begin parsing and create an xml element
        this->numRows++;


        QStringList fields = line.split(",");

        if (fields.size() > 3) {
            QMessageBox msgBox;
            msgBox.setText("CSV file has too many columns");
            msgBox.exec();
            return;
        }

        if (fields.size() < 2) {
            QMessageBox msgBox;
            msgBox.setText("CSV file has too few columns");
            msgBox.exec();
            return;
        }

        if (numFields == -1) {
            numFields = fields.size();
        } else if (numFields != fields.size()) {
            qDebug() << "something is wrong!";
            --numRows;
            continue;
        }
        // for each field
        for (int i = 0; i < (int)fields.size(); ++i) {
            if (i < 2) {
                qint32 num = fields[i].toUInt();
                access << num;
            } else {
                float num = fields[i].toFloat();
                access << num;
            }

        }
    }

    values.clear();
    for (int i = 0; i < (int)numFields; ++i) {
        if (i == 0) this->values.push_back("src");
        if (i == 1) this->values.push_back("dst");
        if (i == 2) this->values.push_back("delay");
    }

    // flush out the output...
    this->file.flush();
}


void csv_connection::import_packed_binary(QFile &fileIn)
{
    this->changes.clear();

    //wipe file;
    file.resize(0);

    file.seek(0);

    QDataStream access(&file);

    int count = 0;

    // load in the binary packed data
    while (!(fileIn.atEnd())) {

        // read in the required values:
        // first two int32s
        int srcVal, dstVal;
        float delayVal;

        fileIn.read((char *) &srcVal,sizeof(int));
        fileIn.read((char *) &dstVal,sizeof(int));

        // add the row...
        qint32 num = srcVal;
        access << num;
        num = dstVal;
        access << num;

        // if we have a delay then read that too
        if (this->values.size() == 3) {
            // we have a delay
            fileIn.read((char *) &delayVal,sizeof(float));
            access << delayVal;
        }
        ++count;
    }

    if (count != this->getNumRows()) {
        qDebug() << "Mismatch between the number of rows in the XML and in the binary file";
    }

    // flush out the output...
    this->file.flush();
}

int csv_connection::getNumRows()
{
    return this->numRows;
}

void csv_connection::setNumRows(int num)
{
    this->numRows = num;
}

int csv_connection::getNumCols()
{
    return this->values.size();
}

void csv_connection::setNumCols(int num)
{
    if (num == 2) {
        this->values.clear();
        this->values.push_back("src");
        this->values.push_back("dst");
    }
    if (num == 3) {
        this->values.clear();
        this->values.push_back("src");
        this->values.push_back("dst");
        this->values.push_back("delay");
    }
}


void csv_connection::getAllData(QVector < conn > &conns)
{
    //qDebug() << "ALL CONN DATA FETCHED";

    // rewind file
    file.seek(0);

    QDataStream access(&file);

    conns.resize(this->getNumRows());
    int counter = 0;

    for (int i = 0; i < getNumRows(); ++i) {

        conn newConn;

        qint32 src;
        qint32 dst;

        access >> src;
        access >> dst;

        if (getNumCols() > 2) {
            float temp_delay;
            access >> temp_delay;
            newConn.metric = temp_delay;
        }

        newConn.src = src;
        newConn.dst = dst;

        conns[counter] = (newConn);
        ++counter;
    }
}

float csv_connection::getData(int rowV, int col)
{
    int colVal = getNumCols();
    if (colVal > 2) ++colVal;
    // we multiply by cols +1 as QT seems to do some padding on the stream
    int seekTo = rowV*(colVal)+col;

    if (seekTo*4 > file.size()) {
        return -1;
    }

    file.seek(seekTo*4); // seek to location in bytes

    // get a datastream to serialise the data
    QDataStream access(&file);
    if (col < 2) {
        qint32 data;
        access >> data;
        return float(data);
    }
    else {
        float data;
        access >> data;
        return data;
    }

    return -0.1f;
}

float csv_connection::getData(QModelIndex &index)
{
    int colVal = getNumCols();
    if (colVal > 2) ++colVal;
    // we multiply by cols +1 as QT seems to do some padding on the stream
    int seekTo = index.row()*(colVal)+index.column();

    if (seekTo*4 > file.size()) {
        return -1;
    }

    file.seek(seekTo*4); // seek to location in bytes

    // get a datastream to serialise the data
    QDataStream access(&file);
    if (index.column() < 2) {
        qint32 data;
        access >> data;
        return float(data);
    }
    else {
        float data;
        access >> data;
        return data;
    }

    return -0.1f;
}

void csv_connection::setUniqueName(QString *path)
{
    // generate a unique filename to save the weights under

    // are we writing to the Library or to a save dir?
    QDir lib_dir;
    if (path == NULL) {
        // start investigating the library
    #if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
        lib_dir = QDir(QDesktopServices::storageLocation(QDesktopServices::DataLocation));
    #else
        lib_dir = QDir(QStandardPaths::writableLocation(QStandardPaths::DataLocation));
    #endif
        if (!lib_dir.exists()) {
            if (!lib_dir.mkpath(lib_dir.absolutePath())) {
                qDebug() << "error creating library";
            }
        }
    }
    else
    {
        lib_dir = QDir(*path);
        qDebug() << lib_dir;
    }

    // Get a list of the existing files in the directory. The
    // behaviour here was previously to avoid over-writing any
    // connectionN.bin files. I believe it's preferable to over-write
    // connection files, which makes it easier to track a model in
    // git. To do this, we need to clear out the connection binaries
    // before calling this function the first time.
    QStringList filters;
    filters << "conn*";
    lib_dir.setNameFilters(filters);
    QStringList files = lib_dir.entryList();

    QString baseName = "connection";
    QString uniqueName;
    bool unique = false;
    int index = 0;
    while(!unique) {
        unique = true;
        uniqueName = baseName + QString::number(float(index));
        if (path != NULL) {
            uniqueName += ".bin";
        }
        for (int i = 0; i < (int)files.count(); ++i) {
            // see if the new name is unique
            if (uniqueName == files[i]) {
                unique = false;
                ++index;
            }
        }
    }

    if (path == NULL) {
        this->filename = uniqueName;
    }
    else
    {
        *path = *path + QDir::toNativeSeparators("/") + uniqueName;
        qDebug() << *path;
    }
}

QString csv_connection::getHeader(int section)
{
    return this->values[section];
}

void csv_connection::setData(const QModelIndex & index, float value)
{
    // get a datastream to serialise the data
    file.seek(file.size());

    QDataStream access(&file);

    if (index.row() > this->getNumRows()) {
        // resize
        for (int i = getNumRows()*getNumCols(); i < getNumCols()*index.row(); ++i) {
            qint32 num = 0;
            access << num;
        }
    }

    int colVal = getNumCols();
    if (colVal > 2) ++colVal;
    int seekTo = index.row()*(colVal)+index.column();

    file.seek(seekTo*4); // seek to location in bytes

    if (index.column() < 2) {
        access << (qint32) value;
    } else {
        access << (float) value;
    }
    file.flush();
}

void csv_connection::setData(int row, int col, float value)
{
    // get a datastream to serialise the data
    file.seek(file.size());

    QDataStream access(&file);

    if (row > this->getNumRows()) {
        // resize
        for (int i = getNumRows()*getNumCols(); i < getNumCols()*row; ++i) {
            qint32 num = 0;
            access << num;
        }
    }

    int colVal = getNumCols();
    if (colVal > 2) ++colVal;
    int seekTo = row*(colVal)+col;

    file.seek(seekTo*4); // seek to location in bytes

    if (col < 2) {
        access << (qint32) value;
    } else {
        access << (float) value;
    }
    file.flush();
}

void csv_connection::clearData()
{
    file.remove();
    // open the storage file
    if( !this->file.open( QIODevice::ReadWrite ) ) {
        QMessageBox msgBox;
        msgBox.setText("Could not open output file for conversion");
        msgBox.exec();
        return;
    }
}

void csv_connection::abortChanges()
{
    changes.clear();
}

kernel_connection::kernel_connection()
{
    type = Kernel;
    this->isAList = false;
    selfConnections = false;
    this->kernel_scale = 1.0;
    this->kernel_size = 3;
    for (int i = 0; i < 11; ++i) {
        for (int j = 0; j < 11; ++j) {
            kernel[i][j] = 0.0;
        }
    }
    rotation = 0;
    hasChanged = true;
}

kernel_connection::~kernel_connection()
{
}

QLayout * kernel_connection::drawLayout(rootData*, viewVZLayoutEditHandler*, rootLayout*)
{
    return new QVBoxLayout();
}

bool kernel_connection::changed()
{
    if (src->numNeurons != srcSize || dst->numNeurons != dstSize) {
        return true;
    } else {
        return hasChanged;
    }
}

void kernel_connection::setUnchanged(bool state)
{
    if (state) {
        srcSize = src->numNeurons;
        dstSize = dst->numNeurons;
    }
    hasChanged = !state;
}

void kernel_connection::setKernelSize(int size)
{
    if (kernel_size != size) {
        hasChanged = true;
        kernel_size = size;
    }
}

void kernel_connection::setKernelScale(float scale)
{
    if (kernel_scale != scale) {
        hasChanged = true;
        kernel_scale = scale;
    }
}

void kernel_connection::setKernel(int i, int j, float value)
{
    if (kernel[i][j] != value) {
        hasChanged = true;
        kernel[i][j] = value;
    }
}

void kernel_connection::write_node_xml(QXmlStreamWriter &xmlOut)
{
    QSettings settings;

    if (!this->isAList) {
        xmlOut.writeStartElement("KernelConnection");
        // extra stuff
        xmlOut.writeStartElement("Kernel");
            xmlOut.writeAttribute("scale", QString::number(this->kernel_scale));
            xmlOut.writeAttribute("size", QString::number(float(this->kernel_size)));
            for (int i = 0; i < this->kernel_size; ++i) {
                xmlOut.writeEmptyElement("KernelRow");
                for (int j = 0; j < this->kernel_size; ++j)
                    xmlOut.writeAttribute("col" + QString::number(float(j)), QString::number(this->kernel[i][j]));
            }
        xmlOut.writeEndElement(); // Kernel
        this->writeDelay(xmlOut);
        xmlOut.writeEndElement(); // KernelConnection
    } else {

        xmlOut.writeStartElement("ConnectionList");

        // generate connections:
        QMutex * connGenerationMutex = new QMutex();

        if (changed()) {
            setUnchanged(true);
            connections.clear();
            generate_dialog generate(this, this->src, this->dst, connections, connGenerationMutex, (QWidget *)NULL);
            bool retVal = generate.exec();
            if (!retVal) {
                return;
            }
        }

        if (connections.size() == 0) {
            QMessageBox msgBox;
            msgBox.setText("Error: no connections generated for Kernel Connection");
            msgBox.exec();
            return;
        }

        delete connGenerationMutex;

        // load path
        bool saveBinaryConnections = settings.value("fileOptions/saveBinaryConnections", "error").toBool();

        if (!saveBinaryConnections || connections.size() < MIN_CONNS_TO_FORCE_BINARY) {

            // loop through connections writing them out
            for (int i=0; i < connections.size(); ++i) {

                xmlOut.writeEmptyElement("Connection");

                xmlOut.writeAttribute("src_neuron", QString::number(float(connections[i].src)));
                xmlOut.writeAttribute("dst_neuron", QString::number(float(connections[i].dst)));
            }

        } else {

            QUuid uuid = QUuid::createUuid();
            QString export_filename = uuid.toString();
            export_filename.chop(1);
            export_filename[0] = 'C';
            export_filename += ".bin"; // need to generate a unique filename - preferably a descriptive one... but not for now!
            QString saveFileName = QDir::toNativeSeparators(settings.value("current_model_path").toString() + "/" + export_filename);

            // add a tag to the binary file
            xmlOut.writeEmptyElement("BinaryFile");
            xmlOut.writeAttribute("file_name", export_filename);
            xmlOut.writeAttribute("num_connections", QString::number(float(connections.size())));
            xmlOut.writeAttribute("explicit_delay_flag", QString::number(float(0)));

            // re-write the data

            // write out
            QFile export_file(saveFileName);
            if (!export_file.open( QIODevice::WriteOnly)) {
                QMessageBox msgBox;
                msgBox.setText("Error creating binary connection file '" + saveFileName
                               + "' (Check disk space; permissions)");
                msgBox.exec();
                return;
            }

            QDataStream access(&export_file);
            for (int i = 0; i < connections.size(); ++i) {
                access.writeRawData((char*) &connections[i].src, sizeof(int));
                access.writeRawData((char*) &connections[i].dst, sizeof(int));
            }
        }

        this->writeDelay(xmlOut);

        xmlOut.writeEndElement(); // ConnectionList
    }
}

void kernel_connection::import_parameters_from_xml(QDomNode &e)
{
    QDomNodeList kernelNode = e.toElement().elementsByTagName("Kernel");
    if (kernelNode.size() == 1) {
        QDomNode n = kernelNode.item(0);
        this->kernel_scale = n.toElement().attribute("scale").toFloat();
        this->kernel_size = n.toElement().attribute("size").toInt();
        QDomNodeList rows = n.toElement().elementsByTagName("KernelRow");
        for (int i = 0; i < rows.size(); ++i) {
            for (int j = 0; j < this->kernel_size; ++j)
                kernel[i][j] = rows.item(i).toElement().attribute("col" + QString::number(float(j))).toFloat();
        }
    }

    QDomNodeList delayProp = e.toElement().elementsByTagName("Delay");
    if (delayProp.size() == 1) {

        QDomNode n = delayProp.item(0);

        QDomNodeList propVal = n.toElement().elementsByTagName("FixedValue");
        if (propVal.size() == 1) {
            this->delay->currType = FixedValue;
            this->delay->value.resize(1);
            this->delay->value.fill(0);
            this->delay->value[0] = propVal.item(0).toElement().attribute("value").toFloat();
        }
        propVal = n.toElement().elementsByTagName("UniformDistribution");
        if (propVal.size() == 1) {
            this->delay->currType = Statistical;
            this->delay->value.resize(4);
            this->delay->value.fill(0);
            this->delay->value[0] = 1;
            this->delay->value[1] = propVal.item(0).toElement().attribute("minimum").toFloat();
            this->delay->value[2] = propVal.item(0).toElement().attribute("maximum").toFloat();
            this->delay->value[3] = propVal.item(0).toElement().attribute("seed").toFloat();
        }
        propVal = n.toElement().elementsByTagName("NormalDistribution");
        if (propVal.size() == 1) {
            this->delay->currType = Statistical;
            this->delay->value.resize(4);
            this->delay->value.fill(0);
            this->delay->value[0] = 2;
            this->delay->value[1] = propVal.item(0).toElement().attribute("mean").toFloat();
            this->delay->value[2] = propVal.item(0).toElement().attribute("variance").toFloat();
            this->delay->value[3] = propVal.item(0).toElement().attribute("seed").toFloat();
        }
    }
}

void kernel_connection::generate_connections()
{
    conns->clear();

    QString errorLog;
    src->layoutType->generateLayout(src->numNeurons,&src->layoutType->locations,errorLog);
    if (!errorLog.isEmpty()) {
        return;
    }
    dst->layoutType->generateLayout(dst->numNeurons,&dst->layoutType->locations,errorLog);
    if (!errorLog.isEmpty()) {
        return;
    }

    float total_ops = src->layoutType->locations.size();
    int scale_val = round(100000000.0/(src->layoutType->locations.size()*dst->layoutType->locations.size()));

    int oldprogress = 0;

    for (int i = 0; i < src->layoutType->locations.size(); ++i) {
        //#pragma omp parallel for
        for (int j = 0; j < (int) dst->layoutType->locations.size(); ++j) {

            // CALCULATE (kernels ignore z component for now!)
            float xRaw = dst->layoutType->locations[j].x - src->layoutType->locations[i].x;
            float yRaw = dst->layoutType->locations[j].y - src->layoutType->locations[i].y;

            // rotate:
            float x;
            float y;
            if (rotation != 0) {
                x = cos(rotation)*xRaw - sin(rotation)*yRaw;
                y = sin(rotation)*xRaw + cos(rotation)*yRaw;
            } else {
                x = xRaw;
                y = yRaw;
            }

            // if we are outside the kernel
            if (fabs(x) > floor(kernel_size/2.0) * kernel_scale || \
                    fabs(y) > floor(kernel_size/2.0) * kernel_scale)
                continue;

            // otherwise find the right kernel box
            int boxX = floor(x / kernel_scale + 0.5) + floor(kernel_size/2.0);
            int boxY = floor(y / kernel_scale + 0.5) + floor(kernel_size/2.0);

            // add connection based on kernel
            if (float(rand())/float(RAND_MAX) < kernel[boxX][boxY]) {
                mutex->lock();
                conn newConn;
                newConn.src = i;
                newConn.dst = j;
                conns->push_back(newConn);
                mutex->unlock();
            }
        }
        if (round(float(i)/total_ops * 100.0) > oldprogress) {
            emit progress((int) round(float(i)/total_ops * 100.0));
            oldprogress = round(float(i)/total_ops * 100.0)+scale_val;
        }
    }
    this->moveToThread(QApplication::instance()->thread());
    emit connectionsDone();
}

bool kernel_connection::isList()
{
    return this->isAList;
}

pythonscript_connection::pythonscript_connection(QSharedPointer <population> src, QSharedPointer <population> dst, csv_connection *  conn_targ)
{
    type = Python;
    this->isAList = false;
    selfConnections = false;
    rotation = 0;
    hasChanged = true;
    this->scriptValidates = false;
    this->hasWeight = false;
    this->hasDelay = false;
    this->src = src;
    this->dst = dst;
    this->connection_target = conn_targ;
}

pythonscript_connection::~pythonscript_connection()
{
}

int pythonscript_connection::getIndex()
{
    QSettings settings;
    settings.beginGroup("pythonscripts");
    QStringList scripts = settings.childKeys();
    settings.endGroup();

    // sanity
    int index = scripts.indexOf(this->scriptName);
    if (index == -1) {
        qDebug() << "Error with script " << this->scriptName;
    }

    // use Python as the base index and increment by the script number
    return (int) this->type + index;
}

QLayout * pythonscript_connection::drawLayout(rootData * data, viewVZLayoutEditHandler * viewVZhandler, rootLayout * rootLay)
{
    // refetch the script text
    QSettings settings;
    // enter group of scripts
    settings.beginGroup("pythonscripts");
    // find the script by name
    QString script = settings.value(this->scriptName, "has been deleted").toString();

    if (script == "has been deleted") {
        // add it back in - just to be annoying
        settings.setValue(this->scriptName, this->scriptText);
    } else {
        this->scriptText = script;
    }
    settings.endGroup();

    // most draw stuff is the same if we are a generator or not...
    QVBoxLayout * vlay = new QVBoxLayout;

    // check if we are a generator...
    if (this->connection_target == NULL) {
        // NOT a generator
        QLabel * label = new QLabel("Oops");
        vlay->addWidget(label);
    } else {
        // ARE a generator
        // we have a python script to generate the connections

        QHBoxLayout * buttons = new QHBoxLayout;
        vlay->addLayout(buttons);
        if (viewVZhandler) {
            // add the delete signal
            connect(viewVZhandler, SIGNAL(deleteProperties()), buttons, SLOT(deleteLater()));
        }
        if (rootLay) {
            // add the delete signal
            connect(rootLay, SIGNAL(deleteProperties()), buttons, SLOT(deleteLater()));
        }

        // add a 'Generate connectivity' button
        QPushButton * gen = new QPushButton("Generate");
        if (!this->changed()) {
            gen->setEnabled(false);
        }
        // should we enable the button?
        //gen->setEnabled(this->scriptValidates);
        // connect up to the Connection object
        connect(gen, SIGNAL(clicked(bool)), this, SLOT(setUnchanged(bool)));
        connect(gen, SIGNAL(clicked()), this, SLOT(regenerateConnections()));
        connect(gen, SIGNAL(clicked()), data, SLOT(reDrawAll()));
        connect(this, SIGNAL(setGenEnabled(bool)), gen, SLOT(setEnabled(bool)));
        if (viewVZhandler) {
            // redraw to update glview
            connect(gen, SIGNAL(clicked()), data->main->viewVZ.OpenGLWidget, SLOT(parsChangedProjection()));
            // add the delete signal
            connect(viewVZhandler, SIGNAL(deleteProperties()), gen, SLOT(deleteLater()));
        }
        if (rootLay) {
            // add the delete signal
            connect(rootLay, SIGNAL(deleteProperties()), gen, SLOT(deleteLater()));
        }
        // add a connection so we can disable the button if the script changes
        // add to the HBoxLayout
        buttons->addWidget(gen);

        // add a button to investigate the connections
        QPushButton *view = new QPushButton("View");
        view->setMaximumWidth(70);
        view->setToolTip("view connectivity");
        view->setProperty("ptr", qVariantFromValue((void *) this->connection_target));

        if (viewVZhandler) {
            // add the delete signal
            connect(viewVZhandler, SIGNAL(deleteProperties()), view, SLOT(deleteLater()));
        }
        if (rootLay) {
            // add the delete signal
            connect(rootLay, SIGNAL(deleteProperties()), view, SLOT(deleteLater()));
        }


        buttons->addWidget(view);

        // add connection:
        connect(view, SIGNAL(clicked()), data, SLOT(editConnections()));

        buttons->addStretch();

        // if we have a weight produced add a combobox (which is safe as it never deletes itself)
        if (this->hasWeight && !this->getPropList().isEmpty()) {
            QLabel * wLabel = new QLabel("Weight:");
            if (viewVZhandler) {
                // add the delete signal
                connect(viewVZhandler, SIGNAL(deleteProperties()), wLabel, SLOT(deleteLater()));
            }
            if (rootLay) {
                // add the delete signal
                connect(rootLay, SIGNAL(deleteProperties()), wLabel, SLOT(deleteLater()));
            }
            buttons->addWidget(wLabel);
            QComboBox * weightTarget = new QComboBox;
            weightTarget->setFocusPolicy(Qt::StrongFocus);
            weightTarget->installEventFilter(new FilterOutUndoRedoEvents);
            // find the WeightUpdate for this Connection
            QStringList list = this->getPropList();
            list.push_front("-no weight set-");
            // add the props to the combobox
            weightTarget->addItems(list);
            // now set the prop to the currently selected one
            for (int i = 0; i < list.size();++i) {
                if (this->weightProp == list[i]) {
                    // set index
                    weightTarget->setCurrentIndex(i);
                }
            }
            if (viewVZhandler) {
                // add the delete signal
                connect(viewVZhandler, SIGNAL(deleteProperties()), weightTarget, SLOT(deleteLater()));
            }
            if (rootLay) {
                // add the delete signal
                connect(rootLay, SIGNAL(deleteProperties()), weightTarget, SLOT(deleteLater()));
            }
            // connect up so we can change the par
            weightTarget->setProperty("action", "changePythonScriptProp");
            weightTarget->setProperty("ptr", qVariantFromValue((void *) this));
            weightTarget->setToolTip("Select a property to be assigned the script weight values");
            // add the change signal
            connect(weightTarget, SIGNAL(currentIndexChanged(int)), data, SLOT(updatePar()));
            // enable the generate button
            connect(weightTarget, SIGNAL(currentIndexChanged(int)), this, SLOT(enableGen(int)));
            // add to the HBoxLayout
            buttons->addWidget(weightTarget);

        }


        // clean up the interface by adding an expanding spacer to the end of the line
        buttons->addStretch();

        // create the grid layout for the script parameters
        QGridLayout * grid = new QGridLayout;
        vlay->addLayout(grid);
        // add the delete signal
        if (viewVZhandler) {
            // add the delete signal
            connect(viewVZhandler, SIGNAL(deleteProperties()), grid, SLOT(deleteLater()));
        }
        if (rootLay) {
            // add the delete signal
            connect(rootLay, SIGNAL(deleteProperties()), grid, SLOT(deleteLater()));
        }

        int maxCols = 1;

        // first add the items that have locations
        for (int i = 0; i < this->parNames.size(); ++i) {
            // if we have a position
            if (this->parPos[i].x() != -1) {
                if (grid->itemAtPosition(this->parPos[i].x(), this->parPos[i].y())) {
                    // grid slot is taken! Get rid of Pos
                    this->parPos[i] = QPoint(-1,-1);
                } else {
                    // grid slot is free, so add the par
                    QHBoxLayout * parBox = new QHBoxLayout;
                    // add the delete signal
                    if (viewVZhandler) {
                        // add the delete signal
                        connect(viewVZhandler, SIGNAL(deleteProperties()), parBox, SLOT(deleteLater()));
                    }
                    if (rootLay) {
                        // add the delete signal
                        connect(rootLay, SIGNAL(deleteProperties()), parBox, SLOT(deleteLater()));
                    }
                    QLabel * name = new QLabel;
                    name->setText(this->parNames[i] + QString("="));
                    if (viewVZhandler) {
                        // add the delete signal
                        connect(viewVZhandler, SIGNAL(deleteProperties()), name, SLOT(deleteLater()));
                    }
                    if (rootLay) {
                        // add the delete signal
                        connect(rootLay, SIGNAL(deleteProperties()), name, SLOT(deleteLater()));
                    }
                    parBox->addWidget(name);
                    QDoubleSpinBox * val = new QDoubleSpinBox;
                    val->setMaximum(100000000.0);
                    val->setMinimum(-100000000.0);
                    val->setDecimals(5);
                    val->setMinimumWidth(120);
                    val->setValue(this->parValues[i]);
                    val->setProperty("par_name", this->parNames[i]);
                    val->setProperty("action", "changePythonScriptPar");
                    val->setProperty("ptr", qVariantFromValue((void *) this));
                    val->setFocusPolicy(Qt::StrongFocus);
                    val->installEventFilter(new FilterOutUndoRedoEvents);
                    if (viewVZhandler) {
                        // add the delete signal
                        connect(viewVZhandler, SIGNAL(deleteProperties()), val, SLOT(deleteLater()));
                    }
                    if (rootLay) {
                        // add the delete signal
                        connect(rootLay, SIGNAL(deleteProperties()), val, SLOT(deleteLater()));
                    }
                    // add the change signal
                    connect(val, SIGNAL(valueChanged(double)), data, SLOT(updatePar()));
                    // enable the generator button
                    connect(val, SIGNAL(valueChanged(double)), this, SLOT(enableGen(double)));
                    parBox->addWidget(val);
                    parBox->addStretch();
                    // now add this to the grid
                    grid->addLayout(parBox,this->parPos[i].x(), this->parPos[i].y(),1,1);
                    // get our grid width
                    if (this->parPos[i].y()>maxCols) {
                        maxCols = this->parPos[i].y();
                    }
                }
            }
        }
        // then add the items that do not have locations
        for (int i = 0; i < this->parNames.size(); ++i) {
            // if we have a position
            if (this->parPos[i].x() == -1) {
                bool inserted = false;
                int row = 0;
                // while we have not placed the par
                while (!inserted) {
                    // for each column
                    for (int col = 0; col < maxCols; ++col) {
                        if (!grid->itemAtPosition(row, col)) {
                            inserted = true;
                            // grid slot is free, so add the par
                            QHBoxLayout * parBox = new QHBoxLayout;
                            if (viewVZhandler) {
                                // add the delete signal
                                connect(viewVZhandler, SIGNAL(deleteProperties()), parBox, SLOT(deleteLater()));
                            }
                            if (rootLay) {
                                // add the delete signal
                                connect(rootLay, SIGNAL(deleteProperties()), parBox, SLOT(deleteLater()));
                            }
                            QLabel * name = new QLabel;
                            name->setText(this->parNames[i] + QString("="));
                            if (viewVZhandler) {
                                // add the delete signal
                                connect(viewVZhandler, SIGNAL(deleteProperties()), name, SLOT(deleteLater()));
                            }
                            if (rootLay) {
                                // add the delete signal
                                connect(rootLay, SIGNAL(deleteProperties()), name, SLOT(deleteLater()));
                            }
                            parBox->addWidget(name);
                            QDoubleSpinBox * val = new QDoubleSpinBox;
                            val->setMaximum(100000000.0);
                            val->setMinimum(-100000000.0);
                            val->setDecimals(5);
                            val->setMinimumWidth(120);
                            val->setValue(this->parValues[i]);
                            val->setProperty("par_name", this->parNames[i]);
                            val->setProperty("action", "changePythonScriptPar");
                            val->setProperty("ptr", qVariantFromValue((void *) this));
                            val->setFocusPolicy(Qt::StrongFocus);
                            val->installEventFilter(new FilterOutUndoRedoEvents);
                            if (viewVZhandler) {
                                // add the delete signal
                                connect(viewVZhandler, SIGNAL(deleteProperties()), val, SLOT(deleteLater()));
                            }
                            if (rootLay) {
                                // add the delete signal
                                connect(rootLay, SIGNAL(deleteProperties()), val, SLOT(deleteLater()));
                            }
                            // add the change signal
                            connect(val, SIGNAL(valueChanged(double)), data, SLOT(updatePar()));
                            // enable the generator button
                            connect(val, SIGNAL(valueChanged(double)), this, SLOT(enableGen(double)));
                            parBox->addWidget(val);
                            parBox->addStretch();
                            // now add this to the grid
                            grid->addLayout(parBox,this->parPos[i].x(), this->parPos[i].y(),1,1);
                        }
                    }
                    // increment the row
                    ++row;
                }
            }
        }
    }

    return vlay;
}

void pythonscript_connection::enableGen(double)
{
    emit setGenEnabled(true);
}

void pythonscript_connection::enableGen(int)
{
    emit setGenEnabled(true);
}

bool pythonscript_connection::changed()
{
    // check all pars
    bool par_changed = false;
    for (int i = 0; i <this->lastGeneratedParValues.size(); ++i) {
        if (this->lastGeneratedParValues[i] != this->parValues[i]) {
            par_changed = true;
        }
    }

    if (this->weightProp != this->lastGeneratedWeightProp) {
        par_changed = true;
    }

    if (this->scriptText != this->lastGeneratedScriptText) {
        par_changed = true;
    }

    if (src->numNeurons != srcSize || dst->numNeurons != dstSize || par_changed) {
        return true;
    } else {
        return hasChanged;
    }
}

void pythonscript_connection::setUnchanged(bool state)
{
    if (state) {
        srcSize = src->numNeurons;
        dstSize = dst->numNeurons;
        for (int i = 0; i <this->lastGeneratedParValues.size(); ++i) {
            this->lastGeneratedParValues[i] = this->parValues[i];
        }
        this->lastGeneratedWeightProp = this->weightProp;
        this->lastGeneratedScriptText = scriptText;
    }
    hasChanged = !state;
}

void pythonscript_connection::configureFromScript(QString script)
{
    // add the script to the class variable
    this->scriptText = script;
    // store old pars
    QStringList oldNames = this->parNames;
    QVector <double> oldValues = this->parValues;
    // clear previous pars
    this->parNames.clear();
    this->parValues.clear();
    this->parPos.clear();
    this->hasWeight = false;
    this->hasDelay = false;
    // parse the script for parameter lines
    QStringList lines = script.split("\n");
    for (int i = 0; i < lines.size(); ++i) {
        // find a par tag
        if (lines[i].contains("#PARNAME=")) {
            QStringList bits = lines[i].split("#PARNAME=");
            // check if the end bit has location info
            if (bits.last().contains("#LOC=")) {
                // we have position info - so extract it
                QStringList posbits = bits.last().split("#LOC=");
                // pos are separated by commas, so split again
                QStringList posvals = posbits.last().split(",");
                if (posvals.size() == 2) {
                    // correct formatting, add the position to the list
                    this->parNames.push_back(posbits.first().replace(" ", ""));
                    this->parValues.push_back(0);
                    this->parPos.push_back(QPoint(posvals[0].toInt(), posvals[1].toInt()));
                }
            } else {
                // add with just the name specified
                this->parNames.push_back(bits.last().replace(" ", ""));
                this->parValues.push_back(0);
                this->parPos.push_back(QPoint(-1,-1)); // -1,-1 indicates no fixed position - the par will be placed where it fits
            }
        }
        if (lines[i].contains("#HASDELAY")) {
            this->hasDelay = true;
        }
        if (lines[i].contains("#HASWEIGHT")) {
            this->hasWeight = true;
        }
    }
    // clear the last par vals
    this->lastGeneratedParValues.clear();
    this->lastGeneratedParValues.resize(this->parValues.size());
    this->lastGeneratedParValues.fill(0);
}

void pythonscript_connection::regenerateConnections()
{
    // refetch the script text
    QSettings settings;
    // enter group of scripts
    settings.beginGroup("pythonscripts");
    // find the script by name
    QString script = settings.value(this->scriptName, "has been deleted").toString();

    if (script == "has been deleted") {
        // add it back in - just to be annoying
        settings.setValue(this->scriptName, this->scriptText);
    } else {
        this->scriptText = script;
    }
    settings.endGroup();

    // test if required
    if (!this->changed()) {
        return;
    }

    // generate connections:
    QMutex * connGenerationMutex = new QMutex();

    this->connections.clear();
    generate_dialog generate(this, this->src, this->dst, this->connections, connGenerationMutex, (QWidget *)NULL);
    bool retVal = generate.exec();
    if (!retVal) {
        return;
    }

    if (connections.size() == 0) {
        if (this->connection_target) {
            if (this->connection_target->getNumRows() == 0) {
                QMessageBox msgBox;
                msgBox.setText("Error: no connections generated for Python Script Connection");
                msgBox.exec();
                delete connGenerationMutex;
                return;
            }
        }
    }

    delete connGenerationMutex;
}

void pythonscript_connection::write_node_xml(QXmlStreamWriter &)
{
    // this should never be called
}

void pythonscript_connection::write_metadata_xml(QDomDocument &meta, QDomNode &e)
{
    // write out the settings for this generator

    // write out the script and parameters
    QDomElement script = meta.createElement( "Script" );
    e.appendChild(script);

    // script text
    script.setAttribute("text", this->scriptText);

    script.setAttribute("name", this->scriptName);

    // parameter values for the script
    for (int i = 0; i < this->parNames.size(); ++i) {
        script.setAttribute(this->parNames[i], QString::number(this->parValues[i]));
    }

    // write out configuration information
    QDomElement config = meta.createElement( "Config" );
    e.appendChild(config);

    if (!this->weightProp.isEmpty()) {
        config.setAttribute("weightProperty", this->weightProp);
    }
}

void pythonscript_connection::read_metadata_xml(QDomNode &e)
{
    // read in the settings for this generator
    QDomNode node = e.firstChild();

    while(!node.isNull()) {

        // read in the script
        if (node.toElement().tagName() == "Script") {

            // fetch the script name
            this->scriptName = node.toElement().attribute("name", "");

            // load in the python script
            this->scriptText = node.toElement().attribute("text", "");

            // find the parameters from the script
            if (this->scriptText.size() > 0) {
                this->configureFromScript(this->scriptText);
            }

            // load the parameters from the metadata
            for (int i = 0; i < this->parNames.size(); ++i) {
                qDebug() << "ParName = " << this->parNames[i];
                this->parValues[i] = node.toElement().attribute(this->parNames[i], "0").toDouble();
            }

        }

        // read the config metadata
        if (node.toElement().tagName() == "Config") {
            // get the name of the weight property associated with the script
            this->weightProp = node.toElement().attribute("weightProperty", "");
        }

        node = node.nextSibling();
    }

    // now try to match the script to a script in the library - if you can't then add the script
    QSettings settings;
    // enter group of scripts
    settings.beginGroup("pythonscripts");
    // fetch a list of scripts
    QStringList scripts = settings.childKeys();
    // first try to match by name and text
    // if this does not evaluate we have a match and can leave it at that
    if (settings.value(this->scriptName,"not found") != this->scriptText) {
        // no match found!
        // test the existing scripts for a match
        bool matchFound = false;
        for (int i = 0; i < scripts.size(); ++i) {
            if (settings.value(scripts[i],"not found") == this->scriptText) {
                this->scriptName = scripts[i];
                matchFound = true;
            }
        }
        // script just isn't in the library...
        if (!matchFound) {
            QString extra = "";
            int i = 1;
            if (this->scriptName == "") {
                this->scriptName = "loaded connection";
                extra = " 1";
                i = 2;
            }

            // make sure the name is unique
            while (scripts.contains(this->scriptName+extra)) {
                extra = QString(" ") + QString::number(i);
            }
            // add the script to the library
            settings.setValue(this->scriptName+extra, this->scriptText);
            // save the modified scriptname
            this->scriptName = this->scriptName+extra;
        }
    }
    // exit the scripts group
    settings.endGroup();
}

ParameterData * pythonscript_connection::getPropPointer()
{
    for (int i = 0; i < this->src->projections.size(); ++i) {
        QSharedPointer <projection> proj = this->src->projections[i];
        for (int j = 0; j < proj->synapses.size(); ++j) {
            QSharedPointer <synapse> syn = proj->synapses[j];
            // if we have found the connection
            bool isConn = false;
            if (syn->connectionType == this) {
                isConn = true;
            }
            // if we are the generator of the connection
            if (syn->connectionType->type == CSV) {
                csv_connection * csvConn = dynamic_cast<csv_connection *> (syn->connectionType);
                CHECK_CAST(csvConn);
                if (csvConn->generator == this) {
                    isConn = true;
                }
            }
            if (isConn) {
                // now we know which weight update we have to look at
                for (int k = 0; k < syn->weightUpdateType->ParameterList.size(); ++k) {
                    if (syn->weightUpdateType->ParameterList[k]->name == this->weightProp) {
                        // found the weight
                        return syn->weightUpdateType->ParameterList[k];
                    }
                }
                for (int k = 0; k < syn->weightUpdateType->StateVariableList.size(); ++k) {
                    if (syn->weightUpdateType->StateVariableList[k]->name == this->weightProp) {
                        // found the weight
                        return syn->weightUpdateType->StateVariableList[k];
                    }
                }
            }
        }
    }
    // not found
    return NULL;
}

QStringList pythonscript_connection::getPropList()
{
    QStringList list;
    for (int i = 0; i < this->src->projections.size(); ++i) {
        QSharedPointer <projection> proj = this->src->projections[i];
        for (int j = 0; j < proj->synapses.size(); ++j) {
            QSharedPointer <synapse> syn = proj->synapses[j];
            // if we have found the connection
            bool isConn = false;
            if (syn->connectionType == this) {
                isConn = true;
            }
            // if we are the generator of the connection
            if (syn->connectionType->type == CSV) {
                csv_connection * csvConn = dynamic_cast<csv_connection *> (syn->connectionType);
                CHECK_CAST(csvConn);
                if (csvConn->generator == this) {
                    isConn = true;
                }
            }
            if (isConn) {
                // now we know which weight update we have to look at
                for (int k = 0; k < syn->weightUpdateType->ParameterList.size(); ++k) {
                    // found the weight
                    list.push_back(syn->weightUpdateType->ParameterList[k]->name);
                }
                for (int k = 0; k < syn->weightUpdateType->StateVariableList.size(); ++k) {
                    // found the weight
                    list.push_back(syn->weightUpdateType->StateVariableList[k]->name);
                }
            }
        }
    }
    // not found
    return list;
}

void pythonscript_connection::import_parameters_from_xml(QDomNode &)
{
    // this should never be called
}

/*!
 * \brief vectorToList
 * \param vect
 * \return
 * A simple function to take a QVector and put it into a Python List
 */
PyObject * vectorToList(QVector <float> * vect)
{
    PyObject * vectList = PyList_New(vect->size());
    for (int i = 0; i < vect->size(); ++i) {
        PyList_SetItem(vectList, i, PyFloat_FromDouble((double) (*vect)[i]));
    }
    return vectList;
}

/*!
 * \brief vectorLocToList
 * \param vect
 * \return
 * A simple function to take a vector of locations and pack them into a Python list of Python tuples
 * each containing the three values (x,y,z)
 */
PyObject * vectorLocToList(QVector <loc> * vect)
{
    // create the new PyList
    PyObject * vectList = PyList_New(vect->size());
    for (int i = 0; i < vect->size(); ++i) {
        // create a tuple from the three vector values then
        // add the tuple to the list
        PyList_SetItem(vectList, i, PyTuple_Pack(3,PyFloat_FromDouble((double) (*vect)[i].x),
                                                 PyFloat_FromDouble((double) (*vect)[i].y),
                                                 PyFloat_FromDouble((double) (*vect)[i].z)));
    }
    return vectList;
}

/*!
 * \brief listToVector
 * \param list
 * \return
 * A simple function to take  Python list and extract it into a QVector
 */
QVector <float> listToVector(PyObject * list)
{
    QVector <float> vect;
    if (PyList_Size(list) < 1) {
        vect.push_back(-234.56);
        return vect;
    }
    vect.resize(PyList_Size(list));
    for (int i = 0; i < PyList_Size(list); ++i) {
        vect[i] = PyFloat_AsDouble(PyList_GetItem(list, i));
    }
    return vect;
}

struct outputUnPackaged
{
    QVector <conn> connections;
    QVector <double> weights;
};

/*!
 * \brief listToVector
 * \param list
 * \return
 * A simple function to take the output of a connection function and unpack it
 */
outputUnPackaged extractOutput(PyObject * output, bool hasDelay, bool hasWeight)
{
    // create the structure to hold the unpacked output
    outputUnPackaged outUnPacked;
    if (PyList_Size(output) < 1) {
        outUnPacked.weights.push_back(-234.56);
        return outUnPacked;
    }
    outUnPacked.connections.resize(PyList_Size(output));
    if (outUnPacked.connections.size()>0) {
        outUnPacked.connections[0].metric = NO_DELAY;
    }
    outUnPacked.weights.resize(PyList_Size(output));
    for (int i = 0; i < PyList_Size(output); ++i) {
        // get the element
        PyObject * element = PyList_GetItem(output,i);
        // check it is a tuple
        if (PyTuple_Check(element)) {
            // if the tuple has enough items for a src index and dst index
            if (PyTuple_Size(element) > 1) {
                outUnPacked.connections[i].src = PyInt_AsLong(PyTuple_GetItem(element,0));
                outUnPacked.connections[i].dst = PyInt_AsLong(PyTuple_GetItem(element,1));
            }
            // if we have a delay as well
            if (PyTuple_Size(element) > 2 && hasDelay) {
                outUnPacked.connections[i].metric = PyFloat_AsDouble(PyTuple_GetItem(element,2));
            }
            // if we have a weight as well
            if (PyTuple_Size(element) > 3 && hasWeight) {
                outUnPacked.weights[i] = PyFloat_AsDouble(PyTuple_GetItem(element,3));
            } else {
                outUnPacked.weights.clear();
            }
        }
    }
    return outUnPacked;
}

/*!
 * \brief createPyFunc
 * \param pymod
 * \return
 * A simple function to take a string and make it into a Python function which can then be called
 */
PyObject * createPyFunc(PyObject * pymod, QString text, QString &errs)
{
    // get the default dict, so we have access to the built in modules
    PyObject * main = PyImport_AddModule("__main__");
    PyObject *pGlobal = PyModule_GetDict(main);

    PyModule_AddStringConstant(pymod, "__file__", "");

    //Get the dictionary object from my module so I can pass this to PyRun_String
    PyObject * pLocal = PyModule_GetDict(pymod);

    //Define my function in the newly created module
    PyObject * pValue = PyRun_String((char *) text.toStdString().c_str(), Py_file_input, pGlobal, pLocal);
    if (!pValue) {
        PyObject * errtype, * errval, * errtrace;
        PyErr_Fetch(&(errtype), &(errval), &(errtrace));

        errs.append("ERROR ");

        if (errtype) {
            errs.append(PyString_AsString(errtype) + QString(". "));
        }
        if (errval) {
            errs.append(PyString_AsString(errval) + QString(". "));
        }
        if (errtrace) {
            PyTracebackObject * errtraceObj = (PyTracebackObject *) errtrace;
            while (errtraceObj->tb_next) {
                errtraceObj = errtraceObj->tb_next;
            }
            errs.append("Line no: " + QString::number(errtraceObj->tb_lineno));
        }
        return NULL;
    }
    Py_DECREF(pValue);

    //Get a pointer to the function I just defined
    return PyObject_GetAttrString(pymod, "connectionFunc");
}

/*!
 * \brief pythonscript_connection::generate_connections
 * function called to generate the connection into an explicit list -
 * used to draw the connections in the 3D view or export for simulation
 */
void pythonscript_connection::generate_connections()
{
    conns->clear();

    this->pythonErrors.clear();

    // regenerate src and dst locations
    QString errorLog;
    src->layoutType->generateLayout(src->numNeurons,&src->layoutType->locations,errorLog);
    if (!errorLog.isEmpty()) {
        qDebug() << "no src locs";
        return;
    }
    dst->layoutType->generateLayout(dst->numNeurons,&dst->layoutType->locations,errorLog);
    if (!errorLog.isEmpty()) {
        qDebug() << "no dst locs";
        return;
    }

    // a tuple to hold the arguments to the Python Script - size of the scripts pars + the src and dst locations
    PyObject * argsPy = PyTuple_New(this->parNames.size()+2/* 2 for the src and dst locations*/);

    // convert the locations into Python Objects:
    PyObject * srcPy = vectorLocToList(&src->layoutType->locations);
    PyObject * dstPy = vectorLocToList(&dst->layoutType->locations);

    // add them to the tuple
    PyTuple_SetItem(argsPy,0,srcPy);
    PyTuple_SetItem(argsPy,1,dstPy);

    // convert the parameters into Python Objects and add them to the tuple
    for (int i = 0; i < this->parNames.size(); ++i) {
        PyTuple_SetItem(argsPy,i+2,PyFloat_FromDouble(parValues[i]));
    }

    // check the tuple is sound
    if (!argsPy) {
        qDebug() << "Bad args tuple";
        Py_XDECREF(argsPy);
        Py_XDECREF(srcPy);
        Py_XDECREF(dstPy);
        return;
    }

    //Create a new module object
    PyObject *pymod = PyModule_New("mymod");

    // add the function to Python, and get a PyObject for it
    PyObject * pyFunc = createPyFunc(pymod, this->scriptText, this->pythonErrors);

    // check that function creation worked
    if (!pyFunc) {
        if (pythonErrors.isEmpty()) {
            pythonErrors = "Python Error: Script function is not named connectionFunc.";
        }
        Py_XDECREF(argsPy);
        Py_XDECREF(srcPy);
        Py_XDECREF(dstPy);
        Py_XDECREF(pyFunc);
        Py_XDECREF(pymod);
        return;
    }

    //Call my function
    PyObject * output = PyObject_CallObject(pyFunc, argsPy);
    Py_XDECREF(argsPy);
    Py_XDECREF(srcPy);
    Py_XDECREF(dstPy);

    Py_XDECREF(pyFunc);
    Py_XDECREF(pymod);

    if (!output) {
        this->pythonErrors = "Python Error: ";
        PyObject * errtype, * errval, * errtrace;
        PyErr_Fetch(&(errtype), &(errval), &(errtrace));

        if (errval) {
            pythonErrors += PyString_AsString(errval);
        }
        if (errtrace) {
            PyTracebackObject * errtraceObj = (PyTracebackObject *) errtrace;
            while (errtraceObj->tb_next) {
                errtraceObj = errtraceObj->tb_next;
            }
            pythonErrors += QString("Error found on line:") + QString::number(errtraceObj->tb_lineno);
        }
        return;
    }

    // unpack the output into C++ forms
    outputUnPackaged unpacked = extractOutput(output, this->hasDelay, this->hasWeight);

    // transfer the unpacked output to the local storage location for connections
    if (this->connection_target != NULL) {

        // remove existing connections
        this->connection_target->clearData();

        // if no connections are returned
        if (unpacked.connections.size() > 0) {
            // otherwise...
            if (this->hasDelay) {
                // if we have delays, resize
                this->connection_target->setNumCols(3);
            } else {
                //  no delays
                this->connection_target->setNumCols(2);
            }
        }

        for (int i = 0; i < unpacked.connections.size(); ++i) {

            // transfer connection
            this->connection_target->setData(i, 0, unpacked.connections[i].src);
            this->connection_target->setData(i, 1, unpacked.connections[i].dst);
            if (unpacked.connections[0].metric != NO_DELAY) {
                this->connection_target->setData(i, 2, unpacked.connections[i].metric);
            }
            this->connection_target->setNumRows(i);
        }

        this->connection_target->setNumRows(unpacked.connections.size());

    } else {
        this->connections = unpacked.connections;
        (*this->conns) = unpacked.connections;
    }

    // transfer the unpacked output to the local storage location for weights
    this->weights = unpacked.weights;

    // if we get to the end then that's good enough
    this->scriptValidates = true;
    this->setUnchanged(true);
}

bool pythonscript_connection::isList()
{
    return this->isAList;
}
