#ifndef SETTINGS_H
#define SETTINGS_H

#include <QStringList>
#include "heatprofile.h"

enum {
    RS_OK,
    RS_FILE_NOT_FOUND,
    RS_FILE_PARSE_FAILED
};

class Settings {
public:
    static Settings& getInstance();
    int readSettings();
    bool saveSettings();

    void setFullScreen(bool tf);
    bool isFullScreen();

    void setDarkPlot(bool tf);
    bool isDarkPlot();

    void setStatusOnRight(bool tf);
    bool isStatusOnRight();

    void setIdleTemp(float t);
    float getIdleTemp();

    void setOutputPWMScale(float t);
    float getOutputPWMScale();

    HeatProfile* getProfileByName(QString name, HeatProfile* exclude = NULL);
    QString getUniqueProfileName();
    void addNewProfile(HeatProfile *p);
    void deleteProfile(HeatProfile* profile);
    QStringList getProfileNameList();
    HeatProfile* getProfileByIndex(int i);

private:
    Settings();
    float idleTemp;
    float outputPWMScale;
    bool fullScreen;
    bool darkPlot;
    bool statusOnRight;

    QList<HeatProfile*> profiles;
};

#endif // SETTINGS_H
