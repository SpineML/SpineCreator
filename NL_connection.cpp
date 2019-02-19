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


#include <cmath>

#ifdef _DEBUG
  #undef _DEBUG
  #include <Python.h>
  #define _DEBUG
#else
  #include <Python.h>
#endif


#include <QUuid>

#include <QSettings>

#include "NL_connection.h"
#include "SC_layout_cinterpreter.h"
#include "SC_python_connection_generate_dialog.h"
#include "SC_viewVZlayoutedithandler.h"
#include "filteroutundoredoevents.h"

connection::connection()
{
    this->type = none;
    this->delay = new ParameterInstance("ms");
    this->delay->name = "delay";
    this->delay->currType = Undefined;
    this->srcName = "";
    this->dstName = "";
    this->synapseIndex = -1;
    this->generator = NULL;
    this->scriptText = "";
}

connection::~connection()
{
    delete this->delay;
}

int connection::getIndex()
{
    return (int) this->type;
}

QString connection::getTypeStr(void)
{
    QString ctype("");
    switch (this->type) {
    case AlltoAll:
        ctype = "all to all";
        break;
    case OnetoOne:
        ctype = "one to one";
        break;
    case FixedProb:
        ctype = "fixed probability"; // add params?
        break;
    case CSV:
        ctype = "explicit list";
        break;
    case Python:
        ctype = "python script"; // Get script name. Should be handled by python conn type
        break;
    case CSA:
        ctype = "CSA (deprecated)";
        break;
    default:
        ctype = "Unknown";
        break;
    }
    return ctype;
}

void connection::setSrcName (QString& s)
{
    this->srcName = s;
}

void connection::setDstName (QString& d)
{
    this->dstName = d;
}

void connection::setSynapseIndex (int synidx)
{
    this->synapseIndex = synidx;
}

int connection::getSynapseIndex (void)
{
    return this->synapseIndex;
}

void connection::setParent (QSharedPointer<systemObject> ptr)
{
    this->parent = ptr;
}

void connection::writeDelay(QXmlStreamWriter &xmlOut)
{
    xmlOut.writeStartElement("Delay");

    xmlOut.writeAttribute("dimension", this->delay->dims->toString());

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
    this->synapseIndex = -2;
}

alltoAll_connection::~alltoAll_connection()
{
}

QLayout * alltoAll_connection::drawLayout(nl_rootdata *, viewVZLayoutEditHandler * viewVZhandler, nl_rootlayout * rootLay)
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

QLayout * onetoOne_connection::drawLayout(nl_rootdata *, viewVZLayoutEditHandler * viewVZhandler, nl_rootlayout * rootLay)
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
    // Sanity check - if we are exporting for simulation, we want to make sure
    // that src and dst are the same size...
    // if parent is inputObject, that means we can access the destination and source from that object
    int srcSize = -2;
    int dstSize = -1;
    QString srcName = "";
    QString dstName = "";
    QSettings settings;
    if (!this->parent.isNull()) {
        switch (this->parent->type) {
        case synapseObject:
        {
            // This is the usual
            // Test this->src and this->dst first
            if (!this->srcPop.isNull()) {
                srcSize = this->srcPop->numNeurons;
                srcName = this->srcPop->getName();
            } // else src is null - there's no population as a destination for this connection
            if (!this->dstPop.isNull()) {
                dstSize = this->dstPop->numNeurons;
                dstName = this->dstPop->getName();
            } // else dst is null - there's no population as a destination for this connection
            break;
        }
        case projectionObject:
        {
            DBG() << "onetoOne_connection parent is a projectionObject (unexpected)";
            break;
        }
        case populationObject:
        {
            DBG() << "onetoOne_connection parent is a populationObject (unexpected)";
            break;
        }
        case inputObject:
        {
            QSharedPointer<genericInput> par = qSharedPointerCast <genericInput> (this->parent);
            srcSize = par->getSrcSize();
            srcName = par->getSrcName();
            dstSize = par->getDestSize();
            dstName = par->getDestName();
            /*if (srcSize == -1 || dstSize == -1) {
                int num_errs = settings.beginReadArray("errors");
                settings.endArray();
                settings.beginWriteArray("errors");
                settings.setArrayIndex(num_errs + 1);
                settings.setValue("errorText",  QString("One to one connections to Weight Updates are not allowed: ") + par->getSrcName() + QString(" -> ") + par->getDestName());
                settings.endArray();
            }*/
            break;
        }
        default:
            break;
        }
    } else {
        srcSize = dstSize;
    }
    if (srcSize != dstSize && false) { // not used for now

       int num_errs = settings.beginReadArray("errors");
        settings.endArray();
        settings.beginWriteArray("errors");
        settings.setArrayIndex(num_errs + 1);
        settings.setValue("errorText",  QString("One to one connection with different src and dst sizes: ") + srcName + QString(" -> ") + dstName);
        settings.endArray();

    } else {

        xmlOut.writeStartElement("OneToOneConnection");
        this->writeDelay(xmlOut);
        xmlOut.writeEndElement(); // oneToOneConnection

    }
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
    p = 0.01f;
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
QLayout * fixedProb_connection::drawLayout(nl_rootdata * data, viewVZLayoutEditHandler * viewVZhandler, nl_rootlayout * rootLay)
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
    // no connectivity generator in constructor
    generator = NULL;

    this->srcName = "";
    this->dstName = "";
    this->synapseIndex = -3;

    // Defaults to having 3 things in. This sets numCols to be 3 by default.
    this->values.push_back("src");
    this->values.push_back("dst");
    this->values.push_back("delay");

    copiedFrom = NULL;

    // Generate the unique UUID style filename here in the constructor.
    this->generateUUIDFilename();
}

QDir csv_connection::getLibDir (void) const
{
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    QDir lib_dir = QDir(QDesktopServices::storageLocation(QDesktopServices::DataLocation));
#else
    QDir lib_dir = QDir(QStandardPaths::writableLocation(QStandardPaths::DataLocation));
#endif
    if (!lib_dir.exists()) {
        if (!lib_dir.mkpath(lib_dir.absolutePath())) {
            DBG() << "error creating library";
        }
    }
    return lib_dir;
}

csv_connection::~csv_connection()
{
    // remove generator
    if (this->generator) {
        delete this->generator;
        this->generator = NULL;
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

void csv_connection::updateGlobalDelay (void)
{
    QCheckBox* sndr = (QCheckBox*)sender();
    nl_rootdata * data = (nl_rootdata *) sender()->property("dataptr").value<void *>();
    data->updateConnection (this->parent, sndr->isChecked());
}

QLayout * csv_connection::drawLayout(nl_rootdata * data, viewVZLayoutEditHandler * viewVZhandler, nl_rootlayout * rootLay)
{
    // if we do not have a generator...
    if (this->generator == NULL) {

        QVBoxLayout* vlay = new QVBoxLayout();
        QTableView* tableView = new QTableView();
        csv_connectionModel* connMod = new csv_connectionModel();

        // rootLayout::projSelected has communicated this information
        // so we can stick it into this connection.
        if (data->currentlySelectedProjection != NULL) {
            this->srcPop = data->currentlySelectedProjection->source;
            this->dstPop = data->currentlySelectedProjection->destination;
        } else {
            DBG() << "Maybe a false assumption here: NL_connection.cpp, csv_connection::drawLayout";
        }

        connMod->setConnection(this);
        tableView->setModel(connMod);

        vlay->addWidget(tableView);

        QHBoxLayout * hlay = new QHBoxLayout();

        QPushButton *import = new QPushButton("Import list");
        import->setToolTip("Import the explicit value list");
        import->setProperty("ptr", qVariantFromValue((void *) this));
        hlay->addWidget(import);
        // add connection:
        connect(import, SIGNAL(clicked()), data, SLOT(editConnections()));

        QCheckBox* globalDelay = new QCheckBox("Global delay");
        globalDelay->setToolTip("Switch between a single, global delay for each connection or per-connection delays (which are not supported in some simulators).");
        globalDelay->setProperty("dataptr", qVariantFromValue((void *) data));
        // Set from number of cols in connection
        if (this->getNumCols() == 2) {
            globalDelay->setCheckState(Qt::Checked);
        } else { // should be 3
            globalDelay->setCheckState(Qt::Unchecked);
        }
        hlay->addWidget(globalDelay);
        connect(globalDelay, SIGNAL(clicked()), this, SLOT(updateGlobalDelay()));

        vlay->addLayout(hlay);

        // set up GL:
        if (viewVZhandler) {
            connect(viewVZhandler, SIGNAL(deleteProperties()), tableView, SLOT(deleteLater()));
            if (viewVZhandler->viewVZ->OpenGLWidget->getConnectionsModel() != (QAbstractTableModel *)0) {
                // don't fetch data if we already have for this connection
                csv_connectionModel * connModel = dynamic_cast<csv_connectionModel *> (viewVZhandler->viewVZ->OpenGLWidget->getConnectionsModel());
                CHECK_CAST(connModel);
                if (connModel->getConnection() == this
                    && (int)viewVZhandler->viewVZ->OpenGLWidget->connections.size() == this->getNumRows()) {
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
            connect(viewVZhandler, SIGNAL(deleteProperties()), globalDelay, SLOT(deleteLater()));
            connect(viewVZhandler, SIGNAL(deleteProperties()), hlay, SLOT(deleteLater()));
            connect(viewVZhandler, SIGNAL(deleteProperties()), vlay, SLOT(deleteLater()));
        }
        if (rootLay) {
            connect(rootLay, SIGNAL(deleteProperties()), import, SLOT(deleteLater()));
            connect(rootLay, SIGNAL(deleteProperties()), tableView, SLOT(deleteLater()));
            connect(rootLay, SIGNAL(deleteProperties()), globalDelay, SLOT(deleteLater()));
            connect(rootLay, SIGNAL(deleteProperties()), hlay, SLOT(deleteLater()));
            connect(rootLay, SIGNAL(deleteProperties()), vlay, SLOT(deleteLater()));
        }

        return vlay;

    } else {
        // we have a generator, so pass on the task of drawing the layout to it!
        return generator->drawLayout(data, viewVZhandler, rootLay);
    }
}

void csv_connection::setFileName(QString name)
{
    this->filename = name;
}

QString csv_connection::getFileName()
{
    return filename;
}

QString csv_connection::getUUIDFileName()
{
    return uuidFilename;
}

void csv_connection::write_node_xml(QXmlStreamWriter &xmlOut)
{
    if (this->filename.isEmpty()) {
        this->generateFilename();
    }

    QFile f;
    QDir lib_dir = this->getLibDir(); // This is the temporary location for conn data files
    f.setFileName(lib_dir.absoluteFilePath(this->uuidFilename));
    if (!f.open( QIODevice::ReadOnly)) {
        QMessageBox msgBox;
        msgBox.setText("csv_connection::write_node_xml(QXmlStreamWriter &xmlOut): Could not open temporary file '" + f.fileName() + "' for Explicit Connection");
        msgBox.exec();
        return;
    }
    f.seek(0);
    QDataStream access(&f);

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
        DBG() << "Error getting current project path - THIS SHOULD NEVER HAPPEN!";
        return;
    }

    QDir saveDir(filePathString);

    bool saveBinaryConnections = settings.value("fileOptions/saveBinaryConnections", "error").toBool();

    // write containing tag
    xmlOut.writeStartElement("ConnectionList");

    // Annotations
    if (this->generator || !this->annotation.isEmpty()) {
        xmlOut.writeStartElement("LL:Annotation");
        // old annotations
        if (!this->annotation.isEmpty()) {
            this->annotation.replace("\n", "");
            this->annotation.replace("<LL:Annotation>", "");
            this->annotation.replace("</LL:Annotation>", "");
            QXmlStreamReader reader(this->annotation);
            while (!reader.atEnd()) {
                if (reader.tokenType() != QXmlStreamReader::StartDocument
                    && reader.tokenType() != QXmlStreamReader::EndDocument) {
                    xmlOut.writeCurrentToken(reader);
                }
                reader.readNext();
            }
        }

        if (this->generator) {
            this->generator->write_metadata_xml(&xmlOut);
        }
        xmlOut.writeEndElement();//Annotation
    }

    // if we have more than 30 rows and we want to write to binary then write binary file
    // (less than 30 rows is sufficiently compact that it should go in the XML)
    QString saveFullFileName;
    if (saveBinaryConnections && this->getNumRows() > MIN_CONNS_TO_FORCE_BINARY) {

        QString saveProjectName = settings.value("files/currentFileName").toString();

        QDir project_dir(saveProjectName);

        // remove filename
        project_dir.cdUp();

        if (this->filename.isEmpty()) {
            QMessageBox msgBox;
            msgBox.setText("Error creating exported binary connection file srcName/dstName:'" + this->srcName
                           + "/" + this->dstName + "' (filename could not be generated from src/dest population names)");
            msgBox.exec();
            return;
        }
        saveFullFileName = QDir::toNativeSeparators(project_dir.absoluteFilePath(this->filename));

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

            QDataStream access2(&export_file);
            for (int i = 0; i < conns.size(); ++i) {
                access2.writeRawData((char*) &conns[i].src, sizeof(int));
                access2.writeRawData((char*) &conns[i].dst, sizeof(int));
                access2.writeRawData((char*) &conns[i].metric, sizeof(float));
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

            QDataStream access3(&export_file);
            for (int i = 0; i < conns.size(); ++i) {
                access3.writeRawData((char*) &conns[i].src, sizeof(int));
                access3.writeRawData((char*) &conns[i].dst, sizeof(int));
            }
        }


    } else { // non-binary; write only into XML

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
 * \brief csv_connection::read_metadata_xml
 * \param e
 *
 * Read the metaData for a connection generator - may be blank if there is not one
 * TO BE DEPRECATED.
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
    // check for annotations
    QDomNodeList anns = e.toElement().elementsByTagName("LL:Annotation");

    if (anns.size() == 1) {
        // annotations found - do we have a generator?
        QDomNode metaData;
        QDomNodeList scAnns = anns.at(0).toElement().elementsByTagName("SpineCreator");
        if (scAnns.length() == 1) {
            metaData = scAnns.at(0).cloneNode();
            anns.at(0).removeChild(scAnns.at(0));
            // add generator
            if (this->srcPop == NULL) {
                DBG() << "Warning: srcPop is null and using it to create pythonscript_connection...";
            }
            this->generator = new pythonscript_connection(this->srcPop, this->dstPop, this);
            pythonscript_connection * pyConn = dynamic_cast<pythonscript_connection *> (this->generator);
            CHECK_CAST(pyConn)
            // extract data for connection generator
            pyConn->read_metadata_xml (metaData);
            // prevent regeneration
            //pyConn->setUnchanged(true);
        }
        QTextStream temp(&this->annotation);
        anns.at(0).save(temp,1);
    }

    QDomNodeList BinaryFileList = e.toElement().elementsByTagName("BinaryFile");

    if (BinaryFileList.count() == 1) {

        // is a binary file so load accordingly

        // set number of connections
        this->setNumRows(BinaryFileList.at(0).toElement().attribute("num_connections").toUInt());

        // do we have explicit delays
        bool explicit_delay = BinaryFileList.at(0).toElement().attribute("explicit_delay_flag").toInt();
        if (explicit_delay) {
            this->setNumCols(3);
        } else {
            this->setNumCols(2);
        }

        // check what the file type is, since we changed from using QStreamData written binary to
        // packed binary we need to check for the packed_binary flag

        QString isPacked = BinaryFileList.at(0).toElement().attribute("packed_data", "false");

        if (isPacked == "true") {

            // get a handle to the saved file
            QSettings settings;
            QString filePathString = settings.value("files/currentFileName", "error").toString();

            if (filePathString == "error") {
                DBG() << "Error getting current project path - THIS SHOULD NEVER HAPPEN!";
                return;
            }

            QDir filePath(filePathString);

            // get file name and path
            QString fileName = BinaryFileList.at(0).toElement().attribute("file_name");

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

            // Open the binary data file
            QDir lib_dir = this->getLibDir();
            QFile f;
            f.setFileName(lib_dir.absoluteFilePath(this->uuidFilename));

            // open the storage file
            if( !f.open( QIODevice::ReadWrite | QIODevice::Truncate) ) {
                QMessageBox msgBox;
                msgBox.setText("csv_connection::import_parameters_from_xml(QDomNode &e): Could not open temporary file '" + f.fileName() + "' for Explicit Connection");
                msgBox.exec();
                return;
            }

            // now we need to read from the savedData file and put this into a QDataStream...
            this->import_packed_binary(savedData, f);
            f.close();

        } else {
            DBG() << "Old, non-packed data format is no longer supported";
        }

    } else { // BinaryFileList.count() != 1

        // No BinaryFileList, so the ConnectionList must be stated in the XML
        /*if (this->filename.isEmpty()) {
            this->generateFilename();
        }*/

        // load connections from xml
        QFile f;
        QDir lib_dir = this->getLibDir();
        f.setFileName(lib_dir.absoluteFilePath(this->uuidFilename));
        if (!f.open( QIODevice::ReadWrite | QIODevice::Truncate)) {
            QMessageBox msgBox;
            msgBox.setText("csv_connection::import_parameters_from_xml(QDomNode &e) [2]: Could not open temporary file '" + f.fileName() + "' for Explicit Connection");
            msgBox.exec();
            return;
        }
        QDataStream access(&f);

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
                if (this->values.size()> 2) {
                    this->values.removeLast();
                }
            }
        }

        f.close();
    }

    // After loading, sort the connections.
    this->sortData();

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
}

void csv_connection::fetch_headings()
{
}

bool csv_connection::import_csv (QString fileName)
{
    DBG() << "csv_connection::import_csv(" << fileName << ") called.";

    if (fileName.isEmpty()) {
        DBG() << "No data to read, return false";
        return false;
    }

    // Assume import fails to work. Set true once import has succeeded.
    bool import_worked = false;

    // open the input csv file for reading
    QFile fileIn(fileName);
    if (!fileIn.open(QIODevice::ReadOnly)) {
        QMessageBox msgBox;
        msgBox.setText("Could not open the selected CSV file");
        msgBox.exec();
        return import_worked;
    }

    QFile f;
    QDir lib_dir = this->getLibDir();
    // Set up a temporary uuid
    f.setFileName(lib_dir.absoluteFilePath (this->uuidFilename));
    if (!f.open( QIODevice::ReadWrite | QIODevice::Truncate)) {
        QMessageBox msgBox;
        msgBox.setText("csv_connection::import_csv(QString): Could not open temporary file '"
                       + f.fileName() + "' for Explicit Connection");
        msgBox.exec();
        return import_worked;
    } // else the data file for this connection has now been truncated, so numRows can be set to 0.

    this->numRows = 0;
    this->changes.clear();

    // use textstream so we can read lines into a QString
    QTextStream stream(&fileIn);
    QDataStream access(&f);

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
            return import_worked;
        }

        if (fields.size() < 2) {
            QMessageBox msgBox;
            msgBox.setText("CSV file has too few columns");
            msgBox.exec();
            return import_worked;
        }

        if (numFields == -1) {
            numFields = fields.size();
        } else if (numFields != fields.size()) {
            DBG() << "something is wrong!";
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
        if (i == 0) { this->values.push_back("src"); }
        if (i == 1) { this->values.push_back("dst"); }
        if (i == 2) { this->values.push_back("delay"); }
    }

    // flush out the output...
    f.flush();
    f.close();

    // Sort the connection list now.
    this->sortData();

    import_worked = true;
    return import_worked;
}

void csv_connection::import_packed_binary(QFile& fileIn, QFile& fileOut)
{
    this->changes.clear();

    //wipe file;
    fileOut.resize(0);

    fileOut.seek(0);

    QDataStream access(&fileOut);

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
        DBG() << "Mismatch between the number of rows in the XML and in the binary file";
    }

    // flush out the output...
    fileOut.flush();
}

int csv_connection::getNumRows() const
{
    return this->numRows;
}

void csv_connection::setNumRows(int num)
{
    this->numRows = num;
}

int csv_connection::getNumCols() const
{
    return this->values.size();
}

void csv_connection::setNumCols(int num)
{
    if (num == 2) {
        this->values.clear();
        this->values.push_back("src");
        this->values.push_back("dst");
    } else if (num == 3) {
        this->values.clear();
        this->values.push_back("src");
        this->values.push_back("dst");
        this->values.push_back("delay");
    } else {
        DBG() << "Bad number of cols requested.";
        exit (-1);
    }
}

void csv_connection::sortData (void)
{
    // AJC 12/04/2017 - Disabled as conflicts with Python scripts that generate weights - this
    // function causes the weights to not line up with the associated connection...
    /*QVector<conn> clist;
    this->getAllData (clist);
    std::sort (clist.begin(), clist.end(), csv_connection::sorttwo);
    this->setAllData (clist);*/
}

bool csv_connection::sorttwo (conn a, conn b) {
    if (a.src < b.src) { return true; }
    if (a.src == b.src && a.dst < b.dst) { return true; }
    return false;
}

void csv_connection::getAllData(QVector<conn>& conns)
{
    conns.clear();

    QFile f;
    QDir lib_dir = this->getLibDir();
    f.setFileName(lib_dir.absoluteFilePath(this->uuidFilename));
    if (!f.open( QIODevice::ReadOnly)) {
        return;
    }

    QDataStream access(&f);

    conns.resize(this->getNumRows());
    int counter = 0;

    for (int i = 0; i < this->getNumRows(); ++i) {

        conn newConn;
        qint32 src;
        qint32 dst;

        access >> src;
        access >> dst;

        if (this->getNumCols() > 2) {
            float temp_delay;
            access >> temp_delay;
            newConn.metric = temp_delay;
        }

        newConn.src = src;
        newConn.dst = dst;

        conns[counter] = (newConn);
        ++counter;
    }

    f.close();
}

float csv_connection::getData(int rowV, int col) const
{
    QFile f;
    QDir lib_dir = this->getLibDir();
    f.setFileName(lib_dir.absoluteFilePath(this->uuidFilename));
    if (!f.open( QIODevice::ReadOnly)) {
        QMessageBox msgBox;
        msgBox.setText("csv_connection::getData(int, int): Could not open file for Explicit Connection");
        msgBox.exec();
        return -0.1f;
    }

    // we multiply by cols +1 as QT seems to do some padding on the stream
    int colVal = this->getNumCols();
    if (colVal > 2) { ++colVal; }
    int seekTo = rowV*(colVal)+col;

    if (seekTo*4 > f.size()) {
        return -1;
    }

    f.seek(seekTo*4); // seek to location in bytes

    // get a datastream to serialise the data
    QDataStream access(&f);
    if (col < 2) {
        qint32 data;
        access >> data;
        f.close();
        return float(data);
    } else {
        float data;
        access >> data;
        f.close();
        return data;
    }

    return -0.1f;
}

float csv_connection::getData(QModelIndex &index) const
{
    QFile f;
    QDir lib_dir = this->getLibDir();
    f.setFileName(lib_dir.absoluteFilePath(this->uuidFilename));
    if (!f.open( QIODevice::ReadOnly)) {
        QMessageBox msgBox;
        msgBox.setText("csv_connection::getData(QModelIndex&): Could not open file '"
                       + this->uuidFilename + "' for Explicit Connection");
        msgBox.exec();
        return -0.1f;
    }

    int colVal = getNumCols();
    if (colVal > 2) ++colVal;
    // we multiply by cols +1 as QT seems to do some padding on the stream
    int seekTo = index.row()*(colVal)+index.column();

    if (seekTo*4 > f.size()) {
        return -1;
    }

    f.seek(seekTo*4); // seek to location in bytes

    // get a datastream to serialise the data
    QDataStream access(&f);
    if (index.column() < 2) {
        qint32 data;
        access >> data;
        f.close();
        return float(data);
    }
    else {
        float data;
        access >> data;
        f.close();
        return data;
    }

    return -0.1f;
}

/*!
 * Character sets useful when calling sanitize function.
 *
 * These are ordered so that the most common chars appear earliest.
 */
//@{
#define CHARS_NUMERIC            "0123456789"
#define CHARS_ALPHA              "etaoinshrdlcumwfgypbvkjxqzETAOINSHRDLCUMWFGYPBVKJXQZ"
#define CHARS_ALPHALOWER         "etaoinshrdlcumwfgypbvkjxqz"
#define CHARS_ALPHAUPPER         "ETAOINSHRDLCUMWFGYPBVKJXQZ"
#define CHARS_NUMERIC_ALPHA      "etaoinshrdlcumwfgypbvkjxqz0123456789ETAOINSHRDLCUMWFGYPBVKJXQZ"
#define CHARS_NUMERIC_ALPHALOWER "etaoinshrdlcumwfgypbvkjxqz0123456789"
#define CHARS_NUMERIC_ALPHAUPPER "0123456789ETAOINSHRDLCUMWFGYPBVKJXQZ"
//@}

void csv_connection::generateUUIDFilename(void)
{
    QUuid fntag = QUuid::createUuid();
    this->uuidFilename = "conn_";
    this->uuidFilename += fntag.toString();
    this->uuidFilename.replace(QString("{"), QString(""));
    this->uuidFilename.replace(QString("}"), QString(""));
    this->uuidFilename += ".bin";
    //DBG() << "csv_connection::generateUUIDFilename(): Set uuidFilename to " << this->uuidFilename;
}

void csv_connection::generateFilename(void)
{
    QString baseName = "conn_";
    this->filename = "";

    // if parent is inputObject, that means we can access the destination and source from that object
    switch (this->parent->type) {
    case synapseObject:
    {
        // This is the usual
        // Test this->src and this->dst first
        if (!this->srcPop.isNull()) {
            this->srcName = this->srcPop->name;
        } // else src is null - there's no population as a destination for this connection
        if (!this->dstPop.isNull()) {
            this->dstName = this->dstPop->name;
        } // else dst is null - there's no population as a destination for this connection
        break;
    }
    case projectionObject:
    {
        DBG() << "csv_connection parent is a projectionObject (unexpected)";
        break;
    }
    case populationObject:
    {
        DBG() << "csv_connection parent is a populationObject (unexpected)";
        break;
    }
    case inputObject:
    {
        QSharedPointer<genericInput> par = qSharedPointerCast <genericInput> (this->parent);
        this->srcName = par->getSrcName();
        this->dstName = par->getDestName();
        break;
    }
    default:
        break;
    }

    // Now make sure srcName and dstName are filename friendly -
    // replace all spaces, tabs and other gnarly characters with an
    // underscore.
    char replaceChar = '_';
    QString allowed (CHARS_NUMERIC_ALPHA".");
    this->sanitizeReplace (this->srcName, allowed, replaceChar);
    this->sanitizeReplace (this->dstName, allowed, replaceChar);

    if (this->synapseIndex > -1) {
        this->filename = baseName + this->srcName + "_to_" + this->dstName + "_syn" + QString::number(this->synapseIndex) + ".bin";
    } else {
        this->filename = baseName + this->srcName + "_to_" + this->dstName + ".bin";
    }
}

void csv_connection::sanitizeReplace (QString& str,
                                      const QString& allowed,
                                      const char replaceChar)
{
    int i=0;
    while (i<str.size()) {
        if (allowed.indexOf(str[i], 0) == -1) {
            // str[i] is forbidden
            str[i] = replaceChar;
        }
        i++;
    }
}

QString csv_connection::getHeader(int section)
{
    return this->values[section];
}

void csv_connection::updateDataForNumCols (int num)
{
    if (this->values.size() == num) {
        // Assume nothing to do
        return;
    }

    QVector<conn> conns;
    conns.resize(this->getNumRows());
    this->getAllData (conns);

    // Now update the number of cols
    this->setNumCols (num);

    // Write out again
    this->setAllData (conns);
}

void csv_connection::setData(const QModelIndex & index, float value)
{
    QFile f;
    QDir lib_dir = this->getLibDir();
    f.setFileName(lib_dir.absoluteFilePath(this->uuidFilename));
    if (!f.open( QIODevice::ReadWrite)) {
        QMessageBox msgBox;
        msgBox.setText("csv_connection::setData(const QModelIndex&, float): Could not open temporary file " + this->uuidFilename + " for Explicit Connection");
        msgBox.exec();
        return;
    }

    // get a datastream to serialise the data
    f.seek(f.size());

    QDataStream access(&f);

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

    f.seek(seekTo*4); // seek to location in bytes

    if (index.column() < 2) {
        access << (qint32) value;
    } else {
        access << (float) value;
    }
    f.flush();
    f.close();
}

void csv_connection::setData(int row, int col, float value)
{
    QFile f;
    QDir lib_dir = this->getLibDir();
    f.setFileName(lib_dir.absoluteFilePath(this->uuidFilename));
    if (!f.open( QIODevice::ReadWrite)) {
        QMessageBox msgBox;
        msgBox.setText("csv_connection::setData(int, int, float): Could not open temporary file "
                       + this->uuidFilename + " for Explicit Connection");
        msgBox.exec();
        return;
    }

    // get a datastream to serialise the data
    f.seek(f.size());

    QDataStream access(&f);

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

    f.seek(seekTo*4); // seek to location in bytes

    if (col < 2) {
        access << (qint32) value;
    } else {
        access << (float) value;
    }
    f.flush();
    f.close();
}

void csv_connection::setAllData (QVector<conn>& conns)
{
    QFile f;
    QDir lib_dir = this->getLibDir();
    f.setFileName(lib_dir.absoluteFilePath(this->uuidFilename));
    if (!f.open( QIODevice::ReadWrite | QIODevice::Truncate)) {
        QMessageBox msgBox;
        msgBox.setText("csv_connection::setAllData(QVector<conn>&): Could not open temporary file "
                       + this->uuidFilename + " for Explicit Connection");
        msgBox.exec();
        return;
    }

    // get a datastream to serialise the data
    f.seek(f.size());

    QDataStream access(&f);

    int nc = this->getNumCols();

    int seekTo = 0;
    f.seek(seekTo*4);

    float singleDelay = -1.0;
    if (nc == 3) {
        if (this->delay != (ParameterInstance*)0) {
            if (this->delay->currType == FixedValue) {
                singleDelay = (float)this->delay->value[0];
            } else {
                // We have a ParameterInstance, but it's not
                // null. It's also not a FixedValue. If the delays are
                // specified as individual, per-connection delays,
                // then we will never have copied them into the
                // ParameterInstance object, but they will exist, so
                // write out the individual delays here.
                singleDelay = -1.0;
            }
        } else {
            // No delay ParameterInstance, so write out delay from conns[i].
            singleDelay = -1.0;
        }
    }

    for (int i = 0; i<conns.size(); ++i) {
        access << (qint32) conns[i].src;
        access << (qint32) conns[i].dst;
        if (nc == 3) {
            if (singleDelay > 1.0) {
                access << (float) singleDelay;
            } else {
                access << (float) conns[i].metric;
            }
        }
        f.flush();
    }

    f.close();
}

void csv_connection::clearData()
{
    QFile f;
    QDir lib_dir = this->getLibDir();
    f.setFileName(lib_dir.absoluteFilePath(this->uuidFilename));
    if (f.open(QIODevice::ReadWrite)) {
        f.remove();
        f.close();
        DBG() << "csv_connection::clearData(): removed temporary connection data file " << this->uuidFilename;
    }
}

void csv_connection::abortChanges()
{
    this->changes.clear();
}

void csv_connection::copyDataValues (const csv_connection* other)
{
    // If either this or other have 2 cols, then copy just 2 cols, if
    // both have same number of cols, direct copy is fine.
    int maxcol = 2;
    if (this->getNumCols() == other->getNumCols()) {
        // Direct copy data...
        maxcol = other->getNumCols();
    } // else copy data cols 1 and 2 only - maxcols remains 2.

    for (this->numRows = 0; this->numRows < other->getNumRows(); ++this->numRows) {
        for (int j = 0; j < maxcol; j++) {
            float d = other->getData(this->numRows,j);
            this->setData (this->numRows, j, d);
        }
    }
}

connection * csv_connection::newFromExisting()
{
    // create a new csv_connection
    csv_connection * c = new csv_connection;

    // only use the same columns
    c->values = this->values;

    // Copy the parent. The new csv_connection will share the same parent.
    c->parent = this->parent;

    // use the same delay
    c->delay = new ParameterInstance(this->delay);

    c->copiedFrom = this;

    // now copy the data... (do this later)
    //c->copyDataValues(this);

    // now, do we have a generator?
    if (this->generator != NULL) {
        // copy generator
        c->generator = this->generator->newFromExisting();
    }

    return c;
}

pythonscript_connection::pythonscript_connection(QSharedPointer <population> src, QSharedPointer <population> dst, csv_connection* conn_targ)
{
    this->type = Python;
    this->isAList = false;
    this->selfConnections = false;
    this->rotation = 0;
    this->hasChanged = true;
    this->scriptValidates = false;
    this->hasWeight = false;
    this->hasDelay = false;
    this->srcPop = src;
    this->dstPop = dst;
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
        DBG() << "Error with script " << this->scriptName;
    }

    // use Python as the base index and increment by the script number
    return (int) this->type + index;
}

QString pythonscript_connection::getTypeStr(void)
{
    // Could add "with weights" and "with delays".
    return this->scriptName;
}

QLayout * pythonscript_connection::drawLayout(nl_rootdata * data, viewVZLayoutEditHandler * viewVZhandler, nl_rootlayout * rootLay)
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

    if (srcPop->numNeurons != srcSize || dstPop->numNeurons != dstSize || par_changed) {
        return true;
    } else {
        return hasChanged;
    }
}

void pythonscript_connection::setUnchanged(bool state)
{
    if (state) {
        if (this->srcPop != NULL) {
            this->srcSize = srcPop->numNeurons;
        } else {
            DBG() << "pythonscript_connection::setUnchanged: No srcPop!?";
        }
        if (this->dstPop != NULL) {
            this->dstSize = dstPop->numNeurons;
        } else {
            DBG() << "pythonscript_connection::setUnchanged: No dstPop!?";
        }
        for (int i = 0; i < this->lastGeneratedParValues.size(); ++i) {
            this->lastGeneratedParValues[i] = this->parValues[i];
        }
        this->lastGeneratedWeightProp = this->weightProp;
        this->lastGeneratedScriptText = scriptText;

        this->hasChanged = false;

    } else {
        this->hasChanged = true;
    }
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
    generate_dialog generate(this, this->srcPop, this->dstPop, this->connections, connGenerationMutex, (QWidget *)NULL);
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
    DBG() << "pythonscript_connection::write_node_xml(QXmlStreamWriter &) called (that's an error)";
}

void pythonscript_connection::write_metadata_xml(QXmlStreamWriter* xmlOut)
{
    // write out the settings for this generator
    xmlOut->writeStartElement("SpineCreator");

    // write out the script and parameters
    xmlOut->writeEmptyElement("Script");

    // script text
    xmlOut->writeAttribute("text", this->scriptText);
    xmlOut->writeAttribute("name", this->scriptName);

    // parameter values for the script
    for (int i = 0; i < this->parNames.size(); ++i) {
        xmlOut->writeAttribute(this->parNames[i], QString::number(this->parValues[i]));
    }

    // write out configuration information
    xmlOut->writeEmptyElement("Config");

    if (!this->weightProp.isEmpty()) {
        xmlOut->writeAttribute("weightProperty", this->weightProp);
    }

    xmlOut->writeEndElement(); // SpineCreator
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

    // Now try to match the script from the model with a script in the
    // library. If there's no match, then add the script to the
    // library. Deal with the case where the model script named "x"
    // differs from the library script named "x".

    QSettings settings;
    // enter group of scripts
    settings.beginGroup("pythonscripts");
    // fetch a list of scripts
    QStringList scripts = settings.childKeys();

    // Here, we're comparing the script stored in the library with
    // name scriptName with scriptText (which has just been copied
    // in from the model).
    if (settings.value(this->scriptName,"not found") == this->scriptText) {
        // User's library DOES contain a script whose name and content
        // matches the one in the model.
        // DBG() << "Your library has an identical copy of the model script " << this->scriptName;

    } else if (settings.value(this->scriptName,"not found") == "not found") {

        DBG() << "No library script named " << this->scriptName
              << " was found; loading it into your library.";

        // In this case, load the model script into the library with the same name.
        if (this->scriptName == "") {
            this->scriptName = "loaded connection";
        }
        // add the script to the library
        settings.setValue(this->scriptName, this->scriptText);

    } else {

        DBG() << "library version of script " << this->scriptName << " differs from model version.";

        // In this case, ask the user what to do. User may either load
        // the model version into their library and rename their
        // existing function OR keep their existing version and save
        // the one from the model into the library for reference, but
        // when they save, their library version of the connection
        // function will make its way into the model.
        bool preferLibraryVersionOfScript = false;
        QMessageBox libModel;
        libModel.setWindowTitle("Versions of " + this->scriptName + " script differ");
        libModel.setText("This model contains a version of the python connection script "
                         + this->scriptName + " which is different from the version in "
                         + "your script library (Edit->Settings->Python Scripts). Please "
                         + "choose whether to prefer the library version (importing the "
                         + "model's version as " + this->scriptName
                         + "_model_<date>) or the model version (making a backup of the "
                         + "current library version as " + this->scriptName + "_lib_<date>).");
        QPushButton* libraryButton = libModel.addButton("Prefer Library", QMessageBox::AcceptRole);
        QPushButton* modelButton = libModel.addButton("Prefer Model", QMessageBox::AcceptRole);
        libModel.exec();
        if (libModel.clickedButton() == libraryButton) {
            preferLibraryVersionOfScript = true;
        } else if (libModel.clickedButton() == modelButton) {
            preferLibraryVersionOfScript = false;
        } else {
            DBG() << "Box closed/cancelled; preferring model version of script.";
            // Leave preferLibraryVersionOfScript = false;
        }

        if (preferLibraryVersionOfScript) {

            DBG() << "User prefers library version of the script " << this->scriptName;

            // Now test to see if there is an identical script already
            // in the library with another (or the same) name.
            bool identicalFound = false;
            QString identicalName = "";
            for (int i = 0; i < scripts.size(); ++i) {
                if (settings.value(scripts[i],"not found") == this->scriptText) {
                    identicalFound = true;
                    identicalName = scripts[i];
                }
            }

            if (identicalFound == true) {
                // Library already contains this script.
                DBG() << "A version of the script " << this->scriptName
                      << " from the model was found in the library where it is named "
                      << identicalName;
            } else {
                // ...otherwise, add the script. However, if there's a
                // non-identical version of the script in the library with
                // the new name, change the new name with a suffix.
                QString newScriptName(this->scriptName + "_model_" + QDateTime::currentDateTime().toString("yyyyMMdd"));
                int i = 1;
                while (scripts.contains(newScriptName)) {
                    newScriptName = this->scriptName + "_model_"
                        + QDateTime::currentDateTime().toString("yyyyMMdd")
                        + "_" + QString::number(i++);
                }
                settings.setValue(newScriptName, this->scriptText);
            }

        } else { // preferModelVersionOfScript

            DBG() << "User prefers the model version of the script " << this->scriptName;

            QString newScriptName(this->scriptName + "_lib_" + QDateTime::currentDateTime().toString("yyyyMMdd"));
            // If the existing version of scriptName is stored in the
            // library as a copy with another name, signal this to the
            // user and do nothing before copying the model version
            // into the library.
            bool copyFound = false;
            QString copyName = "";
            for (int i = 0; i < scripts.size(); ++i) {
                if (scripts[i] != this->scriptName
                    && ( settings.value(scripts[i], "not found")
                         == settings.value(this->scriptName,
                                           "# An error occurred in connection.cpp: "
                                           "pythonscript_connection::read_metadata_xml"))) {
                    copyFound = true;
                    copyName = scripts[i];
                }
            }

            if (copyFound == true) {
                DBG() << "An existing copy of the library version of "
                      << this->scriptName << " is present with the name " << copyName;
            } else {
                // No existing copy, make a new copy.
                DBG() << "Copying existing library version of "
                      << this->scriptName << " to " << newScriptName;
                settings.setValue(newScriptName,
                                  settings.value(this->scriptName,
                                                 "# An error occurred in connection.cpp: "
                                                 "pythonscript_connection::read_metadata_xml"));
            }

            // Finally, copy the model version of the script into the
            // library with the name scriptName
            settings.setValue(this->scriptName, this->scriptText);
        }
    }

    // exit the scripts group
    settings.endGroup();
}

ParameterInstance * pythonscript_connection::getPropPointer()
{
    for (int i = 0; i < this->srcPop->projections.size(); ++i) {
        QSharedPointer <projection> proj = this->srcPop->projections[i];
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
                for (int k = 0; k < syn->weightUpdateCmpt->ParameterList.size(); ++k) {
                    if (syn->weightUpdateCmpt->ParameterList[k]->name == this->weightProp) {
                        // found the weight
                        return syn->weightUpdateCmpt->ParameterList[k];
                    }
                }
                for (int k = 0; k < syn->weightUpdateCmpt->StateVariableList.size(); ++k) {
                    if (syn->weightUpdateCmpt->StateVariableList[k]->name == this->weightProp) {
                        // found the weight
                        return syn->weightUpdateCmpt->StateVariableList[k];
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
    for (int i = 0; i < this->srcPop->projections.size(); ++i) {
        QSharedPointer <projection> proj = this->srcPop->projections[i];
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
                for (int k = 0; k < syn->weightUpdateCmpt->ParameterList.size(); ++k) {
                    // found the weight
                    list.push_back(syn->weightUpdateCmpt->ParameterList[k]->name);
                }
                for (int k = 0; k < syn->weightUpdateCmpt->StateVariableList.size(); ++k) {
                    // found the weight
                    list.push_back(syn->weightUpdateCmpt->StateVariableList[k]->name);
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
        vect.push_back(-234.56f);
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
                outUnPacked.connections[i].src = PyLong_AsLong(PyTuple_GetItem(element,0));
                outUnPacked.connections[i].dst = PyLong_AsLong(PyTuple_GetItem(element,1));
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
PyObject* createPyFunc(PyObject* pymod, QString text, QString &errs)
{
    // get the default dict, so we have access to the built in modules
    PyObject* main = PyImport_AddModule ("__main__");
    PyObject* pGlobal = PyModule_GetDict (main);

    PyModule_AddStringConstant (pymod, "__file__", "");

    // Get the dictionary object from my module so I can pass this to PyRun_String
    PyObject* pLocal = PyModule_GetDict (pymod);

    // Define my function in the newly created module
    PyObject * pValue = PyRun_String ((char*)text.toStdString().c_str(), Py_file_input, pGlobal, pLocal);
    if (!pValue) {
        PyObject * errtype, * errval, * errtrace;
        PyErr_Fetch(&(errtype), &(errval), &(errtrace));

        errs.append("ERROR in PyRun_String ");
        cerr << "Error in PyRun_String()" << endl;

        if (errtype) {
            errs.append(PyBytes_AsString(errtype) + QString("(errtype). "));
        }
        if (errval) {
            errs.append(PyBytes_AsString(errval) + QString("(errval). "));
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

    // Get a pointer to the function I just defined
    return PyObject_GetAttrString (pymod, "connectionFunc");
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
    srcPop->layoutType->generateLayout(srcPop->numNeurons,&srcPop->layoutType->locations,errorLog);
    if (!errorLog.isEmpty()) {
        DBG() << "no src locs";
        return;
    }
    dstPop->layoutType->generateLayout(dstPop->numNeurons,&dstPop->layoutType->locations,errorLog);
    if (!errorLog.isEmpty()) {
        DBG() << "no dst locs";
        return;
    }

    // a tuple to hold the arguments to the Python Script - size of the scripts pars + the src and dst locations
    PyObject * argsPy = PyTuple_New(this->parNames.size()+2/* 2 for the src and dst locations*/);

    // convert the locations into Python Objects:
    PyObject * srcPy = vectorLocToList(&srcPop->layoutType->locations);
    PyObject * dstPy = vectorLocToList(&dstPop->layoutType->locations);

    // add them to the tuple
    PyTuple_SetItem(argsPy,0,srcPy);
    PyTuple_SetItem(argsPy,1,dstPy);

    // convert the parameters into Python Objects and add them to the tuple
    for (int i = 0; i < this->parNames.size(); ++i) {
        PyTuple_SetItem(argsPy,i+2,PyFloat_FromDouble(parValues[i]));
    }

    // check the tuple is sound
    if (!argsPy) {
        DBG() << "Bad args tuple";
        Py_XDECREF(argsPy);
        Py_XDECREF(srcPy);
        Py_XDECREF(dstPy);
        return;
    }

    // Create a new module object
    PyObject* pymod = PyModule_New ("mymod");

    // add the function to Python, and get a PyObject for it
    PyObject* pyFunc = createPyFunc (pymod, this->scriptText, this->pythonErrors);

    // check that function creation worked
    if (!pyFunc) {
        cerr << "createPyFunc returned null" << endl;
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

    // Call my function
    DBG() << "Calling the function";
    PyObject * output = PyObject_CallObject (pyFunc, argsPy);
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
            pythonErrors += PyBytes_AsString(errval);
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
    DBG() << "Got output from function";

    // unpack the output into C++ forms
    outputUnPackaged unpacked = extractOutput (output, this->hasDelay, this->hasWeight);

    // transfer the unpacked output to the local storage location for connections
    if (this->connection_target != NULL) {

        DBG() << "pythonscript_connection::generate_connections: setting src/dst popn names in connection_target";
        this->connection_target->setSrcName (this->srcPop->name);
        this->connection_target->setDstName (this->dstPop->name);

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
        DBG() << "connection_target is null";
        this->connections = unpacked.connections;
        (*this->conns) = unpacked.connections;
    }

    // transfer the unpacked output to the local storage location for weights
    this->weights = unpacked.weights;

    // if we get to the end then that's good enough
    this->scriptValidates = true;
    this->setUnchanged(true);

    DBG() << "Returning";
}

connection * pythonscript_connection::newFromExisting()
{

    // create a new, identical, python connection

    pythonscript_connection * c = new pythonscript_connection();

    c->isAList = this->isList();
    c->selfConnections = this->selfConnections;
    c->rotation = this->rotation;
    //c->setUnchanged(!this->hasChanged);
    c->scriptValidates = this->scriptValidates;
    c->hasWeight = this->hasWeight;
    c->hasDelay = this->hasDelay;
    c->connection_target = this->connection_target;
    c->scriptName = this->scriptName;
    c->scriptText = this->scriptText;
    c->srcPop = this->srcPop;
    c->dstPop = this->dstPop;

    // copy script pars
    c->parNames = this->parNames;
    c->parValues = this->parValues;
    c->parPos = this->parPos;

    return c;

}

bool pythonscript_connection::isList()
{
    return this->isAList;
}
