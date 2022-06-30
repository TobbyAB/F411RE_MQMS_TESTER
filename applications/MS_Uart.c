/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-05-25     Gwvt       the first version
 */
#include "MS_Uart.h"

static struct rt_semaphore rx_sem;
static rt_device_t serial;
static uint8_t data_end = 0;
static uint8_t data_len;
static uint8_t uart_timeot = 0;
static uint8_t uart_flag = 0;
static uint8_t uart_commend;
char data[ONE_DATA_MAXLEN];
static uint8_t MS_Set_Command[3] = {'A','E','G'};
static uint8_t MS_Get_Command[3] = {'B','F','I'};
static uint32_t uart_get_data;
uint8_t number_hex(uint8_t number){
    if(number >= 10)return number-10+'A';
    else return number+'0';
}
uint8_t hex_number(uint8_t hex){
    if(hex >= 'A' && hex <= 'F')return hex-'A'+10;
    else if(hex >= 'a' && hex <= 'f')return hex-'a'+10;
    else if(hex >= '0' && hex <= '9')return hex-'0';
}

static rt_err_t uart_rx_ind(rt_device_t dev, rt_size_t size)
{
    /* 串口接收到数据后产生中断，调用此回调函数，然后发送接收信号量 */
    if (size > 0)
    {
        rt_sem_release(&rx_sem);
    }
    return RT_EOK;
}

static char uart_sample_get_char(void)
{
    char ch;

    while (rt_device_read(serial, 0, &ch, 1) == 0)
    {
        rt_sem_control(&rx_sem, RT_IPC_CMD_RESET, RT_NULL);
        rt_sem_take(&rx_sem, RT_WAITING_FOREVER);
    }
    return ch;
}

static void my_data_parsing(){
    uint16_t hex_check;
    uint8_t i;
    hex_check = 0;
    for(i=0;i< (data_len-4);i++){
        hex_check += data[i+1];
        hex_check %= 0x100;
    }
    if(data[data_len - 1] == '*' && data[data_len-3]  == number_hex(hex_check/0x10) && data[data_len-2] == number_hex(hex_check%0x10)){
        if(data[2] == 'A' || data[2] == 'E' || data[2] == 'G'){
            if(data[3] == '0')uart_flag = UART_READ_OK;
            else uart_flag = UART_ERROR_DATA;
        }else if(data[2] == 'B' || data[2] == 'F'){
            uart_get_data = 0;
            uart_flag = UART_READ_OK;
            for(i=0;i<(data_len-6);i++){
                uart_get_data += hex_number(data[3+i]);
                if(i < (data_len-7))uart_get_data *= 0x10;
//                rt_kprintf("\r\n%d\r\n",uart_get_data);
            }
        }else if(data[2] == 'I'){
            uart_flag = UART_READ_OK;
            if(data_len == 8){
                data[4] = hex_number(data[4]);
                uart_get_data = 0;
                if(data[4]&0x01)uart_get_data |= 0x01;      //低电量标志
                if(data[4]&0x02)uart_get_data |= 0x02;      //干扰标志
            }
            else if(data_len == 11){
                data[4] = hex_number(data[4]);
                data[5] = hex_number(data[5]);
                data[6] = hex_number(data[6]);
                data[7] = hex_number(data[7]);

                uart_get_data = 0;
                if(data[4]&0x01)uart_get_data |= 0x01;      //低电量标志
                if(data[4]&0x02)uart_get_data |= 0x02;      //干扰标志
                if(data[5]&0x01)uart_get_data |= 0x04;      //超低电量标志
                if(data[6]&0x01)uart_get_data |= 0x08;      //安检逾期标志
                if(data[6]&0x02)uart_get_data |= 0x10;      //微漏标志
                if(data[6]&0x04)uart_get_data |= 0x20;      //超漏标志
                if(data[6]&0x08)uart_get_data |= 0x40;      //阀门标志状态
                if(data[7]&0x01)uart_get_data |= 0x80;      //阀门连接标志
                if(data[7]&0x02)uart_get_data |= 0x100;     //阀门故障标志（堵转）
                if(data[7]&0x04)uart_get_data |= 0x200;     //气密性标志
            }
//            rt_kprintf("\r\n%d\r\n",uart_get_data);
        }else if(data[2] == 'R'){
            uart_flag = UART_READ_OK;
            uart_get_data = data[data_len-4];
//            rt_kprintf("uart_get_data = %c  %d",uart_get_data,uart_get_data);
        }
    }else {
        rt_kprintf("xiao yan error");
        uart_flag = UART_ERROR_DATA;
    }

    uart_timeot = 0;
}

/* 数据解析线程 */
static void data_parsing(void)
{
    char ch;
    data_end = 0;
    while (1)
    {
        ch = uart_sample_get_char();
        rt_kprintf("%c",ch);
        if(data_end){
            if((data_len == 0 && ch == '@') || (data_len == 1 && ch == 'A') || (data_len >= 3) || (data_len == 2 && ch == uart_commend)){
                data[data_len++] = ch;
                uart_timeot = 10;
            }
            if(data_len == data_end)my_data_parsing();
        }
    }
}
static void data_timeout_parsing(void)
{
    while (1)
    {
        if(uart_timeot){
            uart_timeot--;
        }
        if(uart_flag == UART_WAIT && uart_timeot == 0){
//            rt_kprintf("uart_read_timeout");
            uart_flag = UART_ERROR_TIMEOUT;
        }
        rt_thread_mdelay(50);
    }
}
void uart_data_sample()
{
    struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;

    serial = rt_device_find(MY_UART_MODBUS);
    if (!serial)
    {
        rt_kprintf("uart modubs failed!\n");
    }

    config.baud_rate = 600;
    config.data_bits = DATA_BITS_8;
    config.stop_bits = STOP_BITS_1;
    config.bufsz     = 128;
    config.parity    = PARITY_NONE;

    rt_sem_init(&rx_sem, "rx_sem", 0, RT_IPC_FLAG_FIFO);
    rt_device_control(serial, RT_DEVICE_CTRL_CONFIG, &config);
    /* 以中断接收及轮询发送模式打开串口设备 */
    rt_device_open(serial, RT_DEVICE_FLAG_INT_RX);
    /* 设置接收回调函数 */
    rt_device_set_rx_indicate(serial, uart_rx_ind);
    /* 创建 serial 线程 */
    rt_thread_t thread = rt_thread_create("serial", (void (*)(void *parameter))data_parsing, RT_NULL, 1024, 11, 10);
    rt_thread_t thread1 = rt_thread_create("timeout_uart", (void (*)(void *parameter))data_timeout_parsing, RT_NULL, 1024, 12, 10);
    /* 创建成功则启动线程 */
    if (thread != RT_NULL)
    {
        rt_thread_startup(thread);
    }
    else
    {
        rt_kprintf("uart thread error \n");
    }
    if(thread1 != RT_NULL)rt_thread_startup(thread1);

}

uint8_t fs_data[40] = "@AB83*";

void ms_uart_fs(){
    fs_data[0] = 0xff;
    fs_data[1] = '@';
    fs_data[2] = 'A';
    fs_data[3] = 'B';
    fs_data[4] = '8';
    fs_data[5] = '3';
    fs_data[6] = '*';
    fs_data[7] = 0x0D;
    rt_device_write(serial, 0, fs_data, 8);
}
void ms_uart_fs1(){
    fs_data[0] = 0xff;
    fs_data[1] = '@';
    fs_data[2] = 'A';
    fs_data[3] = 'I';
    fs_data[4] = '8';
    fs_data[5] = 'A';
    fs_data[6] = '*';
    fs_data[7] = 0x0D;
    rt_device_write(serial, 0, fs_data, 8);
}
void ms_uart_fs2(){
    fs_data[0] = 0xff;
    fs_data[1] = '@';
    fs_data[2] = 'A';
    fs_data[3] = 'F';
    fs_data[4] = '8';
    fs_data[5] = '7';
    fs_data[6] = '*';
    fs_data[7] = 0x0D;
    rt_device_write(serial, 0, fs_data, 8);
}
void my_uart_SetData(uint8_t flag,uint8_t zf_list[10],uint8_t len){
    uint8_t i,k = 0;
    uint16_t number = 0;
    uint32_t list_number = 0;

    if(flag != 2){
        for(i=0;i<len;i++){
            list_number += zf_list[i+1]-100;
            if(i != (len-1))list_number *= 10;
        }
    }else {
        list_number = (zf_list[1]-100)*36000+(zf_list[2]-100)*3600+(zf_list[4]-100)*600+(zf_list[5]-100)*60+(zf_list[7]-100)*10+zf_list[8]-100;
        list_number *= 2;
    }
//    rt_kprintf("list_number = %d\r\n",list_number);

    k = 0;
    fs_data[k++] = 0xff;
    fs_data[k++] = '@';
    fs_data[k++] = 'A';
    fs_data[k++] = MS_Set_Command[flag];
    uart_commend = MS_Set_Command[flag];
    for(i=0;i<len-1;i++){
        fs_data[k+(len-2)-i] = number_hex(list_number%0x10);
        list_number /= 0x10;
    }
    k+=(len-1);

    number = 0;
    for(i=0;i<len+1;i++){
        number += fs_data[2+i];
        number %= 0x100;
    }
    fs_data[k++] = number_hex(number/0x10);
    fs_data[k++] = number_hex(number%0x10);

    fs_data[k++] = '*';
    fs_data[k++] = 0x0D;
    rt_device_write(serial, 0, fs_data, k);

    data_len = 0;
    data_end = 7;
    uart_flag = UART_WAIT;
    uart_timeot = 40;
}

void my_uart_GetData(uint8_t flag,uint8_t choose){
    uint16_t number = 0;
    fs_data[0] = 0xff;
    fs_data[1] = '@';
    fs_data[2] = 'A';
    fs_data[3] = MS_Get_Command[flag];
    uart_commend = MS_Get_Command[flag];
    fs_data[6] = '*';
    fs_data[7] = 0x0D;
    number = (fs_data[2]+fs_data[3])%0x100;
    fs_data[4] = number_hex(number/0x10);
    fs_data[5] = number_hex(number%0x10);

    if(flag == 0){
        if(choose == 0)data_end = 12;
        else data_end = 14;
    }else if(flag == 1){
        data_end = 12;
    }else if(flag == 2){
        if(choose == 0)data_end = 8;
        else data_end = 11;
    }
    data_len = 0;
    uart_flag = UART_WAIT;
    uart_timeot = 80;

    rt_device_write(serial, 0, fs_data, 8);
}
uint32_t get_UartData(){
    return uart_get_data;
}
uint8_t get_uart_flag(){
    while(uart_timeot){
        rt_thread_mdelay(100);
    }
    if(uart_flag != UART_READ_OK)rt_kprintf("uart error %d\r\n",uart_flag);
    return uart_flag;
}

void ms_open_valve(){
    uint8_t k,i;
    uint16_t number = 0;
    k = 0;
    fs_data[k++] = 0xff;
    fs_data[k++] = '@';
    fs_data[k++] = 'A';
    fs_data[k++] = 'R';
    uart_commend = 'R';
    fs_data[k++] = '0';
    for(i=0;i<24;i++)fs_data[k++] = "F";
    number = 0;
    for(i = 2;i < k;i++){
        number += fs_data[i];
        number %= 0x100;
    }
    fs_data[k++] = number_hex(number/0x10);
    fs_data[k++] = number_hex(number%0x10);
    fs_data[k++] = '*';
    fs_data[k++] = 0x0D;

    data_len = 0;
    data_end = 33;
    uart_flag = UART_WAIT;
    uart_timeot = 120;

    rt_device_write(serial, 0, fs_data, k);
}
MSH_CMD_EXPORT(ms_uart_fs,ms uart fs);
MSH_CMD_EXPORT(ms_uart_fs1,ms uart fs1);
MSH_CMD_EXPORT(ms_uart_fs2,ms uart fs2);
