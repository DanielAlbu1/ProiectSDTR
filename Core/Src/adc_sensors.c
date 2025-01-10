#include "adc_sensors.h"


void humiditySensorAdcInit(void){

	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
	RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;

	GPIOA->MODER |= GPIO_MODER_MODER3;
	GPIOA->PUPDR &= ~(GPIO_PUPDR_PUPDR3);

	ADC1->CR2 = 0;
	ADC1->CR2 |= ADC_CR2_ADON;
	ADC1->SQR3 = 3;

	ADC1->SMPR2 |= ADC_SMPR2_SMP3;
	ADC1->CR1 = 0;
	ADC1->CR2 |= ADC_CR2_EXTSEL;
	ADC1->CR2 |= ADC_CR2_EXTEN_0;

}

uint16_t humiditySensorReadValue(void){

	ADC1->CR2 |= ADC_CR2_SWSTART;

	while(!(ADC1->SR & ADC_SR_EOC));

	return ADC1 -> DR;

}
void humidity_read_task(void *argument)
{
	while (1)
	    {
			PrintTaskTiming("ADC_start");
	        int adc_value = humiditySensorReadValue();

	        if (adc_value < 1200) adc_value = 1200;
	        if (adc_value > 4000) adc_value = 4000;

	        float humidity = (float)(4000 - adc_value) / (4000 - 1200) * 100;

	        HumiditySensorData sensorData;
	        sensorData.humidity = humidity;
	        sensorData.adc_value = adc_value;


	        if (xQueueOverwrite(sensorQueue, &sensorData) != pdTRUE)
	        {
	            printf("Failed to send humidity data to queue\n");
	        }

	        PrintTaskTiming("ADC_end");
	        vTaskDelay(pdMS_TO_TICKS(1000));
	    }
}
void init_humidity_task(void)
{
    osThreadAttr_t humidityTask_attributes = {
        .name = "humidityTask",
        .stack_size = 512 * 4,
        .priority = osPriorityNormal,
    };


    osThreadNew(humidity_read_task, NULL, &humidityTask_attributes);
}

void pump_control_task(void *argument)
{

    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_RESET);

    HumiditySensorData sensor_data;

    while (1)
    {
    	PrintTaskTiming("PumpControl_start");
        if (xQueuePeek(sensorQueue, &sensor_data, 0) == pdTRUE)
        {
            if (sensor_data.humidity < 50.0)
            {

                HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_RESET);
               // printf("Pompa oprită (Umiditate: %.2f%%)\n", sensor_data.humidity);
            }
            else
            {

                HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_SET);
               // printf("Pompa pornită (Umiditate: %.2f%%)\n", sensor_data.humidity);
            }
        }
        PrintTaskTiming("PumpControl_end");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void init_pump_task(void)
{
    osThreadAttr_t pumpTask_attributes = {
        .name = "pumpTask",
        .stack_size = 512 * 4,
        .priority = (osPriority_t) osPriorityNormal,
    };

    osThreadNew(pump_control_task, NULL, &pumpTask_attributes);
}
