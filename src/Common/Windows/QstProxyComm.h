/****************************************************************************/
/*                                                                          */
/*  Module:         QstProxyComm.h                                          */
/*                                                                          */
/*  Description:    Provides necessary definitions for communications with  */
/*                  the  Intel(R)  Quiet System Technology (QST) Subsystem  */
/*                  via the Proxy Service.                                  */
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

#ifndef _QSTPROXYCOMM_H
#define _QSTPROXYCOMM_H

#include <windows.h>
#include <tchar.h>

#include "Service.h"
#include "QstCmd.h"

#pragma pack(1)

/****************************************************************************/
/* Definitions                                                              */
/****************************************************************************/

#define EXEC_QST_COMMAND    0x88

#define QST_SERVICE_NAME    __TEXT("Intel(R) Quiet System Technology Proxy Service")
#define QST_SERVICE_WINDOW  __TEXT("QstPseudoServiceWindow")

#define QST_COMM_SEG_NAME   __TEXT("Global\\QSTCommandSegment")
#define QST_COMM_MUTEX      __TEXT("Global\\QSTCommandMutex")

/****************************************************************************/
/* QST_PROXY_COMM_SEG - Global memory segment used to pass information to   */
/* and from the QST Proxy Service.                                          */
/****************************************************************************/

typedef struct _QST_PROXY_COMM_SEG
{
    // Buffer for name of semaphore to be used by service to signal completion

    TCHAR                   tszRspSem[64];

    // Buffer for QST Command/Response passing

    BYTE                    byCmdRsp[sizeof(QST_SET_SUBSYSTEM_CONFIG_CMD)];

    // Field for returning length of response

    WORD                    wLength;

} QST_PROXY_COMM_SEG, *P_QST_PROXY_COMM_SEG;

#pragma pack()

#endif // ndef _QSTPROXYCOMM_H

