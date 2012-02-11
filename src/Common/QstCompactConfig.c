/****************************************************************************/
/*                                                                          */
/*  Module:         QstCompactConfig.c                                      */
/*                                                                          */
/*  Description:    Provides functions used to compact the QST config.      */
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

#include "QstCompactConfig.h"

/****************************************************************************/
/* CompactConfig () - Compacts the configuration by removing any disabled   */
/* entities that may exist in the binary configuration payload.  The size   */
/* of the compacted configuration is returned to the caller.                */
/****************************************************************************/
MANIP_STATUS
CompactConfig (
   void     *ExpConfig,
   UINT32   ExpConfigSize,
   void     *CompConfig,
   UINT32   *CompConfigSize
   )
{
   UINT8                      Index;
   UINT8                      NumResponses;
   UINT8                      NumUsedResponses;
   UINT8                      CopySize;
   UINT8                      *ExpCfg = (UINT8*)ExpConfig;
   UINT8                      *ExpCfgBase = (UINT8*)ExpConfig;
   UINT8                      *CompCfg = (UINT8*)CompConfig;
   UINT8                      *CompCfgBase = (UINT8*)CompConfig;
   QST_PAYLOAD_HEADER_STRUCT  *CfgHeader;
   QST_HEADER_STRUCT          *EntityHeader;
   QST_FAN_CONTROLLER_STRUCT  *FanCtrl;

   //
   // Validate data buffer pointer values
   //
   if (ExpConfig == NULL || CompConfig == NULL || CompConfigSize == NULL)
   {
      return MANIP_INVALID_PARAMETER;
   }

   //
   // Zero fill the output buffer
   //
   memset(CompConfig, 0, *CompConfigSize);

   //
   // Validate buffer sizes
   //
   if (ExpConfigSize < sizeof(QST_PAYLOAD_HEADER_STRUCT) ||
       *CompConfigSize < sizeof(QST_PAYLOAD_HEADER_STRUCT) ||
       ExpConfigSize > QST_ABS_PAYLOAD_SIZE)
   {
      return MANIP_BUFFER_TOO_SMALL;
   }

   //
   // Check that the QST configuration header exists and copy it
   //
   CfgHeader = (QST_PAYLOAD_HEADER_STRUCT*)ExpCfg;
   if (memcmp( &CfgHeader->Signature, QST_SIGNATURE_DWORD, 4 ) != 0 ||
       CfgHeader->VersionMajor < MANIP_MIN_CFG_MAJOR_VERSION ||
       CfgHeader->PayloadLength != ExpConfigSize)
   {
      return MANIP_INVALID_HEADER;
   }

   //
   // Copy header information into output buffer
   //
   memcpy(CompCfg, ExpCfg, sizeof(QST_PAYLOAD_HEADER_STRUCT));

   //
   // Update pointer to first entity in config
   //
   CompCfg += sizeof(QST_PAYLOAD_HEADER_STRUCT);
   ExpCfg += sizeof(QST_PAYLOAD_HEADER_STRUCT);

   //
   // Loop through all the entries and remove any disabled entries
   //
   while ((((UINT32)(ExpCfg - ((UINT8*)ExpConfig))) < ExpConfigSize) &&
          (((UINT32)(CompCfg - ((UINT8*)CompConfig))) < *CompConfigSize))
   {
      //
      // Assign pointer to header information for the entity
      //
      EntityHeader = (QST_HEADER_STRUCT*)ExpCfg;

      //
      // Check for the entity to be enabled
      //
      if (!(EntityHeader->EntityEnabled))
      {
         //
         // Just need to update the read pointer and move to the
         // next entity.
         //
         ExpCfg += EntityHeader->StructLength;
         continue;
      }

      //
      // Set the default size to copy based off the structure length.
      //
      CopySize = EntityHeader->StructLength;

      //
      // Need to special case the fan controller as it can have a variable size
      //
      if (EntityHeader->EntityType == QST_FAN_CONTROLLER)
      {
         //
         // Get the number of weightings
         //
         NumResponses = (UINT8) QST_FAN_CONTROLLER_RESPONSES (EntityHeader->StructLength);

         //
         // Assign fan controller structure pointer.
         //
         // NOTE: The structure may map beyond the end of the data entry
         //
         FanCtrl = (QST_FAN_CONTROLLER_STRUCT*) ExpCfg;

         //
         // Determine last used response weighting entry
         //
         NumUsedResponses = 0;
         for (Index = 0; Index < NumResponses; Index++)
         {
            if (FanCtrl->ResponseWeighting[Index] != 0)
            {
               NumUsedResponses = (UINT8) (Index + 1);
            }
         }

         //
         // Now determine the minimum size of the structure that needs to be copied
         //
         CopySize = (UINT8) QST_FAN_CONTROLLER_SIZE(NumUsedResponses);
      }

      //
      // Entity is enabled so we need to see if the entity will fit in the
      // destination buffer.
      //
      if (((UINT32)((CompCfg + CopySize) - CompCfgBase)) > *CompConfigSize)
      {
         return MANIP_BUFFER_TOO_SMALL;
      }

      //
      // Also need to check to make sure that the copy does not overrun the
      // source buffer.
      //
      if (((UINT32)((ExpCfg + EntityHeader->StructLength) - ExpCfgBase)) > ExpConfigSize)
      {
         return MANIP_INVALID_CFG_FORMAT;
      }

      //
      // Copy data into buffer
      //
      memcpy(CompCfg, ExpCfg, CopySize);

      //
      // Update pointer into expanded configuration
      //
      ExpCfg += EntityHeader->StructLength;

      //
      // Update the entity header in the compacted config with the correct size.
      // This may be updated in some cases as all the data did not need to be
      // copied.
      //
      EntityHeader = (QST_HEADER_STRUCT*) CompCfg;
      EntityHeader->StructLength = CopySize;

      //
      // Update pointer into compacted buffer
      //
      CompCfg += CopySize;
   }

   //
   // Update the new configuration size
   //
   CfgHeader = (QST_PAYLOAD_HEADER_STRUCT*)CompConfig;
   CfgHeader->PayloadLength = (UINT16)(CompCfg - ((UINT8*)CompConfig));
   *CompConfigSize = (UINT32)(CompCfg - ((UINT8*)CompConfig));

   return MANIP_SUCCESS;
}


