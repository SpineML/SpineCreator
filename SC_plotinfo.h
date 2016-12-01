/***************************************************************************
**                                                                        **
**  This file is part of SpineCreator, an easy to use GUI for             **
**  describing spiking neural network models.                             **
**  Copyright (C) 2013-2016 Alex Cope, Paul Richmond, Seb James           **
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
**           Author: Seb James                                            **
**  Website/Contact: http://spineml.github.io/                            **
****************************************************************************/

#ifndef PLOTINFO_H
#define PLOTINFO_H

#include "qcustomplot.h"
#include "SC_logged_data.h"

/*!
 * \brief Holds one of the single graphs stored in PlotInfo.
 */
class SingleGraph
{
public: // methods
    explicit SingleGraph();
    ~SingleGraph();
    // Copy constructor
    SingleGraph(const SingleGraph& rhs);

    void setData (QCPDataMap* d);

public: // attributes
    QString type;
    QString source;
    int index;
    // Same data container as used in a QCustomPlot graph
    QCPDataMap* data;
};

/*!
 * \brief The PlotInfo class holds information from a QCustomPlot in
 * non-Qt style object. It allows us to store away the pertinent
 * information about old QCustomPlots and then recreate a new set of
 * QCustomPlots,
 */
class PlotInfo
{
public: // methods
    explicit PlotInfo (void);
    explicit PlotInfo (QCustomPlot* p);
    PlotInfo (const PlotInfo& rhs);
    ~PlotInfo();

    QString getTitle (void) const;

    /*!
     * Configure this PlotInfo from the passed in QCustomPlot.
     */
    void setupFrom (QCustomPlot* p);

public: // attributes
    QVector<SingleGraph> graphs;
    QString xlabel;
    QString ylabel;
private: // attributes
    QString title;
};

#endif // PLOTINFO_H
