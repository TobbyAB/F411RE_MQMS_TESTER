/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-05-20     Gwvt       the first version
 */
#ifndef APPLICATIONS_LCD_19264_H_
#define APPLICATIONS_LCD_19264_H_

#include <rtthread.h>
#include <rtdbg.h>
#include "rtdevice.h"

#define LCD_SCK     0x1d
#define LCD_SDA     0x1f
#define LCD_RST     0x1e
#define LCD_CD      0x26
#define LCD_NSS     0x1c
#define LCD_BK      0x27

#define CS_1()              rt_pin_write(LCD_NSS,1)                                               //片选至 1
#define CS_0()              rt_pin_write(LCD_NSS,0)                                         //片选至 0

#define CD_1()              rt_pin_write(LCD_CD,1)                                               //指令/数据至 1
#define CD_0()              rt_pin_write(LCD_CD,0)                                         //指令/数据至 0

#define RST_1()             rt_pin_write(LCD_RST,1)                                          //复位至 1
#define RST_0()             rt_pin_write(LCD_RST,0)                                        //复位至 0

#define SCK_1()             rt_pin_write(LCD_SCK,1)                                          //时钟至 1
#define SCK_0()             rt_pin_write(LCD_SCK,0)                                        //时钟至 0

#define SDA_1()             rt_pin_write(LCD_SDA,1)                                          //数据至 1
#define SDA_0()             rt_pin_write(LCD_SDA,0)                                        //数据至 0

void Init_lcd(void);
void Dispgraphic(unsigned char D);
void lcd_number(unsigned char y,unsigned char l,unsigned char k,unsigned char reverse);
void lcd_chinese(unsigned char y,unsigned char l,unsigned char k,unsigned char reverse);
void lcd_show(uint8_t y,uint8_t l,uint8_t list[20],uint8_t len,uint8_t reverse);
void lcd_show_zf(uint8_t y,uint8_t l,char *data);
#endif /* APPLICATIONS_LCD_19264_H_ */
