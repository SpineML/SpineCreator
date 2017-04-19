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

#ifndef GLOBALHEADER_H
#define GLOBALHEADER_H

// define a macro to test dynamic casts return non-NULL
#define CHECK_CAST(A) if (A==NULL) {qDebug() << "Bad cast in " << __FILE__ << ", " << __LINE__; exit(0);}

// prototype all classes;
class viewELLayoutEditHandler;
class viewVZLayoutEditHandler;
class connection;
class csv_connection;
class csv_connectionModel;
class glConnectionWidget;
class GLWidget;
class MainWindow;
class population;
class projection;
class nl_rootdata;
class nl_rootlayout;
class Component;
class ComponentInstance;
class NineMLLayout;
class systemObject;
class valueListDialog;
class BRAHMS_dialog;
class genericInput;
class experiment;
class RootComponentItem;
class synapse;
class systemmodel;
class ParameterInstance;
class fixedProb_connection;
class kernel_connection;
class pythonscript_connection;
class versionControl;
class projectObject;

// C++ Library
#include <vector>
#include <iostream>
#include <sstream>

// QT
#include <QMainWindow>
#include <QAbstractTableModel>
#include <QString>
#include <QtXml>
#include <QMessageBox>
#include <QtOpenGL>
#include <QObject>
#include <QWidget>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QFile>
#include <QColorDialog>
#include <string>
#include "qtimer.h"
#include <QFileDialog>
#include <QtGui>
#include <QHostInfo>

#ifdef _MSC_VER
#include <limits>
#define INFINITY std::numeric_limits<float>::infinity()
#define _USE_MATH_DEFINES
#include "math.h"
#define asinh(x) log(x + sqrt(x*x + 1.0))
#define acosh(x) log(x + sqrt(x*x - 1.0))
#define atanh(x) (log(1.0+x) - log(1.0-x))/2.0
#endif

// Debugging defines
#define DBG() qDebug() << __FUNCTION__ << ": "
#define DBGBRK() qDebug() << "---";

using namespace std;

class versionNumber
{
public:
    versionNumber();
    versionNumber& operator=(const versionNumber& data);
    friend bool operator==(versionNumber& v1, versionNumber& v2);
    friend bool operator!=(versionNumber& v1, versionNumber& v2);
    friend bool operator>=(versionNumber& v1, versionNumber& v2);
    friend bool operator<=(versionNumber& v1, versionNumber& v2);
    friend bool operator>(versionNumber& v1, versionNumber& v2);
    friend bool operator<(versionNumber& v1, versionNumber& v2);
    void setVersion (int maj, int min, int rev, QString owner) {major = maj; minor = min; revision = rev; last_owner = owner;}
    int major;
    int minor;
    int revision;
    QString last_owner;
    QString toString();
    QString toFileString();
    void fromFileString(QString);
};

struct loc {
    float x;
    float y;
    float z;
};

struct conn {
    int src;
    int dst;
    float metric;
};

// Used to store the cursor position in the network view
struct cursorType {
    GLfloat x;
    GLfloat y;
};

enum connectionType {
    AlltoAll,
    OnetoOne,
    FixedProb,
    CSV,
    Python,
    CSA,
    none
};

typedef enum {
    FixedValue,
    Statistical,
    ExplicitList,
    Undefined
} ParameterType;

enum drawStyle {
    standardDrawStyle,
    microcircuitDrawStyle,
    layersDrawStyle,
    spikeSourceDrawStyle,
    standardDrawStyleExcitatory,
    saveNetworkImageDrawStyle
};

struct trans {
    float GLscale;
    float viewX;
    float viewY;
    float width;
    float height;
};

const QFont addFont("Helvetica [Cronyx]", 12);

inline double round( double d )
{
    return floor( d + 0.5 );
}

#endif // GLOBALHEADER_H
