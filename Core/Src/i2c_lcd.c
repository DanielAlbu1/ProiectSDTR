
#include "i2c_lcd.h"
extern I2C_HandleTypeDef hi2c2;

#define SLAVE_ADDRESS_LCD 0x4E

void lcd_send_cmd (char cmd)
{
  char data_u, data_l;
	uint8_t data_t[4];
	data_u = (cmd&0xf0);
	data_l = ((cmd<<4)&0xf0);
	data_t[0] = data_u|0x0C;
	data_t[1] = data_u|0x08;
	data_t[2] = data_l|0x0C;
	data_t[3] = data_l|0x08;
	HAL_I2C_Master_Transmit (&hi2c2, SLAVE_ADDRESS_LCD,(uint8_t *) data_t, 4, 100);
}

void lcd_send_data (char data)
{
	char data_u, data_l;
	uint8_t data_t[4];
	data_u = (data&0xf0);
	data_l = ((data<<4)&0xf0);
	data_t[0] = data_u|0x0D;
	data_t[1] = data_u|0x09;
	data_t[2] = data_l|0x0D;
	data_t[3] = data_l|0x09;
	HAL_I2C_Master_Transmit (&hi2c2, SLAVE_ADDRESS_LCD,(uint8_t *) data_t, 4, 100);
}

void lcd_clear (void)
{
	lcd_send_cmd (0x80);
	for (int i=0; i<70; i++)
	{
		lcd_send_data (' ');
	}
}

void lcd_put_cur(int row, int col)
{
    switch (row)
    {
        case 0:
            col |= 0x80;
            break;
        case 1:
            col |= 0xC0;
            break;
    }

    lcd_send_cmd (col);
}


void lcd_init (void)
{

	vTaskDelay(pdMS_TO_TICKS(40));
	lcd_send_cmd (0x30);
	vTaskDelay(pdMS_TO_TICKS(5));
	lcd_send_cmd (0x30);
	vTaskDelay(pdMS_TO_TICKS(1));
	lcd_send_cmd (0x30);
	vTaskDelay(pdMS_TO_TICKS(10));
	lcd_send_cmd (0x20);
	vTaskDelay(pdMS_TO_TICKS(10));


	lcd_send_cmd (0x28);
	vTaskDelay(pdMS_TO_TICKS(1));
	lcd_send_cmd (0x08);
	vTaskDelay(pdMS_TO_TICKS(1));
	lcd_send_cmd (0x01);
	vTaskDelay(pdMS_TO_TICKS(1)); vTaskDelay(pdMS_TO_TICKS(1));
	lcd_send_cmd (0x06);
	vTaskDelay(pdMS_TO_TICKS(1));
	lcd_send_cmd (0x0C);
}

void lcd_send_string (char *str)
{
	while (*str) lcd_send_data (*str++);
}
void init_lcd_control_task(void)
{



    osThreadAttr_t lcdTask_attributes = {
        .name = "lcdControlTask",
        .stack_size = 512 * 4,
        .priority = (osPriority_t)osPriorityNormal7,
    };


    osThreadNew(lcd_control_task, NULL, &lcdTask_attributes);
}
void lcd_control_task(void *argument)
{	lcd_init();
HumiditySensorData sensor_data;

char adc_str[16];
char humidity_str[16];
//lcd_clear();
	while(1)
	{
		//PrintTaskTiming("LCD_start");
	    if (xQueuePeek(sensorQueue, &sensor_data, pdMS_TO_TICKS(10)) == pdTRUE)
	    {


		    sprintf(adc_str, "ADC: %d", sensor_data.adc_value);
		    sprintf(humidity_str, "Hum: %.2f%%", sensor_data.humidity);

		    //lcd_clear();

		    lcd_put_cur(0, 0);
		    lcd_send_string(adc_str);
		    lcd_put_cur(1, 0);
		    lcd_send_string(humidity_str);
	    }
	  //  PrintTaskTiming("LCD_end");


	    vTaskDelay(pdMS_TO_TICKS(200));




	}

}
