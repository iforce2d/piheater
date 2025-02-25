#include "mainwindow.h"

#include <QApplication>
#include <QMessageBox>
#include "hardware.h"
#include "settings.h"

int main(int argc, char *argv[])
{
    if ( ! initHardware() ) {
        printf("initHardware failed\n");
        return -1;
    }

    QApplication a(argc, argv); // do this before using NULL as the message box parent below

    int settingsLoadStatus = Settings::getInstance().readSettings();
    if ( settingsLoadStatus == RS_FILE_NOT_FOUND )
        QMessageBox::warning(NULL, QString("No settings"), QString("No settings file found, using defaults"), QMessageBox::Ok);
    else if ( settingsLoadStatus == RS_FILE_PARSE_FAILED )
        QMessageBox::critical(NULL, QString("Invalid settings"), QString("Failed to parse settings file, using defaults"), QMessageBox::Ok);

    MainWindow w;
    w.show();
    int result = a.exec();

    denitHardware();

    return result;
}
