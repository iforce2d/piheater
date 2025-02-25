#ifndef HEATPROFILE_H
#define HEATPROFILE_H

#include <QString>
#include <QList>

enum heatProfileType_e {
    HPT_CONSTANT = 0,
    HPT_RAMPING
};

struct heatRampPhase_t {
    float targetTemp;
    float duration;

    heatRampPhase_t(float temp, float dur) { targetTemp = temp; duration = dur; }
};

class HeatProfile {
public:
    QString name;
    heatProfileType_e type;
    float constantTemperature;
    QList<heatRampPhase_t> rampPhases;

    //float timeElapsed;

    HeatProfile(QString n);

    void setDefaultRamp();
    QStringList getRampPhasesStringList();
    void addRampPhase(heatRampPhase_t phase);
    void removeRampPhase(int index);
    void setRampPhase(int index, heatRampPhase_t phase);
    heatRampPhase_t getPhaseByIndex(int index);
    int getPlotPoints(QVector<double> &xs, QVector<double> &ys);

    //void reset();
    bool tick(float timeElapsed);
    //float getTimeElapsed();
    float getFractionElapsed(float timeElapsed);
    float getCurrentTarget(float timeElapsed);
};

#endif // HEATPROFILE_H
