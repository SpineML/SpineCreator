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

#ifndef BRAHMS_DIALOG_H
#define BRAHMS_DIALOG_H

#include <QDialog>
#include <QFileDialog>
#include "rootdata.h"
#include <qthread.h>

class I : public QThread
{
public:
    static void sleep(unsigned long secs) {
        QThread::sleep(secs);
    }
    static void msleep(unsigned long msecs) {
        QThread::msleep(msecs);
    }
    static void usleep(unsigned long usecs) {
        QThread::usleep(usecs);
    }
};

namespace Ui {
class BRAHMS_dialog;
}

class BRAHMS_dialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit BRAHMS_dialog(rootData * data, QWidget *parent = 0);
    ~BRAHMS_dialog();
    
private:
    Ui::BRAHMS_dialog *ui;
    rootData * data;

signals:
    void  write_out_model(QString);

public slots:
    void getNamespace();
    void getPath();
    void runIt();

};

#endif // BRAHMS_DIALOG_H
