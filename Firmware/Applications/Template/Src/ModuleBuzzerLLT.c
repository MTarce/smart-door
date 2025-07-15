#include "ModuleBuzzerLLT.h"

#include <stdio.h>

#include <pad_qcx212.h>
#include <HT_gpio_qcx212.h>

// GPIO04- Module_Buzzer_low_level_Triger
#define BUZZER_INSTANCE 0                   /**</ BUZZER pin instance. */
#define BUZZER_PIN 4                        /**</ BUZZER pin number. */
#define BUZZER_PAD_ID 15                    /**</ BUZZER Pad ID. */
#define BUZZER_PAD_ALT_FUNC PAD_MuxAlt0     /**</ BUZZER pin alternate function. */

void BuzzerInit(void)
{
    GPIO_InitType GPIO_InitStruct = {0};
    GPIO_InitStruct.af = PAD_MuxAlt0;
    GPIO_InitStruct.pad_id = BUZZER_PAD_ID;
    GPIO_InitStruct.gpio_pin = BUZZER_PIN;
    GPIO_InitStruct.pin_direction = GPIO_DirectionOutput;
    GPIO_InitStruct.instance = BUZZER_INSTANCE;
    GPIO_InitStruct.interrupt_config = GPIO_InterruptDisabled;

    HT_GPIO_Init(&GPIO_InitStruct);
}
