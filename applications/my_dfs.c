/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-05-27     Gwvt       the first version
 */

#include "my_dfs.h"

char buffer[80];
static uint16_t record_len = 0;
static uint16_t ax5043_len = 0;
static uint8_t version = 1;

uint16_t get_recordled_data(){
    return record_len;
}
uint16_t get_ax5045len_data(){
    return ax5043_len;
}
uint8_t get_version(){
    return version;
}

void set_Record_txt(uint16_t r_len,uint16_t a_len,uint8_t v){
    int fd;

    fd = open("/Record_len.txt", O_RDWR | O_CREAT);

    if(fd >= 0){
        r_len %= RECORD_MAX;
        buffer[0] = r_len/0x100;
        buffer[1] = r_len%0x100;
        a_len %= (AX5043_MAX+1);
        buffer[2] = a_len/0x100;
        buffer[3] = a_len%0x100;

        buffer[4] = v;

        write(fd, buffer, 5);
        close(fd);
        record_len = r_len;
        ax5043_len = a_len;
    }else {
        rt_kprintf("Record_len.txt error\r\n");
    }
}

void AX_delete(){
    set_Record_txt(record_len,0,version);
    unlink("/AX_record.txt");
    rt_kprintf("ax delete ok\r\n");
}
void MS_delete(){
    set_Record_txt(0,ax5043_len,version);
    unlink("/MS_record.txt");
    rt_kprintf("ms delete ok\r\n");

}
MSH_CMD_EXPORT(AX_delete,ax delete);
MSH_CMD_EXPORT(MS_delete,MS delete);

void get_Record_len(){
    int fd, size;

    fd = open("/Record_len.txt", O_RDWR | O_CREAT);

    if(fd >= 0){
        size = read(fd,buffer,5);
        close(fd);
        if(size < 5){
            record_len = 0;
            ax5043_len = 0;
            version = 0;
        }else {
            record_len = buffer[0]*0x100+buffer[1];
            ax5043_len = buffer[2]*0x100+buffer[3];
//            version = buffer[4];
            if(version != buffer[4]){
                unlink("/MS_record.txt");
                unlink("/AX_record.txt");
                set_Record_txt(0,0,version);
                record_len = 0;
                ax5043_len = 0;
            }
        }
        rt_kprintf("%d",record_len);
    }else {
        rt_kprintf("Record_len.txt error\r\n");
    }
}
void unlink_record(){
    unlink("/MS_record.txt");
}
MSH_CMD_EXPORT(unlink_record,unlink record);

void my_dfs_init(){
    get_Record_len();
}

void get_MSID(uint8_t *data){
    int fd, size;
    uint8_t i;

    fd = open("/MS_MSID.txt", O_RDWR | O_CREAT);
    if (fd>= 0)
    {
        size = read(fd,buffer,sizeof(buffer));
        close(fd);
        if(size < 20){
            for(i=0;i<6;i++)buffer[i] = i+1,buffer[i+14] = i+1;
            for(i=0;i<8;i++)buffer[i+6] = i+1;
            fd = open("/MS_MSID.txt", O_WRONLY);
            if(fd >= 0){
                write(fd, buffer, 20);
                close(fd);
            }
        }
        for(i=0;i<20;i++){
            if(buffer[i] < 10)data[i] = buffer[i];
            else data[i] = 1;
        }
        rt_kprintf("MSID6:");
        for(i=0;i<6;i++)rt_kprintf("%d",buffer[i]);
        rt_kprintf("\r\nMSID8:");
        for(i=0;i<8;i++)rt_kprintf("%d",buffer[6+i]);
        rt_kprintf("\r\nMSID6:");
        for(i=0;i<6;i++)rt_kprintf("%d",buffer[14+i]);
    }else {
        rt_kprintf("MS_MSID.txt error\r\n");
    }
}
void set_MSID(uint8_t msid1[10],uint8_t msid2[10],uint8_t msid3[10]){
    int fd;
    uint8_t i;

    fd = open("/MS_MSID.txt", O_WRONLY);
    if (fd>= 0)
    {
        for(i=0;i<8;i++){
            if(i < 6){
                buffer[i] = msid1[i+1]-100;
                buffer[i+14] = msid3[i+1]-100;
            }
            buffer[i+6] = msid2[i+1]-100;
        }
        write(fd, buffer, 20);
        close(fd);
    }else {
        rt_kprintf("MS_MSID.txt error\r\n");
    }
}
static uint8_t ms_number_hex(uint8_t data){
    if(data < 10)return data+'0';
    else return data-10+'A';
}
void set_record(uint8_t choose,uint8_t zf_list[10],uint8_t ms_error,uint8_t ms_time[3]){
    int fd;
    static uint8_t i,k;


    fd = open("/MS_record.txt", O_RDWR | O_CREAT);
    if (fd>= 0)
    {
        k = 0;
        if(choose == 1){
            for(i=0;i<8;i++)buffer[k++] = zf_list[i+1]-100+'0';
        }else {
            for(i=0;i<6;i++)buffer[k++] = zf_list[i+1]-100+'0';
        }

        buffer[k++] = ' ';

        buffer[k++] = ms_number_hex(ms_error);

        buffer[k++] = ' ';

        buffer[k++] = ms_number_hex(ms_time[2]/10);
        buffer[k++] = ms_number_hex(ms_time[2]%10);
        buffer[k++] = ',';
        buffer[k++] = ms_number_hex(ms_time[1]/10);
        buffer[k++] = ms_number_hex(ms_time[1]%10);
        buffer[k++] = ',';
        buffer[k++] = ms_number_hex(ms_time[0]/10);
        buffer[k++] = ms_number_hex(ms_time[0]%10);

        if(choose != 1){
            buffer[k++] = ' ';
            if(choose == 0)buffer[k++] = 'A';
            else if(choose == 2)buffer[k++] = 'B';
        }

        buffer[k++] = 0x0d;
        lseek(fd,record_len*20,SEEK_SET);
        write(fd, buffer, k);
        close(fd);
    }else {
        rt_kprintf("MS_record.txt error\r\n");
    }
}
void set_ax_record(uint8_t version,uint8_t zf_list[10],uint32_t msid,uint32_t msv,uint8_t my_time[7],uint16_t rssi){
    int fd;
    static uint8_t i,k,len;
    uint16_t ax_len = 0;

    fd = open("/AX_record.txt", O_RDWR | O_CREAT);
    if (fd>= 0)
    {
        k = 0;
        buffer[k++] = version + '0';
        buffer[k++] = ' ';

        len = 8;
//        rt_kprintf("misd = ");
        for(i=0;i<len;i++){
            buffer[k+len-1-i] = msid%10 + '0';
//            rt_kprintf("%c",buffer[k+len-1-i]);
            msid/=10;
        }
//        rt_kprintf("\r\n");
        k+=len;
        buffer[k++] = ' ';

        len = 6;
        for(i=0;i<len;i++){
            buffer[k+len-1-i] = msv%10 + '0';
            msv/=10;
        }
        k+=len;
        buffer[k++] = ' ';

        for(i=0;i<10;i++){
            buffer[k++] = zf_list[i]+'0';
            buffer[k++] = ' ';
        }
        for(i=0;i<3;i++){
            buffer[k++] = my_time[6-i]/10%10+'0';
            buffer[k++] = my_time[6-i]%10+'0';
            if(i<2)buffer[k++] = '-';
        }
        buffer[k++] = ' ';

        for(i=0;i<3;i++){
            buffer[k++] = my_time[2-i]/10%10+'0';
            buffer[k++] = my_time[2-i]%10+'0';
            if(i<2)buffer[k++] = ':';
        }
        buffer[k++] = ' ';

        buffer[k++] = rssi/100%10+'0';
        buffer[k++] = rssi/10%10+'0';
        buffer[k++] = rssi%10+'0';

        buffer[k++] = 0x0d;
        ax_len  = ax5043_len%AX5043_MAX;
        lseek(fd,ax_len*60,SEEK_SET);
        write(fd, buffer, k);
        close(fd);
    }else {
        rt_kprintf("AX_record.txt error\r\n");
    }
}
uint8_t get_ax_Record(uint16_t ax_addr){
    int fd, size;
    uint8_t i;
    fd = open("/AX_record.txt", O_RDWR | O_CREAT);

    if(fd >= 0){
        lseek(fd,ax_addr*60,SEEK_SET);
        size = read(fd,buffer,60);
        close(fd);
        if(size < 60){
            return 0;
        }else {
//            for(i=0;i<60;i++)rt_kprintf("%c ",buffer[i]);
            return 1;
        }
    }else {
        rt_kprintf("Record_len.txt error\r\n");
    }
    return 0;
}
uint8_t get_ax_data(uint8_t addr){
    return buffer[addr];
}

uint16_t stat_sample(void)
{
    int ret;
     struct stat buf;
     ret = stat("/AX_record.txt", &buf);
     if(ret == 0){
         rt_kprintf("text.txt file size = %d\n", buf.st_size);
         return buf.st_size/60;
     }
     else{
         rt_kprintf("text.txt file not fonud\n");
     }

}
