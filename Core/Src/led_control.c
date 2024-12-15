#include "led_control.h"

// Coada pentru LED-uri
QueueHandle_t ledQueue;

// Variabile pentru starea LED-urilor
static uint8_t led_states[3] = {0, 0, 0}; // 0 = OFF, 1 = ON

// Funcția pentru inițializarea LED-urilor și task-ului
void init_led_control_task(void)
{
    // Creăm coada pentru mesaje
    ledQueue = xQueueCreate(10, sizeof(LedMessage));
    if (ledQueue == NULL) {
        printf("Failed to create LED queue\n");
        return;
    }

    // Atributele task-ului
    osThreadAttr_t ledTask_attributes = {
        .name = "ledControlTask",
        .stack_size = 512 * 4,
        .priority = osPriorityNormal,
    };

    // Creăm task-ul pentru controlul LED-urilor
    osThreadNew(led_control_task, NULL, &ledTask_attributes);
}

// Task-ul pentru controlul LED-urilor
void led_control_task(void *argument)
{
    LedMessage message;

    // Inițializează LED-urile ca OUTPUT și le setează ca OFF
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);  // LED1 - PB0
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);  // LED2 - PB7
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET); // LED3 - PB14

    while (1) {
        // Așteaptă mesaje din coadă
        if (xQueueReceive(ledQueue, &message, portMAX_DELAY) == pdTRUE) {
            uint8_t led_index = (uint8_t)message.led;

            if (led_index < 3) {
                // Schimbă starea LED-ului
                led_states[led_index] ^= 1; // Toggle
                GPIO_PinState pin_state = (led_states[led_index]) ? GPIO_PIN_SET : GPIO_PIN_RESET;

                switch (message.led) {
                    case LED1: HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, pin_state); break;
                    case LED2: HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, pin_state); break;
                    case LED3: HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, pin_state); break;
                }

                printf("LED%d state: %s\n", led_index + 1, (pin_state == GPIO_PIN_SET) ? "ON" : "OFF");
            }
        }
    }
}
