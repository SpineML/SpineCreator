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

#ifndef VIEWVISEXPTPANELHANDLER_H
#define VIEWVISEXPTPANELHANDLER_H

#include "globalHeader.h"
#include <QTemporaryDir>

struct viewELstruct;


class viewELExptPanelHandler : public QObject
{
    Q_OBJECT
public:
    explicit viewELExptPanelHandler(QObject *parent = 0);
    explicit viewELExptPanelHandler(viewELstruct * viewEL, rootData * data, QObject *parent = 0);
    rootData * data;
    //void redraw();

private:
    viewELstruct * viewEL;
    void redrawExpt();
    void redrawPanel();

    /*!
     * A temporary directory into which the model is copied if the
     * simulation is in an unsaved state.
     */
    QTemporaryDir tdir;

    QVBoxLayout * exptSetup;
    QVBoxLayout * exptInputs;
    QVBoxLayout * exptOutputs;
    QVBoxLayout * exptChanges;

    QToolButton * runButton;
    QString simulatorStdOutText;
    QString simulatorStdErrText;

    QStringList errorStrings;
    QStringList errorMessages;

    QVector <QWidget * > forDeleting;

    void recursiveDeleteLoop(QLayout * parentLayout);
    void recursiveDelete(QLayout * parentLayout);
    void recursiveDeleteExpt(QLayout * parentLayout);

    void reorderParams (QVector<float> &params);

    /*!
     * \brief Redraw the simulator parameters - this is called if there is no current systemObject selected
     * \param currentExperiment
     */
    void redrawSimulatorParams(experiment *currentExperiment);

    QPointF cursor;

    QSharedPointer<systemObject> currSystemObject;

    GLWidget * gl;


signals:
    void enableRun(bool);
    void simulationDone();

public slots:
    void addExperiment();
    void delExperiment();
    void moveExperiment();
    void editExperiment();
    void doneEditExperiment();
    void cancelEditExperiment();
    void changeSelection();
    void changedEngine(QString);
    void changedDt();
    void changedDuration();
    void changedSolver(int);
    void changedSolverOrder(int);
    void addInput();
    void setInputName();
    void setInputComponent();
    void setInputPort(int);
    void setInputType(int);
    void setInputRateDistributionType(int);
    void acceptInput();
    void editInput();
    void setInputTypeData(double);
    void setInputExternalData();
    void setInputParams(int, int);
    void fillInputParams();
    void changeInputIndex(int);
    void setInputAddTVRow();
    void setInputDelTVRow();
    void delInput();
    void addOutput();
    void setOutputName();
    void setOutputComponent();
    void setOutputPort(int);
    void setOutputType();
    void setOutputIndices();
    void setOutputStartT(double);
    void setOutputEndT(double);
    void acceptOutput();
    void editOutput();
    void delOutput();
    void toggleExternalOutput(bool);
    void setOutputExternalData();
    void addLesion();
    void setLesionProjection();
    void acceptLesion();
    void editLesion();
    void delLesion();
    void setChangeParComponent();
    void setChangeProp(QString name);
    void addChangedProp();
    void acceptChangedProp();
    void editChangedProp();
    void delChangedProp();
    void redraw();
    void redraw(int);
    void redraw(double);

    void run();
    void simulatorFinished(int, QProcess::ExitStatus);
    void simulatorStandardOutput();
    void simulatorStandardError();

    /*!
     * \brief Called when the mouse moves on the model view
     * \param xGL
     * \param yGL
     */
    void mouseMove(float xGL, float yGL);

    /*!
     * \brief Redraw the model
     * \param painter
     * \param GLscale
     * \param viewX
     * \param viewY
     * \param width
     * \param height
     * \param style
     */
    void reDrawModel(QPainter* painter,float GLscale, float viewX, float viewY, int width, int height, drawStyle style);

    /*!
     * \brief selectByMouseDown
     * \param xGL
     * \param yGL
     * \param GLScale
     */
    void selectByMouseDown(float xGL, float yGL, float GLScale);

    /*!
     * \brief batch_clicked
     * Launch the batch window for running more complex experiments
     */
    void batch_clicked();
};

#endif // VIEWVISEXPTPANELHANDLER_H
