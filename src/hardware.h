#ifndef HARDWARE_H
#define HARDWARE_H

bool initHardware();
void setHardwareSafeValues();
void updateHardwareInputs();
void updateHardwareOutputs();
void denitHardware();

int getFaultFlags();

float getTemperatureAmbient();
float getTemperatureSensor();

void setPWMPercent(float v);
float getPWMPercent();

void setCoolingOn(bool tf);
bool getCoolingOn();

void setRelayOn(bool tf);
bool getRelayOn();

void setBuzzerOn(bool tf);
bool getBuzzerOn();

#endif // HARDWARE_H
