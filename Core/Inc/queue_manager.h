/*
 * queue_manager.h
 *
 *  Created on: Dec 14, 2024
 *      Author: danie
 */

#ifndef INC_QUEUE_MANAGER_H_
#define INC_QUEUE_MANAGER_H_

#include "FreeRTOS.h"
#include "queue.h"
#include "led_control.h"
typedef struct {
    float humidity; // Valoarea senzorului de umiditate
    int adc_value;
} HumiditySensorData;




// Declararea externă a cozii
extern QueueHandle_t sensorQueue;


#endif /* INC_QUEUE_MANAGER_H_ */
