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

#include "SC_network_layer_rootdata.h"
#include "NL_population.h"
#include "NL_projection_and_synapse.h"

using namespace std;

class nl_rootlayout : public QVBoxLayout
{
    Q_OBJECT
public:
    explicit nl_rootlayout(nl_rootdata * , QWidget *parent = 0);
    ~nl_rootlayout();
    void clearOld();
    void addParamSelection();
    QComboBox* addDropBox(QVBoxLayout * ,QString, QString);
    void addDivider();

private:
    // draw functions for systemObject types
    void popSelected(QSharedPointer<population> &, nl_rootdata*);
    void projSelected(QSharedPointer<projection> &, nl_rootdata*);
    void inSelected(QSharedPointer<genericInput>, nl_rootdata*);

    // draw the params
    void drawParamsLayout(nl_rootdata * data);
    void drawSingleParam(QFormLayout * varLayout, ParameterInstance * currPar, nl_rootdata * data, bool connectionBool, QString type, QSharedPointer<ComponentRootInstance>  type9ml, connection * conn);

    // are these needed anymore?
    void recursiveDeleteLater(QLayout * parentLayout);
    QVector <QWidget *> forDeleting;
    QSharedPointer<systemObject> lastObject;

    // initialisation
    void initModelHeader(nl_rootdata *);
    void initPopulationHeader(nl_rootdata *);
    void initProjectionHeader(nl_rootdata *);
    void initInputHeader(nl_rootdata *);
    void initTabBox(nl_rootdata *);
    void initTabBoxPopulation(nl_rootdata *);
    void initTabBoxProjection(nl_rootdata *);
    void initFinish(nl_rootdata *);

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
    void renameSignal(nl_rootlayout*);
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
    void updatePanel(nl_rootdata* data);
    void modelNameChanged();

    // update lists:
    void updateLayoutList(nl_rootdata *);
    void updateComponentLists(nl_rootdata *);


};

#endif // ROOTLAYOUT_H
