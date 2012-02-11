/****************************************************************************/
/*                                                                          */
/*  Module:         AccessQst.h                                             */
/*                                                                          */
/*  Description:    Defines the functions that are used  to  obtain/update  */
/*                  readings/duty   cycle  settings  from/to  the  various  */
/*                  sensors/controllers, through direct query of the  QST   */
/*                  Subsystem.                                              */
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

#ifndef _ACCESSQST_H
#define _ACCESSQST_H

#if defined(_WIN32) || defined(__WIN32__)
#include <windows.h>
#endif

#include "QstCmd.h"

/****************************************************************************/
/* Initialization Routines for access to QstComm subsystem                  */
/****************************************************************************/

BOOL   InitializeQst( BOOL bInitQstSeg );
void   CleanupQst( void );

/****************************************************************************/
/* Temperature Routines for access to QstComm subsystem                     */
/****************************************************************************/

BOOL   GetTempMonUpdateQst( void );
BOOL   SetTempThreshQst( int iLocSensor, float fNonCritical, float fCritical, float fNonRecoverable );

/****************************************************************************/
/* Fan Monitor Routines for access to QstComm subsystem                     */
/****************************************************************************/

BOOL   GetFanMonUpdateQst( void );
BOOL   SetFanThreshQst( int iLocSensor, float fNonCritical, float fCritical, float fNonRecoverable );

/****************************************************************************/
/* Voltage Monitor Routines for access to QstComm subsystem                 */
/****************************************************************************/

BOOL   GetVoltMonUpdateQst( void );
BOOL   SetVoltThreshLowQst( int iLocSensor, float fNonCritical, float fCritical, float fNonRecoverable );
BOOL   SetVoltThreshHighQst( int iLocSensor, float fNonCritical, float fCritical, float fNonRecoverable );

/****************************************************************************/
/* Current Monitor Routines for access to QstComm subsystem                 */
/****************************************************************************/

BOOL   GetCurrMonUpdateQst( void );
BOOL   SetCurrThreshLowQst( int iLocSensor, float fNonCritical, float fCritical, float fNonRecoverable );
BOOL   SetCurrThreshHighQst( int iLocSensor, float fNonCritical, float fCritical, float fNonRecoverable );

/****************************************************************************/
/* Fan Controller Routines for access to QstComm subsystem                  */
/****************************************************************************/

BOOL   GetFanCtrlUpdateQst( void );

#endif // ndef _ACCESSQST_H
