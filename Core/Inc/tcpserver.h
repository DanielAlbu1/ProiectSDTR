
#ifndef INC_TCPSERVER_H_
#define INC_TCPSERVER_H_
#include "led_control.h"
#include "adc_sensors.h"
float get_humidity_value(void);
void tcpserver_init (void);
int get_humidity_adc_value(void);
#endif /* INC_TCPSERVER_H_ */
