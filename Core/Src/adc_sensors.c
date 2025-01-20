#include "adc_sensors.h"
osThreadId_t adcTaskHandle;
osThreadId_t pumpTaskHandle;
TIM_HandleTypeDef htim3;
volatile uint32_t notify_start = 0;
volatile uint32_t task_start = 0;
volatile uint32_t pump_start = 0;
volatile uint32_t difference = 0;

void Timer3_Init(void)
{

    __HAL_RCC_TIM3_CLK_ENABLE();

    htim3.Instance = TIM3;
    htim3.Init.Prescaler = 10800 - 1;
    htim3.Init.Period = 5 - 1;
    htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    HAL_TIM_Base_Init(&htim3);

    HAL_TIM_Base_Start_IT(&htim3);
}
void DWT_Init(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->LAR = 0xC5ACCE55;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}
void TIM3_IRQHandler(void)
{

	    if (__HAL_TIM_GET_FLAG(&htim3, TIM_FLAG_UPDATE)) {
	        __HAL_TIM_CLEAR_FLAG(&htim3, TIM_FLAG_UPDATE);
	        notify_start = DWT->CYCCNT;
	        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	        xTaskNotifyFromISR(adcTaskHandle, 0, eNoAction, &xHigherPriorityTaskWoken);

	    }
}
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
			ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
			//PrintTaskTiming("ADC_start");

	        int adc_value = humiditySensorReadValue();
	        task_start = DWT->CYCCNT;
	        if (adc_value < 1200) adc_value = 1200;
	        if (adc_value > 4000) adc_value = 4000;

	        float humidity = (float)(4000 - adc_value) / (4000 - 1200) * 100;

	        HumiditySensorData sensorData;
	        sensorData.humidity = humidity;
	        sensorData.adc_value = adc_value;


	        if (xQueueOverwrite(sensorQueue, &sensorData) != pdTRUE)
	        {
	           // printf("Failed to send humidity data to queue\n");
	        }

	       // PrintTaskTiming("ADC_end");
	        xTaskNotify(pumpTaskHandle, 0, eNoAction);


	    }
}
void init_humidity_task(void)
{
    osThreadAttr_t humidityTask_attributes = {
        .name = "humidityTask",
        .stack_size = 512 * 4,
        .priority = osPriorityRealtime2,
    };


    adcTaskHandle =   osThreadNew(humidity_read_task, NULL, &humidityTask_attributes);

}

void pump_control_task(void *argument)
{

    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_RESET);

    HumiditySensorData sensor_data;

    while (1)

    {
    	ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    	//PrintTaskTiming("PumpControl_start");

        if (xQueuePeek(sensorQueue, &sensor_data, 0) == pdTRUE)
        {	pump_start = DWT->CYCCNT;
            if (sensor_data.humidity < 10.0)
            {

                HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_RESET);
               // printf("Pompa oprită (Umiditate: %.2f%%)\n", sensor_data.humidity);
            }
            else
            {

                HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_SET);
               // printf("Pompa pornită (Umiditate: %.2f%%)\n", sensor_data.humidity);
            }
            difference = pump_start - task_start;
            float time_in_seconds;
            time_in_seconds = (float)difference / 216000000.0f;
        }
       // PrintTaskTiming("PumpControl_end");

    }
}

void init_pump_task(void)
{
    osThreadAttr_t pumpTask_attributes = {
        .name = "pumpTask",
        .stack_size = 512 * 4,
        .priority = (osPriority_t) osPriorityRealtime1,
    };
    DWT_Init();
    Timer3_Init();

    pumpTaskHandle =  osThreadNew(pump_control_task, NULL, &pumpTask_attributes);

}
