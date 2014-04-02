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

#include "editsimulators.h"
#include "ui_editsimulators.h"
#include "QSettings"

editSimulators::editSimulators(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::editSimulators)
{

    edited = false;

    ui->setupUi(this);

    ui->buttonBox->setFocusPolicy(Qt::NoFocus);

    connect(ui->addEnv, SIGNAL(clicked()), this, SLOT(addEnvVar()));

    // load existing simulators:
    QSettings settings;

    settings.beginGroup("simulators");
    QStringList sims = settings.childGroups();
    for (int i = 0; i < sims.size(); ++i) {
        this->ui->comboBox->addItem(sims[i]);
        if (i == 0) selectSimulator(sims[0]);
    }
    settings.endGroup();

    ((QHBoxLayout *) ui->scrollAreaWidgetContents->layout())->removeWidget(ui->addEnv);
    ((QHBoxLayout *) ui->scrollAreaWidgetContents->layout())->addWidget(ui->addEnv);

    // connect up comboBox
    connect(ui->comboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(selectSimulator(QString)));
    // connect up buttons
    connect(this->ui->findFile, SIGNAL(clicked()), this, SLOT(getScript()));
    connect(this->ui->findWorkingDir, SIGNAL(clicked()), this, SLOT(getWorkingDir()));
    connect(this->ui->addSim, SIGNAL(clicked()), this, SLOT(getNewSimName()));

    // accept
    connect(ui->buttonBox->button(QDialogButtonBox::Apply), SIGNAL(clicked()), this, SLOT(applyChanges()));

    // cancel
    connect(ui->buttonBox->button(QDialogButtonBox::Reset), SIGNAL(clicked()), this, SLOT(cancelChanges()));

    // close
    connect(ui->buttonBox_2->button(QDialogButtonBox::Close), SIGNAL(clicked()), this, SLOT(close()));

    // change path
    connect(ui->scriptLineEdit, SIGNAL(editingFinished()), this, SLOT(changeScript()));

    // change if we save large connections as binary data
    bool writeBinary = settings.value("fileOptions/saveBinaryConnections", "error").toBool();
    ui->save_as_binary->setChecked(writeBinary);
    connect(ui->save_as_binary, SIGNAL(toggled(bool)), this, SLOT(saveAsBinaryToggled(bool)));

    // change level of detail box
    int lod = settings.value("glOptions/detail", 5).toInt();
    ui->openGLDetailSpinBox->setValue(lod);
    connect(ui->openGLDetailSpinBox, SIGNAL(valueChanged(int)), this, SLOT(setGLDetailLevel(int)));

    // change dev stuff box
    bool devMode = settings.value("dev_mode_on", "false").toBool();
    ui->dev_mode_check->setChecked(devMode);
    connect(ui->dev_mode_check, SIGNAL(toggled(bool)), this, SLOT(setDevMode(bool)));

    // TESTING:
    //connect(ui->test, SIGNAL(clicked()), this, SLOT(testFunc()));

    redrawEnvVars();
}

editSimulators::~editSimulators()
{
    delete ui;
}

void editSimulators::saveAsBinaryToggled(bool toggle)
{
    QSettings settings;
    settings.setValue("fileOptions/saveBinaryConnections", QString::number((float) toggle));
}

void editSimulators::setGLDetailLevel(int value)
{
    QSettings settings;
    settings.setValue("glOptions/detail", value);
}

void editSimulators::setDevMode(bool toggle)
{
    QSettings settings;
    settings.setValue("dev_mode_on", toggle);
}

/*void editSimulators::testFunc()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Library is where?"), qgetenv("HOME"), tr("All files (*.*)"));
    QString log = "nrn";
    QString port = "v";
    int index = 0;


    QLibrary library(fileName);
    if (!library.load()) {
        qDebug() << "No dice on loading";
        return;
    }
    typedef int(*testFunc)(int, int);

    testFunc testf = (testFunc)library.resolve("test");

    if (!testf) {
        qDebug() << "no dice on resolving";
        return;
    }

    int out;

    out = testf(3,4);

    qDebug() << out;

    typedef double*(*getLogFunc)(const char*, const char*, const char*, const char*, int, const char*);

    getLogFunc getL = (getLogFunc)library.resolve("getLog");

    if (!getL) {
        qDebug() << "no dice on resolving";
        return;
    }

    QString dirName = QFileDialog::getExistingDirectory(this, "Choose rep dir", qgetenv("HOME"));

    QString error;

    double * data = getL(dirName.toStdString().c_str(), log.toStdString().c_str(), port.toStdString().c_str(), "analog", index, error.toStdString().c_str());


    if (data == NULL) {
        qDebug() << "no dice with return";
        return;
    }

    qDebug() << data[0];

    ui->plainTextEdit->appendPlainText("Logs");
    ui->plainTextEdit->appendPlainText(error);

    QString newLine;
    if (data[0] > 0) {
        for (uint i = 0; i < (int) data[0]; ++i) {
            newLine = newLine.setNum(data[i]);
            ui->plainTextEdit->appendPlainText(newLine);
        }
    }

    free((void *) data);

    library.unload();

}*/

void editSimulators::addEnvVar()
{
    keys.push_back("newVariable");
    values.push_back("newValue");

    this->edited = true;
    redrawEnvVars();
}

void editSimulators::delEnvVar()
{
    QLineEdit * envNameLineEdit = (QLineEdit *) sender()->property("ptr").value<void *>();

    // remove var
    for (int i = 0; i < keys.size(); ++i) {
        if (keys[i] == envNameLineEdit->text()) {
            keys.erase(keys.begin()+i);
            values.erase(values.begin()+i);
            break;
        }
    }

    this->edited = true;
    redrawEnvVars();
}

void editSimulators::changeEnvVar()
{
    edited = true;

    if (sender()->property("type").toString() == "key") {
        keys[sender()->property("index").toInt()] = ((QLineEdit *) sender())->text();
    } else if (sender()->property("type").toString() == "value")  {
        values[sender()->property("index").toInt()] = ((QLineEdit *) sender())->text();
    }
}

void editSimulators::changedEnvVar(QString)
{
    edited = true;
    ui->buttonBox_2->button(QDialogButtonBox::Close)->setEnabled(false);
    ui->comboBox->setEnabled(false);
    ui->addSim->setEnabled(false);
}

void editSimulators::recursiveDeleteLaterloop(QLayout * parentLayout)
{
    QLayoutItem * item;
    while ((item = parentLayout->takeAt(0))) {
        if (item->widget() != ui->addEnv) {
            item->widget()->deleteLater();
        }
        if (item->layout()) {
            recursiveDeleteLaterloop(item->layout());
        }
        delete item;
    }
    parentLayout->deleteLater();
}

void editSimulators::recursiveDeleteLater(QLayout * parentLayout)
{
    QLayoutItem * item;
    while ((item = parentLayout->takeAt(0))) {
        if (item->widget() && item->widget() != ui->addEnv) {
            item->widget()->deleteLater();
        }
        if (item->layout()) {
            recursiveDeleteLaterloop(item->layout());
        }
        delete item;
    }
}

void editSimulators::redrawEnvVars()
{
    recursiveDeleteLater(((QHBoxLayout *) ui->scrollAreaWidgetContents->layout()));

    ((QHBoxLayout *) ui->scrollAreaWidgetContents->layout())->addWidget(ui->addEnv);

    for (int i = 0; i < keys.size(); ++i) {

        QHBoxLayout * newEnv = new QHBoxLayout;

        QLineEdit * envNameLineEdit = new QLineEdit;
        envNameLineEdit->setMaximumWidth(220);
        envNameLineEdit->setProperty("type", "key");
        envNameLineEdit->setProperty("index", i);
        envNameLineEdit->setText(keys[i]);
        QLineEdit * envLineEdit = new QLineEdit;
        envLineEdit->setMaximumWidth(320);
        envLineEdit->setProperty("type", "value");
        envLineEdit->setText(values[i]);
        envLineEdit->setProperty("index", i);
        QPushButton * envDel = new QPushButton;
        envDel->setFlat(true);
        envDel->setFocusPolicy(Qt::NoFocus);
        envDel->setIcon(QIcon(":/icons/toolbar/delShad.png"));
        envDel->setProperty("ptr", qVariantFromValue((void *) envNameLineEdit));
        connect(envDel, SIGNAL(clicked()), this, SLOT(delEnvVar()));
        connect(envNameLineEdit, SIGNAL(textChanged(QString)), this, SLOT(changeEnvVar()));
        connect(envLineEdit, SIGNAL(textChanged(QString)), this, SLOT(changeEnvVar()));
        connect(envNameLineEdit, SIGNAL(textEdited(QString)), this, SLOT(changedEnvVar(QString)));
        connect(envLineEdit, SIGNAL(textEdited(QString)), this, SLOT(changedEnvVar(QString)));

        newEnv->addWidget(new QLabel("Name"));
        newEnv->addWidget(envNameLineEdit);
        newEnv->addWidget(new QLabel("Value"));
        newEnv->addWidget(envLineEdit);
        newEnv->addWidget(envDel);

        ((QHBoxLayout *) ui->scrollAreaWidgetContents->layout())->insertLayout(((QHBoxLayout *) ui->scrollAreaWidgetContents->layout())->count()-1,newEnv);
    }

    ((QHBoxLayout *) ui->scrollAreaWidgetContents->layout())->addStretch();

    if (edited) {
        // disable this stuff
        ui->buttonBox_2->button(QDialogButtonBox::Close)->setEnabled(false);
        ui->comboBox->setEnabled(false);
        ui->addSim->setEnabled(false);
    } else {
        // enable this stuff
        ui->buttonBox_2->button(QDialogButtonBox::Close)->setEnabled(true);
        ui->comboBox->setEnabled(true);
        ui->addSim->setEnabled(true);
    }
}

void editSimulators::getScript()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Choose the simulator script"), qgetenv("HOME"), tr("All files (*)"));

    if (!fileName.isEmpty()) {
        this->ui->scriptLineEdit->setText(fileName);
        this->edited = true;
    }

    redrawEnvVars();
}

void editSimulators::getWorkingDir()
{
    QString wk_dir = QFileDialog::getExistingDirectory(this,
                                                        tr("Choose the simulator's working directory"),
                                                        qgetenv("HOME"));

    if (!wk_dir.isEmpty()) {
        this->ui->scriptWDLineEdit->setText(wk_dir);
        this->edited = true;
    }

    redrawEnvVars();
}

void editSimulators::getNewSimName()
{
    bool ok;
    QString simName = QInputDialog::getText(this,"Enter new simulator name", tr("Simulator name:"), QLineEdit::Normal, "", &ok);
    if (ok && !simName.isEmpty()) {
        this->ui->comboBox->addItem(simName);
        this->ui->comboBox->setCurrentIndex(this->ui->comboBox->count()-1);
        this->ui->useBinary->setChecked(false);
        edited = true;
    }

    redrawEnvVars();
}

void editSimulators::selectSimulator(QString simName)
{
    // clear old sim
    this->keys.clear();
    this->values.clear();

    QSettings settings;

    // load path
    settings.beginGroup("simulators/" + simName);
    this->path = settings.value("path").toString();
    this->working_dir = settings.value("working_dir").toString();
    ui->useBinary->setChecked(settings.value("binary").toBool());
    settings.endGroup();

    settings.beginGroup("simulators/" + simName + "/envVar");

    // load values
    QStringList keysTemp = settings.childKeys();

    for (int i = 0; i < keysTemp.size(); ++i) {
        keys.push_back(keysTemp[i]);
        values.push_back(settings.value(keys[i]).toString());
    }

    settings.endGroup();

    ui->scriptLineEdit->setText(this->path);
    ui->scriptWDLineEdit->setText(this->working_dir);

    redrawEnvVars();
}

void editSimulators::applyChanges()
{
    QSettings settings;

    // add a group for this simulator
    settings.beginGroup("simulators/" + ui->comboBox->currentText());

    // Check newSim is an executable script. If not, issue warning.
    QFile scriptfile (ui->scriptLineEdit->text());
    if (!scriptfile.exists()) {
        // warning - file doesn't exist.
        QMessageBox::warning(this,
                             QString("Script not found"),
                             QString("The convert script wasn't found; simulations will not be able to execute."));
    }

    settings.setValue("path", ui->scriptLineEdit->text());
    settings.setValue("working_dir", ui->scriptWDLineEdit->text());
    settings.setValue("binary", ui->useBinary->isChecked());
    settings.endGroup();

    settings.beginGroup("simulators/" + ui->comboBox->currentText() + "/envVar");

    // remove existing envVars
    QStringList currKeys = settings.childKeys();
    for (int i = 0; i < currKeys.size(); ++i) {
        settings.remove(currKeys[i]);
    }
    for (int i = 0; i < this->keys.size(); ++i) {
        settings.setValue(keys[i], values[i]);
    }
    settings.endGroup();

    edited = false;

    ui->buttonBox_2->button(QDialogButtonBox::Close)->setEnabled(true);
    ui->comboBox->setEnabled(true);
    ui->addSim->setEnabled(true);
}

void editSimulators::cancelChanges()
{
    selectSimulator(ui->comboBox->currentText());
    edited = false;
    redrawEnvVars();
}

void editSimulators::changeScript()
{
    path = ((QLineEdit *) sender())->text();
    edited = true;
    redrawEnvVars();
}

void editSimulators::close()
{
    this->applyChanges();
    delete this;
}
