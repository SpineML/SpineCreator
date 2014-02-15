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

#include "connectionlistdialog.h"
#include "ui_connectionlistdialog.h"
#include "connectionmodel.h"
#include "connection.h"
//#include "stringify.h"

connectionListDialog::connectionListDialog(csv_connection * conn, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::connectionListDialog)
{
    ui->setupUi(this);

    this->conn = conn;

    ui->spinBox->setRange(0, 10000000); // set max from the component
    ui->spinBox->setValue(conn->getNumRows());

    connect(ui->spinBox,SIGNAL(valueChanged(int)), this, SLOT(updateValSize(int)));

    this->vModel = new csv_connectionModel();
    this->vModel->setConnection(conn);
    this->ui->tableView->setModel(this->vModel);
    QHeaderView * header = this->ui->tableView->horizontalHeader();
    header->setStretchLastSection(true);
    //header->setResizeMode(0, QHeaderView::Fixed);
    header->resizeSection(0, 80);
    header->resizeSection(1, 80);

    connect(this->ui->import_csv,SIGNAL(clicked()), this, SLOT(importCSV()));
    connect(this->vModel,SIGNAL(setSpinBoxVal(int)), ui->spinBox, SLOT(setValue(int)));
}

connectionListDialog::~connectionListDialog()
{
    delete vModel;
    delete ui;
}



void connectionListDialog::accept() {

    //this->conn->flushChangesToDisk();
    // nothing to do
    delete this;

}

void connectionListDialog::reject() {

    this->conn->abortChanges();
    delete this;

}

void connectionListDialog::importCSV() {

    QString fileName = QFileDialog::getOpenFileName(this, tr("Open CSV file for import"), qgetenv("HOME"), tr("CSV files (*.csv *.txt);; All files (*.*)"));
    conn->import_csv(fileName);
    this->vModel->deleteLater();
    this->vModel = new csv_connectionModel();
    this->vModel->setConnection(conn);
    ui->tableView->setModel(this->vModel);
    connect(this->vModel,SIGNAL(setSpinBoxVal(int)), ui->spinBox, SLOT(setValue(int)));
    ui->spinBox->setValue(conn->getNumRows());
    this->vModel->emitDataChanged();

}

void connectionListDialog::updateValSize(int val) {

    /*bool returnVal;
    returnVal = */this->vModel->insertRows(val);

}
