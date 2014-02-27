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

    this->setWindowTitle("Generate connections using Distance");

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

    this->setWindowTitle("Generate connections using a Kernel");

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

generate_dialog::generate_dialog(pythonscript_connection * currConn, population * src, population * dst, vector < conn > &conns, QMutex * mutex, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::generate_dialog)
{
    ui->setupUi(this);

    this->setWindowTitle("Generate connections using Python");

    // generating connectivity

    this->currConn = currConn;
    currConn->src = src;
    currConn->dst = dst;
    currConn->conns = &conns;
    currConn->mutex = mutex;



    // get rid of the dialog if we are successful
    QTimer * timer = new QTimer;
    timer->setSingleShot(true);
    connect(timer, SIGNAL(timeout()), this, SLOT(doPython()));
    timer->start(100);


    //workerThread = new QThread(this);

    //connect(workerThread, SIGNAL(started()), currConn, SLOT(generate_connections()));
    //connect(workerThread, SIGNAL(finished()), this, SLOT(moveFromThread()));
    //connect(currConn, SIGNAL(connectionsDone()), this, SLOT(moveFromThread()));

    //connect(currConn, SIGNAL(progress(int)), ui->progressBar, SLOT(setValue(int)));
    //if (parent != NULL)
    //    connect(currConn, SIGNAL(progress(int)), ((glConnectionWidget *)parent), SLOT(redraw()));

    //currConn->moveToThread(workerThread);

    //workerThread->start();

}

void generate_dialog::doPython() {

    ((pythonscript_connection *) currConn)->generate_connections();

    if (!((pythonscript_connection *) currConn)->errorLog.isEmpty()) {
        ui->errors->setText(((pythonscript_connection *) currConn)->errorLog);
    } else if (!((pythonscript_connection *) currConn)->pythonErrors.isEmpty()) {
        ui->errors->setText(((pythonscript_connection *) currConn)->pythonErrors);
    } else {
        // move the weights across
        pythonscript_connection * currPyConn = (pythonscript_connection *) currConn;
        ParameterData * par = currPyConn->getPropPointer();
        if (par) {
            par->currType = ExplicitList;
            par->value = currPyConn->weights;
            for (uint i = 0; i < currPyConn->weights.size(); ++i) {
                par->indices.push_back(i);
            }
        }
        this->accept();
    }

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
    else if (currConn->type == Python) {
        if (!((pythonscript_connection *) currConn)->errorLog.isEmpty()) {
            ui->errors->setText(((pythonscript_connection *) currConn)->errorLog);
        } else if (!((pythonscript_connection *) currConn)->pythonErrors.isEmpty()) {
            ui->errors->setText(((pythonscript_connection *) currConn)->pythonErrors);
        } else {
            // move the weights across
            pythonscript_connection * currPyConn = (pythonscript_connection *) currConn;
            for (uint i = 0; i < currPyConn->src->projections.size(); ++i) {
                projection * proj = currPyConn->src->projections[i];
                for (uint j = 0; j < proj->synapses.size(); ++j) {
                    synapse * syn = proj->synapses[j];
                    // if we have found the connection
                    if (syn->connectionType == currPyConn) {
                        // now we know which weight update we have to look at
                        for (uint k = 0; k < syn->weightUpdateType->ParameterList.size(); ++k) {
                            if (syn->weightUpdateType->ParameterList[k]->name == currPyConn->weightProp) {
                                // found the weight - now to alter it
                                ParameterData * par = syn->weightUpdateType->ParameterList[k];
                                par->currType = ExplicitList;
                                par->value = currPyConn->weights;
                            }
                        }
                    }
                }
            }
            this->accept();
        }
    }

}

generate_dialog::~generate_dialog()
{
    delete ui;
}
