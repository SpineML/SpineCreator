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
class NetViewWidget;
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

/*!
 * Colour definitions used in projection::draw and elsewhere. These
 * were chosen using Hue/Saturation/Lightness/Alpha with Saturation
 * 255, Alpha 255, lightness 106 and the Hue varied.
 */
//@{
#define QCOL_BASICBLUE  QColor(0x00,0x00,0xff,0xff)

#define QCOL_MAGENTA1   QColor(0xd3,0x00,0xbf,0xff)
#define QCOL_PURPLE1    QColor(0xa6,0x00,0xd3,0xff)
#define QCOL_PURPLE2    QColor(0x49,0x00,0xd3,0xff)
#define QCOL_BLUE1      QColor(0x00,0x09,0xd3,0xff)
#define QCOL_BLUE2      QColor(0x00,0x81,0xd3,0xff)
#define QCOL_CYAN1      QColor(0x00,0xc8,0xd3,0xff)

#define QCOL_GREEN1     QColor(0x00,0xd3,0x50,0xff)
#define QCOL_GREEN2     QColor(0x07,0xd3,0x00,0xff)
#define QCOL_GREEN3     QColor(0x7a,0xd3,0x00,0xff)
#define QCOL_GREEN4     QColor(0x00,0xff,0x00,0xff)
#define QCOL_ORANGE1    QColor(0xd3,0x83,0x00,0xff)
#define QCOL_RED1       QColor(0xd3,0x26,0x00,0xff)
#define QCOL_RED2       QColor(0xd3,0x00,0x00,0xff)
#define QCOL_RED3       QColor(0xff,0x00,0x00,0x64)

#define QCOL_GREY1      QColor(0xc8,0xc8,0xc8,0xff)
#define QCOL_GREY2      QColor(0x3e,0x3e,0x3e,0xff)
#define QCOL_BLACK      QColor(0x00,0x00,0x00,0xff)

#define QCOL_BLACK1     QColor(0x00,0x00,0x00,0x1e)
#define QCOL_BLACK2     QColor(0x00,0x00,0x00,0x32)

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
    loc() : x(0.0f), y(0.0f), z(0.0f) {}
    loc (float _x, float _y, float _z)
        : x(_x)
        , y(_y)
        , z(_z) {}
    float x;
    float y;
    float z;
    loc operator+ (const loc& l) const
    {
        loc lrtn;
        lrtn.x = this->x + l.x;
        lrtn.y = this->y + l.y;
        lrtn.z = this->z + l.z;
        return lrtn;
    }
    void operator+= (const loc& l)
    {
        this->x += l.x;
        this->y += l.y;
        this->z += l.z;
    }
    template<typename T>
    void operator/= (const T& f)
    {
        this->x /= static_cast<float>(f);
        this->y /= static_cast<float>(f);
        this->z /= static_cast<float>(f);
    }
};

struct conn {
    int src;
    int dst;
    float metric;  // Used as delay. This data structure reflects the
                   // connection list file, which contains only src,
                   // dst and delay. The *weight* is stored in a
                   // separate file, assocated with the weight update
                   // for the connection. If it's a csv list of
                   // connections with weights, then the weights will
                   // be found in an explicitDataBinaryFile. HOWEVER,
                   // at the time of writing, SpineCreator code never
                   // reads those weights in, because it never needed
                   // to. I would like these weights so that the
                   // connection patterns can be colour coded using
                   // the weights, so first I need new code to read
                   // the weights, possibly storing them in this
                   // structure in a float weight attribute.
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
    float scale;
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
