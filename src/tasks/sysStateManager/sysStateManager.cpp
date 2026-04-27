#include "sysStateManager.h"

SystemState systemState;
Sensors sensors;
HeaterPidControl heaterPidControl;

void sysStateManagerTask(void *pvParameters)
{
    Serial.println("[sysStateManager] Task started");

	systemState.systemStatus = SystemStatus::IDLE; // Init state to idle

    TickType_t xLastWakeTime;
	const TickType_t xTimeInterval = pdMS_TO_TICKS(10); // 10ms

	// Initialise the xLastWakeTime variable with the current time.
    xLastWakeTime = xTaskGetTickCount();

	/*
		Just to keep the task alive
		Cant just delete the task, the lost references would break the system
	*/
	while (true)
	{
		receiveFromSensorManager();
		receiveFromCommsManager();

		// This time interval may need to be removed
		if (xTaskGetTickCount() - xLastWakeTime >= xTimeInterval)
		{	
			manageSystem();

		// Send data to PID control queue
		sendToPidManager();

		vTaskDelay(pdTICKS_TO_MS(1)); // Switch context control back to the OS
	}
    
}

void receiveFromSensorManager()
{
    SystemSensors rawSensorData;

    BaseType_t xReceivedDataStatus = xQueueReceive(
        xSensorDataQueue,           // Target queue handle
        &rawSensorData,  			// Pointer to the buffer for the received data
        pdMS_TO_TICKS(0)            // Max time this task should be in the Blocked state
                                    // waiting for a message, if the queue is empty
    );

    if (xReceivedDataStatus) // Valid data
    {
		// Interior SHT readings
		if (rawSensorData.MuxSensorData[static_cast<uint8_t>(MuxSensorChannel::SHT_INTERIOR_0)].valid)
		{
			sensors.chamberSensorTemp_0 = rawSensorData.MuxSensorData[static_cast<uint8_t>(MuxSensorChannel::SHT_INTERIOR_0)].temp;
			sensors.chamberSensorHum_0 = rawSensorData.MuxSensorData[static_cast<uint8_t>(MuxSensorChannel::SHT_INTERIOR_0)].hum;
		}
		if (rawSensorData.MuxSensorData[static_cast<uint8_t>(MuxSensorChannel::SHT_INTERIOR_1)].valid)
		{
			sensors.chamberSensorTemp_1 = rawSensorData.MuxSensorData[static_cast<uint8_t>(MuxSensorChannel::SHT_INTERIOR_1)].temp;
			sensors.chamberSensorHum_1 = rawSensorData.MuxSensorData[static_cast<uint8_t>(MuxSensorChannel::SHT_INTERIOR_1)].hum;
		}
		if (rawSensorData.MuxSensorData[static_cast<uint8_t>(MuxSensorChannel::SHT_INTERIOR_2)].valid)
		{
			sensors.chamberSensorTemp_2 = rawSensorData.MuxSensorData[static_cast<uint8_t>(MuxSensorChannel::SHT_INTERIOR_2)].temp;
			sensors.chamberSensorHum_2 = rawSensorData.MuxSensorData[static_cast<uint8_t>(MuxSensorChannel::SHT_INTERIOR_2)].hum;
		}
		
		// Exterior SHT readings
		if (rawSensorData.MuxSensorData[static_cast<uint8_t>(MuxSensorChannel::SHT_EXTERIOR)].valid)
		{
			sensors.ambientSensorTemp = rawSensorData.MuxSensorData[static_cast<uint8_t>(MuxSensorChannel::SHT_EXTERIOR)].temp;
			sensors.ambientSensorHum = rawSensorData.MuxSensorData[static_cast<uint8_t>(MuxSensorChannel::SHT_EXTERIOR)].hum;
		}

		// MLX readings
		if (rawSensorData.MuxSensorData[static_cast<uint8_t>(MuxSensorChannel::MLX_0)].valid)
			sensors.filamentSurfaceTemp_0 = rawSensorData.MuxSensorData[static_cast<uint8_t>(MuxSensorChannel::MLX_0)].temp;
		if (rawSensorData.MuxSensorData[static_cast<uint8_t>(MuxSensorChannel::MLX_1)].valid)
			sensors.filamentSurfaceTemp_1 = rawSensorData.MuxSensorData[static_cast<uint8_t>(MuxSensorChannel::MLX_1)].temp;

		// DS18B20 readings
		if (rawSensorData.DS18B20.valid)
		{
			sensors.heaterSensorTemp = rawSensorData.DS18B20.temp;
		}
void receiveFromPidManager()
{
	HeaterPidControl receivedPidData;

    BaseType_t xReceivedDataStatus = xQueueReceive(
        xPidManagerToSysStateManagerQueue,	// Target queue handle
        &receivedPidData,  					// Pointer to the buffer for the received data
        pdMS_TO_TICKS(0)            		// Max time this task should be in the Blocked state
                                    		// waiting for a message, if the queue is empty
    );

    if (xReceivedDataStatus) // Valid data
    {
		// Only read useful data
		heaterPidControl.tuneCompleted = heaterPidControl.tuneCompleted;
    }
}

void sendToPidManager()
{
	xQueueOverwrite(xSysStateManagerToPidManagerQueue, &heaterPidControl); // Overwrites if full
}

void computeSysVariables()
{
	// Chamber temperature
	systemState.chamberState.maxChamberSensorTemp = max(sensors.chamberSensorTemp_0, max(sensors.chamberSensorTemp_1, sensors.chamberSensorTemp_2));
	systemState.chamberState.avgChamberSensorTemp = (sensors.chamberSensorTemp_0 + sensors.chamberSensorTemp_1 + sensors.chamberSensorTemp_2)/3.0;

	// Chamber humidity
	systemState.chamberState.maxChamberSensorHum = max(sensors.chamberSensorHum_0, max(sensors.chamberSensorHum_1, sensors.chamberSensorHum_2));
	systemState.chamberState.avgChamberSensorHum = (sensors.chamberSensorHum_0 + sensors.chamberSensorHum_1 + sensors.chamberSensorHum_2)/3.0;

	// Ambient variables
	systemState.ambientState.ambientSensorTemp = sensors.ambientSensorTemp;
	systemState.ambientState.ambientSensorHum = sensors.ambientSensorHum;

	// Filament temperature
	systemState.filamentState.maxFilamentTemp = max(sensors.filamentSurfaceTemp_0, sensors.filamentSurfaceTemp_1);
	systemState.filamentState.avgFilamentTemp = (sensors.filamentSurfaceTemp_0 + sensors.filamentSurfaceTemp_1)/2.0;

	// Absolute humidity
	systemState.exteriorAbsHum = absHumidity(
		systemState.ambientState.ambientSensorTemp, 
		systemState.ambientState.ambientSensorHum
	);
	systemState.interiorAbsHum = absHumidity(
		systemState.chamberState.avgChamberSensorTemp, 
		systemState.chamberState.avgChamberSensorHum
	);
}

void receiveFromCommsManager()
{
	
}

void manageSystem()
{
	// System state machine to change states
	switch (systemState.systemStatus)
	{
		case SystemStatus::IDLE :
		{
			// Set actuators to secure states
			// Deactivate heater
			ActuatorManager::deactivateFan();
			ActuatorManager::closeVents();
			

			// Determine if user interface was set to testing or drying mode

			// Activate drying if the master controller demands it (failure in filament detected)

			if (systemState.chamberState.avgChamberSensorHum > systemState.systemConfig.targetRelHum)
			{
				// Activate drying
				systemState.systemStatus = SystemStatus::DRYING;
				break;
			}
			
			break;
		}

		case SystemStatus::DRYING :
		{
			// Change to emergency venting if filament temp goes overboard
			if (systemState.filamentState.maxFilamentTemp > systemState.systemConfig.maxAllowedChamberTemp)
			{
				// Max allowable filament surface temp reached
				systemState.systemStatus = SystemStatus::EMERGENCY_VENTING;
				break;
			}
			// Continue drying
			
			// Set target heater temp (if not already set/if deactivated)
			// Activate fan (to spread heat), may not be needed due the integrated fan on the heater
			ActuatorManager::activateFan();
			
			// Change to venting if theres a chance of drying the air
			if (systemState.interiorAbsHum > systemState.exteriorAbsHum)
			{
				// Internal humidity > external, drying is possible by venting the chamber
				systemState.systemStatus = SystemStatus::VENTING;
			}
				
			break;
		}

		case SystemStatus::VENTING :
		{
			// Deactivate heater (would rather not waste power by unneccesary heating)
			// Open cooling vent
			ActuatorManager::openVents();
			// Activate fan
			ActuatorManager::activateFan();

			// Wait to purge air (calculate or just a simple timer?)

			// Deactivate fan
			ActuatorManager::deactivateFan();
			// Close cooling vent
			ActuatorManager::closeVents();

			// Change mode back to drying
			systemState.systemStatus = SystemStatus::DRYING;

			break;
		}

		case SystemStatus::EMERGENCY_VENTING :
		{
			// Deactivate heater
			// Activate fan
			ActuatorManager::activateFan();
			// Open cooling vent
			ActuatorManager::openVents();
			if (systemState.filamentState.maxFilamentTemp <= systemState.systemConfig.maxAllowedChamberTemp)
			{
				// Deactivate fan
				ActuatorManager::deactivateFan();
				// Heater must not be reactivated from here, it will be handled by the other modes
				// Close cooling vent
				ActuatorManager::closeVents();
				
				// Change mode back to drying
				systemState.systemStatus = SystemStatus::DRYING;
				
				break;
			}
				
			break;
		}

		case SystemStatus::TESTING_MODE :
		{
			// Manual actuator controls
			// Activate heater per user insctructions
			// Activate fan per user instructions
			// Toggle vent per user instructions

			break;
		}

		case SystemStatus::SHOWCASE_MODE :
		{
			// Maybe feed simulated sensor readings to showcase usage quickly

			break;
		}
		
		default:
		{
			// Some undefined state tried to be invoked, default it to idle to avoid problems
			systemState.systemStatus = SystemStatus::IDLE;
			break;
		}
	}
	


}

void sendToCommsManager()
{
SystemStatus sysStatusStrTosysStatus(const char* str)
{
	if (strcmp(str, "IDLE") == 0)
		return SystemStatus::IDLE ;
	if (strcmp(str, "TUNING") == 0)
		return SystemStatus::TUNING ;
	if (strcmp(str, "DRYING") == 0)
		return SystemStatus::DRYING ;
	if (strcmp(str, "VENTING") == 0)
		return SystemStatus::VENTING ;
	if (strcmp(str, "EMERGENCY_VENTING") == 0)
		return SystemStatus::EMERGENCY_VENTING ;
	if (strcmp(str, "TESTING_MODE") == 0)
		return SystemStatus::TESTING_MODE ;
	if (strcmp(str, "SHOWCASE_MODE") == 0)
		return SystemStatus::SHOWCASE_MODE ;
	return SystemStatus::UNKNOWN ; // Default return value if no match is found
}

const char* sysStatusToSysStatusStr(SystemStatus status)
{
	switch (status)
	{
		case SystemStatus::IDLE : return "IDLE";
		case SystemStatus::TUNING : return "TUNING";
		case SystemStatus::DRYING : return "DRYING";
		case SystemStatus::VENTING : return "VENTING";
		case SystemStatus::EMERGENCY_VENTING : return "EMERGENCY_VENTING";
		case SystemStatus::TESTING_MODE : return "TESTING_MODE";
		case SystemStatus::SHOWCASE_MODE : return "SHOWCASE_MODE";
        default: return "UNKNOWN";
	}
}