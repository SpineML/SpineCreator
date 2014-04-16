#ifndef BATCHEXPERIMENTWINDOW_H
#define BATCHEXPERIMENTWINDOW_H

#include <QDialog>

namespace Ui {
class BatchExperimentWindow;
}

class BatchExperimentWindow : public QDialog
{
    Q_OBJECT

public:
    explicit BatchExperimentWindow(QWidget *parent = 0);
    ~BatchExperimentWindow();

private:
    Ui::BatchExperimentWindow *ui;
};

#endif // BATCHEXPERIMENTWINDOW_H
