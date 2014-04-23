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

#include "valuelistdialog.h"
#include "ui_valuelistdialog.h"

valueListDialog::valueListDialog(ParameterData * par, NineMLComponent * /*comp*/, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::valueListDialog)
{
    ui->setupUi(this);

    ui->spinBox->setRange(0, 1000000); // set max from the component
    ui->spinBox->setValue(par->value.size());

    connect(ui->spinBox,SIGNAL(valueChanged(int)), this, SLOT(updateValSize(int)));

    this->par = par;

    this->vals = par->value;
    this->inds = par->indices;

    this->vModel = new vectorModel();
    this->vModel->setPointer(par);
    this->ui->tableView->setModel(this->vModel);
    QHeaderView * header = this->ui->tableView->horizontalHeader();
    header->setStretchLastSection(true);
    //header->setResizeMode(0, QHeaderView::Fixed);
    header->resizeSection(0, 80);

    connect(this->vModel,SIGNAL(setSpinBoxVal(int)), ui->spinBox, SLOT(setValue(int)));
    connect(this->ui->import_csv,SIGNAL(clicked()), this, SLOT(import()));
}

valueListDialog::~valueListDialog()
{
    delete vModel;
    delete ui;
}

void valueListDialog::accept() {

    // nothing to do
    delete this;

}

void valueListDialog::reject() {

    // restore values:
    par->value = vals;
    par->indices = inds;

    delete this;

}

void valueListDialog::import() {

    QString fileName = QFileDialog::getOpenFileName(this, tr("Open CSV file for import"), qgetenv("HOME"), tr("CSV files (*.csv *.txt);; All files (*.*)"));

    import_csv(fileName);

}

void valueListDialog::import_csv(QString fileName) {

    // open the input csv file for reading
    QFile fileIn(fileName);

    if( !fileIn.open( QIODevice::ReadOnly ) ) {
        QMessageBox msgBox;
        msgBox.setText("Could not open the selected file");
        msgBox.exec();
        return;}

    // use textstream so we can read lines into a QString
    QTextStream stream(&fileIn);

    // number of columns
    int numFields = -1;
    // column to load
    int col = 0;

    int counter = 0;

    par->value.clear();
    par->indices.clear();

    // load in the csv line by line
    while (!(stream.atEnd())) {

        // get a line
        QString line = stream.readLine();

        // see if it is a comment:
        if (line[0] == '#') {
            continue;
        }

        // or a blank line
        QString line2 = line;
        line2.replace(" ", "");
        if (line2 == "\n") {
            continue;
        }

        // not a comment - so begin parsing and create an xml element
        QStringList fields = line.split(",");

        if (fields.size() > 1 && numFields == -1) {
            col = QInputDialog::getInt(this,"Choose column", "Column to load", 1, 1, fields.size()) - 1;
            }

        if (fields.size() < 1) {
            QMessageBox msgBox;
            msgBox.setText("CSV file could not be read");
            msgBox.exec();
            par->value.clear();
            par->indices.clear();
            this->ui->spinBox->setValue(par->value.size());
            return;}

        if (numFields == -1) {
            numFields = fields.size();
        } else if (numFields != fields.size()) {
            std::cerr << "something is wrong!\n";
            continue;
        }

        // for the chosen field
        float num = fields[col].toFloat();
        par->value.push_back(num);
        par->indices.push_back(counter);

        ++counter;
    }

    fileIn.close();

    this->ui->spinBox->setValue(par->value.size());
    this->vModel->emitDataChanged();
}

void valueListDialog::updateValSize(int val) {

    /*bool returnVal;
    returnVal = */this->vModel->insertConnRows(val);

}
