/****************************************************************************/
/*                                                                          */
/*  Module:         LegTrnaslationFuncs.h                                   */
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

#ifndef _LEG_TRANSLATION_FUNCS_H
#define _LEG_TRANSLATION_FUNCS_H

// Error platform independent error codes for using this module

typedef enum
{
   TRANSLATE_CMD_SUCCESS = 0,
   TRANSLATE_CMD_INVALID_PARAMETER,
   TRANSLATE_CMD_BAD_COMMAND,
   TRANSLATE_CMD_NOT_ENOUGH_MEMORY,
   TRANSLATE_CMD_FAILED_WITH_ERROR_SET

} CMD_TRANSLATION_STATUS;

// Function prototypes

BOOL GetSubsystemInformation( void );

BOOL TranslationToLegacyRequired( void );

BOOL TranslationToNewRequired( void );

CMD_TRANSLATION_STATUS TranslateToLegacyCommand(

   IN  void       *pvCmdBuf,          // Address of buffer contaiing command packet
   IN  size_t     tCmdSize,           // Size of command packet
   OUT void       *pvRspBuf,          // Address of buffer for response packet
   IN  size_t     tRspSize            // Expected size of response packet
);

CMD_TRANSLATION_STATUS TranslateToNewCommand(

   IN  void       *pvCmdBuf,          // Address of buffer contaiing command packet
   IN  size_t     tCmdSize,           // Size of command packet
   OUT void       *pvRspBuf,          // Address of buffer for response packet
   IN  size_t     tRspSize            // Expected size of response packet
);

BOOL CommonCmdHandler(

   IN  void       *pvCmdBuf,          // Address of buffer contaiing command packet
   IN  size_t     tCmdSize,           // Size of command packet
   OUT void       *pvRspBuf,          // Address of buffer for response packet
   IN  size_t     tRspSize            // Expected size of response packet
);

#endif // ndef _LEG_TRANSLATION_FUNCS_H
