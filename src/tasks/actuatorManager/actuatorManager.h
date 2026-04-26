#pragma once

void pidManagerTask(void *pvParameters);

namespace ActuatorManager
{
    void activateFan();
    void deactivateFan();
    void openVents();
    void closeVents();
};
