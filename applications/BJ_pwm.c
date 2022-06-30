/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-05-20     Gwvt       the first version
 */

#include "BJ_pwm.h"

#define SPEED_THREAD_SIZE   1024
#define SPEED_THREAD_PRIORITY     2
#define SPEED_THREAD_TIMESLICE    20

#define MS1_SPEED_ALL 192000
#define MS2_SPEED_ALL 44800

struct rt_device_pwm *pwm_dev;
TIM_HandleTypeDef htim3;
static uint32_t stepper_number = 0;
static rt_uint32_t period;
static uint16_t speed_flag = 0;
static uint16_t speed_k = 0;
static uint16_t speed_number = 0;
static uint32_t speed_all = 19200;

static struct rt_semaphore pwm_sem;

uint32_t speed_period;

#define PWM_SPEED   625000
#define PWM_SPEED_HIGH  625000
#define PWM_SPEED_LOW   125000
//#define PWM_SPEED   937500


void TIM3_IRQHandler(void)
{
  /* USER CODE BEGIN TIM3_IRQn 0 */

  /* USER CODE END TIM3_IRQn 0 */
  HAL_TIM_IRQHandler(&htim3);
  /* USER CODE BEGIN TIM3_IRQn 1 */

  /* USER CODE END TIM3_IRQn 1 */
}

void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 0;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 65535;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  HAL_TIM_MspPostInit(&htim3);

}

static void pwm_thread(void)
{
    uint8_t i;
    while (1)
    {
        rt_sem_take(&pwm_sem, RT_WAITING_FOREVER);
        for(i=0;i<10;i++){
            rt_thread_mdelay(50);
            if(i < 4)speed_period = PWM_SPEED_LOW - speed_k * 10000;
            else if(i < 8)speed_period = PWM_SPEED_LOW - 40000 - (speed_k - 4)*5000;
            else if(i == 9)speed_period = 62500;
            rt_pwm_set(pwm_dev, PWM_DEV_CHANNEL, speed_period, speed_period/20);
        }
    }
}

void pwm_sample(void){

    rt_pin_mode(MOT_EN, PIN_MODE_OUTPUT);
    rt_pin_mode(MOT_DIR, PIN_MODE_OUTPUT);

    rt_pin_write(MOT_DIR, PIN_HIGH);
    rt_pin_write(MOT_EN, PIN_HIGH);

    MX_TIM3_Init();
    TIM3_IRQHandler();
    period = PWM_SPEED;
    pwm_dev = (struct rt_device_pwm *)rt_device_find(PWM_DEV_NAME);
    if(pwm_dev == RT_NULL){
        rt_kprintf("1\n");
        return;
    }

    rt_sem_init(&pwm_sem, "pwm_sem", 0, RT_IPC_FLAG_FIFO);
    rt_thread_t thread = rt_thread_create("pwm_thread", (void (*)(void *parameter))pwm_thread, RT_NULL, 1024, 5, 10);
    if (thread != RT_NULL)
    {
        rt_thread_startup(thread);
    }
    else
    {
        rt_kprintf("pwm thread error \n");
    }
}
void high_speed(){
    rt_uint32_t period;

    rt_pin_write(MOT_EN, PIN_LOW);
    period = PWM_SPEED_HIGH;
    stepper_number = 0;
    speed_flag = 0;
    speed_k = 0;
    speed_all = MS2_SPEED_ALL;

    rt_pwm_set(pwm_dev, PWM_DEV_CHANNEL, period, period/20);
    rt_pwm_enable(pwm_dev, PWM_DEV_CHANNEL);
}
void low_speed(){
    rt_uint32_t period;

    rt_pin_write(MOT_EN, PIN_LOW);
    period = PWM_SPEED_LOW;
    stepper_number = 0;
    speed_flag = 1;
    speed_k = 0;
    speed_all = MS1_SPEED_ALL;
    rt_sem_release(&pwm_sem);

    rt_pwm_set(pwm_dev, PWM_DEV_CHANNEL, period, period/20);
    rt_pwm_enable(pwm_dev, PWM_DEV_CHANNEL);

}
void stop_turning(){
    rt_pwm_disable(pwm_dev,PWM_DEV_CHANNEL);
}

MSH_CMD_EXPORT(high_speed,bj high speed);
MSH_CMD_EXPORT(low_speed,bj low speed);
MSH_CMD_EXPORT(stop_turning,bj stop turning);

void wait_pwm_finish(){
    while(stepper_number < speed_all){
        rt_thread_mdelay(20);
    }
}


void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim)
{
//    if(speed_flag){
//        if(speed_k < 10)speed_number++;
//        if(speed_number >= 50){
//            speed_k ++;
//            if(speed_k < 4)speed_period = PWM_SPEED_LOW - speed_k * 10000;
//            else if(speed_k < 8)speed_period = PWM_SPEED_LOW - 40000 - (speed_k - 4)*5000;
//            else if(speed_k < 10)speed_period = PWM_SPEED_LOW - 60000 - (speed_k - 8)*2500;
//            speed_number = 0;
//            rt_pwm_set(pwm_dev, PWM_DEV_CHANNEL, speed_period, speed_period/20);
//        }
//    }

    stepper_number++;
    if(stepper_number >= speed_all){
        rt_pwm_disable(pwm_dev,PWM_DEV_CHANNEL);
        rt_pin_write(MOT_EN, PIN_HIGH);
    }
}


