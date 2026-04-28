#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "sensorManager/sensorManager.h"
#include "pidManager/pidManager.h"
#include "sysStateManager/sysStateManager.h"
#include "commsManager/commsManager.h"
#include "sysMonitor/sysMonitor.h"

// Sensor handler
// Thermodynamics/System state control (actuators act in here?)
// Actuator manager
// Data handler/ Comms handler

extern TaskHandle_t xSensorManagerTaskHandle;
extern TaskHandle_t xPidManagerTaskHandle;
extern TaskHandle_t xSysStateManagerTaskHandle;
extern TaskHandle_t xCommsManagerTaskHandle;
extern TaskHandle_t xSysMonitorTaskHandle;

void createTasks();