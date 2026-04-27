#pragma once

#include <Arduino.h>

/*
    ESP32-S3 has recommended servo pins GPIO 1-21,35-45,47-48
    According to the code at: 
        https://github.com/madhephaestus/ESP32Servo/blob/master/examples/Multiple-Servo-Example-Arduino/Multiple-Servo-Example-Arduino.ino
*/
#define SERVO_0_PIN GPIO_NUM_17
#define SERVO_1_PIN GPIO_NUM_18

#define DS18B20_PIN GPIO_NUM_36
#define FAN_PIN GPIO_NUM_37
#define HEATER_PIN GPIO_NUM_38

#define I2C_MASTER_SDA_PIN GPIO_NUM_1
#define I2C_MASTER_SCL_PIN GPIO_NUM_2

#define I2C_MUX_SDA_PIN GPIO_NUM_4
#define I2C_MUX_SCL_PIN GPIO_NUM_5

#define SERIAL_BAUD_RATE 921600
#define VENT_SERVO_OPEN_POSITION 180 // (deg)
#define VENT_SERVO_CLOSED_POSITION 0 // (deg)