#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

#include "tasks/sensorManager/sensorManager.h" // Need access to the sensors struct

// Neccesary globals

extern QueueHandle_t xSensorDataQueue;
extern MessageBufferHandle_t xSysStateManagerToCommsManager;
extern MessageBufferHandle_t xCommsManagerToSysStateManager;

constexpr size_t SENSOR_DATA_QUEUE_SIZE = 1;
constexpr size_t MAX_MSG_SIZE = 1024;
constexpr size_t MSG_BUFFER_SIZE = MAX_MSG_SIZE*2;