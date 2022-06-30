/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-05-20     Gwvt       the first version
 */
#ifndef APPLICATIONS_BUTTON_16_H_
#define APPLICATIONS_BUTTON_16_H_

#include <rtthread.h>
#include <rtdbg.h>
#include "rtdevice.h"

#define BUTTON1     0x32
#define BUTTON2     0x13
#define BUTTON3     0x14
#define BUTTON4     0x15
#define BUTTON5     0x16
#define BUTTON6     0x17
#define BUTTON7     0x18
#define BUTTON8     0x19

#define BUTTON9     0x25
#define BUTTON10    0x10

void my_button_init();
uint8_t get_button_number();

#endif /* APPLICATIONS_BUTTON_16_H_ */
