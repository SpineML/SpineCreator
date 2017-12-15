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

#include <QApplication>
#include "mainwindow.h"

// A global for a fixed qhash, which should work between machines. See:
// http://stackoverflow.com/questions/27378143/qt-5-produce-random-attribute-order-in-xml
extern Q_CORE_EXPORT QBasicAtomicInt qt_qhash_seed;

int main(int argc, char *argv[])
{
    // stop qt 5 salting the hash table and giving undeterministic xml attributes. Grrr...
#if QT_VERSION >= QT_VERSION_CHECK(5, 9, 0)
    qSetGlobalQHashSeed(12345);
#else
    qt_qhash_seed.store(12345);
#endif


    QApplication a(argc, argv);
    a.setAttribute(Qt::AA_DontCreateNativeWidgetSiblings, true);
    MainWindow w;
    w.show();
    // Some features that we switch on if possible:
#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling, true);
#endif
#if QT_VERSION >= QT_VERSION_CHECK(5, 2, 0)
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps,true);
#endif

    return a.exec();
}
