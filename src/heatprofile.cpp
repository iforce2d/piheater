#include "heatprofile.h"
#include "hardware.h"

HeatProfile::HeatProfile(QString n)
{
    name = n;
    type = HPT_RAMPING;

    constantTemperature = 20;

    //reset();
}

void HeatProfile::setDefaultRamp()
{
    rampPhases.clear();

    rampPhases.append( heatRampPhase_t(100, 3) );
    rampPhases.append( heatRampPhase_t(150, 9) );
    rampPhases.append( heatRampPhase_t(183, 3) );
    rampPhases.append( heatRampPhase_t(235, 6) );
    rampPhases.append( heatRampPhase_t( 30, 6) );
}

QStringList HeatProfile::getRampPhasesStringList()
{
    QStringList list;
    foreach (heatRampPhase_t p, rampPhases) {
        QString s = QString::asprintf(" %0.0f °C, %0.0f sec", p.targetTemp, p.duration );
        list.append( s );
    }
    return list;
}

void HeatProfile::addRampPhase(heatRampPhase_t phase)
{
    rampPhases.append(phase);
}

void HeatProfile::removeRampPhase(int index)
{
    rampPhases.remove(index);
}

void HeatProfile::setRampPhase(int index, heatRampPhase_t phase)
{
    rampPhases.replace(index, phase);
}

heatRampPhase_t HeatProfile::getPhaseByIndex(int index)
{
    return rampPhases.at( index );
}

int HeatProfile::getPlotPoints(QVector<double> &xs, QVector<double> &ys)
{
    xs.clear();
    ys.clear();
    float target = 20;
    float cumulativeTime = 0;
    foreach (heatRampPhase_t p, rampPhases) {
        xs.append( cumulativeTime );
        ys.append( target );
        cumulativeTime += p.duration;
        target = p.targetTemp;
    }
    xs.append( cumulativeTime );
    ys.append( target );

    return xs.size();
}

// void HeatProfile::reset()
// {
//     timeElapsed = 0;
// }

bool HeatProfile::tick(float timeElapsed)
{
    float target = getCurrentTarget( timeElapsed );
    if ( target > getTemperatureSensor() + 2 ) {
        setPWMPercent(15);
        setCoolingOn(false);
    }
    else if ( target < getTemperatureSensor() - 2 ) {
        setPWMPercent(0);
        setCoolingOn(true);
    }
    else {
        setPWMPercent(0);
        setCoolingOn(true);
    }

    float fraction = getFractionElapsed( timeElapsed );
    setRelayOn( fraction > 0 && fraction < 1 );

    return true;
}

// float HeatProfile::getTimeElapsed()
// {
//     return timeElapsed;
// }

float HeatProfile::getFractionElapsed(float timeElapsed)
{
    if ( type == HPT_CONSTANT )
        return 0.5; // always 'part way through'
    else {
        float cumulativeTime = 0;
        foreach (heatRampPhase_t p, rampPhases) {
            cumulativeTime += p.duration;
        }
        if ( timeElapsed > cumulativeTime )
            return 1;
        else if ( cumulativeTime > 0 )
            return timeElapsed / cumulativeTime;
        else
            return 1;
    }
}

float HeatProfile::getCurrentTarget(float timeElapsed)
{
    if ( type == HPT_CONSTANT )
        return constantTemperature;
    else {
        float startTarget = 20;
        float cumulativeTime = 0;
        foreach (heatRampPhase_t p, rampPhases) {
            float phaseEndTime = cumulativeTime + p.duration;
            if ( timeElapsed > cumulativeTime && timeElapsed <= phaseEndTime ) {
                float fraction = (timeElapsed - cumulativeTime) / (phaseEndTime - cumulativeTime);
                return startTarget + fraction * (p.targetTemp - startTarget);
            }
            cumulativeTime += p.duration;
            startTarget = p.targetTemp;
        }
        return startTarget; // if outside ramp span, return final target
    }
}


