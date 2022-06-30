/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-11-22     Rick       the first version
 */
#ifndef RADIO_RADIO_DECODER_H_
#define RADIO_RADIO_DECODER_H_

#include "rtthread.h"

typedef struct
{
    long Target_ID;
    long From_ID;
    int Counter;
    int Command;
    int Data;
    int Rssi;
}Message;

void rf433_rx_callback(int rssi,uint8_t *rx_buffer,uint8_t rx_len);
void rf4068_rx_callback(int rssi,uint8_t *rx_buffer,uint8_t rx_len);

#endif /* RADIO_RADIO_DECODER_H_ */
