#include "projectobject.h"
#include "nineML_classes.h"
#include "nineml_layout_classes.h"
#include "rootdata.h"
#include "mainwindow.h"
#include "versioncontrol.h"
#include "experiment.h"
#include "systemmodel.h"

projectObject::projectObject(QObject *parent) :
    QObject(parent)
{
    this->name = "New Project";

    menuAction = new QAction(this);

    undoStack = new QUndoStack(this);

    // default fileNames
    networkFile = "model.xml";
    metaFile = "metaData.xml";

    // create the catalog blank entries:
    this->catalogGC.push_back((new NineMLComponent()));
    this->catalogGC[0]->name = "none";
    this->catalogGC[0]->type = "moo";
    this->catalogNB.push_back((new NineMLComponent()));
    this->catalogNB[0]->name = "none";
    this->catalogNB[0]->type = "neuron_body";
    this->catalogWU.push_back((new NineMLComponent()));
    this->catalogWU[0]->name = "none";
    this->catalogWU[0]->type = "weight_update";
    this->catalogPS.push_back((new NineMLComponent()));
    this->catalogPS[0]->name = "none";
    this->catalogPS[0]->type = "postsynapse";
    this->catalogLAY.push_back((new NineMLLayout()));
    this->catalogLAY[0]->name = "none";
}

projectObject::~projectObject()
{
    // clean up these
    delete undoStack;
    delete menuAction;

    // destroy experiments
    for (uint i = 0; i < experimentList.size(); ++i) {
        delete experimentList[i];
    }

    for (uint i = 0; i < network.size(); ++i) {
        for (uint j = 0; j < network[i]->projections.size(); ++j) {
            for (uint k = 0; k < network[i]->projections[j]->synapses.size(); ++k) {
                delete network[i]->projections[j]->synapses[k];
            }
            delete network[i]->projections[j];
        }
        delete network[i];
    }

    // delete catalog components
    for (uint i = 0; i < catalogLAY.size(); ++i) {
        delete catalogLAY[i];
    }
    for (uint i = 0; i < catalogNB.size(); ++i) {
        delete catalogNB[i];
    }
    for (uint i = 0; i < catalogPS.size(); ++i) {
        delete catalogPS[i];
    }
    for (uint i = 0; i < catalogWU.size(); ++i) {
        delete catalogWU[i];
    }
    for (uint i = 0; i < catalogGC.size(); ++i) {
        delete catalogGC[i];
    }

    // clear catalog vectors
    catalogLAY.clear();
    catalogNB.clear();
    catalogPS.clear();
    catalogWU.clear();
    catalogGC.clear();
}

bool projectObject::open_project(QString fileName)
{
    QDir project_dir(fileName);

    // remove filename
    project_dir.cdUp();

    QSettings settings;
    settings.setValue("files/currentFileName", project_dir.absolutePath());

    // first try and open the project file
    if (!load_project_file(fileName)) {
        printErrors("Errors found loading the project file:");
        return false;
    }

    // then load in all the components listed in the project file ///////////
    for (int i = 0; i < components.size(); ++i) {
        loadComponent(components[i], project_dir);
    }
    printErrors("Errors found loading project Components:");

    // then load in all the layouts listed in the project file //////////////
    for (int i = 0; i < layouts.size(); ++i) {
        loadLayout(layouts[i], project_dir);
    }
    printErrors("Errors found loading project Layouts:");

    // now the network //////////////////////////////////////////////////////
    loadNetwork(networkFile, project_dir);
    if (printErrors("Errors prevented loading the Project:"))
        return false;

    // finally the experiments //////////////////////////////////////////////
    for (int i = 0; i < experiments.size(); ++i) {
        loadExperiment(experiments[i], project_dir);
    }
    printErrors("Errors found loading project Experiments:");

    // check for errors /////////////////////////////////////////////////////
    printWarnings("Issues were found while loading the project:");

    // store the new file name
    this->filePath = fileName;

    return true;
}

bool projectObject::save_project(QString fileName, rootData * data)
{
    if (!fileName.contains(".")) {
        QMessageBox msgBox;
        msgBox.setText("Project file needs .proj suffix.");
        msgBox.exec();
        return false;
    }

    QDir project_dir(fileName);

    // remove filename
    project_dir.cdUp();

    QSettings settings;
    settings.setValue("files/currentFileName", project_dir.absolutePath());

    // check for version control
    version.setupVersion();

    // remove old binary files
    project_dir.setNameFilters(QStringList() << "*.bin");
    QStringList files = project_dir.entryList(QDir::Files);
    for (int i = 0; i < files.size(); ++i) {
        // delete
        project_dir.remove(files[i]);
        // and remove from version control
        if (version.isModelUnderVersion()) {
            version.removeFromVersion(files[i]);
        }
    }

    // sync project
    copy_back_data(data);

    // write project file
    if (!save_project_file(fileName)) {
        return false;
    }

    // write components
    for (uint i = 1; i < catalogNB.size(); ++i) {
        saveComponent(catalogNB[i]->getXMLName(), project_dir, catalogNB[i]);
    }
    for (uint i = 1; i < catalogWU.size(); ++i) {
        saveComponent(catalogWU[i]->getXMLName(), project_dir, catalogWU[i]);
    }
    for (uint i = 1; i < catalogPS.size(); ++i) {
        saveComponent(catalogPS[i]->getXMLName(), project_dir, catalogPS[i]);
    }
    for (uint i = 1; i < catalogGC.size(); ++i) {
        saveComponent(catalogGC[i]->getXMLName(), project_dir, catalogGC[i]);
    }

    // write layouts
    for (uint i = 1; i < catalogLAY.size(); ++i) {
        saveLayout(catalogLAY[i]->getXMLName(), project_dir, catalogLAY[i]);
    }

    // write network
    saveNetwork(networkFile, project_dir);

    // saveMetaData
    saveMetaData(metaFile, project_dir);

    // write experiments
    for (uint i = 0; i < experimentList.size(); ++i) {
        saveExperiment("experiment" + QString::number(i) + ".xml", project_dir, experimentList[i]);
    }

    // store the new file name
    this->filePath = fileName;

    if (printErrors("Errors found")) {
        settings.remove("export_for_simulation");
        return false;
    }
    if (printWarnings("Warnings found")) {
        settings.remove("export_for_simulation");
        return false;
    }

    this->undoStack->setClean();

    return true;
}

bool projectObject::export_for_simulator(QString fileName, rootData * data)
{
    QSettings settings;

    settings.setValue("export_for_simulation", QString::number((float) true));

    // fetch current experiment
    experiment * currentExperiment = NULL;

    // transfer model back from data
    this->copy_back_data(data);

    // find currentExperiment
    for (uint i = 0; i < this->experimentList.size(); ++i) {
        if (this->experimentList[i]->selected) {currentExperiment = this->experimentList[i]; break;}
    }

    if (currentExperiment == NULL) return false;

    QDir project_dir(fileName);
    // Before writing components out, ensure that the simulator working
    // directory exists.
    if (!project_dir.exists()) {
        // Create directory.
        if (project_dir.mkpath(fileName)) {
            // fileName now exists.
        } else {
            // failure.
            addError("export_for_simulator: Error creating working directory " + fileName);
            if (printErrors("Errors found")) {
                settings.remove("export_for_simulation");
                return false;
            }
        }
    }

    // remove old files
    project_dir.setNameFilters(QStringList() << "*");
    QStringList files = project_dir.entryList(QDir::Files);
    for (int i = 0; i < files.size(); ++i) {
        // delete
        project_dir.remove(files[i]);
        // and remove from version control (not needed for simulation)
        /*if (version.isModelUnderVersion()) {
            version.removeFromVersion(files[i]);
        }*/
    }

    // sync project
    copy_back_data(data);

    // write components
    for (uint i = 1; i < catalogNB.size(); ++i) {
        QString fileName = catalogNB[i]->getXMLName();
        fileName.replace(" ", "_");
        saveComponent(fileName, project_dir, catalogNB[i]);
    }
    for (uint i = 1; i < catalogWU.size(); ++i) {
        QString fileName = catalogWU[i]->getXMLName();
        fileName.replace(" ", "_");
        saveComponent(fileName, project_dir, catalogWU[i]);
    }
    for (uint i = 1; i < catalogPS.size(); ++i) {
        QString fileName = catalogPS[i]->getXMLName();
        fileName.replace(" ", "_");
        saveComponent(fileName, project_dir, catalogPS[i]);
    }
    for (uint i = 1; i < catalogGC.size(); ++i) {
        QString fileName = catalogGC[i]->getXMLName();
        fileName.replace(" ", "_");
        saveComponent(fileName, project_dir, catalogGC[i]);
    }

    // write layouts
    for (uint i = 1; i < catalogLAY.size(); ++i) {
        saveLayout(catalogLAY[i]->getXMLName(), project_dir, catalogLAY[i]);
    }

    // write network
    saveNetwork(networkFile, project_dir);

    // write experiment
    saveExperiment("experiment.xml", project_dir, currentExperiment);


    if (printErrors("Errors found")) {
        settings.remove("export_for_simulation");
        return false;
    }
    if (printWarnings("Warnings found")) {
        settings.remove("export_for_simulation");
        return false;
    }

    settings.remove("export_for_simulation");

    return true;
}

bool projectObject::import_network(QString fileName) {

    QDir project_dir(fileName);

    // remove filename
    project_dir.cdUp();

    // get a list of the files in the directory
    QStringList files = project_dir.entryList();

    // load all the components in the list
    for (int i = 0; i < files.size(); ++i) {
        if (isComponent(project_dir.absoluteFilePath(files[i])))
            loadComponent(files[i], project_dir);
    }

    // load all the layouts in the list
    for (int i = 0; i < files.size(); ++i) {
        if (isLayout(project_dir.absoluteFilePath(files[i]))) {
            loadLayout(files[i], project_dir);
        }
    }

    // load the network
    loadNetwork(fileName, project_dir, false);
    if (printErrors("Errors prevented importing the Network:"))
        return false;

    // set up metaData if not loaded
    if (metaFile == "not found") {
        metaFile = "metaData.xml";

        // place populations
        for (uint i = 0; i < this->network.size(); ++i) {
            population * p = this->network[i];
            p->x = i*2.0; p->targx = i*2.0;
            p->y = i*2.0; p->targy = i*2.0;
            p->size = 1.0;
            p->aspect_ratio = 5.0/3.0;
            p->setupBounds();
        }

        // make projection curves
        for (uint i = 0; i < this->network.size(); ++i) {
            population * p = this->network[i];
            for (uint j = 0; j < p->projections.size(); ++j) {
                projection * pr = p->projections[j];
                pr->add_curves();
            }
        }
    }

    // finally load the experiments
    for (int i = 0; i < files.size(); ++i) {
        loadExperiment(files[i], project_dir, true);
    }

    printWarnings("Issues found importing the Network:");
    printErrors("Errors found importing the Network:");

    return true;
}

void projectObject::import_component(QString fileName) {

    QDir project_dir(fileName);

    // remove filename
    project_dir.cdUp();

    loadComponent(fileName, project_dir);

}

void projectObject::import_layout(QString fileName) {

    QDir project_dir(fileName);

    // remove filename
    project_dir.cdUp();

    loadLayout(fileName, project_dir);

}

bool projectObject::load_project_file(QString fileName) {

    //////////////// OPEN FILE

    QFile file( fileName );
    if( !file.open( QIODevice::ReadOnly ) ) {
        QMessageBox msgBox;
        msgBox.setText("Could not open the project file");
        msgBox.exec();
        return false;}

    // get a streamreader
    QXmlStreamReader * reader = new QXmlStreamReader;

    // set the stream reader device to the file
    reader->setDevice( &file );

    // read elements
    while (reader->readNextStartElement()) {

        if (reader->name() == "SpineCreatorProject") {

            while (reader->readNextStartElement()) {

                if (reader->name() == "Network") {

                    while (reader->readNextStartElement()) {

                        if (reader->name() == "File") {

                            if (reader->attributes().hasAttribute("name"))
                                networkFile = reader->attributes().value("name").toString();
                            else {
                                QSettings settings;
                                int num_errs = settings.beginReadArray("errors");
                                settings.endArray();
                                settings.beginWriteArray("errors");
                                    settings.setArrayIndex(num_errs + 1);
                                    settings.setValue("errorText", "XML Error in Project File - missing attribute 'name'");
                                settings.endArray();
                            }
                            if (reader->attributes().hasAttribute("metaFile"))
                                metaFile = reader->attributes().value("metaFile").toString();
                            else {
                                QSettings settings;
                                int num_errs = settings.beginReadArray("errors");
                                settings.endArray();
                                settings.beginWriteArray("errors");
                                    settings.setArrayIndex(num_errs + 1);
                                    settings.setValue("errorText", "XML Error in Project File - missing attribute 'metaFile'");
                                settings.endArray();
                            }
                            reader->skipCurrentElement();

                        } else {
                            QSettings settings;
                            int num_errs = settings.beginReadArray("errors");
                            settings.endArray();
                            settings.beginWriteArray("errors");
                                settings.setArrayIndex(num_errs + 1);
                                settings.setValue("errorText", "XML Error in Project File - unknown tag '" + reader->name().toString() + "'");
                            settings.endArray();
                        }

                    }

                } else if (reader->name() == "Components") {

                    while (reader->readNextStartElement()) {

                        if (reader->name() == "File") {

                            if (reader->attributes().hasAttribute("name"))
                                components.push_back(reader->attributes().value("name").toString());
                            else {
                                QSettings settings;
                                int num_errs = settings.beginReadArray("errors");
                                settings.endArray();
                                settings.beginWriteArray("errors");
                                    settings.setArrayIndex(num_errs + 1);
                                    settings.setValue("errorText", "XML Error in Project File - missing attribute 'name'");
                                settings.endArray();
                            }
                            reader->skipCurrentElement();

                        } else {
                            QSettings settings;
                            int num_errs = settings.beginReadArray("errors");
                            settings.endArray();
                            settings.beginWriteArray("errors");
                                settings.setArrayIndex(num_errs + 1);
                                settings.setValue("errorText", "XML Error in Project File - unknown tag '" + reader->name().toString() + "'");
                            settings.endArray();
                        }

                    }

                } else if (reader->name() == "Layouts") {

                    while (reader->readNextStartElement()) {

                        if (reader->name() == "File") {

                            if (reader->attributes().hasAttribute("name"))
                                layouts.push_back(reader->attributes().value("name").toString());
                            else {
                                QSettings settings;
                                int num_errs = settings.beginReadArray("errors");
                                settings.endArray();
                                settings.beginWriteArray("errors");
                                    settings.setArrayIndex(num_errs + 1);
                                    settings.setValue("errorText", "XML Error in Project File - missing attribute 'name'");
                                settings.endArray();
                            }
                            reader->skipCurrentElement();

                        } else {
                            QSettings settings;
                            int num_errs = settings.beginReadArray("errors");
                            settings.endArray();
                            settings.beginWriteArray("errors");
                                settings.setArrayIndex(num_errs + 1);
                                settings.setValue("errorText", "XML Error in Project File - unknown tag '" + reader->name().toString() + "'");
                            settings.endArray();
                        }

                    }

                } else if (reader->name() == "Experiments") {

                    while (reader->readNextStartElement()) {

                        if (reader->name() == "File") {

                            if (reader->attributes().hasAttribute("name"))
                                experiments.push_back(reader->attributes().value("name").toString());
                            else {
                                QSettings settings;
                                int num_errs = settings.beginReadArray("errors");
                                settings.endArray();
                                settings.beginWriteArray("errors");
                                    settings.setArrayIndex(num_errs + 1);
                                    settings.setValue("errorText", "XML Error in Project File - missing attribute 'name'");
                                settings.endArray();
                            }
                            reader->skipCurrentElement();

                        } else {
                            QSettings settings;
                            int num_errs = settings.beginReadArray("errors");
                            settings.endArray();
                            settings.beginWriteArray("errors");
                                settings.setArrayIndex(num_errs + 1);
                                settings.setValue("errorText", "XML Error in Project File - unknown tag '" + reader->name().toString() + "'");
                            settings.endArray();
                        }

                    }

                }  else {
                    QSettings settings;
                    int num_errs = settings.beginReadArray("errors");
                    settings.endArray();
                    settings.beginWriteArray("errors");
                        settings.setArrayIndex(num_errs + 1);
                        settings.setValue("errorText", "XML Error in Project File - unknown tag '" + reader->name().toString() + "'");
                    settings.endArray();
                }

            }

        } else {
            QSettings settings;
            int num_errs = settings.beginReadArray("errors");
            settings.endArray();
            settings.beginWriteArray("errors");
                settings.setArrayIndex(num_errs + 1);
                settings.setValue("errorText", "XML Error in Project File - incorrect start tag");
            settings.endArray();
        }

    }

    return true;
}

bool projectObject::save_project_file(QString fileName)
{
    // complain if there's no extension (client code should correctly set fileName)
    if (!fileName.contains(".")) {
        QMessageBox msgBox;
        msgBox.setText("Project file needs .proj suffix.");
        msgBox.exec();
        return false;
    }

    QFile file( fileName );
    if( !file.open( QIODevice::WriteOnly ) ) {
        QMessageBox msgBox;
        msgBox.setText("Could not create the project file");
        msgBox.exec();
        return false;
    }

    // get a streamwriter
    QXmlStreamWriter * writer = new QXmlStreamWriter;

    // set the stream reader device to the file
    writer->setDevice( &file );

    // write elements
    writer->writeStartDocument();
    writer->writeStartElement("SpineCreatorProject");

    writer->writeStartElement("Network");

    writer->writeEmptyElement("File");
    writer->writeAttribute("name", networkFile);
    writer->writeAttribute("metaFile", metaFile);

    writer->writeEndElement(); // Network

    writer->writeStartElement("Components");

    for (uint i = 1; i < catalogNB.size(); ++i) {
        writer->writeEmptyElement("File");
        writer->writeAttribute("name", catalogNB[i]->getXMLName());
    }
    for (uint i = 1; i < catalogWU.size(); ++i) {
        writer->writeEmptyElement("File");
        writer->writeAttribute("name", catalogWU[i]->getXMLName());
    }
    for (uint i = 1; i < catalogPS.size(); ++i) {
        writer->writeEmptyElement("File");
        writer->writeAttribute("name", catalogPS[i]->getXMLName());
    }
    for (uint i = 1; i < catalogGC.size(); ++i) {
        writer->writeEmptyElement("File");
        writer->writeAttribute("name", catalogGC[i]->getXMLName());
    }

    writer->writeEndElement(); // Components

    writer->writeStartElement("Layouts");

    for (uint i = 0; i < catalogLAY.size(); ++i) {
        writer->writeEmptyElement("File");
        writer->writeAttribute("name", catalogLAY[i]->getXMLName());
    }

    writer->writeEndElement(); // Layouts

    writer->writeStartElement("Experiments");

    for (uint i = 0; i < experimentList.size(); ++i) {
        writer->writeEmptyElement("File");
        writer->writeAttribute("name", "experiment" + QString::number(i) + ".xml");
    }

    writer->writeEndElement(); // Experiments

    writer->writeEndElement(); // SpineCreatorProject

    // add to version control
    if (version.isModelUnderVersion())
        version.addToVersion(file.fileName());

    return true;
}

bool projectObject::isComponent(QString fileName) {

    // try opening the file and loading the XML
    QFile file( fileName );
    if( !file.open( QIODevice::ReadOnly ) )
        return false;
    if( !doc.setContent( &file ) )
        return false;

    // we have loaded the XML - discard the file handle
    file.close();

    // confirm root tag is correct
    QDomElement root = doc.documentElement();
    if( root.tagName() != "SpineML" )
        {doc.clear(); return false;}

    // if a componentclass
    QDomElement classType = root.firstChildElement();
    if (classType.tagName() != "ComponentClass")
        {doc.clear(); return false;}

    // clean up
    doc.clear();
    return true;

}

bool projectObject::isLayout(QString fileName) {

    // try opening the file and loading the XML
    QFile file( fileName );
    if( !file.open( QIODevice::ReadOnly ) )
        return false;
    if( !doc.setContent( &file ) )
        return false;

    // we have loaded the XML - discard the file handle
    file.close();

    // confirm root tag is correct
    QDomElement root = doc.documentElement();
    if ( root.tagName() != "SpineML" )
        {doc.clear(); return false;}

    // if a componentclass
    QDomElement classType = root.firstChildElement();
    if (classType.tagName() != "LayoutClass")
        {doc.clear(); return false;}

    // clean up
    doc.clear();

    return true;

}

void projectObject::loadComponent(QString fileName, QDir project_dir) {

    if (fileName == "none.xml")
        return;

    // try opening the file and loading the XML
    QFile file( project_dir.absoluteFilePath(fileName) );
    if( !file.open( QIODevice::ReadOnly ) ) {
        addError("Cannot open required file '" + fileName + "'");
        return;
    }
    if( !doc.setContent( &file ) ) {
        addError("Cannot read required file '" + fileName + "'");
        return;
    }

    // confirm root tag is correct
    QDomElement root = doc.documentElement();
    if( root.tagName() != "SpineML" ) {
        addError("Missing or incorrect root tag in required file '" + fileName + "'");
        return;
    }

    // if a componentclass
    QDomElement classType = root.firstChildElement();

    if (classType.tagName() == "ComponentClass") {

        // HANDLE SPINEML COMPONENTS ////////////////

        // create a new AL class instance and populate it from the data
        NineMLComponent *tempALobject = new NineMLComponent();

        tempALobject->load(&doc);

        // check for errors:
        QSettings settings;
        int num_errs = settings.beginReadArray("errors");
        settings.endArray();

        // if there are errors then clean up and leave
        if (num_errs != 0) {
            delete tempALobject;
            // write tail for errors:
            addError("<b>IN COMPONENT FILE '" + fileName + "'</b>");
            return;
        }

        // get lib to add component to
        vector < NineMLComponent * > * curr_lib;
        if (tempALobject->type == "neuron_body")
            curr_lib = &catalogNB;
        else if (tempALobject->type == "weight_update")
            curr_lib = &catalogWU;
        else if (tempALobject->type == "postsynapse")
            curr_lib = &catalogPS;
        else
            curr_lib = &catalogGC;

        // check the name doesn't already exist in the library
        for (uint i = 0; i < curr_lib->size(); ++i) {
            if ((*curr_lib)[i]->name == tempALobject->name && (*curr_lib)[i]->path == tempALobject->path && tempALobject->name != "none") {
                // same name
                addWarning("Two required files have the same Component Name - this project may be corrupted");
                delete tempALobject;
                return;
            }
        }

        // add to the correct catalog
        curr_lib->push_back(tempALobject);

    } else {
        addError("Unknown XML tag found in required file '" + fileName + "'");
    }

}

void projectObject::saveComponent(QString fileName, QDir project_dir, NineMLComponent * component) {

    // if no extension then append a .xml
    if (!fileName.contains("."))
        fileName.append(".xml");

    QString fname = project_dir.absoluteFilePath(fileName);
    QFile file( fname );
    if (!file.open( QIODevice::WriteOnly)) {
        addError("saveComponent: Error creating file for '" + fname + "' - is there sufficient disk space?");
        return;
    }

    doc.setContent(QString(""));

    // get the 9ML description
    component->write(&doc);

    // write out to file
    QTextStream tsFromFile( &file );
    tsFromFile << doc.toString();
    tsFromFile.flush();

    // add to version control
    if (version.isModelUnderVersion())
        version.addToVersion(file.fileName());

    file.close();

    // store path for easy access
    component->filePath = project_dir.absoluteFilePath(fileName);

    // kill off the DOM document
    doc.clear();
}

void projectObject::loadLayout(QString fileName, QDir project_dir) {

    if (fileName == "none.xml")
        return;

    // try opening the file and loading the XML
    QFile file( project_dir.absoluteFilePath(fileName) );
    if( !file.open( QIODevice::ReadOnly ) ) {
        addError("Cannot open required file '" + fileName + "'");
        return;
    }
    if( !doc.setContent( &file ) ) {
        addError("Cannot read required file '" + fileName + "'");
        return;
    }

    // confirm root tag is correct
    QDomElement root = doc.documentElement();
    if( root.tagName() != "SpineML" ) {
        addError("Missing or incorrect root tag in required file '" + fileName + "'");
        return;
    }

    // if a componentclass
    QDomElement classType = root.firstChildElement();

    if (classType.tagName() == "LayoutClass") {

            // HANDLE LAYOUTS ////////////////////

            // create a new AL class instance and populate it from the data
            NineMLLayout *tempALobject = new NineMLLayout();

            tempALobject->load(&doc);

            // check for errors:
            QSettings settings;
            int num_errs = settings.beginReadArray("errors");
            settings.endArray();

            if (num_errs != 0) {
                delete tempALobject;
                // write tail for errors:
                addError("<b>IN LAYOUT FILE '" + fileName + "'</b>");
                return;
            }

            for (unsigned int i = 0; i < this->catalogLAY.size(); ++i) {
                if (this->catalogLAY[i]->name.compare(tempALobject->name) == 0 && tempALobject->name != "none") {
                    // same name
                    addWarning("Two required files have the same Layout Name - this project may be corrupted");
                    delete tempALobject;
                    return;
                }
            }

            // all good - add layout to catalog
            catalogLAY.push_back(tempALobject);

        } else {
            addError("Unknown XML tag found in required file '" + fileName + "'");
        }

}

void projectObject::saveLayout(QString fileName, QDir project_dir, NineMLLayout * layout) {

    // if no extension then append a .xml
    if (!fileName.contains("."))
        fileName.append(".xml");

    QFile file( project_dir.absoluteFilePath(fileName) );
    if (!file.open( QIODevice::WriteOnly)) {
        addError("Error creating file for '" + fileName + "' - is there sufficient disk space?");
        return;
    }

    doc.setContent(QString(""));

    // get the 9ML description
    layout->write(&doc);

    // write out to file
    QTextStream tsFromFile( &file );
    tsFromFile << doc.toString();
    tsFromFile.flush();

    // add to version control
    if (version.isModelUnderVersion())
        version.addToVersion(file.fileName());

    file.close();

    // store path for easy access
    layout->filePath = project_dir.absoluteFilePath(fileName);

    // kill off the DOM document
    doc.clear();

}


void projectObject::loadNetwork(QString fileName, QDir project_dir, bool isProject) {

    // load up the file and check it is valid XML
    QFile file( project_dir.absoluteFilePath(fileName) );
    if( !file.open( QIODevice::ReadOnly ) ) {
        addError("Could not open the Network file for reading");
        return;
    }
    if( !doc.setContent( &file ) ) {
        addError("Could not parse the Network file XML - is the selected file correctly formed XML?");
        return;
    }

    QSettings settings;
    settings.setValue("files/currentFileName", project_dir.absolutePath());

    // we have loaded the XML file - discard the file handle
    file.close();

    // confirm root tag is correct
    QDomElement root = doc.documentElement();
    if ( root.tagName() != "LL:SpineML" ) {
        addError("Network file is not valid SpineML Low Level Network Layer description");
        return;
    }

    // get the model name
    name = root.toElement().attribute("name", "Untitled project");

    // only load metadata for projects

    QString metaFilePath = project_dir.absoluteFilePath(metaFile);
    QFile fileMeta( metaFilePath );

    if( !fileMeta.open( QIODevice::ReadOnly) ) {
        // if is not a project we don't expect a metaData file
        if (isProject) {
            addError("Could not open the MetaData file for reading");
            return;
        } else {
            metaFile = "not found";
        }
    } else {
        if( !meta.setContent( &fileMeta ))
        {
            addError("Could not parse the MetaData file XML - is the selected file correctly formed XML?");
            return;
        }
        // we have loaded the XML file - discard the file handle
        fileMeta.close();

        // confirm root tag is correct
        root = meta.documentElement();
        if( root.tagName() != "modelMetaData") {
            addError("MetaData file is not valid");
            return;
        }
    }


    //////////////// LOAD POPULATIONS
    QDomNode n = doc.documentElement().firstChild();
    while( !n.isNull() )
    {

        QDomElement e = n.toElement();
        if( e.tagName() == "LL:Population" )
        {
            // add population from population xml:
            this->network.push_back(new population(e, &doc, &meta, this));

            // check for duplicate names:
            for (uint i = 0; i < network.size() - 1; ++i) {
                if (network[i]->name == network.back()->name) {
                    network[i]->name = getUniquePopName(network[i]->name);
                    addWarning("Duplicate Population name found: renamed existing Population to '" + network[i]->name + "'");
                }
            }

            // check for errors:
            QSettings settings;
            int num_errs = settings.beginReadArray("errors");
            settings.endArray();

            if (num_errs != 0) {
                // no dice - give up!
                return;
            }

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
            this->network[counter]->load_projections_from_xml(e, &doc, &meta, this);

            // check for errors:
            QSettings settings;
            int num_errs = settings.beginReadArray("errors");
            settings.endArray();

            if (num_errs != 0) {
                // no dice - give up!
                return;
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
            this->network[counter]->read_inputs_from_xml(e, &meta, this);

            int projCount = 0;
            QDomNodeList n2 = n.toElement().elementsByTagName("LL:Projection");
            for (uint i = 0; i < (uint) n2.size(); ++i)
            {
                QDomElement e = n2.item(i).toElement();
                this->network[counter]->projections[projCount]->read_inputs_from_xml(e, &meta, this);
                ++projCount;
            }
            ++counter;
        }
        n = n.nextSibling();

    }

    meta.clear();
    doc.clear();
}

void projectObject::saveNetwork(QString fileName, QDir projectDir) {

    QFile fileModel( projectDir.absoluteFilePath( fileName ) );
    if (!fileModel.open( QIODevice::WriteOnly)) {
        addError("Error creating Network file - is there sufficient disk space?");
        return;
    }

    // use stream writing for model UL file
    QXmlStreamWriter xmlOut;

    xmlOut.setAutoFormatting(true);

    xmlOut.setDevice(&(fileModel));

    // create the root of the file:
    xmlOut.writeStartDocument();
    xmlOut.writeStartElement("LL:SpineML");
    xmlOut.writeAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
    xmlOut.writeAttribute("xmlns", "http://www.shef.ac.uk/SpineMLNetworkLayer");
    xmlOut.writeAttribute("xmlns:LL", "http://www.shef.ac.uk/SpineMLLowLevelNetworkLayer");
    xmlOut.writeAttribute("xsi:schemaLocation", "http://www.shef.ac.uk/SpineMLLowLevelNetworkLayer SpineMLLowLevelNetworkLayer.xsd http://www.shef.ac.uk/SpineMLNetworkLayer SpineMLNetworkLayer.xsd");
    xmlOut.writeAttribute("name", name);

    // create a node for each population with the variables set
    for (unsigned int pop = 0; pop < this->network.size(); ++pop) {

        //// WE NEED TO HAVE A PROPER MODEL NAME!
        network[pop]->write_population_xml(xmlOut);

    }

    xmlOut.writeEndDocument();

    // add to version control
    if (version.isModelUnderVersion())
        version.addToVersion(fileModel.fileName());

}

void projectObject::saveMetaData(QString fileName, QDir projectDir) {

    QFile fileMeta( projectDir.absoluteFilePath( fileName ) );
    if (!fileMeta.open( QIODevice::WriteOnly)) {
        addError("Error creating MetaData file - is there sufficient disk space?");
        return;
    }

    meta.setContent(QString(""));

    // create the root of the file:
    QDomElement root = meta.createElement( "modelMetaData" );
    meta.appendChild(root);

    // iterate through the populations and get the xml
    for (unsigned int i = 0; i < network.size(); ++i) {
        network[i]->write_model_meta_xml(meta, root);
    }

    QTextStream tsFromFileMeta( &fileMeta );
    tsFromFileMeta << meta.toString();

    // add to version control
    if (version.isModelUnderVersion())
        version.addToVersion(fileMeta.fileName());


}

void projectObject::loadExperiment(QString fileName, QDir project_dir, bool skipFileError) {

    QFile file( project_dir.absoluteFilePath(fileName) );
    if( !file.open( QIODevice::ReadOnly ) ) {
        if (!skipFileError)
            addError("Could not open Experiment file '" + fileName + "' for reading");
        return;
    }

    // setup stream reader
    QXmlStreamReader * reader = new QXmlStreamReader;
    reader->setDevice( &file );

    // check we have an experiment
    reader->readNextStartElement();
    reader->readNextStartElement();

    if (reader->name() != "Experiment") {
        if (!skipFileError)
            addError("Could not parse Experiment file '" + fileName + "'");
        return;
    }

    // reset
    delete reader;
    file.seek(0);

    // set stream reader again
    reader = new QXmlStreamReader;
    reader->setDevice( &file );

    // load experiment:

    experiment * newExperiment = new experiment;
    newExperiment->readXML(reader, this);

    // check for errors:
    QSettings settings;
    int num_errs = settings.beginReadArray("errors");
    settings.endArray();

    if (num_errs == 0)
        this->experimentList.push_back(newExperiment);
    else {
        // add error tail
        addError("<b>IN EXPERIMENT FILE '" + fileName + "'</b>");
    }

    // we have loaded the XML file - discard the file handle
    file.close();

    // kill off the XML reader
    delete reader;

}

void projectObject::saveExperiment(QString fileName, QDir project_dir, experiment * expt) {

    QFile file ( project_dir.absoluteFilePath(fileName) );
    if (!file.open( QIODevice::WriteOnly)) {
        addError("Error creating file - is there sufficient disk space?");
        return;
    }

    // use stream writer
    QXmlStreamWriter * xmlOutExpt = new QXmlStreamWriter;
    xmlOutExpt->setDevice(&(file));

    expt->writeXML(xmlOutExpt, this);


    delete xmlOutExpt;

    file.close();

    // add to version control
    if (version.isModelUnderVersion())
        version.addToVersion(fileName);

}

void projectObject::copy_back_data(rootData * data)
{
    // copy data from rootData to project
    network = data->populations;
    catalogNB = data->catalogNrn;
    catalogWU = data->catalogWU;
    catalogPS = data->catalogPS;
    catalogGC = data->catalogUnsorted;
    catalogLAY = data->catalogLayout;
    experimentList = data->experiments;
}

void projectObject::copy_out_data(rootData * data) {

    // copy from project to rootData
    data->populations = network;
    data->catalogNrn = catalogNB;
    data->catalogWU = catalogWU;
    data->catalogPS = catalogPS;
    data->catalogUnsorted = catalogGC;
    data->catalogLayout = catalogLAY;
    data->experiments = experimentList;

}

void projectObject::deselect_project(rootData * data) {

    // move everything back
    copy_back_data(data);

    // clear the selList
    data->selList.clear();

}

void projectObject::select_project(rootData * data) {

    // store QTreeWidget state of previous Project
    if (data->main->viewVZ.OpenGLWidget != NULL) {
        data->main->viewVZhandler->saveTreeState();
    }

    // move project data into rootData
    copy_out_data(data);

    QSettings settings;
    settings.remove("files/currentFileName");
    if (filePath != "") {
        settings.setValue("files/currentFileName", filePath);
    }
    settings.setValue("model/model_name", "New Project");

    // set the undostack
    data->main->undoStacks->setActiveStack(this->undoStack);

    data->currProject = this;

    // update GUI
    // experiment view
    data->main->viewELhandler->redraw();

    // visualiser view
    // configure TreeView
    if (data->main->viewVZ.OpenGLWidget != NULL) {
        if (!(data->main->viewVZ.sysModel == NULL)) {
            delete data->main->viewVZ.sysModel;
        }
        data->main->viewVZ.sysModel = new systemmodel(data);
        data->main->viewVZ.treeView->setModel(data->main->viewVZ.sysModel);
        data->main->viewVZhandler->restoreTreeState();
    }

    // update title on button bar
    data->setCaptionOut(this->name);

    // redraw everything
    data->reDrawAll();

}

void projectObject::printIssues(QString title) {

    printErrors(title);
    printWarnings(title);

}

bool projectObject::printWarnings(QString title) {

    QString warns;

    // collate warnings:
    QSettings settings;
    int num_warn = settings.beginReadArray("warnings");
    settings.endArray();

    if (num_warn != 0) {

        // list errors
        settings.beginReadArray("warnings");
        for (int j = 1; j < num_warn; ++j) {
            settings.setArrayIndex(j);
            warns = warns + settings.value("warnText", "").toString();
            warns = warns + "<br/>";
        }
        settings.endArray();

        // clear warnings
        settings.remove("warnings");

    } else
        return false;

    // display warnings:
    if (!warns.isEmpty()) {
        // display warnings
        QMessageBox msgBox;
        msgBox.setText("<P><b>" + title + "</b></P>" + warns);
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setTextFormat(Qt::RichText);
        msgBox.exec();
    }

    return true;

}

bool projectObject::printErrors(QString title) {

    QString errors;

    // collate errors:
    QSettings settings;
    int num_errs = settings.beginReadArray("errors");
    settings.endArray();

    if (num_errs != 0) {

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

    } else
        return false;

    // display errors:
    if (!errors.isEmpty()) {
        // display errors
        QMessageBox msgBox;
        msgBox.setText("<P><b>" + title + "</b></P>" + errors);
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setTextFormat(Qt::RichText);
        msgBox.exec();
    }

    return true;

}

void projectObject::addError(QString text) {
    QSettings settings;
    int num_errs = settings.beginReadArray("errors");
    settings.endArray();
    settings.beginWriteArray("errors");
        settings.setArrayIndex(num_errs + 1);
        settings.setValue("errorText",  text);
    settings.endArray();
}

void projectObject::addWarning(QString text) {
    QSettings settings;
    int num_errs = settings.beginReadArray("warnings");
    settings.endArray();
    settings.beginWriteArray("warnings");
        settings.setArrayIndex(num_errs + 1);
        settings.setValue("warnText",  text);
    settings.endArray();
}

bool projectObject::isChanged(rootData * data) {

    if (data->currProject == this)
        copy_back_data(data);

    if (!undoStack->isClean())
         return true;

    for (uint i = 1; i < catalogNB.size(); ++i) {
        if (!catalogNB[i]->undoStack.isClean())
            return true;
    }
    for (uint i = 1; i < catalogWU.size(); ++i) {
        if (!catalogWU[i]->undoStack.isClean())
            return true;
    }
    for (uint i = 1; i < catalogPS.size(); ++i) {
        if (!catalogPS[i]->undoStack.isClean())
            return true;
    }
    for (uint i = 1; i < catalogGC.size(); ++i) {
        if (!catalogGC[i]->undoStack.isClean())
            return true;
    }

    return false;

}

// allow safe usage of systemObject pointers
bool projectObject::isValidPointer(systemObject * ptr) {

    // find the pop / projection / input reference
    for (uint i = 0; i < this->network.size(); ++i) {

        if (this->network[i] == ptr)
            return true;

        for (uint j = 0; j < this->network[i]->neuronType->inputs.size(); ++j)
            if (this->network[i]->neuronType->inputs[j] == ptr)
                return true;

        for (uint j = 0; j < this->network[i]->projections.size(); ++j) {

            if (this->network[i]->projections[j] == ptr)
                return true;

            for (uint k = 0; k < this->network[i]->projections[j]->synapses.size(); ++k) {

                if (this->network[i]->projections[j]->synapses[k] == ptr)
                    return true;

                for (uint l = 0; l < this->network[i]->projections[j]->synapses[k]->weightUpdateType->inputs.size(); ++l) {

                    if (this->network[i]->projections[j]->synapses[k]->weightUpdateType->inputs[l] == ptr)
                        return true;

                }

                for (uint l = 0; l < this->network[i]->projections[j]->synapses[k]->postsynapseType->inputs.size(); ++l) {

                    if (this->network[i]->projections[j]->synapses[k]->postsynapseType->inputs[l] == ptr)
                        return true;

                }

            }

        }
    }

    // not found
    return false;
}

// allow safe usage of NineMLComponentData pointers
bool projectObject::isValidPointer(NineMLComponentData * ptr) {

    // find the reference
    for (uint i = 0; i < this->network.size(); ++i) {

        if (this->network[i]->neuronType == ptr)
            return true;

        for (uint j = 0; j < this->network[i]->projections.size(); ++j) {

            for (uint k = 0; k < this->network[i]->projections[j]->synapses.size(); ++k) {

                if (this->network[i]->projections[j]->synapses[k]->weightUpdateType == ptr)
                    return true;

                if (this->network[i]->projections[j]->synapses[k]->postsynapseType == ptr)
                        return true;


            }

        }
    }

    // not found
    return false;
}

// allow safe usage of NineMLComponent pointers
bool projectObject::isValidPointer(NineMLComponent * ptr) {

    for (uint i = 0; i < this->catalogNB.size(); ++i)
        if (catalogNB[i] == ptr)
            return true;
    for (uint i = 0; i < this->catalogPS.size(); ++i)
        if (catalogPS[i] == ptr)
            return true;
    for (uint i = 0; i < this->catalogGC.size(); ++i)
        if (catalogGC[i] == ptr)
            return true;
    for (uint i = 0; i < this->catalogWU.size(); ++i)
        if (catalogWU[i] == ptr)
            return true;

    // not found
    return false;
}

NineMLComponentData * projectObject::getComponentDataFromName(QString name)
{
    // find the ComponentData requested
    for (uint i = 0; i < this->network.size(); ++i) {
        if (this->network[i]->neuronType->getXMLName() == name) {
            // found - return the ComponentData
            return this->network[i]->neuronType;
        }
        for (uint j = 0; j < this->network[i]->projections.size(); ++j) {
            // current projection
            projection * proj = this->network[i]->projections[j];
            for (uint k = 0; k < proj->synapses.size(); ++k) {
                // current synapse
                synapse * syn = proj->synapses[k];
                if (syn->weightUpdateType->getXMLName() == name) {
                    // found - return the ComponentData
                    return syn->weightUpdateType;
                }
                if (syn->postsynapseType->getXMLName() == name) {
                    // found - return the ComponentData
                    return syn->postsynapseType;
                }
            }
        }
    }

    // not found
    return NULL;
}

QAction * projectObject::action(int i) {

    menuAction->setProperty("number", QString::number(i));
    menuAction->setText(this->name);
    menuAction->setCheckable(true);
    return menuAction;

}

QString projectObject::getUniquePopName(QString newName) {

    int j = 0;

    bool nameGood = false;

    while (!nameGood) {
        // assume name is good
        nameGood = true;
        ++j;
        QString testName = newName;
        testName.append(" dup" + QString::number(j));

        // check name against populations
        for (unsigned int k = 0; k < this->network.size(); ++k) {
            if (this->network[k]->name.compare(testName) == 0) {
                nameGood = false;
            }
        }
    }
    // we have the name - we can exit
    newName.append(" dup" + QString::number(j));

    return newName;
}
