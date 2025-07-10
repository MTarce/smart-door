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

//MQTT broker host address
static const char addr[] = {"131.255.82.115"};

// MQTT Topics to subscribe
char topic_buzzer[] = {"hana/smartdoor/buzzer"};

// MQTT Topics to Publish
char topic_light[] = {"hana/smartdoor/light"};
char topic_door[] = {"hana/smartdoor/door"};

//Buffers
static uint8_t subscribed_payload[HT_SUBSCRIBE_BUFF_SIZE] = {0}; // PayLoad Recebida
static uint8_t subscribed_topic[255] = {0};
volatile MessageData recieved_msg = {0};

static StaticTask_t yield_thread;
static uint8_t yieldTaskStack[1024*4];

static void HT_YieldThread(void *arg);


// para manter a conexão MQTT ativa
static void HT_YieldThread(void *arg) {
    while (1) {
        // Wait function for 10ms to check if some message arrived in subscribed topic
        MQTTYield(&mqttClient, 10);
    }
}

static void HT_Yield_Thread_Start(void *arg) {
    osThreadAttr_t task_attr;

    memset(&task_attr,0,sizeof(task_attr));
    memset(yieldTaskStack, 0xA5,(1024*4));
    task_attr.name = "yield_thread";
    task_attr.stack_mem = yieldTaskStack;
    task_attr.stack_size = (1024*4);
    task_attr.priority = osPriorityNormal;
    task_attr.cb_mem = &yield_thread;
    task_attr.cb_size = sizeof(StaticTask_t);

    osThreadNew(HT_YieldThread, NULL, &task_attr);
}




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

