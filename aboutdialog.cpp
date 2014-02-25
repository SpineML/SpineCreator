#include "aboutdialog.h"
#include "ui_aboutdialog.h"
#include <QDebug>
#include <vector>

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);


}

AboutDialog::~AboutDialog()
{
    delete ui;
}
