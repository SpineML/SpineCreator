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

#include "versioncontrol.h"
#include "QProcess"
#include "QSettings"
#include "globalHeader.h"
#include "commitdialog.h"

versionControl::versionControl(QObject *parent) :
    QObject(parent)
{

    // detect version control systems
    this->detectVCSes();

    this->version = NONE;

}

void versionControl::detectVCSes() {

    // check for mercurial

    // set up the environment for the spawned processes
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("PATH", qgetenv("PATH"));

    // make the QProcess
    QProcess * mercurial = new QProcess;
    mercurial->setWorkingDirectory(qgetenv("HOME"));
    mercurial->setProcessEnvironment(env);

#ifdef NEED_VERSIONCONTROL_FINISHED
    connect(mercurial, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(finished(int, QProcess::ExitStatus)));
#endif
    connect(mercurial, SIGNAL(readyReadStandardOutput()), this, SLOT(standardOutput()));
    connect(mercurial, SIGNAL(readyReadStandardError()), this, SLOT(standardError()));

    mercurial->start("hg");

    isMercurialThere = mercurial->waitForFinished(1000);

    delete mercurial;

}

bool versionControl::haveMercurial() {

    return this->isMercurialThere;
}

bool versionControl::isModelUnderMercurial() {

    if (!this->isMercurialThere)
        return false;

    QSettings settings;

    // get current model path from QSettings

    QString path = settings.value("files/currentFileName", "No model").toString();

    if (path == "No model")
        return false;

    // set up the environment for the spawned processes
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("PATH", qgetenv("PATH"));

    // make the QProcess
    QProcess * mercurial = new QProcess;
    mercurial->setWorkingDirectory(QDir::toNativeSeparators(path));
    mercurial->setProcessEnvironment(env);

    connect(mercurial, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(finished(int, QProcess::ExitStatus)));
    connect(mercurial, SIGNAL(readyReadStandardOutput()), this, SLOT(standardOutput()));
    connect(mercurial, SIGNAL(readyReadStandardError()), this, SLOT(standardError()));

    mercurial->start("hg summary");

    bool noerror = mercurial->waitForFinished(1000);

    if (noerror) {
        // check output
        if (mercurial->exitCode() == 0)
            return true;
        else
            return false;

    }
    return false;
}

// add a file to the repository
bool versionControl::addToMercurial(QString file) {

    return runMercurial("add " + file);
}

// remove a file from the repository
bool versionControl::removeFromMercurial(QString file) {

    return runMercurial("remove " + file);

}

// commit a new version
bool versionControl::commitMercurial(QString message) {

    // different OS require different tricks for multi-line messages
#ifdef Q_OS_LINUX
    QString moo = "commit -m \"$(echo -e '" + message + "')\"";
    qDebug() << moo;
    return runMercurial("commit -m \"" + message + "\"");
#endif
#ifdef Q_OS_MAC
    return runMercurial("commit -m \"$(echo -e '" + message + "')\"");
#endif
#ifdef Q_OS_WIN
    return runMercurial("commit -m \"" + message + "\"");
#endif

}

// remove a file from the repository
bool versionControl::updateMercurial() {

    return runMercurial("update");

}

// remove a file from the repository
bool versionControl::revertMercurial() {

    return runMercurial("revert");

}

// read the log
QString versionControl::showMercurialLog() {

    runMercurial("log -v");
    return stdOutText;

}

// fetch the status
QString versionControl::showMercurialStatus() {

    runMercurial("status");
    return stdOutText;

}

bool versionControl::runMercurial(QString options) {

    if (!this->isMercurialThere)
        return false;

    QSettings settings;

    // get current model path from QSettings

    QString path = settings.value("files/currentFileName", "No model").toString();

    if (path == "No model")
        return false;

    // set up the environment for the spawned processes
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("PATH", qgetenv("PATH"));
    // to be used as a username if one is not set
    env.insert("EMAIL", QHostInfo::localHostName());

    // make the QProcess
    QProcess * mercurial = new QProcess;
    mercurial->setWorkingDirectory(QDir::toNativeSeparators(path));
    mercurial->setProcessEnvironment(env);

    connect(mercurial, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(finished(int, QProcess::ExitStatus)));
    connect(mercurial, SIGNAL(readyReadStandardOutput()), this, SLOT(standardOutput()));
    connect(mercurial, SIGNAL(readyReadStandardError()), this, SLOT(standardError()));

    // clear stdOut & stdErr texts
    stdOutText.clear();
    stdErrText.clear();

    // launch
    mercurial->start("hg " + options);

    // wait until complete, or 3 s
    bool noerror = mercurial->waitForFinished(3000);

    if (noerror) {
        // check output
        if (mercurial->exitCode() == 0) {
            delete mercurial;
            return true;
        } else {
            delete mercurial;
            return false;
        }

    }
    delete mercurial;
    return false;

}

bool versionControl::haveVersion() {

    if (haveMercurial())
        return true;

    return false;

}

bool versionControl::isModelUnderVersion() {

    if (this->version == NONE)
        return false;

    return true;

}

bool versionControl::addToVersion(QString file) {

    switch (this->version) {
    case NONE:
        return false;
    case MERCURIAL:
        return addToMercurial(file);
    case SVN:
    case CVS:
        return false;
    }
    return false;
}

bool versionControl::removeFromVersion(QString file) {

    switch (this->version) {
    case NONE:
        return false;
    case MERCURIAL:
        return removeFromMercurial(file);
    case SVN:
    case CVS:
        return false;
    }
    return false;
}

bool versionControl::commitVersion() {

    QString message;

    // obtain message
    commitDialog * dialog = new commitDialog;
    switch(dialog->exec()) {
    case QDialog::Accepted:
        message = dialog->getString();
        break;
    case QDialog::Rejected:
        return false;

    }


    switch (this->version) {
    case NONE:
        return false;
    case MERCURIAL:
        return commitMercurial(message);
    case SVN:
    case CVS:
        return false;
    }
    return false;
}

bool versionControl::updateVersion() {

    switch (this->version) {
    case NONE:
        return false;
    case MERCURIAL:
        return updateMercurial();
    case SVN:
    case CVS:
        return false;
    }
    return false;
}

bool versionControl::revertVersion() {

    switch (this->version) {
    case NONE:
        return false;
    case MERCURIAL:
        return revertMercurial();
    case SVN:
    case CVS:
        return false;
    }
    return false;
}

bool versionControl::showVersionStatus() {

    QString status;

    switch (this->version) {
    case NONE:
        return false;
    case MERCURIAL:
        status = showMercurialStatus();
        break;
    case SVN:
    case CVS:
        return false;
    }

    // show log
    commitDialog * dialog = new commitDialog;
    dialog->showStatus(status);
    switch(dialog->exec()) {
    case QDialog::Accepted:
        return true;
    case QDialog::Rejected:
        return true;
    }

    return true;

}

bool versionControl::showVersionLog() {

    QString log;

    switch (this->version) {
    case NONE:
        return false;
    case MERCURIAL:
        log = showMercurialLog();
        break;
    case SVN:
    case CVS:
        return false;
    }

    // show log
    commitDialog * dialog = new commitDialog;
    dialog->showLog(log);
    switch(dialog->exec()) {
    case QDialog::Accepted:
        return true;
    case QDialog::Rejected:
        return true;
    }

    return true;
}

void versionControl::setupVersion() {

    if (isModelUnderMercurial()) {
        this->version = MERCURIAL;
        return;
    }

    this->version = NONE;

}


#ifdef NEED_VERSIONCONTROL_FINISHED
void versionControl::finished(int, QProcess::ExitStatus status)
{
    // get status
    /*if (status == QProcess::CrashExit) {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Simulator Error");
        msgBox.setText(stdErrText);
        msgBox.exec();
        return;
    }
    if (status == QProcess::NormalExit) {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Simulator Complete");
        msgBox.setText(stdOutText);
        msgBox.exec();
        return;
    }*/

    // collect logs
}
#endif

void versionControl::standardOutput() {

    QByteArray data = ((QProcess *) sender())->readAllStandardOutput();
    stdOutText = stdOutText + QString().fromUtf8(data);

}

void versionControl::standardError() {

    QByteArray data = ((QProcess *) sender())->readAllStandardError();
    stdErrText = stdErrText + QString().fromUtf8(data);

}
