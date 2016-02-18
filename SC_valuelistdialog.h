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

#ifndef VALUELISTDIALOG_H
#define VALUELISTDIALOG_H

#include <QDialog>
#include "globalHeader.h"
#include "CL_classes.h"
#include "SC_vectormodel.h"

namespace Ui {
class valueListDialog;
}

class valueListDialog : public QDialog
{
    Q_OBJECT

public:
    explicit valueListDialog(ParameterInstance *par, QWidget *parent=0);
    ~valueListDialog();

private:
    Ui::valueListDialog *ui;
    vectorModel * vModel;
    QVector <double> vals;
    QVector <int> inds;
    ParameterInstance * par;
    void import_csv(QString);
    void keyPressEvent(QKeyEvent *evt);

public slots:
    void accept();
    void reject();
    void updateValSize();
    void import();

};

#endif // VALUELISTDIALOG_H
