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

#ifndef VIEW2LAYOUTEDITHANDLER_H
#define VIEW2LAYOUTEDITHANDLER_H

#include "globalHeader.h"

struct viewNLstruct;
struct viewVZstruct;

#include "nineml_classes.h"
#include "connectionmodel.h"
#include "glconnectionwidget.h"
#include "rootdata.h"
#include "rootlayout.h"
#include "layoutaliaseditdialog.h"
#include "layouteditpreviewdialog.h"
#include "mainwindow.h"


class viewVZLayoutEditHandler : public QObject
{
    Q_OBJECT
public:
    explicit viewVZLayoutEditHandler(rootData *data, viewNLstruct *viewNL, viewVZstruct *viewVZ, QObject *parent = 0);
    viewNLstruct * viewNL;
    viewVZstruct * viewVZ;
    rootData * data;
    void redrawProperties();
    void drawDeletables();
    //QHBoxLayout * drawSA(OnConditionSpace * srcOnCond, StateAssignment * srcSA);
    QHBoxLayout * drawTransform(QVariant transContainer, Transform *srcTrans);
    //QHBoxLayout * drawOnCondition(RegimeSpace * srcRegime, OnConditionSpace * srcOnCond);
    QHBoxLayout * drawRegime(RegimeSpace * srcRegime);
    void clearAll();

    // TreeWidget state saving/loading
    void saveTreeState(void);
    void restoreTreeState(void);

private:
    QComboBox * addDropBox(QVBoxLayout *layout, QString name, QString type);
    void viewVZeditDisplayErrors(QStringList errs);
    void updateRegimeNameRefs(QLayout * lay);
    void updateStateVariableRefs(QLayout * lay);
    QFrame * getDivider();
    void initGlobal();
    void initPopulation();
    void initConnection();
    void updateConnectionList();

    void setAllSelectState(bool);

    // widgets
    QSlider * timeSlider;
    QPushButton * playButton;
    QSpinBox * playTimeStep;

    // timer
    QTimer playBack;
    int playBackTimeStep;

    // population
    QComboBox * layoutComboBox;
    QComboBox * connectionComboBox;
    // to make it better store the spinboxes so we can disconnect them
    QSpinBox * sizeSpin;
    QSpinBox * xSpin;
    QSpinBox * ySpin;
    QSpinBox * zSpin;

    // kernel
    QComboBox * kernelComboBox;


signals:
    void hideAll();
    void hideTree();
    void showAll();
    void showConnection();
    void showPopulation();

    // set up population
    void setPopulationSize(int);
    void setPopulationX(int);
    void setPopulationY(int);
    void setPopulationZ(int);
    void setMinDistance(double);
    void setSeed(int);

    void deleteProperties();

public slots:
    void redrawFromObject(QString name);
    void redrawHeaders();
    void redrawLayoutEdit();

    void deleteTransform();
    void addTransform();
    void transformOrderDown();
    void transformOrderUp();
    void updateParameters();
    void updateStateVariables();
    void editAliases();

    void saveEditedLayout();
    void discardEditedLayout();
    void previewEditedLayout();

    void selectionChanged(QItemSelection, QItemSelection);
    void updateLayoutList(rootData * data);

    void changeTypeOfTransition(QString type);
    void changeTransitionMaths();

    void selectAll();
    void deselectAll();

    void togglePlay();
    void playBackTimeout();
    void setPlayTimeStep(int tstep);

    void disableButton();

};

#endif // VIEW2LAYOUTEDITHANDLER_H
