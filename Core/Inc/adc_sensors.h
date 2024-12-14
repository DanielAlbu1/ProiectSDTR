/*
 * adc_sensors.h
 *
 *  Created on: Dec 14, 2024
 *      Author: danie
 */

#ifndef INC_ADC_SENSORS_H_
#define INC_ADC_SENSORS_H_

#include "queue_manager.h"
#include "main.h"
#include "cmsis_os.h"
#include "lwip.h"
#include "tcpserver.h"
#include "semphr.h"
void humiditySensorAdcInit(void);
uint16_t humiditySensorReadValue(void);
void init_humidity_task(void);
void humidity_read_task(void *argument);
#endif /* INC_ADC_SENSORS_H_ */
