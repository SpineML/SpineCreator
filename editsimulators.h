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

#ifndef EDITSIMULATORS_H
#define EDITSIMULATORS_H

#include <QDialog>
#include "globalHeader.h"

namespace Ui {
class editSimulators;
}

class editSimulators : public QDialog
{
    Q_OBJECT
    
public:
    explicit editSimulators(QWidget *parent = 0);
    ~editSimulators();
    
private:
    Ui::editSimulators *ui;

    /*!
     * \brief path The absolute path to the current convert script.
     */
    QString path;

    /*!
     * \brief working_dir The working directory for the current convert script.
     */
    QString working_dir;

    QStringList keys;
    QStringList values;
    bool edited;
    void redrawEnvVars();
    void recursiveDeleteLater(QLayout *);
    void recursiveDeleteLaterloop(QLayout *);

public slots:

    void addEnvVar();
    void delEnvVar();
    void changeEnvVar();
    void selectSimulator(QString);
    void getScript();
    void getWorkingDir();
    void getNewSimName();
    void applyChanges();
    void cancelChanges();
    void changeScript();
    void changedEnvVar(QString);
    void saveAsBinaryToggled(bool);
    void setGLDetailLevel(int);
    void setDevMode(bool);
    void close();
    //void testFunc();
};

#endif // EDITSIMULATORS_H
