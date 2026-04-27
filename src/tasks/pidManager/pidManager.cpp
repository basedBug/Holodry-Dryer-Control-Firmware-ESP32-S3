#include "pidManager.h"

HeaterPidControl pidData;

AutoTunePID pidController(
    HEATER_PID_MIN_OUTPUT, 
    HEATER_PID_MAX_OUTPUT, 
    TuningMethod::TyreusLuyben  // Allegedly, "provides robust tuning with minimal overshoot, ideal for
                                // systems requiring stability"
);

void pidManagerTask(void *pvParameters)
{
	Serial.println("[pidManager] Task started");

    // Init
    initPidManager();
    
	TickType_t xLastWakeTime;

    // Aparently it has to maintain a constant sampling time, and the examples show 100ms
	const TickType_t xTimeInterval = pdMS_TO_TICKS(100);

	// Initialise the xLastWakeTime variable with the current time.
    xLastWakeTime = xTaskGetTickCount();

	/*
		Just to keep the task alive
		Cant just delete the task, the lost references would break the system
	*/
	while (true)
	{
        receiveFromTaskManager();
        sendToTaskManager();

		if (xTaskGetTickCount() - xLastWakeTime >= xTimeInterval)
		{	
			pidController.update(pidData.sensorTemp);
            analogWrite(HEATER_PIN, pidController.getOutput());

            if (pidData.performTune)
            {
                // Change operational mode to tune, this changes the controller to perform the tuning
                pidController.setOperationalMode(OperationalMode::Tune);
            }
            
            if (pidData.tuneCompleted)
            {
                // Reset flag
                pidData.tuneCompleted = false;

                // Change back the operational mode to normal for standard PID operation
                pidController.setOperationalMode(OperationalMode::Normal);
            }
				
			xLastWakeTime = xTaskGetTickCount();
		}

		vTaskDelay(pdTICKS_TO_MS(1));
	}
}

void initPidManager()
{
    Serial.println("[pidManager] Initializing heater");
    pinMode(HEATER_PIN, OUTPUT);
    Serial.println("[pidManager] Heater initalized");

    Serial.println("[pidManager] Initializing PID controller");

    /*
        Apparently, the library uses a method described as "Relay feedback", based on the work 
            "Automatic Tuning of Simple Regulators" (K.J. Åström, T. Hägglund) [https://doi.org/10.1016/S1474-6670(17)61248-5] 
        in order to bring the system into oscillation so that it's response can be analyzed, and then
        compute the PID components from there.

        The oscillation mode just adjusts how aggresive to make the oscillations during tuning.
        We have nothing to lose (the system will be empty during tuning), so PUSH IT SON!
    */
    pidController.setOscillationMode(OscillationMode::Normal);

    // Set operational mode to normal (we dont need anything special for initialization)
    pidController.setOperationalMode(OperationalMode::Normal);

    // Initialize it to 0 so that the heater starts OFF
    pidData.setpoint = 0;
    pidController.setSetpoint(pidData.setpoint);

    Serial.println("[pidManager] PID controller initialized");
}

void receiveFromTaskManager()
{
    HeaterPidControl receivedPidControlData;

    BaseType_t xReceivedDataStatus = xQueueReceive(
        xSysStateManagerToPidManagerQueue,  // Target queue handle
        &receivedPidControlData,  	        // Pointer to the buffer for the received data
        pdMS_TO_TICKS(0)                    // Max time this task should be in the Blocked state
                                            // waiting for a message, if the queue is empty
    );

    if (xReceivedDataStatus) // Valid data
    {
		pidData.setpoint = receivedPidControlData.setpoint;
        pidData.sensorTemp = receivedPidControlData.sensorTemp;
        pidData.performTune = receivedPidControlData.performTune;
    }
}

void sendToTaskManager()
{
    // Send data to PID control queue
    xQueueOverwrite(xPidManagerToSysStateManagerQueue, &pidData); // Overwrites if full
}