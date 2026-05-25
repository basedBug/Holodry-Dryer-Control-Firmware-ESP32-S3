#pragma once

#include <Arduino.h>

/*
    ESP32-S3 has recommended servo pins GPIO 1-21,35-45,47-48
    According to the code at: 
        https://github.com/madhephaestus/ESP32Servo/blob/master/examples/Multiple-Servo-Example-Arduino/Multiple-Servo-Example-Arduino.ino
*/
#define SERVO_0_PIN GPIO_NUM_6
#define SERVO_1_PIN GPIO_NUM_7

#define DS18B20_PIN GPIO_NUM_21
#define FAN_PIN GPIO_NUM_4
#define HEATER_PIN GPIO_NUM_5

#define I2C_MUX_SDA_PIN GPIO_NUM_1
#define I2C_MUX_SCL_PIN GPIO_NUM_2

/*
    It's convenient to use the UART0 (Serial0) port as it's the one that's connected to the other USB 
    connector onboard (the one with "COM"), so the physical connection is just a normal USB C cable 
    instead of dealing with bare pins trying to make another connector

    If we are using Serial0, we just need to set the baudrate, as Serial0 cannot be reassigned to
	other pins
*/
#define EXTERNAL_COMMS_UART_SERIAL_RX_PIN GPIO_NUM_44
#define EXTERNAL_COMMS_UART_SERIAL_TX_PIN GPIO_NUM_43

#define DEBUG_SERIAL_BAUD_RATE 921600 // Needs to be as fast as possible to reduce delays when printing
#define EXTERNAL_COMMS_SERIAL_BAUD_RATE 921600 // Maybe make it slower to try and reduce erros in the 
                                               // comms?

#define VENT_SERVO_OPEN_POSITION 180 // (deg)
#define VENT_SERVO_CLOSED_POSITION 0 // (deg)
#define VENTILATION_TIME_INTERVAL 10000 // ms

#define HEATER_PID_MAX_OUTPUT 100 // %
#define HEATER_PID_MIN_OUTPUT 100 // %