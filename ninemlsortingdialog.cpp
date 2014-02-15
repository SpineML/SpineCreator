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

#include "ninemlsortingdialog.h"
#include "ui_ninemlsortingdialog.h"
//#include "stringify.h"

NineMLSortingDialog::NineMLSortingDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NineMLSortingDialog)
{
    ui->setupUi(this);
}

NineMLSortingDialog::NineMLSortingDialog(rootData * data, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NineMLSortingDialog)
{

    ui->setupUi(this);
    this->dPtr = data;

    // set up drag and drop
    this->ui->model->setDragDropMode(QAbstractItemView::DragDrop);
    this->ui->model->setDefaultDropAction(Qt::MoveAction);
    this->ui->model->setSelectionMode(QAbstractItemView::ExtendedSelection);
    this->ui->temp->setDragDropMode(QAbstractItemView::DragDrop);
    this->ui->temp->setDefaultDropAction(Qt::MoveAction);
    this->ui->temp->setSelectionMode(QAbstractItemView::ExtendedSelection);
    this->ui->lib->setDragDropMode(QAbstractItemView::DragDrop);
    this->ui->lib->setDefaultDropAction(Qt::MoveAction);
    this->ui->lib->setSelectionMode(QAbstractItemView::ExtendedSelection);

    // add items
    for (unsigned int i = 0; i < this->dPtr->catalogUnsorted.size(); ++i) {
        if (this->dPtr->catalogUnsorted[i]->path != "none") {
            if (this->dPtr->catalogUnsorted[i]->path == "model")
                this->ui->model->addItem(this->dPtr->catalogUnsorted[i]->name + " (model)");
            if (this->dPtr->catalogUnsorted[i]->path == "temp")
               this->ui->temp->addItem(this->dPtr->catalogUnsorted[i]->name + " (temp)");
            if (this->dPtr->catalogUnsorted[i]->path == "lib")
               this->ui->lib->addItem(this->dPtr->catalogUnsorted[i]->name + " (lib)");
        }
    }

    for (unsigned int i = 0; i < this->dPtr->catalogNrn.size(); ++i) {
        if (this->dPtr->catalogNrn[i]->path != "none") {
            if (this->dPtr->catalogNrn[i]->path == "model")
               this->ui->model->addItem(this->dPtr->catalogNrn[i]->name + " (model)");
            if (this->dPtr->catalogNrn[i]->path == "temp")
               this->ui->temp->addItem(this->dPtr->catalogNrn[i]->name + " (temp)");
            if (this->dPtr->catalogNrn[i]->path == "lib")
               this->ui->lib->addItem(this->dPtr->catalogNrn[i]->name + " (lib)");
        }
    }

    for (unsigned int i = 0; i < this->dPtr->catalogWU.size(); ++i) {
        if (this->dPtr->catalogWU[i]->path != "none") {
            if (this->dPtr->catalogWU[i]->path == "model")
               this->ui->model->addItem(this->dPtr->catalogWU[i]->name + " (model)");
            if (this->dPtr->catalogWU[i]->path == "temp")
               this->ui->temp->addItem(this->dPtr->catalogWU[i]->name + " (temp)");
            if (this->dPtr->catalogWU[i]->path == "lib")
               this->ui->lib->addItem(this->dPtr->catalogWU[i]->name + " (lib)");
        }
    }

    for (unsigned int i = 0; i < this->dPtr->catalogPS.size(); ++i) {
        if (this->dPtr->catalogPS[i]->path != "none") {
            if (this->dPtr->catalogPS[i]->path == "model")
               this->ui->model->addItem(this->dPtr->catalogPS[i]->name + " (model)");
            if (this->dPtr->catalogPS[i]->path == "temp")
               this->ui->temp->addItem(this->dPtr->catalogPS[i]->name + " (temp)");
            if (this->dPtr->catalogPS[i]->path == "lib")
               this->ui->lib->addItem(this->dPtr->catalogPS[i]->name + " (lib)");
        }
    }

    QObject::connect(this->ui->del, SIGNAL(clicked()), this, SLOT(del()));
    QPushButton* applyButton = this->ui->buttonBox->button(QDialogButtonBox::Reset);
    QObject::connect(applyButton, SIGNAL(clicked()), this, SLOT(reset()));

}

NineMLSortingDialog::~NineMLSortingDialog()
{
    delete ui;
}


void NineMLSortingDialog::accept() {

    // switch around the components...

    vector < NineMLComponent * > * library;
    QListWidget * currWidget;
    QString widgetPath;

    // for each path (model / temp / lib) widget
    for (unsigned int srcInd = 0; srcInd < 3; ++srcInd) {

        // for each src
        switch (srcInd) {
        case 0:
            currWidget = ui->model;
            widgetPath = "model";
            break;
        case 1:
            currWidget = ui->temp;
            widgetPath = "temp";
            break;
        case 2:
            currWidget = ui->lib;
            widgetPath = "lib";
            break;
        }

        // for each entry in the widget
        for (unsigned int i = 0; i < (uint) currWidget->count(); ++i) {
            QString name = currWidget->item(i)->text();
            QString newName;

            // extract path from name
            int bracketIndex = name.lastIndexOf("(");
            QString currPath = name;
            currPath.remove(0,bracketIndex+1);
            currPath.chop(1);
            name.chop(name.count() - bracketIndex+1);

            // check if the path is changed
            if (currPath != widgetPath) {

                // path has changed so change the source component path

                // check for name conflicts between the namespaces
                bool uniqueName = false;

                while (!uniqueName) {
                    uniqueName = true;
                    for (unsigned int j = 0; j < (uint) currWidget->count(); ++j) {

                        // skip current index (don't compare with self)
                        if (j == i) continue;

                        QString name2 = currWidget->item(i)->text();

                        // extract name
                        int bracketIndex2 = name2.lastIndexOf("(");
                        name2.chop(name2.count() - bracketIndex2+1);

                        if ((name == name2 && newName.isEmpty()) || (newName == name2 && !newName.isEmpty())) {
                            newName = "";
                            // conflict! get alterative and loop
                            uniqueName = false;
                            bool ok = false;
                            while (newName == "") {
                                QInputDialog input;
                                input.setInputMode(QInputDialog::TextInput);
                                //input.setCancelButtonText("Overwrite");
                                newName = input.getText((QWidget *)this->parent(), tr("Name conflict"),
                                                                 tr("New component name:"), QLineEdit::Normal,
                                                                 name, &ok);
                                newName.replace("_", " ");
                            }
                            if (!ok) {
                               return;
                            }
                        }

                    }
                }


                // for each library
                for (unsigned int libraryInd = 0; libraryInd < 4; ++libraryInd) {

                    // for each dest
                    switch (libraryInd) {
                    case 0:
                        library = &this->dPtr->catalogUnsorted;
                        break;
                    case 1:
                        library = &this->dPtr->catalogNrn;
                        break;
                    case 2:
                        library = &this->dPtr->catalogPS;
                        break;
                    case 3:
                        library = &this->dPtr->catalogWU;
                        break;
                    }

                    // find the component with that name:
                    for (unsigned int j = 1; j < library->size(); ++j) {
                        if ((*library)[j]->name == name) {
                            // change the path
                            (*library)[j]->path = widgetPath;
                            if (!newName.isEmpty())
                                (*library)[j]->name = newName;
                        }
                    }
                }
            }



        }

    }

    // delete any unfound components...

    delete this;
}

void NineMLSortingDialog::reject() {

    delete this;

}

void NineMLSortingDialog::reset() {


    // clear changes
    this->ui->model->clear();
    this->ui->temp->clear();
    this->ui->lib->clear();

    // add items again

    // add items
    for (unsigned int i = 0; i < this->dPtr->catalogUnsorted.size(); ++i) {
        if (this->dPtr->catalogUnsorted[i]->path != "none") {
            if (this->dPtr->catalogUnsorted[i]->path == "model")
                this->ui->model->addItem(this->dPtr->catalogUnsorted[i]->name + " (model)");
            if (this->dPtr->catalogUnsorted[i]->path == "temp")
               this->ui->temp->addItem(this->dPtr->catalogUnsorted[i]->name + " (temp)");
            if (this->dPtr->catalogUnsorted[i]->path == "lib")
               this->ui->lib->addItem(this->dPtr->catalogUnsorted[i]->name + " (lib)");
        }
    }

    for (unsigned int i = 0; i < this->dPtr->catalogNrn.size(); ++i) {
        if (this->dPtr->catalogNrn[i]->path != "none") {
            if (this->dPtr->catalogNrn[i]->path == "model")
               this->ui->model->addItem(this->dPtr->catalogNrn[i]->name + " (model)");
            if (this->dPtr->catalogNrn[i]->path == "temp")
               this->ui->temp->addItem(this->dPtr->catalogNrn[i]->name + " (temp)");
            if (this->dPtr->catalogNrn[i]->path == "lib")
               this->ui->lib->addItem(this->dPtr->catalogNrn[i]->name + " (lib)");
        }
    }

    for (unsigned int i = 0; i < this->dPtr->catalogWU.size(); ++i) {
        if (this->dPtr->catalogWU[i]->path != "none") {
            if (this->dPtr->catalogWU[i]->path == "model")
               this->ui->model->addItem(this->dPtr->catalogWU[i]->name + " (model)");
            if (this->dPtr->catalogWU[i]->path == "temp")
               this->ui->temp->addItem(this->dPtr->catalogWU[i]->name + " (temp)");
            if (this->dPtr->catalogWU[i]->path == "lib")
               this->ui->lib->addItem(this->dPtr->catalogWU[i]->name + " (lib)");
        }
    }

    for (unsigned int i = 0; i < this->dPtr->catalogPS.size(); ++i) {
        if (this->dPtr->catalogPS[i]->path != "none") {
            if (this->dPtr->catalogPS[i]->path == "model")
               this->ui->model->addItem(this->dPtr->catalogPS[i]->name + " (model)");
            if (this->dPtr->catalogPS[i]->path == "temp")
               this->ui->temp->addItem(this->dPtr->catalogPS[i]->name + " (temp)");
            if (this->dPtr->catalogPS[i]->path == "lib")
               this->ui->lib->addItem(this->dPtr->catalogPS[i]->name + " (lib)");
        }
    }

}

void NineMLSortingDialog::del() {

    QList < QListWidgetItem * > items;

    // get selected items and delete
    items = this->ui->model->selectedItems();
    qDeleteAll(items);

    // get selected items and delete
    items = this->ui->temp->selectedItems();
    qDeleteAll(items);

    // get selected items and delete
    items = this->ui->lib->selectedItems();
    qDeleteAll(items);

}
