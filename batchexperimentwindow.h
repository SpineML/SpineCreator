#ifndef BATCHEXPERIMENTWINDOW_H
#define BATCHEXPERIMENTWINDOW_H

#include <QDialog>
#include "vectorlistmodel.h"
#include "experiment.h"
#include "viewGVpropertieslayout.h"

namespace Ui {
class BatchExperimentWindow;
}

class BatchExperimentWindow : public QDialog
{
    Q_OBJECT

public:
    explicit BatchExperimentWindow(experiment * currExpt, viewELExptPanelHandler *eH, viewGVpropertieslayout * logs, QWidget *parent = 0);
    ~BatchExperimentWindow();

private:
    Ui::BatchExperimentWindow *ui;
    vectorListModel * model;
    experiment * currExpt;
    viewGVpropertieslayout * logs;
    viewELExptPanelHandler * eH;
    exptOutput * selectedLog;
    QVector < double > results;
    int currIndex;

public slots:
    void addExpt();
    void simulationDone();
    void logSelected(int);
    void runAll();
    void importData();
    void exportData();

};

#endif // BATCHEXPERIMENTWINDOW_H
