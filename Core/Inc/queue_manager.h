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

typedef struct {
    float humidity; // Valoarea senzorului de umiditate
    int adc_value;
} HumiditySensorData;

typedef struct {
    uint8_t led;       // ID-ul LED-ului (de ex. LED1, LED2 etc.)
    uint8_t intensity; // Intensitatea (0 - 100%)
} LedControl;

// Coada globală
extern QueueHandle_t ledQueue;
// Declararea externă a cozii
extern QueueHandle_t sensorQueue;


#endif /* INC_QUEUE_MANAGER_H_ */
