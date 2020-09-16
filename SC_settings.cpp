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

#include "SC_settings.h"
#include "ui_settings_window.h"
#include <QSettings>
#include <QInputDialog>

settings_window::settings_window(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::settings_window)
{
    edited = false;

    ui->setupUi(this);

    ui->buttonBox->setFocusPolicy(Qt::NoFocus);

    // set name to 'options' on Linux
#ifdef Q_OS_LINUX
    this->setWindowTitle("Options");
#else
# ifndef Q_OS_OSX
    this->setWindowTitle("Settings");
# endif
#endif

    connect(ui->addEnv, SIGNAL(clicked()), this, SLOT(addEnvVar()));

    // load existing simulators:
    QSettings settings;

    settings.beginGroup("simulators");
    QStringList sims = settings.childGroups();
    for (int i = 0; i < sims.size(); ++i) {
        this->ui->comboBox->addItem(sims[i]);
        if (i == 0) selectSimulator(sims[0]);
    }
    settings.endGroup();

    QVBoxLayout * hbox = qobject_cast<QVBoxLayout *> (ui->scrollAreaWidgetContents->layout());
    CHECK_CAST(hbox);
    hbox->removeWidget(ui->addEnv);
    hbox->addWidget(ui->addEnv);

    // connect up comboBox
    connect(ui->comboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(selectSimulator(QString)));
    // connect up buttons
    connect(this->ui->findFile, SIGNAL(clicked()), this, SLOT(getScript()));
    connect(this->ui->findWorkingDir, SIGNAL(clicked()), this, SLOT(getWorkingDir()));
    connect(this->ui->addSim, SIGNAL(clicked()), this, SLOT(getNewSimName()));

    // accept
    connect(ui->buttonBox->button(QDialogButtonBox::Apply), SIGNAL(clicked()), this, SLOT(applyChanges()));

    // cancel
    connect(ui->buttonBox->button(QDialogButtonBox::Reset), SIGNAL(clicked()), this, SLOT(cancelChanges()));

    // close
    connect(ui->buttonBox_2->button(QDialogButtonBox::Close), SIGNAL(clicked()), this, SLOT(close()));

    // change path
    connect(ui->scriptLineEdit, SIGNAL(editingFinished()), this, SLOT(changeScript()));

    // change if we save large connections as binary data
    bool writeBinary = settings.value("fileOptions/saveBinaryConnections", "error").toBool();
    ui->save_as_binary->setChecked(writeBinary);
    connect(ui->save_as_binary, SIGNAL(toggled(bool)), this, SLOT(saveAsBinaryToggled(bool)));

    // change level of detail box
    int lod = settings.value("glOptions/detail", 5).toInt();
    ui->openGLDetailSpinBox->setValue(lod);
    connect(ui->openGLDetailSpinBox, SIGNAL(valueChanged(int)), this, SLOT(setGLDetailLevel(int)));

    // change dev stuff box
    bool devMode = settings.value("dev_mode_on", "false").toBool();
    ui->dev_mode_check->setChecked(devMode);

    connect(ui->dev_mode_check, SIGNAL(toggled(bool)), this, SLOT(setDevMode(bool)));

    // Fill Python program path and PYTHONHOME text boxes
    this->ui->pythonpath->setText(settings.value("python/programname", "").toString());
    connect(this->ui->pythonpath,
            SIGNAL(editingFinished()), this, SLOT(changePythonPath()));

    this->ui->pythonhome->setPlainText(settings.value("python/pythonhome", "").toString());
    connect(this->ui->pythonhome,
            SIGNAL(textChanged()), this, SLOT(changePythonHome()));

    // populate script list
    this->ui->scriptList->setSelectionMode(QAbstractItemView::SingleSelection);
    settings.beginGroup("pythonscripts");
    QStringList scripts = settings.childKeys();
    this->ui->scriptList->addItems(scripts);
    connect(this->ui->scriptList, SIGNAL(currentItemChanged(QListWidgetItem *,QListWidgetItem *)), this, SLOT(scriptSelectionChanged(QListWidgetItem *,QListWidgetItem *)));
    if (scripts.size() > 0) {
        // select first script
        this->ui->scriptList->setCurrentRow(0);
    }
    settings.endGroup();

    // setup text editor
    QFont font;
    font.setFamily("Courier");
    font.setFixedPitch(true);
    font.setPointSize(10);

    ui->scriptTextBox->setFont(font);
    ui->scriptTextBox->setTabStopWidth(20);

    pySyn = new PythonSyntaxHighlighter(ui->scriptTextBox);

    // connect buttons for script management
    connect(this->ui->addScript, SIGNAL(clicked()), this, SLOT(addScript()));
    connect(this->ui->removeScript, SIGNAL(clicked()), this, SLOT(removeScript()));
    connect(this->ui->renameScript, SIGNAL(clicked()), this, SLOT(renameScript()));

    // we are disabling renaming for now!
    this->ui->renameScript->setVisible(false);

    redrawEnvVars();
}

settings_window::~settings_window()
{
    // store curently selected python script
    QSettings settings;
    // move into the group of scripts
    settings.beginGroup("pythonscripts");
    // save currently selected script
    if (ui->scriptList->currentItem()) {
        settings.setValue(ui->scriptList->currentItem()->text(), this->ui->scriptTextBox->toPlainText());
    }
    settings.endGroup();

    delete ui;
}

void settings_window::scriptSelectionChanged(QListWidgetItem * current, QListWidgetItem * previous)
{

    QSettings settings;
    // enter the group of scripts (if valid)
    settings.beginGroup("pythonscripts");
    // store previous selected script
    if (previous) {
        //qDebug() << "storing! " << previous->text() << ": " << this->ui->scriptTextBox->toPlainText();
        settings.setValue(previous->text(), this->ui->scriptTextBox->toPlainText());
    }
    // load current selected script (if valid)
    this->ui->scriptTextBox->clear();
    if (current) {
        //qDebug() << "loading! " << current->text() << ": " << settings.value(current->text(),"error").toString();
        this->ui->scriptTextBox->setPlainText(settings.value(current->text(),"error").toString());
    }
    settings.endGroup();
}

void settings_window::addScript()
{

    QSettings settings;
    // move into the group of scripts
    settings.beginGroup("pythonscripts");
    // save previously selected script
    if (ui->scriptList->currentItem()) {
        settings.setValue(ui->scriptList->currentItem()->text(), this->ui->scriptTextBox->toPlainText());
    }

    bool ok;
    QString text = QInputDialog::getText(this, tr("QInputDialog::getText()"),
                                         tr("Script name"), QLineEdit::Normal,
                                         "New Script", &ok);
    // did the dialog return correctly and sensibly?
    if (ok && !text.isEmpty()) {
        // check for existing script with the same name
        QStringList scripts = settings.childKeys();
        if (scripts.contains(text)) {
            // name exists - we do not add a script
            QMessageBox::information(this, tr("Error"), "A script with that name exists", QMessageBox::Ok);
            // exit the scripts group
            settings.endGroup();
            return;
        }
        // clear scriptTextBox as we are now adding a new script
        this->ui->scriptTextBox->clear();
        // add the script to the QSetting registry
        settings.setValue(text, "");
        // refetch the list of scripts
        scripts = settings.childKeys();
        // disconnect the scriptList to avoid issues when rewriting
        disconnect(this->ui->scriptList, 0, 0, 0);
        // add the new list of scripts
        this->ui->scriptList->clear();
        this->ui->scriptList->addItems(scripts);
        // reconnect the scriptList
        connect(this->ui->scriptList, SIGNAL(currentItemChanged(QListWidgetItem *,QListWidgetItem *)), this, SLOT(scriptSelectionChanged(QListWidgetItem *,QListWidgetItem *)));
        // this should always be true as we just added a script, but doesn't hurt to check
        if (scripts.size() > 0) {
            // select added script
            this->ui->scriptList->setCurrentRow(scripts.indexOf(text));
        }

    }
    // exit the scripts group
    settings.endGroup();
}

void settings_window::removeScript()
{
    QSettings settings;
    // enter group of scripts
    settings.beginGroup("pythonscripts");
    // remove key corresponding to selected script
    settings.remove(this->ui->scriptList->currentItem()->text());
    // fetch the new list of scripts
    QStringList scripts = settings.childKeys();
    // disconnect the scriptList to avoid issues
    disconnect(this->ui->scriptList, 0, 0, 0);
    // clear scripts, then add the updated list
    this->ui->scriptList->clear();
    this->ui->scriptList->addItems(scripts);
    // reconnect the scriptList
    connect(this->ui->scriptList, SIGNAL(currentItemChanged(QListWidgetItem *,QListWidgetItem *)), this, SLOT(scriptSelectionChanged(QListWidgetItem *,QListWidgetItem *)));
    // select a new script if there is one (the last script)
    if (scripts.size() > 0) {
        // select added script
        this->ui->scriptList->setCurrentRow(scripts.size() - 1);
    }
    // exit the scripts group
    settings.endGroup();
}

void settings_window::renameScript() {
    bool ok;
    QString oldName = this->ui->scriptList->currentItem()->text();
    QString text = QInputDialog::getText(this, tr("QInputDialog::getText()"),
                                         tr("Script name"), QLineEdit::Normal,
                                         this->ui->scriptList->currentItem()->text(), &ok);
    if (ok && !text.isEmpty()) {
        if (text == oldName) {
            // nothing to do
            return;
        }
        QSettings settings;
        settings.beginGroup("pythonscripts");
        // check for existing
        QStringList scripts = settings.childKeys();
        if (scripts.contains(text)) {
            QMessageBox::information(this, tr("Error"), "A script with that name exists", QMessageBox::Ok);
            return;
        }
        settings.setValue(text, this->ui->scriptTextBox->toPlainText());
        scripts = settings.childKeys();
        disconnect(this->ui->scriptList, 0, 0, 0);
        this->ui->scriptList->clear();
        this->ui->scriptList->addItems(scripts);
        connect(this->ui->scriptList, SIGNAL(currentItemChanged(QListWidgetItem *,QListWidgetItem *)), this, SLOT(scriptSelectionChanged(QListWidgetItem *,QListWidgetItem *)));
        if (scripts.size() > 0) {
            // reselect script
            this->ui->scriptList->setCurrentRow(scripts.indexOf(text));
        }
        settings.endGroup();
    }
}

void settings_window::saveAsBinaryToggled(bool toggle)
{
    QSettings settings;
    settings.setValue("fileOptions/saveBinaryConnections", QString::number((float) toggle));
}

void settings_window::setGLDetailLevel(int value)
{
    QSettings settings;
    settings.setValue("glOptions/detail", value);
}

void settings_window::setDevMode(bool toggle)
{
    QSettings settings;
    settings.setValue("dev_mode_on", toggle);
}

void settings_window::addEnvVar()
{
    keys.push_back("newVariable");
    values.push_back("newValue");

    this->edited = true;
    redrawEnvVars();
}

void settings_window::delEnvVar()
{
    QWidget * temp = (QWidget *) (sender()->property("ptr").value<void *>());
    QLineEdit * envNameLineEdit = qobject_cast<QLineEdit *> (temp);
    CHECK_CAST(envNameLineEdit);

    // remove var
    for (int i = 0; i < keys.size(); ++i) {
        if (keys[i] == envNameLineEdit->text()) {
            keys.erase(keys.begin()+i);
            values.erase(values.begin()+i);
            break;
        }
    }

    this->edited = true;
    redrawEnvVars();
}

void settings_window::changeEnvVar()
{
    edited = true;

    QLineEdit * envNameLineEdit = qobject_cast<QLineEdit *> (sender());
    CHECK_CAST(envNameLineEdit);

    if (sender()->property("type").toString() == "key") {
        keys[sender()->property("index").toInt()] = envNameLineEdit->text();
    } else if (sender()->property("type").toString() == "value")  {
        values[sender()->property("index").toInt()] = envNameLineEdit->text();
    }
}

void settings_window::changedEnvVar(QString)
{
    edited = true;
    ui->buttonBox_2->button(QDialogButtonBox::Close)->setEnabled(false);
    ui->comboBox->setEnabled(false);
    ui->addSim->setEnabled(false);
}

void settings_window::recursiveDeleteLaterloop(QLayout * parentLayout)
{
    QLayoutItem * item;
    while ((item = parentLayout->takeAt(0))) {
        if (item->widget() != ui->addEnv) {
            item->widget()->deleteLater();
        }
        if (item->layout()) {
            recursiveDeleteLaterloop(item->layout());
        }
        delete item;
    }
    parentLayout->deleteLater();
}

void settings_window::recursiveDeleteLater(QLayout * parentLayout)
{
    QLayoutItem * item;
    while ((item = parentLayout->takeAt(0))) {
        if (item->widget() && item->widget() != ui->addEnv) {
            item->widget()->deleteLater();
        }
        if (item->layout()) {
            recursiveDeleteLaterloop(item->layout());
        }
        delete item;
    }
}

void settings_window::redrawEnvVars()
{

    //qDebug() << ui->scrollAreaWidgetContents->layout();
    QVBoxLayout * hbox = qobject_cast<QVBoxLayout *> (ui->scrollAreaWidgetContents->layout());
    CHECK_CAST(hbox);

    recursiveDeleteLater(hbox);

    hbox->addWidget(ui->addEnv);

    for (int i = 0; i < keys.size(); ++i) {

        QHBoxLayout * newEnv = new QHBoxLayout;

        QLineEdit * envNameLineEdit = new QLineEdit;
        envNameLineEdit->setMaximumWidth(220);
        envNameLineEdit->setProperty("type", "key");
        envNameLineEdit->setProperty("index", i);
        envNameLineEdit->setText(keys[i]);
        QLineEdit * envLineEdit = new QLineEdit;
        envLineEdit->setMaximumWidth(320);
        envLineEdit->setProperty("type", "value");
        envLineEdit->setText(values[i]);
        envLineEdit->setProperty("index", i);
        QPushButton * envDel = new QPushButton;
        envDel->setFlat(true);
        envDel->setFocusPolicy(Qt::NoFocus);
        envDel->setIcon(QIcon(":/icons/toolbar/delShad.png"));
        envDel->setProperty("ptr", qVariantFromValue((void *) envNameLineEdit));
        connect(envDel, SIGNAL(clicked()), this, SLOT(delEnvVar()));
        connect(envNameLineEdit, SIGNAL(textChanged(QString)), this, SLOT(changeEnvVar()));
        connect(envLineEdit, SIGNAL(textChanged(QString)), this, SLOT(changeEnvVar()));
        connect(envNameLineEdit, SIGNAL(textEdited(QString)), this, SLOT(changedEnvVar(QString)));
        connect(envLineEdit, SIGNAL(textEdited(QString)), this, SLOT(changedEnvVar(QString)));

        newEnv->addWidget(new QLabel("Name"));
        newEnv->addWidget(envNameLineEdit);
        newEnv->addWidget(new QLabel("Value"));
        newEnv->addWidget(envLineEdit);
        newEnv->addWidget(envDel);

        hbox->insertLayout(hbox->count()-1,newEnv);
    }

    hbox->addStretch();

    if (edited) {
        // disable this stuff
        ui->buttonBox_2->button(QDialogButtonBox::Close)->setEnabled(false);
        ui->comboBox->setEnabled(false);
        ui->addSim->setEnabled(false);
    } else {
        // enable this stuff
        ui->buttonBox_2->button(QDialogButtonBox::Close)->setEnabled(true);
        ui->comboBox->setEnabled(true);
        ui->addSim->setEnabled(true);
    }
}

void settings_window::getScript()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Choose the simulator script"), qgetenv("HOME"), tr("All files (*)"));

    if (!fileName.isEmpty()) {
        this->ui->scriptLineEdit->setText(fileName);
        this->edited = true;
    }

    redrawEnvVars();
}

void settings_window::getWorkingDir()
{
    QString wk_dir = QFileDialog::getExistingDirectory(this,
                                                       tr("Choose the simulator's working directory"),
                                                       qgetenv("HOME"));

    if (!wk_dir.isEmpty()) {
        this->ui->scriptWDLineEdit->setText(wk_dir);
        this->edited = true;
    }

    redrawEnvVars();
}

void settings_window::getNewSimName()
{
    bool ok;
    QString simName = QInputDialog::getText(this,"Enter new simulator name", tr("Simulator name:"), QLineEdit::Normal, "", &ok);
    if (ok && !simName.isEmpty()) {
        this->ui->comboBox->addItem(simName);
        this->ui->comboBox->setCurrentIndex(this->ui->comboBox->count()-1);
        this->ui->useBinary->setChecked(false);
        edited = true;
    }

    redrawEnvVars();
}

void settings_window::selectSimulator(QString simName)
{
    // clear old sim
    this->keys.clear();
    this->values.clear();

    QSettings settings;

    // load path
    settings.beginGroup("simulators/" + simName);
    this->path = settings.value("path").toString();
    this->working_dir = settings.value("working_dir").toString();
    ui->useBinary->setChecked(settings.value("binary").toBool());
    settings.endGroup();

    settings.beginGroup("simulators/" + simName + "/envVar");

    // load values
    QStringList keysTemp = settings.childKeys();

    for (int i = 0; i < keysTemp.size(); ++i) {
        keys.push_back(keysTemp[i]);
        values.push_back(settings.value(keys[i]).toString());
    }

    settings.endGroup();

    ui->scriptLineEdit->setText(this->path);
    ui->scriptWDLineEdit->setText(this->working_dir);

    redrawEnvVars();
}

void settings_window::applyChanges()
{
    QSettings settings;

    // add a group for this simulator
    settings.beginGroup("simulators/" + ui->comboBox->currentText());

    // Check newSim is an executable script. If not, issue warning.
    QFile scriptfile (ui->scriptLineEdit->text());
    if (!scriptfile.exists()) {
        // warning - file doesn't exist.
        QMessageBox::warning(this,
                             QString("Script not found"),
                             QString("The convert script wasn't found; simulations will not be able to execute."));
    }

    settings.setValue("path", ui->scriptLineEdit->text());
    settings.setValue("working_dir", ui->scriptWDLineEdit->text());
    settings.setValue("binary", ui->useBinary->isChecked());
    settings.endGroup();

    settings.beginGroup("simulators/" + ui->comboBox->currentText() + "/envVar");

    // remove existing envVars
    QStringList currKeys = settings.childKeys();
    for (int i = 0; i < currKeys.size(); ++i) {
        settings.remove(currKeys[i]);
    }
    for (int i = 0; i < this->keys.size(); ++i) {
        settings.setValue(keys[i], values[i]);
    }
    settings.endGroup();

    edited = false;

    ui->buttonBox_2->button(QDialogButtonBox::Close)->setEnabled(true);
    ui->comboBox->setEnabled(true);
    ui->addSim->setEnabled(true);
}

void settings_window::cancelChanges()
{
    selectSimulator(ui->comboBox->currentText());
    edited = false;
    redrawEnvVars();
}

void settings_window::changeScript()
{

    QLineEdit * edit = qobject_cast<QLineEdit *> (sender());
    CHECK_CAST(edit)

    path = edit->text();
    edited = true;
    redrawEnvVars();
}

void settings_window::close()
{
    this->applyChanges();
    delete this;
}

void settings_window::changePythonPath (void)
{
    QLineEdit * edit = qobject_cast<QLineEdit *> (sender());
    CHECK_CAST(edit);
    QSettings settings;
    settings.setValue("python/programname", edit->text());
    this->ui->lbl_restartrequest->setText("Python path changed. Please re-start SpineCreator to use the new Python environment.");
}

void settings_window::changePythonHome (void)
{
    QTextEdit * edit = qobject_cast<QTextEdit *> (sender());
    CHECK_CAST(edit);
    QSettings settings;
    QString phtext = edit->toPlainText();
    // For some reason, the text from edit->toPlainText() gets a space at the start, even when
    // pasting in text with no leading space. So, chomp off any leading/trailing whitespace with
    // QString::trimmed()
    settings.setValue("python/pythonhome", phtext.trimmed());
    this->ui->lbl_restartrequest->setText("PYTHONHOME changed. Please re-start SpineCreator to use the new Python environment.");
}

// SYNTAX HIGHLIGHTING

const QStringList PythonSyntaxHighlighter::d_keywords = QStringList() << "and" << "assert" << "break"
                    << "class" << "continue"  << "def" << "del"
                    << "elif" << "else" << "except" << "exec"
                    << "finally" << "for" << "from" << "global"
                    << "if" << "import" << "in" << "is"
                    << "lambda" << "not" << "or" << "pass"
                    << "print" << "raise" << "return" << "try" << "while";

PythonSyntaxHighlighter::PythonSyntaxHighlighter(QPlainTextEdit *parent)
    : SyntaxHighlighter(parent)
{
    HighlightingRule rule;

    keywordFormat.setForeground(Qt::darkRed);
    keywordFormat.setFontWeight(QFont::Bold);

    foreach (QString pattern, d_keywords) {
        rule.pattern = QRegExp("\\b" + pattern + "\\b");
        rule.format = keywordFormat;
        pythonHighlightingRules.append(rule);
    }

    classFormat.setFontWeight(QFont::Bold);
    classFormat.setForeground(Qt::darkBlue);
    rule.pattern = QRegExp("\\bQ[A-Za-z]+\\b");
    rule.format = classFormat;
    pythonHighlightingRules.append(rule);
}

void PythonSyntaxHighlighter::highlightBlock(const QString &text)
{
    QString s = text;
    QRegExp comment = QRegExp("\"{3}");
    s.replace(comment, "   ");

    foreach (HighlightingRule rule, pythonHighlightingRules) {
        QRegExp expression(rule.pattern);
        int index = s.indexOf(expression);
        while (index >= 0) {
            int length = expression.matchedLength();
            setFormat(index, length, rule.format);
            index = s.indexOf(expression, index + length);
        }
    }

    SyntaxHighlighter::highlightBlock(text);//process common rules and parentheses matching

    int startIndex = text.indexOf(comment);
    int prevState = previousBlockState ();
    int comments = text.count(comment);

    if (comments > 1){
            int aux = 1;
            if (prevState == 1)
                setFormat(0, startIndex + 3, commentFormat);

            while (aux < comments) {
                int endIndex = text.indexOf(comment, startIndex + 3);
                aux++;
                if ((!prevState && (aux %2 == 0)) || (prevState == 1 && (aux %2 != 0)))
                    setFormat(startIndex, endIndex - startIndex + 3, commentFormat);

                startIndex = endIndex;
            }

        int state = 0;
        if ((!prevState && (comments % 2 != 0)) || (prevState && (comments % 2 == 0))){
            state = 1;
            setFormat(startIndex, text.length() - startIndex, commentFormat);
        }
        setCurrentBlockState(state);
    } else if (comments == 1){
        if (prevState == 1){
            setCurrentBlockState(0);// end of comment block
            setFormat(0, startIndex + 3, commentFormat);
        } else {
            setCurrentBlockState(1);// start of comment block
            setFormat(startIndex, text.length() - startIndex, commentFormat);
        }
    } else {
        if (prevState == 1){
            setCurrentBlockState(1);// inside comment block
            setFormat(0, text.length(), commentFormat);
        } else
            setCurrentBlockState(0);// outside comment block
    }
}

SyntaxHighlighter::SyntaxHighlighter(QPlainTextEdit * parent) : QSyntaxHighlighter(parent->document())
{
    HighlightingRule rule;

    functionFormat.setFontItalic(true);
    functionFormat.setForeground(Qt::darkBlue);
    rule.pattern = QRegExp("\\b[A-Za-z0-9_]+(?=\\()");
    rule.format = functionFormat;
    highlightingRules.append(rule);

    numericFormat.setForeground(Qt::darkMagenta);
    rule.pattern = QRegExp("\\b\\d+[eE.,]*\\d*\\b");
    rule.format = numericFormat;
    highlightingRules.append(rule);

    quotationFormat.setForeground(Qt::darkRed);
    rule.pattern = QRegExp("\".*\"");
    rule.pattern.setMinimal(true);
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    commentFormat.setForeground(Qt::darkGreen);
    rule.pattern = QRegExp("#[^\n]*");
    rule.format = commentFormat;
    highlightingRules.append(rule);
}

//! Parentheses matching code taken from Qt Quarterly Issue 31 � Q3 2009
void SyntaxHighlighter::highlightBlock(const QString &text)
{
    QString s = text;
    foreach (HighlightingRule rule, highlightingRules) {
        QRegExp expression(rule.pattern);
        int index = s.indexOf(expression);
        while (index >= 0) {
            int length = expression.matchedLength();
            setFormat(index, length, rule.format);
            index = s.indexOf(expression, index + length);
        }
    }

    TextBlockData *data = new TextBlockData;

    int leftPos = text.indexOf('(');
    while (leftPos != -1) {
        ParenthesisInfo *info = new ParenthesisInfo;
        info->character = '(';
        info->position = leftPos;

        data->insert(info);
        leftPos = text.indexOf('(', leftPos + 1);
    }

    int rightPos = text.indexOf(')');
    while (rightPos != -1) {
        ParenthesisInfo *info = new ParenthesisInfo;
        info->character = ')';
        info->position = rightPos;

        data->insert(info);

        rightPos = text.indexOf(')', rightPos +1);
    }

    setCurrentBlockUserData(data);
}
