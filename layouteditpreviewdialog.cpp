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

#include "layouteditpreviewdialog.h"

layoutEditPreviewDialog::layoutEditPreviewDialog(QSharedPointer<NineMLLayout> inSrcNineMLLayout, glConnectionWidget * glConn, QWidget *parent) :
    QDialog(parent)
{

    srcNineMLLayout = inSrcNineMLLayout;
    glView = glConn;
    QObject::connect(this, SIGNAL(drawLayout(vector<loc>)), glView, SLOT(drawLocations(vector<loc>)));

    this->setModal(true);
    this->setMinimumSize(200, 400);
    this->setWindowTitle("Preview layout");
    //this->setWindowIcon();

    // add the main layout
    this->setLayout(new QVBoxLayout);
    this->layout()->setContentsMargins(0,0,0,0);

    // make the dialog scrollable
    this->layout()->addWidget(new QScrollArea);
    QWidget * content = new QWidget;
    ((QScrollArea *) this->layout()->itemAt(0)->widget())->setWidget(content);
    ((QScrollArea *) this->layout()->itemAt(0)->widget())->setWidgetResizable(true);
    content->setLayout(new QFormLayout);
    QFormLayout * contentLayout = (QFormLayout *) content->layout();

    QSpinBox * numNeurons = new QSpinBox;
    numNeurons->setRange(1, 20000000);
    numNeurons->setProperty("name", "numNeurons");
    numNeurons->setValue(99.0);
    QObject::connect(numNeurons, SIGNAL(valueChanged(QString)), this, SLOT(reDraw(QString)));


    contentLayout->addRow("Number of neurons:", numNeurons);


    // start adding the items
    if (srcNineMLLayout->ParameterList.size() > 1) {
        contentLayout->addRow("<b>Parameters</b>", new QLabel(""));
    }

    // add the parameters
    for (uint i = 0; i < srcNineMLLayout->ParameterList.size(); ++i) {

        if (srcNineMLLayout->ParameterList[i]->name != "numNeurons") {
            // add to main layout
            QDoubleSpinBox * value = new QDoubleSpinBox;
            value->setRange(-200000, 200000);
            value->setSingleStep(0.1);
            value->setDecimals(3);
            value->setValue(1.0);
            value->setProperty("name", srcNineMLLayout->ParameterList[i]->name);
            QObject::connect(value, SIGNAL(valueChanged(QString)), this, SLOT(reDraw(QString)));
            contentLayout->addRow(srcNineMLLayout->ParameterList[i]->name + ":", value);
        }

    }

    if (srcNineMLLayout->StateVariableList.size() > 3) {
        contentLayout->addRow("<b>State variables</b>", new QLabel(""));
    }

    // add the state vars
    for (uint i = 0; i < srcNineMLLayout->StateVariableList.size(); ++i) {

        if (srcNineMLLayout->StateVariableList[i]->name != "x" && srcNineMLLayout->StateVariableList[i]->name != "y" && srcNineMLLayout->StateVariableList[i]->name != "z") {
            // add to main layout
            QDoubleSpinBox * value = new QDoubleSpinBox;
            value->setRange(-200000, 200000);
            value->setSingleStep(0.1);
            value->setDecimals(3);
            value->setValue(1.0);
            value->setProperty("name", srcNineMLLayout->StateVariableList[i]->name);
            QObject::connect(value, SIGNAL(valueChanged(QString)), this, SLOT(reDraw(QString)));
            contentLayout->addRow(srcNineMLLayout->StateVariableList[i]->name + ":", value);
        }

    }

    contentLayoutRef = contentLayout;

    // force a redraw!
    numNeurons->setValue(100.0);
}

void layoutEditPreviewDialog::reDraw(QString) {

    // create a new layoutData from the layout:

    QSharedPointer<NineMLLayoutData> data = QSharedPointer<NineMLLayoutData> (new NineMLLayoutData(srcNineMLLayout));

    // populate from the spinboxes:
    for (uint i = 0; i < data->ParameterList.size(); ++i) {
        for (uint j = 0; j < (uint) contentLayoutRef->count(); ++j) {
            if (contentLayoutRef->itemAt(j)->widget()) {
                QString clsName = "QDoubleSpinBox";
                if (clsName.contains(contentLayoutRef->itemAt(j)->widget()->metaObject()->className())) {
                    if (contentLayoutRef->itemAt(j)->widget()->property("name").toString() == data->ParameterList[i]->name) {
                        data->ParameterList[i]->value[0] = ((QDoubleSpinBox *) contentLayoutRef->itemAt(j)->widget())->value();
                    }
                }
            }
        }
    }

    for (uint i = 0; i < data->StateVariableList.size(); ++i) {
        for (uint j = 0; j < (uint) contentLayoutRef->count(); ++j) {
            if (contentLayoutRef->itemAt(j)->widget()) {
                QString clsName = "QDoubleSpinBox";
                if (clsName.contains(contentLayoutRef->itemAt(j)->widget()->metaObject()->className())) {
                    if (contentLayoutRef->itemAt(j)->widget()->property("name").toString() == data->StateVariableList[i]->name) {
                        data->StateVariableList[i]->value[0] = ((QDoubleSpinBox *) contentLayoutRef->itemAt(j)->widget())->value();
                    }
                }
            }
        }
    }

    int numNeurons;
    for (uint j = 0; j < (uint) contentLayoutRef->count(); ++j) {
        if (contentLayoutRef->itemAt(j)->widget()) {
            QString clsName = "QSpinBox";
            if (clsName.contains(contentLayoutRef->itemAt(j)->widget()->metaObject()->className())) {
                numNeurons = ((QSpinBox *) contentLayoutRef->itemAt(j)->widget())->value();
             }
        }
    }


    // ok, data is filled in, now to get the locations
    vector <loc> locations;

    QString err;
    data->generateLayout(numNeurons, &locations, err);

    if (err.size() == 0) {
        emit drawLayout(locations);
    } else
    {
        emit drawLayout(locations);
        QMessageBox msgBox;
        msgBox.setText(err);
        msgBox.exec();
    }

}




