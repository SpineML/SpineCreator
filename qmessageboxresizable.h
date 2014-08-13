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
**          Authors: Alex Cope, Seb James                                 **
**  Website/Contact: http://bimpa.group.shef.ac.uk/                       **
****************************************************************************/

#ifndef _QMESSAGEBOXRESIZABLE_H_
#define _QMESSAGEBOXRESIZABLE_H_ 1

#include <QMessageBox>

/*!
 * This is a resizable version of the Qt message box. Necessary for
 * showing brahms error messages so that you can actually read them!
 *
 * Copied from the end of this thread:
 *
 * http://www.qtcentre.org/threads/24888-Resizing-a-QMessageBox
 */
class QMessageBoxResizable: public QMessageBox
{
public:
    QMessageBoxResizable() {
        setMouseTracking(true);
        setSizeGripEnabled(true);
    }
private:
    virtual bool event(QEvent *e) {
        bool res = QMessageBox::event(e);
        switch (e->type()) {
        case QEvent::MouseMove:
        case QEvent::MouseButtonPress:
            setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
            if (QWidget *textEdit = findChild<QTextEdit *>()) {
                textEdit->setMaximumHeight(QWIDGETSIZE_MAX);
            }
            break;
        default:
            break;
        }
        return res;
    }
};

#endif // _QMESSAGEBOXRESIZABLE_H_
