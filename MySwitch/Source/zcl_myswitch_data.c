/**************************************************************************************************
  Filename:       zcl_myswitch_data.c
  Revised:        $Date: 2014-05-12 13:14:02 -0700 (Mon, 12 May 2014) $
  Revision:       $Revision: 38502 $


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
 * INCLUDES
 */
#include "ZComDef.h"
#include "OSAL.h"
#include "AF.h"
#include "ZDConfig.h"

#include "zcl.h"
#include "zcl_general.h"
#include "zcl_ha.h"

#include "zcl_poll_control.h"
#include "zcl_electrical_measurement.h"
#include "zcl_diagnostic.h"
#include "zcl_meter_identification.h"
#include "zcl_appliance_identification.h"
#include "zcl_appliance_events_alerts.h"
#include "zcl_power_profile.h"
#include "zcl_appliance_control.h"
#include "zcl_appliance_statistics.h"
#include "zcl_hvac.h"

#include "zcl_myswitch.h"

/*********************************************************************
 * CONSTANTS
 */

#define MYSWITCH_DEVICE_VERSION     1
#define MYSWITCH_FLAGS              0

#define MYSWITCH_HWVERSION          0
#define MYSWITCH_ZCLVERSION         0

#define DEFAULT_PHYSICAL_ENVIRONMENT 0
#define DEFAULT_DEVICE_ENABLE_STATE DEVICE_ENABLED
#define DEFAULT_IDENTIFY_TIME 0

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */

// Global attributes
const uint16 zclMySwitch_clusterRevision_all = 0x0001; 

// Basic Cluster
const uint8 zclMySwitch_HWRevision = MYSWITCH_HWVERSION;
const uint8 zclMySwitch_ZCLVersion = MYSWITCH_ZCLVERSION;
const uint8 zclMySwitch_ManufacturerName[] = { 16, 'T','e','x','a','s','I','n','s','t','r','u','m','e','n','t','s' };
const uint8 zclMySwitch_ModelId[] = { 19, 'Z','B','T','-','D','I','M','S','w','i','t','c','h','-','D','0','0','0','1' };
const uint8 zclMySwitch_DateCode[] = { 16, '2','0','0','6','0','8','3','1',' ',' ',' ',' ',' ',' ',' ',' ' };
const uint8 zclMySwitch_PowerSource = POWER_SOURCE_MAINS_1_PHASE;

uint8 zclMySwitch_LocationDescription[17] = { 16, ' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ' };
uint8 zclMySwitch_PhysicalEnvironment = 0;
uint8 zclMySwitch_DeviceEnable = DEVICE_ENABLED;

// Identify Cluster
uint16 zclMySwitch_IdentifyTime;

/*********************************************************************
 * ATTRIBUTE DEFINITIONS - Uses REAL cluster IDs
 */
CONST zclAttrRec_t zclMySwitch_Attrs[] =
{
  // *** General Basic Cluster Attributes ***
  {
    ZCL_CLUSTER_ID_GEN_BASIC,             // Cluster IDs - defined in the foundation (ie. zcl.h)
    {  // Attribute record
      ATTRID_BASIC_HW_VERSION,            // Attribute ID - Found in Cluster Library header (ie. zcl_general.h)
      ZCL_DATATYPE_UINT8,                 // Data Type - found in zcl.h
      ACCESS_CONTROL_READ,                // Variable access control - found in zcl.h
      (void *)&zclMySwitch_HWRevision  // Pointer to attribute variable
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_ZCL_VERSION,
      ZCL_DATATYPE_UINT8,
      ACCESS_CONTROL_READ,
      (void *)&zclMySwitch_ZCLVersion
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_MANUFACTURER_NAME,
      ZCL_DATATYPE_CHAR_STR,
      ACCESS_CONTROL_READ,
      (void *)zclMySwitch_ManufacturerName
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_MODEL_ID,
      ZCL_DATATYPE_CHAR_STR,
      ACCESS_CONTROL_READ,
      (void *)zclMySwitch_ModelId
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_DATE_CODE,
      ZCL_DATATYPE_CHAR_STR,
      ACCESS_CONTROL_READ,
      (void *)zclMySwitch_DateCode
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_POWER_SOURCE,
      ZCL_DATATYPE_ENUM8,
      ACCESS_CONTROL_READ,
      (void *)&zclMySwitch_PowerSource
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_LOCATION_DESC,
      ZCL_DATATYPE_CHAR_STR,
      (ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE),
      (void *)zclMySwitch_LocationDescription
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_PHYSICAL_ENV,
      ZCL_DATATYPE_ENUM8,
      (ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE),
      (void *)&zclMySwitch_PhysicalEnvironment
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_DEVICE_ENABLED,
      ZCL_DATATYPE_BOOLEAN,
      (ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE),
      (void *)&zclMySwitch_DeviceEnable
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    {  // Attribute record
      ATTRID_CLUSTER_REVISION,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      (void *)&zclMySwitch_clusterRevision_all
    }
  },
  // *** Identify Cluster Attribute ***
  {
    ZCL_CLUSTER_ID_GEN_IDENTIFY,
    { // Attribute record
      ATTRID_IDENTIFY_TIME,
      ZCL_DATATYPE_UINT16,
      (ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE),
      (void *)&zclMySwitch_IdentifyTime
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_IDENTIFY,
    {  // Attribute record
      ATTRID_CLUSTER_REVISION,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ | ACCESS_GLOBAL,
      (void *)&zclMySwitch_clusterRevision_all
    }
  },
  // *** On / Off Switch Configuration Cluster *** //
  {
    ZCL_CLUSTER_ID_GEN_ON_OFF_SWITCH_CONFIG,
    { // Attribute record
      ATTRID_ON_OFF_SWITCH_TYPE,
      ZCL_DATATYPE_ENUM8,
      ACCESS_CONTROL_READ,
      (void *)&zclMySwitch_OnOffSwitchType
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_ON_OFF_SWITCH_CONFIG,
    { // Attribute record
      ATTRID_ON_OFF_SWITCH_ACTIONS,
      ZCL_DATATYPE_ENUM8,
      ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE,
      (void *)&zclMySwitch_OnOffSwitchActions
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_ON_OFF_SWITCH_CONFIG,
    {  // Attribute record
      ATTRID_CLUSTER_REVISION,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      (void *)&zclMySwitch_clusterRevision_all
    }
  },
  // *** On / Off Cluster *** //
  {
    ZCL_CLUSTER_ID_GEN_ON_OFF,
    {  // Attribute record
      ATTRID_CLUSTER_REVISION,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ | ACCESS_CLIENT,
      (void *)&zclMySwitch_clusterRevision_all
    }
  },
  // *** Groups Cluster *** //
  {
    ZCL_CLUSTER_ID_GEN_GROUPS,
    {  // Attribute record
      ATTRID_CLUSTER_REVISION,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ | ACCESS_CLIENT,
      (void *)&zclMySwitch_clusterRevision_all
    }
  }
};

uint8 CONST zclMySwitch_NumAttributes = ( sizeof(zclMySwitch_Attrs) / sizeof(zclMySwitch_Attrs[0]) );

/*********************************************************************
 * SIMPLE DESCRIPTOR
 */
// This is the Cluster ID List and should be filled with Application
// specific cluster IDs.
const cId_t zclMySwitch_InClusterList[] =
{
  ZCL_CLUSTER_ID_GEN_BASIC,
  ZCL_CLUSTER_ID_GEN_IDENTIFY,
  ZCL_CLUSTER_ID_GEN_ON_OFF_SWITCH_CONFIG
};
#define ZCLMYSWITCH_MAX_INCLUSTERS   (sizeof(zclMySwitch_InClusterList) / sizeof(zclMySwitch_InClusterList[0]))


const cId_t zclMySwitch_OutClusterList[] =
{
  ZCL_CLUSTER_ID_GEN_IDENTIFY,
  ZCL_CLUSTER_ID_GEN_ON_OFF,
  ZCL_CLUSTER_ID_GEN_GROUPS,
};
#define ZCLMYSWITCH_MAX_OUTCLUSTERS  (sizeof(zclMySwitch_OutClusterList) / sizeof(zclMySwitch_OutClusterList[0]))


SimpleDescriptionFormat_t zclMySwitch_SimpleDesc =
{
  MYSWITCH_ENDPOINT,                  //  int Endpoint;
  ZCL_HA_PROFILE_ID,                  //  uint16 AppProfId[2];
  ZCL_HA_DEVICEID_ON_OFF_LIGHT_SWITCH, //  uint16 AppDeviceId[2]; 
  MYSWITCH_DEVICE_VERSION,            //  int   AppDevVer:4;
  MYSWITCH_FLAGS,                     //  int   AppFlags:4;
  ZCLMYSWITCH_MAX_INCLUSTERS,         //  byte  AppNumInClusters;
  (cId_t *)zclMySwitch_InClusterList, //  byte *pAppInClusterList;
  ZCLMYSWITCH_MAX_OUTCLUSTERS,        //  byte  AppNumInClusters;
  (cId_t *)zclMySwitch_OutClusterList //  byte *pAppInClusterList;
};

/*********************************************************************
 * GLOBAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL FUNCTIONS
 */
  
void zclMySwitch_ResetAttributesToDefaultValues(void)
{
  int i;
  
  zclMySwitch_LocationDescription[0] = 16;
  for (i = 1; i <= 16; i++)
  {
    zclMySwitch_LocationDescription[i] = ' ';
  }
  
  zclMySwitch_PhysicalEnvironment = DEFAULT_PHYSICAL_ENVIRONMENT;
  zclMySwitch_DeviceEnable = DEFAULT_DEVICE_ENABLE_STATE;

  zclMySwitch_IdentifyTime = DEFAULT_IDENTIFY_TIME;

  zclMySwitch_OnOffSwitchActions = ON_OFF_SWITCH_ACTIONS_TOGGLE; //note that the default specified by the zcl spec is ON_OFF_SWITCH_ACTIONS_ON, but for backward compatibility with TI's legacy sample-switch, we use here ON_OFF_SWITCH_ACTIONS_TOGGLE
}

/****************************************************************************
****************************************************************************/


