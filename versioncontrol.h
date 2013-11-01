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

#ifndef VERSIONCONTROL_H
#define VERSIONCONTROL_H

#include <QObject>
#include <QProcess>

enum versionType {
    NONE,
    MERCURIAL,
    SVN,
    CVS
};

class versionControl : public QObject
{
    Q_OBJECT
public:
    explicit versionControl(QObject *parent = 0);

    void detectVCSes();

    bool haveMercurial();
    bool isModelUnderMercurial();
    bool addToMercurial(QString file);
    bool removeFromMercurial(QString file);
    bool commitMercurial(QString message);
    bool updateMercurial();
    bool revertMercurial();
    QString showMercurialLog();
    QString showMercurialStatus();
    bool runMercurial(QString options);

    bool haveVersion();
    bool isModelUnderVersion();
    bool addToVersion(QString file);
    bool removeFromVersion(QString file);
    bool commitVersion();
    bool updateVersion();
    bool revertVersion();
    bool showVersionStatus();
    bool showVersionLog();

    void setupVersion();

private:
    QString stdOutText;
    QString stdErrText;
    bool isMercurialThere;
    versionType version;
    
signals:
    
public slots:
    void finished(int, QProcess::ExitStatus status);
    void standardOutput();
    void standardError();
    
};

#endif // VERIONCONTROL_H
