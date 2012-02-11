/****************************************************************************/
/*                                                                          */
/*  Module:         LegTrnaslationFuncs.c                                   */
/*                                                                          */
/*  Description:    This module provides compatibility support to allow     */
/*                  QST 2.x applications run on QST 1.x firmware.           */
/*                                                                          */
/****************************************************************************/

/****************************************************************************/
/*                                                                          */
/*     Copyright (c) 2005-2009, Intel Corporation. All Rights Reserved.     */
/*                                                                          */
/*  Redistribution and use in source and binary  forms,  with  or  without  */
/*  modification, are permitted provided that the following conditions are  */
/*  met:                                                                    */
/*                                                                          */
/*    - Redistributions of source code must  retain  the  above  copyright  */
/*      notice, this list of conditions and the following disclaimer.       */
/*                                                                          */
/*    - Redistributions  in binary form must reproduce the above copyright  */
/*      notice, this list of conditions and the  following  disclaimer  in  */
/*      the   documentation  and/or  other  materials  provided  with  the  */
/*      distribution.                                                       */
/*                                                                          */
/*    - Neither the name  of  Intel  Corporation  nor  the  names  of  its  */
/*      contributors  may  be  used to endorse or promote products derived  */
/*      from this software without specific prior written permission.       */
/*                                                                          */
/*  DISCLAIMER: THIS SOFTWARE IS PROVIDED BY  THE  COPYRIGHT  HOLDERS  AND  */
/*  CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,  */
/*  BUT  NOT  LIMITED  TO,  THE  IMPLIED WARRANTIES OF MERCHANTABILITY AND  */
/*  FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN  NO  EVENT  SHALL  */
/*  INTEL  CORPORATION  OR  THE  CONTRIBUTORS  BE  LIABLE  FOR ANY DIRECT,  */
/*  INDIRECT, INCIDENTAL, SPECIAL,  EXEMPLARY,  OR  CONSEQUENTIAL  DAMAGES  */
/*  (INCLUDING,  BUT  NOT  LIMITED  TO, PROCUREMENT OF SUBSTITUTE GOODS OR  */
/*  SERVICES; LOSS OF USE, DATA, OR  PROFITS;  OR  BUSINESS  INTERRUPTION)  */
/*  HOWEVER  CAUSED  AND  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,  */
/*  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING  */
/*  IN  ANY  WAY  OUT  OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE  */
/*  POSSIBILITY OF SUCH DAMAGE.                                             */
/*                                                                          */
/****************************************************************************/

#pragma warning(disable: 4201 4701)

#include <stdlib.h>
#include <malloc.h>
#include <memory.h>

#include "QstCmd.h"
#include "QstCmdLeg.h"
#include "LegTranslationFuncs.h"

/****************************************************************************/
/* QST Subsystem Information structure defaults to QST 1.0 values           */
/****************************************************************************/
BOOL  QstSubsystemInfoFound = FALSE;

#define QST1_MAJOR_VER_NUM_MAX      5

QST_GET_SUBSYSTEM_INFO_RSP   QstSubsystemInfo = {
   QST_CMD_REJECTED_UNSUPPORTED,
   QST1_MAJOR_VER_NUM_MAX,
   0,
   0,
   0,
   QST_LEG_MAX_TEMP_MONITORS,
   QST_LEG_MAX_FAN_MONITORS,
   QST_LEG_MAX_VOLT_MONITORS,
   0,
   QST_LEG_MAX_TEMP_RESPONSES,
   QST_LEG_MAX_FAN_CONTROLLERS,
   sizeof(QST_LEG_SET_SUBSYSTEM_CONFIG_CMD)
};


/****************************************************************************/
/* Internal functions                                                       */
/****************************************************************************/

/****************************************************************************/
/* ConvertToLegQstCommand () - Converts the QST 2.0 command and entity data */
/* into a legacy command.  Also returns the maximum required buffer sizes.  */
/****************************************************************************/

static BOOL ConvertToLegQstCommand(

   IN    QST_CMD_HEADER *QstCmd,             // 2.x command format
   OUT   UINT8          *LegCommand,
   OUT   size_t         *LegCmdSize,
   OUT   size_t         *LegRspSize
){
   //
   // Assume that the command translation failed by assigning a known bad
   // legacy values.
   //
   *LegCommand = QST_LEG_LAST_CMD_CODE + 1;
   *LegCmdSize = 0;
   *LegRspSize = 0;

   //
   // Begin to decode commands
   //
   switch (QstCmd->byCommand)
   {
   case QST_GET_SUBSYSTEM_INFO:
   case QST_GET_SUBSYSTEM_CONFIG:
   case QST_SET_SUBSYSTEM_CONFIG:
   case QST_UPDATE_CPU_CONFIG:
   case QST_GET_CPU_CONFIG_UPDATE:
   case QST_UPDATE_FAN_CONFIG:
   case QST_GET_FAN_CONFIG_UPDATE:
   case QST_UPDATE_CPU_DTS_CONFIG:
   case QST_GET_CPU_DTS_CONFIG_UPDATE:
   case QST_GET_CURR_MON_UPDATE:
   case QST_GET_CURR_MON_CONFIG:
   case QST_SET_CURR_MON_THRESHOLDS:
      //
      // Handle commands that are not supported here.  This is the only place
      // that unsupported commands will be checked!!!!
      //
      break;

   case QST_GET_SUBSYSTEM_STATUS:
      *LegCommand = QST_LEG_GET_SUBSYSTEM_STATUS;
      *LegCmdSize = sizeof(QST_LEG_GENERIC_CMD);
      *LegRspSize = sizeof(QST_LEG_GET_SUBSYSTEM_STATUS_RSP);
      break;

   case QST_GET_SUBSYSTEM_CONFIG_PROFILE:
      *LegCommand = QST_LEG_GET_SUBSYSTEM_CONFIG_PROFILE;
      *LegCmdSize = sizeof(QST_LEG_GENERIC_CMD);
      *LegRspSize = sizeof(QST_LEG_GET_SUBSYSTEM_CONFIG_PROFILE_RSP);
      break;

   case QST_LOCK_SUBSYSTEM:
      *LegCommand = QST_LEG_LOCK_SUBSYSTEM;
      *LegCmdSize = sizeof(QST_LEG_LOCK_SUBSYSTEM_CMD);
      *LegRspSize = sizeof(QST_LEG_GENERIC_RSP);
      break;

   case QST_SST_PASS_THROUGH:
      //
      // For SST PT buffer sizes will be handled in other places
      //
      *LegCommand = QST_LEG_SST_PASS_THROUGH;
      break;

   case QST_GET_TEMP_MON_UPDATE:
      *LegCommand = QST_LEG_GET_TEMP_MON_UPDATE;
      *LegCmdSize = sizeof(QST_LEG_GENERIC_CMD);
      *LegRspSize = sizeof(QST_LEG_GET_TEMP_MON_UPDATE_RSP);
      break;

   case QST_GET_TEMP_MON_CONFIG:
      if (QstCmd->byEntity < QST_LEG_MAX_TEMP_MONITORS)
      {
         *LegCommand = (UINT8) (QST_LEG_GET_TEMP_MON_1_CONFIG + QstCmd->byEntity);
         *LegCmdSize = sizeof(QST_LEG_GENERIC_CMD);
         *LegRspSize = sizeof(QST_LEG_GET_TEMP_MON_CONFIG_RSP);
      }
      break;

   case QST_SET_TEMP_MON_THRESHOLDS:
      if (QstCmd->byEntity < QST_LEG_MAX_TEMP_MONITORS)
      {
         *LegCommand = (UINT8) (QST_LEG_SET_TEMP_MON_1_THRESHOLDS + QstCmd->byEntity);
         *LegCmdSize = sizeof(QST_LEG_SET_TEMP_MON_THRESHOLDS_CMD);
         *LegRspSize = sizeof(QST_LEG_GENERIC_RSP);
      }
      break;

   case QST_SET_TEMP_MON_READING:
      if (QstCmd->byEntity < QST_LEG_MAX_TEMP_MONITORS)
      {
         *LegCommand = (UINT8) (QST_LEG_SET_TEMP_MON_1_READING + QstCmd->byEntity);
         *LegCmdSize = sizeof(QST_LEG_SET_TEMP_MON_READING_CMD);
         *LegRspSize = sizeof(QST_LEG_GENERIC_RSP);
      }
      break;

   case QST_NO_TEMP_MON_READINGS:
      if (QstCmd->byEntity < QST_LEG_MAX_TEMP_MONITORS)
      {
         *LegCommand = (UINT8) (QST_LEG_NO_TEMP_MON_1_READINGS + QstCmd->byEntity);
         *LegCmdSize = sizeof(QST_LEG_GENERIC_CMD);
         *LegRspSize = sizeof(QST_LEG_GENERIC_RSP);
      }
      break;

   case QST_GET_FAN_MON_UPDATE:
      *LegCommand = QST_LEG_GET_FAN_MON_UPDATE;
      *LegCmdSize = sizeof(QST_LEG_GENERIC_CMD);
      *LegRspSize = sizeof(QST_LEG_GET_FAN_MON_UPDATE_RSP);
      break;

   case QST_GET_FAN_MON_CONFIG:
      if (QstCmd->byEntity < QST_LEG_MAX_FAN_MONITORS)
      {
         *LegCommand = (UINT8) (QST_LEG_GET_FAN_MON_1_CONFIG + QstCmd->byEntity);
         *LegCmdSize = sizeof(QST_LEG_GENERIC_CMD);
         *LegRspSize = sizeof(QST_LEG_GET_FAN_MON_CONFIG_RSP);
      }
      break;

   case QST_SET_FAN_MON_THRESHOLDS:
      if (QstCmd->byEntity < QST_LEG_MAX_FAN_MONITORS)
      {
         *LegCommand = (UINT8) (QST_LEG_SET_FAN_MON_1_THRESHOLDS + QstCmd->byEntity);
         *LegCmdSize = sizeof(QST_LEG_SET_FAN_MON_THRESHOLDS_CMD);
         *LegRspSize = sizeof(QST_LEG_GENERIC_RSP);
      }
      break;

   case QST_ENABLE_FAN_MON:
      if (QstCmd->byEntity < QST_LEG_MAX_FAN_MONITORS)
      {
         *LegCommand = (UINT8) (QST_LEG_ENABLE_FAN_MON_1 + QstCmd->byEntity);
         *LegCmdSize = sizeof(QST_LEG_GENERIC_CMD);
         *LegRspSize = sizeof(QST_LEG_GENERIC_RSP);
      }
      break;

   case QST_DISABLE_FAN_MON:
      if (QstCmd->byEntity < QST_LEG_MAX_FAN_MONITORS)
      {
         *LegCommand = (UINT8) (QST_LEG_DISABLE_FAN_MON_1 + QstCmd->byEntity);
         *LegCmdSize = sizeof(QST_LEG_GENERIC_CMD);
         *LegRspSize = sizeof(QST_LEG_GENERIC_RSP);
      }
      break;

   case QST_REDETECT_FAN_PRESENCE:
      *LegCommand = QST_LEG_REDETECT_FAN_PRESENCE;
      *LegCmdSize = sizeof(QST_LEG_GENERIC_CMD);
      *LegRspSize = sizeof(QST_LEG_GENERIC_RSP);
      break;

   case QST_GET_VOLT_MON_UPDATE:
      *LegCommand = QST_LEG_GET_VOLT_MON_UPDATE;
      *LegCmdSize = sizeof(QST_LEG_GENERIC_CMD);
      *LegRspSize = sizeof(QST_LEG_GET_VOLT_MON_UPDATE_RSP);
      break;

   case QST_GET_VOLT_MON_CONFIG:
      if (QstCmd->byEntity < QST_LEG_MAX_VOLT_MONITORS)
      {
         *LegCommand = (UINT8) (QST_LEG_GET_VOLT_MON_1_CONFIG + QstCmd->byEntity);
         *LegCmdSize = sizeof(QST_LEG_GENERIC_CMD);
         *LegRspSize = sizeof(QST_LEG_GET_VOLT_MON_CONFIG_RSP);
      }
      break;

   case QST_SET_VOLT_MON_THRESHOLDS:
      if (QstCmd->byEntity < QST_LEG_MAX_VOLT_MONITORS)
      {
         *LegCommand = (UINT8) (QST_LEG_SET_VOLT_MON_1_THRESHOLDS + QstCmd->byEntity);
         *LegCmdSize = sizeof(QST_LEG_SET_VOLT_MON_THRESHOLDS_CMD);
         *LegRspSize = sizeof(QST_LEG_GENERIC_RSP);
      }
      break;

   case QST_GET_FAN_CTRL_UPDATE:
      *LegCommand = QST_LEG_GET_FAN_CTRL_UPDATE;
      *LegCmdSize = sizeof(QST_LEG_GENERIC_CMD);
      *LegRspSize = sizeof(QST_LEG_GET_FAN_CTRL_UPDATE_RSP);
      break;

   case QST_GET_FAN_CTRL_CONFIG:
      if (QstCmd->byEntity < QST_LEG_MAX_FAN_CONTROLLERS)
      {
         *LegCommand = (UINT8) (QST_LEG_GET_FAN_CTRL_1_CONFIG + QstCmd->byEntity);
         *LegCmdSize = sizeof(QST_LEG_GENERIC_CMD);
         *LegRspSize = sizeof(QST_LEG_GET_FAN_CTRL_CONFIG_RSP);
      }
      break;

   case QST_SET_FAN_CTRL_DUTY:
      if (QstCmd->byEntity < QST_LEG_MAX_FAN_CONTROLLERS)
      {
         *LegCommand = (UINT8) (QST_LEG_SET_FAN_CTRL_1_DUTY + QstCmd->byEntity);
         *LegCmdSize = sizeof(QST_LEG_SET_FAN_CTRL_DUTY_CMD);
         *LegRspSize = sizeof(QST_LEG_GENERIC_RSP);
      }
      break;

   case QST_SET_FAN_CTRL_AUTO:
      if (QstCmd->byEntity < QST_LEG_MAX_FAN_CONTROLLERS)
      {
         *LegCommand = (UINT8) (QST_LEG_SET_FAN_CTRL_1_AUTO + QstCmd->byEntity);
         *LegCmdSize = sizeof(QST_LEG_GENERIC_CMD);
         *LegRspSize = sizeof(QST_LEG_GENERIC_RSP);
      }
      break;

   case QST_RESET_FAN_CTRL_MIN_DUTY:
      if (QstCmd->byEntity < QST_LEG_MAX_FAN_CONTROLLERS)
      {
         *LegCommand = (UINT8) (QST_LEG_RESET_FAN_CTRL_1_MIN_DUTY + QstCmd->byEntity);
         *LegCmdSize = sizeof(QST_LEG_GENERIC_CMD);
         *LegRspSize = sizeof(QST_LEG_GENERIC_RSP);
      }
      break;

   default:
      break;
   }

   //
   // Check to see if the command was not decoded
   //
   if (*LegCommand == (QST_LEG_LAST_CMD_CODE + 1))
   {
      return FALSE;
   }

   //
   // Command decoded
   //
   return TRUE;
}

/****************************************************************************/
/* ConvertToNewQstCommand() - Converts the QST 1.x command into a 2.x       */
/* command and also returns the required buffer sizes for the new command.  */
/****************************************************************************/

static BOOL ConvertToNewQstCommand(

   IN    QST_LEG_CMD_HEADER   *QstCmd,       // 1.x command format
   OUT   UINT8                *NewCommand,
   OUT   UINT8                *NewEntity,
   OUT   size_t               *NewCmdSize,
   OUT   size_t               *NewRspSize
){
   //
   // Assume that the command translation failed by assigning known bad values.
   //
   *NewCommand = QST_LAST_CMD_CODE + 1;
   *NewEntity  = 0;
   *NewCmdSize = 0;
   *NewRspSize = 0;

   //
   // Convert the command to the 2.x command set and update buffer sizes
   //
   switch (QstCmd->byCommand)
   {
   case QST_LEG_GET_SUBSYSTEM_CONFIG:
   case QST_LEG_SET_SUBSYSTEM_CONFIG:
   case QST_LEG_UPDATE_CPU_1_CONFIG:
   case QST_LEG_UPDATE_CPU_2_CONFIG:
   case QST_LEG_UPDATE_CPU_3_CONFIG:
   case QST_LEG_UPDATE_CPU_4_CONFIG:
   case QST_LEG_GET_CPU_1_CONFIG_UPDATE:
   case QST_LEG_GET_CPU_2_CONFIG_UPDATE:
   case QST_LEG_GET_CPU_3_CONFIG_UPDATE:
   case QST_LEG_GET_CPU_4_CONFIG_UPDATE:
   case QST_LEG_UPDATE_FAN_1_CONFIG:
   case QST_LEG_UPDATE_FAN_2_CONFIG:
   case QST_LEG_UPDATE_FAN_3_CONFIG:
   case QST_LEG_UPDATE_FAN_4_CONFIG:
   case QST_LEG_UPDATE_FAN_5_CONFIG:
   case QST_LEG_UPDATE_FAN_6_CONFIG:
   case QST_LEG_UPDATE_FAN_7_CONFIG:
   case QST_LEG_UPDATE_FAN_8_CONFIG:
   case QST_LEG_GET_FAN_1_CONFIG_UPDATE:
   case QST_LEG_GET_FAN_2_CONFIG_UPDATE:
   case QST_LEG_GET_FAN_3_CONFIG_UPDATE:
   case QST_LEG_GET_FAN_4_CONFIG_UPDATE:
   case QST_LEG_GET_FAN_5_CONFIG_UPDATE:
   case QST_LEG_GET_FAN_6_CONFIG_UPDATE:
   case QST_LEG_GET_FAN_7_CONFIG_UPDATE:
   case QST_LEG_GET_FAN_8_CONFIG_UPDATE:
      //
      // Handle unsupported commands here...  This will be the only location
      // that these commands will be checked!!!
      //
      break;

   case QST_LEG_GET_SUBSYSTEM_STATUS:
      *NewCommand = QST_GET_SUBSYSTEM_STATUS;
      *NewEntity  = 0;
      *NewCmdSize = sizeof(QST_GENERIC_CMD);
      *NewRspSize = sizeof(QST_GET_SUBSYSTEM_STATUS_RSP);
      break;

   case QST_LEG_GET_SUBSYSTEM_CONFIG_PROFILE:
      *NewCommand = QST_GET_SUBSYSTEM_CONFIG_PROFILE;
      *NewEntity  = 0;
      *NewCmdSize = sizeof(QST_GENERIC_CMD);
      *NewRspSize = sizeof(QST_GET_SUBSYSTEM_CONFIG_PROFILE_RSP);
      break;

   case QST_LEG_LOCK_SUBSYSTEM:
      *NewCommand = QST_LOCK_SUBSYSTEM;
      *NewEntity  = 0;
      *NewCmdSize = sizeof(QST_LOCK_SUBSYSTEM_CMD);
      *NewRspSize = sizeof(QST_GENERIC_RSP);
      break;

   case QST_LEG_SST_PASS_THROUGH:
      //
      // Assign command code but buffer sizes need to be handles outside this
      // function.
      //
      *NewCommand = QST_SST_PASS_THROUGH;
      break;

   case QST_LEG_GET_TEMP_MON_UPDATE:
      *NewCommand = QST_GET_TEMP_MON_UPDATE;
      *NewEntity  = 0;
      *NewCmdSize = sizeof(QST_GENERIC_CMD);
      *NewRspSize = sizeof(QST_GET_TEMP_MON_UPDATE_RSP);
      break;

   case QST_LEG_GET_TEMP_MON_1_CONFIG:
   case QST_LEG_GET_TEMP_MON_2_CONFIG:
   case QST_LEG_GET_TEMP_MON_3_CONFIG:
   case QST_LEG_GET_TEMP_MON_4_CONFIG:
   case QST_LEG_GET_TEMP_MON_5_CONFIG:
   case QST_LEG_GET_TEMP_MON_6_CONFIG:
   case QST_LEG_GET_TEMP_MON_7_CONFIG:
   case QST_LEG_GET_TEMP_MON_8_CONFIG:
   case QST_LEG_GET_TEMP_MON_9_CONFIG:
   case QST_LEG_GET_TEMP_MON_10_CONFIG:
   case QST_LEG_GET_TEMP_MON_11_CONFIG:
   case QST_LEG_GET_TEMP_MON_12_CONFIG:
      *NewCommand = QST_GET_TEMP_MON_CONFIG;
      *NewEntity  = (UINT8) (QstCmd->byCommand - QST_LEG_GET_TEMP_MON_1_CONFIG);
      *NewCmdSize = sizeof(QST_GENERIC_CMD);
      *NewRspSize = sizeof(QST_GET_TEMP_MON_CONFIG_RSP);
      break;

   case QST_LEG_SET_TEMP_MON_1_THRESHOLDS:
   case QST_LEG_SET_TEMP_MON_2_THRESHOLDS:
   case QST_LEG_SET_TEMP_MON_3_THRESHOLDS:
   case QST_LEG_SET_TEMP_MON_4_THRESHOLDS:
   case QST_LEG_SET_TEMP_MON_5_THRESHOLDS:
   case QST_LEG_SET_TEMP_MON_6_THRESHOLDS:
   case QST_LEG_SET_TEMP_MON_7_THRESHOLDS:
   case QST_LEG_SET_TEMP_MON_8_THRESHOLDS:
   case QST_LEG_SET_TEMP_MON_9_THRESHOLDS:
   case QST_LEG_SET_TEMP_MON_10_THRESHOLDS:
   case QST_LEG_SET_TEMP_MON_11_THRESHOLDS:
   case QST_LEG_SET_TEMP_MON_12_THRESHOLDS:
      *NewCommand = QST_SET_TEMP_MON_THRESHOLDS;
      *NewEntity  = (UINT8) (QstCmd->byCommand - QST_LEG_SET_TEMP_MON_1_THRESHOLDS);
      *NewCmdSize = sizeof(QST_SET_TEMP_MON_THRESHOLDS_CMD);
      *NewRspSize = sizeof(QST_GENERIC_RSP);
      break;

   case QST_LEG_SET_TEMP_MON_1_READING:
   case QST_LEG_SET_TEMP_MON_2_READING:
   case QST_LEG_SET_TEMP_MON_3_READING:
   case QST_LEG_SET_TEMP_MON_4_READING:
   case QST_LEG_SET_TEMP_MON_5_READING:
   case QST_LEG_SET_TEMP_MON_6_READING:
   case QST_LEG_SET_TEMP_MON_7_READING:
   case QST_LEG_SET_TEMP_MON_8_READING:
   case QST_LEG_SET_TEMP_MON_9_READING:
   case QST_LEG_SET_TEMP_MON_10_READING:
   case QST_LEG_SET_TEMP_MON_11_READING:
   case QST_LEG_SET_TEMP_MON_12_READING:
      *NewCommand = QST_SET_TEMP_MON_READING;
      *NewEntity  = (UINT8) (QstCmd->byCommand - QST_LEG_SET_TEMP_MON_1_READING);
      *NewCmdSize = sizeof(QST_SET_TEMP_MON_READING_CMD);
      *NewRspSize = sizeof(QST_GENERIC_RSP);
      break;

   case QST_LEG_NO_TEMP_MON_1_READINGS:
   case QST_LEG_NO_TEMP_MON_2_READINGS:
   case QST_LEG_NO_TEMP_MON_3_READINGS:
   case QST_LEG_NO_TEMP_MON_4_READINGS:
   case QST_LEG_NO_TEMP_MON_5_READINGS:
   case QST_LEG_NO_TEMP_MON_6_READINGS:
   case QST_LEG_NO_TEMP_MON_7_READINGS:
   case QST_LEG_NO_TEMP_MON_8_READINGS:
   case QST_LEG_NO_TEMP_MON_9_READINGS:
   case QST_LEG_NO_TEMP_MON_10_READINGS:
   case QST_LEG_NO_TEMP_MON_11_READINGS:
   case QST_LEG_NO_TEMP_MON_12_READINGS:
      *NewCommand = QST_NO_TEMP_MON_READINGS;
      *NewEntity  = (UINT8) (QstCmd->byCommand - QST_LEG_NO_TEMP_MON_1_READINGS);
      *NewCmdSize = sizeof(QST_GENERIC_CMD);
      *NewRspSize = sizeof(QST_GENERIC_RSP);
      break;

   case QST_LEG_GET_FAN_MON_UPDATE:
      *NewCommand = QST_GET_FAN_MON_UPDATE;
      *NewEntity  = 0;
      *NewCmdSize = sizeof(QST_GENERIC_CMD);
      *NewRspSize = sizeof(QST_GET_FAN_MON_UPDATE_RSP);
      break;

   case QST_LEG_GET_FAN_MON_1_CONFIG:
   case QST_LEG_GET_FAN_MON_2_CONFIG:
   case QST_LEG_GET_FAN_MON_3_CONFIG:
   case QST_LEG_GET_FAN_MON_4_CONFIG:
   case QST_LEG_GET_FAN_MON_5_CONFIG:
   case QST_LEG_GET_FAN_MON_6_CONFIG:
   case QST_LEG_GET_FAN_MON_7_CONFIG:
   case QST_LEG_GET_FAN_MON_8_CONFIG:
      *NewCommand = QST_GET_FAN_MON_CONFIG;
      *NewEntity  = (UINT8) (QstCmd->byCommand - QST_LEG_GET_FAN_MON_1_CONFIG);
      *NewCmdSize = sizeof(QST_GENERIC_CMD);
      *NewRspSize = sizeof(QST_GET_FAN_MON_CONFIG_RSP);
      break;

   case QST_LEG_SET_FAN_MON_1_THRESHOLDS:
   case QST_LEG_SET_FAN_MON_2_THRESHOLDS:
   case QST_LEG_SET_FAN_MON_3_THRESHOLDS:
   case QST_LEG_SET_FAN_MON_4_THRESHOLDS:
   case QST_LEG_SET_FAN_MON_5_THRESHOLDS:
   case QST_LEG_SET_FAN_MON_6_THRESHOLDS:
   case QST_LEG_SET_FAN_MON_7_THRESHOLDS:
   case QST_LEG_SET_FAN_MON_8_THRESHOLDS:
      *NewCommand = QST_SET_FAN_MON_THRESHOLDS;
      *NewEntity  = (UINT8) (QstCmd->byCommand - QST_LEG_SET_FAN_MON_1_THRESHOLDS);
      *NewCmdSize = sizeof(QST_SET_FAN_MON_THRESHOLDS_CMD);
      *NewRspSize = sizeof(QST_GENERIC_RSP);
      break;

   case QST_LEG_ENABLE_FAN_MON_1:
   case QST_LEG_ENABLE_FAN_MON_2:
   case QST_LEG_ENABLE_FAN_MON_3:
   case QST_LEG_ENABLE_FAN_MON_4:
   case QST_LEG_ENABLE_FAN_MON_5:
   case QST_LEG_ENABLE_FAN_MON_6:
   case QST_LEG_ENABLE_FAN_MON_7:
   case QST_LEG_ENABLE_FAN_MON_8:
      *NewCommand = QST_ENABLE_FAN_MON;
      *NewEntity  = (UINT8) (QstCmd->byCommand - QST_LEG_ENABLE_FAN_MON_1);
      *NewCmdSize = sizeof(QST_GENERIC_CMD);
      *NewRspSize = sizeof(QST_GENERIC_RSP);
      break;

   case QST_LEG_DISABLE_FAN_MON_1:
   case QST_LEG_DISABLE_FAN_MON_2:
   case QST_LEG_DISABLE_FAN_MON_3:
   case QST_LEG_DISABLE_FAN_MON_4:
   case QST_LEG_DISABLE_FAN_MON_5:
   case QST_LEG_DISABLE_FAN_MON_6:
   case QST_LEG_DISABLE_FAN_MON_7:
   case QST_LEG_DISABLE_FAN_MON_8:
      *NewCommand = QST_DISABLE_FAN_MON;
      *NewEntity  = (UINT8) (QstCmd->byCommand - QST_LEG_DISABLE_FAN_MON_1);
      *NewCmdSize = sizeof(QST_GENERIC_CMD);
      *NewRspSize = sizeof(QST_GENERIC_RSP);
      break;

   case QST_LEG_REDETECT_FAN_PRESENCE:
      *NewCommand = QST_REDETECT_FAN_PRESENCE;
      *NewEntity  = 0;
      *NewCmdSize = sizeof(QST_GENERIC_CMD);
      *NewRspSize = sizeof(QST_GENERIC_RSP);
      break;

   case QST_LEG_GET_VOLT_MON_UPDATE:
      *NewCommand = QST_GET_VOLT_MON_UPDATE;
      *NewEntity  = 0;
      *NewCmdSize = sizeof(QST_GENERIC_CMD);
      *NewRspSize = sizeof(QST_GET_VOLT_MON_UPDATE_RSP);
      break;

   case QST_LEG_GET_VOLT_MON_1_CONFIG:
   case QST_LEG_GET_VOLT_MON_2_CONFIG:
   case QST_LEG_GET_VOLT_MON_3_CONFIG:
   case QST_LEG_GET_VOLT_MON_4_CONFIG:
   case QST_LEG_GET_VOLT_MON_5_CONFIG:
   case QST_LEG_GET_VOLT_MON_6_CONFIG:
   case QST_LEG_GET_VOLT_MON_7_CONFIG:
   case QST_LEG_GET_VOLT_MON_8_CONFIG:
      *NewCommand = QST_GET_VOLT_MON_CONFIG;
      *NewEntity  = (UINT8) (QstCmd->byCommand - QST_LEG_GET_VOLT_MON_1_CONFIG);
      *NewCmdSize = sizeof(QST_GENERIC_CMD);
      *NewRspSize = sizeof(QST_GET_VOLT_MON_CONFIG_RSP);
      break;

   case QST_LEG_SET_VOLT_MON_1_THRESHOLDS:
   case QST_LEG_SET_VOLT_MON_2_THRESHOLDS:
   case QST_LEG_SET_VOLT_MON_3_THRESHOLDS:
   case QST_LEG_SET_VOLT_MON_4_THRESHOLDS:
   case QST_LEG_SET_VOLT_MON_5_THRESHOLDS:
   case QST_LEG_SET_VOLT_MON_6_THRESHOLDS:
   case QST_LEG_SET_VOLT_MON_7_THRESHOLDS:
   case QST_LEG_SET_VOLT_MON_8_THRESHOLDS:
      *NewCommand = QST_SET_VOLT_MON_THRESHOLDS;
      *NewEntity  = (UINT8) (QstCmd->byCommand - QST_LEG_SET_VOLT_MON_1_THRESHOLDS);
      *NewCmdSize = sizeof(QST_SET_VOLT_MON_THRESHOLDS_CMD);
      *NewRspSize = sizeof(QST_GENERIC_RSP);
      break;

   case QST_LEG_GET_FAN_CTRL_UPDATE:
      *NewCommand = QST_GET_FAN_CTRL_UPDATE;
      *NewEntity  = 0;
      *NewCmdSize = sizeof(QST_GENERIC_CMD);
      *NewRspSize = sizeof(QST_GET_FAN_CTRL_UPDATE_RSP);
      break;

   case QST_LEG_GET_FAN_CTRL_1_CONFIG:
   case QST_LEG_GET_FAN_CTRL_2_CONFIG:
   case QST_LEG_GET_FAN_CTRL_3_CONFIG:
   case QST_LEG_GET_FAN_CTRL_4_CONFIG:
   case QST_LEG_GET_FAN_CTRL_5_CONFIG:
   case QST_LEG_GET_FAN_CTRL_6_CONFIG:
   case QST_LEG_GET_FAN_CTRL_7_CONFIG:
   case QST_LEG_GET_FAN_CTRL_8_CONFIG:
      *NewCommand = QST_GET_FAN_CTRL_CONFIG;
      *NewEntity  = (UINT8) (QstCmd->byCommand - QST_LEG_GET_FAN_CTRL_1_CONFIG);
      *NewCmdSize = sizeof(QST_GENERIC_CMD);
      *NewRspSize = sizeof(QST_GET_FAN_CTRL_CONFIG_RSP);
      break;

   case QST_LEG_SET_FAN_CTRL_1_DUTY:
   case QST_LEG_SET_FAN_CTRL_2_DUTY:
   case QST_LEG_SET_FAN_CTRL_3_DUTY:
   case QST_LEG_SET_FAN_CTRL_4_DUTY:
   case QST_LEG_SET_FAN_CTRL_5_DUTY:
   case QST_LEG_SET_FAN_CTRL_6_DUTY:
   case QST_LEG_SET_FAN_CTRL_7_DUTY:
   case QST_LEG_SET_FAN_CTRL_8_DUTY:
      *NewCommand = QST_SET_FAN_CTRL_DUTY;
      *NewEntity  = (UINT8) (QstCmd->byCommand - QST_LEG_SET_FAN_CTRL_1_DUTY);
      *NewCmdSize = sizeof(QST_SET_FAN_CTRL_DUTY_CMD);
      *NewRspSize = sizeof(QST_GENERIC_RSP);
      break;

   case QST_LEG_SET_FAN_CTRL_1_AUTO:
   case QST_LEG_SET_FAN_CTRL_2_AUTO:
   case QST_LEG_SET_FAN_CTRL_3_AUTO:
   case QST_LEG_SET_FAN_CTRL_4_AUTO:
   case QST_LEG_SET_FAN_CTRL_5_AUTO:
   case QST_LEG_SET_FAN_CTRL_6_AUTO:
   case QST_LEG_SET_FAN_CTRL_7_AUTO:
   case QST_LEG_SET_FAN_CTRL_8_AUTO:
      *NewCommand = QST_SET_FAN_CTRL_AUTO;
      *NewEntity  = (UINT8) (QstCmd->byCommand - QST_LEG_SET_FAN_CTRL_1_AUTO);
      *NewCmdSize = sizeof(QST_GENERIC_CMD);
      *NewRspSize = sizeof(QST_GENERIC_RSP);
      break;

   case QST_LEG_RESET_FAN_CTRL_1_MIN_DUTY:
   case QST_LEG_RESET_FAN_CTRL_2_MIN_DUTY:
   case QST_LEG_RESET_FAN_CTRL_3_MIN_DUTY:
   case QST_LEG_RESET_FAN_CTRL_4_MIN_DUTY:
   case QST_LEG_RESET_FAN_CTRL_5_MIN_DUTY:
   case QST_LEG_RESET_FAN_CTRL_6_MIN_DUTY:
   case QST_LEG_RESET_FAN_CTRL_7_MIN_DUTY:
   case QST_LEG_RESET_FAN_CTRL_8_MIN_DUTY:
      *NewCommand = QST_RESET_FAN_CTRL_MIN_DUTY;
      *NewEntity  = (UINT8) (QstCmd->byCommand - QST_LEG_RESET_FAN_CTRL_1_MIN_DUTY);
      *NewCmdSize = sizeof(QST_GENERIC_CMD);
      *NewRspSize = sizeof(QST_GENERIC_RSP);
      break;

   default:
      //
      // Unknown command passed in so error out
      //
      return FALSE;
   }

   //
   // Check to see if the command was not decoded
   //
   if (*NewCommand == (QST_LAST_CMD_CODE + 1))
   {
      return FALSE;
   }


   return TRUE;
}

/****************************************************************************/
/* ConvertToLegCommandData() - Translates the data from the QST 2.x command */
/* buffer to a legacy QST 1.x command buffer.                               */
/*                                                                          */
/* NOTE: ConvertToLegQstCommand must be called first to validate command.   */
/****************************************************************************/

static BOOL ConvertToLegCommandData(

   IN    void     *CmdBuf,                   // 2.x command format
   IN    size_t   CmdSize,
   IN    UINT8    LegCmdCode,
   OUT   void     *LegCmdBuf,
   IN    size_t   LegCmdSize,
   IN    size_t   LegRspSize
){
   QST_LEG_CMD_HEADER   *LegQstCmd = LegCmdBuf;
   UINT8                *CmdData = NULL;
   UINT8                *LegCmdData = NULL;

   //
   // Determine pointer to data area of commands.  Pointer is NULL if no data in
   // command to worry about.
   //
   if (LegCmdSize > sizeof(QST_LEG_CMD_HEADER) && CmdSize > sizeof(QST_CMD_HEADER))
   {
      CmdData = CmdBuf;
      CmdData += sizeof(QST_CMD_HEADER);
      LegCmdData = LegCmdBuf;
      LegCmdData += sizeof(QST_LEG_CMD_HEADER);
   }

   //
   // Assign command data to header
   //
   LegQstCmd->byCommand = LegCmdCode;
   LegQstCmd->wCommandLength = (UINT8) (LegCmdSize - sizeof(QST_LEG_CMD_HEADER));
   LegQstCmd->wResponseLength = (UINT8) LegRspSize;

   //
   // Copy data to the legacy buffer if required
   //
   if (LegCmdData != NULL && CmdData != NULL)
   {
      memcpy (LegCmdData, CmdData, LegQstCmd->wCommandLength);
   }

   return TRUE;
}

/****************************************************************************/
/* ConvertToNewCommandData() - Translates the data from the QST 1.x command */
/* buffer to a new QST 2.x command buffer.                                  */
/****************************************************************************/

static BOOL ConvertToNewCommandData(

   IN    void     *CmdBuf,                   // 1.x command format
   IN    size_t   CmdSize,
   IN    UINT8    NewCmdCode,
   IN    UINT8    NewCmdEntity,
   OUT   void     *NewCmdBuf,
   IN    size_t   NewCmdSize,
   IN    size_t   NewRspSize
){
   QST_CMD_HEADER       *NewQstCmd = NewCmdBuf;
   UINT8                *CmdData = NULL;
   UINT8                *NewCmdData = NULL;

   //
   // Determine pointer to data area of commands.  Pointer is NULL if no data in
   // command to worry about.
   //
   if (NewCmdSize > sizeof(QST_CMD_HEADER) && CmdSize > sizeof(QST_LEG_CMD_HEADER))
   {
      CmdData = CmdBuf;
      CmdData += sizeof(QST_LEG_CMD_HEADER);
      NewCmdData = NewCmdBuf;
      NewCmdData += sizeof(QST_CMD_HEADER);
   }

   //
   // Assign command data to header
   //
   NewQstCmd->byCommand = NewCmdCode;
   NewQstCmd->byEntity = NewCmdEntity;
   NewQstCmd->wCommandLength = (UINT8) (NewCmdSize - sizeof(QST_CMD_HEADER));
   NewQstCmd->wResponseLength = (UINT8) NewRspSize;

   //
   // Copy data to the legacy buffer if required
   //
   if (NewCmdData != NULL && CmdData != NULL)
   {
      memcpy (NewCmdData, CmdData, NewQstCmd->wCommandLength);
   }

   return TRUE;
}

/****************************************************************************/
/* ConvertToNewResponseData() - Converts the 1.x response to the new 2.0    */
/* command response format.                                                 */
/****************************************************************************/

static BOOL ConvertToNewResponseData(

   IN    void           *CmdBuf,             // 2.x command format
   IN    void           *LegRspBuf,
   IN    size_t         LegRspSize,
   OUT   void           *RspBuf,
   IN    size_t         RspSize
){
   QST_GET_SUBSYSTEM_CONFIG_PROFILE_RSP      *ConfigProfileRsp = RspBuf;
   QST_LEG_GET_SUBSYSTEM_CONFIG_PROFILE_RSP  *LegConfigProfileRsp = LegRspBuf;
   size_t                                    MaxCopySize;
   size_t                                    CopySize;
   UINT8                                     MaxEntityCount = 0;
   UINT8                                     CopyCount = 0;
   QST_CMD_HEADER                            QstCmd = {0};
   void                                      *CopyStartAddr = NULL;

   //
   // Check to see if we have any work to do.
   //
   if (RspSize == 0)
   {
      return TRUE;
   }

   //
   // Create a local copy of the command buffer just in case the command and
   // response buffers are shared.
   //
   memcpy (&QstCmd, CmdBuf, sizeof(QST_CMD_HEADER));

   //
   // Determine the maximum data that can be copied between the two response
   // buffers.
   //
   MaxCopySize = LegRspSize;
   if (LegRspSize > RspSize)
   {
      MaxCopySize = RspSize;
   }

   //
   // Zero fill the buffer as many commands need this to be the case.
   //
   // NOTE: Clearing the response buffer may cause the command buffer to be
   //       cleared if the buffers are shared.  Use the cached copy to be safe.
   //
   memset (RspBuf, 0, RspSize);

   //
   // Copy data based on command
   //
   if (QstCmd.byCommand == QST_GET_SUBSYSTEM_CONFIG_PROFILE)
   {
      //
      // Profile data structure changed so copy individual settings across
      //
      ConfigProfileRsp->byStatus = LegConfigProfileRsp->byStatus;
      ConfigProfileRsp->dwTempMonsConfigured = LegConfigProfileRsp->wTempMonsConfigured;
      ConfigProfileRsp->dwFanMonsConfigured = LegConfigProfileRsp->byFanMonsConfigured;
      ConfigProfileRsp->dwVoltMonsConfigured = LegConfigProfileRsp->byVoltMonsConfigured;
      ConfigProfileRsp->dwCurrMonsConfigured = 0;
      ConfigProfileRsp->dwTempRespConfigured = LegConfigProfileRsp->byTempRespConfigured;
      ConfigProfileRsp->dwFanCtrlsConfigured = LegConfigProfileRsp->byFanCtrlConfigured;
   }
   else if (QstCmd.byCommand == QST_GET_TEMP_MON_UPDATE ||
            QstCmd.byCommand == QST_GET_FAN_MON_UPDATE ||
            QstCmd.byCommand == QST_GET_VOLT_MON_UPDATE ||
            QstCmd.byCommand == QST_GET_FAN_CTRL_UPDATE)
   {
      //
      // Determine bounds for legacy entities
      //
      switch (QstCmd.byCommand)
      {
      case QST_GET_TEMP_MON_UPDATE:
         MaxEntityCount = QST_LEG_MAX_TEMP_MONITORS;
         CopyCount = (UINT8) QST_TEMP_MON_UPDATE_SENSOR_COUNT(QstCmd.wResponseLength);
         break;

      case QST_GET_FAN_MON_UPDATE:
         MaxEntityCount = QST_LEG_MAX_FAN_MONITORS;
         CopyCount = (UINT8) QST_FAN_MON_UPDATE_SENSOR_COUNT(QstCmd.wResponseLength);
         break;

      case QST_GET_VOLT_MON_UPDATE:
         MaxEntityCount = QST_LEG_MAX_VOLT_MONITORS;
         CopyCount = (UINT8) QST_VOLT_MON_UPDATE_SENSOR_COUNT(QstCmd.wResponseLength);
         break;

      case QST_GET_FAN_CTRL_UPDATE:
         MaxEntityCount = QST_LEG_MAX_FAN_CONTROLLERS;
         CopyCount = (UINT8) QST_FAN_CTRL_UPDATE_SENSOR_COUNT(QstCmd.wResponseLength);
         break;
      }

      //
      // Check to see if any data needs to be copied
      //
      if (QstCmd.byEntity >= MaxEntityCount)
      {
         return TRUE;
      }

      //
      // Determine the number of entries to copy
      //
      if ((QstCmd.byEntity + CopyCount) >= MaxEntityCount)
      {
         CopyCount = (UINT8) (MaxEntityCount - QstCmd.byEntity);
      }

      //
      // Get start address for copy
      //
      switch (QstCmd.byCommand)
      {
      case QST_GET_TEMP_MON_UPDATE:
         CopyStartAddr = &((QST_LEG_GET_TEMP_MON_UPDATE_RSP*) LegRspBuf)->stMonitorUpdate[QstCmd.byEntity];
         CopySize = sizeof(QST_LEG_TEMP_MON_UPDATE) * CopyCount;
         break;

      case QST_GET_FAN_MON_UPDATE:
         CopyStartAddr = &((QST_LEG_GET_FAN_MON_UPDATE_RSP*) LegRspBuf)->stMonitorUpdate[QstCmd.byEntity];
         CopySize = sizeof(QST_LEG_FAN_MON_UPDATE) * CopyCount;
         break;

      case QST_GET_VOLT_MON_UPDATE:
         CopyStartAddr = &((QST_LEG_GET_VOLT_MON_UPDATE_RSP*) LegRspBuf)->stMonitorUpdate[QstCmd.byEntity];
         CopySize = sizeof(QST_LEG_VOLT_MON_UPDATE) * CopyCount;
         break;

      case QST_GET_FAN_CTRL_UPDATE:
         CopyStartAddr = &((QST_LEG_GET_TEMP_MON_UPDATE_RSP*) LegRspBuf)->stMonitorUpdate[QstCmd.byEntity];
         CopySize = sizeof(QST_LEG_TEMP_MON_UPDATE) * CopyCount;
         break;
      }

      //
      // Copy status and data
      //
      ((QST_GENERIC_RSP*) RspBuf)->byStatus = ((QST_LEG_GENERIC_RSP*) LegRspBuf)->byStatus;
      memcpy ((((UINT8*) RspBuf) + sizeof(QST_GENERIC_RSP)), CopyStartAddr, CopySize);
   }
   else
   {
      //
      // Only need to copy data from one response buffer to the next.  Size
      // parameters must be validated prior to getting to this point.
      //
      if (MaxCopySize > 0)
      {
         memcpy (RspBuf, LegRspBuf, MaxCopySize);
      }
   }

   return TRUE;
}


/****************************************************************************/
/* ConvertToLegResponseData() - Converts the 2.0 response to the old 1.x    */
/* command response format.                                                 */
/****************************************************************************/

static BOOL ConvertToLegResponseData(

   IN    void                 *CmdBuf,       // 1.x command format
   IN    void                 *NewRspBuf,
   IN    size_t               NewRspSize,
   OUT   void                 *RspBuf,
   IN    size_t               RspSize
){
   QST_GET_SUBSYSTEM_CONFIG_PROFILE_RSP      *ConfigProfileRsp = NewRspBuf;
   QST_LEG_GET_SUBSYSTEM_CONFIG_PROFILE_RSP  *LegConfigProfileRsp = RspBuf;
   size_t                                    MaxCopySize;
   QST_LEG_CMD_HEADER                        QstCmd = {0};

   //
   // Check to see if we have any work to do.
   //
   if (RspSize == 0)
   {
      return TRUE;
   }

   //
   // Create a local copy of the command buffer just in case the command and
   // response buffers are shared.
   //
   memcpy (&QstCmd, CmdBuf, sizeof(QST_LEG_CMD_HEADER));

   //
   // Determine the maximum data that can be copied between the two response
   // buffers.
   //
   MaxCopySize = NewRspSize;
   if (NewRspSize > RspSize)
   {
      MaxCopySize = RspSize;
   }

   //
   // Zero fill the buffer as many commands need this to be the case.
   //
   // NOTE: Clearing the response buffer may cause the command buffer to be
   //       cleared if the buffers are shared.  Use the cached copy to be safe.
   //
   memset (RspBuf, 0, RspSize);

   //
   // Begin to process commands here
   //
   if (QstCmd.byCommand == QST_LEG_GET_SUBSYSTEM_CONFIG_PROFILE)
   {
      LegConfigProfileRsp->byStatus = ConfigProfileRsp->byStatus;
      LegConfigProfileRsp->wTempMonsConfigured = (UINT16) ConfigProfileRsp->dwTempMonsConfigured;
      LegConfigProfileRsp->byFanMonsConfigured = (UINT8) ConfigProfileRsp->dwFanMonsConfigured;
      LegConfigProfileRsp->byVoltMonsConfigured = (UINT8) ConfigProfileRsp->dwVoltMonsConfigured;
      LegConfigProfileRsp->byTempRespConfigured = (UINT16) ConfigProfileRsp->dwTempRespConfigured;
      LegConfigProfileRsp->byFanCtrlConfigured = (UINT8) ConfigProfileRsp->dwFanCtrlsConfigured;
   }
   else
   {
      //
      // Only need to copy data from one response buffer to the next.  Size
      // parameters must be validated prior to getting to this point.
      //
      if (MaxCopySize > 0)
      {
         memcpy (RspBuf, NewRspBuf, MaxCopySize);
      }
   }

   return TRUE;
}


/****************************************************************************/
/* External functions                                                       */
/****************************************************************************/

/****************************************************************************/
/* GetSubsystemInformation() - Sends the QST 2.0 GetSubsystemInfo command   */
/* to QST to determine the version level that is supported by the target    */
/* QST FW.  If the command fails a QST 1.0 FW must be assumed.              */
/****************************************************************************/

BOOL GetSubsystemInformation( void )
{
   QST_GENERIC_CMD                  QstCmd;
   QST_LEG_GENERIC_CMD              QstLegCmd;
   QST_GET_SUBSYSTEM_INFO_RSP       QstSubInfoRsp;
   QST_LEG_GET_SUBSYSTEM_STATUS_RSP QstSubStatusRsp;
   BOOL                             CmdSuccess;

   //
   // Use cached data if data has been buffered
   //
   if (QstSubsystemInfoFound)
   {
      return TRUE;
   }

   //
   // Send the subsystem info message to QST to get version information
   //
   QstCmd.stHeader.byCommand = QST_GET_SUBSYSTEM_INFO;
   QstCmd.stHeader.byEntity = 0;
   QstCmd.stHeader.wCommandLength = 0;
   QstCmd.stHeader.wResponseLength = sizeof(QST_GET_SUBSYSTEM_INFO_RSP);

   CmdSuccess = CommonCmdHandler (&QstCmd, sizeof(QST_GENERIC_CMD), &QstSubInfoRsp, QstCmd.stHeader.wResponseLength);
   if (CmdSuccess && !QstSubInfoRsp.byStatus)
   {
      memcpy (&QstSubsystemInfo, &QstSubInfoRsp, sizeof(QST_GET_SUBSYSTEM_INFO_RSP));
   }
   else
   {
      //
      // Subsystem Info message failed so see if we are on a 1.x QST system
      //
      QstLegCmd.stHeader.byCommand = QST_LEG_GET_SUBSYSTEM_STATUS;
      QstLegCmd.stHeader.wCommandLength = 0;
      QstLegCmd.stHeader.wResponseLength = sizeof(QST_LEG_GET_SUBSYSTEM_STATUS_RSP);

      CmdSuccess = CommonCmdHandler (&QstLegCmd, sizeof(QST_LEG_GENERIC_CMD), &QstSubStatusRsp, QstLegCmd.stHeader.wResponseLength);
      if (!CmdSuccess || QstSubStatusRsp.byStatus != QST_CMD_SUCCESSFUL)
      {
         return FALSE;
      }
   }

   //
   // Mark the data as being valid
   //
   QstSubsystemInfoFound = TRUE;

   return TRUE;
}

/****************************************************************************/
/* TranslationToLegacyRequired() - Determines if the current command set    */
/* needs to be translated to the legacy command set based on the current    */
/* firmware being used.                                                     */
/****************************************************************************/

BOOL TranslationToLegacyRequired( void )
{
   //
   // Check the major version of the firmware to see if any translation is
   // required for QST commands.
   //
   if (QstSubsystemInfo.uMajorVersionNumber <= QST1_MAJOR_VER_NUM_MAX)
   {
      return TRUE;
   }

   return FALSE;
}

/****************************************************************************/
/* TranslationToNewRequired() - Determines if the legacy command set        */
/* needs to be translated to the current command set based on the current   */
/* firmware being used.                                                     */
/****************************************************************************/

BOOL TranslationToNewRequired( void )
{
   //
   // Check the major version of the firmware to see if any translation is
   // required for QST commands.
   //
   if (QstSubsystemInfo.uMajorVersionNumber > QST1_MAJOR_VER_NUM_MAX)
   {
      return TRUE;
   }

   return FALSE;
}

/****************************************************************************/
/* TranslateToLegacyCommand() - Translates a QST 2.x command into a legacy  */
/* QST command.  This is done to allow new QST applications work on older   */
/* revisions of the firmware.                                               */
/****************************************************************************/

CMD_TRANSLATION_STATUS TranslateToLegacyCommand(

   IN  void       *pvCmdBuf,          // Address of buffer contaiing command packet
   IN  size_t     tCmdSize,           // Size of command packet
   OUT void       *pvRspBuf,          // Address of buffer for response packet
   IN  size_t     tRspSize            // Expected size of response packet
){
   BOOL              bSucceeded = FALSE;
   void              *LegCmdBuf = NULL;
   size_t            LegCmdSize = 0;
   void              *LegRspBuf = NULL;
   size_t            LegRspSize = 0;
   QST_CMD_HEADER    *QstCmd    = pvCmdBuf;
   UINT8             LegCmdCode = 0;
   size_t            MinRspSize = 0;
   size_t            MaxRspSize = 0;

   //
   // Convert the command and entity information into a legacy command
   //
   if (!ConvertToLegQstCommand (QstCmd, &LegCmdCode, &LegCmdSize, &LegRspSize))
   {
      //
      // Special handling for commands not supported for QST 1.x but that can
      // not fail per the QST 2.x command specification.
      //
      if (QstCmd->byCommand == QST_GET_SUBSYSTEM_INFO)
      {
         //
         // Special case for this command.  Return default data.
         //
         if (tRspSize == sizeof(QST_GET_SUBSYSTEM_INFO_RSP))
         {
            memcpy (pvRspBuf, &QstSubsystemInfo, sizeof(QST_GET_SUBSYSTEM_INFO_RSP));
            ((QST_GET_SUBSYSTEM_INFO_RSP*) pvRspBuf)->byStatus = QST_CMD_SUCCESSFUL;
            return TRANSLATE_CMD_SUCCESS;
         }

         //
         // Response buffer bad so exit with a failure
         //
         if (tRspSize >= sizeof(QST_GENERIC_RSP))
         {
            ((QST_GENERIC_RSP*) pvRspBuf)->byStatus = QST_CMD_REJECTED_RSP_SIZE;
         }
         return TRANSLATE_CMD_INVALID_PARAMETER;
      }
      else
      {
         //
         // Handler for commands that just return zero filled buffers if not
         // supported.
         //
         switch (QstCmd->byCommand)
         {
         case QST_GET_CURR_MON_UPDATE:
            MinRspSize = QST_CURR_MON_UPDATE_RSP_SIZE(1);
            MaxRspSize = QST_CURR_MON_UPDATE_RSP_SIZE(QST_ABS_CURR_MONITORS);
            break;

         case QST_GET_CPU_CONFIG_UPDATE:
            MinRspSize = MaxRspSize = sizeof(QST_GET_CPU_CONFIG_UPDATE_RSP);
            break;

         case QST_GET_FAN_CONFIG_UPDATE:
            MinRspSize = MaxRspSize = sizeof(QST_GET_FAN_CONFIG_UPDATE_RSP);
            break;

         case QST_GET_CPU_DTS_CONFIG_UPDATE:
            MinRspSize = MaxRspSize = sizeof(QST_GET_CPU_DTS_UPDATE_RSP);
            break;

         default:
            //
            // Return an error for the command as it could not be translated
            //
            if (tRspSize >= sizeof(QST_GENERIC_RSP))
            {
               ((QST_GENERIC_RSP*) pvRspBuf)->byStatus = QST_CMD_REJECTED_UNSUPPORTED;
            }
            return TRANSLATE_CMD_BAD_COMMAND;
         }

         //
         // Return buffer of all zeros as no sensors supported
         //
         if (tRspSize >= MinRspSize && tRspSize <= MaxRspSize)
         {
            memset (pvRspBuf, 0, tRspSize);
            return TRANSLATE_CMD_SUCCESS;
         }

         //
         // Response buffer bad so exit with a failure
         //
         if (tRspSize >= sizeof(QST_GENERIC_RSP))
         {
            ((QST_GENERIC_RSP*) pvRspBuf)->byStatus = QST_CMD_REJECTED_RSP_SIZE;
         }
         return TRANSLATE_CMD_INVALID_PARAMETER;
      }
   }

   //
   // Handle buffer sizes for SST PT commands here
   //
   if (QstCmd->byCommand == QST_SST_PASS_THROUGH)
   {
      LegCmdSize = QstCmd->wCommandLength + sizeof(QST_LEG_CMD_HEADER);
      LegRspSize = QstCmd->wResponseLength;
   }

   //
   // Allocate buffers for commands
   //
   LegCmdBuf = malloc (LegCmdSize);
   LegRspBuf = malloc (LegRspSize);
   if (LegCmdBuf == NULL || LegRspBuf == NULL)
   {
      //
      // Free any allocated buffers
      //
      if (LegCmdBuf != NULL)
      {
         free (LegCmdBuf);
      }
      if (LegRspBuf != NULL)
      {
         free (LegRspBuf);
      }

      //
      // Set error and return
      //
      if (tRspSize >= sizeof(QST_GENERIC_RSP))
      {
         ((QST_GENERIC_RSP*) pvRspBuf)->byStatus = QST_CMD_FAILED_NO_MEMORY;
      }
      return TRANSLATE_CMD_NOT_ENOUGH_MEMORY;
   }

   //
   // Copy command data into legacy command format (May need some additional translation)
   //
   if (!ConvertToLegCommandData (pvCmdBuf, tCmdSize, LegCmdCode, LegCmdBuf, LegCmdSize, LegRspSize))
   {
      //
      // Error so free buffers
      //
      free (LegCmdBuf);
      free (LegRspBuf);

      //
      // Sett error codes
      //
      if (tRspSize >= sizeof(QST_GENERIC_RSP))
      {
         ((QST_GENERIC_RSP*) pvRspBuf)->byStatus = QST_CMD_REJECTED_PARAMETER;
      }
      return TRANSLATE_CMD_INVALID_PARAMETER;
   }

   //
   // Call common command handler (Calls SetLastError before exit if failed)
   //
   bSucceeded = CommonCmdHandler (LegCmdBuf, LegCmdSize, LegRspBuf, LegRspSize);

   //
   // Convert legacy response data to the current response format
   //
   if (!ConvertToNewResponseData (QstCmd, LegRspBuf, LegRspSize, pvRspBuf, tRspSize))
   {
      //
      // Error so free the buffers
      //
      free (LegCmdBuf);
      free (LegRspBuf);

      //
      // Sett error codes
      //
      if (tRspSize >= sizeof(QST_GENERIC_RSP))
      {
         ((QST_GENERIC_RSP*) pvRspBuf)->byStatus = QST_CMD_REJECTED_PARAMETER;
      }
      return TRANSLATE_CMD_INVALID_PARAMETER;
   }

   //
   // Free buffers
   //
   free (LegCmdBuf);
   free (LegRspBuf);

   //
   // Check to see if the command being sent had issues
   //
   if (!bSucceeded)
   {
      return TRANSLATE_CMD_FAILED_WITH_ERROR_SET;
   }

   return TRANSLATE_CMD_SUCCESS;
}

/****************************************************************************/
/* TranslateToNewCommand() - Translates a QST 1.x command into a 2.x        */
/* QST command.  This is done to allow old QST applications work on newer   */
/* revisions of the firmware.                                               */
/****************************************************************************/

CMD_TRANSLATION_STATUS TranslateToNewCommand(

   IN  void       *pvCmdBuf,          // Address of buffer contaiing command packet
   IN  size_t     tCmdSize,           // Size of command packet
   OUT void       *pvRspBuf,          // Address of buffer for response packet
   IN  size_t     tRspSize            // Expected size of response packet
){
   BOOL                 bSucceeded     = FALSE;
   void                 *NewCmdBuf     = NULL;
   size_t               NewCmdSize     = 0;
   void                 *NewRspBuf     = NULL;
   size_t               NewRspSize     = 0;
   QST_LEG_CMD_HEADER   *QstCmd        = pvCmdBuf;
   UINT8                NewCmdCode     = 0;
   UINT8                NewCmdEntity   = 0;

   //
   // Convert the command to the new command set
   //
   if (!ConvertToNewQstCommand (QstCmd, &NewCmdCode, &NewCmdEntity, &NewCmdSize, &NewRspSize))
   {
      //
      // Unable to decode the command so bail...
      //
      if (tRspSize >= sizeof(QST_LEG_GENERIC_RSP))
      {
         ((QST_LEG_GENERIC_RSP*) pvRspBuf)->byStatus = QST_CMD_REJECTED_UNSUPPORTED;
      }
      return TRANSLATE_CMD_BAD_COMMAND;
   }

   //
   // Handle SST PT command sizes here
   //
   if (QstCmd->byCommand == QST_LEG_SST_PASS_THROUGH)
   {
      NewCmdSize = QstCmd->wCommandLength + sizeof(QST_GENERIC_CMD);
      NewRspSize = QstCmd->wResponseLength;
   }

   //
   // Allocate buffers for commands
   //
   NewCmdBuf = malloc (NewCmdSize);
   NewRspBuf = malloc (NewRspSize);
   if (NewCmdBuf == NULL || NewRspBuf == NULL)
   {
      //
      // Free any allocated buffers
      //
      if (NewCmdBuf != NULL)
      {
         free (NewCmdBuf);
      }
      if (NewRspBuf != NULL)
      {
         free (NewCmdBuf);
      }

      //
      // Return memory error
      //
      if (tRspSize >= sizeof(QST_LEG_GENERIC_RSP))
      {
         ((QST_LEG_GENERIC_RSP*) pvRspBuf)->byStatus = QST_CMD_FAILED_NO_MEMORY;
      }
      return TRANSLATE_CMD_NOT_ENOUGH_MEMORY;
   }

   //
   // Copy legacy command data into new command format
   //
   if (!ConvertToNewCommandData (pvCmdBuf, tCmdSize, NewCmdCode, NewCmdEntity, NewCmdBuf, NewCmdSize, NewRspSize))
   {
      //
      // Error converting so free buffers
      //
      free (NewCmdBuf);
      free (NewRspBuf);

      //
      // Sett error codes
      //
      if (tRspSize >= sizeof(QST_LEG_GENERIC_RSP))
      {
         ((QST_LEG_GENERIC_RSP*) pvRspBuf)->byStatus = QST_CMD_REJECTED_PARAMETER;
      }
      return TRANSLATE_CMD_INVALID_PARAMETER;
   }

   //
   // Call common command handler (Calls SetLastError before exit if failed)
   //
   bSucceeded = CommonCmdHandler (NewCmdBuf, NewCmdSize, NewRspBuf, NewRspSize);

   //
   // Convert new response data to the legacy response format
   //
   if (!ConvertToLegResponseData (QstCmd, NewRspBuf, NewRspSize, pvRspBuf, tRspSize))
   {
      //
      // Error so free the buffers and set error
      //
      free (NewCmdBuf);
      free (NewRspBuf);

      //
      // Sett error codes
      //
      if (tRspSize >= sizeof(QST_LEG_GENERIC_RSP))
      {
         ((QST_LEG_GENERIC_RSP*) pvRspBuf)->byStatus = QST_CMD_REJECTED_PARAMETER;
      }
      return TRANSLATE_CMD_INVALID_PARAMETER;
   }

   //
   // Free buffers
   //
   free (NewCmdBuf);
   free (NewRspBuf);

   //
   // Check to see if the command being sent had issues
   //
   if (!bSucceeded)
   {
      return TRANSLATE_CMD_FAILED_WITH_ERROR_SET;
   }

   return TRANSLATE_CMD_SUCCESS;
}


