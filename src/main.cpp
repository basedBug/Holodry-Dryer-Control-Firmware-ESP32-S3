#include <Arduino.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "tasks/tasks.h"
#include "globals/globals.h"
#include "config.h"

// Task handles located in tasks header file

void setup()
{
	// Initialize the serial port for debugging
	Serial.begin(SERIAL_BAUD_RATE);

	vTaskDelay(pdMS_TO_TICKS(2000)); // A little delay to permit me to connect the damn serial to my logger
	
	xSensorManagerToCommsManager = xMessageBufferCreate(MSG_BUFFER_SIZE);
	if (!xSensorManagerToCommsManager)
	{
		Serial.println("[RTOS] Failed to create SensorManager to CommsManager message buffer");
	}

	xCommsManagerToSensorManager = xMessageBufferCreate(MSG_BUFFER_SIZE);
	if (!xCommsManagerToSensorManager)
	{
		Serial.println("[RTOS] Failed to create CommsManager to SensorManager message buffer");
	}

	xSensorDataQueue = xQueueCreate(SENSOR_DATA_QUEUE_SIZE, sizeof(SystemSensors));
	if (!xSensorDataQueue)
	{
		Serial.println("[RTOS] Failed to create SensorData queue");
	}
	
	// Create tasks, they start by themselves
	createTasks();
	
}

void loop()
{
	// Must be left alone, to avoid filling the idle task's stack
}