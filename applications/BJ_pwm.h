/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-05-20     Gwvt       the first version
 */
#ifndef APPLICATIONS_BJ_PWM_H_
#define APPLICATIONS_BJ_PWM_H_

#include <rtthread.h>
#include <rtdbg.h>
#include "rtdevice.h"
#include <drv_common.h>

#define PWM_DEV_NAME        "pwm3"
#define PWM_DEV_CHANNEL     4
#define MOT_EN      0x12
#define MOT_DIR     0x1a

#define SPEED_THREAD_SIZE   1024
#define SPEED_THREAD_PRIORITY     2
#define SPEED_THREAD_TIMESLICE    20

void pwm_sample(void);
void high_speed();
void low_speed();
void wait_pwm_finish();

#endif /* APPLICATIONS_BJ_PWM_H_ */
