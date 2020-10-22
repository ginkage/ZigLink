/**************************************************************************************************
  Filename:       zcl_myswitch.h
  Revised:        $Date: 2014-06-19 08:38:22 -0700 (Thu, 19 Jun 2014) $
  Revision:       $Revision: 39101 $

  Description:    This file contains the ZigBee Cluster Library Home
                  Automation Sample Application.


  Copyright 2006-2014 Texas Instruments Incorporated. All rights reserved.

  IMPORTANT: Your use of this Software is limited to those specific rights
  granted under the terms of a software license agreement between the user
  who downloaded the software, his/her employer (which must be your employer)
  and Texas Instruments Incorporated (the "License").  You may not use this
  Software unless you agree to abide by the terms of the License. The License
  limits your use, and you acknowledge, that the Software may not be modified,
  copied or distributed unless embedded on a Texas Instruments microcontroller
  or used solely and exclusively in conjunction with a Texas Instruments radio
  frequency transceiver, which is integrated into your product.  Other than for
  the foregoing purpose, you may not use, reproduce, copy, prepare derivative
  works of, modify, distribute, perform, display or sell this Software and/or
  its documentation for any purpose.

  YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE
  PROVIDED “AS IS” WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
  INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, TITLE,
  NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL
  TEXAS INSTRUMENTS OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER CONTRACT,
  NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER
  LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
  INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE
  OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT
  OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
  (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.

  Should you have any questions regarding your right to use this Software,
  contact Texas Instruments Incorporated at www.TI.com.
**************************************************************************************************/

#ifndef ZCL_MYSWITCH_H
#define ZCL_MYSWITCH_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "zcl.h"


/*********************************************************************
 * CONSTANTS
 */
#define MYSWITCH_ENDPOINT            1

#define LIGHT_OFF                       0x00
#define LIGHT_ON                        0x01

// Application Events
#define MYSWITCH_MAIN_SCREEN_EVT          0x0001
#define MYSWITCH_LEVEL_CTRL_EVT           0x0002
#define MYSWITCH_END_DEVICE_REJOIN_EVT    0x0004  
  
/* MYSWITCH_TODO: define app events here */
  
#define MYSWITCH_EVT_1                    0x0008
/*
#define MYSWITCH_EVT_2                    0x0010
#define MYSWITCH_EVT_3                    0x0020
*/

// Events for the sample app
#define SAMPLEAPP_END_DEVICE_REJOIN_EVT   0x0001

// UI Events
#define MYSWITCH_LCD_AUTO_UPDATE_EVT       0x0010
#define MYSWITCH_KEY_AUTO_REPEAT_EVT       0x0020

#define MYSWITCH_END_DEVICE_REJOIN_DELAY 10000

#define ZNP_UART_TX_READY_EVENT   0x1000

/*********************************************************************
 * MACROS
 */

/* ------------------------------------------------------------------------------------------------
 *                                          Global Variables
 * ------------------------------------------------------------------------------------------------
 */

#define znpTaskId  MT_TaskID
#define znpBasicRspRate  MT_PeriodicMsgRate

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * VARIABLES
 */

// Added to include ZLL Target functionality
// #if defined ( BDB_TL_INITIATOR ) || defined ( BDB_TL_TARGET )
//   extern bdbTLDeviceInfo_t tlMySwitch_DeviceInfo;
// #endif
  
// MYSWITCH_TODO: Declare application specific attributes here
extern SimpleDescriptionFormat_t zclMySwitch_SimpleDesc;
extern SimpleDescriptionFormat_t zclMySwitch9_SimpleDesc;

extern uint8 zclMySwitch_OnOff;
extern uint8 zclMySwitch_OnOffSwitchActions;
extern uint8 zclMySwitch_OnOffSwitchType;

// attribute list
extern CONST zclAttrRec_t zclMySwitch_Attrs[];
extern CONST uint8 zclMySwitch_NumAttributes;

// Identify attributes
extern uint16 zclMySwitch_IdentifyTime;

// MYSWITCH_TODO: Declare application specific attributes here


/*********************************************************************
 * FUNCTIONS
 */

extern void uartInit( byte task_id );
extern void uartInitSecondary( void );

 /*
  * Initialization for the task
  */
extern void zclMySwitch_Init( byte task_id );

/*
 *  Event Process for the task
 */
extern uint16 znpEventLoop(uint8 taskId, uint16 events);
extern UINT16 zclMySwitch_event_loop( byte task_id, UINT16 events );

/*
 *  Reset all writable attributes to their default values.
 */
extern void zclMySwitch_ResetAttributesToDefaultValues(void);


/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* ZCL_MYSWITCH_H */
