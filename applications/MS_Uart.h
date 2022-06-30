/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-05-25     Gwvt       the first version
 */
#ifndef APPLICATIONS_MS_UART_H_
#define APPLICATIONS_MS_UART_H_

#include <rtthread.h>
#include <rtdevice.h>

#define MY_UART_MODBUS                 "uart2"
#define DATA_CMD_END1                    '\r'
#define ONE_DATA_MAXLEN                  100

#define UART_ERROR_TIMEOUT      1
#define UART_ERROR_DATA         2
#define UART_READ_OK            3
#define UART_WAIT               0


void uart_data_sample();
void my_uart_SetData(uint8_t flag,uint8_t zf_list[10],uint8_t len);
void my_uart_GetData(uint8_t flag,uint8_t choose);
uint8_t get_uart_flag();
uint32_t get_UartData();
void ms_open_valve();
#endif /* APPLICATIONS_MS_UART_H_ */
