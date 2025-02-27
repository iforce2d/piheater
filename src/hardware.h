#ifndef HARDWARE_H
#define HARDWARE_H

#define MAX31855_FAULT_ALL  (0x07)
#define MAX31855_FAULT_OC   (0x01)
#define MAX31855_FAULT_GND  (0x02)
#define MAX31855_FAULT_VCC  (0x04)

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
