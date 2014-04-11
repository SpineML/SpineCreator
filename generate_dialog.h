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

#ifndef GENERATE_DIALOG_H
#define GENERATE_DIALOG_H

#include <QDialog>
#include "globalHeader.h"

namespace Ui {
class generate_dialog;
}

class generate_dialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit generate_dialog(kernel_connection * currConn, population *src, population *dst, vector<conn> &conns, QMutex * mutex, QWidget *parent = 0);
    explicit generate_dialog(pythonscript_connection * currConn, population *src, population *dst, vector<conn> &conns, QMutex * mutex, QWidget *parent = 0);
    ~generate_dialog();
    
private:
    Ui::generate_dialog *ui;
    connection * currConn;
    QThread *workerThread;

public slots:
    void moveFromThread();
    void doPython();
};

#endif // GENERATE_DIALOG_H
