/***************************************************************************
**                                                                        **
**  This file is part of SpineCreator, an easy to use, GUI for            **
**  describing spiking neural network models.                             **
**  Copyright (C) 2013 Alex Cope, Paul Richmond                           **
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

#include "rootdata.h"
#include "connectionlistdialog.h"
#include "experiment.h"
#include "undocommands.h"
//#include "QtSvg/QtSvg"
#include "savenetworkimage_dialog.h"
#include "mainwindow.h"
//#include "stringify.h"
#include "projectobject.h"
#include "systemmodel.h"
#include "nineml_rootcomponentitem.h"

/*
 Alex Cope 2012
 The rootData class stores the 9ML description of the system -
 what populations there are, what projections, and the parameters of all these.
 It is a central Synapse for all changes to the system, and any changes made will
 propagate through here.
*/


QString stringify(int val) {
    stringstream ss (stringstream::in | stringstream::out);
    ss << float(val);
    QString returnVal = ss.str().c_str();
    return returnVal;
}

rootData::rootData(QObject *parent) :
    QObject(parent)
{
    QSettings settings;
    //QString localName = settings.value("versioning/localName", QHostInfo::localHostName()).toString();
    //version.setVersion(0,0,1,localName);

    this->cursor.x = 0;
    this->cursor.y = 0;
    this->largestIndex = 0;
    this->catalogUnsorted.push_back((new NineMLComponent()));
    this->catalogUnsorted[0]->name = "none";
    this->catalogUnsorted[0]->type = "moo";
    this->catalogNrn.push_back((new NineMLComponent()));
    this->catalogNrn[0]->name = "none";
    this->catalogNrn[0]->type = "neuron_body";
    this->catalogWU.push_back((new NineMLComponent()));
    this->catalogWU[0]->name = "none";
    this->catalogWU[0]->type = "weight_update";
    this->catalogPS.push_back((new NineMLComponent()));
    this->catalogPS[0]->name = "none";
    this->catalogPS[0]->type = "postsynapse";
    this->catalogLayout.push_back((new NineMLLayout()));
    this->catalogLayout[0]->name = "none";
    //this->catalogConn.push_back((new connection()));
    //this->catalogConn[0]->name = "none";
    this->selectionMoved = false;

    this->selChange = false;

    if (!popImage.load( ":/icons/objects/icons/nrn.png" )) std::cerr << "warn" << endl;

    // update version and name
    setCaption(settings.value("model/model_name", "err").toString());

    clipboardCData = NULL;

    projectActions = NULL;

    //connect(undoStack, SIGNAL(indexChanged(int)), this, SLOT(undoOrRedoPerformed(int)));

}

void rootData::redrawViews() {

    // redraw the network
    reDrawAll();
    redrawGLview();

    // redraw the component file list
    main->viewCL.fileList->disconnect();
    main->addComponentsToFileList();
    connect(main->viewCL.fileList, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), main, SLOT(fileListItemChanged(QListWidgetItem*,QListWidgetItem*)));
    // clear edited component
    if (main->viewCL.root != NULL) {
        delete main->viewCL.root;
        main->viewCL.root = NULL;
    }

    // redraw experiments
    main->viewELhandler->redraw();

    // redraw viz
    main->viewVZ.OpenGLWidget->refreshAll();
    // clear away old stuff
    main->viewVZ.currObject = (systemObject *)0;
    main->viewVZhandler->clearAll();
    main->viewVZ.OpenGLWidget->clear();
    // configure TreeView
    if (!(main->viewVZ.sysModel == NULL))
        delete main->viewVZ.sysModel;
    main->viewVZ.sysModel = new systemmodel(this);
    main->viewVZ.treeView->setModel(main->viewVZ.sysModel);
    // connect for function
    connect(main->viewVZ.treeView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), main->viewVZhandler, SLOT(selectionChanged(QItemSelection,QItemSelection)));
    connect(main->viewVZ.treeView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), main->viewVZ.OpenGLWidget, SLOT(selectionChanged(QItemSelection,QItemSelection)));
    connect(main->viewVZ.sysModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), main->viewVZ.OpenGLWidget, SLOT(sysSelectionChanged(QModelIndex,QModelIndex)));
    main->viewVZhandler->redrawHeaders(0);
    main->viewVZ.OpenGLWidget->sysSelectionChanged(QModelIndex(), QModelIndex());

}

void rootData::selectProject(QAction * action) {

    currProject->deselect_project(this);

    projects[action->property("number").toInt()]->select_project(this);

    redrawViews();

    // clear loaded component
    /*if (main->viewCL.root != NULL){
        delete main->viewCL.root;
        main->viewCL.root = NULL;
    }*/
}

void rootData::replaceComponent(NineMLComponent * oldComp, NineMLComponent * newComp) {

    for (uint p = 0; p < system.size(); ++p) {

        population * pop = system[p];

        // replace references
        if (pop->neuronType->component == oldComp) {
            // has the type changed?
            if (newComp->type != "neuron_body") {
                pop->neuronType->migrateComponent(catalogNrn[0]);
                for (uint i = 0; i < experiments.size(); ++i) {
                    experiment * currExpt = experiments[i];
                    currExpt->updateChanges(pop->neuronType);
                }
            } else {
                pop->neuronType->migrateComponent(newComp);
                for (uint i = 0; i < experiments.size(); ++i) {
                    experiment * currExpt = experiments[i];
                    currExpt->updateChanges(pop->neuronType);
                }
            }
        }

        for (uint pr = 0; pr < pop->projections.size(); ++pr) {

            projection * proj = pop->projections[pr];

            for (uint sy = 0; sy < proj->synapses.size(); ++sy) {

                synapse * syn = proj->synapses[sy];

                // replace references
                if (syn->weightUpdateType->component == oldComp) {
                    // has the type changed?
                    if (newComp->type != "weight_update") {
                        syn->weightUpdateType->migrateComponent(catalogWU[0]);
                        for (uint i = 0; i < experiments.size(); ++i) {
                            experiment * currExpt = experiments[i];
                            currExpt->updateChanges(syn->weightUpdateType);
                        }
                    } else {
                        syn->weightUpdateType->migrateComponent(newComp);
                        for (uint i = 0; i < experiments.size(); ++i) {
                            experiment * currExpt = experiments[i];
                            currExpt->updateChanges(syn->weightUpdateType);
                        }
                    }
                }

                // replace references
                if (syn->postsynapseType->component == oldComp) {
                    // has the type changed?
                    if (newComp->type != "postsynapse") {
                        syn->postsynapseType->migrateComponent(catalogPS[0]);
                        for (uint i = 0; i < experiments.size(); ++i) {
                            experiment * currExpt = experiments[i];
                            currExpt->updateChanges(syn->postsynapseType);
                        }
                    } else {
                        syn->postsynapseType->migrateComponent(newComp);
                        for (uint i = 0; i < experiments.size(); ++i) {
                            experiment * currExpt = experiments[i];
                            currExpt->updateChanges(syn->postsynapseType);
                        }
                    }
                }

            }

        }

        // also fix experiments with bad pointers to PORTS and PARS & COMPONENTS
        for (uint i = 0; i < experiments.size(); ++i) {
            experiment * currExpt = experiments[i];
            currExpt->purgeBadPointer(oldComp, newComp);
        }

    }

    // clear undo
    this->currProject->undoStack->clear();
}


// centralised function for finding if a component is in the model
bool rootData::isComponentInUse(NineMLComponent * oldComp) {

    for (uint p = 0; p < system.size(); ++p) {

        population * pop = system[p];

        // replace references
        if (pop->neuronType->component == oldComp)
            return true;

        for (uint pr = 0; pr < pop->projections.size(); ++pr) {

            projection * proj = pop->projections[pr];

            for (uint sy = 0; sy < proj->synapses.size(); ++sy) {

                synapse * syn = proj->synapses[sy];

                // replace references
                if (syn->weightUpdateType->component == oldComp)
                    return true;

                // replace references
                if (syn->postsynapseType->component == oldComp)
                    return true;

            }

        }

        // also fix experiments with bad pointers:


    }

    // component not found
    return false;
}

// centralised function for finding if a component is in the model
bool rootData::removeComponent(NineMLComponent * oldComp) {

    if (isComponentInUse(oldComp))
        return false;

    if (oldComp == NULL)
        return true;

    vector < NineMLComponent * > * curr_lib;
    if (oldComp->type == "neuron_body")
        curr_lib = &this->catalogNrn;
    if (oldComp->type == "weight_update")
        curr_lib = &this->catalogWU;
    if (oldComp->type == "postsynapse")
        curr_lib = &this->catalogPS;

    for (uint i = 0; i < curr_lib->size(); ++i) {
        if ((*curr_lib)[i] == oldComp) {
            curr_lib->erase(curr_lib->begin()+i);
            delete oldComp;
            return true;
        }
    }

    // component not found
    return false;
}

void rootData::updateStatusBar(QString in_string, int time) {
    emit statusBarUpdate(in_string, time);
}

void rootData::reDrawPanel() {
    emit updatePanel(this);
}

void rootData::callRedrawGLview() {
    emit redrawGLview();
}

void rootData::saveImage(QString fileName) {

    if (!fileName.isEmpty()) {

        saveNetworkImageDialog svImDiag(this, fileName);
        svImDiag.exec();
    }

    return;
}

void rootData::reDrawAll() {
    // update panel - we don't always want to do this as it loses focus from widgets
    emit updatePanel(this);
}

void rootData::reDrawAll(QPainter *painter, float GLscale, float viewX, float viewY, int width, int height, drawStyle style)
{

    if (style == standardDrawStyle) {
        for (uint i = 0; i < selList.size(); ++i) {
            if (selList[i]->type == populationObject) {
                population * pop = (population *) selList[i];

                float left = ((pop->getLeft()+viewX)*GLscale+float(width))/2;
                float right = ((pop->getRight()+viewX)*GLscale+float(width))/2;
                float top = ((-pop->getTop()+viewY)*GLscale+float(height))/2;
                float bottom = ((-pop->getBottom()+viewY)*GLscale+float(height))/2;

                if (pop->isSpikeSource) {
                    for (unsigned int i = 5; i > 1; --i) {
                        QPen pen(QColor(0,0,0,50/i));
                        pen.setWidthF(float(i*2));
                        painter->setPen(pen);
                        painter->drawEllipse(QPointF((right+left)/2.0, (top+bottom)/2.0),0.5*GLscale/2.0,0.5*GLscale/2.0);
                    }

                } else {

                    for (unsigned int i = 5; i > 1; --i) {
                        QPen pen(QColor(0,0,0,50/i));
                        pen.setWidthF(float(i*2));
                        painter->setPen(pen);
                        QRectF rectangle(left, top, right-left, bottom-top);
                        painter->drawRect(rectangle);
                    }
                }

            }
        }
    }

    // populations
    for (unsigned int i = 0; i < this->system.size(); ++i) {

        this->system[i]->draw(painter, GLscale, viewX, viewY, width, height, this->popImage, style);

    }
    for (unsigned int i = 0; i < this->system.size(); ++i) {

        this->system[i]->drawSynapses(painter, GLscale, viewX, viewY, width, height, style);

    }
    for (unsigned int i = 0; i < this->system.size(); ++i) {

        QPen pen(QColor(100,0,0,100));
        pen.setWidthF(float(1));
        painter->setPen(pen);
        this->system[i]->drawInputs(painter, GLscale, viewX, viewY, width, height, style);

    }

    // selected object
    if (style == standardDrawStyle) {
        for (uint i = 0; i < selList.size(); ++i) {

            if (selList[i]->type == projectionObject) {

                projection * col = (projection *) selList[i];

                for (unsigned int i = 5; i > 1; --i) {
                    QPen pen(QColor(0,0,0,30/i));
                    pen.setWidthF(float(i*2));
                    painter->setPen(pen);
                    col->draw(painter, GLscale, viewX, viewY, width, height, this->popImage, standardDrawStyle);
                    // only draw handles if we aren't using multiple selection
                    if (selList.size() == 1)
                        col->drawHandles(painter, GLscale, viewX, viewY, width, height);
                }

            }

            if (selList[i]->type == inputObject) {

                genericInput * input = (genericInput *) selList[i];

                for (unsigned int i = 5; i > 1; --i) {
                    QPen pen(QColor(0,0,0,30/i));
                    pen.setWidthF(float(i*2));
                    painter->setPen(pen);
                    input->draw(painter, GLscale, viewX, viewY, width, height, this->popImage, standardDrawStyle);
                    // only draw handles if we aren't using multiple selection
                    if (selList.size() == 1)
                        input->drawHandles(painter, GLscale, viewX, viewY, width, height);
                }

            }


        }
    }

    // cursor
    painter->setPen(QColor(0,0,0,255));

    float x = ((this->cursor.x+viewX)*GLscale+float(width))/2;
    float y = ((-this->cursor.y+viewY)*GLscale+float(height))/2;

    QPointF point(x,y);
    painter->drawEllipse(point,10,10);
    painter->drawLine(QLineF(x, y-14.0f, x, y+14.0f));
    painter->drawLine(QLineF(x-14.0f, y, x+14.0f, y));

    // update positions
    for (unsigned int i = 0; i < this->system.size(); ++i) {

        this->system[i]->animate();

    }

    // draw dragselect if present
    if (fabs(this->dragSelection.width()) > 0.001) {
        // convert to screen co-ords
        QRectF dragSelectionScreen;
        // top left conversion
        QPointF point = dragSelection.topLeft();
        point.setX(((point.x()+viewX)*GLscale+width)/2);
        point.setY(((-point.y()+viewY)*GLscale+height)/2);
        dragSelectionScreen.setTopLeft(point);
        // bottom right conversion
        point = dragSelection.bottomRight();
        point.setX(((point.x()+viewX)*GLscale+width)/2);
        point.setY(((-point.y()+viewY)*GLscale+height)/2);
        dragSelectionScreen.setBottomRight(point);
        // make sure the rect is not inverted (i.e. non-negative with & height) so draws correctly
        dragSelectionScreen = dragSelectionScreen.normalized();
        // set draw style
        painter->save();
        painter->setPen(QColor(0,255,0,200));
        painter->setBrush(QColor(0,255,0,50));
        // draw
        painter->drawRect(dragSelectionScreen);
        painter->restore();
    }

}

void destroyDom(QDomNode &node) {
    QDomNodeList childList = node.childNodes();
    QDomNode child;
    for (int i = 0; i < childList.count(); ++i) {
        child = childList.item(i);
        destroyDom(child);
        child.clear();
    }
}

void rootData::import_csv(QString fileName)
{
    // open file and load NineML
    if (fileName.size() == 0) {
        // user cancelled - do nothing
        return;
    }

    this->catalogConn.push_back(new csv_connection(fileName));

    // force redraw of data panel
    updatePanel(this);
}

/*
bool rootData::import_project_xml(QString fileName)
{
    // open file and load SpineML
    if (fileName.size() == 0) {
        // user cancelled - do nothing
        return false;
    }

    // clear the selList
    this->selList.clear();

    // and the experiments
    for (unsigned int i = 0; i < this->experiments.size(); ++i) {
        delete this->experiments[i];
    }
    this->experiments.clear();

    // remove old model components
    this->loadedComponents.clear();


    // get dir path in QT form

    QDir lib_dir(fileName);

    //////////////// LOAD SYSTEM

    QFile file( lib_dir.absoluteFilePath("model.xml") );
    if( !file.open( QIODevice::ReadOnly ) ) {
        QMessageBox msgBox;
        msgBox.setText("Could not open the model file - is it named model.xml?");
        msgBox.exec();
        //delete doc;
        return false;}
    if( !doc.setContent( &file ) )
    {
        QMessageBox msgBox;
        msgBox.setText("Could not load model file XML - is the selected file correctly formed XML?");
        msgBox.exec();
        file.close();
        //delete doc;
        return false;
    }

    // store the file name
    QSettings settings;
    settings.setValue("files/currentFileName", fileName);

    // we have loaded the XML file - discard the file handle
    file.close();

    // confirm root tag is correct
    QDomElement root = doc.documentElement();
    if( root.tagName() != "LL:SpineML" ) {
        QMessageBox msgBox;
        msgBox.setText("File is not valid SpineML Low Level Network Layer description");
        msgBox.exec();
        //delete doc;
        return false;
    }

    // get the model name
    settings.setValue("model/model_name", root.toElement().attribute("name", "Unnamed model"));

    bool no_meta_data = false;
    QDomDocument *meta = new QDomDocument;
    QString metaFileName = lib_dir.absoluteFilePath("metaData.xml");
    QFile fileMeta( metaFileName );
    if( !fileMeta.open( QIODevice::ReadOnly ) ) {
        no_meta_data = true;
        }
    if( !meta->setContent( &fileMeta ) && !no_meta_data)
    {
        QMessageBox msgBox;
        msgBox.setText("Could not load metaData XML - is the selected file correctly formed XML?");
        msgBox.exec();
        fileMeta.close();
        delete meta;
        return false;
    }
    // we have loaded the XML file - discard the file handle
    fileMeta.close();

    // confirm root tag is correct
    root = meta->documentElement();
    if( root.tagName() != "modelMetaData"  && !no_meta_data) {
        QMessageBox msgBox;
        msgBox.setText("File is not valid model metadata description");
        msgBox.exec();
        delete meta;
        return false;
    }

    //////////////// LOAD POPULATIONS
    QDomNode n = doc.documentElement().firstChild();
    while( !n.isNull() )
    {

        QDomElement e = n.toElement();
        if( e.tagName() == "LL:Population" )
        {
            // add population from population xml:
            this->system.push_back(new population(e, &doc, meta, this));

            // check for errors:
            QSettings settings;
            int num_errs = settings.beginReadArray("errors");
            settings.endArray();

            if (num_errs != 0) {

                QString errors = "";

                // list errors
                settings.beginReadArray("errors");
                for (int j = 1; j < num_errs; ++j) {
                    settings.setArrayIndex(j);
                    errors = errors + settings.value("errorText", "").toString();
                    errors = errors + "<br/>";
                }
                settings.endArray();

                // clear errors
                settings.remove("errors");
                settings.remove("warnings");

                // display errors
                QMessageBox msgBox;
                msgBox.setText("<P><b>Errors found loading model</b></P>" + errors);
                msgBox.setIcon(QMessageBox::Critical);
                msgBox.setTextFormat(Qt::RichText);
                msgBox.exec();

                // no dice - give up!
                return false;

            }

            this->system.back()->tag = this->getIndex();

        }
        n = n.nextSibling();
    }

    //////////////// LOAD PROJECTIONS
    n = doc.documentElement().firstChild();
    int counter = 0;
    while( !n.isNull() )
    {

        QDomElement e = n.toElement();
        if( e.tagName() == "LL:Population" )
        {
            // with all the populations added, add the projections and join them up:
            this->system[counter]->load_projections_from_xml(e, &doc, meta, this);

            // check for errors:
            QSettings settings;
            int num_errs = settings.beginReadArray("errors");
            settings.endArray();

            if (num_errs != 0) {

                QString errors = "";

                // list errors
                settings.beginReadArray("errors");
                for (int j = 1; j < num_errs; ++j) {
                    settings.setArrayIndex(j);
                    errors = errors + settings.value("errorText", "").toString();
                    errors = errors + "<br/>";
                }
                settings.endArray();

                // clear errors
                settings.remove("errors");

                // display errors
                QMessageBox msgBox;
                msgBox.setText("<P><b>Errors found loading model</b></P>" + errors);
                msgBox.setIcon(QMessageBox::Critical);
                msgBox.setTextFormat(Qt::RichText);
                msgBox.exec();

                // no dice - give up!
                return false;

            }

            counter++;

        }
        n = n.nextSibling();
    }

    ///////////////// LOAD INPUTS
    counter = 0;
    n = doc.documentElement().firstChild();
    while( !n.isNull() )
    {

        QDomElement e = n.toElement();
        if( e.tagName() == "LL:Population" )
        {
            // add inputs
            this->system[counter]->read_inputs_from_xml(e, meta, this);

            int projCount = 0;
            QDomNodeList n2 = n.toElement().elementsByTagName("LL:Projection");
            for (uint i = 0; i < (uint) n2.size(); ++i)
            {
                QDomElement e = n2.item(i).toElement();
                this->system[counter]->projections[projCount]->read_inputs_from_xml(e, meta, this);
                ++projCount;
            }
            ++counter;
        }
        n = n.nextSibling();

    }

    delete meta;

    //////////////// LOAD EXPERIMENTS:

    QDir lib_dir_expt(fileName);

    QStringList filters;
    filters << "experiment*.xml";

    lib_dir_expt.setNameFilters(filters);

    QStringList files;

    files = lib_dir_expt.entryList();

    for (unsigned int i = 0; i < (uint) files.count(); ++i) {

        QFile file( lib_dir_expt.filePath(files[i]) );
        if( !file.open( QIODevice::ReadOnly ) ) {
            continue;}

        QXmlStreamReader * reader = new QXmlStreamReader;

        reader->setDevice( &file );

        // load experiment:

        experiment * newExperiment = new experiment;
        newExperiment->readXML(reader, this);

        // check for errors:
        QSettings settings;
        int num_errs = settings.beginReadArray("errors");
        settings.endArray();

        if (num_errs == 0)
            this->experiments.push_back(newExperiment);
        else {
            // list errors
            QString errors;
            settings.beginReadArray("errors");
            for (int j = 1; j < num_errs; ++j) {
                settings.setArrayIndex(j);
                errors = errors + settings.value("errorText", "").toString();
                errors = errors + "<br/>";
            }
            settings.endArray();
            // clear errors
            settings.remove("errors");

            // display errors
            QMessageBox msgBox;
            msgBox.setText("<P><b>Errors found loading experiment file '" + files[i] + "</b></P><br/>" + errors);
            msgBox.exec();
        }

        // we have loaded the XML file - discard the file handle
        file.close();

        // kill off the XML reader
        delete reader;

    }

    if (no_meta_data) {

        // add some meta data to make the system work

        // place populations
        for (uint i = 0; i < this->system.size(); ++i) {
            population * p = this->system[i];
            p->x = i*2.0; p->targx = i*2.0;
            p->y = i*2.0; p->targy = i*2.0;
            p->size = 1.0;
            p->aspect_ratio = 5.0/3.0;
            p->setupBounds();
        }

        // make projection curves
        for (uint i = 0; i < this->system.size(); ++i) {
            population * p = this->system[i];
            for (uint j = 0; j < p->projections.size(); ++j) {
                projection * pr = p->projections[j];
                pr->add_curves();
            }
        }

    }

    updatePanel(this);

    // print out:

    return true;
}

void rootData::import_component_xml(QStringList fileNames)
{

    QString errors;

    // open file and load SpineML
    if (fileNames.size() == 0) {
        // user cancelled - do nothing
        return;
    }

    for (unsigned int fileNum = 0; fileNum < (uint) fileNames.size(); ++fileNum) {

        NineMLRootObject * newComponent = import_component_xml_single(fileNames[fileNum]);
        // add loaded components to 'temp' path, (?) unless they exist already in the library (?)

        if (newComponent != NULL) {
            if (newComponent->path == "lib") {
                // do nothing
            }
            else
                newComponent->path = "temp";

            // store path so we can save it later
            newComponent->filePath = fileNames[fileNum];
        }

        // check for errors:
        QSettings settings;
        int num_errs = settings.beginReadArray("errors");
        settings.endArray();

        if (num_errs != 0) {

            errors = errors + "<b>In file " + fileNames[fileNum] + ":</b><br/>";

            // list errors
            settings.beginReadArray("errors");
            for (int j = 1; j < num_errs; ++j) {
                settings.setArrayIndex(j);
                errors = errors + settings.value("errorText", "").toString();
                errors = errors + "<br/>";
            }
            settings.endArray();

            // clear errors
            settings.remove("errors");

        }

    }

    if (!errors.isEmpty()) {
        // display errors
        QMessageBox msgBox;
        msgBox.setText("<P><b>Error loading some components</b></P>" + errors);
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setTextFormat(Qt::RichText);
        msgBox.exec();
    }
}

NineMLRootObject * rootData::import_component_xml_single(QString fileName)
{

    NineMLRootObject * returnPtr = NULL;

    QDomDocument *doc = new QDomDocument( "SpineML" );

    QFile file( fileName );
    if( !file.open( QIODevice::ReadOnly ) ) {
        QSettings settings;
        int num_errs = settings.beginReadArray("errors");
        settings.endArray();
        settings.beginWriteArray("errors");
            settings.setArrayIndex(num_errs + 1);
            settings.setValue("errorText", "Error opening file");
        settings.endArray();
        return NULL;}
    if( !doc->setContent( &file ) )
    {
        QSettings settings;
        int num_errs = settings.beginReadArray("errors");
        settings.endArray();
        settings.beginWriteArray("errors");
            settings.setArrayIndex(num_errs + 1);
            settings.setValue("errorText",  "Could not load XML - is the selected file correctly formed XML?");
        settings.endArray();
        file.close();
        return NULL;
    }
    // we have loaded the XML file - discard the file handle
    file.close();
    // confirm root tag is correct
    QDomElement root = doc->documentElement();
    if( root.tagName() != "SpineML" ) {
        QSettings settings;
        int num_errs = settings.beginReadArray("errors");
        settings.endArray();
        settings.beginWriteArray("errors");
            settings.setArrayIndex(num_errs + 1);
            settings.setValue("errorText",  "File is not valid SpineML");
        settings.endArray();
        return NULL;
    }

    // if a componentclass
    QDomElement classType = root.firstChildElement();
    if (classType.tagName() == "ComponentClass") {

        // HANDLE SPINEML COMPONENTS ////////////////

        // create a new AL class instance and populate it from the data
        NineMLComponent *tempALobject = new NineMLComponent();

        tempALobject->load(doc);

        // check for errors:
        QSettings settings;
        int num_errs = settings.beginReadArray("errors");
        settings.endArray();

        if (num_errs != 0) {
            return NULL;
        }

        versionNumber testVersion;
        testVersion.fromFileString(fileName);

        // get lib to add component to
        vector < NineMLComponent * > * curr_lib;
        if (tempALobject->type == "neuron_body")
            curr_lib = &catalogNrn;
        else if (tempALobject->type == "weight_update")
            curr_lib = &catalogWU;
        else if (tempALobject->type == "postsynapse")
            curr_lib = &catalogPS;
        else
            curr_lib = &catalogUnsorted;


        // get md5 checksum for component
        tempDoc.clear();
        tempALobject->write(&tempDoc);
        QByteArray outputNew = tempDoc.toByteArray();
        QString md5StringNew = QString(QCryptographicHash::hash((outputNew),QCryptographicHash::Md5).toHex());

        // get a unique name if not an existing component
        bool name_updated = true;
        while (name_updated) {
            name_updated = false;
            for (uint i = 0; i < curr_lib->size(); ++i) {
                // get md5 checksum for component
                tempDoc.clear();
                (*curr_lib)[i]->write(&tempDoc);
                QByteArray outputOld = tempDoc.toByteArray();
                QString md5StringOld = QString(QCryptographicHash::hash((outputOld),QCryptographicHash::Md5).toHex());
                if ((*curr_lib)[i]->name == tempALobject->name && (*curr_lib)[i]->path == tempALobject->path && md5StringNew != md5StringOld) {
                    // same name but different component
                    QString text = "";
                    bool ok = false;
                    while (text == "" && !ok) {
                        text = QInputDialog::getText((QWidget *)this->parent(), tr("Name conflict"),
                                                         tr("New component name:"), QLineEdit::Normal,
                                                         tempALobject->name, &ok);
                    }
                    name_updated = true;
                    tempALobject->name = text.replace('_', ' ');
                }
                if (md5StringNew == md5StringOld) {
                    // existing component - return that component
                    delete doc;
                    return (NineMLRootObject *) (*curr_lib)[i];
                }
            }
        }

        // add to the correct catalog
        if (tempALobject->type == "neuron_body") {
            this->catalogNrn.push_back(tempALobject);
            this->catalogNrn.back()->version.fromFileString(fileName);
        }
        else if (tempALobject->type == "weight_update") {
            this->catalogWU.push_back(tempALobject);
            this->catalogWU.back()->version.fromFileString(fileName);
        }
        else if (tempALobject->type == "postsynapse") {
            this->catalogPS.push_back(tempALobject);
            this->catalogPS.back()->version.fromFileString(fileName);
        }
        else {//if (tempALobject->type == "unsorted") {
            this->catalogUnsorted.push_back(tempALobject);
            this->catalogUnsorted.back()->version.fromFileString(fileName);
        }

        returnPtr = tempALobject;

    } else if (classType.tagName() == "LayoutClass") {

        // HANDLE LAYOUTS ////////////////////

        // create a new AL class instance and populate it from the data
        NineMLLayout *tempALobject = new NineMLLayout();

        tempALobject->load(doc);

        // check for errors:
        QSettings settings;
        int num_errs = settings.beginReadArray("errors");
        settings.endArray();

        if (num_errs != 0) {
            return NULL;
        }

        for (unsigned int i = 0; i < this->catalogLayout.size(); ++i) {
            if (this->catalogLayout[i]->name.compare(tempALobject->name) == 0) {
                delete doc;
                delete tempALobject;
                return (NineMLRootObject *) this->catalogLayout[i];
            }
        }

        returnPtr = tempALobject;

    } else {
        QSettings settings;
        int num_errs = settings.beginReadArray("errors");
        settings.endArray();
        settings.beginWriteArray("errors");
            settings.setArrayIndex(num_errs + 1);
            settings.setValue("errorText",  "File is unknown SpineML type");
        settings.endArray();
    }

    // kill off the DOM document
    delete doc;

    return returnPtr;
}

void rootData::import_layout_xml(QStringList fileNames)
{

    for (unsigned int fileNum = 0; fileNum < (uint) fileNames.size(); ++fileNum) {

        QString fileName = fileNames[fileNum];


        // open file and load NineML
        if (fileName.size() == 0) {
            // user cancelled - do nothing
            return;
        }
        QDomDocument *doc = new QDomDocument( "SpineML" );
        QFile file( fileName );
        if( !file.open( QIODevice::ReadOnly ) ) {
            QMessageBox msgBox;
            msgBox.setIcon(QMessageBox::Critical);
            msgBox.setText("Could not open the selected file " + fileName);
            msgBox.exec();
            return;}
        if( !doc->setContent( &file ) )
        {
            QMessageBox msgBox;
            msgBox.setIcon(QMessageBox::Critical);
            msgBox.setText("Could not load XML - is the selected file correctly formed XML?");
            msgBox.exec();
            file.close();
            return;
        }
        // we have loaded the XML file - discard the file handle
        file.close();
        // confirm root tag is correct
        QDomElement root = doc->documentElement();
        if( root.tagName() != "SpineML" ) {
            QMessageBox msgBox;
            msgBox.setIcon(QMessageBox::Critical);
            msgBox.setText("File is not valid NineML");
            msgBox.exec();
            return;
        }
        // create a new AL class instance and populate it from the data
        NineMLLayout *tempALobject = new NineMLLayout();

        tempALobject->load(doc);

        // find if component exists already in catalogs, in which case rename
        bool unique = false;
        QString baseName = tempALobject->name;
        int num = 1;
        while(!unique) {
            unique = true;
            for (unsigned int i = 0; i < this->catalogLayout.size(); ++i) {
                if (this->catalogLayout[i]->name.compare(tempALobject->name) == 0) {
                    unique = false;
                }
            }
            if (!unique) {
                tempALobject->name = baseName + "_" + QString::number(float(num));
                num++;
            }
        }

        // add to the correct catalog
        this->catalogLayout.push_back(tempALobject);

        // kill off the DOM document
        delete doc;

    }
    main->viewNL.layout->updatePanel(this);
}



void rootData::export_component_xml(QString fileName, NineMLComponent * component)
{
    // get ready to write
    if (fileName.size() == 0) {
        // user cancelled - do nothing
        return;
    }

    // if no extension then append a .xml
    if (!fileName.contains("."))
        fileName.append(".xml");

    QFile file( fileName );
    if (!file.open( QIODevice::WriteOnly)) {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setText("Error creating file - is there sufficient disk space?");
        msgBox.exec();
        return;
    }

    // get the 9ML description
    QDomDocument * doc = new QDomDocument( "SpineML" );
    component->write(doc);

    // write out to file
    QTextStream tsFromFile( &file );
    tsFromFile << doc->toString();
    tsFromFile.flush();
    file.close();

    // store filename in component is not a library component
    if (component->path != "lib")
        component->filePath = fileName;

    // kill off the DOM document
    delete doc;
}

void rootData::export_layout_xml(QString fileName, NineMLLayout * component)
{

    // get ready to write
    if (fileName.size() == 0) {
        // user cancelled - do nothing
        return;
    }

    QFile file( fileName );
    if (!file.open( QIODevice::WriteOnly)) {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setText("Error creating file - is there sufficient disk space?");
        msgBox.exec();
        return;
    }

    // get the 9ML description
    QDomDocument * doc = new QDomDocument( "SpineML" );
    component->write(doc);

    // write out to file
    QTextStream tsFromFile( &file );
    tsFromFile << doc->toString();

    // kill off the DOM document
    delete doc;
}

bool rootData::export_model_for_simulator(QString fileName)
{
    QSettings settings;

    settings.setValue("export_for_simulation", QString::number((float) true));

    // fetch current experiment
    experiment * currentExperiment = NULL;

    // find currentExperiment
    for (uint i = 0; i < this->experiments.size(); ++i) {
        if (this->experiments[i]->selected) {currentExperiment = this->experiments[i]; break;}
    }

    if (currentExperiment == NULL) return false;

    // get ready to write
    if (fileName.size() == 0) {
        // user cancelled - do nothing
        settings.remove("export_for_simulation");
        return false;
    }

    QString model_file = QDir::toNativeSeparators(fileName + "/model.xml");
    QString meta_file = QDir::toNativeSeparators(fileName + "/metaData.xml");

    QFile fileModel( model_file );
    if (!fileModel.open( QIODevice::WriteOnly)) {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setText("Error creating file - is there sufficient disk space?");
        msgBox.exec();
        settings.remove("export_for_simulation");
        return false;
    }

    QFile fileMeta( meta_file );
    if (!fileMeta.open( QIODevice::WriteOnly)) {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setText("Error creating file - is there sufficient disk space?");
        msgBox.exec();
        settings.remove("export_for_simulation");
        return false;
    }

    // use stream writing for model UL file
    QXmlStreamWriter xmlOut;

    xmlOut.setDevice(&(fileModel));

    this->get_model_xml(xmlOut);


    // check for errors:
    int num_errs = settings.beginReadArray("errors");
    settings.endArray();
    int num_warn = settings.beginReadArray("warnings");
    settings.endArray();

    if (num_errs != 0 || num_warn != 0) {

        QString errors = "";

        // list errors
        settings.beginReadArray("errors");
        for (int j = 1; j < num_errs; ++j) {
            settings.setArrayIndex(j);
            errors = errors + settings.value("errorText", "").toString();
            errors = errors + "<br/>";
        }
        settings.endArray();

        settings.beginReadArray("warnings");
        for (int j = 1; j < num_warn; ++j) {
            settings.setArrayIndex(j);
            errors = errors + settings.value("warnText", "").toString();
            errors = errors + "<br/>";
        }
        settings.endArray();

        // clear errors
        settings.remove("errors");
        settings.remove("warnings");

        // display errors
        QMessageBox msgBox;
        msgBox.setText("<P><b>Errors found exporting model for simulation</b></P>" + errors);
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setTextFormat(Qt::RichText);
        msgBox.exec();

        // no dice - give up!
        settings.remove("export_for_simulation");
        return false;

    }


   // get the 9ML description
     QDomDocument * meta = new QDomDocument( "MetaData" );
    this->get_model_meta_xml(*meta);

    QTextStream tsFromFileMeta( &fileMeta );
    tsFromFileMeta << meta->toString();

    //delete doc;
    delete meta;

    // output the experiment


    QString exptFile = QDir::toNativeSeparators(fileName + "/experiment.xml");

    QFile file ( exptFile );
    if (!file.open( QIODevice::WriteOnly)) {
        QMessageBox msgBox;
        msgBox.setText("Error creating file - is there sufficient disk space?");
        msgBox.exec();
        settings.remove("export_for_simulation");
        return false;
    }

    // use stream writer
    QXmlStreamWriter * xmlOutExpt = new QXmlStreamWriter;
    xmlOutExpt->setDevice(&(file));

    currentExperiment->writeXML(xmlOutExpt, this);

    delete xmlOutExpt;



    // also output the components:
    vector < NineMLComponent * > outList;
    for (uint i = 0; i < this->system.size(); ++i) {
        // NEURONS
        NineMLComponent * comp = this->system[i]->neuronType->component;
        bool exists = false;
        for (uint j = 0; j < outList.size(); ++j) {
            if (outList[j]->name == comp->name)
                exists = true;
        }
        if (!exists)
            outList.push_back(comp);

        for (uint p = 0; p < system[i]->projections.size(); ++p) {
            for (uint c = 0; c < system[i]->projections[p]->synapses.size(); ++c) {
                // SYNAPSES
                comp = system[i]->projections[p]->synapses[c]->weightUpdateType->component;
                exists = false;
                for (uint j = 0; j < outList.size(); ++j) {
                    if (outList[j]->name == comp->name)
                        exists = true;
                }
                if (!exists)
                    outList.push_back(comp);

                // PSPS
                comp = system[i]->projections[p]->synapses[c]->postsynapseType->component;
                exists = false;
                for (uint j = 0; j < outList.size(); ++j) {
                    if (outList[j]->name == comp->name)
                        exists = true;
                }
                if (!exists)
                    outList.push_back(comp);
            }
        }
    }
    for (uint j = 0; j < outList.size(); ++j) {

        QString outName = outList[j]->name;
        outName.replace( " ", "_" );
        QString comp_file = QDir::toNativeSeparators(fileName + "/" + outName + outList[j]->version.toFileString() + ".xml");

        //cerr << fileName.toStdString() << "\n";

        QFile fileComp( comp_file );
        if (!fileComp.open( QIODevice::WriteOnly)) {
            QMessageBox msgBox;
            msgBox.setText("Error creating file - is there sufficient disk space?");
            msgBox.exec();
            settings.remove("export_for_simulation");
            return false;
        }

        QDomDocument * doc = new QDomDocument( "SpineML" );
        outList[j]->write(doc);

        // write out to file
        QTextStream tsFromFileComp( &fileComp );
        tsFromFileComp << doc->toString();

        // kill off the DOM document
        delete doc;
    }

    settings.remove("export_for_simulation");

    // success!
    return true;
}

void rootData::export_project_xml(QString fileName)
{
    // get ready to write
    if (fileName.size() == 0) {
        // user cancelled - do nothing
        return;
    }

    QString model_file = QDir::toNativeSeparators(fileName + "/model.xml");
    QString meta_file = QDir::toNativeSeparators(fileName + "/metaData.xml");

    // get files to remove from directory
    QDir model_dir(QDir::toNativeSeparators(fileName));
    model_dir.setNameFilters(QStringList()<< "*.bin");
    QStringList files = model_dir.entryList(QDir::Files);
    for (int i = 0; i < files.size(); ++i) {
        model_dir.remove(files[i]);
        // and remove from version control
        if (version->isModelUnderVersion())
            version->removeFromVersion(files[i]);
    }
    model_dir.setNameFilters(QStringList() << "*.xml");
    files = model_dir.entryList(QDir::Files);

    QFile fileModel( model_file );
    if (!fileModel.open( QIODevice::WriteOnly)) {
        QMessageBox msgBox;
        msgBox.setText("Error creating file - is there sufficient disk space?");
        msgBox.exec();
        return;
    }

    QFile fileMeta( meta_file );
    if (!fileMeta.open( QIODevice::WriteOnly)) {
        QMessageBox msgBox;
        msgBox.setText("Error creating file - is there sufficient disk space?");
        msgBox.exec();
        return;
    }

    // store the new file name
    QSettings settings;
    settings.setValue("files/currentFileName", fileName);

    // check for version control
    version->setupVersion();

    // use stream writing for model UL file
    QXmlStreamWriter xmlOut;

    xmlOut.setAutoFormatting(true);

    xmlOut.setDevice(&(fileModel));

    this->get_model_xml(xmlOut);

    // add to version control
    if (version->isModelUnderVersion())
        version->addToVersion(fileModel.fileName());
    files.removeAt(files.indexOf("model.xml"));

    QDomDocument * meta = new QDomDocument( "MetaData" );
    this->get_model_meta_xml(*meta);

    QTextStream tsFromFileMeta( &fileMeta );
    tsFromFileMeta << meta->toString();

    // add to version control
    if (version->isModelUnderVersion())
        version->addToVersion(fileMeta.fileName());
    files.removeAt(files.indexOf("metaData.xml"));

    delete meta;

    // output the experiments:
    for (uint i = 0; i < experiments.size(); ++i) {

        QString exptFile = QDir::toNativeSeparators(fileName + "/experiment" + QString::number(float(i)) + ".xml");

        QFile file ( exptFile );
        if (!file.open( QIODevice::WriteOnly)) {
            QMessageBox msgBox;
            msgBox.setText("Error creating file - is there sufficient disk space?");
            msgBox.exec();
            return;
        }

        // use stream writer
        QXmlStreamWriter * xmlOutExpt = new QXmlStreamWriter;
        xmlOutExpt->setDevice(&(file));

        experiments[i]->writeXML(xmlOutExpt, this);

        delete xmlOutExpt;

        // add to version control
        QStringList fileBits = file.fileName().split(QDir::separator());
        if (version->isModelUnderVersion())
            version->addToVersion(fileBits.back());
        files.removeAt(files.indexOf(fileBits.back()));
    }

    // also output the components:
    vector < NineMLComponent * > outList;
    vector < NineMLLayout * > layoutOutList;
    for (uint i = 0; i < this->system.size(); ++i) {
        // NEURONS
        NineMLComponent * comp = this->system[i]->neuronType->component;
        bool exists = false;
        for (uint j = 0; j < outList.size(); ++j) {
            if (outList[j]->name == comp->name)
                exists = true;
        }
        if (!exists)
            outList.push_back(comp);

        NineMLLayout * lay = this->system[i]->layoutType->component;
        exists = false;
        for (uint j = 0; j < layoutOutList.size(); ++j) {
            if (layoutOutList[j]->name == lay->name)
                exists = true;
        }
        if (!exists)
            layoutOutList.push_back(lay);

        for (uint p = 0; p < system[i]->projections.size(); ++p) {
            for (uint c = 0; c < system[i]->projections[p]->synapses.size(); ++c) {
                // SYNAPSES
                comp = system[i]->projections[p]->synapses[c]->weightUpdateType->component;
                exists = false;
                for (uint j = 0; j < outList.size(); ++j) {
                    if (outList[j]->name == comp->name)
                        exists = true;
                }
                if (!exists)
                    outList.push_back(comp);

                // PSPS
                comp = system[i]->projections[p]->synapses[c]->postsynapseType->component;
                exists = false;
                for (uint j = 0; j < outList.size(); ++j) {
                    if (outList[j]->name == comp->name)
                        exists = true;
                }
                if (!exists)
                    outList.push_back(comp);
            }
        }
    }
    for (uint j = 0; j < outList.size(); ++j) {

        QString outName = outList[j]->name;
        outName.replace( " ", "_" );
        QString comp_file = QDir::toNativeSeparators(fileName + "/" + outName + outList[j]->version.toFileString() + ".xml");

        //cerr << fileName.toStdString() << "\n";

        QFile fileComp( comp_file );
        if (!fileComp.open( QIODevice::WriteOnly)) {
            QMessageBox msgBox;
            msgBox.setText("Error creating file - is there sufficient disk space?");
            msgBox.exec();
            return;
        }

        // clear errors
        settings.remove("errors");
        settings.remove("warnings");

        QDomDocument * doc = new QDomDocument( "SpineML" );
        outList[j]->write(doc);

        // write out to file
        QTextStream tsFromFileComp( &fileComp );
        tsFromFileComp << doc->toString();

        // kill off the DOM document
        delete doc;


        // add to version control
        QStringList fileBits = fileComp.fileName().split(QDir::separator());
        if (version->isModelUnderVersion())
            version->addToVersion(fileBits.back());
        files.removeAt(files.indexOf(fileBits.back()));

    }

    for (uint j = 0; j < layoutOutList.size(); ++j) {

        QString outName = layoutOutList[j]->name;
        outName.replace( " ", "_" );
        QString comp_file = QDir::toNativeSeparators(fileName + "/" + outName + ".xml");

        //cerr << fileName.toStdString() << "\n";

        QFile fileComp( comp_file );
        if (!fileComp.open( QIODevice::WriteOnly)) {
            QMessageBox msgBox;
            msgBox.setText("Error creating file - is there sufficient disk space?");
            msgBox.exec();
            return;
        }

        QDomDocument * doc = new QDomDocument( "SpineML" );
        layoutOutList[j]->write(doc);

        // write out to file
        QTextStream tsFromFileComp( &fileComp );
        tsFromFileComp << doc->toString();

        // kill off the DOM document
        delete doc;


        // add to version control
        QStringList fileBits = fileComp.fileName().split(QDir::separator());
        if (version->isModelUnderVersion())
            version->addToVersion(fileBits.back());
        files.removeAt(files.indexOf(fileBits.back()));

    }

    // remove any files still in list
    for (int i = 0; i < files.size(); ++i) {
        model_dir.remove(files[i]);
        if (version->isModelUnderVersion())
            version->removeFromVersion(files[i]);
    }

    // add binary files to version
    model_dir.setNameFilters(QStringList()<< "*.bin");
    files = model_dir.entryList(QDir::Files);
    for (int i = 0; i < files.size(); ++i) {
        // add to version control
        if (version->isModelUnderVersion())
            version->addToVersion(files[i]);
    }


    // all done - set the undo stack to clean
    currProject->undoStack->setClean();
    // clear the astrix
    emit setWindowTitle();
}

void rootData::get_model_meta_xml(QDomDocument &meta) {

   // create the root of the file:
   QDomElement root = meta.createElement( "modelMetaData" );
   meta.appendChild(root);

   // iterate through the populations and get the xml
   for (unsigned int i = 0; i < system.size(); ++i) {
       system[i]->write_model_meta_xml(meta, root);
   }

}

void rootData::get_model_xml(QXmlStreamWriter &xmlOut) {


    QSettings settings;

    // create the root of the file:
    //xmlOut.writeProcessingInstruction("xml", "version=\"1.0\"");
    xmlOut.writeStartDocument();
    xmlOut.writeStartElement("LL:SpineML");
    xmlOut.writeAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
    xmlOut.writeAttribute("xmlns", "http://www.shef.ac.uk/SpineMLNetworkLayer");
    xmlOut.writeAttribute("xmlns:LL", "http://www.shef.ac.uk/SpineMLLowLevelNetworkLayer");
    xmlOut.writeAttribute("xsi:schemaLocation", "http://www.shef.ac.uk/SpineMLLowLevelNetworkLayer SpineMLLowLevelNetworkLayer.xsd http://www.shef.ac.uk/SpineMLNetworkLayer SpineMLNetworkLayer.xsd");
    xmlOut.writeAttribute("name", settings.value("model/model_name", "err").toString());

    // create a node for each population with the variables set
    for (unsigned int pop = 0; pop < this->system.size(); ++pop) {

        //// WE NEED TO HAVE A PROPER MODEL NAME!
        this->system[pop]->write_population_xml(xmlOut);

    }

    xmlOut.writeEndDocument();

    // check for errors:
    int num_errs = settings.beginReadArray("errors");
    settings.endArray();

    if (num_errs != 0) {
        // list errors
        QString errors;
        settings.beginReadArray("errors");
        for (int j = 1; j < num_errs; ++j) {
            settings.setArrayIndex(j);
            errors = errors + settings.value("errorText", "").toString();
            errors = errors + "<br/>";
        }
        settings.endArray();
        // clear errors
        settings.remove("errors");
        settings.remove("warnings");

        // display errors
        QMessageBox msgBox;
        msgBox.setText("<P><b>Errors found writing model file:</b></P><br/>" + errors);
        msgBox.exec();
    }

}*/

void rootData::rightClickByGL(float xGL, float yGL, float GLscale)
{

    // insert new point into projection
    if (selList.size() == 1) {
        if (this->selList[0]->type == projectionObject || this->selList[0]->type == inputObject) {
            // try to delete control point, if that fails then add one!
            projection * proj = (projection *) selList[0];
            if (!proj->deleteControlPoint(xGL, yGL, GLscale)) {
                proj->insertControlPoint(xGL, yGL, GLscale);}

        }
    }
    // log the co-ordinates for drag select
    this->dragListStart = QPointF(xGL, yGL);

}

void rootData::dragSelect(float xGL, float yGL)
{

    bool addSelection = (QApplication::keyboardModifiers() & Qt::ShiftModifier);

    // setup the QRect for the selection
    this->dragSelection = QRectF(this->dragListStart, QPointF(xGL, yGL));

    vector <systemObject *> oldSel = selList;

    // clear slection list if shift not held
    if (!addSelection)
        selList.clear();

    // add selected objects to list
    for (uint i = 0; i < this->system.size(); ++i) {
        population * pop = system[i];

        if (dragSelection.contains(pop->x, pop->y)) {
            // if not already selected
            bool alreadySelected = false;
            for (uint s = 0; s < this->selList.size(); ++s) {
                if (selList[s] == pop)
                    alreadySelected = true;
            }
            if (!alreadySelected)
                selList.push_back(pop);
        }

        for (uint j = 0; j < pop->neuronType->inputs.size(); ++j) {
            genericInput * in = pop->neuronType->inputs[j];

            if (in->curves.size() > 0) {
                if (dragSelection.contains(in->start) && dragSelection.contains(in->curves.back().end)) {
                    // if not already selected
                    bool alreadySelected = false;
                    for (uint s = 0; s < this->selList.size(); ++s) {
                        if (selList[s] == in)
                            alreadySelected = true;
                    }
                    if (!alreadySelected)
                        selList.push_back(in);
                }
            }

        }

        for (uint j = 0; j < pop->projections.size(); ++j) {
            projection * proj = pop->projections[j];

            if (proj->curves.size() > 0) {
                if (dragSelection.contains(proj->start) && dragSelection.contains(proj->curves.back().end)) {
                    // if not already selected
                    bool alreadySelected = false;
                    for (uint s = 0; s < this->selList.size(); ++s) {
                        if (selList[s] == proj)
                            alreadySelected = true;
                    }
                    if (!alreadySelected)
                        selList.push_back(proj);
                }
            }

            for (uint pt = 0; pt < proj->synapses.size(); ++pt) {

                synapse * projT = proj->synapses[pt];


                // select generic inputs for weightupdate
                for (uint c = 0; c < projT->weightUpdateType->inputs.size(); ++c) {

                    genericInput * in = projT->weightUpdateType->inputs[c];

                    if (in->curves.size() > 0) {
                        if (dragSelection.contains(in->start) && dragSelection.contains(in->curves.back().end)) {
                            // if not already selected
                            bool alreadySelected = false;
                            for (uint s = 0; s < this->selList.size(); ++s) {
                                if (selList[s] == in)
                                    alreadySelected = true;
                            }
                            if (!alreadySelected)
                                selList.push_back(in);
                        }
                    }
                }

                // select generic inputs for postsynapse
                for (uint c = 0; c < projT->postsynapseType->inputs.size(); ++c) {

                    genericInput * in = projT->postsynapseType->inputs[c];

                    if (in->curves.size() > 0) {
                        if (dragSelection.contains(in->start) && dragSelection.contains(in->curves.back().end)) {
                            // if not already selected
                            bool alreadySelected = false;
                            for (uint s = 0; s < this->selList.size(); ++s) {
                                if (selList[s] == in)
                                    alreadySelected = true;
                            }
                            if (!alreadySelected)
                                selList.push_back(in);
                        }
                    }
                }

            }

        }

    }

    // check if the selection has changed to prevent unecessary redrawing
    bool selectionChanged = false;
    if (oldSel.size() == selList.size()) {
        for (uint i = 0; i < oldSel.size(); ++i) {
            if (oldSel[i] != selList[i])
                selectionChanged = true;
        }
    } else
        selectionChanged = true;

    if (selectionChanged)
        reDrawAll();

}

void rootData::endDragSelection() {
    this->dragSelection = QRect(-1,-1,0,0);
}

void rootData::selectCoordMouseUp (float xGL, float yGL, float GLscale)
{

    // are we adding or reselecting?
    bool addSelection = (QApplication::keyboardModifiers() & Qt::ShiftModifier);

    // we don't select on mouseup for drag events
    if (selectionMoved) return;

    vector < genericInput * > allInputs;

    if (addSelection) {

        for (unsigned int i = 0; i < this->system.size(); ++i) {

            // select if under the cursor - no two objects should overlap!
            if (this->system[i]->is_clicked(xGL, yGL, GLscale))
            {
                this->cursor.x = -100000;
                this->cursor.y = -100000;
                this->selChange = true;

                bool removed = false;


                // if in list, then remove
                for (uint j = 0; j < selList.size(); ++j) {
                    if (this->system[i]->getName() == selList[j]->getName()) {
                        selList.erase(selList.begin()+j, selList.begin()+j+1);
                        removed = true;
                    }
                }
                // remove projections with only one end population selected
                for (int j = 0; j < (int) selList.size(); ++j) {
                    if (selList[j]->type == projectionObject) {
                        bool targIn = false;
                        bool destIn = false;
                        for (uint k = 0; k < selList.size(); ++k) {
                            if (((projection *) selList[j])->source->tag == selList[k]->tag) targIn = true;
                            if (((projection *) selList[j])->destination->tag == selList[k]->tag) destIn = true;
                        }
                        if (!(targIn && destIn))
                        {
                            // if there are any projections which no longer have both src and dest selected, remove them
                            selList.erase(selList.begin()+j, selList.begin()+j+1);
                            j = j - 1;
                        }

                    }
                }

                // if not removed, then add
                if (!removed) this->selList.push_back(this->system[i]);

                emit updatePanel(this);

                // selection complete, move on
                return;
            }

            // add inputs to list for population
            for (unsigned int in = 0; in < this->system[i]->neuronType->inputs.size(); ++in)
                allInputs.push_back(this->system[i]->neuronType->inputs[in]);

            // check projections
            for (unsigned int j = 0; j < this->system[i]->projections.size(); ++j) {

                // find if an edge of the projection is hit
                if (this->system[i]->projections[j]->is_clicked(xGL, yGL, GLscale)) {

                    this->cursor.x = -100000;
                    this->cursor.y = -100000;
                    this->selChange = true;

                    bool removed = false;


                    // if in list, then remove
                    for (uint k = 0; k < selList.size(); ++k) {
                        if (this->system[i]->projections[j]->getName() == selList[k]->getName()) {
                            selList.erase(selList.begin()+k, selList.begin()+k+1);
                            removed = true;
                        }
                    }


                    // if not removed, and src and dest are in the list, then add
                    if (!removed) {

                        bool targIn = false;
                        bool destIn = false;
                        for (uint k = 0; k < selList.size(); ++k) {
                            if (this->system[i]->projections[j]->source->tag == selList[k]->tag) targIn = true;
                            if (this->system[i]->projections[j]->destination->tag == selList[k]->tag) destIn = true;
                        }
                        if (targIn && destIn)
                            this->selList.push_back(this->system[i]->projections[j]);

                    }

                    emit updatePanel(this);

                    // selection complete, move on
                    return;
                }

                // add inputs to list for projection
                for (unsigned int pt = 0; pt < this->system[i]->projections[j]->synapses.size(); ++pt) {
                    for (unsigned int in = 0; in < this->system[i]->projections[j]->synapses[pt]->weightUpdateType->inputs.size(); ++in)
                        allInputs.push_back(this->system[i]->projections[j]->synapses[pt]->weightUpdateType->inputs[in]);
                    for (unsigned int in = 0; in < this->system[i]->projections[j]->synapses[pt]->postsynapseType->inputs.size(); ++in)
                        allInputs.push_back(this->system[i]->projections[j]->synapses[pt]->postsynapseType->inputs[in]);
                }
            }



        }
        // for all generic inputs
        for (unsigned int i = 0; i < allInputs.size(); ++i) {

            // find if an edge of the projection is hit
            if (allInputs[i]->is_clicked(xGL, yGL, GLscale)) {

                this->cursor.x = -100000;
                this->cursor.y = -100000;
                this->selChange = true;

                bool removed = false;

                // if in list, then remove
                for (uint k = 0; k < selList.size(); ++k) {
                    if (allInputs[i] == selList[k]) {
                        selList.erase(selList.begin()+k, selList.begin()+k+1);
                        removed = true;
                    }
                }


                // if not removed, and src and dest are in the list, then add
                if (!removed) {

                    bool targIn = false;
                    bool destIn = false;
                    for (uint k = 0; k < selList.size(); ++k) {
                        if (allInputs[i]->source == selList[k]) targIn = true;
                        if (allInputs[i]->destination == selList[k]) destIn = true;
                    }
                    if (targIn && destIn)
                        this->selList.push_back(allInputs[i]);

                }

                emit updatePanel(this);

                // selection complete, move on
                return;
            }

        }

    }

}

void rootData::itemWasMoved(float xGL, float yGL, float GLscale)
{
    if (!selList.empty()) {
        // We have a pointer to the moved item. Check its type to see what to do with it.
        if (selList[0]->type == populationObject) {
            this->populationMoved();
        } // else do nothing.
    }
}

void rootData::populationMoved()
{
    // We don't need to record that the population moved here - the
    // mouseMove events already handle that (allowing the population
    // widget to be re-drawn as it's moved. However, what we DO need
    // to do is to record that the population moved in the undo stack.
    population * pop = this->currSelPopulation();
    if (pop == NULL) {
        return;
    }

    // oldPos needs to be the one that was stored at the START of the
    // move. That's stored in this->lastSelectionPosition by
    // rootData::selectCoord.
    pair<float, float> newPos = make_pair(pop->x, pop->y);
    currProject->undoStack->push(new movePopulation(this, pop, this->lastSelectionPosition, newPos));
}

void rootData::selectCoord(float xGL, float yGL, float GLscale)
{

    // are we adding or reselecting? NB: Move this into the glwidget class (todo: seb)
    bool addSelection = (QApplication::keyboardModifiers() & Qt::ShiftModifier);

    if (!addSelection) {

        // Record the position of the selection.
        this->lastSelectionPosition.first = xGL;
        this->lastSelectionPosition.second = yGL;

        // prioritise the handles of a selected projection:
        if (this->selList.size() == 1) {
            if (this->selList[0]->type == projectionObject || this->selList[0]->type == inputObject) {
                if (((projection *) selList[0])->selectControlPoint(xGL, yGL, GLscale)) {
                    return;
                }
            }
        }

        for (unsigned int i = 0; i < this->system.size(); ++i) {


            // check pop inputs:
            for (unsigned int j = 0; j < this->system[i]->neuronType->inputs.size(); ++j) {

                // find if an edge of the input is hit
                if (this->system[i]->neuronType->inputs[j]->is_clicked(xGL, yGL, GLscale)) {

                    this->cursor.x = -100000;
                    this->cursor.y = -100000;
                    this->selChange = true;

                    this->selList.clear();

                    //  add to selection list
                    this->selList.push_back(this->system[i]->neuronType->inputs[j]);


                    emit updatePanel(this);
                    for (uint i = 0; i < selList.size(); ++i) {
                        // register locations relative to cursor:
                        selList[i]->setRelativeLocation(xGL, yGL);
                    }

                    // selection complete, move on
                    return;
                }
            }

            // check projections
            for (unsigned int j = 0; j < this->system[i]->projections.size(); ++j) {

                // find if an edge of the projection is hit
                if (this->system[i]->projections[j]->is_clicked(xGL, yGL, GLscale)) {

                    this->cursor.x = -100000;
                    this->cursor.y = -100000;
                    this->selChange = true;

                    this->selList.clear();

                    //  add to selection list
                    this->selList.push_back(this->system[i]->projections[j]);


                    emit updatePanel(this);
                    for (uint i = 0; i < selList.size(); ++i) {
                        // register locations relative to cursor:
                        selList[i]->setRelativeLocation(xGL, yGL);
                    }

                    // selection complete, move on
                    return;
                }

                // check proj inputs:
                for (unsigned int k = 0; k < this->system[i]->projections[j]->synapses.size(); ++k) {

                    synapse * col = this->system[i]->projections[j]->synapses[k];

                    for (unsigned int l = 0; l < col->weightUpdateType->inputs.size(); ++l) {

                        // find if an edge of the input is hit
                        if (col->weightUpdateType->inputs[l]->is_clicked(xGL, yGL, GLscale)) {

                            this->cursor.x = -100000;
                            this->cursor.y = -100000;
                            this->selChange = true;

                            this->selList.clear();

                            //  add to selection list
                            this->selList.push_back(col->weightUpdateType->inputs[l]);


                            emit updatePanel(this);
                            for (uint i = 0; i < selList.size(); ++i) {
                                // register locations relative to cursor:
                                selList[i]->setRelativeLocation(xGL, yGL);
                            }

                            // selection complete, move on
                            return;
                        }
                    }

                    for (unsigned int l = 0; l < col->postsynapseType->inputs.size(); ++l) {

                        // find if an edge of the input is hit
                        if (col->postsynapseType->inputs[l]->is_clicked(xGL, yGL, GLscale)) {

                            this->cursor.x = -100000;
                            this->cursor.y = -100000;
                            this->selChange = true;

                            this->selList.clear();

                            //  add to selection list
                            this->selList.push_back(col->postsynapseType->inputs[l]);


                            emit updatePanel(this);
                            for (uint i = 0; i < selList.size(); ++i) {
                                // register locations relative to cursor:
                                selList[i]->setRelativeLocation(xGL, yGL);
                            }

                            // selection complete, move on
                            return;
                        }
                    }
                }

            }

            // select if under the cursor - no two objects should overlap!
            if (this->system[i]->is_clicked(xGL, yGL, GLscale))
            {

                this->cursor.x = -100000;
                this->cursor.y = -100000;
                this->selChange = true;

                this->selList.clear();

                //  add to selection list
                this->selList.push_back(this->system[i]);

                emit updatePanel(this);
                for (uint i = 0; i < selList.size(); ++i) {
                    // register locations relative to cursor:
                    selList[i]->setRelativeLocation(xGL, yGL);
                }

                // selection complete, move on
                return;
            }

        }

        // if nothing new selected
        if (!addSelection)
        {
            cursor.x = xGL;
            cursor.y = yGL;
            this->selList.clear();
        }
        emit updatePanel(this);
    } else {

        for (uint i = 0; i < selList.size(); ++i) {
            // register locations relative to cursor:
            selList[i]->setRelativeLocation(xGL, yGL);
        }
        selectionMoved = false;
    }

}

QColor rootData::getColor(QColor initCol) {

    // launch the QT colorpicker, and get a colour - if cancel is pressed then the output fails .isValid(), and we return the input instead
    QColor out = QColorDialog::getColor(initCol);
    if (out.isValid()) {
        return out;
    } else {
        return initCol;
    }

}

void rootData::deleteCurrentSelection()
{

    if (selList.size() > 1) {
        currProject->undoStack->push(new delSelection(this, selList));
    } else if (selList.size() == 1) {
        // seperate out:
        if (selList[0]->type == populationObject)
            currProject->undoStack->push(new delPopulation(this,(population *) selList[0]));
        if (selList[0]->type == projectionObject)
            currProject->undoStack->push(new delProjection(this, (projection *) selList[0]));
        if (selList[0]->type == inputObject)
            currProject->undoStack->push(new delInput(this,(genericInput *) selList[0]));

    }

}

void rootData::addPopulation()
{
    if (this->cursor.x != -100000) {
        QString popName = "Population";
        // go and get a decent number:
        popName = getUniquePopName(popName);

        population * pop = new population(cursor.x, cursor.y, 1.0f,  5.0/3.0f, popName);
        pop->tag = this->getIndex();
        pop->layoutType = new NineMLLayoutData(this->catalogLayout[0]);
        pop->neuronType = new NineMLComponentData(this->catalogNrn[0]);
        pop->neuronType->owner = pop;

        currProject->undoStack->push(new addPopulationCmd(this, pop));

        emit updatePanel(this);
        emit redrawGLview();

    }
}

void rootData::addSpikeSource()
{
    if (this->cursor.x != -100000) {
        QString popName = "Spike Source";
        // go and get a decent number:
        popName = getUniquePopName(popName);

        population * pop = new population(cursor.x, cursor.y, 1.0f,  5.0/3.0f, popName);
        pop->tag = this->getIndex();
        pop->layoutType = new NineMLLayoutData(this->catalogLayout[0]);

        // make a spikes source
        pop->makeSpikeSource();

        currProject->undoStack->push(new addPopulationCmd(this, pop));

        emit updatePanel(this);
        emit redrawGLview();

    }
}

void rootData::addBezierOrProjection(float xGL, float yGL)
{
    if (this->selList.size() == 1) {
        if (this->selList[0]->type == projectionObject) {

                // shortcuts to the projection and population
            projection * proj = (projection *) this->selList[0];
                population * pop = proj->source;

                // find if we have hit a population:
                for (unsigned int i = 0; i < this->system.size(); ++i) {

                    // ignore spike sources
                    if (this->system[i]->isSpikeSource) continue;

                    QPainterPath box;
                    if (this->system[i]->addToPath(&box)->contains(QPointF(xGL,yGL))) {

                        // we have a collision, so fix up the connection and return:
                        population * dest = this->system[i];

                        // first check for an existing connection...
                        if (dest->connectsTo(pop)) {
                            // EXISTING CONNECTION - NOTIFY USER
                            emit statusBarUpdate(tr("Connection exists already!"), 2000);
                            return;
                        } else {
                            // new connection! finalise the connection...
                            dest->reverseProjections.push_back(proj);
                            proj->destination = dest;

                            // add to undo stack
                            currProject->undoStack->push(new addProjection(this, proj));

                            // redraw the parameters panel
                            emit updatePanel(this);

                            // tell gl viewport to stop tracking the mouse
                            emit finishDrawingSynapse();

                            return;

                        }

                    }

                }

                // no population hit, so add a curve:
                emit statusBarUpdate(tr("Adding new point"), 2000);
                // create new bezierCurve
                bezierCurve newCurve;
                proj->curves.push_back(newCurve);

            }
    }

}

void rootData::startAddBezier(float xGL, float yGL)
{
    if (this->selList.size() == 1) {
        if (this->selList[0]->type == populationObject) {

            population * pop = (population *) selList[0];

            emit statusBarUpdate(tr("Adding new connection"), 2000);
            // create new bezierCurve
            bezierCurve newCurve;

            // add the new projection (for now)
            pop->projections.push_back(new projection());
            pop->projections.back()->tag = getIndex();
            pop->projections.back()->source = pop;

            // select the new projection
            this->selList.clear();
            this->selList.push_back(pop->projections.back());

            pop->projections.back()->curves.push_back(newCurve);
        }

        if (this->selList[0]->type == projectionObject) {

            //shortcut to projection:
            projection * col = (projection *) selList[0];
            // and source pop
            population * pop = col->source;

            // setup the curve
            // add start where the line from pop to cursor hits the bounding box of the pop

            // if we are a self selection
            QPainterPath * tempPP = new QPainterPath;
            if (pop->addToPath(tempPP)->contains(QPointF(xGL, yGL))) {

                // self selection
                col->start = QPointF(pop->getRight(), pop->getBottom() + 0.3*pop->size);
                col->curves.back().end = QPointF(pop->getRight()- 0.3*pop->size, pop->getBottom());
                col->curves.back().C1 = QPointF(pop->getRight() + 1.0, pop->getBottom() + 0.3*pop->size);
                col->curves.back().C2 = QPointF(pop->getRight()- 0.3*pop->size, pop->getBottom() - 1.0);

            } else {

                // see if any of the other populations are under the mouse cursor
                int selPop = -1;
                for (unsigned int i = 0; i < this->system.size(); ++i) {
                    // ignore spike sources
                    if (this->system[i]->isSpikeSource) continue;
                    QPainterPath * tempPP = new QPainterPath;
                    if (this->system[i]->addToPath(tempPP)->contains(QPointF(xGL, yGL)))
                        selPop = i;
                    delete tempPP;
                }

                if (selPop != -1) {

                    population * dest = this->system[selPop];

                    if (col->curves.size() == 1) {
                        // if first curve then setup start point
                        // find where on the population box we should start the col
                        QPointF boxEdge = col->findBoxEdge(pop, xGL, yGL);
                        col->start = boxEdge;

                        // add end point
                        // find where on the population box we should start the col
                        boxEdge = col->findBoxEdge(dest, boxEdge.x(), boxEdge.y());
                        col->curves.back().end = boxEdge;

                    } else {

                        // add end point
                        // find where on the population box we should start the col
                        QPointF boxEdge = col->findBoxEdge(dest, col->curves[col->curves.size()-2].end.x(), col->curves[col->curves.size()-2].end.y());
                        col->curves.back().end = boxEdge;

                    }

                    // set up handles
                    col->setAutoHandles(pop, dest, QPointF(xGL,yGL));

                } else {

                    if (col->curves.size() == 1) {
                        // if first curve then setup start point
                        // find where on the population box we should start the col
                        QPointF boxEdge = col->findBoxEdge(pop, xGL, yGL);
                        col->start = boxEdge;}

                    // setup end point
                    col->curves.back().end = QPointF(xGL, yGL);

                    // set up handles
                    col->setAutoHandles(pop, pop, QPointF(xGL,yGL));

                }

            }
            delete tempPP;

        }
    }

}

void rootData::abortProjection() {

    // the new projection should be the only one in the selList, but may not be...
    if (selList[0]->type == projectionObject) {
        projection * proj = (projection *) this->selList[0];
        // remove from the selList so we don't break when it is deleted
        selList.clear();
        // remove references from other components
        proj->disconnect();
        // remove from the system
        proj->remove(this);
    }

}


void rootData::mouseMoveGL(float xGL, float yGL)
{
    selectionMoved = true;
    // qDebug() << "pos = " << float(xGL) << " " << float(yGL);

    if (this->selList.empty()) {
        // move viewpoint only, then return.
        GLWidget * source = (GLWidget *) sender();
        source->move(xGL+source->viewX-cursor.x,yGL-source->viewY-cursor.y);
        return;
    }

    // revised move code for multiple objects

    // if grid is on, snap to grid
    GLWidget * source = (GLWidget *) sender();
    if (source->gridSelect) {
        xGL = round(xGL/source->gridScale)*source->gridScale;
        yGL = round(yGL/source->gridScale)*source->gridScale;
    }

    if (selList.size() > 1) {
        for (uint i = 0; i < selList.size(); ++i) {
            selList[i]->move(xGL, yGL);
        }
    } else {

        // if only one thing

        if (this->selList[0]->type == populationObject)
        {
            bool collision = false;
            population * pop = (population *) selList[0];

            // avoid collisions
            for (unsigned int i = 0; i < system.size(); ++i) {
                if (system[i]->getName() != pop->getName()) {
                    if (system[i]->within_bounds(pop->leftBound(xGL)+0.01, pop->topBound(yGL)-0.01)) collision = true;
                    if (system[i]->within_bounds(pop->rightBound(xGL)-0.01, pop->topBound(yGL)-0.01)) collision = true;
                    if (system[i]->within_bounds(pop->leftBound(xGL)+0.01, pop->bottomBound(yGL)+0.01)) collision = true;
                    if (system[i]->within_bounds(pop->rightBound(xGL)-0.01, pop->bottomBound(yGL)+0.01)) collision = true;
                }
            }

            // snap to grid:
            if (source->gridSelect)
                selList[0]->relativeLocation = QPointF(0,0);

            if (!collision)
            {
                selList[0]->move(xGL, yGL);
            }
        }
        // if it is not a population...
        else if (this->selList[0]->type == projectionObject) {

            //int selEdge = this->selected.edge;
            ((projection*) selList[0])->moveSelectedControlPoint(xGL, yGL);

        }
        // if it is not a population...
        else if (this->selList[0]->type == inputObject) {

            //cerr << "moo\n";
            ((genericInput*) selList[0])->moveSelectedControlPoint(xGL, yGL);

        }
    }
}

void rootData::updatePortMap(QString var) {

    // update the variables to connect up the stages of the currently selected projection

    genericInput * ptr = (genericInput *) sender()->property("ptr").value<void *>();
    //QString type = sender()->property("type").toString();

    QStringList ports = var.split("->");
    // for safety
    if (ports.size()>1) {
        ptr->srcPort = ports.at(0);
        ptr->dstPort = ports.at(1);
    }

}

void rootData::updateType(int index) {

    // update the components of the currently selected object
    population * currSel;
    synapse * targSel;
    genericInput * inSel;

    systemObject * ptr = NULL;
    //get ptr
    if (selList.size() ==1)
        ptr = selList[0];

    if (ptr == NULL)
        return;

    // if projection we need the current synapse
    if (ptr->type == projectionObject) {
        projection * proj = (projection *) ptr;
        ptr = proj->synapses[proj->currTarg];
    }

    // get type of change
    QString type = sender()->property("type").toString();

    // event says we need to update the selected object accordingly, so we'll do that first:
    switch (ptr->type) {
        case populationObject:
            // if there is a mis-match between the type and the selected type then update
            currSel = (population *) ptr;

            if (type == "layout") {
                if (index >= 0) {
                    if (currSel->layoutType->component->name.compare(this->catalogLayout[index]->name) != 0) {
                        delete currSel->layoutType;
                        currSel->layoutType = new NineMLLayoutData(this->catalogLayout[index]);
                        this->reDrawAll();
                    }
                }
            }
            if (type == "neuron") {
                if (index >= 0) {
                    if (currSel->neuronType->component->name != this->catalogNrn[index]->name || \
                        currSel->neuronType->component->path != this->catalogNrn[index]->path) {
                        currProject->undoStack->push(new updateComponentType(this, currSel->neuronType, this->catalogNrn[index]));
                    }
                }
            }
            break;
        case synapseObject:
            targSel = (synapse *) ptr;
            if (type == "weight_update") {
                if (index >= 0) {
                    if (targSel->weightUpdateType->component->name != this->catalogWU[index]->name || \
                        targSel->weightUpdateType->component->path != this->catalogWU[index]->path) {
                        currProject->undoStack->push(new updateComponentType(this, targSel->weightUpdateType, this->catalogWU[index]));
                    }
                }
            }
            if (type == "postsynapse") {
                if (index >= 0) {
                    if (targSel->postsynapseType->component->name != this->catalogPS[index]->name || \
                        targSel->postsynapseType->component->path != this->catalogPS[index]->path) {
                        currProject->undoStack->push(new updateComponentType(this, targSel->postsynapseType, this->catalogPS[index]));
                    }
                }
            }
            if (type == "conn") {
                if (index >= 0) {
                    if (targSel->connectionType->type != index) {
                        currProject->undoStack->push(new changeConnection(this, ptr, index));
                    }
                }
            }
            break;
        case inputObject:
            inSel = (genericInput *) ptr;
            if (type == "input") {
                if (index >= 0) {
                    if (inSel->connectionType->type != index) {
                        currProject->undoStack->push(new changeConnection(this, ptr, index));
                    }
                }
            }
            break;
        default:
            cerr << "Something has gone badly wrong!";
            exit(-1);
            break;
    }

    // redraw GL view
    emit redrawGLview();
    //emit updatePanel(this);

    emit updatePanelView2("comboboxOSXfix");

}

void rootData::updatePanelView2Accessor() {
    emit updatePanelView2("");
}

void rootData::updatePar() {

    QString action = sender()->property("action").toString();

    // update the type of parameter
    if (action == "updateType") {
        ParameterData * par = (ParameterData *) sender()->property("ptr").value<void *>();
        QString newType = sender()->property("newType").toString();
        currProject->undoStack->push(new updateParType(this, par, newType));
    }
    // launch the list editor dialog
    if (action == "editList") {
        ParameterData * par = (ParameterData *) sender()->property("ptr").value<void *>();
        NineMLComponent * comp = (NineMLComponent *) sender()->property("ptrComp").value<void *>();
        valueListDialog * dialog  = new valueListDialog(par, comp);
        dialog->show();
    }

    if (action == "changeVal") {
        // Update the parameter value
        ParameterData * par = (ParameterData *) sender()->property("ptr").value<void *>();
        int index = sender()->property("valToChange").toInt();
        float value = ((QDoubleSpinBox *) sender())->value();
        // only add undo if value has changed
        if (value != par->value[index])
            currProject->undoStack->push(new updateParUndo(this, par, index, value));
    }

    if (action == "changeConnProb") {
        // Update the parameter value
        //qDebug() << "change par";
        fixedProb_connection * conn = (fixedProb_connection *) sender()->property("ptr").value<void *>();
        float value = ((QDoubleSpinBox *) sender())->value();
        // only add undo if value has changed
        if (value != conn->p)
            currProject->undoStack->push(new updateConnProb(this, conn, value));
    }

    if (action == "changeConnEq") {
        // Update the parameter value
        //qDebug() << "change par";
        distanceBased_connection * conn = (distanceBased_connection *) sender()->property("ptr").value<void *>();
        QString newEq = ((QLineEdit *) sender())->text();
        // only add undo if value has changed
        if (newEq != conn->equation)
            currProject->undoStack->push(new updateConnEquation(this, conn, newEq));
    }

    if (action == "changeConnDelayEq") {
        // Update the parameter value
        //qDebug() << "change par";
        distanceBased_connection * conn = (distanceBased_connection *) sender()->property("ptr").value<void *>();
        QString newEq = ((QLineEdit *) sender())->text();
        // only add undo if value has changed
        if (newEq != conn->delayEquation)
            currProject->undoStack->push(new updateConnDelayEquation(this, conn, newEq));
    }

    if (action == "changeConnKerSize") {
        // Update the parameter value
        //qDebug() << "change par";
        kernel_connection * conn = (kernel_connection *) sender()->property("ptr").value<void *>();
        int kernel_size = ((QComboBox *) sender())->currentIndex() * 2 + 3;
        // only add undo if value has changed
            conn->setKernelSize(kernel_size);
            emit updatePanelView2("");
            //undoStack->push(new updateConnEquation(this, conn, newEq));
    }

    if (action == "changeConnKerScale") {
        // Update the parameter value
        //qDebug() << "change par";
        kernel_connection * conn = (kernel_connection *) sender()->property("ptr").value<void *>();
        float kernel_scale = ((QDoubleSpinBox *) sender())->value();
        // only add undo if value has changed
        conn->setKernelScale(kernel_scale);
            //undoStack->push(new updateConnEquation(this, conn, newEq));
    }

    if (action == "changeConnKernel") {
        // Update the parameter value
        //qDebug() << "change par";
        kernel_connection * conn = (kernel_connection *) sender()->property("ptr").value<void *>();
        int kernel_value = ((QDoubleSpinBox *) sender())->value();
        int i = sender()->property("i").toInt();
        int j = sender()->property("j").toInt();
        // only add undo if value has changed
            conn->setKernel(i,j,kernel_value);
            //undoStack->push(new updateConnEquation(this, conn, newEq));
    }

    //updatePanel(this);
}

void rootData::updatePar(int value) {
    // Update the parameter value
    ParameterData * par = (ParameterData *) sender()->property("ptr").value<void *>();
    par->value[0] = value;

    switch (value) {
    case 0:
        par->value.resize(4,0);
        break;
    case 1:
        par->value.resize(4,0);
        par->value[2] = 1;
        par->value[3] = 123;
        break;
    case 2:
        par->value.resize(4,0);
        par->value[2] = 1;
        par->value[3] = 123;
        break;
    }

    // update panel
    updatePanel(this);
}

population* rootData::currSelPopulation()
{
    // get the currently selected population
    population* currSel = NULL;
    if (this->selList.size() == 1) {
        if (this->selList[0]->type == populationObject) {
            currSel = (population*) this->selList[0];
        }
    }
    return currSel;
}

void rootData::updateLayoutPar() {

    // get the currently selected population
    population * currSel = this->currSelPopulation();

    if (currSel == NULL)
        return;

    int type = sender()->property("type").toInt();
    switch (type) {
    case 0:
    {
        QDoubleSpinBox * source = (QDoubleSpinBox *) sender();
        NineMLLayoutData * layout = currSel->layoutType;
        if (layout->minimumDistance != source->value())
            currProject->undoStack->push(new updateLayoutMinDist(this,layout,source->value()));
        break;
    }
    case 1:
    {
        QSpinBox * source = (QSpinBox *) sender();
        NineMLLayoutData * layout = currSel->layoutType;
        if (layout->seed != source->value())
            currProject->undoStack->push(new updateLayoutSeed(this,layout,source->value()));
        break;
    }
    }


}

void rootData::setSize() {

    // get the currently selected population
    population * currSel = this->currSelPopulation();

    if (currSel == NULL)
        return;

    // get value
    int value = ((QSpinBox *) sender())->value();

    // only update if we have a change
    if (value != currSel->numNeurons)
        currProject->undoStack->push(new setSizeUndo(this, currSel, value));

    // redraw view
    //emit redrawGLview();
}

void rootData::setLoc3() {

    // get the currently selected population
    population * currSel = this->currSelPopulation();

    if (currSel == NULL)
        return;

    int index = sender()->property("type").toInt();
    int value = ((QSpinBox *) sender())->value();

    currProject->undoStack->push(new setLoc3Undo(this, currSel, index, value));
}

void rootData::renamePopulation() {

    // get the currently selected population
    population * currSel = this->currSelPopulation();

    if (currSel == NULL)
        return;

    // get the title label so we can update it with the new name
    QLabel * titleLabel = (QLabel *) sender()->property("ptrTitle").value<void *>();

    // get the rename box so we can get the new title (it isn't necessarily the sender)
    QLineEdit * renameBox = (QLineEdit *) sender()->property("ptrRename").value<void *>();

    QString newName = renameBox->text();
    // check name is unique
    QString finalName = this->getUniquePopName(newName);
    if (finalName.compare(newName) != 0) {
            QMessageBox msgBox;
            msgBox.setText("A population exists with that name, adding a number");
            msgBox.exec();
    }

    // update name (undo-able)
    if (finalName != currSel->name) {
        titleLabel->setText("<u><b>" + finalName + "</b></u>");
        currProject->undoStack->push(new updateTitle(this, currSel, finalName, currSel->name, titleLabel));
    }

    // redraw view
    emit redrawGLview();
}


QString rootData::getUniquePopName(QString newName) {

    // are we the selected pop, or not?
    int ind = 0;
    if (selList.size() == 0) {
        ind = -1;
    } else if (selList.size() == 1) {
        for (uint i = 0; i < system.size(); ++i) {
            if (system[i]->getName() == selList[0]->getName()) ind = i;
        }
    }

    // is the root name used? If not we can just pass it back
    for (unsigned int i = 0; i < this->system.size(); ++i) {
        if (this->system[i]->name.compare(newName) == 0 && (int) i != ind) {

            // name in use - find the lowest number we can add to make a unique name
            bool nameGood = false;
            int j = 0;
            while (!nameGood) {
                // assume name is good
                nameGood = true;
                ++j;
                QString testName = newName;
                testName.append(" " + stringify(j));

                // check name against populations
                for (unsigned int k = 0; k < this->system.size(); ++k) {
                    if (ind == -1) {
                        if (this->system[k]->name.compare(testName) == 0) {
                            nameGood = false;
                        }
                    } else {
                        if (this->system[k]->name.compare(testName) == 0 && system[i]->getName() != selList[0]->getName()) {
                            nameGood = false;
                        }
                    }
                }
            }
            // we have the name - we can exit
            newName.append(" " + stringify(j));
        }
    }
    return newName;
}



int rootData::getIndex() {
    ++(this->largestIndex);
    return this->largestIndex;
}

void rootData::changeSynapse() {

    QPushButton * source = (QPushButton *) sender();
    QString dir = source->property("direction").toString();

    // get the currently selected projection
    projection * currSel = NULL;
    if (this->selList.size() == 1)
        if (this->selList[0]->type == projectionObject)
            currSel = (projection *)  this->selList[0];

    if (currSel == NULL)
        return;

    // change the current Synapse of the projection accordingly

    int currTarg = currSel->currTarg;
    if (dir.compare("left") == 0) {
        if (currTarg == 0) {
            currTarg = currSel->synapses.size() - 1;
        } else {
            --currTarg;
        }
    } else if (dir.compare("right") == 0) {
        if (currTarg == (int) currSel->synapses.size() - 1) {
            currTarg = 0;
        } else {
            ++currTarg;
        }
    } else if (dir.compare("add") == 0) {

        // add the Synapse
        currProject->undoStack->push(new addSynapse(this, currSel));

    } else if (dir.compare("rem") == 0) {

        // remove the Synapse
        currProject->undoStack->push(new delSynapse(this, currSel, currSel->synapses[currTarg]));

    }
    currSel->currTarg = currTarg;

    // force complete redraw of the parameters panels;
    emit updatePanel(this);
    //emit updatePanelView2(currSel->getName());

}

void rootData::selectColour() {

    // get the currently selected population
    population * currSel = this->currSelPopulation();

    if (currSel == NULL)
        return;

    // change the current Synapse of the projection accordingly
    currSel->colour = this->getColor(currSel->colour);

    // redraw GL
    emit redrawGLview();

}

void rootData::setCurrConnectionModel(csv_connectionModel *connModel) {

    if (this->catalogConn.size() > 1) {
        connModel->setConnection((csv_connection *) this->catalogConn.back());
    }

}

void rootData::setCurrConnectionModelSig(csv_connectionModel *connModel) {

    this->setCurrConnectionModel(connModel);

}


void rootData::launchComponentSorter() {

    NineMLSortingDialog * dialog  = new NineMLSortingDialog(this);

    dialog->show();

}

void rootData::getNeuronLocationsSrc(vector<vector<loc> > *locations,vector <QColor> * cols, QString name) {

    vector <loc> tempLoc;
    cols->clear();

    if (name == "") {

        if (this->selList.size() == 1) {

            if (this->selList[0]->type == populationObject) {
                if (((population *) selList[0])->layoutType->component->name == "none") {

                    population * pop = (population *) selList[0];

                    for (uint i = 0; i < locations->size(); ++i) {
                        locations[i].clear();
                    }
                    locations->clear();

                    // linear layout by default:
                    for (unsigned int i = 0; i < (uint) pop->numNeurons; ++i) {

                        loc newLoc;
                        /*newLoc.x = i;
                        newLoc.y = 0;
                        newLoc.z = 0;
                        locations->push_back(newLoc);*/

                        //do a square:
                        newLoc.x = i%10;
                        newLoc.y = floor(float(i) / 10.0);
                        newLoc.z = 0;
                        tempLoc.push_back(newLoc);

                    }

                    locations->push_back(tempLoc);
                    cols->push_back(pop->colour);

                }

                else {

                    population * pop = (population *) selList[0];

                    locations->clear();
                    cols->push_back(pop->colour);

                    // generate the locations!
                    QString err = "";
                    locations->resize(1);
                    pop->layoutType->generateLayout(pop->numNeurons, &(locations->at(0)), err);
                    if (err != "") emit statusBarUpdate(err, 2000);
                    return;
                }
            }
        }
    }
    else
    {


        // find what has that name, and send back the details
        for (unsigned int ind = 0; ind < this->system.size(); ++ind) {
            if (this->system[ind]->name == name) {

                if (this->system[ind]->layoutType->component->name == "none") {

                    for (uint i = 0; i < locations->size(); ++i) {
                        locations[i].clear();
                    }
                    locations->clear();

                    // linear layout by default:
                    for (unsigned int i = 0; i < (uint) this->system[ind]->numNeurons; ++i) {

                        loc newLoc;

                        //do a square:
                        newLoc.x = i%10;
                        newLoc.y = floor(float(i) / 10.0);
                        newLoc.z = 0;
                        tempLoc.push_back(newLoc);

                    }
                    locations->push_back(tempLoc);
                    cols->push_back(this->system[ind]->colour);
                    return;
                }
                else
                {

                    for (uint i = 0; i < locations->size(); ++i) {
                        locations[i].clear();
                    }
                    locations->clear();
                    cols->push_back(this->system[ind]->colour);
                    // generate the locations!
                    QString err = "";
                    locations->resize(1);
                    this->system[ind]->layoutType->generateLayout(this->system[ind]->numNeurons, &(locations->at(0)), err);
                    if (err != "") emit statusBarUpdate(err, 2000);
                    return;

                }
            }
            for (unsigned int cInd = 0; cInd < this->system[ind]->projections.size(); ++cInd) {

                if (this->system[ind]->projections[cInd]->getName() == name) {

                    for (uint i = 0; i < locations->size(); ++i) {
                        locations[i].clear();
                    }
                    locations->clear();

                    // generate src and dst locations:
                    // SOURCE
                    QString err = "";
                    locations->resize(2);
                    if (this->system[ind]->projections[cInd]->source->layoutType->component->name != "none") {
                        this->system[ind]->projections[cInd]->source->layoutType->generateLayout(this->system[ind]->projections[cInd]->source->numNeurons, &(locations->at(0)), err);
                        cols->push_back(this->system[ind]->projections[cInd]->source->colour);
                    }
                    else {
                        // linear layout by default:
                        tempLoc.clear();
                        for (unsigned int i = 0; i < (uint) this->system[ind]->projections[cInd]->source->numNeurons; ++i) {

                            loc newLoc;

                            //do a square:
                            newLoc.x = i%10;
                            newLoc.y = floor(float(i) / 10.0);
                            newLoc.z = 0;
                            tempLoc.push_back(newLoc);

                        }
                        locations->at(0) = tempLoc;
                        cols->push_back(this->system[ind]->projections[cInd]->source->colour);
                    }
                    if (err != "") emit statusBarUpdate(err, 2000);

                    // DESTINATION
                    if (this->system[ind]->projections[cInd]->destination->layoutType->component->name != "none") {
                        this->system[ind]->projections[cInd]->destination->layoutType->generateLayout(this->system[ind]->projections[cInd]->destination->numNeurons, &(locations->at(1)), err);
                        cols->push_back(this->system[ind]->projections[cInd]->destination->colour);
                    }
                    else {
                        // linear layout by default:
                        tempLoc.clear();
                        for (unsigned int i = 0; i < (uint) this->system[ind]->projections[cInd]->destination->numNeurons; ++i) {

                            loc newLoc;

                            //do a square:
                            newLoc.x = i%10;
                            newLoc.y = floor(float(i) / 10.0);
                            newLoc.z = 0;
                            tempLoc.push_back(newLoc);

                        }
                        locations->at(1) = tempLoc;
                        cols->push_back(this->system[ind]->projections[cInd]->destination->colour);
                    }
                    if (err != "") emit statusBarUpdate(err, 2000);

                    return;

                }

            }
        }

    }

}

systemObject * rootData::getObjectFromName(QString name) {

    systemObject * currObject = (systemObject *)0;

    // find the pop / projection that is being displayed
    for (uint i = 0; i < this->system.size(); ++i) {

        if (this->system[i]->getName() == name)
        {currObject = this->system[i];
            break;}

        for (uint j = 0; j < this->system[i]->projections.size(); ++j) {

            if (this->system[i]->projections[j]->getName() == name)
            {currObject = this->system[i]->projections[j];
                break;}

        }
    }

    return currObject;
}


// allow safe usage of systemObject pointers
bool rootData::isValidPointer(systemObject * ptr) {

    // find the pop / projection / input reference
    for (uint i = 0; i < this->system.size(); ++i) {

        if (this->system[i] == ptr)
            return true;

        for (uint j = 0; j < this->system[i]->neuronType->inputs.size(); ++j)
            if (this->system[i]->neuronType->inputs[j] == ptr)
                return true;

        for (uint j = 0; j < this->system[i]->projections.size(); ++j) {

            if (this->system[i]->projections[j] == ptr)
                return true;

            for (uint k = 0; k < this->system[i]->projections[j]->synapses.size(); ++k) {

                if (this->system[i]->projections[j]->synapses[k] == ptr)
                    return true;

                for (uint l = 0; l < this->system[i]->projections[j]->synapses[k]->weightUpdateType->inputs.size(); ++l) {

                    if (this->system[i]->projections[j]->synapses[k]->weightUpdateType->inputs[l] == ptr)
                        return true;

                }

                for (uint l = 0; l < this->system[i]->projections[j]->synapses[k]->postsynapseType->inputs.size(); ++l) {

                    if (this->system[i]->projections[j]->synapses[k]->postsynapseType->inputs[l] == ptr)
                        return true;

                }

            }

        }
    }

    // not found
    return false;
}

// allow safe usage of NineMLComponentData pointers
bool rootData::isValidPointer(NineMLComponentData * ptr) {

    // find the reference
    for (uint i = 0; i < this->system.size(); ++i) {

        if (this->system[i]->neuronType == ptr)
            return true;

        for (uint j = 0; j < this->system[i]->projections.size(); ++j) {

            for (uint k = 0; k < this->system[i]->projections[j]->synapses.size(); ++k) {

                if (this->system[i]->projections[j]->synapses[k]->weightUpdateType == ptr)
                    return true;

                if (this->system[i]->projections[j]->synapses[k]->postsynapseType == ptr)
                        return true;


            }

        }
    }

    // not found
    return false;
}

// allow safe usage of NineMLComponent pointers
bool rootData::isValidPointer(NineMLComponent * ptr) {

    for (uint i = 0; i < this->catalogNrn.size(); ++i)
        if (catalogNrn[i] == ptr)
            return true;
    for (uint i = 0; i < this->catalogPS.size(); ++i)
        if (catalogPS[i] == ptr)
            return true;
    for (uint i = 0; i < this->catalogUnsorted.size(); ++i)
        if (catalogUnsorted[i] == ptr)
            return true;
    for (uint i = 0; i < this->catalogWU.size(); ++i)
        if (catalogWU[i] == ptr)
            return true;

    // not found
    return false;
}

void rootData::setSelectionbyName(QString name) {

    systemObject * currObject = (systemObject *)0;

    // find the pop / projection that is being displayed
    for (uint i = 0; i < this->system.size(); ++i) {

        if (this->system[i]->getName() == name)
        {currObject = this->system[i];
            break;}

        for (uint j = 0; j < this->system[i]->projections.size(); ++j) {

            if (this->system[i]->projections[j]->getName() == name)
            {currObject = this->system[i]->projections[j];
                break;}

        }
    }

    this->selList.clear();
    this->selList.push_back(currObject);

}

void rootData::returnPointerToSelf(rootData * * data) {

    (*data) = this;

}

void rootData::addgenericInput() {

    // input text
    QString text = ((QLineEdit *) sender())->text();
    NineMLComponentData * src = (NineMLComponentData *)0;

    // find source:
    for (uint i = 0; i < this->system.size(); ++i) {
        if (this->system[i]->neuronType->getXMLName() == text) {
            src = this->system[i]->neuronType;
        }
        for (uint j = 0; j < this->system[i]->projections.size(); ++j) {
            for (uint k = 0; k < this->system[i]->projections[j]->synapses.size(); ++k) {
                if (this->system[i]->projections[j]->synapses[k]->weightUpdateType->getXMLName() == text) {
                    src = this->system[i]->projections[j]->synapses[k]->weightUpdateType;
                }
                if (this->system[i]->projections[j]->synapses[k]->postsynapseType->getXMLName() == text) {
                    src = this->system[i]->projections[j]->synapses[k]->postsynapseType;
                }
            }
        }
    }

    if (src != (NineMLComponentData *)0) {

        NineMLComponentData * dst = (NineMLComponentData *) sender()->property("ptr").value<void *>();

        // disconnect so we don't get multiples
        sender()->disconnect((QObject *) 0);

        // add the genericInput
        currProject->undoStack->push(new addInput(this, src, dst));

        // redraw panel
        emit updatePanel(this);
        emit redrawGLview();

    } else {
        // src not found - set the LineEdit background red-ish
        QPalette p = ((QLineEdit *) sender())->palette();
        p.setColor( QPalette::Normal, QPalette::Base, QColor(255, 200, 200) );
        ((QLineEdit *) sender())->setPalette(p);
    }

}

void rootData::delgenericInput() {

    genericInput * ptr = (genericInput *) sender()->property("ptr").value<void *>();
    //NineMLComponentData * ptrDst = (NineMLComponentData *) sender()->property("ptrDst").value<void *>();

    // delete the genericInput
    currProject->undoStack->push(new delInput(this, ptr));

    emit updatePanel(this);

}

void rootData::editConnections() {
    // launch the list editor dialog
    csv_connection * conn = (csv_connection *) sender()->property("ptr").value<void *>();
    connectionListDialog * dialog  = new connectionListDialog(conn);
    dialog->show();

}

void rootData::setTitle() {
    emit setWindowTitle();
}

void rootData::setModelTitle(QString model_name) {

    currProject->undoStack->push(new updateModelTitle(this, model_name, currProject));
    setCaptionOut(model_name);
}

void rootData::setCaptionOut(QString model_name) {
    // update version and name caption
    emit setCaption(model_name + " <i>" + "</i>");
}

void rootData::undoOrRedoPerformed(int) {
    emit redrawGLview();
    QSettings settings;
    setCaptionOut(settings.value("model/model_name", "err").toString());
    // update file list for components
    emit setWindowTitle();
}

void rootData::copyParsToClipboard() {

    // safety
    if (selList.size() == 1) {

        // if population
        if (selList[0]->type == populationObject) {
            if (sender()->property("source").toString() == "tab1") {
                if (clipboardCData != NULL)
                    delete clipboardCData;
                clipboardCData = new NineMLComponentData(((population *) selList[0])->neuronType);
            }
        }

        // if projection
        if (selList[0]->type == projectionObject) {
            if (sender()->property("source").toString() == "tab1") {
                if (clipboardCData != NULL)
                    delete clipboardCData;
                clipboardCData = new NineMLComponentData(((projection *) selList[0])->synapses[((projection *) selList[0])->currTarg]->weightUpdateType);
            }
            if (sender()->property("source").toString() == "tab2") {
                if (clipboardCData != NULL)
                    delete clipboardCData;
                clipboardCData = new NineMLComponentData(((projection *) selList[0])->synapses[((projection *) selList[0])->currTarg]->postsynapseType);
            }
        }
    }
    this->reDrawAll();
}

void rootData::pasteParsFromClipboard() {

    // safety
    if (selList.size() == 1) {

        // if population
        if (selList[0]->type == populationObject) {
            if (sender()->property("source").toString() == "tab1") {
                currProject->undoStack->push(new pastePars(this,clipboardCData,((population *) selList[0])->neuronType));
            }
        }

        // if projection
        if (selList[0]->type == projectionObject) {
            if (sender()->property("source").toString() == "tab1") {
                currProject->undoStack->push(new pastePars(this,clipboardCData,((projection *) selList[0])->synapses[((projection *) selList[0])->currTarg]->weightUpdateType));
            }
            if (sender()->property("source").toString() == "tab2") {
                currProject->undoStack->push(new pastePars(this,clipboardCData,((projection *) selList[0])->synapses[((projection *) selList[0])->currTarg]->postsynapseType));
            }
        }
    }
}
