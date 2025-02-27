#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QStringListModel>

#include "settings.h"
#include "hardware.h"
#include "configureprofiledialog.h"
#include "settingsdialog.h"

#define TICK_MS             1000    // milliseconds between temperature readings
#define PRE_HISTORY_SEC     20      // seconds of temperature history to record before a run
#define POST_HISTORY_SEC    600     // seconds of temperature history to record after a run
#define LIVE_TEMP_SEC       60      // seconds of live reading to show on plot (green line)
#define GRADIENT_SEC        10      // seconds over which to calculate the degrees per second rate (should be <= LIVE_TEMP_SEC)

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    activeProfile = NULL;
    running = false;
    paused = false;
    justPausedNow = false;
    isSettingsDialogOpen = false;
    remainingHistoryCount = 0;

    ui->setupUi(this);

    if ( Settings::getInstance().isFullScreen() )
        setWindowState(Qt::WindowFullScreen);
    if ( Settings::getInstance().isStatusOnRight() )
        moveStatusPanel(1);

    ui->faultStatusLabel->setStyleSheet("QLabel { color : red; }");
    ui->faultStatusLabel->hide();

    ui->startButton->setStyleSheet("QPushButton {color: green;}");
    ui->pauseButton->setStyleSheet("QPushButton {color: blue;}");
    ui->stopButton->setStyleSheet("QPushButton {color: red;}");

    ui->plotWidget->hide();
    ui->backButton->hide();
    ui->startButton->hide();
    ui->stopButton->hide();
    ui->pauseButton->hide();
    ui->configureProfileButton->hide();

    profileListModel.setStringList( Settings::getInstance().getProfileNameList() );
    ui->profileListView->setModel(&profileListModel);

    ui->selectProfileButton->setEnabled( false );

    ui->currentTargetLabel->hide();
    ui->timeElapsedLabel->hide();
    ui->progressLabel->hide();

    updateHardwareOutputsDisplay();

    hardwareUpdateTimer = new QTimer(this);
    connect(hardwareUpdateTimer, SIGNAL(timeout()), this, SLOT(on_hardwareUpdateTimer()));
    hardwareUpdateTimer->start(TICK_MS);

    updatePlot();
    repositionPlot();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updateHardwareOutputsDisplay()
{
    ui->pwmStatusLabel->setText(     QString::asprintf("PWM: %0.0f %%", getPWMPercent()) );
    ui->relayStatusLabel->setText(   QString::asprintf("Relay: %s", getRelayOn() ? "on":"off") );
    ui->coolingStatusLabel->setText( QString::asprintf("Cooling: %s", getCoolingOn() ? "on":"off") );
    ui->buzzerStatusLabel->setText(  QString::asprintf("Buzzer: %s", getBuzzerOn() ? "on":"off") );
}

void MainWindow::on_createProfileButton_clicked()
{
    HeatProfile* newProfile = new HeatProfile("");
    newProfile->setDefaultRamp();

    ConfigureProfileDialog dialog(this, newProfile);

    if ( dialog.exec() == QDialog::Accepted ) {
        Settings::getInstance().addNewProfile( newProfile );
        Settings::getInstance().saveSettings();
        updateProfileListDisplay();
        updatePlot();
        repositionPlot();
    }
    else {
        delete newProfile;
    }
}

void MainWindow::on_quitButton_clicked()
{
    hardwareUpdateTimer->stop();
    this->close();
}

void MainWindow::on_hardwareUpdateTimer()
{
    updateHardwareInputs();
    ui->ambientStatusLabel->setText( QString::asprintf("Ambient: %0.2f", getTemperatureAmbient()) );

    char buf[32];
    buf[0] = 0;
    if ( liveTempHistory.length() > (GRADIENT_SEC*1000/TICK_MS) ) {
        float oldVal = liveTempHistory.at( liveTempHistory.length() - (GRADIENT_SEC*1000/TICK_MS) );
        float newVal = liveTempHistory.last();
        float gradient = (newVal - oldVal) / (float)GRADIENT_SEC;
        sprintf(buf, " (%0.1f/sec)", gradient );
    }

    ui->sensorStatusLabel->setText(  QString::asprintf("Sensor: %0.2f%s", getTemperatureSensor(), buf) );

    while ( liveTempHistory.size() > (LIVE_TEMP_SEC*1000)/TICK_MS )
        liveTempHistory.pop_front();
    liveTempHistory.append( getTemperatureSensor() );

    int ff = getFaultFlags();
    if ( ff ) {
        ui->faultStatusLabel->show();
        if ( ff & MAX31855_FAULT_OC )
            ui->faultStatusLabel->setText( "(open)" );
        else if ( ff & MAX31855_FAULT_GND )
            ui->faultStatusLabel->setText( "(gnd)" );
        else if ( ff & MAX31855_FAULT_VCC )
            ui->faultStatusLabel->setText( "(vcc)" );
    }
    else
        ui->faultStatusLabel->hide();

    if ( activeProfile ) {

        float dt = TICK_MS / 1000.f;

        if ( running ) {
            if ( ! paused )
                activeProfile->tick( dt );

            float fraction = activeProfile->getFractionElapsed( getSecondsElapsed() );

            ui->currentTargetLabel->setText(  QString::asprintf("Current target: %0.1f °C", activeProfile->getCurrentTarget( getSecondsElapsed() )) );
            ui->timeElapsedLabel->setText(  QString::asprintf("Time elapsed: %0.2f sec", getSecondsElapsed()) );
            ui->progressLabel->setText(  QString::asprintf("Progress: %0.0f %%", 100 * fraction) );

            //printf("%0.2f, %0.2f\n", activeProfile->getTimeElapsed(), activeProfile->getCurrentTarget()); fflush(stdout);

            if ( fraction >= 1 ) {
                this->on_stopButton_clicked();
                remainingHistoryCount = (POST_HISTORY_SEC * 1000) / TICK_MS;
            }
        }

        if ( running || remainingHistoryCount > 0 ) {

            if ( running && paused ) {
                if ( ! justPausedNow ) {
                    runTempHistoryX.removeLast();
                    runTempHistoryY.removeLast();
                }
                justPausedNow = false;
            }

            float xval = running ? getSecondsElapsed() : (runTempHistoryX.last() + dt);
            runTempHistoryX.append( xval );
            runTempHistoryY.append( getTemperatureSensor() );
            remainingHistoryCount--;
        }
    }
    else {
        // when no profile is active, obey idle temp setting
        if ( ! isSettingsDialogOpen ) {
            float idleTemp = Settings::getInstance().getIdleTemp();
            if ( getTemperatureSensor() >= idleTemp + 2 )
                setCoolingOn( true );
            else if ( getTemperatureSensor() <= idleTemp - 2 )
                setCoolingOn( false );
        }
    }

    updateHardwareOutputsDisplay();
    updatePlot();
    // don't reposition plot
}

void MainWindow::on_profileListView_clicked(const QModelIndex &index)
{
    ui->selectProfileButton->setEnabled( index.row() > -1 );
}

void MainWindow::on_showPlotButton_clicked()
{
    ui->profileNameLabel->hide();
    ui->selectProfilesWidget->hide();
    ui->quitButton->hide();

    ui->plotWidget->show();
    ui->backButton->show();

    updatePlot();
    repositionPlot();
}

void MainWindow::on_selectProfileButton_clicked()
{
    QModelIndex index = ui->profileListView->currentIndex();
    if ( index.row() < 0 )
        return;

    HeatProfile* profile = Settings::getInstance().getProfileByIndex( index.row() );
    if ( profile ) {

        activeProfile = profile;

        ui->profileNameLabel->setText(  QString::asprintf("Profile: ").append(activeProfile->name) );

        ui->selectProfilesWidget->hide();
        ui->quitButton->hide();

        ui->startButton->show();
        ui->plotWidget->show();
        ui->backButton->show();
        ui->configureProfileButton->show();

        updatePlot();
        repositionPlot();
    }
}

void MainWindow::on_configureProfileButton_clicked()
{
    if ( ! activeProfile )
        return;

    ConfigureProfileDialog dialog(this, activeProfile);
    dialog.exec();

    if ( dialog.profileWasDeleted ) {
        Settings::getInstance().deleteProfile(activeProfile);
        Settings::getInstance().saveSettings();
        updateProfileListDisplay();
        on_backButton_clicked();
    }
    else {
        ui->profileNameLabel->setText(  QString::asprintf("Profile: ").append(activeProfile->name) );
        Settings::getInstance().saveSettings();
        updateProfileListDisplay();
        updatePlot();
        repositionPlot();
    }
}

void MainWindow::on_backButton_clicked()
{
    ui->configureProfileButton->hide();
    ui->backButton->hide();
    ui->plotWidget->hide();
    ui->startButton->hide();

    ui->selectProfilesWidget->show();
    ui->quitButton->show();

    activeProfile = NULL;
}

void MainWindow::on_settingsButton_clicked()
{
    SettingsDialog dialog(this);
    isSettingsDialogOpen = true;
    dialog.exec();
    isSettingsDialogOpen = false;

    setPWMPercent(0);
    setRelayOn(false);
    setCoolingOn(false);
    setBuzzerOn(false);

    updateHardwareOutputsDisplay();
    Settings::getInstance().saveSettings();
}

void MainWindow::on_startButton_clicked()
{
    if ( ! activeProfile )
        return;

    ui->configureProfileButton->hide();
    ui->startButton->hide();
    ui->backButton->hide();

    //activeProfile->reset();
    elapsedTimer.start();

    runTempHistoryX.clear();
    runTempHistoryY.clear();

    runTempHistoryY = liveTempHistory;
    int numToDrop = runTempHistoryY.length() - (PRE_HISTORY_SEC * 1000/TICK_MS);
    if ( numToDrop > 0 )
        runTempHistoryY.remove(0, numToDrop);
    for (int i = 0; i < runTempHistoryY.length(); i++) {
        runTempHistoryX.prepend( -i * (TICK_MS/1000.f) );
    }

    ui->currentTargetLabel->setText( "Current target:" );
    ui->timeElapsedLabel->setText( "Time elapsed: 0.0 sec" ); // just to prevent old value from showing briefly
    ui->progressLabel->setText( "Progress:" );

    ui->stopButton->show();
    ui->currentTargetLabel->show();
    ui->timeElapsedLabel->show();

    if ( activeProfile->type == HPT_RAMPING  ) {
        ui->progressLabel->show();
        //ui->pauseButton->show();
    }
    else {
        ui->progressLabel->hide();
        ui->pauseButton->hide();
    }

    running = true;
    paused = false;
}

void MainWindow::on_stopButton_clicked()
{
    ui->stopButton->hide();
    ui->pauseButton->hide();
    ui->progressLabel->hide();
    ui->timeElapsedLabel->hide();
    ui->currentTargetLabel->hide();

    setPWMPercent(0);
    setCoolingOn(false);
    setRelayOn(false);
    setBuzzerOn(false);
    updateHardwareOutputsDisplay();

    running = false;
    paused = false;
    ui->pauseButton->setText("Pause"); // for next time

    ui->configureProfileButton->show();
    ui->startButton->show();
    ui->backButton->show();
}

void MainWindow::on_pauseButton_clicked()
{
    paused = ! paused;
    if ( paused ) {
        ui->pauseButton->setText("Resume");
        justPausedNow = true;
    }
    else
        ui->pauseButton->setText("Pause");
}

void MainWindow::setFullScreen(bool tf) {
    if ( tf )
        this->setWindowState(Qt::WindowFullScreen);
    else
        this->setWindowState(Qt::WindowNoState);
    Settings::getInstance().setFullScreen(tf);
}

float MainWindow::getSecondsElapsed()
{
    return elapsedTimer.elapsed() / 1000.f;
}

void MainWindow::updateProfileListDisplay()
{
    profileListModel.setStringList( Settings::getInstance().getProfileNameList() );
}

void MainWindow::moveStatusPanel(int dir)
{
    QWidget* widget = ui->statusDisplayWidget;

    QHBoxLayout* myLayout = qobject_cast<QHBoxLayout*>(widget->parentWidget()->layout());

    int newIndex = (dir < 0) ? 0 : (myLayout->count()-1);
    myLayout->removeWidget(widget);
    myLayout->insertWidget(newIndex, widget);

    Settings::getInstance().setStatusOnRight( dir > 0 );
}

void MainWindow::getProfilePlotPoints(QVector<double> &xs, QVector<double> &ys)
{
    xs.clear();
    ys.clear();

    if ( ! activeProfile )
        return;

    if ( activeProfile->type == HPT_RAMPING ) {
        activeProfile->getPlotPoints(xs, ys);
    }
    else {
        xs.append(0);
        xs.append(300);
        ys.append(activeProfile->constantTemperature);
        ys.append(activeProfile->constantTemperature);
    }
}

void MainWindow::updatePlot()
{
    QCustomPlot* cp = ui->customPlot;

    QColor bgColor =           QColor(255, 255, 255, 255);
    QColor fgColor =           QColor(  0,   0,   0, 255);
    QColor liveReadingColor =  QColor(  0, 192,   0, 255);
    QColor targetPhasesColor = QColor(  0,   0, 255, 255);
    QColor currentTimeColor =  QColor(127,   0, 127, 255);
    QColor runHistoryColor =   QColor(255,   0,   0, 255);

    if ( Settings::getInstance().isDarkPlot() ) {
        bgColor =           QColor(  0,   0,   0, 255);
        fgColor =           QColor(255, 255, 255, 255);
        liveReadingColor =  QColor(100, 255, 100, 255);
        targetPhasesColor = QColor(100, 100, 255, 255);
        currentTimeColor =  QColor(255,   0, 255, 255);
        runHistoryColor =   QColor(255, 100, 100, 255);
    }

    cp->setBackground( QBrush(bgColor) );

    cp->clearPlottables();

    {
        // live temperature reading
        QVector<double> xs;

        float miny = 999;
        float maxy = -999;
        for (int i = 0; i < liveTempHistory.length(); i++) {
            miny = qMin(miny, liveTempHistory[i]);
            maxy = qMax(maxy, liveTempHistory[i]);
            xs.prepend( -i * (TICK_MS/1000.f) );
        }

        QCPAxis* liveAxisX = cp->xAxis2;
        QCPAxis* liveAxisY = cp->yAxis2;

        if ( ! activeProfile ) { // only showing live values
            liveAxisX = cp->xAxis;
            liveAxisY = cp->yAxis;
            cp->setInteractions(QCP::iNone);
        }

        liveAxisX->setRange(-LIVE_TEMP_SEC, 0);
        liveAxisY->setRange(miny - 5, maxy + 5);

        liveAxisX->setBasePen( QPen(fgColor) );
        liveAxisY->setBasePen( QPen(fgColor) );
        liveAxisX->setTickLabelColor( fgColor );
        liveAxisY->setTickLabelColor( fgColor );

        cp->addGraph(liveAxisX, liveAxisY); // graph 0, last LIVE_TEMP_SEC sec (top and right axes)
        cp->graph(cp->graphCount()-1)->setData(xs, liveTempHistory);
        QPen pen = QPen( liveReadingColor );
        pen.setWidth(2);
        cp->graph(cp->graphCount()-1)->setPen( pen );
        liveAxisX->setVisible(true);
        liveAxisY->setVisible(true);
    }

    if ( activeProfile ) {
        {
            // target phases
            QVector<double> xs, ys;
            getProfilePlotPoints(xs, ys);
            cp->addGraph(); // profile span (bottom and left axes)
            cp->graph(cp->graphCount()-1)->setData(xs, ys);
            QPen pen = QPen( targetPhasesColor );
            pen.setWidth(2);
            cp->graph(cp->graphCount()-1)->setPen( pen );
            cp->xAxis->setLabel("Time (sec)");
            cp->yAxis->setLabel("Temp (°C)");

            cp->xAxis->setBasePen( QPen(fgColor) );
            cp->yAxis->setBasePen( QPen(fgColor) );
            cp->xAxis->setLabelColor( fgColor );
            cp->yAxis->setLabelColor( fgColor );
            cp->xAxis->setTickLabelColor( fgColor );
            cp->yAxis->setTickLabelColor( fgColor );
        }

        if ( running && activeProfile->type == HPT_RAMPING )
        {
            // current time line
            QVector<double> xs, ys;
            xs.append( getSecondsElapsed() );
            xs.append( getSecondsElapsed() );
            ys.append( cp->yAxis->range().lower );
            ys.append( cp->yAxis->range().upper );
            cp->addGraph(); // progress position line only
            cp->graph(cp->graphCount()-1)->setData(xs, ys);
            cp->graph(cp->graphCount()-1)->setPen( QPen( currentTimeColor ) );
        }

        if ( ! runTempHistoryX.empty() )
        {
            // run history
            cp->addGraph();
            cp->graph(cp->graphCount()-1)->setData(runTempHistoryX, runTempHistoryY);
            QPen pen = QPen( runHistoryColor );
            pen.setWidth(2);
            cp->graph(cp->graphCount()-1)->setPen( pen );
        }

        cp->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    }

    cp->replot();
}

void MainWindow::repositionPlot()
{
    QCustomPlot* cp = ui->customPlot;

    QVector<double> xs, ys;
    getProfilePlotPoints(xs, ys);

    float minx = 999;
    float maxx = -999;
    float miny = 999;
    float maxy = -999;
    for (int i = 0; i < xs.length(); i++) {
        minx = qMin(minx, xs[i]);
        maxx = qMax(maxx, xs[i]);
        miny = qMin(miny, ys[i]);
        maxy = qMax(maxy, ys[i]);
    }

    cp->xAxis->setRange(minx - 10, maxx + 10);
    cp->yAxis->setRange(miny - 10, maxy + 10);
}





























