#include "actuators.h"

Servo ventServo_0, ventServo_1;

// Initialize actuators, except for heater (it will be done by the pidManager task)
bool Actuators::initActuators()
{
    Serial.println("[sysStateManager] Initializing actuators");

    Serial.println("[sysStateManager] Initializing fan");
    pinMode(FAN_PIN, OUTPUT);
    Serial.println("[sysStateManager] Fan initalized");

    /*
        Servo configuration defaults to 50Hz period (standard)
        No extra configuration needed
    */
    Serial.println("[sysStateManager] Initializing ventServo_0");
    if (!ventServo_0.attach(SERVO_0_PIN))
    {
        Serial.println("[sysStateManager] Error: ventServo_0 initalization failed");
        return false;
    }
    Serial.println("[sysStateManager] VentServo_0 initialized");

    Serial.println("[sysStateManager] Initializing ventServo_1");
    if (!ventServo_1.attach(SERVO_1_PIN))
    {
        Serial.println("[sysStateManager] Error: ventServo_1 initalization failed");
        return false;
    }
    Serial.println("[sysStateManager] VentServo_1 initialized");

    // Set default states
    deactivateFan();
    closeVents();

    Serial.println("[sysStateManager] Actuators initalized");

    return true;
}

void Actuators::activateFan()
{
    digitalWrite(FAN_PIN, HIGH);
    systemState.actuatorsStatus.fanStatus = FanStatus::ON;
}

void Actuators::deactivateFan()
{
    digitalWrite(FAN_PIN, LOW);
    systemState.actuatorsStatus.fanStatus = FanStatus::OFF;
}

void Actuators::openVents()
{
    ventServo_0.write(VENT_SERVO_OPEN_POSITION);
    ventServo_1.write(VENT_SERVO_OPEN_POSITION);
    systemState.actuatorsStatus.ventsStatus = VentsStatus::OPEN;
}

void Actuators::closeVents()
{
    ventServo_0.write(VENT_SERVO_CLOSED_POSITION);
    ventServo_1.write(VENT_SERVO_CLOSED_POSITION);
    systemState.actuatorsStatus.ventsStatus = VentsStatus::CLOSED;
}

uint8_t Actuators::fanStatusStrToBitmask(const char* keyValue)
{
	if (strcmp(keyValue, "OFF") == 0)
		return static_cast<uint8_t>(FanStatus::OFF);
	if (strcmp(keyValue, "ON") == 0)
		return static_cast<uint8_t>(FanStatus::ON);
    return static_cast<uint8_t>(FanStatus::UNKNOWN); // Default return value if no match is found
}

const char* Actuators::fanStatusBitmaskToStr(uint8_t bitmask)
{
	switch (bitmask)
	{
		case static_cast<uint8_t>(FanStatus::OFF): return "OFF";
		case static_cast<uint8_t>(FanStatus::ON): return "ON";
        default: return "UNKNOWN";
	}
}

uint8_t Actuators::ventsStatusStrToBitmask(const char* keyValue)
{
	if (strcmp(keyValue, "CLOSED") == 0)
		return static_cast<uint8_t>(VentsStatus::CLOSED);
	if (strcmp(keyValue, "OPEN") == 0)
		return static_cast<uint8_t>(VentsStatus::OPEN);
    return static_cast<uint8_t>(VentsStatus::UNKNOWN); // Default return value if no match is found
}

const char* Actuators::ventsStatusBitmaskToStr(uint8_t bitmask)
{
	switch (bitmask)
	{
		case static_cast<uint8_t>(VentsStatus::CLOSED): return "CLOSED";
		case static_cast<uint8_t>(VentsStatus::OPEN): return "OPEN";
        default: return "UNKNOWN";
	}
}