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

#ifndef VERSIONCHANGE_DIALOG_H
#define VERSIONCHANGE_DIALOG_H

#include <QDialog>
#include "globalHeader.h"

namespace Ui {
class versionChange_Dialog;
}

class versionChange_Dialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit versionChange_Dialog(versionNumber *ver, versionNumber *minVer, QWidget *parent = 0);
    ~versionChange_Dialog();
    
private:
    Ui::versionChange_Dialog *ui;
    versionNumber * ver;
    versionNumber * minVer;

public slots:
    void complete();
    void changed(int);
};

#endif // VERSIONCHANGE_DIALOG_H
