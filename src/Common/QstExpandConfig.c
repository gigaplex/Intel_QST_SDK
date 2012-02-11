/****************************************************************************/
/*                                                                          */
/*  Module:         QstExpandConfig.c                                       */
/*                                                                          */
/*  Description:    Provides functions used to expand the QST config.       */
/*                                                                          */
/*  Notes:      1.  Entity indices and instance numbers are 1-based within  */
/*                  the INI file but are 0-based within the binary data.    */
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

#include "QstExpandConfig.h"

/****************************************************************************/
/* Internal function headers                                                */
/****************************************************************************/
static
BOOL
GetExpandedConfigPointers (
   void                 *ExpandedCfgBuffer,
   QST_CFG_COUNTS       *ConfigCounts,
   QST_DYNAMIC_PAYLOAD  *CfgPayload
   );

static
BOOL
SetDefaultValues (
   void                 *ExpandedCfgBuffer,
   UINT32               ExpandedCfgSize,
   QST_CFG_COUNTS       *ConfigCounts,
   QST_DYNAMIC_PAYLOAD  *CfgPayload
   );


/****************************************************************************/
/* ExpandConfig () - Expands the configuration data into the desired QST    */
/* configuration format.                                                    */
/****************************************************************************/

MANIP_STATUS
ExpandConfig (
   void           *ExpConfig,
   UINT32         ExpConfigSize,
   QST_CFG_COUNTS *CfgCounts,
   void           *CompConfig,
   UINT32         CompBufferSize
   )
{
   QST_DYNAMIC_PAYLOAD        ExpandedConfigLayout = {0};
   UINT32                     ExpectedCfgSize = 0;
   UINT8                      *CompCfg = (UINT8*)CompConfig;
   UINT8                      *CompCfgBase = (UINT8*)CompConfig;
   QST_PAYLOAD_HEADER_STRUCT  *CfgHeader;
   QST_HEADER_STRUCT          *EntityHeader;
   void                       *CopyTarget;
   UINT32                     CompCfgSize = 0;
   UINT8                      ActualStructureSize;

   //
   // Check for valid pointers
   //
   if (ExpConfig == NULL || CfgCounts == NULL || CompConfig == NULL)
   {
      return MANIP_INVALID_PARAMETER;
   }

   //
   // Check to make sure that the buffer size is correct based on the number
   // of entities supported.
   //
   ExpectedCfgSize = GET_QST_CONFIG_SIZE(CfgCounts->TempMons,
                                         CfgCounts->FanMons,
                                         CfgCounts->VoltMons,
                                         CfgCounts->CurrMons,
                                         CfgCounts->TempRsps,
                                         CfgCounts->FanCtrls
                                         );
   if (ExpectedCfgSize != ExpConfigSize || CompBufferSize < sizeof(QST_PAYLOAD_HEADER_STRUCT))
   {
      return MANIP_BUFFER_TOO_SMALL;
   }

   //
   // Set dynamic configuration pointers
   //
   GetExpandedConfigPointers (ExpConfig, CfgCounts, &ExpandedConfigLayout);

   //
   // Now fill in all the individual headers so all entities are disabled...
   //
   SetDefaultValues (ExpConfig, ExpConfigSize, CfgCounts, &ExpandedConfigLayout);

   //
   // Now validate the configuration header
   //
   CfgHeader = (QST_PAYLOAD_HEADER_STRUCT*)CompCfg;
   if (memcmp( &CfgHeader->Signature, QST_SIGNATURE_DWORD, 4 ) != 0 ||
       CfgHeader->VersionMajor < MANIP_MIN_CFG_MAJOR_VERSION)
   {
      return MANIP_INVALID_CFG_FORMAT;
   }

   //
   // Set size of buffer that is used for the compacted configuration data.  This
   // was updated when compacted.
   //
   CompCfgSize = CfgHeader->PayloadLength;

   //
   // Copy the configuration header into buffer
   //
   memcpy(ExpConfig, CompCfg, sizeof(QST_PAYLOAD_HEADER_STRUCT));
   CompCfg += sizeof(QST_PAYLOAD_HEADER_STRUCT);

   //
   // Update payload size
   //
   CfgHeader = (QST_PAYLOAD_HEADER_STRUCT*) ExpConfig;
   CfgHeader->PayloadLength = (UINT16) ExpectedCfgSize;

   //
   // Now expand the entities into the expanded buffer.  Just need to read all
   // the entries in the compacted data structure.
   //
   while (((UINT32)(CompCfg - ((UINT8*)CompConfig))) < CompCfgSize)
   {
      //
      // Actual structure size.  Only non-zero if the structure size in the
      // compacted configuration is not the actual size of the expanded structure.
      // Set the variable to zero each iteration.
      //
      ActualStructureSize = 0;

      //
      // Assign pointer to entity header
      //
      EntityHeader = (QST_HEADER_STRUCT*)CompCfg;

      //
      // Only need to process enabled entries
      //
      if (!EntityHeader->EntityEnabled)
      {
         CompCfg += EntityHeader->StructLength;
         continue;
      }

      //
      // Decode the structure type and check to see if it will fit in the
      // new buffer.  Also get target copy location.
      //
      switch (EntityHeader->EntityType)
      {
      case QST_TEMP_MONITOR:
         if (EntityHeader->EntityIndex >= CfgCounts->TempMons ||
             ExpandedConfigLayout.TempMon == NULL)
         {
            return MANIP_INVALID_CFG_FORMAT;
         }
         CopyTarget = &(ExpandedConfigLayout.TempMon[EntityHeader->EntityIndex]);
         break;
      case QST_FAN_MONITOR:
         if (EntityHeader->EntityIndex >= CfgCounts->FanMons ||
             ExpandedConfigLayout.FanMon == NULL)
         {
            return MANIP_INVALID_CFG_FORMAT;
         }
         CopyTarget = &(ExpandedConfigLayout.FanMon[EntityHeader->EntityIndex]);
         break;
      case QST_VOLT_MONITOR:
         if (EntityHeader->EntityIndex >= CfgCounts->VoltMons ||
             ExpandedConfigLayout.VoltMon == NULL)
         {
            return MANIP_INVALID_CFG_FORMAT;
         }
         CopyTarget = &(ExpandedConfigLayout.VoltMon[EntityHeader->EntityIndex]);
         break;
      case QST_CURR_MONITOR:
         if (EntityHeader->EntityIndex >= CfgCounts->CurrMons ||
             ExpandedConfigLayout.CurrMon == NULL)
         {
            return MANIP_INVALID_CFG_FORMAT;
         }
         CopyTarget = &(ExpandedConfigLayout.CurrMon[EntityHeader->EntityIndex]);
         break;
      case QST_TEMP_RESPONSE:
         if (EntityHeader->EntityIndex >= CfgCounts->TempRsps ||
             ExpandedConfigLayout.TempRsp == NULL)
         {
            return MANIP_INVALID_CFG_FORMAT;
         }
         CopyTarget = &(ExpandedConfigLayout.TempRsp[EntityHeader->EntityIndex]);
         break;
      case QST_FAN_CONTROLLER:
         if (EntityHeader->EntityIndex >= CfgCounts->FanCtrls ||
             ExpandedConfigLayout.FanCtrl == NULL)
         {
            return MANIP_INVALID_CFG_FORMAT;
         }
         ActualStructureSize = (UINT8) QST_FAN_CONTROLLER_SIZE(CfgCounts->TempRsps);
         CopyTarget = ((UINT8*) (ExpandedConfigLayout.FanCtrl)) + (EntityHeader->EntityIndex * ActualStructureSize);
         break;
      case QST_PAYLOAD_HEADER:
      default:
         return MANIP_INVALID_CFG_FORMAT;
      }

      //
      // Check that the source buffer is large enough for the copy
      //
      if (((UINT32)((CompCfg + EntityHeader->StructLength) - CompCfgBase)) > CompCfgSize)
      {
         return MANIP_INVALID_CFG_FORMAT;
      }

      //
      // Copy data into the entry location
      //
      memcpy(CopyTarget, CompCfg, EntityHeader->StructLength);

      //
      // Update the header size to the make sure it is the actual size of the
      // structure.
      //
      if (ActualStructureSize != 0)
      {
         ((QST_HEADER_STRUCT*) CopyTarget)->StructLength = ActualStructureSize;
      }

      //
      // Move to next structure in compacted buffer
      //
      CompCfg += EntityHeader->StructLength;
   }

   return MANIP_SUCCESS;
}


/****************************************************************************/
/* GetExpandedConfigPointers () - The function computes the offsets in the  */
/* expanded buffer for each of the configuration sections.                  */
/****************************************************************************/

static
BOOL
GetExpandedConfigPointers (
   void                 *ExpandedCfgBuffer,
   QST_CFG_COUNTS       *ConfigCounts,
   QST_DYNAMIC_PAYLOAD  *CfgPayload
   )
{
   //
   // check all the pointers
   //
   if (ExpandedCfgBuffer == NULL || ConfigCounts == NULL || CfgPayload == NULL)
   {
      return FALSE;
   }

   //
   // Set the config header
   //
   CfgPayload->Header = (QST_PAYLOAD_HEADER_STRUCT*) ExpandedCfgBuffer;

   //
   // Set the TempMon location
   //
   if (ConfigCounts->FanMons == 0)
   {
      CfgPayload->FanMon = NULL;
   }
   else
   {
      CfgPayload->TempMon =
         (QST_TEMP_MONITOR_STRUCT*) (((UINT8*) ExpandedCfgBuffer) +
                                     GET_QST_CONFIG_SIZE(0,
                                                         0,
                                                         0,
                                                         0,
                                                         0,
                                                         0)
                                     );
   }

   //
   // Set the FanMon location
   //
   if (ConfigCounts->FanMons == 0)
   {
      CfgPayload->FanMon = NULL;
   }
   else
   {
      CfgPayload->FanMon =
         (QST_FAN_MONITOR_STRUCT*) (((UINT8*) ExpandedCfgBuffer) +
                                    GET_QST_CONFIG_SIZE(ConfigCounts->TempMons,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0)
                                    );
   }

   //
   // Set the VoltMon location
   //
   if (ConfigCounts->VoltMons == 0)
   {
      CfgPayload->VoltMon = NULL;
   }
   else
   {
      CfgPayload->VoltMon =
         (QST_VOLT_MONITOR_STRUCT*) (((UINT8*) ExpandedCfgBuffer) +
                                     GET_QST_CONFIG_SIZE(ConfigCounts->TempMons,
                                                         ConfigCounts->FanMons,
                                                         0,
                                                         0,
                                                         0,
                                                         0)
                                     );
   }

   //
   // Set the CurrMon location
   //
   if (ConfigCounts->CurrMons == 0)
   {
      CfgPayload->CurrMon = NULL;
   }
   else
   {
      CfgPayload->CurrMon =
         (QST_CURR_MONITOR_STRUCT*) (((UINT8*) ExpandedCfgBuffer) +
                                     GET_QST_CONFIG_SIZE(ConfigCounts->TempMons,
                                                         ConfigCounts->FanMons,
                                                         ConfigCounts->VoltMons,
                                                         0,
                                                         0,
                                                         0)
                                     );
   }

   //
   // Set the TempRsp location
   //
   if (ConfigCounts->TempRsps == 0)
   {
      CfgPayload->TempRsp = NULL;
   }
   else
   {
      CfgPayload->TempRsp =
         (QST_TEMP_RESPONSE_STRUCT*) (((UINT8*) ExpandedCfgBuffer) +
                                      GET_QST_CONFIG_SIZE(ConfigCounts->TempMons,
                                                          ConfigCounts->FanMons,
                                                          ConfigCounts->VoltMons,
                                                          ConfigCounts->CurrMons,
                                                          0,
                                                          0)
                                      );
   }

   //
   // Set the FanCtrl location
   //
   if (ConfigCounts->FanCtrls == 0)
   {
      CfgPayload->FanCtrl = NULL;
   }
   else
   {
      CfgPayload->FanCtrl =
         (UINT8*) (((UINT8*) ExpandedCfgBuffer) +
                   GET_QST_CONFIG_SIZE(ConfigCounts->TempMons,
                                       ConfigCounts->FanMons,
                                       ConfigCounts->VoltMons,
                                       ConfigCounts->CurrMons,
                                       ConfigCounts->TempRsps,
                                       0)
                   );
   }

   return TRUE;
}


/****************************************************************************/
/* SetDefaultValues () - Sets all entries to a default state so the data is */
/* valid with no entities enabled.                                          */
/****************************************************************************/

static
BOOL
SetDefaultValues (
   void                 *ExpandedCfgBuffer,
   UINT32               ExpandedCfgSize,
   QST_CFG_COUNTS       *ConfigCounts,
   QST_DYNAMIC_PAYLOAD  *CfgPayload
   )
{
   UINT8             Index;
   UINT8             FanCtrlSize;
   QST_HEADER_STRUCT *Header;

   //
   // Be sure that the buffer for the expanded data starts out clean
   //
   memset(ExpandedCfgBuffer, 0, ExpandedCfgSize);

   //
   // Fill in the header information
   //
   CfgPayload->Header->Signature = QST_SIGNATURE;
   CfgPayload->Header->VersionMajor = MANIP_MIN_CFG_MAJOR_VERSION;
   CfgPayload->Header->VersionMinor = 0;
   CfgPayload->Header->PayloadLength = (UINT16) ExpandedCfgSize;

   //
   // Fill in default header informtaion for temp monitors
   //
   for (Index = 0; Index < ConfigCounts->TempMons; Index++)
   {
      CfgPayload->TempMon[Index].Header.EntityEnabled = 0;
      CfgPayload->TempMon[Index].Header.EntityType = QST_TEMP_MONITOR;
      CfgPayload->TempMon[Index].Header.EntityIndex = Index;
      CfgPayload->TempMon[Index].Header.EntityUsage = 0;
      CfgPayload->TempMon[Index].Header.StructLength = QST_TEMP_MONITOR_SIZE;
   }

   //
   // Fill in default header informtaion for fan monitors
   //
   for (Index = 0; Index < ConfigCounts->FanMons; Index++)
   {
      CfgPayload->FanMon[Index].Header.EntityEnabled = 0;
      CfgPayload->FanMon[Index].Header.EntityType = QST_FAN_MONITOR;
      CfgPayload->FanMon[Index].Header.EntityIndex = Index;
      CfgPayload->FanMon[Index].Header.EntityUsage = 0;
      CfgPayload->FanMon[Index].Header.StructLength = QST_FAN_MONITOR_SIZE;
   }

   //
   // Fill in default header informtaion for voltage monitors
   //
   for (Index = 0; Index < ConfigCounts->VoltMons; Index++)
   {
      CfgPayload->VoltMon[Index].Header.EntityEnabled = 0;
      CfgPayload->VoltMon[Index].Header.EntityType = QST_VOLT_MONITOR;
      CfgPayload->VoltMon[Index].Header.EntityIndex = Index;
      CfgPayload->VoltMon[Index].Header.EntityUsage = 0;
      CfgPayload->VoltMon[Index].Header.StructLength = QST_VOLT_MONITOR_SIZE;
   }

   //
   // Fill in default header informtaion for voltage monitors
   //
   for (Index = 0; Index < ConfigCounts->CurrMons; Index++)
   {
      CfgPayload->CurrMon[Index].Header.EntityEnabled = 0;
      CfgPayload->CurrMon[Index].Header.EntityType = QST_CURR_MONITOR;
      CfgPayload->CurrMon[Index].Header.EntityIndex = Index;
      CfgPayload->CurrMon[Index].Header.EntityUsage = 0;
      CfgPayload->CurrMon[Index].Header.StructLength = QST_CURR_MONITOR_SIZE;
   }

   //
   // Fill in default header informtaion for response units
   //
   for (Index = 0; Index < ConfigCounts->TempRsps; Index++)
   {
      CfgPayload->TempRsp[Index].Header.EntityEnabled = 0;
      CfgPayload->TempRsp[Index].Header.EntityType = QST_TEMP_RESPONSE;
      CfgPayload->TempRsp[Index].Header.EntityIndex = Index;
      CfgPayload->TempRsp[Index].Header.EntityUsage = 0;
      CfgPayload->TempRsp[Index].Header.StructLength = QST_TEMP_RESPONSE_SIZE;
   }

   //
   // Fill in default header informtaion for response units
   //
   FanCtrlSize = (UINT8) QST_FAN_CONTROLLER_SIZE(ConfigCounts->TempRsps);
   for (Index = 0; Index < ConfigCounts->FanCtrls; Index++)
   {
      Header = (QST_HEADER_STRUCT*) (((UINT8*) (CfgPayload->FanCtrl)) + (Index * FanCtrlSize));
      Header->EntityEnabled = 0;
      Header->EntityType = QST_FAN_CONTROLLER;
      Header->EntityIndex = Index;
      Header->EntityUsage = 0;
      Header->StructLength = FanCtrlSize;
   }

   return TRUE;
}
