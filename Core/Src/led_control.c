#include "led_control.h"


QueueHandle_t ledQueue;


static uint8_t led_states[3] = {0, 0, 0};


void init_led_control_task(void)
{

    ledQueue = xQueueCreate(5, sizeof(LedMessage));
    if (ledQueue == NULL) {
        printf("Failed to create LED queue\n");
        return;
    }


    osThreadAttr_t ledTask_attributes = {
        .name = "ledControlTask",
        .stack_size = 512 * 4,
        .priority = osPriorityNormal,
    };


    osThreadNew(led_control_task, NULL, &ledTask_attributes);
}


void led_control_task(void *argument)
{
    LedMessage message;


    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);

    while (1) {
    //	PrintTaskTiming("LED_start");
        if (xQueueReceive(ledQueue, &message, portMAX_DELAY) == pdTRUE) {
            uint8_t led_index = (uint8_t)message.led;

            if (led_index < 3) {

                led_states[led_index] ^= 1;
                GPIO_PinState pin_state = (led_states[led_index]) ? GPIO_PIN_SET : GPIO_PIN_RESET;

                switch (message.led) {
                    case LED1: HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, pin_state); break;
                    case LED2: HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, pin_state); break;
                    case LED3: HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, pin_state); break;
                }


            }
        }
     //   PrintTaskTiming("LED_end");
    }
}
