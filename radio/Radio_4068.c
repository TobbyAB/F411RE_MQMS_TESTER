/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-12-01     Rick       the first version
 */
#include <rtthread.h>
#include <rtdevice.h>
#include "drv_spi.h"
#include <string.h>
#include <stdio.h>
#include "AX5043.h"
#include "Radio_Common.h"
#include "pin_config.h"
#include "Radio_Decoder.h"
//#include "Radio_Encoder.h"
#include "Radio_Drv.h"
#include "Config_4068.h"

#define DBG_TAG "radio_4068"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

struct ax5043 rf_4068;

rt_sem_t IRQ2_Sem=RT_NULL;
rt_thread_t rf_4068_task=RT_NULL;
rt_timer_t rf_4068_send_timer = RT_NULL;



void IRQ2_ISR(void *parameter)
{
    rt_sem_release(IRQ2_Sem);
}
void IRQ2_Bounding(void)
{
    rt_pin_mode(IRQ2, PIN_MODE_INPUT_PULLUP);
    rt_pin_attach_irq(IRQ2, PIN_IRQ_MODE_RISING, IRQ2_ISR, RT_NULL);
    rt_pin_irq_enable(IRQ2, PIN_IRQ_ENABLE);
}
struct ax5043_config *rf_4068_config_init(void)
{
    static struct ax5043_config *config;
    config = rt_malloc(sizeof(struct ax5043_config));
    config->axradio_phy_vcocalib = 0;
    config->axradio_phy_preamble_len = 16;
    config->axradio_phy_preamble_byte = 0x55;
    config->axradio_phy_preamble_flags = 0x38;
    config->axradio_phy_preamble_appendbits = 0;
    config->axradio_phy_preamble_appendpattern = 0x00;
    config->axradio_phy_chanfreq[0] = 0x01908aa3;
    config->axradio_phy_chanvcoiinit[0] = 0x00;
    config->axradio_phy_chanpllrnginit[0] = 0xFF;
    config->axradio_phy_maxfreqoffset = 157;
//    config->axradio_phy_rssireference = 0x3A;
    config->axradio_phy_rssireference = 0x36;//调整过的RSSI
    config->axradio_phy_rssioffset = 64;
    config->axradio_framing_synclen = 16;
    config->axradio_framing_syncflags = 0x38;
    config->axradio_framing_syncword[0] = 0x55;
    config->axradio_framing_syncword[1] = 0x5a;
    config->axradio_framing_syncword[2] = 0x33;
    config->axradio_framing_syncword[3] = 0x55;
    return config;
}
void rf_4068_sem_init(void)
{
    IRQ2_Sem = rt_sem_create("IRQ2_Sem", 0, RT_IPC_FLAG_FIFO);
}
void rf_4068_Init(void)
{
    rf_4068.config = rf_4068_config_init();
    rf_4068.socket = rf_4068_radio_spi_init();
    strcpy(rf_4068.name,"rf_4068");



    memcpy(rf_4068.RegValue,set_registers_4068,sizeof(set_registers_4068));
    memcpy(rf_4068.TXRegValue,set_registers_tx_4068,sizeof(set_registers_tx_4068));
    memcpy(rf_4068.RXRegValue,set_registers_rx_4068,sizeof(set_registers_rx_4068));
    IRQ2_Bounding();
    rf_startup(&rf_4068);

    /*选频后，赋值给注册数组*/
    LOG_D("VCOI Initial value:  %x\r\n",set_registers_4068[82][1]);
    set_registers_4068[82][1] = simple_autorange_pll(&rf_4068);

    LOG_D("VCOI Correction value: %x\r\n",set_registers_4068[82][1]);
    memcpy(rf_4068.RegValue,set_registers_4068,sizeof(set_registers_4068));
    rf_startup(&rf_4068);


    vcoi_rng_get(&rf_4068);
    Ax5043SetRegisters_RX(&rf_4068);
    Ax5043ReceiverON(&rf_4068);
}
void rf_4068_send_timer_callback(void *parameter)
{
    if(rf_4068.ubRFState != trxstate_rx)
    {
        LOG_W("rf_4068 Send timeout\r\n");
        SpiWriteSingleAddressRegister(&rf_4068,REG_AX5043_RADIOEVENTMASK0, 0x00);
        rf_restart(&rf_4068);
    }
}
void rf_4068_send_timer_start(void)
{
    rt_timer_start(rf_4068_send_timer);
}

void rf_4068_task_callback(void *parameter)
{
    while(1)
    {
        static rt_err_t result;
        result = rt_sem_take(IRQ2_Sem, RT_WAITING_FOREVER);
        if (result == RT_EOK)
        {
            switch (rf_4068.ubRFState)
            {
            case trxstate_rx: //0x01
                ReceiveData(&rf_4068);
                Ax5043SetRegisters_RX(&rf_4068);
                Ax5043Receiver_Continuous(&rf_4068);
                if (rf_4068.RxLen != 0)
                {
                    rf4068_rx_callback(rf_4068.ubRssi,rf_4068.RXBuff,rf_4068.RxLen);
                }
                LOG_D("rf_4068 trxstate_rx");
                break;
            case trxstate_wait_xtal:     //3
                SpiWriteSingleAddressRegister(&rf_4068,REG_AX5043_IRQMASK1, 0x00);//AX5043_IRQMASK1 = 0x00 otherwise crystal ready will fire all over again
                rf_4068.ubRFState = trxstate_xtal_ready;

                LOG_D("rf_4068 trxstate_xtal_ready");
                break;
            case trxstate_pll_ranging:     //5
                SpiWriteSingleAddressRegister(&rf_4068,REG_AX5043_IRQMASK1, 0x00);//AX5043_IRQMASK1 = 0x00 otherwise autoranging done will fire all over again
                rf_4068.ubRFState = trxstate_pll_ranging_done;

                LOG_D("rf_4068 trxstate_pll_ranging_done");
                vcoi_rng_get(&rf_4068);
                break;
            case trxstate_pll_settling:     //7
                SpiWriteSingleAddressRegister(&rf_4068,REG_AX5043_RADIOEVENTMASK0, 0x00);// AX5043_RADIOEVENTMASK0 = 0x00;
                rf_4068.ubRFState = trxstate_pll_settled;
                LOG_D("rf_4068 trxstate_pll_settled");
                break;
            case trxstate_tx_xtalwait:    //9
                SpiReadSingleAddressRegister(&rf_4068,REG_AX5043_RADIOEVENTREQ0); //make sure REVRDONE bit is cleared, so it is a reliable indicator that the packet is out
                SpiWriteSingleAddressRegister(&rf_4068,REG_AX5043_FIFOSTAT, 0x03);
                SpiWriteSingleAddressRegister(&rf_4068,REG_AX5043_IRQMASK1, 0x00);
                SpiWriteSingleAddressRegister(&rf_4068,REG_AX5043_IRQMASK0, 0x08);// enable fifo free threshold
                rf_4068.ubRFState = trxstate_tx_longpreamble;
                if ((SpiReadSingleAddressRegister(&rf_4068,REG_AX5043_MODULATION) & 0x0F) == 9) { // 4-FSK
                    SpiWriteSingleAddressRegister(&rf_4068,REG_AX5043_FIFODATA, (AX5043_FIFOCMD_DATA | (7 << 5)));
                    SpiWriteSingleAddressRegister(&rf_4068,REG_AX5043_FIFODATA, 2);  // length (including flags)
                    SpiWriteSingleAddressRegister(&rf_4068,REG_AX5043_FIFODATA, 0x01);  // flag PKTSTART -> dibit sync
                    SpiWriteSingleAddressRegister(&rf_4068,REG_AX5043_FIFODATA, 0x11); // dummy byte for forcing dibit sync
                }
                TransmitData(&rf_4068);
                LOG_D("rf_4068 trxstate_tx_xtalwait");
                SpiWriteSingleAddressRegister(&rf_4068,REG_AX5043_PWRMODE, AX5043_PWRSTATE_FULL_TX); //AX5043_PWRMODE = AX5043_PWRSTATE_FULL_TX;
                break;
            case trxstate_tx_longpreamble:LOG_D("rf_4068 longpreamble");
            case trxstate_tx_shortpreamble:LOG_D("rf_4068 shortpreamble");
            case trxstate_tx_packet:LOG_D("rf_4068 trxstate_tx_packet");
                TransmitData(&rf_4068);
                break;
            case trxstate_tx_waitdone:                 //D
                LOG_D("rf_4068 trxstate_tx_waitdone");
                vcoi_rng_get(&rf_4068);
                rt_timer_stop(rf_4068_send_timer);
                SpiReadSingleAddressRegister(&rf_4068,REG_AX5043_RADIOEVENTREQ0);        //clear Interrupt flag
                if (SpiReadSingleAddressRegister(&rf_4068,REG_AX5043_RADIOSTATE) != 0)
                {
                    LOG_D("rf_4068 trxstate_tx_waitdone %d",SpiReadSingleAddressRegister(&rf_4068,REG_AX5043_RADIOSTATE));
                    break;
                }
                LOG_D("rf_4068 trxstate_tx_waitdone %d",SpiReadSingleAddressRegister(&rf_4068,REG_AX5043_RADIOSTATE));
                SpiWriteSingleAddressRegister(&rf_4068,REG_AX5043_RADIOEVENTMASK0, 0x00);

                rf_restart(&rf_4068);

                LOG_D("RF4068 Restart");
                break;
            default:
                SpiWriteSingleAddressRegister(&rf_4068,REG_AX5043_IRQMASK1, 0x00);
                SpiWriteSingleAddressRegister(&rf_4068,REG_AX5043_IRQMASK0, 0x00);
                LOG_D("rf_4068 default");
                break;
            }
        }
    }
}
void rf_4068_start(void)
{

    rf_4068_sem_init();
    rf_4068_task = rt_thread_create("rf_4068_task", rf_4068_task_callback, RT_NULL, 2048, 8, 10);
    rt_thread_startup(rf_4068_task);
    rf_4068_send_timer = rt_timer_create("rf_4068_send timeout", rf_4068_send_timer_callback, RT_NULL, 1000, RT_TIMER_FLAG_ONE_SHOT|RT_TIMER_FLAG_SOFT_TIMER);
    rf_4068_Init();

}


uint8_t buf1[]={0x31,0x31,0x32,0x32,0x33,0x33,0x34,0x34,0x35,0x35,0x36,0x36,0x37,0x37};
void sendlow(void)
{
    rf_4068_Init();
    Normal_send(&rf_4068,buf1,14);
    LOG_D("test 40.67MHz once Now\r\n");
}
MSH_CMD_EXPORT(sendlow,sendtest1);
