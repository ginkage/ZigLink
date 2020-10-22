/**************************************************************************************************
  Filename:       zcl_myswitch.c
  Revised:        $Date: 2014-10-24 16:04:46 -0700 (Fri, 24 Oct 2014) $
  Revision:       $Revision: 40796 $


  Description:    Zigbee Cluster Library - sample device application.


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
  PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
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

/*********************************************************************
  This application implements a ZigBee On/Off Switch, based on Z-Stack 3.0.

  This application is based on the common sample-application user interface. Please see the main
  comment in zcl_sampleapp_ui.c. The rest of this comment describes only the content specific for
  this sample applicetion.
  
  Application-specific UI peripherals being used:

  - none (LED1 is currently unused by this application).

  Application-specific menu system:

    <TOGGLE LIGHT> Send an On, Off or Toggle command targeting appropriate devices from the binding table.
      Pressing / releasing [OK] will have the following functionality, depending on the value of the 
      zclMySwitch_OnOffSwitchActions attribute:
      - OnOffSwitchActions == 0: pressing [OK] will send ON command, releasing it will send OFF command;
      - OnOffSwitchActions == 1: pressing [OK] will send OFF command, releasing it will send ON command;
      - OnOffSwitchActions == 2: pressing [OK] will send TOGGLE command, releasing it will not send any command.

*********************************************************************/

#if ! defined ZCL_ON_OFF
#error ZCL_ON_OFF must be defined for this project.
#endif

/*********************************************************************
 * INCLUDES
 */
#include "ZComDef.h"
#include "OSAL.h"
#include "AF.h"
#include "ZDApp.h"
#include "ZDObject.h"
#include "ZDProfile.h"

#include "MT.h"
#include "MT_AF.h"
#include "MT_SYS.h"
#include "MT_UART.h"

#include "nwk_util.h"

#include "zcl.h"
#include "zcl_general.h"
#include "zcl_ha.h"
#include "zcl_diagnostic.h"
#include "zcl_myswitch.h"

#include "bdb.h"
#include "bdb_interface.h"
#include "gp_interface.h"

#include "onboard.h"

/* HAL */
#include "hal_lcd.h"
#include "hal_led.h"
#include "hal_key.h"
#include "hal_uart.h"

#include "bdb.h"
#include "bdb_interface.h"

#include "zcl_sampleapps_ui.h"

/*********************************************************************
 * MACROS
 */
#define UI_STATE_TOGGLE_LIGHT 1 //UI_STATE_BACK_FROM_APP_MENU is item #0, so app item numbers should start from 1

#define APP_TITLE "TI Sample Switch"
  
/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */
byte zclMySwitch_TaskID;

uint8 zclMySwitchSeqNum;

uint8 zclMySwitch_OnOffSwitchType = ON_OFF_SWITCH_TYPE_MOMENTARY;

uint8 zclMySwitch_OnOffSwitchActions;

uint8 znpCfg1;

/*********************************************************************
 * GLOBAL FUNCTIONS
 */
 
/*********************************************************************
 * LOCAL VARIABLES
 */

afAddrType_t zclMySwitch_DstAddr;

// Endpoint to allow SYS_APP_MSGs
static endPointDesc_t mySwitch_TestEp =
{
  MYSWITCH_ENDPOINT,                  // endpoint
  0,
  &zclMySwitch_TaskID,
  (SimpleDescriptionFormat_t *)NULL,  // No Simple description for this test endpoint
  (afNetworkLatencyReq_t)0            // No Network Latency req
};

uint8 gPermitDuration = 0;    // permit joining default to disabled

devStates_t zclMySwitch_NwkState = DEV_INIT;

static osal_msg_q_t npTxQueue;

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static void zclMySwitch_HandleKeys( byte shift, byte keys );
static void zclMySwitch_BasicResetCB( void );
static void zclMySwitch_BindNotification( bdbBindNotificationData_t *data );

static void zclMySwitch_ProcessCommissioningStatus(bdbCommissioningModeMsg_t *bdbCommissioningModeMsg);

// Functions to process ZCL Foundation incoming Command/Response messages
static void zclMySwitch_ProcessIncomingMsg( zclIncomingMsg_t *msg );
#ifdef ZCL_READ
static uint8 zclMySwitch_ProcessInReadRspCmd( zclIncomingMsg_t *pInMsg );
#endif
#ifdef ZCL_WRITE
static uint8 zclMySwitch_ProcessInWriteRspCmd( zclIncomingMsg_t *pInMsg );
#endif
static uint8 zclMySwitch_ProcessInDefaultRspCmd( zclIncomingMsg_t *pInMsg );
#ifdef ZCL_DISCOVER
static uint8 zclMySwitch_ProcessInDiscCmdsRspCmd( zclIncomingMsg_t *pInMsg );
static uint8 zclMySwitch_ProcessInDiscAttrsRspCmd( zclIncomingMsg_t *pInMsg );
static uint8 zclMySwitch_ProcessInDiscAttrsExtRspCmd( zclIncomingMsg_t *pInMsg );
#endif

void zclMySwitch_UiActionToggleLight(uint16 keys);

static void zclSampleApp_BatteryWarningCB( uint8 voltLevel);

static void npUartCback(uint8 port, uint8 event);
static void npUartTxReady(void);

/*********************************************************************
 * STATUS STRINGS
 */

/*********************************************************************
 * CONSTANTS
 */
  const uiState_t zclMySwitch_UiStatesMain[] = 
  {
    /*  UI_STATE_BACK_FROM_APP_MENU  */   {UI_STATE_DEFAULT_MOVE,       UI_STATE_TOGGLE_LIGHT,  UI_KEY_SW_5_PRESSED, &UI_ActionBackFromAppMenu}, //do not change this line, except for the second item, which should point to the last entry in this menu
    /*  UI_STATE_TOGGLE_LIGHT        */   {UI_STATE_BACK_FROM_APP_MENU, UI_STATE_DEFAULT_MOVE,  UI_KEY_SW_5_PRESSED | UI_KEY_SW_5_RELEASED, &zclMySwitch_UiActionToggleLight},
  };

/*********************************************************************
 * REFERENCED EXTERNALS
 */
extern int16 zdpExternalStateTaskID;

/*********************************************************************
 * ZCL General Profile Callback table
 */
static zclGeneral_AppCallbacks_t zclMySwitch_CmdCallbacks =
{
  zclMySwitch_BasicResetCB,             // Basic Cluster Reset command
  NULL,                                   // Identify Trigger Effect command
  NULL,                                   // On/Off cluster commands
  NULL,                                   // On/Off cluster enhanced command Off with Effect
  NULL,                                   // On/Off cluster enhanced command On with Recall Global Scene
  NULL,                                   // On/Off cluster enhanced command On with Timed Off
#ifdef ZCL_LEVEL_CTRL
  NULL,                                   // Level Control Move to Level command
  NULL,                                   // Level Control Move command
  NULL,                                   // Level Control Step command
  NULL,                                   // Level Control Stop command
#endif
#ifdef ZCL_GROUPS
  NULL,                                   // Group Response commands
#endif
#ifdef ZCL_SCENES
  NULL,                                  // Scene Store Request command
  NULL,                                  // Scene Recall Request command
  NULL,                                  // Scene Response command
#endif
#ifdef ZCL_ALARMS
  NULL,                                  // Alarm (Response) commands
#endif
#ifdef SE_UK_EXT
  NULL,                                  // Get Event Log command
  NULL,                                  // Publish Event Log command
#endif
  NULL,                                  // RSSI Location command
  NULL                                   // RSSI Location Response command
};

/*********************************************************************
 * MYSWITCH_TODO: Add other callback structures for any additional application specific 
 *       Clusters being used, see available callback structures below.
 *
 *       bdbTL_AppCallbacks_t 
 *       zclApplianceControl_AppCallbacks_t 
 *       zclApplianceEventsAlerts_AppCallbacks_t 
 *       zclApplianceStatistics_AppCallbacks_t 
 *       zclElectricalMeasurement_AppCallbacks_t 
 *       zclGeneral_AppCallbacks_t 
 *       zclGp_AppCallbacks_t 
 *       zclHVAC_AppCallbacks_t 
 *       zclLighting_AppCallbacks_t 
 *       zclMS_AppCallbacks_t 
 *       zclPollControl_AppCallbacks_t 
 *       zclPowerProfile_AppCallbacks_t 
 *       zclSS_AppCallbacks_t  
 *
 */

void uartInit(uint8 taskId)
{
  znpTaskId = taskId;
  (void)osal_set_event(taskId, MT_SECONDARY_INIT_EVENT);
#if defined MT_ZNP_FUNC
  znpBasicRspRate = ZNP_BASIC_RSP_RATE;
  (void)osal_start_reload_timer(taskId, ZNP_BASIC_RSP_EVENT, ZNP_BASIC_RSP_RATE);
#endif
}

void uartInitSecondary( void )
{
  if (ZNP_CFG1_UART == znpCfg1)
  {
    halUARTCfg_t uartConfig;

    uartConfig.configured           = TRUE;
    uartConfig.baudRate             = HAL_UART_BR_115200;
    uartConfig.flowControl          = TRUE;
    uartConfig.flowControlThreshold = HAL_UART_FLOW_THRESHOLD;
    uartConfig.rx.maxBufSize        = HAL_UART_RX_BUF_SIZE;
    uartConfig.tx.maxBufSize        = HAL_UART_TX_BUF_SIZE;
    uartConfig.idleTimeout          = HAL_UART_IDLE_TIMEOUT;
    uartConfig.intEnable            = TRUE;
    uartConfig.callBackFunc         = npUartCback;
    HalUARTOpen(HAL_UART_PORT, &uartConfig);
    MT_UartRegisterTaskID(znpTaskId);
  }
  else
  {
    /* npSpiInit() is called by hal_spi.c: HalSpiInit().*/
  }
}

/**************************************************************************************************
 * @fn          znpEventLoop
 *
 * @brief       This function processes the OSAL events and messages for the application.
 *
 * input parameters
 *
 * @param taskId - The task ID assigned to this application by OSAL at system initialization.
 * @param events - A bit mask of the pending event(s).
 *
 * output parameters
 *
 * None.
 *
 * @return      The events bit map received via parameter with the bits cleared which correspond to
 *              the event(s) that were processed on this invocation.
 **************************************************************************************************
 */
uint16 znpEventLoop(uint8 taskId, uint16 events)
{
  osal_event_hdr_t *pMsg;

  if (events & SYS_EVENT_MSG)
  {
    while ((pMsg = (osal_event_hdr_t *) osal_msg_receive(znpTaskId)) != NULL)
    {
      switch (pMsg->event)
      {
      /* incoming message from UART transport */
      case CMD_SERIAL_MSG:
        MT_ProcessIncoming(((mtOSALSerialData_t *)pMsg)->msg);
        break;

      case AF_INCOMING_MSG_CMD:
        {
          MT_AfIncomingMsg((afIncomingMSGPacket_t *)pMsg);
        }
        break;

      case AF_DATA_CONFIRM_CMD:
        MT_AfDataConfirm((afDataConfirm_t *)pMsg);
        break;

      default:
        break;
      }

      osal_msg_deallocate((byte *)pMsg);
    }

    events ^= SYS_EVENT_MSG;
  }
  else if (events & ZNP_UART_TX_READY_EVENT)
  {
    npUartTxReady();
    events ^= ZNP_UART_TX_READY_EVENT;
  }
  else if (events & MT_SECONDARY_INIT_EVENT)
  {
    uartInitSecondary();
    events ^= MT_SECONDARY_INIT_EVENT;
  }
  else if (events & MT_AF_EXEC_EVT)
  {
    MT_AfExec();
    events ^= MT_AF_EXEC_EVT;
  }
  else
  {
    events = 0;  /* Discard unknown events. */
  }

  return ( events );
}

/**************************************************************************************************
 * @fn          npUartCback
 *
 * @brief       This function is the UART callback processor.
 *
 * input parameters
 *
 * @param port - The port being used for UART.
 * @param event - The reason for the callback.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
static void npUartCback(uint8 port, uint8 event)
{
  uint8  ch;

  switch (event) {
  case HAL_UART_RX_FULL:
  case HAL_UART_RX_ABOUT_FULL:
  case HAL_UART_RX_TIMEOUT:
    //MT_UartProcessZToolData(port, znpTaskId);
    while (Hal_UART_RxBufLen(port))
    {
      HalUARTRead (port, &ch, 1);
      if (ch == '0') {
        zclGeneral_SendOnOff_CmdOff( MYSWITCH_ENDPOINT, &zclMySwitch_DstAddr, FALSE, bdb_getZCLFrameCounter() );
      } else if (ch == '1') {
        zclGeneral_SendOnOff_CmdOn( MYSWITCH_ENDPOINT, &zclMySwitch_DstAddr, FALSE, bdb_getZCLFrameCounter() );
      }
    }
    break;

  case HAL_UART_TX_EMPTY:
    osal_set_event(znpTaskId, ZNP_UART_TX_READY_EVENT);
    break;

  default:
    break;
  }
}

/**************************************************************************************************
 * @fn          npUartTxReady
 *
 * @brief       This function gets and writes the next chunk of data to the UART.
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
static void npUartTxReady(void)
{
  static uint16 npUartTxCnt = 0;
  static uint8 *npUartTxMsg = NULL;
  static uint8 *pMsg = NULL;

  if (!npUartTxMsg)
  {
    if ((pMsg = npUartTxMsg = osal_msg_dequeue(&npTxQueue)))
    {
      /* | SOP | Data Length | CMD |  DATA   | FSC |
       * |  1  |     1       |  2  | as dLen |  1  |
       */
      npUartTxCnt = pMsg[1] + MT_UART_FRAME_OVHD + MT_RPC_FRAME_HDR_SZ;
    }
  }

  if (npUartTxMsg)
  {
    uint16 len = HalUARTWrite(HAL_UART_PORT, pMsg, npUartTxCnt);
    npUartTxCnt -= len;

    if (npUartTxCnt == 0)
    {
      osal_msg_deallocate(npUartTxMsg);
      npUartTxMsg = NULL;
    }
    else
    {
      pMsg += len;
    }
  }
}

/**************************************************************************************************
 * @fn          npMtUartAlloc
 *
 * @brief       This function allocates a buffer for Txing on UART.
 *
 * input parameters
 *
 * @param cmd0 - The first byte of the MT command id containing the command type and subsystem.
 * @param len - Data length required.
 *
 * output parameters
 *
 * None.
 *
 * @return      Pointer to the buffer obtained; possibly NULL if an allocation failed.
 **************************************************************************************************
 */
uint8* MT_TransportAlloc(uint8 cmd0, uint8 len)
{
  uint8 *p;

  if ((p = osal_msg_allocate(len + MT_RPC_FRAME_HDR_SZ + MT_UART_FRAME_OVHD)) != NULL)
  {
    return p + 1;
  }

  return NULL;
}

/**************************************************************************************************
 * @fn          npMtUartSend
 *
 * @brief       This function transmits or enqueues the buffer for transmitting on UART.
 *
 * input parameters
 *
 * @param pBuf - Pointer to the buffer to transmit on the UART.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
void MT_TransportSend(uint8 *pBuf)
{
  uint8 len = pBuf[0] + MT_RPC_FRAME_HDR_SZ;

  pBuf[len] = MT_UartCalcFCS(pBuf, len);
  pBuf--;
  pBuf[0] = MT_UART_SOF;

  osal_msg_enqueue(&npTxQueue, pBuf);
  osal_set_event(znpTaskId, ZNP_UART_TX_READY_EVENT);
}

/*********************************************************************
 * @fn          zclMySwitch_Init
 *
 * @brief       Initialization function for the zclGeneral layer.
 *
 * @param       none
 *
 * @return      none
 */
void zclMySwitch_Init( byte task_id )
{
  zclMySwitch_TaskID = task_id;

  // Set destination address to indirect
  zclMySwitch_DstAddr.addrMode = (afAddrMode_t)AddrNotPresent;
  zclMySwitch_DstAddr.endPoint = 0;
  zclMySwitch_DstAddr.addr.shortAddr = 0;

  // This app is part of the Home Automation Profile
  bdb_RegisterSimpleDescriptor( &zclMySwitch_SimpleDesc );

  // Register the ZCL General Cluster Library callback functions
  zclGeneral_RegisterCmdCallbacks( MYSWITCH_ENDPOINT, &zclMySwitch_CmdCallbacks );
  
  // MYSWITCH_TODO: Register other cluster command callbacks here
  zclMySwitch_ResetAttributesToDefaultValues();

  // Register the application's attribute list
  zcl_registerAttrList( MYSWITCH_ENDPOINT, zclMySwitch_NumAttributes, zclMySwitch_Attrs );

  // Register the Application to receive the unprocessed Foundation command/response messages
  zcl_registerForMsg( zclMySwitch_TaskID );

  // Register low voltage NV memory protection application callback
  RegisterVoltageWarningCB( zclSampleApp_BatteryWarningCB );

  // Register for all key events - This app will handle all key events
  RegisterForKeys( zclMySwitch_TaskID );

  bdb_RegisterCommissioningStatusCB( zclMySwitch_ProcessCommissioningStatus );
  bdb_RegisterBindNotificationCB( zclMySwitch_BindNotification );

  // Register for a test endpoint
  afRegister( &mySwitch_TestEp );

#ifdef ZCL_DIAGNOSTIC
  // Register the application's callback function to read/write attribute data.
  // This is only required when the attribute data format is unknown to ZCL.
  zcl_registerReadWriteCB( MYSWITCH_ENDPOINT, zclDiagnostic_ReadWriteAttrCB, NULL );

  if ( zclDiagnostic_InitStats() == ZSuccess )
  {
    // Here the user could start the timer to save Diagnostics to NV
  }
#endif

  zdpExternalStateTaskID = zclMySwitch_TaskID;

  bdb_StartCommissioning(BDB_COMMISSIONING_REJOIN_EXISTING_NETWORK_ON_STARTUP | BDB_COMMISSIONING_MODE_NWK_STEERING | BDB_COMMISSIONING_MODE_NWK_FORMATION | BDB_COMMISSIONING_MODE_FINDING_BINDING);
}

/*********************************************************************
 * @fn          zclSample_event_loop
 *
 * @brief       Event Loop Processor for zclGeneral.
 *
 * @param       none
 *
 * @return      none
 */
uint16 zclMySwitch_event_loop( uint8 task_id, uint16 events )
{
  afIncomingMSGPacket_t *MSGpkt;

  (void)task_id;  // Intentionally unreferenced parameter

  if ( events & SYS_EVENT_MSG )
  {
    while ( (MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( zclMySwitch_TaskID )) )
    {
      switch ( MSGpkt->hdr.event )
      {
        case ZCL_INCOMING_MSG:
          // Incoming ZCL Foundation command/response messages
          zclMySwitch_ProcessIncomingMsg( (zclIncomingMsg_t *)MSGpkt );
          break;

        case KEY_CHANGE:
          zclMySwitch_HandleKeys( ((keyChange_t *)MSGpkt)->state, ((keyChange_t *)MSGpkt)->keys );
          break;

        case ZDO_STATE_CHANGE:
          zclMySwitch_NwkState = (devStates_t)(MSGpkt->hdr.status);

          // now on the network
          if (zclMySwitch_NwkState == DEV_END_DEVICE)
          {
            //
          }
          break;

        default:
          break;
      }

      // Release the memory
      osal_msg_deallocate( (uint8 *)MSGpkt );
    }

    // return unprocessed events
    return (events ^ SYS_EVENT_MSG);
  }
  
#if ZG_BUILD_ENDDEVICE_TYPE    
  if ( events & MYSWITCH_END_DEVICE_REJOIN_EVT )
  {
    bdb_ZedAttemptRecoverNwk();
    return ( events ^ MYSWITCH_END_DEVICE_REJOIN_EVT );
  }
#endif

  /* MYSWITCH_TODO: handle app events here */
  
  if ( events & MYSWITCH_EVT_1 )
  {
    // toggle LED 2 state, start another timer for 500ms
    HalLedSet ( HAL_LED_2, HAL_LED_MODE_TOGGLE );
    osal_start_timerEx( zclMySwitch_TaskID, MYSWITCH_EVT_1, 500 );
    
    return ( events ^ MYSWITCH_EVT_1 );
  }

  // Discard unknown events
  return 0;
}


/*********************************************************************
 * @fn      zclMySwitch_HandleKeys
 *
 * @brief   Handles all key events for this device.
 *
 * @param   shift - true if in shift/alt.
 * @param   keys - bit field for key events. Valid entries:
 *                 HAL_KEY_SW_5
 *                 HAL_KEY_SW_4
 *                 HAL_KEY_SW_2
 *                 HAL_KEY_SW_1
 *
 * @return  none
 */
static void zclMySwitch_HandleKeys( byte shift, byte keys )
{
  if ( keys & HAL_KEY_SW_1 )
  {
    zclGeneral_SendOnOff_CmdOn( MYSWITCH_ENDPOINT, &zclMySwitch_DstAddr, TRUE, bdb_getZCLFrameCounter() );
  }

  if ( keys & HAL_KEY_SW_2 )
  {
    zclGeneral_SendOnOff_CmdOff( MYSWITCH_ENDPOINT, &zclMySwitch_DstAddr, TRUE, bdb_getZCLFrameCounter() );
  }

  if ( keys & HAL_KEY_SW_3 )
  {
  }

  if ( keys & HAL_KEY_SW_4 )
  {
  }

  if ( keys & HAL_KEY_SW_5 )
  {
    bdb_resetLocalAction();
  }
}

/*********************************************************************
 * @fn      zclMySwitch_ProcessCommissioningStatus
 *
 * @brief   Callback in which the status of the commissioning process are reported
 *
 * @param   bdbCommissioningModeMsg - Context message of the status of a commissioning process
 *
 * @return  none
 */
static void zclMySwitch_ProcessCommissioningStatus(bdbCommissioningModeMsg_t *bdbCommissioningModeMsg)
{
  switch(bdbCommissioningModeMsg->bdbCommissioningMode)
  {
    case BDB_COMMISSIONING_FORMATION:
      if(bdbCommissioningModeMsg->bdbCommissioningStatus == BDB_COMMISSIONING_SUCCESS)
      {
        //After formation, perform nwk steering again plus the remaining commissioning modes that has not been process yet
        bdb_StartCommissioning(BDB_COMMISSIONING_MODE_NWK_STEERING | bdbCommissioningModeMsg->bdbRemainingCommissioningModes);
      }
      else
      {
        //Want to try other channels?
        //try with bdb_setChannelAttribute
      }
    break;
    case BDB_COMMISSIONING_NWK_STEERING:
      if(bdbCommissioningModeMsg->bdbCommissioningStatus == BDB_COMMISSIONING_SUCCESS)
      {
        //YOUR JOB:
        //We are on the nwk, what now?
      }
      else
      {
        //See the possible errors for nwk steering procedure
        //No suitable networks found
        //Want to try other channels?
        //try with bdb_setChannelAttribute
      }
    break;
    case BDB_COMMISSIONING_FINDING_BINDING:
      if(bdbCommissioningModeMsg->bdbCommissioningStatus == BDB_COMMISSIONING_SUCCESS)
      {
        //YOUR JOB:
      }
      else
      {
        //YOUR JOB:
        //retry?, wait for user interaction?
      }
    break;
    case BDB_COMMISSIONING_INITIALIZATION:
      //Initialization notification can only be successful. Failure on initialization
      //only happens for ZED and is notified as BDB_COMMISSIONING_PARENT_LOST notification

      //YOUR JOB:
      //We are on a network, what now?

    break;
#if ZG_BUILD_ENDDEVICE_TYPE    
    case BDB_COMMISSIONING_PARENT_LOST:
      if(bdbCommissioningModeMsg->bdbCommissioningStatus == BDB_COMMISSIONING_NETWORK_RESTORED)
      {
        //We did recover from losing parent
      }
      else
      {
        //Parent not found, attempt to rejoin again after a fixed delay
        osal_start_timerEx(zclMySwitch_TaskID, MYSWITCH_END_DEVICE_REJOIN_EVT, MYSWITCH_END_DEVICE_REJOIN_DELAY);
      }
    break;
#endif 
  }
}

/*********************************************************************
 * @fn      zclMySwitch_BindNotification
 *
 * @brief   Called when a new bind is added.
 *
 * @param   data - pointer to new bind data
 *
 * @return  none
 */
static void zclMySwitch_BindNotification( bdbBindNotificationData_t *data )
{
  // MYSWITCH_TODO: process the new bind information
  //zclMySwitch_DstAddr.addrMode = (afAddrMode_t)Addr16Bit;
  //zclMySwitch_DstAddr.endPoint = data->ep;
  //zclMySwitch_DstAddr.addr.shortAddr = data->dstAddr.addr.shortAddr;
}

/*********************************************************************
 * @fn      zclMySwitch_BasicResetCB
 *
 * @brief   Callback from the ZCL General Cluster Library
 *          to set all the Basic Cluster attributes to default values.
 *
 * @param   none
 *
 * @return  none
 */
static void zclMySwitch_BasicResetCB( void )
{

  /* MYSWITCH_TODO: remember to update this function with any
     application-specific cluster attribute variables */
  
  zclMySwitch_ResetAttributesToDefaultValues();
  
}
/*********************************************************************
 * @fn      zclSampleApp_BatteryWarningCB
 *
 * @brief   Called to handle battery-low situation.
 *
 * @param   voltLevel - level of severity
 *
 * @return  none
 */
void zclSampleApp_BatteryWarningCB( uint8 voltLevel )
{
  if ( voltLevel == VOLT_LEVEL_CAUTIOUS )
  {
    // Send warning message to the gateway and blink LED
  }
  else if ( voltLevel == VOLT_LEVEL_BAD )
  {
    // Shut down the system
  }
}

/******************************************************************************
 *
 *  Functions for processing ZCL Foundation incoming Command/Response messages
 *
 *****************************************************************************/

/*********************************************************************
 * @fn      zclMySwitch_ProcessIncomingMsg
 *
 * @brief   Process ZCL Foundation incoming message
 *
 * @param   pInMsg - pointer to the received message
 *
 * @return  none
 */
static void zclMySwitch_ProcessIncomingMsg( zclIncomingMsg_t *pInMsg )
{
  switch ( pInMsg->zclHdr.commandID )
  {
#ifdef ZCL_READ
    case ZCL_CMD_READ_RSP:
      zclMySwitch_ProcessInReadRspCmd( pInMsg );
      break;
#endif
#ifdef ZCL_WRITE
    case ZCL_CMD_WRITE_RSP:
      zclMySwitch_ProcessInWriteRspCmd( pInMsg );
      break;
#endif
    case ZCL_CMD_CONFIG_REPORT:
    case ZCL_CMD_CONFIG_REPORT_RSP:
    case ZCL_CMD_READ_REPORT_CFG:
    case ZCL_CMD_READ_REPORT_CFG_RSP:
    case ZCL_CMD_REPORT:
      //bdb_ProcessIncomingReportingMsg( pInMsg );
      break;
      
    case ZCL_CMD_DEFAULT_RSP:
      zclMySwitch_ProcessInDefaultRspCmd( pInMsg );
      break;
#ifdef ZCL_DISCOVER
    case ZCL_CMD_DISCOVER_CMDS_RECEIVED_RSP:
      zclMySwitch_ProcessInDiscCmdsRspCmd( pInMsg );
      break;

    case ZCL_CMD_DISCOVER_CMDS_GEN_RSP:
      zclMySwitch_ProcessInDiscCmdsRspCmd( pInMsg );
      break;

    case ZCL_CMD_DISCOVER_ATTRS_RSP:
      zclMySwitch_ProcessInDiscAttrsRspCmd( pInMsg );
      break;

    case ZCL_CMD_DISCOVER_ATTRS_EXT_RSP:
      zclMySwitch_ProcessInDiscAttrsExtRspCmd( pInMsg );
      break;
#endif
    default:
      break;
  }

  if ( pInMsg->attrCmd )
    osal_mem_free( pInMsg->attrCmd );
}

#ifdef ZCL_READ
/*********************************************************************
 * @fn      zclMySwitch_ProcessInReadRspCmd
 *
 * @brief   Process the "Profile" Read Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8 zclMySwitch_ProcessInReadRspCmd( zclIncomingMsg_t *pInMsg )
{
  zclReadRspCmd_t *readRspCmd;
  uint8 i;

  readRspCmd = (zclReadRspCmd_t *)pInMsg->attrCmd;
  for (i = 0; i < readRspCmd->numAttr; i++)
  {
    // Notify the originator of the results of the original read attributes
    // attempt and, for each successfull request, the value of the requested
    // attribute
  }

  return ( TRUE );
}
#endif // ZCL_READ

#ifdef ZCL_WRITE
/*********************************************************************
 * @fn      zclMySwitch_ProcessInWriteRspCmd
 *
 * @brief   Process the "Profile" Write Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8 zclMySwitch_ProcessInWriteRspCmd( zclIncomingMsg_t *pInMsg )
{
  zclWriteRspCmd_t *writeRspCmd;
  uint8 i;

  writeRspCmd = (zclWriteRspCmd_t *)pInMsg->attrCmd;
  for ( i = 0; i < writeRspCmd->numAttr; i++ )
  {
    // Notify the device of the results of the its original write attributes
    // command.
  }

  return ( TRUE );
}
#endif // ZCL_WRITE

/*********************************************************************
 * @fn      zclMySwitch_ProcessInDefaultRspCmd
 *
 * @brief   Process the "Profile" Default Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8 zclMySwitch_ProcessInDefaultRspCmd( zclIncomingMsg_t *pInMsg )
{
  // zclDefaultRspCmd_t *defaultRspCmd = (zclDefaultRspCmd_t *)pInMsg->attrCmd;

  // Device is notified of the Default Response command.
  (void)pInMsg;

  return ( TRUE );
}

#ifdef ZCL_DISCOVER
/*********************************************************************
 * @fn      zclMySwitch_ProcessInDiscCmdsRspCmd
 *
 * @brief   Process the Discover Commands Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8 zclMySwitch_ProcessInDiscCmdsRspCmd( zclIncomingMsg_t *pInMsg )
{
  zclDiscoverCmdsCmdRsp_t *discoverRspCmd;
  uint8 i;

  discoverRspCmd = (zclDiscoverCmdsCmdRsp_t *)pInMsg->attrCmd;
  for ( i = 0; i < discoverRspCmd->numCmd; i++ )
  {
    // Device is notified of the result of its attribute discovery command.
  }

  return ( TRUE );
}

/*********************************************************************
 * @fn      zclMySwitch_ProcessInDiscAttrsRspCmd
 *
 * @brief   Process the "Profile" Discover Attributes Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8 zclMySwitch_ProcessInDiscAttrsRspCmd( zclIncomingMsg_t *pInMsg )
{
  zclDiscoverAttrsRspCmd_t *discoverRspCmd;
  uint8 i;

  discoverRspCmd = (zclDiscoverAttrsRspCmd_t *)pInMsg->attrCmd;
  for ( i = 0; i < discoverRspCmd->numAttr; i++ )
  {
    // Device is notified of the result of its attribute discovery command.
  }

  return ( TRUE );
}

/*********************************************************************
 * @fn      zclMySwitch_ProcessInDiscAttrsExtRspCmd
 *
 * @brief   Process the "Profile" Discover Attributes Extended Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8 zclMySwitch_ProcessInDiscAttrsExtRspCmd( zclIncomingMsg_t *pInMsg )
{
  zclDiscoverAttrsExtRsp_t *discoverRspCmd;
  uint8 i;

  discoverRspCmd = (zclDiscoverAttrsExtRsp_t *)pInMsg->attrCmd;
  for ( i = 0; i < discoverRspCmd->numAttr; i++ )
  {
    // Device is notified of the result of its attribute discovery command.
  }

  return ( TRUE );
}
#endif // ZCL_DISCOVER

/****************************************************************************
****************************************************************************/

void zclMySwitch_UiActionToggleLight(uint16 keys)
{
  if (zclMySwitch_OnOffSwitchActions == ON_OFF_SWITCH_ACTIONS_TOGGLE)
  {
    if (keys & UI_KEY_SW_5_PRESSED)
    {
      zclGeneral_SendOnOff_CmdToggle( MYSWITCH_ENDPOINT, &zclMySwitch_DstAddr, FALSE, bdb_getZCLFrameCounter() );
    }
  }
  else if (((keys & UI_KEY_SW_5_PRESSED) && (zclMySwitch_OnOffSwitchActions == ON_OFF_SWITCH_ACTIONS_ON))
    || ((keys & UI_KEY_SW_5_RELEASED) && (zclMySwitch_OnOffSwitchActions == ON_OFF_SWITCH_ACTIONS_OFF)))
  {
    zclGeneral_SendOnOff_CmdOn( MYSWITCH_ENDPOINT, &zclMySwitch_DstAddr, FALSE, bdb_getZCLFrameCounter() );
  }
  else
  {
    zclGeneral_SendOnOff_CmdOff( MYSWITCH_ENDPOINT, &zclMySwitch_DstAddr, FALSE, bdb_getZCLFrameCounter() );
  }
}
