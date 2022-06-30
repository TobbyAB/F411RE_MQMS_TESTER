/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-05-27     Gwvt       the first version
 */
#ifndef APPLICATIONS_MY_DFS_H_
#define APPLICATIONS_MY_DFS_H_

#include <rtthread.h>
#include <dfs_posix.h>

#define RECORD_MAX  50000
#define AX5043_MAX  50000

void get_MSID(uint8_t *data);
void set_MSID(uint8_t msid1[10],uint8_t msid2[10],uint8_t msid3[10]);
void set_time(uint8_t my_time[3]);
void set_record(uint8_t choose,uint8_t zf_list[10],uint8_t ms_error,uint8_t ms_time[3]);
void set_Record_txt(uint16_t r_len,uint16_t a_len,uint8_t v);
uint16_t get_recordled_data();
uint16_t get_ax5045len_data();
uint8_t get_version();
#endif /* APPLICATIONS_MY_DFS_H_ */
