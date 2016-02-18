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
**           Author: Paul Richmond                                        **
**  Website/Contact: http://bimpa.group.shef.ac.uk/                       **
****************************************************************************/

#include "SC_export_component_image.h"
#include "ui_exportimage.h"

ExportImageDialog::ExportImageDialog(int width, int height, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ExportImageDialog)
{
    ui->setupUi(this);
    ui->ImageWidth->setMaximum(width*ExportImageDialog::MAX_IMAGE_SCALE);
    ui->ImageHeight->setMaximum(height*ExportImageDialog::MAX_IMAGE_SCALE);

    ui->ImageWidth->setValue(width*1.0);
    ui->ImageHeight->setValue(height*1.0);
    aspect_ratio = (double)width/(double)height;

    connect(ui->ImageWidth, SIGNAL(valueChanged(int)), this, SLOT(widthChanged(int)));
    connect(ui->ImageHeight, SIGNAL(valueChanged(int)), this, SLOT(heightChanged(int)));
}

ExportImageDialog::~ExportImageDialog()
{
    delete ui;
    ui = NULL;
}

int ExportImageDialog::getWidth()
{
    return ui->ImageWidth->value();
}

int ExportImageDialog::getHeight()
{
    return ui->ImageHeight->value();
}

bool ExportImageDialog::getAliasing()
{
    return ui->Aliasing->isChecked();
}

void ExportImageDialog::widthChanged(int width)
{
    if (ui->LockAspect->isChecked())
    {
        int height = (int)((double)width/aspect_ratio);

        disconnect(ui->ImageHeight, SIGNAL(valueChanged(int)), this, SLOT(heightChanged(int)));
        ui->ImageHeight->setValue(height);
        connect(ui->ImageHeight, SIGNAL(valueChanged(int)), this, SLOT(heightChanged(int)));

    }
}

void ExportImageDialog::heightChanged(int height)
{
    if (ui->LockAspect->isChecked())
    {
        int width = (int)(aspect_ratio*(double)height);

        disconnect(ui->ImageWidth, SIGNAL(valueChanged(int)), this, SLOT(widthChanged(int)));
        ui->ImageWidth->setValue(width);
        connect(ui->ImageWidth, SIGNAL(valueChanged(int)), this, SLOT(widthChanged(int)));
    }
}
