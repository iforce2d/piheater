#include "configureprofiledialog.h"
#include "ui_configureprofiledialog.h"
#include "configurephasedialog.h"
#include "mainwindow.h"

#include <QPushButton>
#include <QMessageBox>

ConfigureProfileDialog::ConfigureProfileDialog(QWidget *parent, HeatProfile *prof) : QDialog(parent), ui(new Ui::ConfigureProfileDialog)
{
    mainwindow = (MainWindow*)parent;

    ui->setupUi(this);
    profile = prof;

    creatingNew = profile->name == ""; // name can only be empty when creating new

    profileWasDeleted = false;

    ui->profileNameLineEdit->setText( profile->name );

    ui->alreadyExistsLabel->setStyleSheet("QLabel {color: red;}");
    ui->alreadyExistsLabel->hide();

    ui->deleteProfileButton->setStyleSheet("QPushButton {color: red;}");

    ui->constantTempSpinBox->setValue( profile->constantTemperature );

    ui->rampPhasesListView->setModel(&rampPhasesListModel);
    updatePhaseListDisplay();

    ui->constantTypeRadioButton->setChecked( profile->type == HPT_CONSTANT );
    ui->rampingTypeRadioButton->setChecked( profile->type == HPT_RAMPING );

    if ( creatingNew ) {
        setWindowTitle("Create profile");
        ui->deleteProfileButton->hide();
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    }
    else {
        setWindowTitle("Edit profile");
        ui->buttonBox->button(QDialogButtonBox::Cancel)->hide();
    }
}

ConfigureProfileDialog::~ConfigureProfileDialog()
{
    delete ui;
}

void ConfigureProfileDialog::updatePhaseListDisplay()
{
    rampPhasesListModel.setStringList( profile->getRampPhasesStringList() );
}

void ConfigureProfileDialog::on_constantTypeRadioButton_toggled(bool checked)
{
    if ( checked ) {
        profile->type = HPT_CONSTANT;
        ui->rampSettingsWidget->hide();
        ui->constantSettingsWidget->show();
    }
}


void ConfigureProfileDialog::on_rampingTypeRadioButton_toggled(bool checked)
{
    if ( checked ) {
        profile->type = HPT_RAMPING;
        ui->constantSettingsWidget->hide();
        ui->rampSettingsWidget->show();
    }
}


void ConfigureProfileDialog::on_profileNameLineEdit_textEdited(const QString &arg1)
{
    if ( arg1 == "" ) {
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    }
    else if ( Settings::getInstance().getProfileByName(arg1, profile) ) {
        ui->alreadyExistsLabel->show();
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    }
    else {
        ui->alreadyExistsLabel->hide();
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
        profile->name = arg1;
    }
}


void ConfigureProfileDialog::on_constantTempSpinBox_valueChanged(int arg1)
{
    profile->constantTemperature = arg1;
}


void ConfigureProfileDialog::on_deleteProfileButton_clicked()
{
    QMessageBox::StandardButton reply = QMessageBox::warning(this, "Delete profile", QString("Delete profile '%1'?").arg(profile->name), QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        profileWasDeleted = true;
        this->close();
    }
}


void ConfigureProfileDialog::on_addPhaseButton_clicked()
{
    heatRampPhase_t phase(30, 30);

    ConfigurePhaseDialog dialog(this, &phase, true);

    if ( dialog.exec() == QDialog::Accepted ) {
        profile->addRampPhase( phase );
        updatePhaseListDisplay();
        mainwindow->updatePlot();
    }
}

void ConfigureProfileDialog::on_rampPhasesListView_clicked(const QModelIndex &index)
{
    ui->editPhaseButton->setEnabled( index.row() > -1 );
}

void ConfigureProfileDialog::on_editPhaseButton_clicked()
{
    QModelIndex index = ui->rampPhasesListView->currentIndex();
    if ( index.row() < 0 )
        return;

    heatRampPhase_t phase = profile->getPhaseByIndex( index.row() );

    ConfigurePhaseDialog dialog(this, &phase, false);
    dialog.exec();

    if ( dialog.phaseWasDeleted ) {
        profile->removeRampPhase( index.row() );
    }
    else {
        profile->setRampPhase( index.row(), phase );
    }

    updatePhaseListDisplay();
    mainwindow->updatePlot();

}



















