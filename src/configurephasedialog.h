#ifndef CONFIGUREPHASEDIALOG_H
#define CONFIGUREPHASEDIALOG_H

#include <QDialog>

#include "heatprofile.h"

namespace Ui {
class ConfigurePhaseDialog;
}

class ConfigurePhaseDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConfigurePhaseDialog(QWidget *parent, heatRampPhase_t* phase, bool isNew);
    ~ConfigurePhaseDialog();

    bool phaseWasDeleted;

private slots:
    void on_tempSpinBox_valueChanged(int arg1);
    void on_durationSpinBox_valueChanged(int arg1);

    void on_deletePhaseButton_clicked();

private:
    heatRampPhase_t* phase;
    bool isNew;
    Ui::ConfigurePhaseDialog *ui;
};

#endif // CONFIGUREPHASEDIALOG_H
