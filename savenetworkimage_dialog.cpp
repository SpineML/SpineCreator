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

#include "savenetworkimage_dialog.h"
#include "ui_savenetworkimage_dialog.h"
#include "rootdata.h"
#include "glconnectionwidget.h"

saveNetworkImageDialog::saveNetworkImageDialog(rootData * data, QString fileName, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::saveNetworkImageDialog)
{
    ui->setupUi(this);

    height = -1;

    this->setWindowTitle("Image export settings for network");

    // set data in place
    this->data = data;
    this->fileName = fileName;

    // setup combobox
    ui->comboBox->addItem("Circle and Arrow");
    ui->comboBox->setCurrentIndex(0);

    // setup sizes
    ui->scale_spin->setValue(1.0);
    ui->scale_spin->setMaximum(7.0);
    ui->scale_spin->setMinimum(0.1);
    scale = 1.0;
    connect(ui->scale_spin,SIGNAL(valueChanged(double)), this, SLOT(changeScale(double)));
    ui->border_spin->setValue(1.0);
    border = 1.0;
    ui->scale_spin->setMaximum(5.0);
    connect(ui->border_spin,SIGNAL(valueChanged(double)), this, SLOT(changeBorder(double)));

    // setup listView

    // setup preview
    QPixmap pix = drawPixMap();

    // width and height:
    ui->height_label->setText("Height = " + QString::number(pix.height()));
    ui->width_label->setText("Width = " + QString::number(pix.width()));

    pix = pix.scaled(ui->preview->size(),Qt::KeepAspectRatio,Qt::SmoothTransformation);

    ui->preview->setPixmap(pix);

    // for now
    ui->objects->setVisible(false);
    ui->label_2->setVisible(false);

    connect(ui->buttonBox,SIGNAL(accepted()), this, SLOT(save()));

}

saveNetworkImageDialog::saveNetworkImageDialog(glConnectionWidget * glConnWidget, QString fileName, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::saveNetworkImageDialog)
{
    ui->setupUi(this);

    this->setWindowTitle("Image export settings for visualisation");

    // set data in place
    this->glConnWidget = glConnWidget;
    this->fileName = fileName;

    // setup combobox
    ui->comboBox->hide();

    // setup sizes
    ui->scaleLabel->setText("Width");
    ui->borderLabel->setText("Height");

    ui->height_label->hide();
    ui->width_label->hide();

    ui->scale_spin->setMaximum(20000.0);
    ui->scale_spin->setMinimum(100);
    ui->scale_spin->setValue(1000.0);

    ui->border_spin->setMaximum(20000.0);
    ui->border_spin->setMinimum(100);
    ui->border_spin->setValue(1000.0);

    width = 1000;
    height = 1000;

    connect(ui->scale_spin,SIGNAL(valueChanged(double)), this, SLOT(changeWidth(double)));
    connect(ui->border_spin,SIGNAL(valueChanged(double)), this, SLOT(changeHeight(double)));

    // setup listView
    ui->objects->hide();

    // setup preview
    QPixmap pix = drawPixMapVis();

    pix = pix.scaled(ui->preview->size(),Qt::KeepAspectRatio,Qt::SmoothTransformation);

    ui->preview->setPixmap(pix);

    // for now
    ui->objects->setVisible(false);
    ui->label_2->setVisible(false);

    connect(ui->buttonBox,SIGNAL(accepted()), this, SLOT(save()));

}

saveNetworkImageDialog::~saveNetworkImageDialog()
{
    delete ui;
}

void saveNetworkImageDialog::reDrawPreview() {

    QPixmap pix;
    if (height > 0)
        pix = drawPixMapVis();
    else {
        pix = drawPixMap();
        ui->height_label->setText("Height = " + QString::number(pix.height()));
        ui->width_label->setText("Width = " + QString::number(pix.width()));
    }

    pix = pix.scaled(ui->preview->size(),Qt::KeepAspectRatio,Qt::SmoothTransformation);

    ui->preview->setPixmap(pix);

}

QPixmap saveNetworkImageDialog::drawPixMap() {

    // what to draw
    QVector <QSharedPointer<systemObject> > list;
    list = data->selList;

    QRectF bounds = QRectF(100000,100000,-200000,-200000);

    // work out bounding box
    for (int p = 0; p < list.size(); ++p) {

        if (list[p]->type == populationObject) {
            QSharedPointer <population> pop = qSharedPointerDynamicCast <population> (list[p]);
            if (-pop->bottomBound(pop->targy)> bounds.bottom())
                bounds.setBottom(-pop->bottomBound(pop->targy));
            if (-pop->topBound(pop->targy)< bounds.top())
                bounds.setTop(-pop->topBound(pop->targy));
            if (pop->leftBound(pop->targx)< bounds.left())
                bounds.setLeft(pop->leftBound(pop->targx));
            if (pop->rightBound(pop->targx)> bounds.right())
                bounds.setRight(pop->rightBound(pop->targx));
        }


        if (list[p]->type == projectionObject) {

            QSharedPointer <projection> proj = qSharedPointerDynamicCast <projection> (list[p]);
            for (int c = 0; c < proj->curves.size(); ++c) {
                bezierCurve * bz = &proj->curves[c];
                if (-bz->C1.y() > bounds.bottom())
                    bounds.setBottom(-bz->C1.y());
                if (-bz->C1.y() < bounds.top())
                    bounds.setTop(-bz->C1.y());
                if (bz->C1.x() < bounds.left())
                    bounds.setLeft(bz->C1.x());
                if (bz->C1.x() > bounds.right())
                    bounds.setRight(bz->C1.x());

                if (-bz->C2.y() > bounds.bottom())
                    bounds.setBottom(-bz->C2.y());
                if (-bz->C2.y() < bounds.top())
                    bounds.setTop(-bz->C2.y());
                if (bz->C2.x() < bounds.left())
                    bounds.setLeft(bz->C2.x());
                if (bz->C2.x() > bounds.right())
                    bounds.setRight(bz->C2.x());
            }

        }

    }

    bounds.setTopLeft(bounds.topLeft() - QPointF(border,border));
    bounds.setBottomRight(bounds.bottomRight() + QPointF(border,border));

    QPixmap outPix(bounds.width()*100*scale, bounds.height()*100*scale);

    if (ui->checkBox->isChecked()) {
        outPix.fill(Qt::transparent);
    } else {
        outPix.fill();// white
    }

    QPainter *painter = new QPainter(&outPix);

    painter->setRenderHint(QPainter::HighQualityAntialiasing, true);
    painter->setRenderHint(QPainter::Antialiasing,true);
    painter->setRenderHint( QPainter::TextAntialiasing,true);
    painter->setRenderHint(QPainter::SmoothPixmapTransform,true);

    // setup the painter
    QFont font("Monospace", 5.0f);
    font.setStyleHint(QFont::TypeWriter);
    painter->setFont(font);



    // Just render selection:
    for (int i = 0; i < list.size(); ++i) {

            list[i]->draw(painter, 200.0*scale, -bounds.center().x(), -bounds.center().y(), bounds.width()*100*scale, bounds.height()*100*scale, data->popImage, microcircuitDrawStyle);

    }

    //QImage outIm = outPix->toImage();
    //outIm.save("/home/alex/test_image.png","png");

    painter->end();

    delete painter;
    return outPix;
}

QPixmap saveNetworkImageDialog::drawPixMapVis() {

    return glConnWidget->renderImage(width, height);

    /*image.save("/home/alex/test.png", "png");

    QPixmap pix;
    pix.fromImage(image);

    return pix;*/

}

void saveNetworkImageDialog::changeScale(double value) {
    scale = value;
    reDrawPreview();
}

void saveNetworkImageDialog::changeBorder(double value) {
    border = value;
    reDrawPreview();
}

void saveNetworkImageDialog::changeWidth(double value) {
    width = round(value);
    ((QDoubleSpinBox *) sender())->setValue(round(value));
    reDrawPreview();
}

void saveNetworkImageDialog::changeHeight(double value) {
    height = round(value);
    ((QDoubleSpinBox *) sender())->setValue(round(value));
    reDrawPreview();
}


void saveNetworkImageDialog::save() {

    QPixmap pix;
    if (height > 0)
        pix = drawPixMapVis();
    else
        pix = drawPixMap();

    QImage outIm = pix.toImage();
    outIm.save(fileName,"png");

}
