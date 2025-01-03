#include "sys_info.h"

void print_system_info(void) {

    char taskListBuffer[512];


    uint32_t totalRunTime;
    uint32_t numTasks;
    TaskStatus_t taskStatusArray[10];

    printf("\n=== Sistem FreeRTOS ===\n\r");


    printf("Heap disponibil: %u bytes\n\r", xPortGetFreeHeapSize());
    printf("Heap minim disponibil: %u bytes\n\r", xPortGetMinimumEverFreeHeapSize());


    printf("\n=== Lista Taskuri ===\n\r");
    vTaskList(taskListBuffer);
    printf("Nume Task   Stare   Prioritate   Stack Rămas   Nr. Task\n\r");
    printf("%s\n", taskListBuffer);


    printf("\n=== Detalii Taskuri ===\n");
    numTasks = uxTaskGetSystemState(taskStatusArray, 10, &totalRunTime);
    for (uint32_t i = 0; i < numTasks; i++) {
        printf("Task: %s, Prioritate: %u, Stack liber: %u bytes\n\r",
               taskStatusArray[i].pcTaskName,
               taskStatusArray[i].uxCurrentPriority,
               taskStatusArray[i].usStackHighWaterMark);
    }


    printf("\n=== Alte informații ===\n\r");
    printf("Tick-uri de sistem: %lu\n\r", xTaskGetTickCount());
    printf("Stare scheduler: %u\n\r", xTaskGetSchedulerState());
}
