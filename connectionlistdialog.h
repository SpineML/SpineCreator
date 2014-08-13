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

#ifndef CONNECTIONLISTDIALOG_H
#define CONNECTIONLISTDIALOG_H

#include <QDialog>
#include "globalHeader.h"

namespace Ui {
class connectionListDialog;
}

class connectionListDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit connectionListDialog(csv_connection * conn, QWidget *parent = 0);
    ~connectionListDialog();
    
private:
    Ui::connectionListDialog *ui;
    csv_connectionModel * vModel;
    csv_connection * conn;

signals:
    void completed();

public slots:
    void accept();
    void reject();
    void importCSV();
    void updateValSize(int val);


};

#endif // CONNECTIONLISTDIALOG_H
