/****************************************************************************/
/*                                                                          */
/*  Module:         QstProxyInst.h                                          */
/*                                                                          */
/*  Description:    Provides definitions for Windows Service that provides  */
/*                  a proxy for the instrumentation  of  the  sensors  and  */
/*                  controllers  being  managed  by  Intel(R) Quiet System  */
/*                  Technology (QST).                                       */
/*                                                                          */
/*  Notes:      1.  The  instrumentation proxy is an extension - a wrapper  */
/*                  around - the communications proxy.  See QstProxyComm.h  */
/*                  for more information.                                   */
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

#ifndef _QSTPROXYINST_H
#define _QSTPROXYINST_H

#include <windows.h>

#include "QstProxyComm.h"
#include "QstDll.h"

/****************************************************************************/
/* Command Codes specific to instrumentation supported by the Proxy Service */
/****************************************************************************/

#define SET_TEMP_THRESH             0x81
#define SET_FAN_THRESH              0x82
#define SET_VOLT_THRESH_LOW         0x83
#define SET_VOLT_THRESH_HIGH        0x84
#define SET_CURR_THRESH_LOW         0x85
#define SET_CURR_THRESH_HIGH        0x86
#define SET_POLLING_INTERVAL        0x87

/****************************************************************************/
/* QST_PROXY_INST_SEG - Global memory segment used to expose sensor and     */
/* controller information and support any necessary communication with the  */
/* proxy service.                                                           */
/****************************************************************************/

#pragma pack(4)

typedef struct _QST_PROXY_INST_SEG
{
    // Standard fields for communication via QST Proxy Services (must be first)

    QST_PROXY_COMM_SEG              stCommSeg;

    // Data from AccessQST module, the heart of the instrumentation capability

    QST_DATA_SEGMENT                stDataSeg;

    // Data for commands specific to instrumentation

    DWORD                           dwStatus;
    int                             iSensor;
    DWORD                           dwNewPollingInterval;
    QST_THRESH                      stThresh;

} QST_PROXY_INST_SEG, *P_QST_PROXY_INST_SEG;

#pragma pack()

#endif // ndef _QSTPROXYINST_H

