#ifndef INC_I2C_LCD_H_
#define INC_I2C_LCD_H_


#include "main.h"
#include "queue_manager.h"
#include "adc_sensors.h"
#include "tcpserver.h"
#include "stdio.h"

void lcd_init (void);
void lcd_send_cmd (char cmd);
void lcd_send_data (char data);
void lcd_send_string (char *str);
void lcd_put_cur(int row, int col);
void lcd_clear (void);
void lcd_control_task(void *argument);
void init_lcd_control_task(void);
#endif /* INC_I2C_LCD_H_ */
