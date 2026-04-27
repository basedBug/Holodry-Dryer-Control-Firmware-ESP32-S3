#pragma once

#include "globals/globals.h"
#include "../src/config.h"
#include "../sysStateManager/sysStateManager.h"
#include "AutoTunePID.h"

void pidManagerTask(void *pvParameters);

void initPidManager();
void receiveFromTaskManager();
void sendToTaskManager();
