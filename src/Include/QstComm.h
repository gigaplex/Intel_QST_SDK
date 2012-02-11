/****************************************************************************/
/*                                                                          */
/*  Module:       QstComm.h                                                 */
/*                                                                          */
/*  Description:  Provides the function and  supporting  definitions  that  */
/*                are  necessary  to  utilize  the  QstComm DLL.  This DLL  */
/*                facilitates communication between DOS, Linux and Windows  */
/*                applications  and  the  Intel(R) Quiet  System Technolgy  */
/*                (QST) Subsystem.                                          */
/*                                                                          */
/*  Notes:     1. When building for Windows and  utilizing  the  QstComm.c  */
/*                helper   module,   #define  DYNAMIC_DLL_LOADING   before  */
/*                including this header file. This definition  will  cause  */
/*                prototypes for QstInitialize()  and  QstCleanup() to  be  */
/*                exposed. These functions provide support for the dynamic  */
/*                loading and unloading of the QstComm DLL.                 */
/*                                                                          */
/*             2. When building for DOS,  prototypes  for  QstInitialize()  */
/*                and   QstCleanup()  are  automatically  exposed.   These  */
/*                functions are required in this environment.               */
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

#ifndef _QSTCOMM_H
#define _QSTCOMM_H

#include <stddef.h>

#if defined(_WIN32) || defined(__WIN32__)
#include <windows.h>
#else
#include "typedef.h"
#define  APIENTRY
#endif

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(1)

/****************************************************************************/
/* Function Prototypes                                                      */
/****************************************************************************/

BOOL APIENTRY QstCommand( void *pvCmdBuf, size_t tCmdSize, void *pvRspBuf, size_t tRspSize );
BOOL APIENTRY QstCommand2( void *pvCmdBuf, size_t tCmdSize, void *pvRspBuf, size_t tRspSize );

#if defined(DYNAMIC_DLL_LOADING) || defined(_DOS) || defined(__DOS__) || defined(MSDOS)
BOOL QstInitialize( void );
void QstCleanup( void );
#endif

#if defined(_WIN32) || defined(__WIN32__)
/****************************************************************************/
/* Definitions for explicit DLL loading in the Windows environment          */
/****************************************************************************/

typedef BOOL (APIENTRY *PFN_QST_COMMAND)( void *pvCmdBuf, size_t tCmdSize, void *pvRspBuf, size_t tRspSize );

#define QST_COMM_DLL    __TEXT("QstComm.dll")

#define QST_COMM_ORD    1
#define QST_COMM2_ORD   2


#endif // defined(_WIN32) || defined(__WIN32__)

#pragma pack()

#ifdef __cplusplus
}
#endif

#endif // !defined(_QSTCOMM_H)
