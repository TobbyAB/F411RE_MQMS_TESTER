/**
  ******************************************************************************
  * @file    usb_defines.h
  * @author  MCD Application Team
  * @version V2.2.0
  * @date    09-November-2015
  * @brief   Header of the Core Layer
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USB_DEF_H__
#define __USB_DEF_H__

/* Includes ------------------------------------------------------------------*/
#include  "usb_conf.h"

/** @addtogroup USB_OTG_DRIVER
  * @{
  */
  
/** @defgroup USB_DEFINES
  * @brief This file is the 
  * @{
  */ 


/** @defgroup USB_DEFINES_Exported_Defines
  * @{
  */ 
/**
  * @}
  */ 


/** @defgroup _CORE_DEFINES_
  * @{
  */

#define USB_OTG_SPEED_PARAM_HIGH 0
#define USB_OTG_SPEED_PARAM_HIGH_IN_FULL 1
#define USB_OTG_SPEED_PARAM_FULL 3

#define USB_OTG_SPEED_HIGH      0U
#define USB_OTG_SPEED_FULL      3U

#define USB_OTG_ULPI_PHY      1U
#define USB_OTG_EMBEDDED_PHY  2U

/**
  * @}
  */


/** @defgroup _GLOBAL_DEFINES_
  * @{
  */
#define GAHBCFG_TXFEMPTYLVL_EMPTY              1
#define GAHBCFG_TXFEMPTYLVL_HALFEMPTY          0
#define GAHBCFG_GLBINT_ENABLE                  1
#define GAHBCFG_INT_DMA_BURST_SINGLE           0
#define GAHBCFG_INT_DMA_BURST_INCR             1
#define GAHBCFG_INT_DMA_BURST_INCR4            3
#define GAHBCFG_INT_DMA_BURST_INCR8            5
#define GAHBCFG_INT_DMA_BURST_INCR16           7
#define GAHBCFG_DMAENABLE                      1
#define GAHBCFG_TXFEMPTYLVL_EMPTY              1
#define GAHBCFG_TXFEMPTYLVL_HALFEMPTY          0
#define GRXSTS_PKTSTS_IN                       2U
#define GRXSTS_PKTSTS_IN_XFER_COMP             3U
#define GRXSTS_PKTSTS_DATA_TOGGLE_ERR          5U
#define GRXSTS_PKTSTS_CH_HALTED                7U
/**
  * @}
  */


/** @defgroup _OnTheGo_DEFINES_
  * @{
  */
#define MODE_HNP_SRP_CAPABLE                   0
#define MODE_SRP_ONLY_CAPABLE                  1
#define MODE_NO_HNP_SRP_CAPABLE                2
#define MODE_SRP_CAPABLE_DEVICE                3
#define MODE_NO_SRP_CAPABLE_DEVICE             4
#define MODE_SRP_CAPABLE_HOST                  5
#define MODE_NO_SRP_CAPABLE_HOST               6
#define A_HOST                                 1
#define A_SUSPEND                              2
#define A_PERIPHERAL                           3
#define B_PERIPHERAL                           4
#define B_HOST                                 5
#define DEVICE_MODE                            0
#define HOST_MODE                              1
#define OTG_MODE                               2
/**
  * @}
  */


/** @defgroup __DEVICE_DEFINES_
  * @{
  */
#define DSTS_ENUMSPD_HS_PHY_30MHZ_OR_60MHZ     (0U << 1)
#define DSTS_ENUMSPD_FS_PHY_30MHZ_OR_60MHZ     (1U << 1)
#define DSTS_ENUMSPD_LS_PHY_6MHZ               (2U << 1)
#define DSTS_ENUMSPD_FS_PHY_48MHZ              (3U << 1)

#define DCFG_FRAME_INTERVAL_80                 0U
#define DCFG_FRAME_INTERVAL_85                 1U
#define DCFG_FRAME_INTERVAL_90                 2U
#define DCFG_FRAME_INTERVAL_95                 3U

#define DEP0CTL_MPS_64                         0U
#define DEP0CTL_MPS_32                         1U
#define DEP0CTL_MPS_16                         2U
#define DEP0CTL_MPS_8                          3U

#define EP_SPEED_LOW                           0U
#define EP_SPEED_FULL                          1U
#define EP_SPEED_HIGH                          2U

#define EP_TYPE_CTRL                           0U
#define EP_TYPE_ISOC                           1U
#define EP_TYPE_BULK                           2U
#define EP_TYPE_INTR                           3U
#define EP_TYPE_MSK                            3U

#define STS_GOUT_NAK                           1U
#define STS_DATA_UPDT                          2U
#define STS_XFER_COMP                          3U
#define STS_SETUP_COMP                         4U
#define STS_SETUP_UPDT                         6U
/**
  * @}
  */


/** @defgroup __HOST_DEFINES_
  * @{
  */
#define HC_PID_DATA0                           0U
#define HC_PID_DATA2                           1U
#define HC_PID_DATA1                           2U
#define HC_PID_SETUP                           3U

#define HPRT0_PRTSPD_HIGH_SPEED                0U
#define HPRT0_PRTSPD_FULL_SPEED                1U
#define HPRT0_PRTSPD_LOW_SPEED                 2U

#define HCFG_30_60_MHZ                         0U
#define HCFG_48_MHZ                            1U
#define HCFG_6_MHZ                             2U

#define HCCHAR_CTRL                            0U
#define HCCHAR_ISOC                            1U
#define HCCHAR_BULK                            2U
#define HCCHAR_INTR                            3U

#define  MIN(a, b)      (((a) < (b)) ? (a) : (b))

/**
  * @}
  */


/** @defgroup USB_DEFINES_Exported_Types
  * @{
  */ 

typedef enum
{
  USB_OTG_HS_CORE_ID = 0,
  USB_OTG_FS_CORE_ID = 1
}USB_OTG_CORE_ID_TypeDef;
/**
  * @}
  */ 


/** @defgroup USB_DEFINES_Exported_Macros
  * @{
  */ 
/**
  * @}
  */ 

/** @defgroup USB_DEFINES_Exported_Variables
  * @{
  */ 
/**
  * @}
  */ 

/** @defgroup USB_DEFINES_Exported_FunctionsPrototype
  * @{
  */ 
/**
  * @}
  */ 


/** @defgroup Internal_Macro's
  * @{
  */
#define USB_OTG_READ_REG32(reg)  (*(__IO uint32_t *)(reg))
#define USB_OTG_WRITE_REG32(reg,value) (*(__IO uint32_t *)(reg) = (value))
#define USB_OTG_MODIFY_REG32(reg,clear_mask,set_mask) \
  USB_OTG_WRITE_REG32((reg), (((USB_OTG_READ_REG32(reg)) & ~(clear_mask)) | (set_mask)) )

/********************************************************************************
                              ENUMERATION TYPE
********************************************************************************/
enum USB_OTG_SPEED {
  USB_SPEED_UNKNOWN = 0,
  USB_SPEED_LOW,
  USB_SPEED_FULL,
  USB_SPEED_HIGH
};

#endif /* __USB_DEFINES__H__ */


/**
  * @}
  */ 

/**
  * @}
  */ 
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

