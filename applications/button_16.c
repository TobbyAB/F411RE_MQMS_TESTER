/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-05-20     Gwvt       the first version
 */

#include "button_16.h"

uint8_t button_list[8] = {
        BUTTON8,
        BUTTON7,
        BUTTON6,
        BUTTON5,
        BUTTON4,
        BUTTON3,
        BUTTON2,
        BUTTON1,
};

static uint16_t button_data[16] = {
        1,2,3,10,
        4,5,6,11,
        7,8,9,12,
        13,0,14,15,
};


static uint8_t button = 16;
static uint8_t button_lock = 16;

void my_button_init(){
    uint8_t i;
    for(i=0;i<4;i++){
        rt_pin_mode(button_list[i], PIN_MODE_OUTPUT);
        rt_pin_write(button_list[i], PIN_LOW);
        rt_pin_mode(button_list[i+4], PIN_MODE_INPUT_PULLDOWN);
    }
    rt_pin_mode(BUTTON9, PIN_MODE_INPUT);
    rt_pin_mode(BUTTON10, PIN_MODE_INPUT);
}

uint8_t get_button_number(){
    uint8_t i,j;
    button = 16;
    for(i=0;i<4;i++)rt_pin_write(button_list[i], PIN_LOW);
    for(i=0;i<4;i++){
        rt_pin_write(button_list[i], PIN_HIGH);
        for(j=0;j<4;j++){
            if(rt_pin_read(button_list[j+4]) == PIN_HIGH){
                if(button_lock == 16){
                    button = button_data[j+i*4];
                    button_lock = button;
                    return button;
                }else {
//                    return button_data[j+i*4];
                    return 16;
                }
            }
        }
        rt_pin_write(button_list[i], PIN_LOW);
    }
    button_lock = 16;
    return 16;

}
