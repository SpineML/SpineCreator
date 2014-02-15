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

#include "generate_dialog.h"
#include "ui_generate_dialog.h"
#include "population.h"
#include "connection.h"
#include "glconnectionwidget.h"

generate_dialog::generate_dialog(distanceBased_connection * currConn, population * src, population * dst, vector < conn > &conns, QMutex * mutex, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::generate_dialog)
{
    ui->setupUi(this);

    // generating connectivity

    this->currConn = currConn;
    currConn->src = src;
    currConn->dst = dst;
    currConn->conns = &conns;
    currConn->mutex = mutex;

    workerThread = new QThread(this);

    connect(workerThread, SIGNAL(started()), currConn, SLOT(generate_connections()));
    //connect(workerThread, SIGNAL(finished()), this, SLOT(moveFromThread()));
    connect(currConn, SIGNAL(connectionsDone()), this, SLOT(moveFromThread()));

    connect(currConn, SIGNAL(progress(int)), ui->progressBar, SLOT(setValue(int)));
    if (parent != NULL)
        connect(currConn, SIGNAL(progress(int)), ((glConnectionWidget *)parent), SLOT(redraw()));

    currConn->moveToThread(workerThread);

    workerThread->start();

}

generate_dialog::generate_dialog(kernel_connection * currConn, population * src, population * dst, vector < conn > &conns, QMutex * mutex, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::generate_dialog)
{
    ui->setupUi(this);

    // generating connectivity

    this->currConn = currConn;
    currConn->src = src;
    currConn->dst = dst;
    currConn->conns = &conns;
    currConn->mutex = mutex;

    workerThread = new QThread(this);

    connect(workerThread, SIGNAL(started()), currConn, SLOT(generate_connections()));
    //connect(workerThread, SIGNAL(finished()), this, SLOT(moveFromThread()));
    connect(currConn, SIGNAL(connectionsDone()), this, SLOT(moveFromThread()));

    connect(currConn, SIGNAL(progress(int)), ui->progressBar, SLOT(setValue(int)));
    if (parent != NULL)
        connect(currConn, SIGNAL(progress(int)), ((glConnectionWidget *)parent), SLOT(redraw()));

    currConn->moveToThread(workerThread);

    workerThread->start();

}

void generate_dialog::moveFromThread() {
    //currConn->moveToThread(QApplication::instance()->thread());
    workerThread->exit();
    // see if errors:
    if (currConn->type == Kernel) {
        if (!((kernel_connection *) currConn)->errorLog.isEmpty())
            ui->errors->setText(((kernel_connection *) currConn)->errorLog);
        else
            this->accept();
    }
    else if (currConn->type == DistanceBased) {
        if (!((distanceBased_connection *) currConn)->errorLog.isEmpty())
            ui->errors->setText(((distanceBased_connection *) currConn)->errorLog);
        else
            this->accept();
    }

}

generate_dialog::~generate_dialog()
{
    delete ui;
}
