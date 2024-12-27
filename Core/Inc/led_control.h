/*
 * led_control.h
 *
 *  Created on: Dec 14, 2024
 *      Author: danie
 */

#ifndef INC_LED_CONTROL_H_
#define INC_LED_CONTROL_H_

#include "queue_manager.h"
#include "main.h"
#include "cmsis_os.h"
#include "lwip.h"
#include "tcpserver.h"
#include "semphr.h"
typedef enum {
    LED1,
    LED2,
    LED3
} LedId;



typedef struct {
    LedId led;      // LED-ul vizat (LED1, LED2, LED3)
    uint8_t toggle; // 1 = ON/OFF (toggle)
} LedMessage;


// Prototipuri
void init_led_control_task(void);
void led_control_task(void *argument);;

#endif /* INC_LED_CONTROL_H_ */
