
/** Put this in the src folder **/

#include "i2c_lcd.h"
extern I2C_HandleTypeDef hi2c2;  // change your handler here accordingly

#define SLAVE_ADDRESS_LCD 0x4E // change this according to ur setup

void lcd_send_cmd (char cmd)
{
  char data_u, data_l;
	uint8_t data_t[4];
	data_u = (cmd&0xf0);
	data_l = ((cmd<<4)&0xf0);
	data_t[0] = data_u|0x0C;  //en=1, rs=0 -> bxxxx1100
	data_t[1] = data_u|0x08;  //en=0, rs=0 -> bxxxx1000
	data_t[2] = data_l|0x0C;  //en=1, rs=0 -> bxxxx1100
	data_t[3] = data_l|0x08;  //en=0, rs=0 -> bxxxx1000
	HAL_I2C_Master_Transmit (&hi2c2, SLAVE_ADDRESS_LCD,(uint8_t *) data_t, 4, 100);
}

void lcd_send_data (char data)
{
	char data_u, data_l;
	uint8_t data_t[4];
	data_u = (data&0xf0);
	data_l = ((data<<4)&0xf0);
	data_t[0] = data_u|0x0D;  //en=1, rs=0 -> bxxxx1101
	data_t[1] = data_u|0x09;  //en=0, rs=0 -> bxxxx1001
	data_t[2] = data_l|0x0D;  //en=1, rs=0 -> bxxxx1101
	data_t[3] = data_l|0x09;  //en=0, rs=0 -> bxxxx1001
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
	// 4 bit initialisation
	vTaskDelay(pdMS_TO_TICKS(40));  // wait for >40ms
	lcd_send_cmd (0x30);
	vTaskDelay(pdMS_TO_TICKS(5));   // wait for >4.1ms
	lcd_send_cmd (0x30);
	vTaskDelay(pdMS_TO_TICKS(1)); // wait for >100us
	lcd_send_cmd (0x30);
	vTaskDelay(pdMS_TO_TICKS(10));
	lcd_send_cmd (0x20);  // 4bit mode
	vTaskDelay(pdMS_TO_TICKS(10));

  // dislay initialisation
	lcd_send_cmd (0x28); // Function set --> DL=0 (4 bit mode), N = 1 (2 line display) F = 0 (5x8 characters)
	vTaskDelay(pdMS_TO_TICKS(1));
	lcd_send_cmd (0x08); //Display on/off control --> D=0,C=0, B=0  ---> display off
	vTaskDelay(pdMS_TO_TICKS(1));
	lcd_send_cmd (0x01);  // clear display
	vTaskDelay(pdMS_TO_TICKS(1)); vTaskDelay(pdMS_TO_TICKS(1));
	lcd_send_cmd (0x06); //Entry mode set --> I/D = 1 (increment cursor) & S = 0 (no shift)
	vTaskDelay(pdMS_TO_TICKS(1));
	lcd_send_cmd (0x0C); //Display on/off control --> D = 1, C and B = 0. (Cursor and blink, last two bits)
}

void lcd_send_string (char *str)
{
	while (*str) lcd_send_data (*str++);
}
void init_lcd_control_task(void)
{


    // Atributele task-ului
    osThreadAttr_t lcdTask_attributes = {
        .name = "lcdControlTask",
        .stack_size = 512 * 4,
        .priority = (osPriority_t)osPriorityNormal,
    };

    // Creăm task-ul pentru controlul LED-urilor
    osThreadNew(lcd_control_task, NULL, &lcdTask_attributes);
}
void lcd_control_task(void *argument)
{	lcd_init();
HumiditySensorData sensor_data;

char adc_str[16];    // Buffer pentru string-ul valorii ADC
char humidity_str[16]; // Buffer pentru string-ul valorii de umiditate
	while(1)
	{



	    if (xQueuePeek(sensorQueue, &sensor_data, 0) == pdTRUE)
	    {
		    // Conversia valorilor în string
		    sprintf(adc_str, "ADC: %d", sensor_data.adc_value);         // Conversie pentru int
		    sprintf(humidity_str, "Hum: %.2f%%", sensor_data.humidity); // Conversie pentru float cu 2 zecimale

		    lcd_clear();
		    // Afișarea pe LCD
		    lcd_put_cur(0, 0);
		    lcd_send_string(adc_str);
		    lcd_put_cur(1, 0);
		    lcd_send_string(humidity_str);
	    }



	    vTaskDelay(1000);




	}
	  vTaskDelete(NULL);

}
