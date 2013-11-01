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

#ifndef SAVENETWORKIMAGE_DIALOG_H
#define SAVENETWORKIMAGE_DIALOG_H

#include <QDialog>
#include "globalHeader.h"

namespace Ui {
class saveNetworkImageDialog;
}

class saveNetworkImageDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit saveNetworkImageDialog(rootData *data, QString fileName, QWidget *parent = 0);
    explicit saveNetworkImageDialog(glConnectionWidget *glConnWidget, QString fileName, QWidget *parent = 0);
    ~saveNetworkImageDialog();
    
private:
    Ui::saveNetworkImageDialog *ui;
    rootData * data;
    glConnectionWidget * glConnWidget;
    QPixmap drawPixMap();
    QPixmap drawPixMapVis();
    float scale;
    float border;
    int height;
    int width;
    void reDrawPreview();
    QString fileName;

public slots:
    void changeScale(double);
    void changeBorder(double);
    void changeWidth(double);
    void changeHeight(double);
    void save();
};

#endif // SAVENETWORKIMAGE_DIALOG_H
