#include "tasks/sysMonitor/sysMonitor.h"

void sysMonitorTask(void *pvParameters)
{
    Serial.println("[sysMonitor] Task started");
    Serial.println("[sysMonitor] Initiating monitoring");

    TickType_t xLastPrintTime;
    const TickType_t xTimeInterval = pdMS_TO_TICKS(10000); // 10s

    // Initialise the xLastWakeTime variable with the current time.
    xLastPrintTime = xTaskGetTickCount();

    while (true)
    {
        Serial.println("\n[sysMonitor] === SYSTEM STATUS ===");
        
        // Get the total free size of all the heap memory regions
        Serial.printf("[sysMonitor] Free heap: %u bytes\n", ESP.getFreeHeap());              //
        
        /* 
            This adds all the low watermarks of the heap regions. This result gives a "worst case"
            indication for all-time minimum free heap.
        */
       Serial.printf("[sysMonitor] Min free heap: %u bytes\n", ESP.getMinFreeHeap());       //
       
       // Get the largest free block of heap memory able to be allocated.
       Serial.printf("[sysMonitor] Max alloc heap: %u bytes\n", ESP.getMaxAllocHeap());
       
       printTasksStats();

       // Wait for the next cycle
       vTaskDelayUntil(&xLastPrintTime, xTimeInterval);
    }
}

void printTasksStats()
{
    char lineBuffer[128];    // Increased from 64 to avoid any problems at all (64 should be enough)

    TaskStatus_t *pxTaskStatusArray;

    // This causes a momentary noticeable lag
    volatile UBaseType_t uxArraySize = uxTaskGetNumberOfTasks(); // Volatile as it may change at any moment

    pxTaskStatusArray = (TaskStatus_t *)pvPortMalloc(uxArraySize * sizeof(TaskStatus_t));

    // Need to add core id in here
    if(pxTaskStatusArray != NULL)
    {
        uxArraySize = uxTaskGetSystemState(pxTaskStatusArray, uxArraySize, NULL);

        Serial.println("[sysMonitor] --------------------------------------------------");
        Serial.println("[sysMonitor] | Task Name      | State     | Core | Prio | Stack Free |");
        Serial.println("[sysMonitor] --------------------------------------------------");

        for(UBaseType_t taskId = 0; taskId < uxArraySize; taskId++)
        {
            //Serial.printf("%s\t%s\t%u\t\t%u\n",
            snprintf(lineBuffer, sizeof(lineBuffer), "[sysMonitor] | %-15s | %-9s | %-2u | %-4u | %-9u |",
                pxTaskStatusArray[taskId].pcTaskName,
                taskStatusToStr(pxTaskStatusArray[taskId].eCurrentState),
                pxTaskStatusArray[taskId].xCoreID,
                pxTaskStatusArray[taskId].uxCurrentPriority,
                pxTaskStatusArray[taskId].usStackHighWaterMark
            );
            Serial.println(lineBuffer);
        }
        Serial.println("[sysMonitor] --------------------------------------------------");

        vPortFree(pxTaskStatusArray);
    }
    else
        Serial.println("[sysMonitor] Error: Could not allocate memory for tasks stats");
}

// Helper function to return states in string form
const char* taskStatusToStr(eTaskState state)
{
    switch (state)
    {
        case eRunning : return "Running";
        case eReady : return "Ready";
        case eBlocked : return "Blocked";
        case eSuspended : return "Suspended";
        case eDeleted : return "Deleted";
        default : return "Unknown";
    }
}