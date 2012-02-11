/****************************************************************************/
/*                                                                          */
/*  Module:         CSMISMART.h                                             */
/*                                                                          */
/*  Description:    Provides the necessary definitions for the enumeration  */
/*                  and  exposure of failure predictions and attribute and  */
/*                  threshold data from SATA  Hard  Drives,  connected  to  */
/*                  controllers enabled with the Common Storage Management  */
/*                  Interface   (CSMI),   which   support  Self-Monitoring  */
/*                  Analysis and Reporting Technology (S.M.A.R.T.).         */
/*                                                                          */
/*  Notes:      1.  The API exposed by the CSMISMART module is:             */
/*                                                                          */
/*                    CSMISMARTInitialize()       Initializes  module  and  */
/*                                                returns  the  number  of  */
/*                                                SATA drives detected.     */
/*                                                                          */
/*                    CSMISMARTGetPhysicalIndex() Gets physical index  for  */
/*                                                the specified drive.      */
/*                                                                          */
/*                    CSMISMARTGetIdentifyData()  Gets Identify data block  */
/*                                                for the specified drive.  */
/*                                                                          */
/*                    CSMISMARTGetPrediction()    Indicates whether or not  */
/*                                                the  specified  drive is  */
/*                                                predicting its demise.    */
/*                                                                          */
/*                    CSMISMARTGetAttributeData() Gets attribute data  for  */
/*                                                the specified drive.      */
/*                                                                          */
/*                    CSMISMARTGetThresholdData() Gets  threshold data for  */
/*                                                the specified drive.      */
/*                                                                          */
/*                    CSMISMARTCleanup()          Cleans up resources used  */
/*                                                by the module.            */
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

#ifndef _CSMISMART_H
#define _CSMISMART_H

#include "SMART.h"

#ifdef __cplusplus
extern "C" {
#endif

BOOL CSMISMARTInitialize( int *piDrives );
BOOL CSMISMARTGetPhysicalIndex( int iDrive, int *piIndex );
BOOL CSMISMARTGetIdentifyData( int iDrive, void *pvBuffer );
BOOL CSMISMARTGetPrediction( int iDrive, BOOL *pbFailing );
BOOL CSMISMARTGetAttributeData( int iDrive, SMART_ATTRIB *pstAttrib );
BOOL CSMISMARTGetThresholdData( int iDrive, SMART_THRESH *pstThresh );
void CSMISMARTCleanup( void );

#ifdef __cplusplus
}
#endif

#endif // ndef _CSMISMART_H

