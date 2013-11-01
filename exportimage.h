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
**           Author: Paul Richmond                                        **
**  Website/Contact: http://bimpa.group.shef.ac.uk/                       **
****************************************************************************/

#ifndef EXPORTIMAGE_H
#define EXPORTIMAGE_H

#include <QDialog>

namespace Ui {
class ExportImageDialog;
}

class ExportImageDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit ExportImageDialog(int width, int height, QWidget *parent = 0);
    ~ExportImageDialog();

    int getWidth();
    int getHeight();
    bool getAliasing();
    
private slots:
    void widthChanged(int arg1);
    void heightChanged(int arg1);

public:
    static const int MAX_IMAGE_SCALE = 20;

private:
    Ui::ExportImageDialog *ui;
    double aspect_ratio;
};

#endif // EXPORTIMAGE_H
