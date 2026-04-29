#include "sysStateManager.h"

SystemState systemState;
SystemStatus lastSystemStatus;
bool showcaseMode = false;

Sensors sensors;
HeaterPidControl heaterPidControl;

// Default ventilation time interval
TickType_t xVentilationTimeInterval = pdMS_TO_TICKS(VENTILATION_TIME_INTERVAL);

TickType_t xLastWakeTime; // May not be needed

void sysStateManagerTask(void *pvParameters)
{
    Serial.println("[sysStateManager] Task started");

	// Change to actuators init
	while(!Actuators::initActuators())
    {
        // Try to reinitialize
        Serial.println("[sysStateManager] Error: Actuators initialization failed");
        Serial.println("[sysStateManager] Trying reinitialization");
    }

	// Init system state to idle
	systemState.systemStatus = SystemStatus::IDLE;

	const TickType_t xTimeInterval = pdMS_TO_TICKS(10); // 10ms

	// Initialise the xLastWakeTime variable with the current time.
    xLastWakeTime = xTaskGetTickCount();

	/*
		Just to keep the task alive
		Cant just delete the task, the lost references would break the system
	*/
	while (true)
	{
		/*
			Dismis real sensor readings in SHOWCASE_MODE, as we use simulated sensor values sent from
			the master controller (Raspberry Pi 4)
		*/
		if (!showcaseMode)
		{
			receiveFromSensorManager();
		}

		// Compute values based on received sensor readings
		computeSysVariables();

		receiveFromCommsManager();

		manageSystem();

		// Send data to PID control queue
		sendToPidManager();

		sendDataToCommsManager();

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
    }
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

	// Actuators state (only heater, all others are updated at actuators.cpp)
	systemState.actuatorsStatus.heaterTemp = sensors.heaterSensorTemp;

	// Absolute humidity
	systemState.exteriorAbsHum = Thermodynamics::absHumidity(
		systemState.ambientState.ambientSensorTemp, 
		systemState.ambientState.ambientSensorHum
	);
	systemState.interiorAbsHum = Thermodynamics::absHumidity(
		systemState.chamberState.avgChamberSensorTemp, 
		systemState.chamberState.avgChamberSensorHum
	);
}

void manageSystem()
{
	// May need to also track last systemStatus
	registerSysStatusChange(systemState.systemStatus);
	switch (systemState.systemStatus)
	{
		// Maybe implement a first time drying pass to detect humidity
		case SystemStatus::IDLE :
		{
			/*
				Main/default system state
					Only changes to drying mode if: 
						- The chamber relative humidity is high enough that the air is saturated enough 
						  (based on user input). 
						- OR if the master controller demands it (failure in filament detected), at 
						  which point it will just force the system state to drying
			*/

			// Set actuators to secure states
			heaterPidControl.setpoint = 0; // Turn off the heater
			Actuators::deactivateFan();
			Actuators::closeVents();

			/*
				Start drying if the relative humidity is above target (no use in drying if the 
				humidity is good enough for the user)
			*/
			if (systemState.chamberState.avgChamberSensorHum > systemState.systemConfig.targetRelHum)
			{
				// Activate drying
				systemState.systemStatus = SystemStatus::DRYING ;
				break;
			}
			
			break;
		}

		case SystemStatus::TUNING :
		{
			/*
				Only tunes the pid controller, nothing else
			*/

			if (lastSystemStatus != SystemStatus::TUNING)
			{
				// Set setpoint again in here just to make sure it has it before starting the tuning
				heaterPidControl.setpoint = systemState.systemConfig.targetChamberTemp;
				heaterPidControl.performTune = true;
			}
			
			if (heaterPidControl.tuneCompleted)
			{
				// Reset the flag to be able to call tuning mode again
				heaterPidControl.tuneCompleted = false;

				if (showcaseMode)
				{
					// If tuning is done, go back SHOWCASE_MODE
					systemState.systemStatus = SystemStatus::SHOWCASE_MODE ;
					break;
				}

				// If tuning is done, go back to idle
				systemState.systemStatus = SystemStatus::IDLE ;

				break;
			}

			break;
		}

		case SystemStatus::DRYING :
		{
			/*
				Do the drying process
			*/

			// Change to emergency venting if filament temp goes overboard
			if (systemState.filamentState.maxFilamentTemp > systemState.systemConfig.maxAllowedFilamentTemp)
			{
				// Max allowable filament surface temp reached
				systemState.systemStatus = SystemStatus::EMERGENCY_VENTING ;
				break;
			}
			
			// Set target heater temp (if not already set/if deactivated)
			heaterPidControl.setpoint = systemState.systemConfig.targetChamberTemp;

			// Activate fan (to spread heat), may not be needed due the integrated fan on the heater
			Actuators::activateFan();
			
			// Change to venting if theres a chance of drying the air
			if (systemState.interiorAbsHum > systemState.exteriorAbsHum)
			{
				// Internal humidity > external, drying is possible by venting the chamber
				systemState.systemStatus = SystemStatus::VENTING ;
				break;
			}

			/*
				Stop drying if the relative humidity is below or at target (no use in drying if the 
				humidity is good enough for the user)
			*/
			if (systemState.chamberState.avgChamberSensorHum <= systemState.systemConfig.targetRelHum)
			{
				// Stop drying

				/*
					If the SHOWCASE_MODE was the last one, switch back to it after drying
				*/
				if (showcaseMode)
				{
					systemState.systemStatus = SystemStatus::SHOWCASE_MODE ;
					break;
				}

				systemState.systemStatus = SystemStatus::DRYING ;
				break;
			}
				
			break;
		}

		case SystemStatus::VENTING :
		{
			/*
				Exchange air with the ambient to dry the interior
			*/

			// Turn off the heater (would rather not waste power by unneccesary heating)
			heaterPidControl.setpoint = 0; 

			// Open cooling vent
			Actuators::openVents();
			// Activate fan
			Actuators::activateFan();

			// Wait to purge air (calculate or just a simple timer?)
			if (xTaskGetTickCount() - xLastWakeTime >= xVentilationTimeInterval)
			{	
				// Deactivate fan
				Actuators::deactivateFan();
				// Close cooling vent
				Actuators::closeVents();

				// Change mode back to drying
				systemState.systemStatus = SystemStatus::DRYING ;

				xLastWakeTime = xTaskGetTickCount();
				break;
			}

			break;
		}

		case SystemStatus::EMERGENCY_VENTING :
		{
			/*
				Force air venting to cool down the filament temperature when it reaches dangerous 
				levels
			*/

			// Deactivate heater
			heaterPidControl.setpoint = 0;

			// Activate fan
			Actuators::activateFan();
			// Open cooling vent
			Actuators::openVents();

			if (systemState.filamentState.maxFilamentTemp <= systemState.systemConfig.maxAllowedFilamentTemp)
			{
				// Deactivate fan
				Actuators::deactivateFan();
				// Heater must not be reactivated from here, it will be handled by the other modes
				// Close cooling vent
				Actuators::closeVents();
				

				if (showcaseMode)
				{
					// If tuning is done, go back SHOWCASE_MODE
					systemState.systemStatus = SystemStatus::SHOWCASE_MODE ;
					break;
				}

				// Change mode back to drying
				systemState.systemStatus = SystemStatus::DRYING ;
				
				break;
			}
				
			break;
		}

		case SystemStatus::MANUAL_MODE :
		{
			/*
				Manual actuator controls
					Activate heater per json cmd
					Activate fan per json cmd
					Toggle vent per json cmd
				In essence, do nothing but respond to json cmds

				WONT CHANGE TO OTHER MODES BY ITSELF !!!
			*/

			// We do NOTHING, NOTHING EVER HAPPENS

			break;
		}

		case SystemStatus::SHOWCASE_MODE :
		{
			/*
				Handle only simulated sensor readings to showcase usage quickly

				Does the same thing as IDLE
			*/

			/*
				The reception of real sensor readings is deactivated

				The simulated ones for the SHOWCASE_MODE are sent by the master controller (Rasperry Pi 4) through json
				an are handled directly at json reception
			*/

			// Set actuators to secure states
			heaterPidControl.setpoint = 0; // Turn off the heater
			Actuators::deactivateFan();
			Actuators::closeVents();

			/*
				Start drying if the relative humidity is above target (no use in drying if the 
				humidity is good enough for the user)
			*/
			if (systemState.chamberState.avgChamberSensorHum > systemState.systemConfig.targetRelHum)
			{
				// Activate drying
				systemState.systemStatus = SystemStatus::DRYING ;
				break;
			}

			break;
		}
		
		default:
		{
			// Some undefined state tried to be invoked, default it to idle to avoid problems
			Serial.printf(
				"[sysStateManager] Error: system status tried to be changed from %s to UNKNOWN, defaulting to IDLE \n",
				sysStatusToSysStatusStr(lastSystemStatus)
			);
			lastSystemStatus = SystemStatus::UNKNOWN ;
			systemState.systemStatus = SystemStatus::IDLE ;

			break;
		}
	}
	


}

void registerSysStatusChange(SystemStatus status)
{
	if (lastSystemStatus != status)
	{
		Serial.printf(
			"[sysStateManager] System status changed to %s \n", 
			sysStatusToSysStatusStr(status)
		);
		lastSystemStatus = status;
	}
}

void getSysStateData(JsonObject &payload)
{
	// Main object
	JsonObject _systemState = payload["systemState"].to<JsonObject>();

		JsonObject _systemStatus = _systemState["status"].to<JsonObject>();
			_systemStatus["mainStatus"] = sysStatusToSysStatusStr(systemState.systemStatus);
			_systemStatus["exteriorAbsHum"] = systemState.exteriorAbsHum;
			_systemStatus["interiorAbsHum"] = systemState.interiorAbsHum;
			
		JsonObject _actuatorStatus = _systemState["actuatorStatus"].to<JsonObject>();
			_systemStatus["heaterSetpoint"] = heaterPidControl.setpoint; // Maybe indicate if its tuning
			_systemStatus["fan"] = Actuators::fanStatusToFanStatusStr(systemState.actuatorsStatus.fanStatus);
			_systemStatus["vents"] = Actuators::ventsStatusToVentsStatusStr(systemState.actuatorsStatus.ventsStatus);

		JsonObject _systemConfig = _systemState["systemConfig"].to<JsonObject>();
			_systemConfig["targetChamberTemp"] = systemState.systemConfig.targetChamberTemp;
			_systemConfig["maxAllowedFilamentTemp"] = systemState.systemConfig.maxAllowedFilamentTemp;
			_systemConfig["targetRelHum"] = systemState.systemConfig.targetRelHum;
		
		JsonObject _chamberState = _systemState["chamberState"].to<JsonObject>();
			JsonObject _chamberTemp = _chamberState["chamberTemp"].to<JsonObject>();
				_chamberTemp["max"] = systemState.chamberState.maxChamberSensorTemp;
				_chamberTemp["avg"] = systemState.chamberState.avgChamberSensorTemp;
			JsonObject _chamberHum = _chamberState["chamberHum"].to<JsonObject>();
				_chamberHum["max"] = systemState.chamberState.maxChamberSensorHum;
				_chamberHum["avg"] = systemState.chamberState.avgChamberSensorHum;

		JsonObject _ambientState = _systemState["ambientState"].to<JsonObject>();
			_ambientState["temp"] = systemState.ambientState.ambientSensorTemp;
			_ambientState["hum"] = systemState.ambientState.ambientSensorHum;

		JsonObject _filamentState = _systemState["filamentState"].to<JsonObject>();
			JsonObject _filamentTemp = _filamentState["filamentTemp"].to<JsonObject>();
				_filamentTemp["max"] = systemState.filamentState.maxFilamentTemp;
				_filamentTemp["avg"] = systemState.filamentState.avgFilamentTemp;	
}

void getSensorData(JsonObject &payload)
{
	JsonObject _rawSensorTelemetry = payload["rawSensorTelemetry"].to<JsonObject>();

		JsonObject _chamber = _rawSensorTelemetry["chamber"].to<JsonObject>();
			_chamber["sht31_0_temp"] = sensors.chamberSensorTemp_0;
			_chamber["sht31_0_hum"] = sensors.chamberSensorHum_0;
			_chamber["sht31_1_temp"] = sensors.chamberSensorTemp_1;
			_chamber["sht31_1_hum"] = sensors.chamberSensorHum_1;
			_chamber["sht31_2_temp"] = sensors.chamberSensorTemp_2;
			_chamber["sht31_2_hum"] = sensors.chamberSensorHum_2;
			_chamber["ds18b20_temp"] = sensors.heaterSensorTemp;

		JsonObject _ambient = _rawSensorTelemetry["ambient"].to<JsonObject>();
			_chamber["sht31_3_temp"] = sensors.ambientSensorTemp;
			_chamber["sht31_3_hum"] = sensors.ambientSensorHum;

		JsonObject _filament = _rawSensorTelemetry["filament"].to<JsonObject>();
			_filament["mlx90614_0_temp"] = sensors.filamentSurfaceTemp_0;
			_filament["mlx90614_1_temp"] = sensors.filamentSurfaceTemp_1;

}

void loadDataToSend(JsonObject &payload)
{
	/*
		Load data into payload directly (any modifications made to the JSON object that references 
		the doc are reflected into the original doc)
	*/
	getSysStateData(payload);
	getSensorData(payload);
}

void handleReceivedCmds(JsonDocument& cmdDoc)
{
	if (cmdDoc["systemControl"].is<JsonObject>())
	{
		JsonObject systemControl = cmdDoc["systemControl"].as<JsonObject>();

		if (systemControl["systemStatus"].is<JsonString>())
		{
			const char* incomingSystemStatusStr = systemControl["systemStatus"];
			SystemStatus incomingSystemStatus = sysStatusStrTosysStatus(incomingSystemStatusStr);

			// Check for invalid input
			if (incomingSystemStatus != SystemStatus::UNKNOWN)
			{
				systemState.systemStatus = incomingSystemStatus; // Change system status per cmd
			}
		}

		if (systemControl["targetChamberTemp"].is<JsonFloat>())
		{
			float incomingTargetChamberTemp = systemControl["targetChamberTemp"];

			// Check for invalid input
			if (incomingTargetChamberTemp >= 0)
			{
				// Change chamber target temperature per cmd
				systemState.systemConfig.targetChamberTemp = incomingTargetChamberTemp; 
			}
		}

		if (systemControl["maxAllowedFilamentTemp"].is<JsonFloat>())
		{
			float incomingMaxAllowedFilamentTemp = systemControl["maxAllowedFilamentTemp"];

			// Check for invalid input
			if (incomingMaxAllowedFilamentTemp >= 0)
			{
				// Change max allowed chamber temperature per cmd
				systemState.systemConfig.maxAllowedFilamentTemp = incomingMaxAllowedFilamentTemp; 
			}
		}

		if (systemControl["targetRelHum"].is<JsonFloat>())
		{
			float incomingTargetRelHum = systemControl["targetRelHum"];

			// Check for invalid input
			if (incomingTargetRelHum >= 0)
			{
				// Change target relative humidity per cmd
				systemState.systemConfig.targetRelHum = incomingTargetRelHum; 
			}
		}

		if (systemControl["actuators"].is<JsonObject>())
		{
			JsonObject actuatorControl = systemControl["actuators"].as<JsonObject>();

			if (actuatorControl["heaterSetpointTemp"].is<JsonFloat>())
			{
				float incomingHeaterSetpointTemp = actuatorControl["heaterSetpointTemp"];

				// Check for invalid input
				if (incomingHeaterSetpointTemp >= 0)
				{
					// Change mode as we are manipulating the actuators directly
					systemState.systemStatus = SystemStatus::MANUAL_MODE ;

					// Change heater temperature setpoint per cmd
					heaterPidControl.setpoint = incomingHeaterSetpointTemp;
				}
			}

			if (actuatorControl["fan"].is<JsonString>())
			{
				const char* incomingFanStatusStr = actuatorControl["fan"];
				Actuators::FanStatus incomingFanStatus = Actuators::fanStatusStrToFanStatus(incomingFanStatusStr);

				// Check for invalid input
				if (incomingFanStatus != Actuators::FanStatus::UNKNOWN)
				{
					// Change mode as we are manipulating the actuators directly
					systemState.systemStatus = SystemStatus::MANUAL_MODE ;

					// Actuate fan based on cmd
					if (incomingFanStatus == Actuators::FanStatus::ON)
					{
						Actuators::activateFan();
					}
					else
					{
						Actuators::deactivateFan();
					}
				}
			}

			if (actuatorControl["vents"].is<JsonString>())
			{
				const char* incomingVentsStatusStr = actuatorControl["vents"];
				Actuators::VentsStatus incomingVentsStatus = Actuators::ventsStatusStrToVentsStatus(incomingVentsStatusStr);

				// Check for invalid input
				if (incomingVentsStatus != Actuators::VentsStatus::UNKNOWN)
				{
					// Change mode as we are manipulating the actuators directly
					systemState.systemStatus = SystemStatus::MANUAL_MODE ;

					// Actuate fan based on cmd
					if (incomingVentsStatus == Actuators::VentsStatus::OPEN)
					{
						Actuators::openVents();
					}
					else
					{
						Actuators::closeVents();
					}
				}
			}
		}
	}
		
	if (cmdDoc["showcaseControl"].is<JsonObject>())
	{
		JsonObject showcaseControl = cmdDoc["showcaseControl"].as<JsonObject>();

		if (showcaseControl["showcaseMode"].is<bool>())
		{
			showcaseMode = showcaseControl["showcaseMode"];
		}
		

		if (showcaseControl["simulatedSensors"].is<JsonObject>())
		{
			JsonObject simulatedSensors = showcaseControl["simulatedSensors"].as<JsonObject>();

			// Change mode as we are feeding simulated sensor data for the SHOWCASE_MODE
			showcaseMode = true;
			systemState.systemStatus = SystemStatus::SHOWCASE_MODE ;

			if (simulatedSensors["chamberSensorTemp_0"].is<JsonFloat>())
				sensors.chamberSensorTemp_0 = simulatedSensors["chamberSensorTemp_0"];
			if (simulatedSensors["chamberSensorTemp_1"].is<JsonFloat>())
				sensors.chamberSensorTemp_1 = simulatedSensors["chamberSensorTemp_1"];
			if (simulatedSensors["chamberSensorTemp_2"].is<JsonFloat>())
				sensors.chamberSensorTemp_1 = simulatedSensors["chamberSensorTemp_1"];

			if (simulatedSensors["chamberSensorHum_0"].is<JsonFloat>())
				sensors.chamberSensorHum_0 = simulatedSensors["chamberSensorHum_0"];
			if (simulatedSensors["chamberSensorHum_1"].is<JsonFloat>())
				sensors.chamberSensorHum_1 = simulatedSensors["chamberSensorHum_1"];
			if (simulatedSensors["chamberSensorHum_2"].is<JsonFloat>())
				sensors.chamberSensorHum_2 = simulatedSensors["chamberSensorHum_2"];

			if (simulatedSensors["ambientSensorTemp"].is<JsonFloat>())
				sensors.ambientSensorTemp = simulatedSensors["ambientSensorTemp"];
			if (simulatedSensors["ambientSensorHum"].is<JsonFloat>())
				sensors.ambientSensorHum = simulatedSensors["ambientSensorHum"];

			if (simulatedSensors["filamentSurfaceTemp_0"].is<JsonFloat>())
				sensors.filamentSurfaceTemp_0 = simulatedSensors["filamentSurfaceTemp_0"];
			if (simulatedSensors["filamentSurfaceTemp_1"].is<JsonFloat>())
				sensors.filamentSurfaceTemp_1 = simulatedSensors["filamentSurfaceTemp_1"];

			if (simulatedSensors["heaterSensorTemp"].is<JsonFloat>())
				sensors.heaterSensorTemp = simulatedSensors["heaterSensorTemp"];
			
		}
	}
}

void receiveFromCommsManager()
{
	static char rxJsonMsgBuffer[MAX_MSG_SIZE];

	size_t receivedBytes = xMessageBufferReceive(
		xCommsManagerToSysStateManagerMsgBuffer,	// Target message buffer handle
		rxJsonMsgBuffer,							// Pointer to the buffer for the received message
		sizeof(rxJsonMsgBuffer), 					// Length of the buffer for the received message
		pdMS_TO_TICKS(0)							// Max time this task should be in the Blocked state
													// waiting for a message, if there buffer is empty
	);
	
	if (receivedBytes > 0)
	{
		// Parse and process the JSON
		JsonDocument rx_doc;
		
		DeserializationError error = deserializeJson(rx_doc, rxJsonMsgBuffer, receivedBytes);
		if (error)
		{
			Serial.printf("[Web] JSON parse error: %s \n", error.c_str());
			return;
		}
		
		// Print contents into serial
		//JsonHandlers::printJsonContents(rx_doc);
		
		// Handle incoming JSON data
		handleReceivedCmds(rx_doc);
	}
}

void sendDataToCommsManager()
{
	/*
		Any modifications made to the JSON object that references the doc
		are reflected into the original doc
	*/
	JsonDocument tx_doc;
	JsonObject tx_data = tx_doc.to<JsonObject>();
	
	// Load up the data
	loadDataToSend(tx_data);
	
	sendToCommsManager(tx_doc);

	// Print sent contents into serial
	//JsonHandlers::printJsonContents(tx_doc);
}

bool sendToCommsManager(JsonDocument &doc)
{
	// Maybe (FUTURE) replace the buffer with thread-safe allocation?
	static char txJsonMsgBuffer[MAX_MSG_SIZE];
	const size_t len = measureJson(doc);
	if (len == 0) 
	{
		Serial.println("[sysStateManager] Warning: Tried to send JSON message of size 0");
		return false;
	}
	if (len > sizeof(txJsonMsgBuffer))
	{
		Serial.printf("[sysStateManager] Warning: JSON message %u bigger than message buffer %u, dropping JSON \n",
			len, 
			sizeof(txJsonMsgBuffer)
		);
		return false;
	}

	serializeJson(doc, txJsonMsgBuffer, len);

	size_t sentBytes = xMessageBufferSend(
		xSysStateManagerToCommsManagerMsgBuffer,	// Target message buffer handle
		txJsonMsgBuffer,							// Pointer to data being sent
		len, 										// Length of the message
		pdMS_TO_TICKS(0)							// Max time this task should be the in Blocked state
													// for enough space in the buffer, if there's 
													// insufficient space when the call is made
	);

	if (sentBytes != len) {
		Serial.println("[sysStateManager] Warning: Message buffer to commsManager full, message dropped");
		return false;
	}

	Serial.printf("[sysStateManager] Sent JSON message of size: %u \n", sentBytes);
	return true;
}

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
	if (strcmp(str, "MANUAL_MODE") == 0)
		return SystemStatus::MANUAL_MODE ;
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
		case SystemStatus::MANUAL_MODE : return "MANUAL_MODE";
		case SystemStatus::SHOWCASE_MODE : return "SHOWCASE_MODE";
        default: return "UNKNOWN";
	}
}