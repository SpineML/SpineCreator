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

#include "versionchange_dialog.h"
#include "ui_versionchange_dialog.h"

versionChange_Dialog::versionChange_Dialog(versionNumber *ver, versionNumber *minVer, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::versionChange_Dialog)
{
    ui->setupUi(this);
    this->ver = ver;
    this->minVer = minVer;

    ui->majorNumberSpinBox->setValue(minVer->major);
    ui->majorNumberSpinBox->setMinimum(minVer->major);

    ui->minorNumberSpinBox->setValue(minVer->minor);
    ui->minorNumberSpinBox->setMinimum(minVer->minor);

    ui->revisionSpinBox->setValue(minVer->revision);
    ui->revisionSpinBox->setMinimum(minVer->revision);

    connect(ui->minorNumberSpinBox, SIGNAL(valueChanged(int)), this, SLOT(changed(int)));
    connect(ui->majorNumberSpinBox, SIGNAL(valueChanged(int)), this, SLOT(changed(int)));
    connect(ui->revisionSpinBox, SIGNAL(valueChanged(int)), this, SLOT(changed(int)));

    ui->buttonBox->button(QDialogButtonBox::Ok)->setDisabled(true);

    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(complete()));

}

versionChange_Dialog::~versionChange_Dialog()
{
    delete ui;
}

void versionChange_Dialog::complete() {

    ver->major = ui->majorNumberSpinBox->value();
    ver->minor = ui->minorNumberSpinBox->value();
    ver->revision = ui->revisionSpinBox->value();

    this->accept();

}

void versionChange_Dialog::changed(int) {

    if (ui->majorNumberSpinBox->value() == minVer->major \
            && ui->minorNumberSpinBox->value() == minVer->minor \
            && ui->revisionSpinBox->value() == minVer->revision) {
        ui->buttonBox->button(QDialogButtonBox::Ok)->setDisabled(true);
    } else {
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
    }

}
