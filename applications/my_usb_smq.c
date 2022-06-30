/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-05-20     Gwvt       the first version
 */

#include "my_usb_smq.h"

#define USB_THREAD_SIZE   8192
#define USB_THREAD_PRIORITY     5
#define USB_THREAD_TIMESLICE    20

USBH_HOST  USB_Host;
USB_OTG_CORE_HANDLE  USB_OTG_Core_dev;
extern HID_Machine_TypeDef HID_Machine;


static void usb_thread(void* parameter)
{
    USBH_Init(&USB_OTG_Core_dev,USB_OTG_FS_CORE_ID,&USB_Host,&HID_cb,&USR_Callbacks);

    while (1)
    {
        USBH_Process(&USB_OTG_Core_dev, &USB_Host);
//        rt_thread_mdelay(1000);
    }
}

void my_usb_init(){
    rt_thread_t tid = RT_NULL;



    tid = rt_thread_create("usb_thread",
            usb_thread, RT_NULL,
            USB_THREAD_SIZE,
            USB_THREAD_PRIORITY, USB_THREAD_TIMESLICE);
    if (tid != RT_NULL)
        rt_thread_startup(tid);
}
