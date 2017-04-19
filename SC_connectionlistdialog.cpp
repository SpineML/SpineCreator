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

#include "SC_connectionlistdialog.h"
#include "ui_connectionlistdialog.h"
#include "SC_connectionmodel.h"
#include "NL_connection.h"

connectionListDialog::connectionListDialog (csv_connection* c, QWidget* parent) :
    QDialog(parent),
    ui(new Ui::connectionListDialog)
{
    this->ui->setupUi (this);

    this->conn = c;
    this->newConn = (csv_connection*)this->conn->newFromExisting();
    // Copy data values in so that they appear in the dialog:
    this->newConn->copyDataValues (this->conn);

    ui->spinBox->setRange (0, INT_MAX); // set max from the component
    ui->spinBox->setValue (this->conn->getNumRows());

    connect (ui->spinBox, SIGNAL(valueChanged(int)), this, SLOT(updateValSize(int)));

    this->vModel = new csv_connectionModel();
    this->vModel->setConnection (this->newConn);
    this->ui->tableView->setModel (this->vModel);
    QHeaderView* header = this->ui->tableView->horizontalHeader();
    header->setStretchLastSection (true);
    header->resizeSection (0, 80);
    header->resizeSection (1, 80);

    // hide import button if is a pythonscript
    if (this->conn->generator != NULL) {
        this->ui->import_csv->setVisible (false);
        this->ui->tableView->setMinimumWidth (240);
        this->ui->spinBox->setEnabled (false);
    }

    connect (this->ui->import_csv, SIGNAL(clicked()), this, SLOT(importCSV()));
    connect (this->vModel, SIGNAL(setSpinBoxVal(int)), this->ui->spinBox, SLOT(setValue(int)));
}

connectionListDialog::~connectionListDialog (void)
{
    delete this->vModel;
    delete this->ui;
}

void connectionListDialog::accept (void)
{
    // We only need to copy the data values from newConn, as that's
    // all we've loaded in via this dialog.
    this->conn->copyDataValues (this->newConn);
    emit completed();
    delete this->newConn;
    delete this;
}

void connectionListDialog::reject (void)
{
    emit completed();
    delete this->newConn;
    delete this;
}

void connectionListDialog::importCSV (void)
{
    QString fileName = QFileDialog::getOpenFileName (this, tr("Open CSV file for import"),
                                                     qgetenv("HOME"),
                                                     tr("CSV files (*.csv *.txt);; All files (*.*)"));

    if (this->newConn->import_csv (fileName) == true) {
        // Import was successful
        this->vModel->deleteLater();
        this->vModel = new csv_connectionModel();
        this->vModel->setConnection (newConn);
        this->ui->tableView->setModel (this->vModel);
        connect (this->vModel, SIGNAL(setSpinBoxVal(int)), this->ui->spinBox, SLOT(setValue(int)));
        this->ui->spinBox->setValue (this->newConn->getNumRows());
        this->vModel->emitDataChanged();
    } // else import failed, nothing further to do.
}

void connectionListDialog::updateValSize (int val)
{
    this->vModel->insertConnRows (val);
}
