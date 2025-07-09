#include "ReedSwitch.h"
#include "HT_MQTT_Api.h"

#include <stdio.h>

#include <pad_qcx212.h>
#include <HT_gpio_qcx212.h>
#include <FreeRTOS.h>
#include <task.h>

#define SAMPLES 32000

/*------MQTT-----*/
static MQTTClient mqttClient;
static Network mqttNetwork;

//Buffer that will be published.
static uint8_t mqttSendbuf[HT_MQTT_BUFFER_SIZE] = {0};
static uint8_t mqttReadbuf[HT_MQTT_BUFFER_SIZE] = {0};

static const char clientID[] = {"SIP_HTNB32L-XXX"};
static const char username[] = {""};
static const char password[] = {""};

// MQTT Topics to subscribe
char topic_led_1[] = {"hana/rafael/htnb32l_led_1"};
char topic_led_2[] = {"hana/rafael/htnb32l_led_2"};
char topic_led_3[] = {"hana/rafael/htnb32l_led_3"};

// MQTT Topics to Publish
char topic_button_1[] = {"hana/rafael/htnb32l_button_1"};
char topic_button_2[] = {"hana/rafael/htnb32l_button_2"};
char topic_ldr[] = {"hana/rafael/htnb32l_ldr_value"};


// GPIO02 - ReedSwitch
#define REED_SWITCH_INSTANCE 0               /**</ REED_SWITCH pin instance. */
#define REED_SWITCH_PIN 2                    /**</ REED_SWITCH pin number. */
#define REED_SWITCH_PAD_ID 13                /**</ REED_SWITCH Pad ID. */
#define REED_SWITCH_PAD_ALT_FUNC PAD_MuxAlt0 /**</ Button pin alternate function. */

void ReedSwitchInit(void)
{
    GPIO_InitType GPIO_InitStruct = {0};
    GPIO_InitStruct.af = PAD_MuxAlt0;
    GPIO_InitStruct.pad_id = REED_SWITCH_PAD_ID;
    GPIO_InitStruct.gpio_pin = REED_SWITCH_PIN;
    GPIO_InitStruct.pin_direction = GPIO_DirectionInput;
    GPIO_InitStruct.pull = PAD_InternalPullUp;
    GPIO_InitStruct.instance = REED_SWITCH_INSTANCE;
    GPIO_InitStruct.interrupt_config = GPIO_InterruptDisabled;

    HT_GPIO_Init(&GPIO_InitStruct);
}

ReadState ReedSwitchGetState(void)
{
    uint32_t filter = 0;
    for (int i = 0; i < SAMPLES; i++)
    {
        filter += HT_GPIO_PinRead(REED_SWITCH_INSTANCE, REED_SWITCH_PIN);
    }
    filter /= SAMPLES;
    if (filter == 1)
    {
        return kOpen;
    }
    return kClose;
}

uint8_t ChangeState(void)
{
    static uint8_t estadoAnterior = 0;
    uint8_t estadoAtual = ReedSwitchGetState();

    if (estadoAtual != estadoAnterior)
    {
        if (estadoAnterior == 0 && estadoAtual == 1)
        {
            printf("OPEN\n");
        }
        else if (estadoAnterior == 1 && estadoAtual == 0)
        {
            printf("CLOSE\n");
        }
        estadoAnterior = estadoAtual;
    }
    vTaskDelay(pdMS_TO_TICKS(500));
    return estadoAtual;
}

