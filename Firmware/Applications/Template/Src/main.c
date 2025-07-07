#include "string.h"
#include "bsp.h"
#include "ostask.h"
#include "debug_log.h"
#include "FreeRTOS.h"
#include "main.h"
#include "bh1750fvi.h"
#include "ReedSwitch.h"

extern USART_HandleTypeDef huart1;

void BH1750_Task(void *arg);

/**
  \fn          static void appInit(void *arg)
  \brief
  \return
*/
static void appInit(void *arg)
{
  xTaskCreate(BH1750_Task, "BH1750", 512, NULL, 2, NULL);
}

/**
  \fn          int main_entry(void)
  \brief       main entry function.
  \return
*/
void main_entry(void)
{
  uint32_t uart_cntrl = (ARM_USART_MODE_ASYNCHRONOUS | ARM_USART_DATA_BITS_8 | ARM_USART_PARITY_NONE |
                         ARM_USART_STOP_BITS_1 | ARM_USART_FLOW_CONTROL_NONE);
  BSP_CommonInit();
  HAL_USART_InitPrint(&huart1, GPR_UART1ClkSel_26M, uart_cntrl, 115200);

  osKernelInitialize();
  registerAppEntry(appInit, NULL);

  if (osKernelGetState() == osKernelReady)
  {
    osKernelStart();
  }
  while (1)
    ;
}

/*=========================================
TASK PARA LEITURA DO SENSOR DE LUMINOSIDADEqu
===========================================*/

void BH1750_Task(void *arg)
{
  ReedSwitchInit();
  // lightSensor_begin(ADDRESS1,CONTINUOUS_AUTO);

  while (1)
  {
    printf ("Mudou: %u\n",ChangeState());
    // uint16_t lux = lightSensor_meter();
    // printf("Luminosidade: %u lux\n", lux);
    // vTaskDelay(pdMS_TO_TICKS(500));
  }
}

