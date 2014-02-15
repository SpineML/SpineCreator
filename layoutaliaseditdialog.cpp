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

#include "layoutaliaseditdialog.h"

QHBoxLayout * LayoutAliasEditDialog::drawAlias(Alias * currentAlias) {

    QHBoxLayout * aliasLayout = new QHBoxLayout;

    QLineEdit * aliasName = new QLineEdit(currentAlias->name);
    aliasName->setProperty("ptr", qVariantFromValue((void *) currentAlias));
    QObject::connect(aliasName, SIGNAL(editingFinished()), this, SLOT(updateName()));
    aliasLayout->addWidget(aliasName, 1);

    QLineEdit * aliasMaths = new QLineEdit(currentAlias->maths->equation);
    aliasMaths->setProperty("ptr", qVariantFromValue((void *) currentAlias));
    QObject::connect(aliasMaths, SIGNAL(editingFinished()), this, SLOT(updateMaths()));
    aliasLayout->addWidget(aliasMaths, 3);

    QPushButton * aliasDelete = new QPushButton;
    aliasDelete->setIcon(QIcon(":/icons/toolbar/delShad.png"));
    aliasDelete->setProperty("ptr", qVariantFromValue((void *) currentAlias));
    aliasDelete->setProperty("layout", qVariantFromValue((void *) aliasLayout));
    aliasDelete->setFlat(true);
    aliasDelete->setMaximumWidth(28);
    aliasDelete->setMaximumHeight(28);
    aliasDelete->setFocusPolicy(Qt::NoFocus);
    QObject::connect(aliasDelete, SIGNAL(clicked()), this, SLOT(deleteAlias()));
    aliasLayout->addWidget(aliasDelete);

    return aliasLayout;

}

LayoutAliasEditDialog::LayoutAliasEditDialog(NineMLLayout * inSrcNineMLLayout, QWidget *parent) :
    QDialog(parent)
{

    srcNineMLLayout = inSrcNineMLLayout;

    this->setModal(true);
    this->setMinimumSize(450, 400);
    this->setWindowTitle("Edit Aliases");
    //this->setWindowIcon();

    // width of the delete box:
    int delWidth = 30;

    // add the main layout
    this->setLayout(new QVBoxLayout);
    this->layout()->setContentsMargins(0,0,0,0);

    // make the dialog scrollable
    this->layout()->addWidget(new QScrollArea);
    QWidget * content = new QWidget;
    ((QScrollArea *) this->layout()->itemAt(0)->widget())->setWidget(content);
    ((QScrollArea *) this->layout()->itemAt(0)->widget())->setWidgetResizable(true);
    content->setLayout(new QVBoxLayout);
    QVBoxLayout * contentLayout = (QVBoxLayout *) content->layout();

    // start adding the items, header first:
    QHBoxLayout * header = new QHBoxLayout;
    header->addWidget(new QLabel("<b>Name</b>"), 1);
    header->addWidget(new QLabel("<b>Maths</b>"),3);
    header->addWidget(new QLabel(""));
    header->itemAt(2)->widget()->setFixedWidth(delWidth);
    contentLayout->addLayout(header);

    // add the aliases
    for (uint i = 0; i < srcNineMLLayout->AliasList.size(); ++i) {

        // add to main layout
        contentLayout->addLayout(drawAlias(srcNineMLLayout->AliasList[i]));

    }

    QFont addFont("Helvetica [Cronyx]", 8);

    // add new alias button
    QPushButton * addAlias = new QPushButton("Alias");
    addAlias->setIcon(QIcon(":/icons/toolbar/addShad.png"));
    addAlias->setMaximumWidth(110);
    addAlias->setFlat(true);
    addAlias->setFocusPolicy(Qt::NoFocus);
    addAlias->setToolTip("Add a new Alias");
    addAlias->setFont(addFont);
    addAlias->setProperty("layout", qVariantFromValue((void *) contentLayout));
    QObject::connect(addAlias, SIGNAL(clicked()), this, SLOT(addAlias()));
    contentLayout->addWidget(addAlias);

    // make sure it all goes to the top of the dialog
    contentLayout->addStretch();

    QHBoxLayout * exitLayout = new QHBoxLayout;
    exitLayout->addStretch();
    QPushButton * exit = new QPushButton("Finished");
    exit->setMaximumWidth(100);
    QObject::connect(exit, SIGNAL(clicked()), this, SLOT(close()));
    exitLayout->addWidget(exit);
    contentLayout->addLayout(exitLayout);

}

void LayoutAliasEditDialog::updateName() {

    QLineEdit * src = (QLineEdit *) sender();

    Alias * currAlias = (Alias *) src->property("ptr").value<void *>();

    QString newName = src->text().replace(" ", "");

    // check if name unique:
    for (uint i = 0; i < this->srcNineMLLayout->AliasList.size(); ++i) {
        if (newName == this->srcNineMLLayout->AliasList[i]->name) {
            // not unique, reset and return
            src->setText(currAlias->name);
            return;
        }
    }

    // unique: update the alias
    currAlias->name = newName;
    src->setText(currAlias->name);

}

void LayoutAliasEditDialog::updateMaths() {

    QLineEdit * src = (QLineEdit *) sender();

    Alias * currAlias = (Alias *) src->property("ptr").value<void *>();

    QString newMaths = src->text();

    currAlias->maths->equation = newMaths;

    QStringList errs;
    currAlias->maths->validateMathInLine(this->srcNineMLLayout, &errs);

    if (errs.size() > 0) {
        QPalette p = src->palette();
        p.setColor( QPalette::Normal, QPalette::Base, QColor(255, 200, 200) );
        src->setPalette(p);
    } else {
        QPalette p = src->palette();
        p.setColor( QPalette::Normal, QPalette::Base, QColor(255, 255, 255) );
        src->setPalette(p);
    }
}

void recursiveDeleteLater2(QLayout * parentLayout) {

    QLayoutItem * item;
    while ((item = parentLayout->takeAt(0))) {
        if (item->widget())
            item->widget()->deleteLater();
        if (item->layout())
            recursiveDeleteLater2(item->layout());
    }
    parentLayout->deleteLater();

}

void LayoutAliasEditDialog::deleteAlias() {

    QPushButton * src = (QPushButton *) sender();

    Alias * currAlias = (Alias *) src->property("ptr").value<void *>();
    QHBoxLayout * aliasLayout = (QHBoxLayout *) src->property("layout").value<void *>();

    for (uint i = 0; i < srcNineMLLayout->AliasList.size(); ++i) {
        if (srcNineMLLayout->AliasList[i] == currAlias) {
            srcNineMLLayout->AliasList.erase(srcNineMLLayout->AliasList.begin()+i, srcNineMLLayout->AliasList.begin()+i+1);
            recursiveDeleteLater2(aliasLayout);
            return;
        }
    }

}

void LayoutAliasEditDialog::addAlias() {

    QPushButton * src = (QPushButton *) sender();

    QVBoxLayout * contentLayout = (QVBoxLayout *) src->property("layout").value<void *>();

    srcNineMLLayout->AliasList.push_back(new Alias());
    srcNineMLLayout->AliasList.back()->name = "newAlias";
    srcNineMLLayout->AliasList.back()->maths = new MathInLine;
    srcNineMLLayout->AliasList.back()->maths->equation = "0";

    // add new Alias to display
    contentLayout->insertLayout(contentLayout->count() - 3, drawAlias(srcNineMLLayout->AliasList.back()));

}
