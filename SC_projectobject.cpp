#include "SC_projectobject.h"
#include "CL_classes.h"
#include "CL_layout_classes.h"
#include "SC_network_layer_rootdata.h"
#include "mainwindow.h"
#include "SC_versioncontrol.h"
#include "EL_experiment.h"
#include "SC_systemmodel.h"

projectObject::projectObject(QObject *parent) :
    QObject(parent)
{
    this->name = "New Project";

    this->menuAction = new QAction(this);

    this->undoStack = new QUndoStack(this);

    // Screen cursor pos initialised in the nl_rootdata object to 0,0 also.
    //this->currentCursorPos.x = 0.0;
    //this->currentCursorPos.y = 0.0;

    // default fileNames
    this->networkFile = "model.xml";
#ifdef KEEP_OLD_STYLE_METADATA_XML_FILE_LOADING_FOR_COMPATIBILITY
    // On loading an old-style project, this->metaFile is set up from the project XML file.
    this->metaFile = "";
#endif

    // create the catalog blank entries:
    this->catalogGC.push_back(QSharedPointer<Component> (new Component()));
    this->catalogGC[0]->name = "none";
    this->catalogGC[0]->type = "moo";
    this->catalogNB.push_back(QSharedPointer<Component> (new Component()));
    this->catalogNB[0]->name = "none";
    this->catalogNB[0]->type = "neuron_body";
    this->catalogWU.push_back(QSharedPointer<Component> (new Component()));
    this->catalogWU[0]->name = "none";
    this->catalogWU[0]->type = "weight_update";
    this->catalogPS.push_back(QSharedPointer<Component> (new Component()));
    this->catalogPS[0]->name = "none";
    this->catalogPS[0]->type = "postsynapse";
    this->catalogLAY.push_back(QSharedPointer<NineMLLayout> (new NineMLLayout()));
    this->catalogLAY[0]->name = "none";

#ifdef CLEANUP_AT_PROJECT_OPEN
    // NO, not here, do this in mainwindow.cpp when the PROGRAM
    //  starts, not when a project is opened. If it's cleaned up here,
    //  then a second opened project will remove the files needed by
    //  the first opened project.
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    QDir lib_dir = QDir(QDesktopServices::storageLocation(QDesktopServices::DataLocation));
#else
    QDir lib_dir = QDir(QStandardPaths::writableLocation(QStandardPaths::DataLocation));
#endif
    lib_dir.setFilter(QDir::Files);
    foreach(QString dirFile, lib_dir.entryList()) {
        lib_dir.remove(dirFile);
    }
#endif
}

projectObject::~projectObject()
{
    // clean up these
    delete this->undoStack;
    delete this->menuAction;

    // destroy experiments
    for (int i = 0; i < this->experimentList.size(); ++i) {
        delete this->experimentList[i];
    }

    for (int i = 0; i < network.size(); ++i) {
        for (int j = 0; j < network[i]->projections.size(); ++j) {
            for (int k = 0; k < network[i]->projections[j]->synapses.size(); ++k) {
                network[i]->projections[j]->synapses[k].clear();
            }
            network[i]->projections[j].clear();
        }
        network[i].clear();
    }

    // delete catalog components
    for (int i = 0; i < this->catalogLAY.size(); ++i) {
        this->catalogLAY[i].clear();
    }
    for (int i = 0; i < this->catalogNB.size(); ++i) {
        this->catalogNB[i].clear();
    }
    for (int i = 0; i < this->catalogPS.size(); ++i) {
        this->catalogPS[i].clear();
    }
    for (int i = 0; i < this->catalogWU.size(); ++i) {
        this->catalogWU[i].clear();
    }
    for (int i = 0; i < this->catalogGC.size(); ++i) {
        this->catalogGC[i].clear();
    }

    // clear catalog vectors
    this->catalogLAY.clear();
    this->catalogNB.clear();
    this->catalogPS.clear();
    this->catalogWU.clear();
    this->catalogGC.clear();
}

QString projectObject::getFilenameFriendlyName (void)
{
    // Make nameFname directory-friendly, replace spaces with '_'
    QString nameFname = this->name;
    nameFname.replace(' ', '_');
    return nameFname;
}

bool projectObject::open_project(QString fileName)
{
    QDir project_dir(fileName);

    // remove filename
    project_dir.cdUp();

    QSettings settings;
    settings.setValue("files/currentFileName", project_dir.absolutePath());

    // Set currentCursorPos to 0 before opening a project to ensure we
    // don't translate anything in position.
    this->currentCursorPos.x = 0;
    this->currentCursorPos.y = 0;

    // first try and open the project file
    if (!this->load_project_file(fileName)) {
        printErrors("Errors found loading the project file:");
        return false;
    }

    // then load in all the components listed in the project file
    for (int i = 0; i < this->components.size(); ++i) {
        this->loadComponent(this->components[i], project_dir);
    }
    printErrors("Errors found loading project Components:");

    // then load in all the layouts listed in the project file
    for (int i = 0; i < this->layouts.size(); ++i) {
        this->loadLayout(this->layouts[i], project_dir);
    }
    printErrors("Errors found loading project Layouts:");

    // now the network
    this->loadNetwork(this->networkFile, project_dir);
    if (printErrors("Errors prevented loading the Project:")) {
        return false;
    }

    // finally the experiments (this->experiments populated in this->load_project_file)
    for (int i = 0; i < this->experiments.size(); ++i) {
        loadExperiment(this->experiments[i], project_dir);
    }
    printErrors("Errors found loading project Experiments:");

    // check for errors
    printWarnings("Issues were found while loading the project:");

    // store the new file name
    this->filePath = fileName;

    return true;
}

bool projectObject::save_project(QString fileName, nl_rootdata * data)
{
    if (!fileName.contains(".")) {
        QMessageBox msgBox;
        msgBox.setText("Project file needs .proj suffix.");
        msgBox.exec();
        return false;
    }
    DBG() << "save_project ('" << fileName << "', rootData*)";

    QDir project_dir(fileName);

    // remove filename
    project_dir.cdUp();

    // check for version control
    this->version.setupVersion();

    // No longer remove explicitDataBinaryFiles on save - we'll
    // overwrite those files which need overwriting, and we'll use the
    // files present in the directory to help choose new names for new
    // property/explicitDataBinaryFiles.
    //
    // However, we DO remove old connection binary files (but not
    // explicitData binary files).
    project_dir.setNameFilters(QStringList() << "conn*.bin");
    QStringList files = project_dir.entryList(QDir::Files);
    for (int i = 0; i < files.size(); ++i) {
        // delete
        project_dir.remove(files[i]);
        // and remove from version control
        if (this->version.isModelUnderVersion()) {
            this->version.removeFromVersion(files[i]);
        }
    }

    // sync project
    copy_back_data(data);

    // write project file
    if (!save_project_file(fileName)) {
        return false;
    }

    // write components
    for (int i = 1; i < this->catalogNB.size(); ++i) {
        saveComponent(this->catalogNB[i]->getXMLName(), project_dir, this->catalogNB[i]);
    }
    for (int i = 1; i < this->catalogWU.size(); ++i) {
        saveComponent(this->catalogWU[i]->getXMLName(), project_dir, this->catalogWU[i]);
    }
    for (int i = 1; i < this->catalogPS.size(); ++i) {
        saveComponent(this->catalogPS[i]->getXMLName(), project_dir, this->catalogPS[i]);
    }
    for (int i = 1; i < this->catalogGC.size(); ++i) {
        saveComponent(this->catalogGC[i]->getXMLName(), project_dir, this->catalogGC[i]);
    }

    // write layouts
    for (int i = 1; i < this->catalogLAY.size(); ++i) {
        saveLayout(this->catalogLAY[i]->getXMLName(), project_dir, this->catalogLAY[i]);
    }

    // write network
    saveNetwork(this->networkFile, project_dir);

    // write experiments
    for (int i = 0; i < this->experimentList.size(); ++i) {
        saveExperiment("experiment" + QString::number(i) + ".xml", project_dir, this->experimentList[i]);
    }

    // copy additional files
    for (int i = 0; i < this->additionalFiles.size(); ++i) {
        // copy additionalFiles[i] to project_dir / additionalFiles[i].fileName()
        QFileInfo fileInfo(this->additionalFiles[i]);
        QFile::copy(additionalFiles[i], project_dir.absolutePath() + QDir::separator() + fileInfo.fileName());
    }

    // store the new file name
    this->filePath = fileName;

    if (printErrors("Errors found")) {
        return false;
    }
    if (printWarnings("Warnings found")) {
        return false;
    }

    this->undoStack->setClean();

    return true;
}

bool projectObject::import_network(QString fileName, cursorType cursorPos)
{
    DBG() << "projectObject::import_network(" << fileName << ")";

#ifdef KEEP_OLD_STYLE_METADATA_XML_FILE_LOADING_FOR_COMPATIBILITY
    // In case there is a metaFile set, make a copy of it.
    QString metaFileCopy(this->metaFile);
#endif

    // Compute the extent of the existing network:
    std::pair<QPointF, QPointF> ext = this->getNetworkExtent (this->network);
    DBG() << "Network extent: TL:" << ext.first << " BR: " << ext.second;

    QDir project_dir(fileName);

    // Update current cursor position, used to offset the imported
    // network (so it won't land on top of the existing network).
    this->currentCursorPos = cursorPos;

    // remove filename
    project_dir.cdUp();

    // Set currentFileName
    QSettings settings;
    settings.setValue("files/currentFileName", project_dir.absolutePath());

    // get a list of all the files in the directory containing fileName
    QStringList files = project_dir.entryList();

    // load all the component files
    for (int i = 0; i < files.size(); ++i) {
        if (isComponent(project_dir.absoluteFilePath(files[i]))) {
            this->loadComponent(files[i], project_dir);
        }
    }

    // load all the layout files
    for (int i = 0; i < files.size(); ++i) {
        if (isLayout(project_dir.absoluteFilePath(files[i]))) {
            this->loadLayout(files[i], project_dir);
        }
    }

    int firstNewPop = this->network.size();

    // load the network file itself
    this->loadNetwork(fileName, project_dir, false);
    if (printErrors("Errors prevented importing the Network:")) {
        return false;
    }

    // set up metaData if not loaded. That's _this_ metaFile. We need
    // to load the network making use of the metadata that comes
    // alongside (old format) or inside (new format) the model.xml
    // file. loadNetwork() can set this->metaFile to "not found".
#ifdef KEEP_OLD_STYLE_METADATA_XML_FILE_LOADING_FOR_COMPATIBILITY
    if (this->metaFile == "not found") {
        this->metaFile = metaFileCopy;
#endif
        // place the new populations in a diagonal line:
        for (int i = firstNewPop; i < this->network.size(); ++i) {
            QSharedPointer <population> p = this->network[i];

            // Adds a bit to x and y positions, leaving the existing
            // network populations unchanged and applying the cursor
            // position offset - that is, the diagonal line of new
            // populations starts at the cursor and is directed up and
            // right.
            p->x = (i-firstNewPop)*2.0f + this->currentCursorPos.x;
            p->targx = p->x;
            p->y = (i-firstNewPop)*2.0f + this->currentCursorPos.y;
            p->targy = p->y;

            p->size = 1.0f;
            p->aspect_ratio = 5.0f/3.0f;
            p->setupBounds();
        }

        // make projection and generic input curves to link up the
        // newly placed populations
        for (int i = firstNewPop; i < this->network.size(); ++i) {

            DBG() << "Placing " << this->network[i]->projections.size() << " projections for population in network["<<i<<"]";
            for (int j = 0; j < this->network[i]->projections.size(); ++j) {
                this->network[i]->projections[j]->add_curves();
            }

            DBG() << "Placing " << this->network[i]->neuronType->inputs.size() << " inputs for network["<<i<<"]";
            for (int j = 0; j < this->network[i]->neuronType->inputs.count(); ++j) {
                this->network[i]->neuronType->inputs[j]->add_curves();
            }
        }
#ifdef KEEP_OLD_STYLE_METADATA_XML_FILE_LOADING_FOR_COMPATIBILITY
    }
#endif

    // finally load the experiments. This will load ALL experiment
    // files in the directory, which may include some stale ones,
    // which will cause errors.
    for (int i = 0; i < files.size(); ++i) {
        this->loadExperiment(files[i], project_dir, true);
    }

    printWarnings("Issues found importing the Network:");
    printErrors("Errors found importing the Network:");

    return true;
}

void projectObject::import_component(QString fileName)
{
    QDir project_dir(fileName);
    project_dir.cdUp(); // removes filename
    loadComponent(fileName, project_dir);
}

void projectObject::import_layout(QString fileName)
{
    QDir project_dir(fileName);
    project_dir.cdUp();
    loadLayout(fileName, project_dir);
}

bool projectObject::load_project_file(QString fileName)
{
    // open the file
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox msgBox;
        msgBox.setText("Could not open the project file");
        msgBox.exec();
        return false;
    }

    // Extract project_dir from project filename
    QDir project_dir(fileName);
    project_dir.cdUp();

    // get a streamreader
    QXmlStreamReader * reader = new QXmlStreamReader;
    reader->setDevice(&file);

    // read elements
    while (reader->readNextStartElement()) {

        if (reader->name() == "SpineCreatorProject") {

            while (reader->readNextStartElement()) {

                if (reader->name() == "Network") {

                    while (reader->readNextStartElement()) {

                        if (reader->name() == "File") {

                            if (reader->attributes().hasAttribute("name")) {
                                this->networkFile = reader->attributes().value("name").toString();
                            } else {
                                QSettings settings;
                                int num_errs = settings.beginReadArray("errors");
                                settings.endArray();
                                settings.beginWriteArray("errors");
                                settings.setArrayIndex(num_errs + 1);
                                settings.setValue("errorText", "XML Error in Project File - missing attribute 'name'");
                                settings.endArray();
                            }
#ifdef KEEP_OLD_STYLE_METADATA_XML_FILE_LOADING_FOR_COMPATIBILITY
                            if (reader->attributes().hasAttribute("metaFile")) {
                                this->metaFile = reader->attributes().value("metaFile").toString();
                            } // else expect to be using new in-model LL:Annotation format
#endif
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

                            if (reader->attributes().hasAttribute("name")) {
                                this->components.push_back(reader->attributes().value("name").toString());
                            } else {
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

                            if (reader->attributes().hasAttribute("name")) {
                                this->layouts.push_back(reader->attributes().value("name").toString());
                            } else {
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

                            if (reader->attributes().hasAttribute("name")) {
                                this->experiments.push_back(reader->attributes().value("name").toString());
                            } else {
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
                } else if (reader->name() == "AdditionalFiles") {

                    while (reader->readNextStartElement()) {

                        if (reader->name() == "File") {

                            if (reader->attributes().hasAttribute("name")) {
                                // Note that we store the additional file as a full path.
                                this->additionalFiles.push_back(project_dir.absolutePath() + QDir::separator() + reader->attributes().value("name").toString());
                            } else {
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

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox msgBox;
        msgBox.setText("Could not create the project file '" + fileName + "'");
        msgBox.exec();
        return false;
    }

    // get a streamwriter
    QXmlStreamWriter * writer = new QXmlStreamWriter;
    writer->setDevice(&file);

    // write elements
    writer->writeStartDocument();
    writer->writeStartElement("SpineCreatorProject");

    writer->writeStartElement("Network");
    writer->writeEmptyElement("File");
    writer->writeAttribute("name", this->networkFile);
#if 0 // In new format, metadata is stored in model.xml (and component.xml files too)
    writer->writeAttribute("metaFile", this->metaFile);
#endif
    writer->writeEndElement(); // Network

    writer->writeStartElement("Components");
    for (int i = 1; i < this->catalogNB.size(); ++i) {
        writer->writeEmptyElement("File");
        writer->writeAttribute("name", this->catalogNB[i]->getXMLName());
    }
    for (int i = 1; i < this->catalogWU.size(); ++i) {
        writer->writeEmptyElement("File");
        writer->writeAttribute("name", this->catalogWU[i]->getXMLName());
    }
    for (int i = 1; i < this->catalogPS.size(); ++i) {
        writer->writeEmptyElement("File");
        writer->writeAttribute("name", this->catalogPS[i]->getXMLName());
    }
    for (int i = 1; i < this->catalogGC.size(); ++i) {
        writer->writeEmptyElement("File");
        writer->writeAttribute("name", this->catalogGC[i]->getXMLName());
    }
    writer->writeEndElement(); // Components

    writer->writeStartElement("Layouts");
    for (int i = 0; i < this->catalogLAY.size(); ++i) {
        writer->writeEmptyElement("File");
        writer->writeAttribute("name", this->catalogLAY[i]->getXMLName());
    }
    writer->writeEndElement(); // Layouts

    writer->writeStartElement("Experiments");
    for (int i = 0; i < this->experimentList.size(); ++i) {
        writer->writeEmptyElement("File");
        writer->writeAttribute("name", "experiment" + QString::number(i) + ".xml");
    }
    writer->writeEndElement(); // Experiments

    if (!this->additionalFiles.isEmpty()) {
        writer->writeStartElement("AdditionalFiles");
        for (int i = 0; i < this->additionalFiles.size(); ++i) {
            QFileInfo fileInfo(this->additionalFiles[i]);
            writer->writeEmptyElement("File");
            writer->writeAttribute("name", fileInfo.fileName());
        }
        writer->writeEndElement(); // AdditionalFiles
    }

    writer->writeEndElement(); // SpineCreatorProject

    // add to version control
    if (this->version.isModelUnderVersion()) {
        this->version.addToVersion(file.fileName());
    }

    return true;
}

bool projectObject::isComponent(QString fileName)
{
    // try opening the file and loading the XML
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    if (!this->doc.setContent(&file)) {
        return false;
    }

    // we have loaded the XML - discard the file handle
    file.close();

    // confirm root tag is correct
    QDomElement root = this->doc.documentElement();
    if (root.tagName() != "SpineML" ) {
        this->doc.clear();
        return false;
    }

    // if a componentclass
    QDomElement classType = root.firstChildElement();
    if (classType.tagName() != "ComponentClass") {
        this->doc.clear();
        return false;
    }

    // clean up
    this->doc.clear();
    return true;
}

bool projectObject::isLayout(QString fileName)
{
    // try opening the file and loading the XML
    QFile file(fileName);
    if(!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    if(!this->doc.setContent(&file)) {
        return false;
    }

    // we have loaded the XML - discard the file handle
    file.close();

    // confirm root tag is correct
    QDomElement root = this->doc.documentElement();
    if (root.tagName() != "SpineML") {
        this->doc.clear();
        return false;
    }

    // if a componentclass
    QDomElement classType = root.firstChildElement();
    if (classType.tagName() != "LayoutClass") {
        this->doc.clear();
        return false;
    }

    // clean up
    this->doc.clear();

    return true;
}

void projectObject::loadComponent(QString fileName, QDir project_dir)
{
    if (fileName == "none.xml") {
        return;
    }

    // try opening the file and loading the XML
    QFile file(project_dir.absoluteFilePath(fileName));
    if (!file.open(QIODevice::ReadOnly)) {
        addError("Cannot open required file '" + fileName + "'");
        return;
    }
    if (!this->doc.setContent(&file)) {
        addError("Cannot read required file '" + fileName + "'");
        return;
    }

    // confirm root tag is correct
    QDomElement root = this->doc.documentElement();
    if (root.tagName() != "SpineML" ) {
        addError("Missing or incorrect root tag in required file '" + fileName + "'");
        return;
    }

    // if a componentclass
    QDomElement classType = root.firstChildElement();

    if (classType.tagName() == "ComponentClass") {

        // HANDLE SPINEML COMPONENTS //

        // create a new AL class instance and populate it from the data
        QSharedPointer<Component>tempALobject = QSharedPointer<Component> (new Component());

        tempALobject->load(&this->doc);

        // check for errors:
        QSettings settings;
        int num_errs = settings.beginReadArray("errors");
        settings.endArray();

        // if there are errors then clean up and leave
        if (num_errs != 0) {
            tempALobject.clear();
            // write tail for errors:
            addError("<b>IN COMPONENT FILE '" + fileName + "'</b>");
            return;
        }

        // get lib to add component to
        QVector < QSharedPointer<Component> > * curr_lib;
        if (tempALobject->type == "neuron_body") {
            curr_lib = &this->catalogNB;
        } else if (tempALobject->type == "weight_update") {
            curr_lib = &this->catalogWU;
        } else if (tempALobject->type == "postsynapse") {
            curr_lib = &this->catalogPS;
        } else {
            curr_lib = &this->catalogGC;
        }

        // check the name doesn't already exist in the library
        for (int i = 0; i < curr_lib->size(); ++i) {
            if ((*curr_lib)[i]->name == tempALobject->name
                && (*curr_lib)[i]->path == tempALobject->path
                && tempALobject->name != "none") {
                // same name
                QString ees = "Two required files have the same Component Name (" + (tempALobject->path) + "/" + (tempALobject->name) +  "). This project may be corrupted";
                addWarning(ees);
                tempALobject.clear();
                return;
            }
        }

        // add to the correct catalog
        curr_lib->push_back(tempALobject);

    } else {
        addError("Unknown XML tag found in required file '" + fileName + "'");
    }
}

void projectObject::saveComponent(QString fileName, QDir project_dir, QSharedPointer<Component> component)
{
    // if no extension then append a .xml
    if (!fileName.contains(".")) {
        fileName.append(".xml");
    }

    QString fname = project_dir.absoluteFilePath(fileName);
    QFile file(fname);
    if (!file.open(QIODevice::WriteOnly)) {
        addError("saveComponent: Error creating file for '" + fname + "' - is there sufficient disk space?");
        return;
    }

    this->doc.setContent(QString(""));

    // get the 9ML description
    component->write(&this->doc);

    // write out to file
    QTextStream tsFromFile(&file );
    tsFromFile << this->doc.toString();
    tsFromFile.flush();

    // add to version control
    if (this->version.isModelUnderVersion()) {
        this->version.addToVersion(file.fileName());
    }

    file.close();

    // store path for easy access
    component->filePath = project_dir.absoluteFilePath(fileName);

    // kill off the DOM document
    this->doc.clear();
}

void projectObject::loadLayout(QString fileName, QDir project_dir)
{
    if (fileName == "none.xml") {
        return;
    }

    // try opening the file and loading the XML
    QFile file(project_dir.absoluteFilePath(fileName));
    if (!file.open(QIODevice::ReadOnly)) {
        addError("Cannot open required file '" + fileName + "'");
        return;
    }
    if (!this->doc.setContent(&file)) {
        addError("Cannot read required file '" + fileName + "'");
        return;
    }

    // confirm root tag is correct
    QDomElement root = this->doc.documentElement();
    if (root.tagName() != "SpineML" ) {
        addError("Missing or incorrect root tag in required file '" + fileName + "'");
        return;
    }

    // if a componentclass
    QDomElement classType = root.firstChildElement();

    if (classType.tagName() == "LayoutClass") {

            // HANDLE LAYOUTS

            // create a new AL class instance and populate it from the data
            QSharedPointer<NineMLLayout>tempALobject = QSharedPointer<NineMLLayout> (new NineMLLayout());

            tempALobject->load(&this->doc);

            // check for errors:
            QSettings settings;
            int num_errs = settings.beginReadArray("errors");
            settings.endArray();

            if (num_errs != 0) {
                tempALobject.clear();
                // tail for errors:
                addError("<b>IN LAYOUT FILE '" + fileName + "'</b>");
                return;
            }

            for (int i = 0; i < this->catalogLAY.size(); ++i) {
                if (this->catalogLAY[i]->name.compare(tempALobject->name) == 0 && tempALobject->name != "none") {
                    // same name
                    addWarning("Two required files have the same Layout Name - this project may be corrupted");
                    tempALobject.clear();
                    return;
                }
            }

            // all good - add layout to catalog
            this->catalogLAY.push_back(tempALobject);

        } else {
            addError("Unknown XML tag found in required file '" + fileName + "'");
        }

}

void projectObject::saveLayout(QString fileName, QDir project_dir, QSharedPointer<NineMLLayout> layout)
{
    // if no extension then append a .xml
    if (!fileName.contains(".")) {
        fileName.append(".xml");
    }

    QFile file(project_dir.absoluteFilePath(fileName));
    if (!file.open(QIODevice::WriteOnly)) {
        addError("Error creating file for '" + fileName + "' - is there sufficient disk space?");
        return;
    }

    this->doc.setContent(QString(""));

    // get the 9ML description
    layout->write(&this->doc);

    // write out to file
    QTextStream tsFromFile(&file);
    tsFromFile << this->doc.toString();
    tsFromFile.flush();

    // add to version control
    if (this->version.isModelUnderVersion()) {
        this->version.addToVersion(file.fileName());
    }

    file.close();

    // store path for easy access
    layout->filePath = project_dir.absoluteFilePath(fileName);

    // kill off the DOM document
    this->doc.clear();
}

cursorType
projectObject::getCursorPos (void)
{
    return this->currentCursorPos;
}

void projectObject::loadNetwork(QString fileName, QDir project_dir, bool isProject)
{
    // load up the file and check it is valid XML
    QFile file(project_dir.absoluteFilePath(fileName));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        addError("Could not open the Network file for reading");
        return;
    }
    if (!this->doc.setContent(&file)) {
        addError("Could not parse the Network file XML - is the selected file correctly formed XML?");
        return;
    }

    // we have loaded the XML file - discard the file handle
    file.close();

    // confirm root tag is correct
    QDomElement root = this->doc.documentElement();
    if (root.tagName() != "LL:SpineML") {
        addError("Network file is not valid SpineML Low Level Network Layer description");
        return;
    }

    // get the model name
    this->name = root.toElement().attribute("name", "Untitled project");

#ifdef KEEP_OLD_STYLE_METADATA_XML_FILE_LOADING_FOR_COMPATIBILITY
    // Cursor offset is applied when loading metadata. Should be 0 when opening a new project.
    //DBG() << "Cursor position is " << this->currentCursorPos.x << "," << this->currentCursorPos.y;

    // only load metadata for projects which have a metaFile path
    if (!this->metaFile.isEmpty()) {
        QString metaFilePath = project_dir.absoluteFilePath(this->metaFile);
        QFile fileMeta(metaFilePath);

        if (!fileMeta.open(QIODevice::ReadOnly)) {
            // if is not a project we don't expect a metaData file
            if (isProject) {
                // It is no longer an error not to find a metadata
                // file. (Since addition of annotations code written by
                // Alex 2016-ish and merged by Seb, April 2017.
            } else {
                this->metaFile = "not found";
            }
        } else {
            if (!this->meta.setContent(&fileMeta)) {
                addError("Could not parse the MetaData file XML - is the selected file correctly formed XML?");
                return;
            }

            // we have loaded the XML file - discard the file handle
            fileMeta.close();

            // confirm root tag is correct
            root = this->meta.documentElement();
            if (root.tagName() != "modelMetaData") {
                addError("MetaData file is not valid");
                return;
            }
        }
    }
#endif

    // This is the starting point in this->network from which to count
    // when counting through newly added populations.
    int firstNewPop = this->network.size();

    // LOAD POPULATIONS
    QDomNode n = this->doc.documentElement().firstChild();
    while (!n.isNull())  {

        if (n.isComment()) {
            n = n.nextSibling();
            continue;
        }

        QDomElement e = n.toElement();

        // load any annotations
        if (e.tagName() == "LL:Annotation") {
            QTextStream temp(&this->annotation);
            n.save(temp,1);
        }

        if (e.tagName() == "LL:Population") {
            // add population from population xml:
            QSharedPointer <population> pop = QSharedPointer<population> (new population());
            pop->readFromXML(e, &this->doc, &this->meta, this, pop);
            this->network.push_back(pop);

            // check for duplicate names:
            for (int i = firstNewPop; i < this->network.size() - 1; ++i) {
                if (this->network[i]->name == this->network.back()->name) {
                    this->network[i]->name = getUniquePopName(this->network[i]->name);
                    addWarning("Duplicate Population name found: renamed existing Population to '" + this->network[i]->name + "'");
                }
            }

            // check for errors:
            QSettings settings;
            int num_errs = settings.beginReadArray("errors");
            settings.endArray();

            if (num_errs != 0) {
                // no dice - give up!
                DBG() << "There were errors reading the population. Giving up and returning.";
                return;
            }

        }
        n = n.nextSibling();
    }

#ifdef __DEBUG_LOAD_NETWORK
    // Some debugging information:
    DBG() << "Order of network names:";
    for (int i = 0; i < this->network.size(); ++i) {
            DBG() << "Population name: " << this->network[i]->name;
            DBG() << "Population Component name: " << this->network[i]->neuronType->getXMLName();
    }
#endif

    // LOAD PROJECTIONS
#ifdef __DEBUG_LOAD_NETWORK
    DBGBRK()
    DBG() << "LOADING PROJECTIONS...";
#endif
    n = this->doc.documentElement().firstChild();
    int counter = firstNewPop;
    while (!n.isNull()) {

        if (n.isComment()) {
            n = n.nextSibling();
            continue;
        }

        QDomElement e = n.toElement();
        if (e.tagName() == "LL:Population" ) {
            // with all the populations added, add the projections and join them up:
            this->network[counter]->load_projections_from_xml(e, &this->doc, &this->meta, this);
#ifdef __DEBUG_LOAD_NETWORK
            DBG() << "After load_projections_from_xml, network["<<counter<<"]->projections.count is " << this->network[counter]->projections.count();
#endif
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

    // LOAD INPUTS
#ifdef __DEBUG_LOAD_NETWORK
    DBGBRK();
    DBG() << "LOADING INPUTS...";
#endif
    counter = firstNewPop;
    n = this->doc.documentElement().firstChild();
    while (!n.isNull()) {

        if (n.isComment()) {
            n = n.nextSibling();
            continue;
        }

        QDomElement e = n.toElement();
        if (e.tagName() == "LL:Population" ) {
            {
                QDomNodeList allneurons = e.elementsByTagName("LL:Neuron");
                QDomElement firstneuron = allneurons.item(0).toElement();
#ifdef __DEBUG_LOAD_NETWORK
                DBG() << "CC Population name of first Neuron element:" << firstneuron.attribute("name", "unknown");
#endif
            }

            // add inputs
#ifdef __DEBUG_LOAD_NETWORK
            DBG() << "Adding inputs from LL:Population element with network->read_inputs_from_xml. e.text(): " << e.text();
            DBG() << "this->network[counter]->neuronType->getName():" << this->network[counter]->neuronType->getXMLName();
            // neuronType is a ComponentInstance, owner is a systemObject.
            DBG() << "this->network[counter]->neuronType->owner->getName():" << this->network[counter]->neuronType->owner->getName();
#endif
            // network is a QVector of pointers to populations. Calls
            // NL_population::read_inputs_from_xml to read the inputs to the populations
            this->network[counter]->read_inputs_from_xml(e, &this->meta, this);

#ifdef __DEBUG_LOAD_NETWORK
            DBG() << "After read_inputs_from_xml, network["<<counter<<"]->inputs.count is "
                  << this->network[counter]->neuronType->inputs.count();

            DBG() << "Check destination/source for population in network["<<counter<<"] which has "
                  << this->network[counter]->neuronType->inputs.count() << " inputs";
#endif
            // Now read inputs to the projections
            int projCount = 0;
            QDomNodeList n2 = n.toElement().elementsByTagName("LL:Projection");
#ifdef __DEBUG_LOAD_NETWORK
            DBG() << "Now read inputs for each of " << n2.size() << " projections in the population in network["<<counter<<"]...";
#endif
            for (int i = 0; i < (int) n2.size(); ++i) {
                QDomElement proj_e = n2.item(i).toElement();
#ifdef __DEBUG_LOAD_NETWORK
                DBG() << "Calling projections->read_inputs_from_xml...";
#endif
                // The problem is that this->network[counter]->projections[projCount] has NO synapses.
                this->network[counter]->projections[projCount]->read_inputs_from_xml(proj_e, &this->meta, this, this->network[counter]->projections[projCount]);
                ++projCount;
            }
            ++counter;
        }
        n = n.nextSibling();
    }

    this->meta.clear();
    this->doc.clear();
}

void projectObject::saveNetwork(QString fileName, QDir projectDir)
{
    QFile fileModel(projectDir.absoluteFilePath(fileName));
    if (!fileModel.open(QIODevice::WriteOnly)) {
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

    // write out the annotations
    if (!this->annotation.isEmpty()) {
        xmlOut.writeStartElement("LL:Annotation");
        // annotations
        this->annotation.replace("\n", "");
        this->annotation.replace("<LL:Annotation>", "");
        this->annotation.replace("</LL:Annotation>", "");
        QXmlStreamReader reader(this->annotation);

        while (!reader.atEnd()) {
            if (reader.tokenType() != QXmlStreamReader::StartDocument && reader.tokenType() != QXmlStreamReader::EndDocument) {
                xmlOut.writeCurrentToken(reader);
            }
            reader.readNext();
        }
        xmlOut.writeEndElement();//LL:Annotation
    }

    // create a node for each population with the variables set
    for (int pop = 0; pop < this->network.size(); ++pop) {
        // WE NEED TO HAVE A PROPER MODEL NAME!
        this->network[pop]->write_population_xml(xmlOut);
    }

    xmlOut.writeEndDocument();

    // add to version control
    if (this->version.isModelUnderVersion()) {
        this->version.addToVersion(fileModel.fileName());
    }

    fileModel.close();

    // Clean up stale explicit data binary files, by searching through
    // xmlOut and comparing with the files in the model dir.
    this->cleanUpStaleExplicitData(fileName, projectDir);
}

void projectObject::cleanUpStaleExplicitData(QString& fileName, QDir& projectDir)
{
    // Make a list of all the explicitDataBinaryFiles in the model.
    QFile modelXmlFile(projectDir.absoluteFilePath(fileName));
    if (!modelXmlFile.open(QIODevice::ReadOnly)) {
        addError("Error reading Network file");
        return;
    }
    QXmlStreamReader modelXml;
    modelXml.setDevice(&(modelXmlFile));
    QStringList ebd_files;
    while(modelXml.readNext() != QXmlStreamReader::EndDocument) {
        if (modelXml.tokenType() == QXmlStreamReader::StartElement
            && modelXml.name() == "BinaryFile") {
            // Examine file_name attribute
            if (modelXml.attributes().hasAttribute("file_name")) {
                QString afile = modelXml.attributes().value("file_name").toString();
                if (afile.contains ("explicitDataBinaryFile")) {
                    ebd_files.push_back (afile);
                }
            }
        }
    }

    // Now we have the list of all explicitDataBinaryFile files which
    // exist in the model, we can see if there are any stale ones in
    // the file store.
    QStringList filters;
    filters << "explicitDataBinaryFile*";
    projectDir.setNameFilters(filters);
    QStringList files = projectDir.entryList();
    for (int i = 0; i < (int)files.count(); ++i) {
        // Is files[i] a member of ebd_files? If NOT then files[i]
        // should be unlinked.
        if (!ebd_files.contains(files[i])) {
            //unlink(files[i])
            DBG() << "Unlinking stale explicitDataBinaryFile: " << files[i];
            QFile::remove(projectDir.absoluteFilePath(files[i]));
        }
    }
}

void projectObject::loadExperiment(QString fileName, QDir project_dir, bool skipFileError)
{
    QFile file(project_dir.absoluteFilePath(fileName));
    if (!file.open(QIODevice::ReadOnly)) {
        if (!skipFileError) {
            addError("Could not open Experiment file '" + fileName + "' for reading");
        }
        return;
    }

    // setup stream reader
    QXmlStreamReader * reader = new QXmlStreamReader;
    reader->setDevice(&file);

    // check we have an experiment
    reader->readNextStartElement();
    reader->readNextStartElement();

    if (reader->name() != "Experiment") {
        if (!skipFileError) {
            addError("Could not parse Experiment file '" + fileName + "'");
        }
        return;
    }

    // reset
    delete reader;
    file.seek(0);

    // set stream reader again
    reader = new QXmlStreamReader;
    reader->setDevice(&file);

    // load experiment:
    experiment * newExperiment = new experiment;
    newExperiment->readXML(reader, this);

    // check for errors:
    QSettings settings;
    int num_errs = settings.beginReadArray("errors");
    settings.endArray();

    if (num_errs == 0) {
        if (this->experimentList.isEmpty()) {
            // If this is the first experiment to be loaded, then make it selected.
            newExperiment->selected = true;
        }
        this->experimentList.push_back(newExperiment);
    } else {
        // add error tail
        addError("<b>" + QString::number(num_errs) + " error" + (num_errs==1?"":"s")
                 + " found in file '" + fileName + "'</b>");
    }

    // we have loaded the XML file; discard file handle & clean up reader
    file.close();
    delete reader;
}

bool projectObject::doesExperimentExist (experiment* e)
{
    // Search QVector < experiment *> experiments;
    QVector<experiment*>::const_iterator ex = this->experimentList.constBegin();
    while (ex != this->experimentList.constEnd()) {
        if ((*ex) == e) {
            return true;
        }
        ++ex;
    }

    return false;
}

void projectObject::saveExperiment(QString fileName, QDir project_dir, experiment * expt)
{
    QString absPath = project_dir.absoluteFilePath(fileName);
    QFile file (absPath);
    if (!file.open(QIODevice::WriteOnly)) {
        addError("Error creating file '" + absPath + "' - is there sufficient disk space?");
        return;
    }

    // use stream writer
    QXmlStreamWriter * xmlOutExpt = new QXmlStreamWriter;
    xmlOutExpt->setDevice(&(file));

    expt->writeXML(xmlOutExpt, this);

    delete xmlOutExpt;

    file.close();

    // add to version control
    if (this->version.isModelUnderVersion()) {
        this->version.addToVersion(fileName);
    }
}

void projectObject::copy_back_data(nl_rootdata * data)
{
    // copy data from rootData to project
    this->network = data->populations;
    this->catalogNB = data->catalogNrn;
    this->catalogWU = data->catalogWU;
    this->catalogPS = data->catalogPS;
    this->catalogGC = data->catalogUnsorted;
    this->catalogLAY = data->catalogLayout;
    this->experimentList = data->experiments;
    this->currentCursorPos = data->cursor;
}

void projectObject::copy_out_data(nl_rootdata * data)
{
    // copy from project to rootData
    data->populations = this->network;
    data->catalogNrn = this->catalogNB;
    data->catalogWU = this->catalogWU;
    data->catalogPS = this->catalogPS;
    data->catalogUnsorted = this->catalogGC;
    data->catalogLayout = this->catalogLAY;
    data->experiments = this->experimentList;
    data->cursor = this->currentCursorPos;
}

void projectObject::deselect_project(nl_rootdata * data)
{
    // move everything back
    copy_back_data(data);

    // clear the selList
    data->selList.clear();
}

void projectObject::select_project(nl_rootdata * data)
{
    // store QTreeWidget state of previous Project
    if (data->main->viewVZ.OpenGLWidget != NULL) {
        data->main->viewVZhandler->saveTreeState();
    }

    // move project data into rootData
    copy_out_data(data);

    QSettings settings;
    settings.remove("files/currentFileName");
    if (this->filePath != "") {
        settings.setValue("files/currentFileName", this->filePath);
    }
    settings.setValue("model/model_name", "New Project");

    // set the undostack
    data->main->undoStacks->setActiveStack(this->undoStack);

    data->currProject = this;

    // update GUI experiment view
    data->main->viewELhandler->redraw();

    // visualiser view - configure TreeView
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

std::pair<QPointF, QPointF>
projectObject::getNetworkExtent (QVector < QSharedPointer <population> >& pops)
{
    // Initialise the return object
    std::pair<QPointF, QPointF> p;
    p.first = QPointF(0,0); // Top-left co-ordinates
    p.second = QPointF(0,0); // Bottom-right co-ordinates

    if (pops.size() == 0) {
        return p;
    }

    // Find top-left and bottom-right coordinates in one sweep through the populations
    p.first.setX(pops[0]->getLeft());
    p.first.setY(pops[0]->getTop());
    p.second.setX(pops[0]->getRight());
    p.second.setY(pops[0]->getBottom());
    for (int i = 0; i < pops.size(); ++i) {
        // Top left extent
        if (pops[i]->getLeft() < p.first.x()) {
            p.first.setX(pops[i]->getLeft());
        }
        if (pops[i]->getTop() > p.first.y()) {
            p.first.setY(pops[i]->getTop());
        }
        // Bottom right extent
        if (pops[i]->getRight() > p.second.x()) {
            p.second.setX(pops[i]->getRight());
        }
        if (pops[i]->getBottom() < p.second.y()) {
            p.second.setY(pops[i]->getBottom());
        }
    }

    return p;
}

void projectObject::printIssues(QString title)
{
    printErrors(title);
    printWarnings(title);
}

bool projectObject::printWarnings(QString title)
{
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

    } else {
        return false;
    }

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

bool projectObject::printErrors(QString title)
{
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
        settings.setProperty("MERR", QString("True"));

    } else {
        return false;
    }

    if (!errors.isEmpty()) {
        // Display errors. (Seb has observed one hang here where the
        // msgBox failed to show when there was a project error.
        DBG() << "Errors in SC_projectobject.cpp: " << errors;
        QMessageBox msgBox;
        msgBox.setText("<P><b>" + title + "</b></P>" + errors);
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setTextFormat(Qt::RichText);
        msgBox.exec();
    }

    return true;
}

void projectObject::addError(QString text)
{
    QSettings settings;
    int num_errs = settings.beginReadArray("errors");
    settings.endArray();
    settings.beginWriteArray("errors");
    settings.setArrayIndex(num_errs + 1);
    settings.setValue("errorText",  text);
    settings.endArray();
}

void projectObject::addWarning(QString text)
{
    QSettings settings;
    int num_errs = settings.beginReadArray("warnings");
    settings.endArray();
    settings.beginWriteArray("warnings");
    settings.setArrayIndex(num_errs + 1);
    settings.setValue("warnText",  text);
    settings.endArray();
}

bool projectObject::isChanged(nl_rootdata * data)
{
    if (data->currProject == this) {
        copy_back_data(data);
    }
    if (!this->undoStack->isClean()) {
         return true;
    }
    for (int i = 1; i < this->catalogNB.size(); ++i) {
        if (!this->catalogNB[i]->undoStack.isClean())
            return true;
    }
    for (int i = 1; i < this->catalogWU.size(); ++i) {
        if (!this->catalogWU[i]->undoStack.isClean())
            return true;
    }
    for (int i = 1; i < this->catalogPS.size(); ++i) {
        if (!this->catalogPS[i]->undoStack.isClean())
            return true;
    }
    for (int i = 1; i < this->catalogGC.size(); ++i) {
        if (!this->catalogGC[i]->undoStack.isClean())
            return true;
    }
    return false;
}

// allow safe usage of systemObject pointers
bool projectObject::isValidPointer(QSharedPointer<systemObject> ptr)
{
    // find the pop / projection / input reference
    for (int i = 0; i < this->network.size(); ++i) {

        if (this->network[i] == ptr) {
            return true;
        }

        for (int j = 0; j < this->network[i]->neuronType->inputs.size(); ++j) {
            if (this->network[i]->neuronType->inputs[j] == ptr) {
                return true;
            }
        }

        for (int j = 0; j < this->network[i]->projections.size(); ++j) {

            if (this->network[i]->projections[j] == ptr) {
                return true;
            }

            for (int k = 0; k < this->network[i]->projections[j]->synapses.size(); ++k) {

                if (this->network[i]->projections[j]->synapses[k] == ptr) {
                    return true;
                }

                for (int l = 0; l < this->network[i]->projections[j]->synapses[k]->weightUpdateCmpt->inputs.size(); ++l) {

                    if (this->network[i]->projections[j]->synapses[k]->weightUpdateCmpt->inputs[l] == ptr) {
                        return true;
                    }
                }

                for (int l = 0; l < this->network[i]->projections[j]->synapses[k]->postSynapseCmpt->inputs.size(); ++l) {

                    if (this->network[i]->projections[j]->synapses[k]->postSynapseCmpt->inputs[l] == ptr) {
                        return true;
                    }
                }
            }
        }
    }

    // pointer not not found, return false
    return false;
}

// allow safe usage of NineMLComponentData pointers
bool projectObject::isValidPointer(QSharedPointer <ComponentInstance> ptr)
{
    // find the reference
    for (int i = 0; i < this->network.size(); ++i) {

        if (this->network[i]->neuronType == ptr) {
            return true;
        }

        for (int j = 0; j < this->network[i]->projections.size(); ++j) {

            for (int k = 0; k < this->network[i]->projections[j]->synapses.size(); ++k) {

                if (this->network[i]->projections[j]->synapses[k]->weightUpdateCmpt == ptr) {
                    return true;
                }

                if (this->network[i]->projections[j]->synapses[k]->postSynapseCmpt == ptr) {
                        return true;
                }
            }
        }
    }

    return false;
}

// allow safe usage of NineMLComponent pointers
bool projectObject::isValidPointer(QSharedPointer<Component> ptr)
{
    for (int i = 0; i < this->catalogNB.size(); ++i) {
        if (this->catalogNB[i] == ptr) {
            return true;
        }
    }
    for (int i = 0; i < this->catalogPS.size(); ++i) {
        if (this->catalogPS[i] == ptr) {
            return true;
        }
    }
    for (int i = 0; i < this->catalogGC.size(); ++i) {
        if (this->catalogGC[i] == ptr) {
            return true;
        }
    }
    for (int i = 0; i < this->catalogWU.size(); ++i) {
        if (this->catalogWU[i] == ptr) {
            return true;
        }
    }

    return false;
}

QSharedPointer <ComponentInstance> projectObject::getComponentDataFromName(QString name)
{
    // find the ComponentData requested
    for (int i = 0; i < this->network.size(); ++i) {
        if (this->network[i]->neuronType->getXMLName() == name) {
            // found - return the ComponentData
            return this->network[i]->neuronType;
        }
        for (int j = 0; j < this->network[i]->projections.size(); ++j) {
            // current projection
            QSharedPointer <projection> proj = this->network[i]->projections[j];
            for (int k = 0; k < proj->synapses.size(); ++k) {
                // current synapse
                QSharedPointer <synapse> syn = proj->synapses[k];
                if (syn->weightUpdateCmpt->getXMLName() == name) {
                    // found - return the ComponentData
                    return syn->weightUpdateCmpt;
                }
                if (syn->postSynapseCmpt->getXMLName() == name) {
                    // found - return the ComponentData
                    return syn->postSynapseCmpt;
                }
            }
        }
    }

    // not found
    QSharedPointer <ComponentInstance> null;
    return null;
}

QAction * projectObject::action(int i)
{
    this->menuAction->setProperty("number", QString::number(i));
    this->menuAction->setText(this->name);
    this->menuAction->setCheckable(true);
    return this->menuAction;
}

QString projectObject::getUniquePopName(QString newName)
{
    int j = 0;

    bool nameGood = false;

    while (!nameGood) {
        // assume name is good
        nameGood = true;
        ++j;
        QString testName = newName;
        testName.append(" dup" + QString::number(j));

        // check name against populations
        for (int k = 0; k < this->network.size(); ++k) {
            if (this->network[k]->name.compare(testName) == 0) {
                nameGood = false;
            }
        }
    }
    // we have the name - we can exit
    newName.append(" dup" + QString::number(j));

    return newName;
}
