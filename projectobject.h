#ifndef PROJECTOBJECT_H
#define PROJECTOBJECT_H

#include <QObject>
#include "globalHeader.h"
#include "versioncontrol.h"

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

    bool export_for_simulator(QString, rootData *);

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
    bool isValidPointer(systemObject *);
    bool isValidPointer(NineMLComponentData *);
    bool isValidPointer(NineMLComponent *);
    QAction * action(int);
    /*!
     * \brief getComponentDataFromName
     * \param name
     * \return
     * Look up a ComponentData by its name - this is used to reconnect pointers after a project is loaded in
     */
    NineMLComponentData* getComponentDataFromName(QString name);

    // info
    QString name;
    QString filePath;

    QString networkFile;
    QString metaFile;
    QStringList components;
    QStringList experiments;
    QStringList layouts;

    // storage for objects
    vector < population * > network;
    vector < NineMLComponent * > catalogNB;
    vector < NineMLComponent * > catalogWU;
    vector < NineMLComponent * > catalogPS;
    vector < NineMLComponent * > catalogGC;

    vector < NineMLLayout * > catalogLAY;

    vector < experiment * > experimentList;

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
    void saveComponent(QString, QDir, NineMLComponent *);
    void loadLayout(QString, QDir);
    void saveLayout(QString, QDir, NineMLLayout *);
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

    QDomDocument doc;
    QDomDocument meta;


signals:
    
public slots:
    
};

#endif // PROJECTOBJECT_H
