#include "brahms_dialog.h"
#include "ui_brahms_dialog.h"


BRAHMS_dialog::BRAHMS_dialog(rootData *data, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BRAHMS_dialog)
{
    ui->setupUi(this);
    this->data = data;
    connect(this->ui->open_ns, SIGNAL(clicked()), this, SLOT(getNamespace()));
    connect(this->ui->open_path, SIGNAL(clicked()), this, SLOT(getPath()));
    connect(this, SIGNAL(write_out_model(QString)), data, SLOT(export_model_xml(QString)));
    connect(this->ui->launch_but, SIGNAL(clicked()), this, SLOT(runIt()));

    // default Namespace
    ui->text_ns->setText(qgetenv("HOME") + "/Namespace");
    ui->text_path->setText(qgetenv("HOME") + "/SystemML");
}

BRAHMS_dialog::~BRAHMS_dialog()
{
    delete ui;
}


void BRAHMS_dialog::getNamespace() {
    QString dirName = QFileDialog::getExistingDirectory(this, "Choose the BRAHMS Namespace directory", qgetenv("HOME"));
    this->ui->text_ns->setText(dirName);
}

void BRAHMS_dialog::getPath() {
    QString dirName = QFileDialog::getExistingDirectory(this, "Choose the BRAHMS SystemML directory", qgetenv("HOME"));
    this->ui->text_path->setText(dirName);
}


void BRAHMS_dialog::runIt() {
    QDir lib_path(QApplication::applicationDirPath());

    // find or make the working directory
    if (!QDir(lib_path.absoluteFilePath("wd")).exists()) {
        QDir().mkdir("wd");
    }

    QDir wk_dir = QDir(lib_path.absoluteFilePath("wd"));

    QStringList filters;
    filters.push_back("rep*");
    QStringList entries = wk_dir.entryList(filters);
    for (int i = 0; i < entries.size(); ++i) {
        wk_dir.remove(entries[i]);
    }

    ui->progressBar->setValue(5);
    this->ui->output->append("## BEGINNING... writing model...");
    this->ui->output->repaint();
    this->ui->progressBar->repaint();

    // write out the model
    emit write_out_model(wk_dir.absolutePath());

    ui->progressBar->setValue(10);
    this->ui->output->append("## done!\n");
    this->ui->output->repaint();
    this->ui->progressBar->repaint();

    // set up the environment for the spawned processes
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("SYSTEMML_INSTALL_PATH", this->ui->text_path->text());
    env.insert("PATH", qgetenv("PATH") + ":" + this->ui->text_path->text() + "/BRAHMS/bin");

    // convert the model
    QProcess convertor;
    convertor.setWorkingDirectory(wk_dir.absolutePath());
    convertor.setProcessEnvironment(env);
    QStringList args;
    args.push_back("model.xml");
    args.push_back(this->ui->text_ns->text());
    if (ui->recompile->isChecked() == true) {
        args.push_back("true");
        this->ui->output->append("## COMPILING COMPONENTS - this may take some time\n");
        this->ui->output->repaint();
        this->ui->progressBar->repaint();
    }
    else
        args.push_back("false");
    this->ui->output->append("## CONVERTING...");
    this->ui->output->repaint();
    this->ui->progressBar->repaint();

    convertor.start("./convert_script", args);
    bool success = convertor.waitForFinished(3000000);
    this->ui->output->append("## done!\n");
    this->ui->output->repaint();
    this->ui->progressBar->repaint();

    if (!success)
        return;
    if (success) {
        this->ui->progressBar->setValue(25);
        QByteArray output = convertor.readAllStandardError();
        QString outputStringErr(output);
        if (outputStringErr.size() > 0 && outputStringErr[0] != 'r') {
            this->ui->output->append(outputStringErr);
            this->ui->progressBar->setValue(0);
            return;
        }
        output = convertor.readAllStandardOutput();
        QString outputString(output);
        this->ui->output->append(outputString);
    }

    // create sys-exe.xml ///////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////

    QFile exeFile( wk_dir.absoluteFilePath("sys-exe.xml") );
    if (!exeFile.open( QIODevice::WriteOnly)) {
        QMessageBox msgBox;
        msgBox.setText("Error BRAHMS execution file - is there sufficient disk space?");
        msgBox.exec();
        return;
    }

    QDomDocument * doc = new QDomDocument;

    QDomProcessingInstruction xmlDeclaration = doc->createProcessingInstruction("xml", "version=\"1.0\" encoding=\"ISO-8859-1\"");
    doc->appendChild(xmlDeclaration);

    QDomElement exe = doc->createElement("Execution");
    exe.setAttribute("Version","1.0");
    exe.setAttribute("AuthTool","SpineCreator BRAHMS Exporter");
    exe.setAttribute("AuthToolVersion","TBC");
    QDomElement title = doc->createElement("Title");
    exe.appendChild(title);

    QDomElement fileIn = doc->createElement("SystemFileIn");
    QDomText fileInText = doc->createTextNode("sys.xml");
    fileIn.appendChild(fileInText);
    exe.appendChild(fileIn);

    QDomElement fileOut = doc->createElement("SystemFileOut");
    exe.appendChild(fileOut);

    QDomElement repFile = doc->createElement("ReportFile");
    QDomText repFileText = doc->createTextNode("rep-((VOICE)).xml");
    repFile.appendChild(repFileText);
    exe.appendChild(repFile);

    QDomElement wd = doc->createElement("WorkingDirectory");
    exe.appendChild(wd);

    QDomElement exeStop = doc->createElement("ExecutionStop");
    QDomText exeStopText = doc->createTextNode(QString::number(float(this->ui->duration->value())/1000.0));
    exeStop.appendChild(exeStopText);
    exe.appendChild(exeStop);

    QDomElement seed = doc->createElement("Seed");
    exe.appendChild(seed);

    QDomElement logs = doc->createElement("Logs");
    logs.setAttribute("Precision", "6");
    logs.setAttribute("Encapsulated", "0");
    logs.setAttribute("All", "0");
    QString logString = ui->logText->text();
    QStringList logStrings = logString.split(";");
    for (int i = 0; i < logStrings.size(); ++i) {
        if (i == 0 && logStrings[i] == "all")
        {logs.setAttribute("All", "1");
            break;}
        QDomElement log = doc->createElement("Log");
        QDomText logText = doc->createTextNode(logStrings[i].replace(" ", ""));
        log.appendChild(logText);
        logs.appendChild(log);
    }
    exe.appendChild(logs);

    QDomElement voices = doc->createElement("Voices");
    QDomElement voice = doc->createElement("Voice");
    voices.appendChild(voice);
    exe.appendChild(voices);

    QDomElement aff = doc->createElement("Affinity");
    exe.appendChild(aff);

    QDomElement exePars = doc->createElement("ExecutionParameters");
    QDomElement maxThread = doc->createElement("MaxThreadCount");
    QDomText maxThreadText = doc->createTextNode("x4");// may not be the best value!
    maxThread.appendChild(maxThreadText);
    exePars.appendChild(maxThread);
    exe.appendChild(exePars);

    doc->appendChild(exe);

    // write out to file
    QTextStream tsFromFile( &exeFile );
    tsFromFile << doc->toString();

    // kill off the DOM document
    delete doc;

    exeFile.close();

    ui->output->append("\n\nCreated BRAHMS execution file...\n");

    ui->progressBar->setValue(50);

    // run the model ////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////

    // convert the model
    QProcess brahms;
    brahms.setWorkingDirectory(wk_dir.absolutePath());
    brahms.setProcessEnvironment(env);
    args.clear();
    args.push_back("sys-exe.xml");
    brahms.start("./brahms_launch", args);
    success = brahms.waitForFinished(3000000);

    if (!success)
    {   QByteArray output = brahms.readAllStandardError();
        QString outputStringErr(output);
        if (outputStringErr.size() > 0) {
            this->ui->output->append(outputStringErr);
            this->ui->progressBar->setValue(0);
            return;
        }
        this->ui->progressBar->setValue(0);
        return;}
    if (success) {
        QByteArray output = brahms.readAllStandardError();
        QString outputStringErr(output);
        this->ui->output->append(outputStringErr);

        this->ui->progressBar->setValue(75);
        output = brahms.readAllStandardOutput();
        QString outputString(output);
        this->ui->output->append(outputString);
    }

    // fix the output file
    QProcess fixer;
    fixer.setWorkingDirectory(wk_dir.absolutePath());
    QStringList args2;
    args2.push_back("sys-exe.xml");
    fixer.start("./fix_for_brahms", args2);
    success = fixer.waitForFinished(300000);

    if (!success)
    {   QByteArray output = fixer.readAllStandardError();
        QString outputStringErr(output);
        if (outputStringErr.size() > 0) {
            this->ui->output->append(outputStringErr);
            this->ui->progressBar->setValue(0);
            return;
        }
        this->ui->progressBar->setValue(0);
        return;}
    if (success) {
        QByteArray output = fixer.readAllStandardError();
        QString outputStringErr(output);
        this->ui->output->append(outputStringErr);

        this->ui->progressBar->setValue(75);
        output = fixer.readAllStandardOutput();
        QString outputString(output);
        this->ui->output->append(outputString);
    }

    // gather the results ///////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////

    // NOT DONE YET

}
