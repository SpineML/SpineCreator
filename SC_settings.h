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

#ifndef EDITSIMULATORS_H
#define EDITSIMULATORS_H

#include <QDialog>
#include "globalHeader.h"

namespace Ui {
    class settings_window;
}

class PythonSyntaxHighlighter;

/*!
 * \brief The settings_window class allows simulators and GUI settings to be updated, and python
 * connections to be defined
 */
class settings_window : public QDialog
{
    Q_OBJECT

public:
    explicit settings_window(QWidget *parent = 0);
    ~settings_window();

private:
    Ui::settings_window *ui;

    /*!
     * \brief path The absolute path to the current convert script.
     */
    QString path;

    /*!
     * \brief working_dir The working directory for the current convert script.
     */
    QString working_dir;

    QStringList keys;
    QStringList values;
    bool edited;
    void redrawEnvVars();
    void recursiveDeleteLater(QLayout *);
    void recursiveDeleteLaterloop(QLayout *);

    PythonSyntaxHighlighter * pySyn;

public slots:

    void addEnvVar();
    void delEnvVar();
    void changeEnvVar();
    void selectSimulator(QString);
    void getScript();
    void getWorkingDir();
    void getNewSimName();
    void applyChanges();
    void cancelChanges();
    void changeScript();
    void changePythonPath (void);
    void changePythonHome (void);
    void changedEnvVar(QString);
    void saveAsBinaryToggled(bool);
    void setGLDetailLevel(int);
    void setDevMode(bool);
    void close();
    void scriptSelectionChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void addScript();
    void removeScript();
    void renameScript();
    //void testFunc();
};

class SyntaxHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    SyntaxHighlighter(QPlainTextEdit * parent);

protected:
    void highlightBlock(const QString &text);

    struct HighlightingRule
    {
        QRegExp pattern;
        QTextCharFormat format;
    };

    QVector<HighlightingRule> highlightingRules;

    QTextCharFormat commentFormat;
    QTextCharFormat quotationFormat;
    QTextCharFormat functionFormat;
    QTextCharFormat numericFormat;
};

class PythonSyntaxHighlighter : public SyntaxHighlighter
{
    Q_OBJECT

public:
    PythonSyntaxHighlighter(QPlainTextEdit *parent);

    static QStringList keywordsList(){return d_keywords;}

protected:
    void highlightBlock(const QString &text);

private:
    QVector<HighlightingRule> pythonHighlightingRules;

    QTextCharFormat keywordFormat;
    QTextCharFormat classFormat;

    static const QStringList d_keywords;
};

struct ParenthesisInfo
{
    char character;
    int position;
};

class TextBlockData : public QTextBlockUserData
{
public:
    TextBlockData(){}

    QVector<ParenthesisInfo *> parentheses(){return m_parentheses;}
    void insert(ParenthesisInfo *info)
    {
        int i = 0;
        while (i < m_parentheses.size() &&
               info->position > m_parentheses.at(i)->position) {
            ++i;
        }
        m_parentheses.insert(i, info);
    }

private:
    QVector<ParenthesisInfo *> m_parentheses;
};

#endif // EDITSIMULATORS_H
