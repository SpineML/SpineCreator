#ifndef PROJECTOBJECT_H
#define PROJECTOBJECT_H

#include <QObject>
#include "globalHeader.h"
#include "SC_versioncontrol.h"

// Limit the precision of floating point numbers in metaData.xml to
// avoid the metaData.xml file changing arbitrarily.
#define METADATA_FLOAT_PRECISION 6

/*!
 * \brief The projectObject class stores the data for a project that is not currently selected.
 *
 * When projects
 * are selected the data for the deselected project is stored into the project object and the data from the
 * selected project object is loaded into the network rootdata class. Data is also stored for save events.
 */
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
    bool save_project(QString, nl_rootdata *);

    bool import_network(QString, cursorType);

    void import_component(QString);

    void import_layout(QString);

    bool load_project_file(QString fileName);
    bool save_project_file(QString fileName);

    void copy_back_data(nl_rootdata *);
    void copy_out_data(nl_rootdata *);
    void deselect_project(nl_rootdata *);
    void select_project(nl_rootdata *);

    // errors
    void printIssues(QString);

    // general helpers
    bool isChanged(nl_rootdata *);
    bool isValidPointer(QSharedPointer<systemObject>);
    bool isValidPointer(QSharedPointer <ComponentInstance>);
    bool isValidPointer(QSharedPointer<Component>);
    QAction * action(int);
    /*!
     * \brief getComponentDataFromName
     * \param name
     * \return
     * Look up a ComponentData by its name - this is used to reconnect pointers after a project is loaded in
     */
    QSharedPointer<ComponentInstance> getComponentDataFromName(QString name);

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
    QVector < QSharedPointer<Component> > catalogNB;
    QVector < QSharedPointer<Component> > catalogWU;
    QVector < QSharedPointer<Component> > catalogPS;
    QVector < QSharedPointer<Component> > catalogGC;

    QVector < QSharedPointer<NineMLLayout> > catalogLAY;

    QVector < experiment * > experimentList;

    // features
    versionControl version;
    QUndoStack * undoStack;

    // state of the visualizer QTreeWidget
    QStringList treeWidgetState;

    cursorType getCursorPos (void);

private:

    cursorType currentCursorPos;

    // load helpers
    bool isComponent(QString);
    bool isLayout(QString);
    void loadComponent(QString, QDir);
    void saveComponent(QString, QDir, QSharedPointer<Component>);
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
