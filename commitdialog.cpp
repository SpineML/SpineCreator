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

#include "commitdialog.h"
#include "ui_commitdialog.h"

commitDialog::commitDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::commitDialog)
{
    ui->setupUi(this);

}

void commitDialog::showLog(QString log) {
    this->setWindowTitle("Repository log");
    ui->label->setText("Log for repository:");
    ui->plainTextEdit->setPlainText(log);
    ui->plainTextEdit->setReadOnly(true);
}

void commitDialog::showStatus(QString status) {
    this->setWindowTitle("Repository status");
    ui->label->setText("Status of repository:");
    ui->plainTextEdit->setPlainText(status);
    ui->plainTextEdit->setReadOnly(true);
}

QString commitDialog::getString() {
    return ui->plainTextEdit->toPlainText();
}

commitDialog::~commitDialog()
{
    delete ui;
}
