#pragma once

#include "globals/globals.h"
#include "config.h"
#include "ArduinoJson.h"

void commsManagerTask(void *pvParameters);

void initCommsManager();
void receiveFromSysStateManager();
bool sendToSysStateManager(JsonDocument &doc);
void receiveFromSerialComms();
void sendToSerialComms(JsonDocument &doc);