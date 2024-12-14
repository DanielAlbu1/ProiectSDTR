#include "adc_sensors.h"


void humiditySensorAdcInit(void){

	//activare ceas pentru GPIOA SI ADC1

	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
	RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;

	//CONFIGURARE PA3 CA INTRARE ANALOGICA
	GPIOA->MODER |= GPIO_MODER_MODER3;
	GPIOA->PUPDR &= ~(GPIO_PUPDR_PUPDR3);

	//CONFIGURARE ADC1
	ADC1->CR2 = 0;//RESETARE REGISTRU CONTROL
	ADC1->CR2 |= ADC_CR2_ADON; //ACTIVARE ADC
	ADC1->SQR3 = 3; //CANALUL 3 IN PRIMA POZITIE DE SECVENTA DE CONVERSIE

	ADC1->SMPR2 |= ADC_SMPR2_SMP3;//SETARE TIMP ESNATIONARE CANALUL 3
	ADC1->CR1 = 0;
	ADC1->CR2 |= ADC_CR2_EXTSEL;//SETARE DECLANSATOR SOFTWARE
	ADC1->CR2 |= ADC_CR2_EXTEN_0; //ACTIVARE DECLANSATOR SOFTWARE

}

uint16_t humiditySensorReadValue(void){

	ADC1->CR2 |= ADC_CR2_SWSTART; //start conversie

	while(!(ADC1->SR & ADC_SR_EOC));//ASTEPTARE FINALIZARE CONVERSIE

	return ADC1 -> DR;


}
void humidity_read_task(void *argument)
{
	while (1)
	    {
	        // Citește valoarea ADC de la senzorul de umiditate
	        int adc_value = humiditySensorReadValue();  // Funcția care citește ADC-ul senzorului de umiditate

	        // Asigură-te că valoarea ADC este între 1200 și 4000
	        if (adc_value < 1200) adc_value = 1200;
	        if (adc_value > 4000) adc_value = 4000;

	        // Calculează valoarea umidității (în procente)
	        float humidity = (float)(4000 - adc_value) / (4000 - 1200) * 100;  // Transforma valoarea ADC într-un procent

	        // Creează structura cu valorile citite
	        HumiditySensorData sensorData;
	        sensorData.humidity = humidity;       // Setează umiditatea
	        sensorData.adc_value = adc_value;    // Setează valoarea ADC

	        // Pune structura pe coadă (suprascrie orice valoare anterioară)
	        if (xQueueOverwrite(sensorQueue, &sensorData) != pdTRUE)
	        {
	            printf("Failed to send humidity data to queue\n");
	        }

	        // Așteaptă 5 secunde înainte de a citi din nou
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

    // Crează task-ul care citește umiditatea la fiecare 5 secunde
    osThreadNew(humidity_read_task, NULL, &humidityTask_attributes);
}
