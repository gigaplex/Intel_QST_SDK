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
#include "typedef.h"

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(1)

/****************************************************************************/
/* Liternal Definitions                                                     */
/****************************************************************************/

#ifndef ERROR_QST_BASE

#if _MSC_VER <= 800  // handle 16-bit MSVC environment

#define ERROR_QST_BASE                     0x5C00
#define QST_NOT_CONFIGURED                 0x00FF

#else

#define ERROR_QST_BASE                     0xAF5C0000
#define QST_NOT_CONFIGURED                 0x0100

#endif

#define ERROR_QST_UNSUPPORTED_COMMAND      (ERROR_QST_BASE | QST_CMD_REJECTED_UNSUPPORTED)
#define ERROR_QST_CAPABILITY_LOCKED        (ERROR_QST_BASE | QST_CMD_REJECTED_LOCKED)
#define ERROR_QST_INVALID_PARAMETER        (ERROR_QST_BASE | QST_CMD_REJECTED_PARAMETER)
#define ERROR_QST_INVALID_VERSION          (ERROR_QST_BASE | QST_CMD_REJECTED_VERSION)
#define ERROR_QST_DEVICE_COMM_ERROR        (ERROR_QST_BASE | QST_CMD_FAILED_COMM_ERROR)
#define ERROR_QST_DEVICE_SENSOR_ERROR      (ERROR_QST_BASE | QST_CMD_FAILED_SENSOR_ERROR)
#define ERROR_QST_NO_MEMORY                (ERROR_QST_BASE | QST_CMD_FAILED_NO_MEMORY)
#define ERROR_QST_NO_RESOURCES             (ERROR_QST_BASE | QST_CMD_FAILED_NO_RESOURCES)
#define ERROR_QST_INVALID_COMMAND          (ERROR_QST_BASE | QST_CMD_REJECTED_INVALID)
#define ERROR_QST_IMPROPER COMMAND_SIZE    (ERROR_QST_BASE | QST_CMD_REJECTED_CMD_SIZE)
#define ERROR_QST_IMPROPER_RESPONSE_SIZE   (ERROR_QST_BASE | QST_CMD_REJECTED_RSP_SIZE)
#define ERROR_QST_NOT_CONFIGURED           (ERROR_QST_BASE | QST_NOT_CONFIGURED)

#define ERROR_QST_FIRST                    ERROR_QST_UNSUPPORTED_COMMAND
#define ERROR_QST_LAST                     ERROR_QST_NOT_CONFIGURED

#if defined(_WIN32) || defined(__WIN32__)
#define QST_STATUS_TO_ERROR(x)             (ERROR_QST_BASE | (DWORD)(x))
#endif

#define QST_STATUS_TO_ERRNO(x)             (int)(ERROR_QST_BASE | (x))

#define IS_QST_ERROR(x)                    (((x) >= ERROR_QST_FIRST) && ((x) <= ERROR_QST_LAST))

#endif // ndef ERROR_QST_BASE

/****************************************************************************/
/* Structure Definitions                                                    */
/****************************************************************************/

#ifndef THRESH_DEFINED
#define THRESH_DEFINED

typedef struct _THRESH
{
   float    fNonCrit;
   float    fCrit;
   float    fNonRecov;

}  THRESH, *P_THRESH;

#endif // ndef THRESHOLD_SET_DEFINED

/****************************************************************************/
/* Function Prototypes                                                      */
/****************************************************************************/

BOOL    InitializeQst( void );
void    CleanupQst( void );

int     GetTempCountQst( void );
int     GetTempIndexQst( int iSensorIndex );
int     GetTempUsageQst( int iSensorIndex );
float   GetTempReadingQst( int iSensorIndex );
int     GetTempHealthQst( int iSensorIndex );
BOOL    GetTempHealthByteQst( int iSensorIndex, QST_MON_HEALTH_STATUS *pstStatus );
BOOL    GetTempThreshQst( int iSensorIndex, THRESH *pstThresh );

int     GetFanCountQst( void );
int     GetFanIndexQst( int iSensorIndex );
int     GetFanUsageQst( int iSensorIndex );
float   GetFanReadingQst( int iSensorIndex );
int     GetFanHealthQst( int iSensorIndex );
BOOL    GetFanHealthByteQst( int iSensorIndex, QST_MON_HEALTH_STATUS *pstStatus );
BOOL    GetFanThreshQst( int iSensorIndex, THRESH *pstThresh );

int     GetVoltCountQst( void );
int     GetVoltIndexQst( int iSensorIndex );
int     GetVoltUsageQst( int iSensorIndex );
float   GetVoltReadingQst( int iSensorIndex );
int     GetVoltHealthQst( int iSensorIndex );
BOOL    GetVoltHealthByteQst( int iSensorIndex, QST_MON_HEALTH_STATUS *pstStatus );
BOOL    GetVoltThreshQst( int iSensorIndex, THRESH *pstThreshLow, THRESH *pstThreshHigh );

int     GetCurrCountQst( void );
int     GetCurrIndexQst( int iSensorIndex );
int     GetCurrUsageQst( int iSensorIndex );
float   GetCurrReadingQst( int iSensorIndex );
int     GetCurrHealthQst( int iSensorIndex );
BOOL    GetCurrHealthByteQst( int iSensorIndex, QST_MON_HEALTH_STATUS *pstStatus );
BOOL    GetCurrThreshQst( int iSensorIndex, THRESH *pstThreshLow, THRESH *pstThreshHigh );

int     GetDutyCountQst( void );
int     GetDutyIndexQst( int iControllerIndex );
int     GetDutyUsageQst( int iControllerIndex );
float   GetDutySettingQst( int iControllerIndex );
int     GetDutyHealthQst( int iControllerIndex );
BOOL    GetDutyHealthByteQst( int iSensorIndex, QST_FAN_CTRL_STATUS *pstStatus );

BOOL    GetDutyManualQst( int iControllerIndex, BOOL *pbSWControl );
BOOL    SetDutyManualQst( int iControllerIndex, float fDutyCycle );
BOOL    SetDutyAutoQst( int iControllerIndex );

#pragma pack()

#ifdef __cplusplus
}
#endif

#endif // ndef _ACCESSQST_H

