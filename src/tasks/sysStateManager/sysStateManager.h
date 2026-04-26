#pragma once

#include "../src/globals/globals.h"
#include "../sensorManager/sensorManager.h"
#include "../actuatorManager/actuatorManager.h"
#include "humidity.h"
#include "ArduinoJson.h"

enum class SystemStatus : uint8_t
{
    IDLE,
    DRYING,
    VENTING,
    EMERGENCY_VENTING,
    TESTING_MODE,
    SHOWCASE_MODE
};

enum class HeaterStatus : uint8_t
{
    IDLE,
    ACTIVE
};

enum class FanStatus : uint8_t
{
    IDLE,
    ACTIVE
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
    ChamberState chamberState;
    AmbientState ambientState;
    FilamentState filamentState;

    SystemConfig systemConfig;
    SystemStatus systemStatus;
    
    float heaterTemp;
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

void sysStateManagerTask(void *pvParameters);
void receiveFromSensorManager();
void computeSysVariables();
void receiveFromCommsManager();

void manageSystem();
void sendToCommsManager();