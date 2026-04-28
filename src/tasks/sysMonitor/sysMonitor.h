#pragma once

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "globals/globals.h"

void sysMonitorTask(void *pvParameters);

void printTasksStats();

const char* taskStatusToStr(eTaskState state);