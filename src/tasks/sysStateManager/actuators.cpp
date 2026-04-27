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

Actuators::FanStatus Actuators::fanStatusStrToFanStatus(const char* str)
{
	if (strcmp(str, "OFF") == 0)
		return FanStatus::OFF ;
	if (strcmp(str, "ON") == 0)
		return FanStatus::ON ;
    return FanStatus::UNKNOWN ; // Default return value if no match is found
}

const char* Actuators::fanStatusToFanStatusStr(FanStatus status)
{
	switch (status)
	{
		case FanStatus::OFF : return "OFF";
		case FanStatus::ON : return "ON";
        default: return "UNKNOWN";
	}
}

Actuators::VentsStatus Actuators::ventsStatusStrToVentsStatus(const char* str)
{
	if (strcmp(str, "CLOSED") == 0)
		return VentsStatus::CLOSED ;
	if (strcmp(str, "OPEN") == 0)
		return VentsStatus::OPEN ;
    return VentsStatus::UNKNOWN ; // Default return value if no match is found
}

const char* Actuators::ventsStatusToVentsStatusStr(VentsStatus status)
{
	switch (status)
	{
		case VentsStatus::CLOSED : return "CLOSED";
		case VentsStatus::OPEN : return "OPEN";
        default: return "UNKNOWN";
	}
}