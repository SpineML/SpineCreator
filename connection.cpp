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

#include "connection.h"
//#include "stringify.h"
#include "cinterpreter.h"
#include "generate_dialog.h"

connection::connection()
{
    type = none;
    delay = new ParameterData("ms");
    delay->name = "delay";
    //delay->dims = new dim("");
    delay->currType = Undefined;
}

connection::~connection() {
    //delete delay->dims;
    delete delay;
}

void connection::writeDelay(QXmlStreamWriter &xmlOut) {

    xmlOut.writeStartElement("Delay");

    xmlOut.writeAttribute("Dimension", this->delay->dims->toString());

    if (this->delay->currType == FixedValue) {

        xmlOut.writeEmptyElement("FixedValue");
        xmlOut.writeAttribute("value", QString::number(this->delay->value[0]));

    }
    else if (this->delay->currType == Statistical) {
        //xmlOut.writeStartElement("stochasticValue");

        switch (int(round(this->delay->value[0]))) {
        case 0:
            break;
        case 1:
        {
            xmlOut.writeEmptyElement("UniformDistribution");
            xmlOut.writeAttribute("minimum", QString::number(this->delay->value[1]));
            xmlOut.writeAttribute("maximum", QString::number(this->delay->value[2]));
            xmlOut.writeAttribute("seed", QString::number(this->delay->value[2]));
        }
            break;
        case 2:
        {
            xmlOut.writeEmptyElement("NormalDistribution");
            xmlOut.writeAttribute("mean", QString::number(this->delay->value[1]));
            xmlOut.writeAttribute("variance", QString::number(this->delay->value[2]));
            xmlOut.writeAttribute("seed", QString::number(this->delay->value[2]));
         }
            break;
        }
        //xmlOut.writeEndElement(); // stochasticValue
    }
    else if (this->delay->currType == ExplicitList) {
        xmlOut.writeStartElement("ValueList");
        for (uint ind = 0; ind < this->delay->value.size(); ++ind) {
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

void alltoAll_connection::write_node_xml(QXmlStreamWriter &xmlOut) {

    xmlOut.writeStartElement("AllToAllConnection");
    this->writeDelay(xmlOut);
    xmlOut.writeEndElement(); // allToAllConnection

}

void alltoAll_connection::import_parameters_from_xml(QDomNode &e) {

    QDomNodeList delayProp = e.toElement().elementsByTagName("Delay");
    if (delayProp.size() == 1) {

        QDomNode n = delayProp.item(0);

        QDomNodeList propVal = n.toElement().elementsByTagName("FixedValue");
        if (propVal.size() == 1) {
            this->delay->currType = FixedValue;
            this->delay->value.resize(1,0);
            this->delay->value[0] = propVal.item(0).toElement().attribute("value").toFloat();
        }
        propVal = n.toElement().elementsByTagName("UniformDistribution");
        if (propVal.size() == 1) {
            this->delay->currType = Statistical;
            this->delay->value.resize(4,0);
            this->delay->value[0] = 1;
            this->delay->value[1] = propVal.item(0).toElement().attribute("minumum").toFloat();
            this->delay->value[2] = propVal.item(0).toElement().attribute("maximum").toFloat();
            this->delay->value[2] = propVal.item(0).toElement().attribute("seed").toFloat();
        }
        propVal = n.toElement().elementsByTagName("NormalDistribution");
        if (propVal.size() == 1) {
            this->delay->currType = Statistical;
            this->delay->value.resize(4,0);
            this->delay->value[0] = 2;
            this->delay->value[1] = propVal.item(0).toElement().attribute("mean").toFloat();
            this->delay->value[2] = propVal.item(0).toElement().attribute("variance").toFloat();
            this->delay->value[2] = propVal.item(0).toElement().attribute("seed").toFloat();
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

void onetoOne_connection::write_node_xml(QXmlStreamWriter &xmlOut) {

    xmlOut.writeStartElement("OneToOneConnection");
    this->writeDelay(xmlOut);
    xmlOut.writeEndElement(); // oneToOneConnection

}

void onetoOne_connection::import_parameters_from_xml(QDomNode &e) {
    QDomNodeList delayProp = e.toElement().elementsByTagName("Delay");
    if (delayProp.size() == 1) {

        QDomNode n = delayProp.item(0);

        QDomNodeList propVal = n.toElement().elementsByTagName("FixedValue");
        if (propVal.size() == 1) {
            this->delay->currType = FixedValue;
            this->delay->value.resize(1,0);
            this->delay->value[0] = propVal.item(0).toElement().attribute("value").toFloat();
        }
        propVal = n.toElement().elementsByTagName("UniformDistribution");
        if (propVal.size() == 1) {
            this->delay->currType = Statistical;
            this->delay->value.resize(3,0);
            this->delay->value[0] = 1;
            this->delay->value[1] = propVal.item(0).toElement().attribute("minumum").toFloat();
            this->delay->value[2] = propVal.item(0).toElement().attribute("maximum").toFloat();
        }
        propVal = n.toElement().elementsByTagName("NormalDistribution");
        if (propVal.size() == 1) {
            this->delay->currType = Statistical;
            this->delay->value.resize(3,0);
            this->delay->value[0] = 2;
            this->delay->value[1] = propVal.item(0).toElement().attribute("mean").toFloat();
            this->delay->value[2] = propVal.item(0).toElement().attribute("variance").toFloat();
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

void fixedProb_connection::write_node_xml(QXmlStreamWriter &xmlOut) {

    xmlOut.writeStartElement("FixedProbabilityConnection");
    xmlOut.writeAttribute("probability", QString::number(this->p));
    xmlOut.writeAttribute("seed", QString::number(this->seed));
    this->writeDelay(xmlOut);
    xmlOut.writeEndElement(); // fixedProbabilityConnection

}

void fixedProb_connection::import_parameters_from_xml(QDomNode &e) {

    this->p = e.toElement().attribute("probability").toFloat();
    this->seed = e.toElement().attribute("seed").toInt();

    QDomNodeList delayProp = e.toElement().elementsByTagName("Delay");
    if (delayProp.size() == 1) {

        QDomNode n = delayProp.item(0);

        QDomNodeList propVal = n.toElement().elementsByTagName("FixedValue");
        if (propVal.size() == 1) {
            this->delay->currType = FixedValue;
            this->delay->value.resize(1,0);
            this->delay->value[0] = propVal.item(0).toElement().attribute("value").toFloat();
        }
        propVal = n.toElement().elementsByTagName("UniformDistribution");
        if (propVal.size() == 1) {
            this->delay->currType = Statistical;
            this->delay->value.resize(4,0);
            this->delay->value[0] = 1;
            this->delay->value[1] = propVal.item(0).toElement().attribute("minumum").toFloat();
            this->delay->value[2] = propVal.item(0).toElement().attribute("maximum").toFloat();
            this->delay->value[2] = propVal.item(0).toElement().attribute("seed").toFloat();
        }
        propVal = n.toElement().elementsByTagName("NormalDistribution");
        if (propVal.size() == 1) {
            this->delay->currType = Statistical;
            this->delay->value.resize(4,0);
            this->delay->value[0] = 2;
            this->delay->value[1] = propVal.item(0).toElement().attribute("mean").toFloat();
            this->delay->value[2] = propVal.item(0).toElement().attribute("variance").toFloat();
            this->delay->value[2] = propVal.item(0).toElement().attribute("seed").toFloat();
        }

    }
}

/////////////////////////////////// EXPLICIT LIST

csv_connection::csv_connection() {

    type = CSV;
    numRows = 0;
    setUniqueName();

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
        if (!lib_dir.mkpath(lib_dir.absolutePath()))
            qDebug() << "error creating library";

    }

    this->file.setFileName(lib_dir.absoluteFilePath(this->filename));

    // open the storage file
    if( !this->file.open( QIODevice::ReadWrite ) ) {
        QMessageBox msgBox;
        msgBox.setText("Could not open output file for conversion");
        msgBox.exec();
        return;}

}

csv_connection::~csv_connection() {

    // remove memory usage

    if (this->file.isOpen()) {
        this->file.close();
    }

}

csv_connection::csv_connection(QString fileName) {

    type = CSV;
    numRows = 0;
    setUniqueName();

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
        if (!lib_dir.mkpath(lib_dir.absolutePath()))
            qDebug() << "error creating library";
    }

    this->file.setFileName(lib_dir.absoluteFilePath(this->filename));

    // open the storage file
    if( !this->file.open( QIODevice::ReadWrite ) ) {
        QMessageBox msgBox;
        msgBox.setText("Could not open output file for conversion");
        msgBox.exec();
        return;}

    QStringList list;
    list = fileName.split("/", QString::SkipEmptyParts);
    list = list.back().split("\\", QString::SkipEmptyParts);

    this->name = list.back();
    this->import_csv(fileName);

}

void csv_connection::setFileName(QString name) {
    filename = name;
}

QString csv_connection::getFileName() {
    return filename;
}

void csv_connection::write_node_xml(QXmlStreamWriter &xmlOut) {

    file.seek(0);
    QDataStream access(&file);

    // get a handle to the saved file
    QSettings settings;
    QString filePathString = settings.value("files/currentFileName", "error").toString();

    if (filePathString == "error") {
        qDebug() << "Error getting current project path - THIS SHOULD NEVER HAPPEN!";
        return;}

    QDir saveDir(filePathString);

    // fetch the option
    bool writeBinary = settings.value("fileOptions/saveBinaryConnections", "error").toBool();

    qDebug() << writeBinary;
    qDebug() << this->getNumRows();

    // load path
    bool exportBinary = false;
    if (settings.value("export_for_simulation", "false").toBool()) {
        writeBinary = false;
        exportBinary = settings.value("export_binary").toBool();
    }

    xmlOut.writeStartElement("ConnectionList");

    if (writeBinary && this->getNumRows() > 30) {

        QString saveFileName = this->filename + ".bin";

        // add a tag to the binary file
        xmlOut.writeEmptyElement("BinaryFile");
        xmlOut.writeAttribute("file_name", saveFileName);
        xmlOut.writeAttribute("num_connections", QString::number(float(getNumRows())));
        xmlOut.writeAttribute("explicit_delay_flag", QString::number(float(getNumCols()==3)));

        // copy the file
        file.copy(saveDir.absoluteFilePath(saveFileName));

        // reopen the file that copy helpfully closed grrr....
        file.open(QIODevice::ReadWrite);

    } else if (exportBinary && this->getNumRows() > 30) {

        QString saveFileName = QDir::toNativeSeparators(settings.value("simulator_export_path").toString() + "/" + this->filename + ".bin");

        // add a tag to the binary file
        xmlOut.writeEmptyElement("BinaryFile");
        xmlOut.writeAttribute("file_name", this->filename + ".bin");
        xmlOut.writeAttribute("num_connections", QString::number(float(getNumRows())));
        xmlOut.writeAttribute("explicit_delay_flag", QString::number(float(getNumCols()==3)));

        // re-write the data
        if (getNumCols()==3) {
            vector <conn> conns;
            this->getAllData(conns);

            // write out
            QFile export_file(saveFileName);

            if (!export_file.open( QIODevice::WriteOnly)) {
                QMessageBox msgBox;
                msgBox.setText("Error creating file - is there sufficient disk space?");
                msgBox.exec();
                return;
            }

            QDataStream access(&export_file);
            for (uint i = 0; i < conns.size(); ++i) {
                access.writeRawData((char*) &conns[i].src, sizeof(uint));
                access.writeRawData((char*) &conns[i].dst, sizeof(uint));
                access.writeRawData((char*) &conns[i].metric, sizeof(float));
            }
        }
        if (getNumCols()==2) {
            vector <conn> conns;
            this->getAllData(conns);

            // write out
            QFile export_file(saveFileName);

            if (!export_file.open( QIODevice::WriteOnly)) {
                QMessageBox msgBox;
                msgBox.setText("Error creating file - is there sufficient disk space?");
                msgBox.exec();
                return;
            }

            QDataStream access(&export_file);
            for (uint i = 0; i < conns.size(); ++i) {
                access.writeRawData((char*) &conns[i].src, sizeof(uint));
                access.writeRawData((char*) &conns[i].dst, sizeof(uint));
            }
        }


    } else {

        // loop through connections writing them out
        for (int i=0; i < this->getNumRows(); ++i) {

            xmlOut.writeEmptyElement("Connection");

            quint32 val;

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

void csv_connection::import_parameters_from_xml(QDomNode &e) {

    QDomNodeList BinaryFileList = e.toElement().elementsByTagName("BinaryFile");

    if (BinaryFileList.count() == 1) {

        // is a binary file so load accordingly

        // set number of connections
        this->setNumRows(BinaryFileList.at(0).toElement().attribute("num_connections").toUInt());

        // do we have explicit delays
        bool explicit_delay = BinaryFileList.at(0).toElement().attribute("explicit_delay_flag").toInt();
        if (explicit_delay)
            this->setNumCols(3);
        else
            this->setNumCols(2);

        // copy across file and set file name
        // first remove existing file
        this->file.remove();

        // get a handle to the saved file
        QSettings settings;
        QString filePathString = settings.value("files/currentFileName", "error").toString();

        if (filePathString == "error") {
            qDebug() << "Error getting current project path - THIS SHOULD NEVER HAPPEN!";
            return;}

        QDir filePath(filePathString);

        // get file name and path
        QString fileName = BinaryFileList.at(0).toElement().attribute("file_name");

        #if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
        QDir lib_dir = QDir(QDesktopServices::storageLocation(QDesktopServices::DataLocation));
        #else
        QDir lib_dir = QDir(QStandardPaths::writableLocation(QStandardPaths::DataLocation));
        #endif
        if (!lib_dir.exists()) {
            if (!lib_dir.mkpath(lib_dir.absolutePath()))
                qDebug() << "error creating library";
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
            return;}

    }

    if (BinaryFileList.count() != 1) {

        // load connections from xml
        file.seek(0);
        QDataStream access(&file);

        QDomNodeList connInstList = e.toElement().elementsByTagName("Connection");

        this->setNumRows(connInstList.size());

        for (uint i=0; i < (uint)connInstList.size(); ++i) {

            quint32 val = connInstList.at(i).toElement().attribute("src_neuron").toUInt();
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
            this->delay->value.resize(1,0);
            this->delay->value[0] = propVal.item(0).toElement().attribute("value").toFloat();
        }
        propVal = n.toElement().elementsByTagName("UniformDistribution");
        if (propVal.size() == 1) {
            this->delay->currType = Statistical;
            this->delay->value.resize(4,0);
            this->delay->value[0] = 1;
            this->delay->value[1] = propVal.item(0).toElement().attribute("minumum").toFloat();
            this->delay->value[2] = propVal.item(0).toElement().attribute("maximum").toFloat();
            this->delay->value[2] = propVal.item(0).toElement().attribute("seed").toFloat();
        }
        propVal = n.toElement().elementsByTagName("NormalDistribution");
        if (propVal.size() == 1) {
            this->delay->currType = Statistical;
            this->delay->value.resize(4,0);
            this->delay->value[0] = 2;
            this->delay->value[1] = propVal.item(0).toElement().attribute("mean").toFloat();
            this->delay->value[2] = propVal.item(0).toElement().attribute("variance").toFloat();
            this->delay->value[2] = propVal.item(0).toElement().attribute("seed").toFloat();
        }

    }


    // flush out the output...
    file.flush();

}

void csv_connection::fetch_headings() {


}

void csv_connection::import_csv(QString fileName) {

    this->numRows = 0;

    this->changes.clear();

    //wipe file;
    file.resize(0);

    // open the input csv file for reading
    QFile fileIn(fileName);

    if( !fileIn.open( QIODevice::ReadOnly ) ) {
        QMessageBox msgBox;
        msgBox.setText("Could not open the selected file");
        msgBox.exec();
        return;}

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
            return;}

        if (fields.size() < 2) {
            QMessageBox msgBox;
            msgBox.setText("CSV file has too few columns");
            msgBox.exec();
            return;}

        if (numFields == -1) {
            numFields = fields.size();
        } else if (numFields != fields.size()) {
            qDebug() << "something is wrong!";
            --numRows;
            continue;
        }
        // for each field
        for (unsigned int i = 0; i < (uint)fields.size(); ++i) {
            if (i < 2) {
                quint32 num = fields[i].toUInt();
                access << num;
            } else {
                float num = fields[i].toFloat();
                access << num;
            }

        }
    }

    values.clear();
    for (unsigned int i = 0; i < (uint)numFields; ++i) {
        if (i == 0) this->values.push_back("src");
        if (i == 1) this->values.push_back("dst");
        if (i == 2) this->values.push_back("delay");
    }

    // flush out the output...
    this->file.flush();

}

int csv_connection::getNumRows() {
    return this->numRows;
}

void csv_connection::setNumRows(int num) {
    this->numRows = num;
}

int csv_connection::getNumCols() {
    return this->values.size();
}

void csv_connection::setNumCols(int num) {
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


void csv_connection::getAllData(vector < conn > &conns) {

    //qDebug() << "ALL CONN DATA FETCHED";

    // rewind file
    file.seek(0);

    QDataStream access(&file);

    conns.resize(this->getNumRows());
    int counter = 0;

    for (int i = 0; i < getNumRows(); ++i) {

        conn newConn;

        quint32 src;
        quint32 dst;

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

float csv_connection::getData(int rowV, int col) {

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
        quint32 data;
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

float csv_connection::getData(QModelIndex &index) {

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
        quint32 data;
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

void csv_connection::setUniqueName() {

    //generate a unique filename to save the weights under

    // start investigating the library
    #if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    QDir lib_dir = QDir(QDesktopServices::storageLocation(QDesktopServices::DataLocation));
#else
    QDir lib_dir = QDir(QStandardPaths::writableLocation(QStandardPaths::DataLocation));
#endif
    qDebug() << lib_dir.absolutePath();
    if (!lib_dir.exists()) {
        if (!lib_dir.mkpath(lib_dir.absolutePath()))
            qDebug() << "error creating library";
    }

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
        for (unsigned int i = 0; i < (uint)files.count(); ++i) {
            // see if the new name is unique
            if (uniqueName == files[i]) {
                unique = false;
                ++index;
            }
        }
    }
    this->filename = uniqueName;

}

QString csv_connection::getHeader(int section) {

    return this->values[section];

}

void csv_connection::setData(const QModelIndex & index, float value) {



    // get a datastream to serialise the data
    file.seek(file.size());

    QDataStream access(&file);

    if (index.row() > this->getNumRows()) {
        // resize
        for (int i = getNumRows()*getNumCols(); i < getNumCols()*index.row(); ++i) {
            quint32 num = 0;
            access << num;
        }
    }

    int colVal = getNumCols();
    if (colVal > 2) ++colVal;
    int seekTo = index.row()*(colVal)+index.column();

    file.seek(seekTo*4); // seek to location in bytes

    if (index.column() < 2)
        access << (quint32) value;
    else
        access << (float) value;

    file.flush();
}

void csv_connection::setData(int row, int col, float value) {

    // get a datastream to serialise the data
    file.seek(file.size());

    QDataStream access(&file);

    if (row > this->getNumRows()) {
        // resize
        for (int i = getNumRows()*getNumCols(); i < getNumCols()*row; ++i) {
            quint32 num = 0;
            access << num;
        }
    }

    int colVal = getNumCols();
    if (colVal > 2) ++colVal;
    int seekTo = row*(colVal)+col;

    file.seek(seekTo*4); // seek to location in bytes

    if (col < 2)
        access << (quint32) value;
    else
        access << (float) value;

    file.flush();
}


void csv_connection::abortChanges() {
    changes.clear();
}

distanceBased_connection::distanceBased_connection()
{
    type = DistanceBased;
    this->isAList = false;
    selfConnections = false;
    hasChanged = true;
    srcSize = 0;
    dstSize = 0;
}

distanceBased_connection::~distanceBased_connection()
{
}

bool distanceBased_connection::changed() {
    if (src->numNeurons != srcSize || dst->numNeurons != dstSize)
        return true;
    else
        return hasChanged;
}

bool distanceBased_connection::changed(QString newEquation) {

    if (newEquation != equation && newEquation.size() > 0)
        return true;
    else if (src->numNeurons != srcSize || dst->numNeurons != dstSize)
        return true;
    else
        return hasChanged;
}

void distanceBased_connection::setUnchanged(bool state) {
    if (state) {
        srcSize = src->numNeurons;
        dstSize = dst->numNeurons;
    }
    hasChanged = !state;
}

void distanceBased_connection::write_node_xml(QXmlStreamWriter &xmlOut) {

    // are we outputting for simulation
    bool forSim = false;
    QSettings settings;
    forSim = settings.value("export_for_simulation", "0").toBool();

    if (!this->isAList && !forSim) {
        xmlOut.writeStartElement("DistanceBasedConnection");
        // extra stuff
        xmlOut.writeStartElement("Probability");

        xmlOut.writeEndElement(); // Probability
        this->writeDelay(xmlOut);
        xmlOut.writeEndElement(); // DistanceBasedConnection
    }

    else {

        xmlOut.writeStartElement("ConnectionList");

        // generate connections:
        QMutex * connGenerationMutex = new QMutex();

        if (changed()) {
            generate_dialog generate(this, this->src, this->dst, connections, connGenerationMutex, (QWidget *)NULL);
            bool retVal = generate.exec();
            if (!retVal)
                return;
            setUnchanged(true);
        }

        delete connGenerationMutex;

        if (connections.size() == 0) {
            QMessageBox msgBox;
            msgBox.setText("Error: no connections generated for Distance-based Connection");
            msgBox.exec();
            return;
        }

        // load path
        bool exportBinary = false;
        if (settings.value("export_for_simulation", "error").toBool()) {
            exportBinary = settings.value("export_binary").toBool();
        }

        // loop through connections writing them out
         if (!exportBinary || connections.size() < 30) {
            for (uint i=0; i < connections.size(); ++i) {

                xmlOut.writeEmptyElement("Connection");

                xmlOut.writeAttribute("src_neuron", QString::number(float(connections[i].src)));
                xmlOut.writeAttribute("dst_neuron", QString::number(float(connections[i].dst)));

                if (!this->delayEquation.isEmpty()) {
                    xmlOut.writeAttribute("delay", QString::number(float(connections[i].metric)));
                }
            }
         }
         else {

           QUuid uuid = QUuid::createUuid();
           QString export_filename = uuid.toString();
           export_filename.chop(1);
           export_filename[0] = 'C';
           export_filename += ".bin"; // need to generate a unique filename - preferably a descriptive one... but not for now!
           QString saveFileName = QDir::toNativeSeparators(settings.value("simulator_export_path").toString() + "/" + export_filename);

           // add a tag to the binary file
           xmlOut.writeEmptyElement("BinaryFile");
           xmlOut.writeAttribute("file_name", export_filename);
           xmlOut.writeAttribute("num_connections", QString::number(float(connections.size())));
           if (!this->delayEquation.isEmpty()) {
                xmlOut.writeAttribute("explicit_delay_flag", QString::number(float(0)));
           } else {
                xmlOut.writeAttribute("explicit_delay_flag", QString::number(float(1)));
           }

           // re-write the data

           // write out
           QFile export_file(saveFileName);
           if (!export_file.open( QIODevice::WriteOnly)) {
               QMessageBox msgBox;
               msgBox.setText("Error creating file - is there sufficient disk space?");
               msgBox.exec();
               return;
           }

           QDataStream access(&export_file);
           for (uint i = 0; i < connections.size(); ++i) {
               access.writeRawData((char*) &connections[i].src, sizeof(uint));
               access.writeRawData((char*) &connections[i].dst, sizeof(uint));

               if (!this->delayEquation.isEmpty()) {
                   access.writeRawData((char*) &connections[i].metric, sizeof(float));
               }
           }

       }


        if (this->delayEquation.isEmpty())
            this->writeDelay(xmlOut);

        xmlOut.writeEndElement(); // ConnectionList
    }


}

void distanceBased_connection::import_parameters_from_xml(QDomNode &e) {

    QDomNodeList delayProp = e.toElement().elementsByTagName("Delay");
    if (delayProp.size() == 1) {

        QDomNode n = delayProp.item(0);

        QDomNodeList propVal = n.toElement().elementsByTagName("FixedValue");
        if (propVal.size() == 1) {
            this->delay->currType = FixedValue;
            this->delay->value.resize(1,0);
            this->delay->value[0] = propVal.item(0).toElement().attribute("value").toFloat();
        }
        propVal = n.toElement().elementsByTagName("UniformDistribution");
        if (propVal.size() == 1) {
            this->delay->currType = Statistical;
            this->delay->value.resize(4,0);
            this->delay->value[0] = 1;
            this->delay->value[1] = propVal.item(0).toElement().attribute("minumum").toFloat();
            this->delay->value[2] = propVal.item(0).toElement().attribute("maximum").toFloat();
            this->delay->value[2] = propVal.item(0).toElement().attribute("seed").toFloat();
        }
        propVal = n.toElement().elementsByTagName("NormalDistribution");
        if (propVal.size() == 1) {
            this->delay->currType = Statistical;
            this->delay->value.resize(4,0);
            this->delay->value[0] = 2;
            this->delay->value[1] = propVal.item(0).toElement().attribute("mean").toFloat();
            this->delay->value[2] = propVal.item(0).toElement().attribute("variance").toFloat();
            this->delay->value[2] = propVal.item(0).toElement().attribute("seed").toFloat();
        }

    }
}
/*
void distanceBased_connection::generate_connections(population * src, population * dst, vector < conn > &conns) {

    // if we have something to do!
    if (!this->equation.isEmpty()) {

        conns.clear();

        // if src or dst hasn't generated locations then do it
        QString errorLog;
        //if (src->layoutType->locations.size() == 0) {
                src->layoutType->generateLayout(src->numNeurons,&src->layoutType->locations,errorLog);
                if (!errorLog.isEmpty())
                    return;
        //}
        //if (dst->layoutType->locations.size() == 0) {
                dst->layoutType->generateLayout(dst->numNeurons,&dst->layoutType->locations,errorLog);
                if (!errorLog.isEmpty())
                    return;
        //}

        // create the variable list:
        vector < lookup > varList;

        varList.push_back(lookup("d", 0));

        // provided:
        varList.push_back(lookup("e", M_E));
        varList.push_back(lookup("pi", M_PI));

        // create stack
        vector < valop > stack;
        errorLog = createStack(this->equation, varList, &stack);

        // if the stack compiles
        if (errorLog.isEmpty()) {

            float total_ops = src->layoutType->locations.size();

            for (uint i = 0; i < src->layoutType->locations.size(); ++i) {
                for (uint j = 0; j < dst->layoutType->locations.size(); ++j) {
                    // set d
                    varList[0].value = sqrt(pow(dst->layoutType->locations[j].x - src->layoutType->locations[i].x,2)+ \
                                            pow(dst->layoutType->locations[j].y - src->layoutType->locations[i].y,2)+ \
                                            pow(dst->layoutType->locations[j].z - src->layoutType->locations[i].z,2));
                    //qDebug() << src->name << " " << dst->name << " " << dst->layoutType->locations[j].x << " " << src->layoutType->locations[i].x;
                    float result = interpretMaths(stack);
                    //qDebug() << result;
                    if (result > 0) {
                        conn newConn;
                        newConn.src = i;
                        newConn.dst = j;
                        conns.push_back(newConn);
                    }


                }
                emit progress((int) round(float(i)/total_ops * 100.0));
            }
        }
    }

}*/


void distanceBased_connection::generate_connections() {

    // if we have something to do!
    if (!this->equation.isEmpty()) {

        conns->clear();
        errorLog.clear();

        // if src or dst hasn't generated locations then do it
        //if (src->layoutType->locations.size() == 0) {
                src->layoutType->generateLayout(src->numNeurons,&src->layoutType->locations,errorLog);
                if (!errorLog.isEmpty()) {
                     this->moveToThread(QApplication::instance()->thread());
                     emit connectionsDone();
                     return;
                }
        //}
        //if (dst->layoutType->locations.size() == 0) {
                dst->layoutType->generateLayout(dst->numNeurons,&dst->layoutType->locations,errorLog);
                if (!errorLog.isEmpty()) {
                    this->moveToThread(QApplication::instance()->thread());
                    emit connectionsDone();
                    return;
                }
        //}

        // create the variable list:
        vector < lookup > varList;

        varList.push_back(lookup("d", 0));
        varList.push_back(lookup("xs_abs", 0));
        varList.push_back(lookup("ys_abs", 0));
        varList.push_back(lookup("zs_abs", 0));


        // provided:
        varList.push_back(lookup("e", M_E));
        varList.push_back(lookup("pi", M_PI));

        // create stack
        vector < valop > stack;
        errorLog = createStack(this->equation, varList, &stack);

        // if delays...
        vector < valop > delayStack;
        if (!this->delayEquation.isEmpty()) {
            errorLog = createStack(this->delayEquation, varList, &delayStack);
        }

        // if the stack compiles
        if (errorLog.isEmpty()) {

            float total_ops = src->layoutType->locations.size();
            int scale_val = round(10000000.0/(src->layoutType->locations.size()*dst->layoutType->locations.size()));

            int oldprogress = 0;

            for (uint i = 0; i < src->layoutType->locations.size(); ++i) {

                for (uint j = 0; j < dst->layoutType->locations.size(); ++j) {
                    // set d
                    varList[0].value = sqrt(pow(dst->layoutType->locations[j].x - src->layoutType->locations[i].x,2)+ \
                                            pow(dst->layoutType->locations[j].y - src->layoutType->locations[i].y,2)+ \
                                            pow(dst->layoutType->locations[j].z - src->layoutType->locations[i].z,2));
                    varList[1].value = src->layoutType->locations[i].x;
                    varList[2].value = src->layoutType->locations[i].y;
                    varList[3].value = src->layoutType->locations[i].z;
                    // HACK!!! - no self connections for now
                    if (varList[0].value < 0.0001 && !this->selfConnections)
                        continue;
                    //qDebug() << src->name << " " << dst->name << " " << dst->layoutType->locations[j].x << " " << src->layoutType->locations[i].x;
                    float result = interpretMaths(stack);
                    //qDebug() << result;
                    if (result > 0) {
                        mutex->lock();
                        conn newConn;
                        newConn.src = i;
                        newConn.dst = j;
                        if (!delayEquation.isEmpty())
                            newConn.metric = interpretMaths(delayStack);
                        conns->push_back(newConn);
                        mutex->unlock();
                    }
                }


                if (round(float(i)/total_ops * 100.0) > oldprogress) {
                    emit progress((int) round(float(i)/total_ops * 100.0));
                    oldprogress = round(float(i)/total_ops * 100.0)+scale_val;
                }
            }
        }
    }
    this->moveToThread(QApplication::instance()->thread());
    emit connectionsDone();
}

void distanceBased_connection::convertToList(bool check) {

    // instantiate the connection for simulators etc...
    this->isAList = check;
    systemObject * ptr;
    ptr = (systemObject *) sender()->property("ptrSrc").value<void *>();
    src = (population *) ptr;
    ptr = (systemObject *) sender()->property("ptrDst").value<void *>();
    dst = (population *) ptr;

}

bool distanceBased_connection::isList() {

    return this->isAList;

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

bool kernel_connection::changed() {
    if (src->numNeurons != srcSize || dst->numNeurons != dstSize)
        return true;
    else
        return hasChanged;
}

void kernel_connection::setUnchanged(bool state) {
    if (state) {
        srcSize = src->numNeurons;
        dstSize = dst->numNeurons;
    }
    hasChanged = !state;
}

void kernel_connection::setKernelSize(int size) {
    if (kernel_size != size) {
        hasChanged = true;
        kernel_size = size;
    }
}

void kernel_connection::setKernelScale(float scale) {
    if (kernel_scale != scale) {
        hasChanged = true;
        kernel_scale = scale;
    }
}

void kernel_connection::setKernel(int i, int j, float value) {
    if (kernel[i][j] != value) {
        hasChanged = true;
        kernel[i][j] = value;
    }
}

void kernel_connection::write_node_xml(QXmlStreamWriter &xmlOut) {

    // are we outputting for simulation
    bool forSim = false;
    QSettings settings;
    QString sim = settings.value("export_for_simulation", "no").toString();

    if (sim != "no")
        forSim = true;

    if (!this->isAList && !forSim) {
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
    }

    else {

        xmlOut.writeStartElement("ConnectionList");

        // generate connections:
        QMutex * connGenerationMutex = new QMutex();

        if (changed()) {
            setUnchanged(true);
            connections.clear();
            generate_dialog generate(this, this->src, this->dst, connections, connGenerationMutex, (QWidget *)NULL);
            bool retVal = generate.exec();
            if (!retVal)
                return;
        }

        if (connections.size() == 0) {
            QMessageBox msgBox;
            msgBox.setText("Error: no connections generated for Kernel Connection");
            msgBox.exec();
            return;
        }

        delete connGenerationMutex;

        // load path
        bool exportBinary = false;
        if (settings.value("export_for_simulation", "error").toBool()) {
            exportBinary = settings.value("export_binary").toBool();
        }

        if (!exportBinary || connections.size() < 30) {

            // loop through connections writing them out
            for (uint i=0; i < connections.size(); ++i) {

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
            QString saveFileName = QDir::toNativeSeparators(settings.value("simulator_export_path").toString() + "/" + export_filename);

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
                msgBox.setText("Error creating file - is there sufficient disk space?");
                msgBox.exec();
                return;
            }

            QDataStream access(&export_file);
            for (uint i = 0; i < connections.size(); ++i) {
                access.writeRawData((char*) &connections[i].src, sizeof(uint));
                access.writeRawData((char*) &connections[i].dst, sizeof(uint));
            }

        }

        this->writeDelay(xmlOut);

        xmlOut.writeEndElement(); // ConnectionList
    }


}

void kernel_connection::import_parameters_from_xml(QDomNode &e) {

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
            this->delay->value.resize(1,0);
            this->delay->value[0] = propVal.item(0).toElement().attribute("value").toFloat();
        }
        propVal = n.toElement().elementsByTagName("UniformDistribution");
        if (propVal.size() == 1) {
            this->delay->currType = Statistical;
            this->delay->value.resize(4,0);
            this->delay->value[0] = 1;
            this->delay->value[1] = propVal.item(0).toElement().attribute("minumum").toFloat();
            this->delay->value[2] = propVal.item(0).toElement().attribute("maximum").toFloat();
            this->delay->value[2] = propVal.item(0).toElement().attribute("seed").toFloat();
        }
        propVal = n.toElement().elementsByTagName("NormalDistribution");
        if (propVal.size() == 1) {
            this->delay->currType = Statistical;
            this->delay->value.resize(4,0);
            this->delay->value[0] = 2;
            this->delay->value[1] = propVal.item(0).toElement().attribute("mean").toFloat();
            this->delay->value[2] = propVal.item(0).toElement().attribute("variance").toFloat();
            this->delay->value[2] = propVal.item(0).toElement().attribute("seed").toFloat();
        }

    }
}

void kernel_connection::generate_connections() {

    conns->clear();

    // if src or dst hasn't generated locations then do it
    QString errorLog;
    //if (src->layoutType->locations.size() == 0) {
            src->layoutType->generateLayout(src->numNeurons,&src->layoutType->locations,errorLog);
            if (!errorLog.isEmpty())
                return;
    //}
    //if (dst->layoutType->locations.size() == 0) {
            dst->layoutType->generateLayout(dst->numNeurons,&dst->layoutType->locations,errorLog);
            if (!errorLog.isEmpty())
                return;
    //}

    float total_ops = src->layoutType->locations.size();
    int scale_val = round(100000000.0/(src->layoutType->locations.size()*dst->layoutType->locations.size()));

    int oldprogress = 0;

    for (uint i = 0; i < src->layoutType->locations.size(); ++i) {
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

void kernel_connection::convertToList(bool check) {

    // instantiate the connection for simulators etc...
    this->isAList = check;
    systemObject * ptr;
    ptr = (systemObject *) sender()->property("ptrSrc").value<void *>();
    src = (population *) ptr;
    ptr = (systemObject *) sender()->property("ptrDst").value<void *>();
    dst = (population *) ptr;

}

bool kernel_connection::isList() {

    return this->isAList;

}





/*

void csv_connection::flushChangesToDisk() {

    // not used anymore!
    return;

    // set the filename
    QString lib_path = QApplication::applicationDirPath();
    QDir lib_dir = QDir(lib_path + "/lib");
    this->file.setFileName(lib_dir.absoluteFilePath(this->filename));

    //open file for reading
    if( !this->file.open( QIODevice::ReadOnly ) ) {
        QMessageBox msgBox;
        msgBox.setText("Error opening internal file - is the drive out of space?");
        msgBox.exec();
        }

    // start investigating the library and find a new filename to transfer the file to
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
        for (unsigned int i = 0; i < (uint)files.count(); ++i) {
            // see if the new name is unique
            if (uniqueName == files[i]) {
                unique = false;
                ++index;
            }
        }
    }
    QString newFilename = uniqueName;

    QFile newFile;
    newFile.setFileName(lib_dir.absoluteFilePath(newFilename));

    //open new file for writing
    if( !newFile.open( QIODevice::WriteOnly ) ) {
        QMessageBox msgBox;
        msgBox.setText("Error opening internal file - is the drive out of space?");
        msgBox.exec();
        }


    // sort the changes so that we have a list where the first change to be applied is first
    vector < change > sortedChanges;
    while (changes.size() > 0) {
        int ind = 0;
        int minRow = 1000000000;
        int minCol = 1000000000;
        for (uint i = 0; i < changes.size(); ++i) {

            if (changes[i].row < minRow) {
                ind = i;
                minRow = changes[i].row;
                minCol = changes[i].col;
            }
            else if (changes[i].row == minRow && changes[i].col < minCol) {
                ind = i;
                minRow = changes[i].row;
                minCol = changes[i].col;
            } else if (changes[i].row == minRow && changes[i].col == minCol) {
                // remove superseded change
                changes.erase(changes.begin()+ind, changes.begin()+ind+1);
                --i;
                // set new lowest change
                ind = i;
                minRow = changes[i].row;
                minCol = changes[i].col;
            }

        }
        // add lowest change to new list and erase from main list;
        sortedChanges.push_back(changes[ind]);
        changes.erase(changes.begin()+ind, changes.begin()+ind+1);
    }

    this->xmlIn.setDevice(&this->file);

    this->xmlOut.setDevice(&newFile);

    int counter = 0;

    // start the document
    this->xmlOut.setAutoFormatting(true);
    this->xmlOut.writeStartDocument();
    this->xmlOut.writeStartElement("NineMLConnection");

    // stream from one file to the other, applying changes as we go!
    while(!this->xmlIn.atEnd()) {
        if(xmlIn.readNext()) {

            if (this->xmlIn.isStartElement() && this->xmlIn.name() == "connection") {

                vector < float > returnVals;

                // read in
                for (unsigned int i = 0; i < (uint)this->xmlIn.attributes().count(); ++i) {
                    returnVals.push_back(this->xmlIn.attributes().value("value"+QString::number(float(i))).toString().toFloat());
                }

                // apply changes
                if (sortedChanges.size()) {
                    while (sortedChanges.front().row == counter) {
                        returnVals[sortedChanges.front().col] = sortedChanges.front().value;
                        sortedChanges.erase(sortedChanges.begin());
                        // get out if we have run out of changes
                        if (!sortedChanges.size()) break;
                    }
                }

                // write out
                this->xmlOut.writeEmptyElement("connection");
                for (unsigned int i = 0; i < returnVals.size(); ++i) {
                    QString attr = "value" + QString::number(float(i));
                    this->xmlOut.writeAttribute(attr, QString::number(returnVals[i]));
                }

                // increase the number of connections seen
                ++counter;

            }

        }
    }

    // flush remaining changes
    while (sortedChanges.size()) {
        vector < float > returnVals;

        while (sortedChanges.front().row == counter) {
            // changes should be in order
            returnVals.push_back(sortedChanges.front().value);
            sortedChanges.erase(sortedChanges.begin());
            // if we run our then get out of here
            if (!sortedChanges.size()) break;
        }

        this->xmlOut.writeEmptyElement("connection");
        for (unsigned int i = 0; i < returnVals.size(); ++i) {
            QString attr = "value" + QString::number(float(i));
            this->xmlOut.writeAttribute(attr, QString::number(returnVals[i]));
        }

        ++counter;
    }
    this->xmlOut.writeEndElement();
    this->xmlOut.writeEndDocument();

    // close up
    this->file.close();
    newFile.close();
    // remove old file
    QFile::remove(lib_dir.absoluteFilePath(this->filename));
    // update filename to new file
    filename = newFilename;
}

*/
