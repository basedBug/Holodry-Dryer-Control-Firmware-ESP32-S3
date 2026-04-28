#pragma once

#include "../src/config.h"

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

    FanStatus fanStatusStrToFanStatus(const char* str);
    const char* fanStatusToFanStatusStr(FanStatus status);
    VentsStatus ventsStatusStrToVentsStatus(const char* str);
    const char* ventsStatusToVentsStatusStr(VentsStatus status);
};