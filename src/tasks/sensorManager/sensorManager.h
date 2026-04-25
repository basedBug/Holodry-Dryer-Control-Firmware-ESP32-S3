#pragma once

#include "globals/globals.h"
#include "config.h"
#include <Arduino.h>
#include <Wire.h>
#include <TCA9548.h>
#include <Adafruit_SHT31.h>
#include <Adafruit_MLX90614.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define MUX_I2C_ADDRESS   0x70
#define SHT_I2C_ADDRESS   0x44
#define MLX_I2C_ADDRESS   0x5A

#define MUX_I2C_MAX_CHANNELS 8
#define SINGLE_DS18B20_SENSOR_INDEX 0

enum class MuxSensorChannel : uint8_t 
{
    SHT_EXTERIOR,
    SHT_INTERIOR_0,
    SHT_INTERIOR_1,
    SHT_INTERIOR_2,
    MLX_0,
    MLX_1,
    OVER_LAST_DEFINED_CHANNEL
};

struct SensorData
{
    float temp;
    float hum;        // NAN for sensors that don't measure humidity
    bool valid;       // true when the read was successful
};

struct SystemSensors
{
    SensorData MuxSensorData[MUX_I2C_MAX_CHANNELS];
    SensorData DS18B20;
};

void sensorManagerTask(void *pvParameters);
bool initSensorManager();
void readSensors();
void readMuxSensors();

const char* muxChannelName(uint8_t channel);