/**
  ******************************************************************************
  * @file    usbh_hid_mouse.c 
  * @author  MCD Application Team
  * @version V2.2.0
  * @date    09-November-2015
  * @brief   This file is the application layer for USB Host HID Mouse Handling.                  
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2015 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/
#include "usbh_hid_mouse.h"


/** @addtogroup USBH_LIB
  * @{
  */

/** @addtogroup USBH_CLASS
  * @{
  */

/** @addtogroup USBH_HID_CLASS
  * @{
  */
  
/** @defgroup USBH_HID_MOUSE 
  * @brief    This file includes HID Layer Handlers for USB Host HID class.
  * @{
  */ 

/** @defgroup USBH_HID_MOUSE_Private_TypesDefinitions
  * @{
  */ 
/**
  * @}
  */ 


/** @defgroup USBH_HID_MOUSE_Private_Defines
  * @{
  */ 
/**
  * @}
  */ 


/** @defgroup USBH_HID_MOUSE_Private_Macros
  * @{
  */ 
/**
  * @}
  */ 

/** @defgroup USBH_HID_MOUSE_Private_FunctionPrototypes
  * @{
  */ 
static void  MOUSE_Init (void);
static void  MOUSE_Decode(uint8_t *data);
/**
  * @}
  */ 


/** @defgroup USBH_HID_MOUSE_Private_Variables
  * @{
  */
#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
 #if defined   (__CC_ARM) /*!< ARM Compiler */
  __align(4) 
 #elif defined ( __ICCARM__ ) /*!< IAR Compiler */
  #pragma data_alignment=4
 #elif defined (__GNUC__) /*!< GNU Compiler */
 #pragma pack(4) 
 #elif defined  (__TASKING__) /*!< TASKING Compiler */                           
  __align(4) 
 #endif /* __CC_ARM */
#endif
 
 
HID_MOUSE_Data_TypeDef HID_MOUSE_Data;
HID_cb_TypeDef HID_MOUSE_cb = 
{
  MOUSE_Init,
  MOUSE_Decode,
};
/**
  * @}
  */ 


/** @defgroup USBH_HID_MOUSE_Private_Functions
  * @{
  */ 

/**
* @brief  MOUSE_Init
*         Init Mouse State.
* @param  None
* @retval None
*/
static void  MOUSE_Init ( void)
{
 /* Call User Init*/
 USR_MOUSE_Init();
}
            
/**
* @brief  MOUSE_Decode
*         Decode Mouse data
* @param  data : Pointer to Mouse HID data buffer
* @retval None
*/
extern HID_Machine_TypeDef HID_Machine;	 
static void  MOUSE_Decode(uint8_t *data)
{
	if(HID_Machine.length==5||HID_Machine.length==6||HID_Machine.length==8)//5/6/8字节长度的USB鼠标数据处理
	{
		HID_MOUSE_Data.button = data[0]; 
		HID_MOUSE_Data.x      = data[1];
		HID_MOUSE_Data.y      = data[3]<<4|data[2]>>4;
		HID_MOUSE_Data.z      = data[4]; 
	}else if(HID_Machine.length==4)	//4字节长度的USB 鼠标数据处理
	{ 
		HID_MOUSE_Data.button = data[0]; 
		HID_MOUSE_Data.x      = data[1];
		HID_MOUSE_Data.y      = data[2];
		HID_MOUSE_Data.z      = data[3];
	} 
	USR_MOUSE_ProcessData(&HID_MOUSE_Data);
}
/**
  * @}
  */ 

/**
  * @}
  */ 

/**
  * @}
  */

/**
  * @}
  */


/**
  * @}
  */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
