/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-11-22     Rick       the first version
 */
#include <rtthread.h>
#include <rtdevice.h>
#include <stdio.h>
#include "drv_spi.h"
#include <string.h>
#include "AX5043.h"
#include "Radio_Decoder.h"
//#include "Radio_Encoder.h"
#include "pin_config.h"
#include "Radio_Drv.h"
#include "heart.h"

#define DBG_TAG "RF_DE"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#define TAMPER_BIT                  0
#define BATTERYSTATE_BIT            1
#define BATTERYCRITICALLOW_BIT      2
#define VALVESTATE_BIT              3
#define CRITICALESCAPEALARM_BIT     4
#define MINORESCAPEALARM_BIT        5
#define INSPECTALARM_BIT            6
#define VALVECONNECTIONERROR_BIT    7
#define VALVECONTROLERROR_BIT       8
#define VALVECOLSEPULSECONTINUE_BIT 9

#define RSSI_JIA    0           //RSSI  + 偏移量
#define RSSI_JIAN   0           //RSSI  - 偏移量



extern rf_info info_433;
extern rf_info info_4068;

static uint8_t ms_tamper,ms_battery_state;
static uint32_t msid;
static uint32_t msv;
static uint8_t Controllable_Ms_flag[10];
static int ax_rssi;
static uint8_t v;

uint8_t get_buffer_bit(uint8_t buff,uint8_t bit){
    if(buff & (1<<bit))return 1;
    else return 0;
}

void rf4068_rx_callback(int rssi,uint8_t *rx_buffer,uint8_t rx_len)
{
    uint8_t i,j,version;
    uint8_t validity1,validity2;
    uint8_t data_buffer[8] = {0,0,0,0,0,0,0,0};
    uint32_t number = 0;
    uint8_t flag = 0;


    ax_rssi = rssi;
    ax_rssi += RSSI_JIA;
    ax_rssi -= RSSI_JIAN;
    if(rx_len == 16 && get_thread_MS() == 11){
        LOG_D("RSSI is %d,rx len = %d\r\n",rssi,rx_len);
        for(i=0;i<8;i++)data_buffer[i] = 0;
        for(i=0;i<16;i++){
            for(j=0;j<4;j++){
                data_buffer[i/2] <<= 1;
                if(get_buffer_bit(rx_buffer[i], 7-j*2) == 0 && get_buffer_bit(rx_buffer[i], 6-j*2) == 1)data_buffer[i/2] |= 0x01;
                else if(get_buffer_bit(rx_buffer[i], 7-j*2) ==  get_buffer_bit(rx_buffer[i], 6-j*2)){
                    rt_kprintf("rx_buffer get data error  %d",rx_buffer[i]);
                    return;
                }
            }
        }
        rt_kprintf("rx_buffer get data ok\r\n");

        for(i=0;i<8;i++){
            for(j=0;j<8;j++){
                rt_kprintf("%d  ",get_buffer_bit(data_buffer[i], j));
            }
            rt_kprintf("\r\n");
        }

        for(i=0;i<8;i++){
            validity1 = 0;
            validity2 = 0;
            for(j=0;j<8;j++){
                if(data_buffer[i] & (1 << j))validity1 ++;
                if(data_buffer[j] & (1 << i))validity2 ++;
            }
            if((validity1 % 2) | (validity2 % 2)){
                rt_kprintf("rx_buffer validity error %d %2x    %d  %d",validity1,data_buffer[i],validity2,i);
                return;
            }
        }
        rt_kprintf("rx_buffer validity ok \r\n");




        version = get_buffer_bit(data_buffer[0], 7)*0x02+get_buffer_bit(data_buffer[0], 6);
        v = version;
        if(version == 0){
            number = 0;
            for(i=0;i<5;i++){
                number <<= 1;
                number |= get_buffer_bit(data_buffer[(i+29)/7], 7-((i+29)%7));
            }
            if(number == 0x1f){
                number = 0;
                for(i=0;i<27;i++){
                    number <<= 1;
                    number |= get_buffer_bit(data_buffer[(i+2)/7], 7-((i+2)%7));
                }
                flag = 1;
                if(msid != number){
                    msid = number;
                    msv = 0;
                    flag = 0;
                }

                number = 0;
                for(i=0;i<10;i++){
                    Controllable_Ms_flag[i]= get_buffer_bit(data_buffer[(i+34)/7], 7-((i+34)%7));
                }
                if(flag)set_thread_MS();

            }else {
                number = 0;
                for(i=0;i<27;i++){
                    number <<= 1;
                    number |= get_buffer_bit(data_buffer[(i+2)/7], 7-((i+2)%7));
                }
                msid = number;

                number = 0;
                for(i=0;i<27;i++){
                    number <<= 1;
                    number |= get_buffer_bit(data_buffer[(i+29)/7], 7-((i+29)%7));
                }
                msv = number;
            }
        }else {
            for(i=0;i<10;i++)Controllable_Ms_flag[i] = 0;
            Controllable_Ms_flag[0] = get_buffer_bit(data_buffer[0], 5);
            Controllable_Ms_flag[1] = get_buffer_bit(data_buffer[0], 4);
            number = 0;
            for(i=0;i<20;i++){
                number <<= 1;
                number |= get_buffer_bit(data_buffer[(i+4)/7], 7-((i+4)%7));
            }
            msid = number;

            number = 0;
            for(i=0;i<20;i++){
                number <<= 1;
                number |= get_buffer_bit(data_buffer[(i+24)/7], 7-((i+24)%7));
            }
            msv = number;
            set_thread_MS();
        }
        if(version != 0)rt_kprintf("tample = %d    battery_state = %d\r\n",ms_tamper,ms_battery_state);
        else {
            for(i=0;i<10;i++)rt_kprintf("%d  ",Controllable_Ms_flag[i]);
            rt_kprintf("\r\n");
        }
        rt_kprintf("msid = %d    msv = %d\r\n",msid,msv);

        for(i=0;i<rx_len;i++)rt_kprintf("%2x ",rx_buffer[i]);
        rt_kprintf("\r\n");
    }

}
uint32_t get_ax_msid(){
    return msid;
}
uint32_t get_ax_msv(){
    return msv;
}
int get_ax_rssi(){
    return ax_rssi;
}
uint8_t get_ax_v(){
    return v;
}
void get_ax_flag(uint8_t *data){
    uint8_t i;
    for(i=0;i<10;i++){
        data[i] = Controllable_Ms_flag[i];
    }
}
