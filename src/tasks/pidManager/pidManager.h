#pragma once

#include "globals/globals.h"
#include "../src/config.h"
#include "../sysStateManager/sysStateManager.h"
#include "AutoTunePID.h"

void pidManagerTask(void *pvParameters);

static void initPidManager();
static void receiveFromSysStateManager();
static void sendToSysStateManager();
