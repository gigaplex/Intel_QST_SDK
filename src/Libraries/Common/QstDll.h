/****************************************************************************/
/*                                                                          */
/*  Module:         QstDLL.h                                                */
/*                                                                          */
/*  Description:    Defines the functions and associated definitions  that  */
/*                  are  necessary to utilize the services of the Quiet     */
/*                  System Technology (QST)  Instrumentation  Layer  DLL.   */
/*                  This   DLL  exposes  the  Health  Monitoring  services  */
/*                  provided by the QST Subsystem.                          */
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

#ifndef _QSTDLL_H
#define _QSTDLL_H

#include <time.h>

#if defined(_WIN32) || defined(__WIN32__)
#include <windows.h>
#else
#define APIENTRY
#endif

#include "QstCmd.h"
#include "AccessQst.h"
#include "MilliTime.h"

/****************************************************************************/
/* Structures                                                               */
/****************************************************************************/

typedef struct _QST_THRESH
{
    float                           fNonCritical;
    float                           fCritical;
    float                           fNonRecoverable;

}  QST_THRESH, *P_QST_THRESH;

typedef struct _QST_DATA_SEGMENT
{
   BOOL                             bInitComplete;

   DWORD                            dwPollingInterval;
   time_t                           tTimePollingIntervalChanged;

   int                              iTempMons;
   int                              iTempMonIndex[QST_ABS_TEMP_MONITORS];
   QST_GET_TEMP_MON_CONFIG_RSP      stTempMonConfigRsp[QST_ABS_TEMP_MONITORS];
   QST_THRESH                       stTempMonThresh[QST_ABS_TEMP_MONITORS];
   time_t                           tTimeTempMonThreshChanged[QST_ABS_TEMP_MONITORS];
   QST_GET_TEMP_MON_UPDATE_RSP      stTempMonUpdateRsp;
   MILLITIME                        stTempMonUpdateTime;

   int                              iFanMons;
   int                              iFanMonIndex[QST_ABS_FAN_MONITORS];
   QST_GET_FAN_MON_CONFIG_RSP       stFanMonConfigRsp[QST_ABS_FAN_MONITORS];
   QST_THRESH                       stFanMonThresh[QST_ABS_TEMP_MONITORS];
   time_t                           tTimeFanMonThreshChanged[QST_ABS_FAN_MONITORS];
   QST_GET_FAN_MON_UPDATE_RSP       stFanMonUpdateRsp;
   MILLITIME                        stFanMonUpdateTime;

   int                              iVoltMons;
   int                              iVoltMonIndex[QST_ABS_VOLT_MONITORS];
   QST_GET_VOLT_MON_CONFIG_RSP      stVoltMonConfigRsp[QST_ABS_VOLT_MONITORS];
   QST_THRESH                       stVoltMonThreshLow[QST_ABS_TEMP_MONITORS];
   time_t                           tTimeVoltMonThreshLowChanged[QST_ABS_VOLT_MONITORS];
   QST_THRESH                       stVoltMonThreshHigh[QST_ABS_TEMP_MONITORS];
   time_t                           tTimeVoltMonThreshHighChanged[QST_ABS_VOLT_MONITORS];
   QST_GET_VOLT_MON_UPDATE_RSP      stVoltMonUpdateRsp;
   MILLITIME                        stVoltMonUpdateTime;

   int                              iCurrMons;
   int                              iCurrMonIndex[QST_ABS_CURR_MONITORS];
   QST_GET_CURR_MON_CONFIG_RSP      stCurrMonConfigRsp[QST_ABS_CURR_MONITORS];
   QST_THRESH                       stCurrMonThreshLow[QST_ABS_TEMP_MONITORS];
   time_t                           tTimeCurrMonThreshLowChanged[QST_ABS_CURR_MONITORS];
   QST_THRESH                       stCurrMonThreshHigh[QST_ABS_TEMP_MONITORS];
   time_t                           tTimeCurrMonThreshHighChanged[QST_ABS_CURR_MONITORS];
   QST_GET_CURR_MON_UPDATE_RSP      stCurrMonUpdateRsp;
   MILLITIME                        stCurrMonUpdateTime;

   int                              iFanCtrls;
   int                              iFanCtrlIndex[QST_ABS_FAN_CONTROLLERS];
   QST_GET_FAN_CTRL_CONFIG_RSP      stFanCtrlConfigRsp[QST_ABS_FAN_CONTROLLERS];
   QST_GET_FAN_CTRL_UPDATE_RSP      stFanCtrlUpdateRsp;
   MILLITIME                        stFanCtrlUpdateTime;


}  QST_DATA_SEGMENT, *P_QST_DATA_SEGMENT;

/****************************************************************************/
/* Global Variables                                                         */
/****************************************************************************/

extern QST_DATA_SEGMENT             *pQstSeg;

#if defined(__WIN32__)
   extern DWORD                     dwInitError;
#else
   extern int                       iInitErrno;
#endif

/****************************************************************************/
/* Function Prototypes                                                      */
/****************************************************************************/

// Module QstDLL.c

BOOL BeginCriticalSection( void );
void EndCriticalSection( void );
BOOL UpdatePollingInterval( DWORD dwInterval );

#endif // ndef _QSTDLL_H

