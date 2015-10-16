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

#ifndef ROOTLAYOUT_H
#define ROOTLAYOUT_H

#include "globalHeader.h"

#include "rootdata.h"
#include "population.h"
#include "projections.h"

using namespace std;

class rootLayout : public QVBoxLayout
{
    Q_OBJECT
public:
    explicit rootLayout(rootData * , QWidget *parent = 0);
    ~rootLayout();
    void clearOld();
    void addParamSelection();
    QComboBox* addDropBox(QVBoxLayout * ,QString, QString);
    void addDivider();

private:
    // draw functions for systemObject types
    void popSelected(QSharedPointer<population> &, rootData*);
    void projSelected(QSharedPointer<projection> &, rootData*);
    void inSelected(QSharedPointer<genericInput>, rootData*);

    // draw the params
    void drawParamsLayout(rootData * data);
    void drawSingleParam(QFormLayout * varLayout, ParameterData * currPar, rootData * data, bool connectionBool, QString type, QSharedPointer<NineMLData>  type9ml, connection * conn);

    // are these needed anymore?
    void recursiveDeleteLater(QLayout * parentLayout);
    QVector <QWidget *> forDeleting;
    QSharedPointer<systemObject> lastObject;

    // initialisation
    void initModelHeader(rootData *);
    void initPopulationHeader(rootData *);
    void initProjectionHeader(rootData *);
    void initInputHeader(rootData *);
    void initTabBox(rootData *);
    void initTabBoxPopulation(rootData *);
    void initTabBoxProjection(rootData *);
    void initFinish(rootData *);

    // tab widget and tabs
    QTabWidget * tabs;
    QFrame * tab1;
    QFrame * tab2;
    QFrame * tab3;

    // type selection boxes
    QComboBox * neuronComboBox;
    QComboBox * layoutComboBox;
    QComboBox * weightUpdateComboBox;
    QComboBox * postSynapseComboBox;
    QComboBox * connectionComboBox;

    // port selection for input
    QComboBox * inputPortSelection;
    QComboBox * inputConnectionComboBox;

    // Projection style radio buttons
    QRadioButton * exc;
    QRadioButton * inh;
    // checkbutton for "show label" for projection
    QCheckBox * showLabel;

    QLabel * inputSrcName; // we need to set the tooltip, which we cannot do through slots

signals:
    void updateParams(QString, int);
    void renameSignal(rootLayout*);
    void setCaption(QString);
    void deleteHeader();
    void deleteProperties();
    void showModel();
    void showPopulation();
    void showSpikeSource();
    void showProjection();
    void showInput();
    void hideHeader();

    // configure:

    // model
    void setModelName(QString);

    // population
    void setPopulationName(QString);
    void setPopulationTitle(QString);
    void setPopulationSize(int);
    void setNeuronType(int);
    void setLayoutType(int);

    // projection
    void setProjectionName(QString);
    void setProjectionSynapseName(QString);
    void setProjectionSynapseMinusOn();
    void setWeightUpdateType(int);
    void setPostSynapseType(int);
    void setConnectionType(int);

    // input
    void setInputName(QString);

    // copy / paste
    void showTab0CopyPaste();
    void showTab1CopyPaste();
    void allowPaste(bool);

    void testLabels(QString);


public slots:
    void updatePanel(rootData* data);
    void modelNameChanged();

    // update lists:
    void updateLayoutList(rootData *);
    void updateComponentLists(rootData *);


};

#endif // ROOTLAYOUT_H
