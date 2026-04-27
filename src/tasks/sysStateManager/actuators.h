#pragma once

#include "../src/config.h"
#include "sysStateManager.h"
#include "ESP32Servo.h"

namespace Actuators
{
    enum class FanStatus : uint8_t
    {
        OFF,
        ON,
        UNKNOWN
    };

    enum class VentsStatus : uint8_t
    {
        CLOSED,
        OPEN,
        UNKNOWN
    };

    struct ActuatorsStatus
    {
        FanStatus fanStatus;
        VentsStatus ventsStatus;
        float heaterTemp;
    };

    bool initActuators();
    void activateFan();
    void deactivateFan();
    void openVents();
    void closeVents();

    uint8_t fanStatusStrToBitmask(const char* keyValue);
    const char* fanStatusBitmaskToStr(uint8_t bitmask);
    uint8_t ventsStatusStrToBitmask(const char* keyValue);
    const char* ventsStatusBitmaskToStr(uint8_t bitmask);
};