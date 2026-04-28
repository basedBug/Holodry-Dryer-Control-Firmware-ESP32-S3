#include "tasks/tasks.h"

TaskHandle_t xSensorManagerTaskHandle = NULL;
TaskHandle_t xPidManagerTaskHandle = NULL;
TaskHandle_t xSysStateManagerTaskHandle = NULL;
TaskHandle_t xCommsManagerTaskHandle = NULL;
TaskHandle_t xSysMonitorTaskHandle = NULL;

void createTasks()
{
    xTaskCreatePinnedToCore(
		sensorManagerTask,			// Task function to be called
		"sensorManager",			// Task name (for debug)
		12000,						// Stack size (bytes)
		NULL,						// Task parameters
		3,							// Priority
		&xSensorManagerTaskHandle,	// Task handle
		0							// Core to run on
	);

	xTaskCreatePinnedToCore(
		pidManagerTask,			// Task function to be called
		"pidManager",			// Task name (for debug)
		8000,					// Stack size (bytes)
		NULL,					// Task parameters
		2,						// Priority
		&xPidManagerTaskHandle,	// Task handle
		0						// Core to run on
	);

	xTaskCreatePinnedToCore(
		sysMonitorTask,			// Task function to be called
		"sysMonitor",			// Task name (for debug)
		7000,					// Stack size (bytes)
		NULL,					// Task parameters
		1,						// Priority
		&xSysMonitorTaskHandle,	// Task handle
		0						// Core to run on
	);

	xTaskCreatePinnedToCore(
		sysStateManagerTask,			// Task function to be called
		"sysStateManager",				// Task name (for debug)
		20000,							// Stack size (bytes)
		NULL,							// Task parameters
		2,								// Priority
		&xSysStateManagerTaskHandle,	// Task handle
		1								// Core to run on
	);

	xTaskCreatePinnedToCore(
		commsManagerTask,			// Task function to be called
		"commsManager",				// Task name (for debug)
		10000,						// Stack size (bytes)
		NULL,						// Task parameters
		2,							// Priority
		&xCommsManagerTaskHandle,	// Task handle
		1							// Core to run on
	);
}