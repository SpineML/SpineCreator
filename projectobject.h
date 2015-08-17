#ifndef PROJECTOBJECT_H
#define PROJECTOBJECT_H

#include <QObject>
#include "globalHeader.h"
#include "versioncontrol.h"

// Limit the precision of floating point numbers in metaData.xml to
// avoid the metaData.xml file changing arbitrarily.
#define METADATA_FLOAT_PRECISION 6

class projectObject : public QObject
{
    Q_OBJECT
public:
    explicit projectObject(QObject *parent = 0);
    ~projectObject();

    // menu item
    QAction * menuAction;

    // save and load
    bool open_project(QString);
    bool save_project(QString, rootData *);

    bool import_network(QString);

    void import_component(QString);

    void import_layout(QString);

    bool load_project_file(QString fileName);
    bool save_project_file(QString fileName);

    void copy_back_data(rootData *);
    void copy_out_data(rootData *);
    void deselect_project(rootData *);
    void select_project(rootData *);

    // errors
    void printIssues(QString);

    // general helpers
    bool isChanged(rootData *);
    bool isValidPointer(QSharedPointer<systemObject>);
    bool isValidPointer(QSharedPointer <NineMLComponentData>);
    bool isValidPointer(QSharedPointer<NineMLComponent>);
    QAction * action(int);
    /*!
     * \brief getComponentDataFromName
     * \param name
     * \return
     * Look up a ComponentData by its name - this is used to reconnect pointers after a project is loaded in
     */
    QSharedPointer<NineMLComponentData> getComponentDataFromName(QString name);

    // info
    QString name;
    QString filePath;

    QString networkFile;
    QString metaFile;
    QStringList components;
    QStringList experiments;
    QStringList layouts;

    // storage for objects
    QVector < QSharedPointer <population> > network;
    QVector < QSharedPointer<NineMLComponent> > catalogNB;
    QVector < QSharedPointer<NineMLComponent> > catalogWU;
    QVector < QSharedPointer<NineMLComponent> > catalogPS;
    QVector < QSharedPointer<NineMLComponent> > catalogGC;

    QVector < QSharedPointer<NineMLLayout> > catalogLAY;

    QVector < experiment * > experimentList;

    // features
    versionControl version;
    QUndoStack * undoStack;

    // state of the visualizer QTreeWidget
    QStringList treeWidgetState;

private:

    // load helpers
    bool isComponent(QString);
    bool isLayout(QString);
    void loadComponent(QString, QDir);
    void saveComponent(QString, QDir, QSharedPointer<NineMLComponent>);
    void loadLayout(QString, QDir);
    void saveLayout(QString, QDir, QSharedPointer<NineMLLayout>);
    void loadNetwork(QString, QDir, bool isProject = true);
    void saveNetwork(QString, QDir);
    void saveMetaData(QString, QDir);
    void loadExperiment(QString, QDir, bool skipFileError = false);
    void saveExperiment(QString, QDir, experiment *);

    // error handling
    bool printWarnings(QString);
    bool printErrors(QString);
    void addError(QString);
    void addWarning(QString);

    // other helper
    QString getUniquePopName(QString);

    /*!
     * Looks in modelXml and finds all the explicitDataBinaryFile
     * file_names. It then checks these are all present and removes
     * any stale ones from the model file directory.
     */
    void cleanUpStaleExplicitData(QString& fileName, QDir& projectDir);

    QDomDocument doc;
    QDomDocument meta;


signals:

public slots:

};

#endif // PROJECTOBJECT_H
