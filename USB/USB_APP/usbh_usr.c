#include "usbh_usr.h"
#include "usb_hcd_int.h"
#include "usbh_hid_mouse.h"
#include "usbh_hid_keybd.h"  
#include "string.h"  
#include <rtthread.h>

//表示USB连接状态
//0,没有连接;
//1,已经连接;
vu8 bDeviceState=0;     //默认没有连接


extern USB_OTG_CORE_HANDLE USB_OTG_Core_dev;
extern void USBH_Msg_Show(u8 msgx);
u8 USB_FIRST_PLUGIN_FLAG=0; //USB第一次插入标志,如果为1,说明是第一次插入

//USB OTG 中断服务函数
//处理所有USB中断
void OTG_FS_IRQHandler(void)
{ 
    USBH_OTG_ISR_Handler(&USB_OTG_Core_dev);
}  

//USB HOST 用户回调函数.
USBH_Usr_cb_TypeDef USR_Callbacks =
{
  USBH_USR_Init,
  USBH_USR_DeInit,
  USBH_USR_DeviceAttached,
  USBH_USR_ResetDevice,
  USBH_USR_DeviceDisconnected,
  USBH_USR_OverCurrentDetected,
  USBH_USR_DeviceSpeedDetected,
  USBH_USR_Device_DescAvailable,
  USBH_USR_DeviceAddressAssigned,
  USBH_USR_Configuration_DescAvailable,
  USBH_USR_Manufacturer_String,
  USBH_USR_Product_String,
  USBH_USR_SerialNum_String,
  USBH_USR_EnumerationDone,
  USBH_USR_UserInput,
  NULL,
  USBH_USR_DeviceNotSupported,
  USBH_USR_UnrecoveredError
};
 
//USB HOST 初始化
void USBH_USR_Init(void)
{
    rt_kprintf("USB OTG FS MSC Host\r\n");
    rt_kprintf("> USB Host library started.\r\n");
    rt_kprintf("  USB Host Library v2.1.0\r\n\r\n");

}
//检测到U盘插入
void USBH_USR_DeviceAttached(void)//U盘插入
{ 
    rt_kprintf("USB device insertion detected!\r\n");
}
//检测到U盘拔出
void USBH_USR_DeviceDisconnected (void)//U盘移除
{ 
    rt_kprintf("USB device unplugged!\r\n");
    bDeviceState=0; //USB设备拔出了
}  
//复位从机
void USBH_USR_ResetDevice(void)
{
    rt_kprintf("reset device...\r\n");
}
//检测到从机速度
//DeviceSpeed:从机速度(0,1,2 / 其他)
void USBH_USR_DeviceSpeedDetected(uint8_t DeviceSpeed)
{
    if(DeviceSpeed==HPRT0_PRTSPD_HIGH_SPEED)
    {
        rt_kprintf("High Speed (HS) USB Devices!\r\n");
    }
    else if(DeviceSpeed==HPRT0_PRTSPD_FULL_SPEED)
    {
        rt_kprintf("Full Speed (FS) USB device!\r\n");
    }
    else if(DeviceSpeed==HPRT0_PRTSPD_LOW_SPEED)
    {
        rt_kprintf("Low Speed (LS) USB device!\r\n");
    }
    else
    {
        rt_kprintf("device error!\r\n");
    }
}
//检测到从机的描述符
//DeviceDesc:设备描述符指针
void USBH_USR_Device_DescAvailable(void *DeviceDesc)
{ 
    USBH_DevDesc_TypeDef *hs;
    hs=DeviceDesc;
    rt_kprintf("VID: %04Xh\r\n" , (uint32_t)(*hs).idVendor);
    rt_kprintf("PID: %04Xh\r\n" , (uint32_t)(*hs).idProduct);
}
//从机地址分配成功
void USBH_USR_DeviceAddressAssigned(void)
{
    rt_kprintf("Slave address assignment is successful!\r\n");
}
//配置描述符获有效
void USBH_USR_Configuration_DescAvailable(USBH_CfgDesc_TypeDef * cfgDesc,
                                          USBH_InterfaceDesc_TypeDef *itfDesc,
                                          USBH_EpDesc_TypeDef *epDesc)
{
    USBH_InterfaceDesc_TypeDef *id;
    id = itfDesc;
    if((*id).bInterfaceClass==0x08)
    {
        rt_kprintf("removable storage device!\r\n");
    }else if((*id).bInterfaceClass==0x03)
    {
        rt_kprintf("HID device!\r\n");
    }
}
//获取到设备Manufacturer String
void USBH_USR_Manufacturer_String(void *ManufacturerString)
{
    rt_kprintf("Manufacturer: %s\r\n",(char *)ManufacturerString);
}
//获取到设备Product String
void USBH_USR_Product_String(void *ProductString)
{
    rt_kprintf("Product: %s\r\n",(char *)ProductString);
}
//获取到设备SerialNum String
void USBH_USR_SerialNum_String(void *SerialNumString)
{
    rt_kprintf("Serial Number: %s\r\n",(char *)SerialNumString);
} 
//设备USB枚举完成
void USBH_USR_EnumerationDone(void)
{ 
    rt_kprintf("Device enumeration complete!\r\n\r\n");
} 
//无法识别的USB设备
void USBH_USR_DeviceNotSupported(void)
{ 
    //rt_kprintf("无法识别的USB设备!\r\n\r\n");
}  
//等待用户输入按键,执行下一步操作
USBH_USR_Status USBH_USR_UserInput(void)
{ 
    rt_kprintf("Skip user confirmation step!\r\n");
    bDeviceState=1;//USB设备已经连接成功
    return USBH_USR_RESP_OK;
} 
//USB接口电流过载
void USBH_USR_OverCurrentDetected (void)
{
    rt_kprintf("Port current is too high!!!\r\n");
}  
//重新初始化
void USBH_USR_DeInit(void)
{
    rt_kprintf("usb host reboot!!!\r\n");
}
//无法恢复的错误!!
void USBH_USR_UnrecoveredError (void)
{
    rt_kprintf("unrecoverable error!!!\r\n\r\n");
}
//////////////////////////////////////////////////////////////////////////////////////////
//下面两个函数,为ALIENTEK添加,以防止USB死机

//USB枚举状态死机检测,防止USB枚举失败导致的死机
//phost:USB_HOST结构体指针
//返回值:0,没有死机
//       1,死机了,外部必须重新启动USB连接.
u8 USBH_Check_EnumeDead(USBH_HOST *phost)
{
    static u16 errcnt=0;
    //这个状态,如果持续存在,则说明USB死机了.
    if(phost->gState==HOST_CTRL_XFER&&(phost->EnumState==ENUM_IDLE||phost->EnumState==ENUM_GET_FULL_DEV_DESC))
    {
        errcnt++;
        if(errcnt>2000)//死机了
        {
            errcnt=0;
            RCC->AHB2RSTR|=1<<7;    //USB OTG FS 复位
            rt_thread_mdelay(5);
            RCC->AHB2RSTR&=~(1<<7); //复位结束
            return 1;
        }
    }else errcnt=0;
    return 0;
} 
//USB HID通信死机检测,防止USB通信死机(暂时仅针对:DTERR,即Data toggle error)
//pcore:USB_OTG_Core_dev_HANDLE结构体指针
//phidm:HID_Machine_TypeDef结构体指针
//返回值:0,没有死机
//       1,死机了,外部必须重新启动USB连接.
u8 USBH_Check_HIDCommDead(USB_OTG_CORE_HANDLE *pcore,HID_Machine_TypeDef *phidm)
{
    if(pcore->host.HC_Status[phidm->hc_num_in]==HC_DATATGLERR)//检测到DTERR错误,直接重启USB.
    {
        return 1;
    }
    return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////
//USB键盘鼠标数据处理

//键盘初始化
void  USR_KEYBRD_Init(void)
{ 
    USB_FIRST_PLUGIN_FLAG=1;//标记第一次插入
}



//USB键盘数据处理
void  USR_KEYBRD_ProcessData (uint8_t data)
{ 
    rt_kprintf("%c",data);
}













