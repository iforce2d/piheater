#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStringListModel>
#include <QTimer>
#include <QElapsedTimer>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void setFullScreen(bool tf);
    float getSecondsElapsed();

    void moveStatusPanel(int dir);
    void updateProfileListDisplay();
    void updatePlot();

public slots:
    void updateHardwareOutputsDisplay();
    void repositionPlot();

private slots:
    void on_createProfileButton_clicked();
    void on_quitButton_clicked();
    void on_hardwareUpdateTimer();
    void on_selectProfileButton_clicked();
    void on_profileListView_clicked(const QModelIndex &index);
    void on_configureProfileButton_clicked();
    void on_backButton_clicked();
    void on_settingsButton_clicked();
    void on_startButton_clicked();
    void on_stopButton_clicked();
    void on_pauseButton_clicked();
    void on_showPlotButton_clicked();

private:
    Ui::MainWindow *ui;
    QStringListModel profileListModel;
    QTimer *hardwareUpdateTimer;
    QElapsedTimer elapsedTimer;
    class HeatProfile* activeProfile;
    bool running;
    bool paused;
    bool justPausedNow;
    bool isSettingsDialogOpen;
    int remainingHistoryCount;
    QVector<double> liveTempHistory;
    QVector<double> runTempHistoryX;
    QVector<double> runTempHistoryY;
    void getProfilePlotPoints(QVector<double> &xs, QVector<double> &ys);
};
#endif // MAINWINDOW_H
