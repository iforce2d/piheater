#ifndef CONFIGUREPROFILEDIALOG_H
#define CONFIGUREPROFILEDIALOG_H

#include <QDialog>
#include <QStringListModel>

#include "settings.h"

namespace Ui {
class ConfigureProfileDialog;
}

class ConfigureProfileDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConfigureProfileDialog(QWidget *parent, HeatProfile* prof);
    ~ConfigureProfileDialog();

    bool profileWasDeleted;

private slots:
    void updatePhaseListDisplay();
    void on_constantTypeRadioButton_toggled(bool checked);
    void on_rampingTypeRadioButton_toggled(bool checked);
    void on_profileNameLineEdit_textEdited(const QString &arg1);
    void on_constantTempSpinBox_valueChanged(int arg1);
    void on_deleteProfileButton_clicked();
    void on_addPhaseButton_clicked();
    void on_editPhaseButton_clicked();
    void on_rampPhasesListView_clicked(const QModelIndex &index);

private:
    class MainWindow* mainwindow;
    Ui::ConfigureProfileDialog *ui;
    HeatProfile* profile;
    bool creatingNew;
    QStringListModel rampPhasesListModel;
};

#endif // CONFIGUREPROFILEDIALOG_H
