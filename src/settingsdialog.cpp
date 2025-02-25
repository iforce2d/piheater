#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include "settings.h"
#include "hardware.h"
#include "mainwindow.h"

ConfigDialog::ConfigDialog(QWidget *parent) : QDialog(parent), ui(new Ui::ConfigDialog)
{
    mainwindow = (MainWindow*)parent;
    ui->setupUi(this);

    ui->pmwPercentSlider->setValue(0);
    ui->idleTempSpinBox->setValue( Settings::getInstance().getIdleTemp() );
    ui->outputPWMScaleSpinBox->setValue( Settings::getInstance().getOutputPWMScale() );

    ui->fullScreenCheckBox->setChecked( Settings::getInstance().isFullScreen() );
    ui->darkPlotCheckBox->setChecked( Settings::getInstance().isDarkPlot() );
    ui->statusPositionCheckBox->setChecked( Settings::getInstance().isStatusOnRight() );
}

ConfigDialog::~ConfigDialog()
{
    delete ui;
}


void ConfigDialog::on_pmwPercentSlider_valueChanged(int value)
{
    setPWMPercent( value );
    mainwindow->updateHardwareOutputsDisplay();
}


void ConfigDialog::on_coolingOnCheckBox_toggled(bool checked)
{
    setCoolingOn( checked );
    mainwindow->updateHardwareOutputsDisplay();
}


void ConfigDialog::on_buzzerOnCheckBox_toggled(bool checked)
{
    setBuzzerOn( checked );
    mainwindow->updateHardwareOutputsDisplay();
}


void ConfigDialog::on_relayOnCheckBox_toggled(bool checked)
{
    setRelayOn( checked );
    mainwindow->updateHardwareOutputsDisplay();
}


void ConfigDialog::on_statusPositionCheckBox_toggled(bool checked)
{
    mainwindow->moveStatusPanel( checked ? 1 : -1 );
}


void ConfigDialog::on_fullScreenCheckBox_toggled(bool checked)
{
    mainwindow->setFullScreen( checked );
}


void ConfigDialog::on_idleTempSpinBox_valueChanged(double arg1)
{
    Settings::getInstance().setIdleTemp( arg1 );
}


void ConfigDialog::on_outputPWMScaleSpinBox_valueChanged(int arg1)
{
    Settings::getInstance().setOutputPWMScale( arg1 );
}


void ConfigDialog::on_darkPlotCheckBox_toggled(bool checked)
{
    Settings::getInstance().setDarkPlot( checked );
}

