#include "settings.h"
#include "hardware.h"
#include "heatprofile.h"

#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>
#include <QMessageBox>

#define SETTINGS_FILE_NAME  "settings.json"
#define DEFAULT_IDLE_TEMP   30

Settings::Settings()
{
    fullScreen = false;
    darkPlot = false;
    statusOnRight = false;
    idleTemp = DEFAULT_IDLE_TEMP;
    outputPWMScale = 100;
}

Settings &Settings::getInstance()
{
    static Settings instance;
    return instance;
}

bool Settings::saveSettings()
{
    QJsonObject root;

    root["fullScreen"] = fullScreen;
    root["darkPlot"] = darkPlot;
    root["statusOnRight"] = statusOnRight;
    root["idleTemp"] = idleTemp;
    root["outputPWMScale"] = outputPWMScale;

    QJsonArray jsonProfilesArray;
    foreach (HeatProfile *p, profiles) {
        QJsonObject jsonProfile;

        jsonProfile["name"] = p->name;
        jsonProfile["type"] = p->type == HPT_CONSTANT ? "constant" : "ramping";
        jsonProfile["constantTemperature"] = p->constantTemperature;

        QJsonArray jsonPhasesArray;
        foreach (heatRampPhase_t phase, p->rampPhases) {
            QJsonObject jsonPhase;

            jsonPhase["targetTemp"] = phase.targetTemp;
            jsonPhase["duration"] = phase.duration;

            jsonPhasesArray.push_back( jsonPhase );
        }
        jsonProfile["phases"] = jsonPhasesArray;

        jsonProfilesArray.push_back( jsonProfile );
    }
    root["profiles"] = jsonProfilesArray;

    QByteArray ba = QJsonDocument(root).toJson();
    {
        QFile fout( SETTINGS_FILE_NAME );
        fout.open(QIODevice::WriteOnly);
        fout.write(ba);
        fout.close();
    }

    return true;
}

int Settings::readSettings()
{
    if ( ! QFile::exists( SETTINGS_FILE_NAME ) ) {
        addNewProfile(NULL);
        return RS_FILE_NOT_FOUND;
    }

    QJsonParseError parseError;
    QJsonDocument jsonDoc;
    {
        QFile fin( SETTINGS_FILE_NAME );
        fin.open(QIODevice::ReadOnly);
        QByteArray ba2 = fin.readAll();
        jsonDoc = QJsonDocument::fromJson(ba2, &parseError);
        fin.close();
    }

    if (parseError.error != QJsonParseError::NoError) {
        printf("Parse error at %d: %s\n", parseError.offset, parseError.errorString().toStdString().c_str());
        addNewProfile(NULL);
        return RS_FILE_PARSE_FAILED;
    }

    QJsonObject root = jsonDoc.object();

    fullScreen = root["fullScreen"].toBool(false);
    darkPlot = root["darkPlot"].toBool(false);
    statusOnRight = root["statusOnRight"].toBool(false);
    idleTemp = root["idleTemp"].toDouble(DEFAULT_IDLE_TEMP);
    outputPWMScale = root["outputPWMScale"].toDouble(100);

    QJsonArray jsonProfilesArray = root["profiles"].toArray();
    for (int i = 0; i < jsonProfilesArray.count(); ++i) {
        QJsonObject jsonProfile = jsonProfilesArray.at(i).toObject();
        HeatProfile* p = new HeatProfile( jsonProfile["name"].toString( "(?)" ) );
        p->type = jsonProfile["type"].toString("constant") == "constant" ? HPT_CONSTANT : HPT_RAMPING;
        p->constantTemperature = jsonProfile["constantTemperature"].toDouble(20);

        QJsonArray jsonPhasesArray = jsonProfile["phases"].toArray();
        for (int i = 0; i < jsonPhasesArray.count(); ++i) {
            QJsonObject jsonPhase = jsonPhasesArray.at(i).toObject();
            float targetTemp = jsonPhase["targetTemp"].toDouble(20);
            float duration = jsonPhase["duration"].toDouble(10);
            heatRampPhase_t phase(targetTemp, duration);
            p->addRampPhase( phase );
        }

        profiles.append( p );
    }

    return RS_OK;
}

void Settings::setFullScreen(bool tf) {
    fullScreen = tf;
}

bool Settings::isFullScreen()
{
    return fullScreen;
}

void Settings::setDarkPlot(bool tf)
{
    darkPlot = tf;
}

bool Settings::isDarkPlot()
{
    return darkPlot;
}

void Settings::setStatusOnRight(bool tf)
{
    statusOnRight = tf;
}

bool Settings::isStatusOnRight()
{
    return statusOnRight;
}

void Settings::setIdleTemp(float t)
{
    idleTemp = t;
}

float Settings::getIdleTemp()
{
    return idleTemp;
}

void Settings::setOutputPWMScale(float t)
{
    outputPWMScale = t;
}

float Settings::getOutputPWMScale()
{
    return outputPWMScale;
}

HeatProfile* Settings::getProfileByName(QString name, HeatProfile* exclude)
{
    foreach (HeatProfile *p, profiles) {
        if ( p == exclude )
            continue;
        if ( p->name == name )
            return p;
    }
    return NULL;
}

QString Settings::getUniqueProfileName()
{
    int index = 1;
    bool found = true;
    while ( found ) {
        found = false;
        QString name = QString::asprintf("Profile %d", index);
        foreach (HeatProfile *p, profiles) {
            if ( p->name == name ) {
                found = true;
                break;
            }
        }
        if ( ! found )
            return name;
        index++;
    }
    return "getUniqueProfileName() failed";
}

void Settings::addNewProfile(HeatProfile* p)
{
    if ( ! p ) {
        p = new HeatProfile( getUniqueProfileName() );
        p->setDefaultRamp();
    }
    profiles.append( p );
}

void Settings::deleteProfile(HeatProfile *profile)
{
    profiles.removeOne(profile);
    delete profile;
}

QStringList Settings::getProfileNameList()
{
    QStringList list;
    foreach (HeatProfile *p, profiles) {
        list.append( p->name );
    }
    return list;
}

HeatProfile* Settings::getProfileByIndex(int i)
{
    if ( i < profiles.size() )
        return profiles.at(i);
    return NULL;
}






