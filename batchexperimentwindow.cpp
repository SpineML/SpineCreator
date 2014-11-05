#include "batchexperimentwindow.h"
#include "ui_batchexperimentwindow.h"
#include "vectorlistmodel.h"


BatchExperimentWindow::BatchExperimentWindow(experiment * currExpt, viewELExptPanelHandler * eH, viewGVpropertieslayout * logs, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BatchExperimentWindow)
{
    ui->setupUi(this);

    this->selectedLog = NULL;
    this->all_logs = true;

    // assign the experiment and log handler locally
    this->currExpt = currExpt;
    this->logs = logs;
    this->eH = eH;

    // set up the model for the table widget
    this->model = new vectorListModel;

    // make up a list of the current changed pars
    QVector < ParameterData * > dsts;
    for (int i = 0; i < this->currExpt->changes.size(); ++i) {
        // only add FixedValue pars
        if (this->currExpt->changes[i]->par->currType == FixedValue) {
            dsts.push_back(this->currExpt->changes[i]->par);
        }
    }
    this->model->setAllDst(dsts);

    // create a new batch data - we can load in existing ones
    QVector < QVector < double > > datas;
    datas.resize(dsts.size());
    QVector < double > each;
    // resize each vector to one and fill with existing values
    each.resize(1);
    datas.fill(each);
    for (int i = 0; i < this->currExpt->changes.size(); ++i) {
        // only add FixedValue pars
        if (this->currExpt->changes[i]->par->currType == FixedValue) {
            datas[i][0] = this->currExpt->changes[i]->par->value[0];
        }
    }
    this->model->setAllData(datas);

    // set the model to the view
    this->ui->tableView->setModel(this->model);

    // signal / slot
    connect(ui->add_but, SIGNAL(clicked()), this, SLOT(addExpt()));

    // connect up
    connect(ui->par_log, SIGNAL(currentIndexChanged(int)), this, SLOT(logSelected(int)));
    connect(ui->run_but, SIGNAL(clicked()), this, SLOT(runAll()));
    connect(ui->import_but, SIGNAL(clicked()), this, SLOT(importData()));
    connect(ui->export_but, SIGNAL(clicked()), this, SLOT(exportData()));

    // set up the log 'getter'
    this->ui->par_log->addItem("All logs");
    for (int i = 0; i < this->currExpt->outs.size(); ++i) {
        if (this->currExpt->outs[i]->portIsAnalog) {
            this->ui->par_log->addItem(this->currExpt->outs[i]->source->getXMLName() + " " + this->currExpt->outs[i]->portName);
        }
    }


}

BatchExperimentWindow::~BatchExperimentWindow()
{

    // put back the first experiment
    currIndex = 0;
    this->model->moveIndex(currIndex);

    delete model;
    delete ui;
}

void BatchExperimentWindow::importData() {

    // get file to load
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open CSV file for import"), qgetenv("HOME"), tr("CSV files (*.csv *.txt);; All files (*.*)"));

    // open the input csv file for reading
    QFile fileIn(fileName);

    if( !fileIn.open( QIODevice::ReadOnly ) ) {
        QMessageBox msgBox;
        msgBox.setText("Could not open the selected file");
        msgBox.exec();
        return;}

    // use textstream so we can read lines into a QString
    QTextStream stream(&fileIn);

    // structure to hold vals
    QVector < QVector < double > > datas;

    datas.resize(this->model->columnCount());

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
        QStringList fields = line.split(",");

        if (fields.size() != datas.size()) {
            // this must be true for loading, so abort
            QMessageBox msgBox;
            msgBox.setText("CSV file could not be read");
            msgBox.exec();
            return;
        }

        if (fields.size() < 1) {
            QMessageBox msgBox;
            msgBox.setText("CSV file could not be read");
            msgBox.exec();
            return;}

        // for add fields
        for (int i = 0; i < fields.size(); ++i) {
            datas[i].push_back(fields[i].toFloat());
        }
    }

    fileIn.close();

    this->model->setAllData(datas);

    this->model->emitDataChanged();
    this->ui->tableView->setModel(NULL);
    this->ui->tableView->setModel(this->model);
}

void BatchExperimentWindow::exportData() {

    // get file to load
    QString fileName = QFileDialog::getSaveFileName(this, tr("Select CSV file for export"), qgetenv("HOME"), tr("CSV files (*.csv *.txt);; All files (*.*)"));

    // open the input csv file for reading
    QFile fileOut(fileName);

    if( !fileOut.open( QIODevice::WriteOnly ) ) {
        QMessageBox msgBox;
        msgBox.setText("Could not open the selected file");
        msgBox.exec();
        return;}

    // use textstream so we can read lines into a QString
    QTextStream stream(&fileOut);

    for (int row = 0; row < this->model->rowCount(); ++row) {
        for (int col = 0; col < this->model->columnCount(); ++col) {
            stream << this->model->data(row,col);
            if (col != this->model->columnCount() -1) {
                stream << ",";
            }
        }
        stream << endl;
    }

}

void BatchExperimentWindow::addExpt() {

    // get the model max row, then add another
    if (this->model->columnCount() > 0) {
        int maxRow = this->model->rowCount();
        this->model->setData(maxRow, 0, 0);
    }

}

void BatchExperimentWindow::logSelected(int index) {
    // find the log so we can reference it...
    if (index == 0) {
        // this means we have selected the "All logs thing"
        this->all_logs = true;
        return;
    }
    this->all_logs = false;
    int currIndex = 1;
    for (int i = 0; i < this->currExpt->outs.size(); ++i) {
        if (this->currExpt->outs[i]->portIsAnalog) {
            if (currIndex == index) {
                this->selectedLog = this->currExpt->outs[i];
                return;
            }
            ++currIndex;
        }
    }

}

void BatchExperimentWindow::runAll() {

    currIndex = 0;
    this->model->moveIndex(currIndex);
    // clear results
    this->results.clear();

    eH->run();

}

void BatchExperimentWindow::simulationDone() {

    // we have a completion so log the required data and then launch next in the batch!

    int num_logged_vals = 0;

    // find the log:
    if (this->all_logs == false) {
        for (int i = 0; i < logs->logs.size(); ++i) {
            logData * log = logs->logs[i];
            // construct log name! This should be replaced by XML data from the log
            if (selectedLog == NULL) continue;
            QString possibleLogName = selectedLog->source->getXMLName() + "_" + selectedLog->portName + "_log.bin";
            possibleLogName.replace(" ", "_");
            if (log->logName == possibleLogName) {
                // extract the required data and store
                QVector < double > rowData;
                float timeIndex = ui->time_log->value();
                if (timeIndex < 0) {
                    rowData = log->getRow(log->endTime/currExpt->setup.dt);
                    qDebug() << "rowData size = " << rowData.size();
                    if (rowData.size() > ui->index_log->value()) {
                        results.push_back(rowData[ui->index_log->value()]);
                    }
                } else if (timeIndex < log->endTime) {
                    rowData = log->getRow(timeIndex/currExpt->setup.dt);
                    if (rowData.size() > ui->index_log->value()) {
                        results.push_back(rowData[ui->index_log->value()]);
                    }
                } else {
                    results.push_back(-1);
                }
<<<<<<< HEAD
            }
        }
    } else {
        qDebug() << "All logs...";
        for (int i = 0; i < logs->logs.size(); ++i) {
            qDebug() << "Log " << i;
            logData * log = logs->logs[i];
            // extract the required data and store
            for (int j = 0; j < this->currExpt->outs.size(); ++j) {
                // for each logged value in the experiment
                if (this->currExpt->outs[j]->portIsAnalog) {
                    QString possibleLogName = this->currExpt->outs[j]->source->getXMLName() + "_" + this->currExpt->outs[j]->portName + "_log.bin";
                    possibleLogName.replace(" ", "_");
                    if (log->logName == possibleLogName) {
                        QVector < double > rowData;
                        float timeIndex = ui->time_log->value();
                        if (timeIndex < 0) {
                            rowData = log->getRow(log->endTime/currExpt->setup.dt);
                            qDebug() << "rowData size = " << rowData.size();
                            if (rowData.size() > ui->index_log->value()) {
                                results.push_back(rowData[ui->index_log->value()]);
                                num_logged_vals++;
                            }
                        } else if (timeIndex < log->endTime) {
                            rowData = log->getRow(timeIndex/currExpt->setup.dt);
                            if (rowData.size() > ui->index_log->value()) {
                                results.push_back(rowData[ui->index_log->value()]);
                                num_logged_vals++;
                            }
                        } else {
                            results.push_back(-1);
                            num_logged_vals++;
                        }
                    }
                }
=======
>>>>>>> Stuff for interfacing with BRAHMS through files
            }
        }
    } else {
           qDebug() << "All logs...";
           for (int i = 0; i < logs->logs.size(); ++i) {
               qDebug() << "Log " << i;
               logData * log = logs->logs[i];
               // extract the required data and store
               for (int j = 0; j < this->currExpt->outs.size(); ++j) {
                   // for each logged value in the experiment
                   if (this->currExpt->outs[j]->portIsAnalog) {
                       QString possibleLogName = this->currExpt->outs[j]->source->getXMLName() + "_" + this->currExpt->outs[j]->portName + "_log.bin";
                       possibleLogName.replace(" ", "_");
                       if (log->logName == possibleLogName) {
                           QVector < double > rowData;
                           float timeIndex = ui->time_log->value();
                           if (timeIndex < 0) {
                               rowData = log->getRow(log->endTime/currExpt->setup.dt);
                               qDebug() << "rowData size = " << rowData.size();
                               if (rowData.size() > ui->index_log->value()) {
                                   results.push_back(rowData[ui->index_log->value()]);
                                   num_logged_vals++;
                               }
                           } else if (timeIndex < log->endTime) {
                               rowData = log->getRow(timeIndex/currExpt->setup.dt);
                               if (rowData.size() > ui->index_log->value()) {
                                   results.push_back(rowData[ui->index_log->value()]);
                                   num_logged_vals++;
                               }
                           } else {
                               results.push_back(-1);
                               num_logged_vals++;
                           }
                       }
                   }
               }
           }
    }

    // unhighlight experiment
    this->model->selectedData = -1;

    // move the index on (if the next index is still in range) and launch a new experiment
    if (currIndex + 1 >= this->model->rowCount()) {
        // show results
        QString finalResults;
        if (this->all_logs == false) {
            for (int i = 0; i < this->results.size(); ++i) {
                finalResults.append(QString::number(this->results[i]) + "\n");
            }
        } else {
            int count = 1;
            qDebug() << num_logged_vals;
            for (int i = 0; i < this->results.size(); ++i) {
                qDebug() << "Count = " << count;
                if (count > num_logged_vals) {
                    // new line
                    count = 1;
                    finalResults.append("\n");
                }
                finalResults.append(QString::number(this->results[i]) + "\t");
                ++count;
            }
        }
        QMessageBox msgBox;
        msgBox.setWindowTitle("Batch Complete");
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setText("Batch has finished. See below for results.");
        msgBox.setDetailedText(finalResults);
        msgBox.addButton(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.exec();
        // give up
        return;
    }

    ++currIndex;
    this->model->moveIndex(currIndex);

    // highlight current experiment
    this->model->selectedData = currIndex;
    this->ui->tableView->selectRow(currIndex);

    // run next model:
    eH->run();

}


