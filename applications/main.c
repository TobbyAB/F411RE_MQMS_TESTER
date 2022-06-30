/*
 * Copyright (c) 2006-2022, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-05-20     RT-Thread    first version
 */

#include <rtthread.h>

#define DBG_TAG "main"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>
#include "rtdevice.h"
#include "BJ_pwm.h"
#include "LCD_19264.h"
#include "button_16.h"
#include "my_usb_smq.h"
#include "MS_Uart.h"
#include "my_dfs.h"
#include "ds3231.h"

#define BUTTON_UP       10
#define BUTTON_DOWN     11
#define BUTTON_LEFT     13
#define BUTTON_RIGHT    14
#define BUTTON_OK       15
#define BUTTON_RETUEN   12

#define IR_DET   0x00
#define IR_LED   0x01

#define LED1 0x2e
#define LED2 0x2f
#define LED3 0x20
#define LED4 0x21

#define BUZZ 0x2d

#define WRITE_ERROR         1
#define MSID_ERROR          2
#define MSV_ERROR           3
#define MSV_INTERFERENCE    4
#define LOW_POWER           5
#define VALVE_ERROR         6
#define TAMPER              7
#define LOW_BATTERY         8
#define CRITICAL_LOW_BATTERY    9
#define VALVE_STATUS_ERROR  10
#define CRITICAL_ESCAPE     11
#define MINOR_ESCAPE_ERROR  12
#define VALVE_CONNECTION_ERROR  13
#define VALVE_ERROR_ALARM   14
#define VALVE_TIGHTNESS_ERROR   15
#define READ_ERROR          16
#define INSPECT_ERROR       17
#define WRITE_ERROR1        18
#define WRITE_ERROR2        19
#define VALVE_ERROR1        20

static uint8_t button_number;
static uint8_t MS = 0;
static uint8_t thread_time;
static uint8_t choose;
static uint8_t choose_number;
static uint8_t reverse;
static uint8_t MS_error;
static uint8_t zf_list1[8] = {7,22,23,111,112,4,5,6};             //MS阀门跑表器
static uint8_t zf_list2[12] = {11,0,1,2,3,111,112,4,5,6,108,21};        //可控MS阀门跑表器8位
static uint8_t zf_list12[12] = {11,0,1,2,3,111,112,4,5,6,106,21};        //可控MS阀门跑表器6位
static uint8_t zf_list13[7] = {6,24,25,26,27,28,29};                    //中继器原型机
static uint8_t zf_list3[6] = {5,111,112,113,114,120};           //MSID:
static uint8_t zf_list4[5] = {4,111,112,115,120};               //MSV:
static uint8_t zf_list5[6] = {5,116,117,118,119,120};           //TIME:
static uint8_t zf_list6[3][10] = {
        {6,101,102,103,104,105,106,100,100,100},                                    //5位SMID   123456
        {8,109,108,107,106,105,104,103,102,100},                                     //7位SMID   98765432
        {6,101,102,103,104,105,106,100,100,100}
};
static uint8_t zf_list7[7] = {6,100,100,100,100,100,100};                     //5位SMV    456789
static uint8_t zf_list8[10] = {9,100,100,7,100,100,8,100,100,9};              //5位SMT    00时00分08秒
static uint8_t zf_list9[8] = {7,10,11,12,13,110,110,20};                      //写入数据  √
static uint8_t zf_list10[8] = {7,14,15,16,17,110,110,20};                      //转动表头  √
static uint8_t zf_list11[8] = {7,18,19,12,13,110,110,20};                      //核对数据  √

static uint8_t MSID[20];
static uint8_t my_time[7];
static uint8_t msv_error = 0;
static uint32_t MS_thread_tick = 0;
static uint16_t show_ax_addr;
static uint8_t time_number = 0;


void MS_thread_GetTime(){
    get_time(my_time);
    zf_list8[1] = my_time[2]/10+100;
    zf_list8[2] = my_time[2]%10+100;
    zf_list8[4] = my_time[1]/10+100;
    zf_list8[5] = my_time[1]%10+100;
    zf_list8[7] = my_time[0]/10+100;
    zf_list8[8] = my_time[0]%10+100;
}
void MS_thread_SetTime(){
    my_time[2] = (zf_list8[1]-100)*10+(zf_list8[2]-100);
    my_time[1] = (zf_list8[4]-100)*10+(zf_list8[5]-100);
    my_time[0] = (zf_list8[7]-100)*10+(zf_list8[8]-100);
    if(my_time[2] > 23)my_time[0] = 23;
    if(my_time[1] > 59)my_time[1] = 59;
    if(my_time[0] > 59)my_time[2] = 59;
    set_time(my_time);
}

void MS_LED_init(){
    rt_pin_mode(LED1, PIN_MODE_OUTPUT);
    rt_pin_mode(LED2, PIN_MODE_OUTPUT);
    rt_pin_mode(LED3, PIN_MODE_OUTPUT);
    rt_pin_mode(LED4, PIN_MODE_OUTPUT);

    rt_pin_write(LED1, PIN_HIGH);
    rt_pin_write(LED2, PIN_HIGH);
    rt_pin_write(LED3, PIN_HIGH);
    rt_pin_write(LED4, PIN_HIGH);

    rt_pin_mode(BUZZ, PIN_MODE_OUTPUT);
    rt_pin_write(BUZZ, PIN_LOW);

    rt_pin_mode(IR_DET, PIN_MODE_OUTPUT);
    rt_pin_write(IR_DET, PIN_LOW);

    rt_pin_mode(IR_LED, PIN_MODE_INPUT);
}
void MS_LED_error(){
    rt_pin_write(LED1, PIN_LOW);
    rt_pin_write(LED3, PIN_LOW);

    rt_pin_write(LED2, PIN_HIGH);
    rt_pin_write(LED4, PIN_HIGH);
}
void MS_LED_ok(){
    rt_pin_write(LED1, PIN_HIGH);
    rt_pin_write(LED3, PIN_HIGH);

    rt_pin_write(LED2, PIN_LOW);
    rt_pin_write(LED4, PIN_LOW);
}
void MS_LED_off(){
    rt_pin_write(LED1, PIN_HIGH);
    rt_pin_write(LED3, PIN_HIGH);

    rt_pin_write(LED2, PIN_HIGH);
    rt_pin_write(LED4, PIN_HIGH);
}

void MS_thread_init(){
    uint8_t i;
    Dispgraphic(0x00);
    lcd_show(0,200,zf_list1,zf_list1[0],0);
    lcd_show(2,200,zf_list2,zf_list2[0],0);
    lcd_show(4,200,zf_list12,zf_list12[0],0);
    lcd_show(6,200,zf_list13,zf_list13[0],0);
    MS = 1;
    thread_time = 0;
    choose = 0;
    get_MSID(MSID);
    for(i=0;i<8;i++){
        if(i < 6){
            zf_list6[0][i+1] = MSID[i]+100;
            zf_list6[2][i+1] = MSID[14+i]+100;
        }
        zf_list6[1][i+1] = MSID[i+6]+100;
    }
}
void MS_thread_SelectMode(){
    if((thread_time % 10) == 0){
        reverse+=1;
        reverse%=2;
        if(choose == 0)lcd_show(0,200,zf_list1,zf_list1[0],reverse);
        else if(choose == 1)lcd_show(2,200,zf_list2,zf_list2[0],reverse);
        else if(choose == 2)lcd_show(4,200,zf_list12,zf_list12[0],reverse);
        else if(choose == 3)lcd_show(6,200,zf_list13,zf_list13[0],reverse);
    }
    if(button_number != 16){
        if(button_number == BUTTON_UP){
            if(reverse){
                if(choose == 0)lcd_show(0,200,zf_list1,zf_list1[0],0);
                else if(choose == 1)lcd_show(2,200,zf_list2,zf_list2[0],0);
                else if(choose == 2)lcd_show(4,200,zf_list12,zf_list12[0],0);
                else if(choose == 3)lcd_show(6,200,zf_list13,zf_list13[0],0);
            }

            if(choose)choose -= 1;
            else choose = 3;
            reverse = 0;
            thread_time = 9;
        }
        if(button_number == BUTTON_DOWN){
            if(reverse){
                if(choose == 0)lcd_show(0,200,zf_list1,zf_list1[0],0);
                else if(choose == 1)lcd_show(2,200,zf_list2,zf_list2[0],0);
                else if(choose == 2)lcd_show(4,200,zf_list12,zf_list12[0],0);
                else if(choose == 3)lcd_show(6,200,zf_list13,zf_list13[0],0);
            }
            choose += 1;
            choose %= 4;
            reverse = 0;
            thread_time = 9;
        }
        if(button_number == BUTTON_OK){
            MS = 2;
            Dispgraphic(0x00);
            thread_time = 0;
            reverse = 0;
            choose_number = 0;
            if(choose == 0)lcd_show(0,200,zf_list1,zf_list1[0],0);
            else if(choose == 1)lcd_show(0,200,zf_list2,zf_list2[0],0);
            else if(choose == 2)lcd_show(0,200,zf_list12,zf_list12[0],0);
            if(choose != 3){
                lcd_show(2, 0, zf_list3, zf_list3[0], reverse);
                lcd_show(4, 4, zf_list4, zf_list4[0], reverse);
                lcd_show(6, 0, zf_list5, zf_list5[0], reverse);

                lcd_show(2, 40, zf_list6[choose], zf_list6[choose][0], reverse);
                lcd_show(4, 40, zf_list7, zf_list7[0], reverse);

                MS_thread_GetTime();
                lcd_show(6, 40, zf_list8, zf_list8[0], reverse);
            }
            if(choose == 3)MS = 10;
        }
    }
}
void MS_thread_SettingMs(){
    static uint8_t setting_time = 0;

    if((thread_time % 10) == 0){
        reverse+=1;
        reverse%=2;

        if(choose_number < 10 && reverse)lcd_show(2, 40, zf_list6[choose], zf_list6[choose][0], choose_number%10+10);
        else lcd_show(2, 40, zf_list6[choose], zf_list6[choose][0], 0);

        if(choose_number >= 10 && choose_number < 20 && reverse)lcd_show(4, 40, zf_list7, zf_list7[0], choose_number%10+10);
        else lcd_show(4, 40, zf_list7, zf_list7[0], 0);

        if(choose_number >= 20 && choose_number < 30 && reverse)lcd_show(6, 40, zf_list8, zf_list8[0], choose_number%10+10+choose_number%10/2);
        else {
            if(setting_time == 0)MS_thread_GetTime();
            lcd_show(6, 40, zf_list8, zf_list8[0], 0);
        }

    }
    if(button_number != 16){
        if(button_number < 10 && choose_number < 30){
            if(choose_number < 10)zf_list6[choose][choose_number+1] = button_number+100;
            else if(choose_number < 20)zf_list7[choose_number%10+1] = button_number+100;
            else if(choose_number < 30)zf_list8[choose_number%10+1+choose_number%10/2] = button_number+100;

            choose_number+=1;
            if(choose == 1){
                if((choose_number%10) >= 8 && choose_number < 10)choose_number = 0;
            }else {
                if((choose_number%10) >= 6)choose_number = choose_number/10*10;
            }

            reverse = 0;
            thread_time = 9;
        }
        if(button_number == BUTTON_UP){
            if((choose_number%10) > 5)choose_number = choose_number/10*10+5;
            if(choose_number >= 10)choose_number -= 10;
            else choose_number = 20 + choose_number;
            reverse = 0;
            thread_time = 9;
        }
        if(button_number == BUTTON_DOWN){
            if((choose_number%10) > 5)choose_number = 5;
            choose_number += 10;
            choose_number %= 30;
            reverse = 0;
            thread_time = 9;
        }
        if(button_number == BUTTON_LEFT){
            if((choose_number%10) > 0)choose_number -= 1;
            else if(choose_number < 10 && choose == 1)choose_number = 7;
            else choose_number = choose_number/10*10+5;
            reverse = 0;
            thread_time = 9;
        }
        if(button_number == BUTTON_RIGHT){
            choose_number += 1;
            if(choose_number < 10 && choose == 1)choose_number = choose_number/10*10 + choose_number%10%8;
            else choose_number = choose_number/10*10 + choose_number%10%6;
            reverse = 0;
            thread_time = 9;
        }
        if(button_number == BUTTON_UP || button_number == BUTTON_DOWN || button_number == BUTTON_LEFT || button_number == BUTTON_RIGHT){
            if(choose_number >= 20 && choose_number <= 25){
                if(setting_time == 0)setting_time = 1;
            }else {
                if(setting_time == 1){
                    setting_time = 0;
                    MS_thread_SetTime();
                }
            }

        }
        if(button_number == BUTTON_RETUEN){
            if(choose_number == 30){
                choose_number = 0;
                reverse = 0;
                thread_time = 9;
            }
            else if(setting_time == 1){
                choose_number = 0;
                reverse = 0;
                thread_time = 9;
                setting_time = 0;
            }else {
                Dispgraphic(0x00);
                lcd_show(0,200,zf_list1,zf_list1[0],0);
                lcd_show(2,200,zf_list2,zf_list2[0],0);
                lcd_show(4,200,zf_list12,zf_list12[0],0);
                lcd_show(6,200,zf_list13,zf_list13[0],0);
                MS = 1;
                thread_time = 0;
                choose = 0;
            }
        }
        if(button_number == BUTTON_OK){
            if(choose_number < 30){
                set_MSID(zf_list6[0],zf_list6[1],zf_list6[2]);
                if(setting_time == 1){
                    setting_time = 0;
                    MS_thread_SetTime();
                }
                choose_number = 30;
            }
        }
    }
    if(choose_number == 30){
        if(rt_pin_read(BUTTON9) == 0){
            MS = 3;
            Dispgraphic(0x00);
            if(choose == 0)lcd_show(0,200,zf_list1,zf_list1[0],0);
            else if(choose == 1)lcd_show(0,200,zf_list2,zf_list2[0],0);
            else if(choose == 2)lcd_show(0,200,zf_list12,zf_list12[0],0);
            lcd_show(2,0,zf_list9,zf_list9[0],0);
            lcd_show_zf(2, 80, "  ");
        }
    }
}
void MS_thread_test0(){
    uint8_t i;
    uint32_t uart_data;
    uint32_t number;
    my_uart_SetData(0, zf_list6[choose], 6);
    if(get_uart_flag() != UART_READ_OK){
        MS = 4;MS_error = WRITE_ERROR;
        return;
    }
    rt_thread_mdelay(200);
    my_uart_SetData(1, zf_list7, 6);
    if(get_uart_flag() != UART_READ_OK){
        MS = 4;MS_error = WRITE_ERROR;
        return;
    }
    rt_thread_mdelay(200);
    my_uart_SetData(2, zf_list8, 6);
    if(get_uart_flag() != UART_READ_OK){
        MS = 4;MS_error = WRITE_ERROR;
        return;
    }
    lcd_show(2,0,zf_list9,zf_list9[0],0);
    lcd_show(4,0,zf_list10,zf_list10[0],0);
    lcd_show_zf(4, 80, "  ");
    low_speed();
    wait_pwm_finish();
    lcd_show(4,0,zf_list10,zf_list10[0],0);
    lcd_show(6,0,zf_list11,zf_list11[0],0);
    lcd_show_zf(6, 80, "  ");

    rt_thread_mdelay(800);
    my_uart_GetData(0, choose);
    if(get_uart_flag() != UART_READ_OK){
        rt_kprintf("read_error 123");
        MS = 4;MS_error = READ_ERROR;
        return;
    }else {
        uart_data = get_UartData();
        number = 0;
        for(i=0;i<6;i++){
            number += zf_list6[choose][i+1]-100;
            if(i < 5)number *= 10;
        }
        if(number != uart_data){
            rt_kprintf("msid error  %d    %d",uart_data,number);
            MS = 4;MS_error = MSID_ERROR;
            return;
        }
    }
    rt_thread_mdelay(800);
    my_uart_GetData(1, choose);
    if(get_uart_flag() != UART_READ_OK){
        rt_kprintf("read_error 333");
        MS = 4;MS_error = READ_ERROR;
        return;
    }else {
        uart_data = get_UartData();
        if(uart_data != 3){
            rt_kprintf("msv error  %d    %d",uart_data);
            msv_error = uart_data%10;
            MS = 4;MS_error = MSV_ERROR;
            return;
        }
    }
    rt_thread_mdelay(200);
    my_uart_GetData(2, choose);
    if(get_uart_flag() != UART_READ_OK){
        rt_kprintf("read_error 444");
        MS = 4;MS_error = READ_ERROR;
        return;
    }else {
        uart_data = get_UartData();
        if(uart_data & 0x02){
            rt_kprintf("msv interference  %d",uart_data);
            MS = 4;MS_error = MSV_INTERFERENCE;
            return;
        }else if(uart_data & 0x01){
            rt_kprintf("low power  %d",uart_data);
            MS = 4;MS_error = LOW_POWER;
            return;
        }
    }
    rt_thread_mdelay(200);
    my_uart_SetData(1, zf_list7, 6);
    if(get_uart_flag() != UART_READ_OK){
        MS = 4;MS_error = WRITE_ERROR1;
        return;
    }
    MS = 5;
    lcd_show(6,0,zf_list11,zf_list11[0],0);
    thread_time = 0;
}

void MS_thread_test1(){
    uint8_t i;
    uint32_t uart_data;
    uint32_t number;
    uint8_t len = 0;
    if(choose == 1)len = 8;
    else len = 6;

    my_uart_SetData(0, zf_list6[choose], len);
    if(get_uart_flag() != UART_READ_OK){
        MS = 4;MS_error = WRITE_ERROR;
        return;
    }
    rt_thread_mdelay(100);
    my_uart_SetData(1, zf_list7, 6);
    if(get_uart_flag() != UART_READ_OK){
        MS = 4;MS_error = WRITE_ERROR;
        return;
    }
    rt_thread_mdelay(100);
    my_uart_SetData(2, zf_list8, 6);
    if(get_uart_flag() != UART_READ_OK){
        MS = 4;MS_error = WRITE_ERROR;
        return;
    }

//    rt_thread_mdelay(100);
//    my_uart_GetData(1, choose);

    lcd_show(2,0,zf_list9,zf_list9[0],0);
    lcd_show(4,0,zf_list10,zf_list10[0],0);
    lcd_show_zf(4, 80, "  ");

    rt_pin_write(IR_DET, PIN_HIGH);
    rt_thread_mdelay(150);
    if(rt_pin_read(IR_LED) == 0){
        rt_pin_write(IR_DET, PIN_LOW);
        ms_open_valve();
        if(get_uart_flag() != UART_READ_OK){
            MS = 4;MS_error = WRITE_ERROR2;
            return;
        }else {
            rt_pin_write(IR_DET, PIN_HIGH);
            rt_thread_mdelay(100);
            if(rt_pin_read(IR_LED) == 0){
                rt_pin_write(IR_DET, PIN_LOW);
                ms_open_valve();
                if(get_uart_flag() != UART_READ_OK){
                    MS = 4;MS_error = WRITE_ERROR2;
                    return;
                }else {
                    rt_pin_write(IR_DET, PIN_HIGH);
                    rt_thread_mdelay(100);
                    if(rt_pin_read(IR_LED) == 0){
                        rt_pin_write(IR_DET, PIN_LOW);
                        MS = 4;MS_error = VALVE_ERROR;
                        return;
                    }
                }
            }
        }
    }
    rt_pin_write(IR_DET, PIN_LOW);
    high_speed();
    wait_pwm_finish();


    rt_pin_write(IR_DET, PIN_HIGH);
    rt_thread_mdelay(100);
    for(i=0;i<4;i++){
        rt_thread_mdelay(1000);
        if(rt_pin_read(IR_LED) == 0)i = 5;
    }
    if(rt_pin_read(IR_LED) == 1){
        rt_pin_write(IR_DET, PIN_LOW);
        MS = 4;MS_error = VALVE_ERROR;
        return;
    }
    rt_pin_write(IR_DET, PIN_LOW);
    lcd_show(4,0,zf_list10,zf_list10[0],0);
    lcd_show(6,0,zf_list11,zf_list11[0],0);
    lcd_show_zf(6, 80, "  ");

    rt_thread_mdelay(1000);
    my_uart_GetData(0, choose);
    if(get_uart_flag() != UART_READ_OK){
        rt_kprintf("read_error 123");
        MS = 4;MS_error = READ_ERROR;
        return;
    }else {
        uart_data = get_UartData();
        number = 0;
        for(i=0;i<len;i++){
            number += zf_list6[choose][i+1]-100;
            if(i < (len-1))number *= 10;
        }
        if(number != uart_data){
            rt_kprintf("msid error  %d    %d",uart_data,number);
            MS = 4;MS_error = MSID_ERROR;
            return;
        }
    }
    rt_thread_mdelay(600);
    my_uart_GetData(1, choose);
    if(get_uart_flag() != UART_READ_OK){
        rt_kprintf("read_error 333");
        MS = 4;MS_error = READ_ERROR;
        return;
    }else {
        uart_data = get_UartData();
        if(uart_data != 7){
            rt_kprintf("msv error  %d    %d",uart_data);
            msv_error = uart_data%10;
            MS = 4;MS_error = MSV_ERROR;
            return;
        }
    }
    rt_thread_mdelay(200);
    my_uart_GetData(2, choose);
    if(get_uart_flag() != UART_READ_OK){
        rt_kprintf("read_error 444");
        MS = 4;MS_error = READ_ERROR;
        return;
    }else {
        uart_data = get_UartData();
        if(uart_data & 0x02){
            rt_kprintf("Tamper  %d",uart_data);
            MS = 4;MS_error = TAMPER;
            return;
        }else if(uart_data & 0x01){
            rt_kprintf("low power  %d",uart_data);
            MS = 4;MS_error = LOW_POWER;
            return;
        }else if(uart_data & 0x04){
            rt_kprintf("critical_low_battery error %d",uart_data);
            MS = 4;MS_error = CRITICAL_LOW_BATTERY;
            return;
        }else if(!(uart_data & 0x40)){
            rt_kprintf("valve_status error  %d",uart_data);
            MS = 4;MS_error = VALVE_STATUS_ERROR;
            return;
        }else if(!(uart_data & 0x20)){
            rt_kprintf("critical escape  %d",uart_data);
            MS = 4;MS_error = CRITICAL_ESCAPE;
            return;
        }else if(uart_data & 0x10){
            rt_kprintf("minor escape error  %d",uart_data);
            MS = 4;MS_error = MINOR_ESCAPE_ERROR;
            return;
        }else if(uart_data & 0x08){
            rt_kprintf("inspect error  %d",uart_data);
            MS = 4;MS_error = INSPECT_ERROR;
            return;
        }else if((uart_data & 0x80)){
            rt_kprintf("valve connection error  %d",uart_data);
            MS = 4;MS_error = VALVE_CONNECTION_ERROR;
            return;
        }else if(uart_data & 0x100){
            rt_kprintf("valve error alarm  %d",uart_data);
            MS = 4;MS_error = VALVE_ERROR_ALARM;
            return;
        }else if(uart_data & 0x200){
            rt_kprintf("valve tightness error  %d",uart_data);
            MS = 4;MS_error = VALVE_TIGHTNESS_ERROR;
            return;
        }
    }
    rt_thread_mdelay(200);
    my_uart_SetData(1, zf_list7, 6);



    if(get_uart_flag() != UART_READ_OK){
        MS = 4;MS_error = WRITE_ERROR1;
        return;
    }

    rt_thread_mdelay(200);
    ms_open_valve();
    if(get_uart_flag() != UART_READ_OK){
        MS = 4;MS_error = WRITE_ERROR1;
        return;
    }
    rt_pin_write(IR_DET, PIN_HIGH);
    rt_thread_mdelay(100);
    if(rt_pin_read(IR_LED) == 0){
        rt_pin_write(IR_DET, PIN_LOW);
        ms_open_valve();
        if(get_uart_flag() != UART_READ_OK){
            MS = 4;MS_error = WRITE_ERROR1;
            return;
        }else {
            rt_pin_write(IR_DET, PIN_HIGH);
            rt_thread_mdelay(100);
            if(rt_pin_read(IR_LED) == 0){
                rt_pin_write(IR_DET, PIN_LOW);
                MS = 4;MS_error = VALVE_ERROR1;
                return;
            }
        }
    }

    MS = 5;
    lcd_show(6,0,zf_list11,zf_list11[0],0);
    thread_time = 0;
}

void MS_thread_ERROR(){
    if(MS_error){
        if(MS_error == WRITE_ERROR){
            lcd_show_zf(2, 0, "        ");
            lcd_show_zf(2, 0, "Write error");
        }else if(MS_error == VALVE_ERROR || MS_error == WRITE_ERROR2){
            lcd_show_zf(4, 0, "        ");
            if(MS_error == VALVE_ERROR)lcd_show_zf(4, 0, "Valve error");
            else lcd_show_zf(4, 0, "Write error");
        }else {
            lcd_show_zf(6, 0, "        ");
            if(MS_error == READ_ERROR)lcd_show_zf(6, 0, "read error");
            else if(MS_error == MSID_ERROR)lcd_show_zf(6, 0, "MSID error");
            else if(MS_error == MSV_ERROR){
                lcd_show_zf(6, 0, "MSV error");
                lcd_number(6,88,msv_error,0);
            }
            else if(MS_error == MSV_INTERFERENCE)lcd_show_zf(6, 0, "MSV interference");
            else if(MS_error == LOW_POWER)lcd_show_zf(6, 0, "Low power");
            else if(MS_error == WRITE_ERROR1)lcd_show_zf(6, 0, "Write error");
            else if(MS_error == TAMPER)lcd_show_zf(6, 0, "Tamper");
            else if(MS_error == CRITICAL_LOW_BATTERY)lcd_show_zf(6, 0, "Critcal Low Battery");
            else if(MS_error == VALVE_STATUS_ERROR)lcd_show_zf(6, 0, "Valve status error");
            else if(MS_error == CRITICAL_ESCAPE)lcd_show_zf(6, 0, "Critical escape");
            else if(MS_error == MINOR_ESCAPE_ERROR)lcd_show_zf(6, 0, "Minor escape error");
            else if(MS_error == INSPECT_ERROR)lcd_show_zf(6, 0, "Inspect error");
            else if(MS_error == VALVE_CONNECTION_ERROR)lcd_show_zf(6, 0, "Valve connection error");
            else if(MS_error == VALVE_ERROR_ALARM)lcd_show_zf(6, 0, "Valve error alarm");
            else if(MS_error == VALVE_TIGHTNESS_ERROR)lcd_show_zf(6, 0, "Valve tightness error");
            else if(MS_error == LOW_BATTERY)lcd_show_zf(6, 0, "Low Battery");
            else if(MS_error == VALVE_ERROR1)lcd_show_zf(6, 0, "Valve error");
        }
        MS_error = 0;
        rt_pin_write(BUZZ, PIN_HIGH);
        rt_thread_mdelay(200);
        rt_pin_write(BUZZ, PIN_LOW);
        rt_thread_mdelay(200);
        rt_pin_write(BUZZ, PIN_HIGH);
        rt_thread_mdelay(200);
        rt_pin_write(BUZZ, PIN_LOW);
        rt_thread_mdelay(200);
        rt_pin_write(BUZZ, PIN_HIGH);
        rt_thread_mdelay(200);
        rt_pin_write(BUZZ, PIN_LOW);
    }
    if(MS_error == 0 && (rt_pin_read(BUTTON9) == 0 || rt_pin_read(BUTTON10) == 0 || button_number == BUTTON_OK || button_number == BUTTON_RETUEN)){
        MS = 2;
        Dispgraphic(0x00);
        thread_time = 0;
        reverse = 0;
        choose_number = 30;
        if(choose == 0)lcd_show(0,200,zf_list1,zf_list1[0],0);
        else if(choose == 1)lcd_show(0,200,zf_list2,zf_list2[0],0);
        else if(choose == 2)lcd_show(0,200,zf_list12,zf_list12[0],0);
        lcd_show(2, 0, zf_list3, zf_list3[0], 0);
        lcd_show(4, 4, zf_list4, zf_list4[0], 0);
        lcd_show(6, 0, zf_list5, zf_list5[0], 0);

        lcd_show(2, 40, zf_list6[choose], zf_list6[choose][0], 0);
        lcd_show(4, 40, zf_list7, zf_list7[0], 0);

        MS_thread_GetTime();
        lcd_show(6, 40, zf_list8, zf_list8[0], 0);
    }
}

void MS_thread_return(){
    uint32_t number;
    uint8_t i;
    uint8_t len;

    number = 0;

    if(choose == 1)len = 8;
    else len = 6;

    for(i=0;i<len;i++){
        number += zf_list6[choose][i+1]-100;
        if(i < (len-1))number *= 10;
        else number += 1;
        rt_kprintf("number = %d\r\n",number);
    }

    for(i=0;i<len;i++){
        zf_list6[choose][len-i] = number%10 + 100;
        number /= 10;
    }
    set_MSID(zf_list6[0],zf_list6[1],zf_list6[2]);
    MS = 2;
    Dispgraphic(0x00);
    thread_time = 0;
    reverse = 0;
    choose_number = 30;
    if(choose == 0)lcd_show(0,200,zf_list1,zf_list1[0],0);
    else if(choose == 1)lcd_show(0,200,zf_list2,zf_list2[0],0);
    else if(choose == 2)lcd_show(0,200,zf_list12,zf_list12[0],0);
    lcd_show(2, 0, zf_list3, zf_list3[0], 0);
    lcd_show(4, 4, zf_list4, zf_list4[0], 0);
    lcd_show(6, 0, zf_list5, zf_list5[0], 0);

    lcd_show(2, 40, zf_list6[choose], zf_list6[choose][0], 0);
    lcd_show(4, 40, zf_list7, zf_list7[0], 0);

    MS_thread_GetTime();
    lcd_show(6, 40, zf_list8, zf_list8[0], 0);
}

void MS_thread_test(){
    rt_pin_write(BUZZ, PIN_HIGH);
    rt_thread_mdelay(200);
    rt_pin_write(BUZZ, PIN_LOW);
    MS_thread_tick = rt_tick_get();
    MS_LED_off();
    MS_error = 0;
    if(choose == 0)MS_thread_test0();
    else MS_thread_test1();
    MS_thread_tick = rt_tick_get() - MS_thread_tick;
    rt_kprintf("MS Test %d ms",MS_thread_tick);
    get_time(my_time);
    set_record(choose,zf_list6[choose],MS_error,my_time);
    set_Record_txt(get_recordled_data()+1, get_ax5045len_data(), get_version());
    if(MS_error)MS_LED_error();
    else MS_LED_ok();
}
void ax_time_show(){
    uint8_t zz[25];
    uint8_t k;
    k = 0;
    zz[k++] = '2';
    zz[k++] = '0';
    zz[k++] = my_time[6]/10 + '0';
    zz[k++] = my_time[6]%10 + '0';
    zz[k++] = '-';
    zz[k++] = my_time[5]/10 + '0';
    zz[k++] = my_time[5]%10 + '0';
    zz[k++] = '-';
    zz[k++] = my_time[4]/10 + '0';
    zz[k++] = my_time[4]%10 + '0';
    zz[k++] = ' ';
    zz[k++] = my_time[2]/10 + '0';
    zz[k++] = my_time[2]%10 + '0';
    zz[k++] = ':';
    zz[k++] = my_time[1]/10 + '0';
    zz[k++] = my_time[1]%10 + '0';
    zz[k++] = ':';
    zz[k++] = my_time[0]/10 + '0';
    zz[k++] = my_time[0]%10 + '0';
    zz[k++] = 0;
    if(time_number < 20 && reverse == 1)lcd_show_zf_time(0, 0, zz, time_number);
    else lcd_show_zf6_8(0,0,zz);
}
void show_ax_data(uint32_t msid,uint32_t msv,uint8_t v,uint8_t data[10],int rssi,uint16_t addr){
    uint8_t i;

    lcd_show16_32(4,176,rssi%10);
    lcd_show16_32(4,160,rssi%100/10);
    lcd_show16_32(4,144,rssi%1000/100);

    lcd_show6_8(1,60,v/2,0);
    lcd_show6_8(1,66,v%2,0);

    if(v == 0){
        for(i=0;i<8;i++){
            lcd_show6_8(2,78-i*6,msid%10,0);
            msid/=10;
        }
    }else {
        for(i=0;i<6;i++){
            lcd_show6_8(2,66-i*6,msid%10,0);
            msid/=10;
        }
        lcd_show6_8(2,72,10,0);
        lcd_show6_8(2,78,10,0);
    }
    for(i=0;i<6;i++){
        lcd_show6_8(3,66-i*6,msv%10,0);
        msv/=10;
    }
    lcd_show6_8(4,66,data[0],0);
    lcd_show6_8(5,66,data[1],0);
    lcd_show6_8(6,66,data[2],0);
    lcd_show6_8(7,66,data[3],0);
    lcd_show6_8(1,108,data[4],0);
    lcd_show6_8(2,108,data[5],0);
    lcd_show6_8(3,108,data[6],0);
    lcd_show6_8(4,108,data[7],0);
    lcd_show6_8(5,108,data[8],0);
    lcd_show6_8(6,108,data[9],0);


    lcd_show6_8(7, 78, addr/10000%10, 0);
    lcd_show6_8(7, 84, addr/1000%10, 0);
    lcd_show6_8(7, 90, addr/100%10, 0);
    lcd_show6_8(7, 96, addr/10%10, 0);
    lcd_show6_8(7, 102, addr%10, 0);
    lcd_show6_8(7, 108, 35, 0);
    addr = stat_sample()-1;
    lcd_show6_8(7, 114, addr/10000%10, 0);
    lcd_show6_8(7, 120, addr/1000%10, 0);
    lcd_show6_8(7, 126, addr/100%10, 0);
    lcd_show6_8(7, 132, addr/10%10, 0);
    lcd_show6_8(7, 138, addr%10, 0);
}
uint8_t get_thread_MS(){
    return MS;
}
uint8_t set_thread_MS(){
    MS = 12;
}

void MS_thread_ax_refresh(){
    uint8_t data[10];
    uint16_t addr = 0;
    if(get_ax5045len_data() == 0){
        lcd_show_zf6_8(1,0,"VERSION:  00   DL:0");
        lcd_show_zf6_8(2,0,"MSID:          XL:0");
        lcd_show_zf6_8(3,0,"MSV:  950703   AJ:0");
        lcd_show_zf6_8(4,0,"TAMPER:    0 V-DK:0");
        lcd_show_zf6_8(5,0,"BATTERY:   0 V-DL:0");
        lcd_show_zf6_8(6,0,"B-LOW:     0 V-QM:0");
        lcd_show_zf6_8(7,0,"VALVE:     0");

        lcd_show16_32(0,128,11);
        lcd_show16_32(0,144,12);
        lcd_show16_32(0,160,12);
        lcd_show16_32(0,176,13);
        lcd_show16_32(4,128,10);
    }
    get_ax_flag(data);
    set_ax_record(get_ax_v(),data,get_ax_msid(),get_ax_msv(),my_time,-get_ax_rssi());
    set_Record_txt(get_recordled_data(), get_ax5045len_data()+1, get_version());
    addr = get_ax5045len_data();
    if(addr == 0)addr = AX5043_MAX;
    show_ax_data(get_ax_msid(),get_ax_msv(),get_ax_v(),data,-get_ax_rssi(),addr-1);
    get_date(my_time);

    show_ax_addr = get_ax5045len_data();
    rt_pin_write(BUZZ, PIN_HIGH);
    rt_thread_mdelay(150);
    rt_pin_write(BUZZ, PIN_LOW);
    MS = 11;
}

void show_ax_record(uint16_t addr){
    uint32_t msid;
    uint32_t msv;
    uint8_t data[10];
    uint8_t version;
    uint16_t rssi;
    uint8_t i;

    get_ax_Record(addr);

    version = get_ax_data(0)-'0';

    msid = 0;
    for(i=0;i<8;i++){
        msid += get_ax_data(2+i) - '0';
        if(i < 7)msid *= 10;
    }
    msv = 0;
    for(i=0;i<6;i++){
        msv += get_ax_data(11+i) - '0';
        if(i < 5)msv *= 10;
    }

    for(i=0;i<10;i++)data[i] = get_ax_data(18+i*2) - '0';
    rssi = (get_ax_data(56)-'0')*100+(get_ax_data(57)-'0')*10+(get_ax_data(58)-'0');
    show_ax_data(msid, msv, version, data, rssi,addr);
}
void MS_thread_ax_start(){


    Dispgraphic(0x00);
    get_date(my_time);
    ax_time_show();
    show_ax_addr = get_ax5045len_data();
    if(show_ax_addr){
        lcd_show_zf6_8(1,0,"VERSION:  00   DL:0");
        lcd_show_zf6_8(2,0,"MSID:          XL:0");
        lcd_show_zf6_8(3,0,"MSV:  950703   AJ:0");
        lcd_show_zf6_8(4,0,"TAMPER:    0 V-DK:0");
        lcd_show_zf6_8(5,0,"BATTERY:   0 V-DL:0");
        lcd_show_zf6_8(6,0,"B-LOW:     0 V-QM:0");
        lcd_show_zf6_8(7,0,"VALVE:     0");

        lcd_show16_32(0,128,11);
        lcd_show16_32(0,144,12);
        lcd_show16_32(0,160,12);
        lcd_show16_32(0,176,13);
        lcd_show16_32(4,128,10);

        show_ax_record(show_ax_addr-1);
    }
    time_number = 20;
    MS = 11;
}
void MS_thread_ax(){
    uint8_t time_addr;
    uint8_t time_list[3];
    if((thread_time%10) == 0){
        reverse+=1;
        reverse%=2;
        if(time_number == 20)get_date(my_time);
        ax_time_show();
    }
    if(button_number != 16){
        if(button_number < 10 && time_number < 12){
            if(time_number < 6)time_addr = 6-time_number/2;
            else time_addr = 5-time_number/2;
            if(time_number%2)my_time[time_addr] = my_time[time_addr]/10*10+button_number;
            else my_time[time_addr] = my_time[time_addr]%10 + button_number * 10;

            time_number += 1;
            time_number %= 12;
            thread_time = 9;
            reverse = 1;
        }
        if(button_number == BUTTON_UP || button_number == BUTTON_LEFT){
            if(time_number == 20){
                if(get_ax5045len_data()){
                    if(show_ax_addr)show_ax_addr-=1;
                    else show_ax_addr = stat_sample()-1;
                    show_ax_record(show_ax_addr);
                    rt_thread_mdelay(200);
                }
            }else {
                if(time_number)time_number--;
                else time_number = 11;
                thread_time = 9;
                reverse = 1;
            }
        }
        if(button_number == BUTTON_DOWN || button_number == BUTTON_RIGHT){
            if(time_number == 20){
                if(get_ax5045len_data()){
                    show_ax_addr += 1;
                    show_ax_addr %= stat_sample();
                    show_ax_record(show_ax_addr);
                    rt_thread_mdelay(200);
                }
            }else {
                time_number += 1;
                time_number %= 12;
                thread_time = 9;
                reverse = 1;
            }
        }
        if(button_number == BUTTON_OK){
            if(time_number == 20){
                time_number = 0;
                thread_time = 9;
                reverse = 1;
            }else {
                if(my_time[5] < 0)my_time[5] = 1;
                else if(my_time[5] > 12)my_time[5] = 12;
                if(my_time[4] < 0)my_time[4] = 1;
                else if(my_time[4] > 31)my_time[4] = 31;
                time_list[0] = my_time[4];
                time_list[1] = my_time[5];
                time_list[2] = my_time[6];
                set_date(time_list);

                if(my_time[2] > 23)my_time[2] = 23;
                if(my_time[1] > 59)my_time[1] = 59;
                if(my_time[0] > 59)my_time[0] = 59;
                set_time(my_time);
                time_number = 20;
                thread_time = 9;
                reverse = 1;

            }
        }
        if(button_number == BUTTON_RETUEN){
            if(time_number < 20){
                time_number = 20;
                thread_time = 9;
            }else {
                Dispgraphic(0x00);
                lcd_show(0,200,zf_list1,zf_list1[0],0);
                lcd_show(2,200,zf_list2,zf_list2[0],0);
                lcd_show(4,200,zf_list12,zf_list12[0],0);
                lcd_show(6,200,zf_list13,zf_list13[0],0);
                MS = 1;
                thread_time = 0;
                choose = 0;
            }
        }
    }
}
void MS_thread(){
    switch(MS){
    case 0:MS_thread_init();break;
    case 1:MS_thread_SelectMode();break;
    case 2:MS_thread_SettingMs();break;
    case 3:MS_thread_test();break;
    case 4:MS_thread_ERROR();break;
    case 5:if(thread_time < 20)rt_pin_write(BUZZ, PIN_HIGH);else rt_pin_write(BUZZ, PIN_LOW);
        if(thread_time >= (3*20))MS_thread_return();break;
    case 10:MS_thread_ax_start();break;
    case 11:MS_thread_ax();break;
    case 12:MS_thread_ax_refresh();break;
    }
}

int main(void)
{
    rt_thread_mdelay(500);
    my_dfs_init();
    MS_LED_init();
    my_button_init();
    pwm_sample();
    uart_data_sample();
    Init_lcd();
    rf_4068_start();

//    my_usb_init();
    while (1)
    {
        button_number = get_button_number();
        MS_thread();
        thread_time++;
        thread_time %= 100;
        rt_thread_mdelay(50);
    }

    return RT_EOK;
}
