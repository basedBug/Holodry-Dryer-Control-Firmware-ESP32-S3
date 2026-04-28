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
	
	xSensorDataQueue = xQueueCreate(SENSOR_DATA_QUEUE_SIZE, sizeof(SystemSensors));
	if (!xSensorDataQueue)
	{
		Serial.println("[RTOS] Failed to create SensorData queue");
	}

	xSysStateManagerToPidManagerQueue = xQueueCreate(PID_CONTROL_QUEUE_SIZE, sizeof(HeaterPidControl));
	if (!xSysStateManagerToPidManagerQueue)
	{
		Serial.println("[RTOS] Failed to create SystemStateManager to PidManager queue");
	}

	xPidManagerToSysStateManagerQueue = xQueueCreate(PID_CONTROL_QUEUE_SIZE, sizeof(HeaterPidControl));
	if (!xPidManagerToSysStateManagerQueue)
	{
		Serial.println("[RTOS] Failed to create PidManager to SystemStateManager queue");
	}

	xSysStateManagerToCommsManagerMsgBuffer = xMessageBufferCreate(MSG_BUFFER_SIZE);
	if (!xSysStateManagerToCommsManagerMsgBuffer)
	{
		Serial.println("[RTOS] Failed to create SysStateManager to CommsManager message buffer");
	}

	xCommsManagerToSysStateManagerMsgBuffer = xMessageBufferCreate(MSG_BUFFER_SIZE);
	if (!xCommsManagerToSysStateManagerMsgBuffer)
	{
		Serial.println("[RTOS] Failed to create CommsManager to SysStateManager message buffer");
	}
	
	// Create tasks, they start by themselves
	createTasks();
	
}

void loop()
{
	// Must be left alone, to avoid filling the idle task's stack
}