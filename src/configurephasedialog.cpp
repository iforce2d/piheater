#include "configurephasedialog.h"
#include "ui_configurephasedialog.h"

#include <QMessageBox>

ConfigurePhaseDialog::ConfigurePhaseDialog(QWidget *parent, heatRampPhase_t* _phase, bool _isNew) : QDialog(parent), ui(new Ui::ConfigurePhaseDialog)
{
    ui->setupUi(this);

    phaseWasDeleted = false;
    phase = _phase;
    isNew = _isNew;

    ui->tempSpinBox->setValue(phase->targetTemp);
    ui->durationSpinBox->setValue(phase->duration);

    ui->deletePhaseButton->setStyleSheet("QPushButton {color: red;}");

    if ( isNew ) {
        setWindowTitle("Add phase");
        ui->deletePhaseButton->hide();
    }
    else {
        setWindowTitle("Edit phase");
        ui->buttonBox->button(QDialogButtonBox::Cancel)->hide();
    }
}

ConfigurePhaseDialog::~ConfigurePhaseDialog()
{
    delete ui;
}

void ConfigurePhaseDialog::on_tempSpinBox_valueChanged(int arg1)
{
    phase->targetTemp = arg1;
}


void ConfigurePhaseDialog::on_durationSpinBox_valueChanged(int arg1)
{
    phase->duration = arg1;
}


void ConfigurePhaseDialog::on_deletePhaseButton_clicked()
{
    QMessageBox::StandardButton reply = QMessageBox::warning(this, "Delete phase", QString("Delete phase?"), QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        phaseWasDeleted = true;
        this->close();
    }
}

