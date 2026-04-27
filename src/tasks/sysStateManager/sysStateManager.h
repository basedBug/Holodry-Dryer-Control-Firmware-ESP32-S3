#pragma once

#include "../src/globals/globals.h"
#include "../sensorManager/sensorManager.h"
#include "actuators.h"
#include "thermodynamics.h"
#include "ArduinoJson.h"

enum class SystemStatus : uint8_t
{
    IDLE,
    TUNING,
    DRYING,
    VENTING,
    EMERGENCY_VENTING,
    TESTING_MODE,
    SHOWCASE_MODE,
    UNKNOWN
};

struct ChamberState
{
    float maxChamberSensorTemp;
    float avgChamberSensorTemp;

    float maxChamberSensorHum;
    float avgChamberSensorHum;
};

struct AmbientState
{
    float ambientSensorTemp;
    float ambientSensorHum;
};

struct FilamentState
{
    float maxFilamentTemp;
    float avgFilamentTemp;
};

struct SystemConfig
{
    float targetChamberTemp;
    float maxAllowedChamberTemp;
    float targetRelHum;
};

struct SystemState
{
    SystemStatus systemStatus;
    SystemConfig systemConfig;

    ChamberState chamberState;
    AmbientState ambientState;
    FilamentState filamentState;

    Actuators::ActuatorsStatus actuatorsStatus;

    float exteriorAbsHum;
    float interiorAbsHum;
};

struct Sensors
{
    float chamberSensorTemp_0;
    float chamberSensorTemp_1;
    float chamberSensorTemp_2;

    float chamberSensorHum_0;
    float chamberSensorHum_1;
    float chamberSensorHum_2;

    float ambientSensorTemp;
    float ambientSensorHum;

    float filamentSurfaceTemp_0;
    float filamentSurfaceTemp_1;

    float heaterSensorTemp;
};

struct HeaterPidControl
{
    float setpoint;
    float sensorTemp;
    bool performTune;
    bool tuneCompleted;
};

extern SystemState systemState;
extern HeaterPidControl heaterPidControl;

void sysStateManagerTask(void *pvParameters);
void receiveFromSensorManager();
void receiveFromPidManager();
void sendToPidManager();
void computeSysVariables();
void receiveFromCommsManager();

void manageSystem();
SystemStatus sysStatusStrTosysStatus(const char* str);
const char* sysStatusToSysStatusStr(SystemStatus status);