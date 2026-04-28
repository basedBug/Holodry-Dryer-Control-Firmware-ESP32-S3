#include "sensorManager.h"

// I2C mux
TCA9548 i2cMux(MUX_I2C_ADDRESS, &Wire);
uint8_t i2cMuxChannels;

// Sensors connected to I2C mux
Adafruit_SHT31 shtSensor = Adafruit_SHT31(&Wire);
Adafruit_MLX90614 mlxSensor = Adafruit_MLX90614(); // Apparently this constructor doesnt take the I2C
                                                   // instance at construction (it does at begin())

// DS18B20
OneWire oneWire(DS18B20_PIN);
DallasTemperature ds18b20(&oneWire);

SystemSensors systemSensors;

void sensorManagerTask(void *pvParameters)
{
    Serial.println("[sensorManager] Task started");

    while(!initSensorManager())
    {
        // Try to reinitialize
        Serial.println("[sensorManager] Error: Comms initialization failed");
        Serial.println("[sensorManager] Trying reinitialization");
    }

    TickType_t xLastWakeTime;
	const TickType_t xTimeInterval = pdMS_TO_TICKS(100); // 100ms

	// Initialise the xLastWakeTime variable with the current time.
    xLastWakeTime = xTaskGetTickCount();

	/*
		Just to keep the task alive
		Cant just delete the task, the lost references would break the system
	*/
	while (true)
	{
		if (xTaskGetTickCount() - xLastWakeTime >= xTimeInterval)
		{	
			readSensors();
            
            // Send data to queue
            xQueueOverwrite(xSensorDataQueue, &systemSensors); // Overwrites if full
				
			xLastWakeTime = xTaskGetTickCount();
		}

		vTaskDelay(pdTICKS_TO_MS(1)); // Switch context control back to the OS
	}
    
}

bool initSensorManager()
{
    Serial.println("[sensorManager] Initializing comms");

    Serial.println("[sensorManager] Initializing I2C comms channel");
    if(!Wire.begin(I2C_MUX_SDA_PIN, I2C_MUX_SCL_PIN))
    {
        Serial.println("[sensorManager] Error: I2C comms channel initialization failed");
        return false;
    }

    // Initialize I2C mux
    Serial.println("[sensorManager] Initializing I2C mux");
    if(!i2cMux.begin())
    {
        Serial.println("[sensorManager] Error: I2C Multiplexer initialization failed");
        return false;
    }
    Serial.println("[sensorManager] I2C Multiplexer initializated");
    i2cMuxChannels = static_cast<uint8_t>(MuxSensorChannel::OVER_LAST_DEFINED_CHANNEL) - 1;

    // Initialize SHT sensors
    Serial.println("[sensorManager] Initializing SHT sensors");
    i2cMux.selectChannel(static_cast<uint8_t>(MuxSensorChannel::SHT_EXTERIOR)); // Must set the mux 
                                                                                // channel on some SHT
                                                                                // channel before trying 
                                                                                // to setup the SHT's
    if(!shtSensor.begin(SHT_I2C_ADDRESS))
    {
        Serial.println("[sensorManager] Error: SHT initialization failed");
        return false;
    }
    Serial.println("[sensorManager] SHT sensors initialized");

    // Initialize MLX sensors
    Serial.println("[sensorManager] Initializing MLX sensors");
    i2cMux.selectChannel(static_cast<uint8_t>(MuxSensorChannel::MLX_0)); // Must set the mux channel on 
                                                                         // some MLX channel before trying
                                                                         // to setup the MLX's
    if(!mlxSensor.begin(MLX_I2C_ADDRESS, &Wire))
    {
        Serial.println("[sensorManager] Error: MLX initialization failed");
        return false;
    }
    Serial.println("[sensorManager] MLX sensors initialized");
    
    // Initialize DS18B20 sensor
    Serial.println("[sensorManager] Initializing DS18B20 sensor");
    ds18b20.begin(); // Doesnt have failure detection
    Serial.println("[sensorManager] DS18B20 sensor initialized");

    Serial.println("[sensorManager] Sensors initialized");
    return true;
}

void readSensors()
{
    readMuxSensors();

    // Other sensors
    systemSensors.DS18B20.hum = NAN; // Humidity is unused on the DS18B20 sensors
    systemSensors.DS18B20.valid = false;
    ds18b20.requestTemperatures();
    float sensorTemp = ds18b20.getTempCByIndex(SINGLE_DS18B20_SENSOR_INDEX);
    if(sensorTemp != DEVICE_DISCONNECTED_C)
    {
        systemSensors.DS18B20.temp = sensorTemp;
        systemSensors.DS18B20.valid = true;
    }
    else
    {
        Serial.println("[sensorManager] Error: Failed to read DS18B20 sensor");
    }
}

void readMuxSensors()
{
    for(uint8_t muxChannelIndex = 0; muxChannelIndex < i2cMuxChannels; muxChannelIndex++)
    {
        float sensorTemp, sensorHum;
        systemSensors.MuxSensorData[muxChannelIndex].valid = false; // Have to assume data is invalid unless specified
        
        i2cMux.selectChannel(muxChannelIndex);
        switch(muxChannelIndex)
        {
            // SHT sensors
            case static_cast<uint8_t>(MuxSensorChannel::SHT_EXTERIOR):
            case static_cast<uint8_t>(MuxSensorChannel::SHT_INTERIOR_0):
            case static_cast<uint8_t>(MuxSensorChannel::SHT_INTERIOR_1):
            case static_cast<uint8_t>(MuxSensorChannel::SHT_INTERIOR_2):
            {
                if(!shtSensor.readBoth(&sensorTemp, &sensorHum))
                {
                    Serial.printf(
                        "[sensorManager] Error: Failed to read SHT sensor on mux channel %d (%s)", 
                        muxChannelIndex,
                        muxChannelName(muxChannelIndex)
                    );
                }
                else
                {
                    systemSensors.MuxSensorData[muxChannelIndex].temp = sensorTemp;
                    systemSensors.MuxSensorData[muxChannelIndex].hum = sensorHum;
                    systemSensors.MuxSensorData[muxChannelIndex].valid = true;
                }
                break;
            }

            // MLX sensors
            case static_cast<uint8_t>(MuxSensorChannel::MLX_0):
            case static_cast<uint8_t>(MuxSensorChannel::MLX_1):
            {
                systemSensors.MuxSensorData[muxChannelIndex].hum = NAN; // Humidity is unused on the MLX sensors
                sensorTemp = mlxSensor.readObjectTempC();
                if(isnan(sensorTemp))
                {
                    Serial.printf(
                        "[sensorManager] Error: Failed to read MLX sensor on mux channel %d (%s)",
                        muxChannelIndex,
                        muxChannelName(muxChannelIndex)
                    );
                }
                else
                {
                    systemSensors.MuxSensorData[muxChannelIndex].temp = sensorTemp;
                    systemSensors.MuxSensorData[muxChannelIndex].valid = true;
                }
                break;
            }

            // Undefined
            default:
                    Serial.printf(
                        "[sensorManager] Error: Tried to access undefined i2c mux channel %d (%s)",
                        muxChannelIndex,
                        muxChannelName(muxChannelIndex)
                    );
                break;
        }
    }
}

const char* muxChannelName(uint8_t channel)
{
    switch(channel)
    {
        case static_cast<uint8_t>(MuxSensorChannel::SHT_EXTERIOR): return "SHT_EXTERIOR";
        case static_cast<uint8_t>(MuxSensorChannel::SHT_INTERIOR_0): return "SHT_INTERIOR_0";
        case static_cast<uint8_t>(MuxSensorChannel::SHT_INTERIOR_1): return "SHT_INTERIOR_1";
        case static_cast<uint8_t>(MuxSensorChannel::SHT_INTERIOR_2): return "SHT_INTERIOR_2";
        case static_cast<uint8_t>(MuxSensorChannel::MLX_0): return "MLX_0";
        case static_cast<uint8_t>(MuxSensorChannel::MLX_1): return "MLX_1";
        default: return "UNKNOWN";
    }
}