#include "batchexperimentwindow.h"
#include "ui_batchexperimentwindow.h"

BatchExperimentWindow::BatchExperimentWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BatchExperimentWindow)
{
    ui->setupUi(this);
}

BatchExperimentWindow::~BatchExperimentWindow()
{
    delete ui;
}
