#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>

namespace Ui {
class ConfigDialog;
}

class ConfigDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConfigDialog(QWidget *parent = nullptr);
    ~ConfigDialog();

signals:
    void sliderChanged();

private slots:
    void on_pmwPercentSlider_valueChanged(int value);
    void on_coolingOnCheckBox_toggled(bool checked);
    void on_buzzerOnCheckBox_toggled(bool checked);
    void on_relayOnCheckBox_toggled(bool checked);
    void on_statusPositionCheckBox_toggled(bool checked);
    void on_fullScreenCheckBox_toggled(bool checked);
    void on_idleTempSpinBox_valueChanged(double arg1);
    void on_outputPWMScaleSpinBox_valueChanged(int arg1);
    void on_darkPlotCheckBox_toggled(bool checked);

private:
    class MainWindow* mainwindow;
    Ui::ConfigDialog *ui;
};

#endif // SETTINGSDIALOG_H
