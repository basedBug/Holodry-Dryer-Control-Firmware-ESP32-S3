#pragma once

#include "globals/globals.h"
#include "config.h"
#include "ArduinoJson.h"

void commsManagerTask(void *pvParameters);

static void initCommsManager();
static void receiveFromSysStateManager();
static bool sendToSysStateManager(JsonDocument &doc);
static void receiveFromSerialComms();
static void sendToSerialComms(JsonDocument &doc);