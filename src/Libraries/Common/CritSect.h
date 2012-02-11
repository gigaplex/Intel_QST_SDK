/****************************************************************************/
/*                                                                          */
/*  Module:         CritSect.h                                              */
/*                                                                          */
/*  Description:    Provides function prototypes and definitions for using  */
/*                  the module implementing support for critical sections.  */
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

#ifndef _CRITSECT_H
#define _CRITSECT_H

#include "typedef.h"

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(1)

/****************************************************************************/
/* Definitions                                                              */
/****************************************************************************/

#ifndef HCRITSECT_DEFINED
#define HCRITSECT_DEFINED
typedef void * HCRITSECT;
#endif

/****************************************************************************/
/*  LookupCritSect() - Returns a handle for the operator that is  used  to  */
/*  implement  the  specified critical section type. If this operator does  */
/*  not exist, the function will return NULL.                               */
/****************************************************************************/

HCRITSECT LookupCritSect( U32 uSectionType );

/****************************************************************************/
/*  CreateCritSect() - Returns a handle for the operator that is  used  to  */
/*  implement  the  specified  critical section type. The bMustCreate flag  */
/*  indicates how the function should handle existing operators. If set to  */
/*  TRUE, the function will fail (return NULL) if the  operator  (already)  */
/*  exists.  If  set  to  FALSE, the function will return a handle for the  */
/*  existing operator.                                                      */
/****************************************************************************/

HCRITSECT CreateCritSect( U32 uSectionType, BOOL bMustCreate );

/****************************************************************************/
/*  CloseCritSect() - Dereferences the handle for  the  operator  that  is  */
/*  used  to implement the specified critical section type. If this is the  */
/*  last handle that was referencing the operator, the  operator  will  be  */
/*  deleted.                                                                */
/****************************************************************************/

BOOL CloseCritSect( HCRITSECT hCritSect );

/****************************************************************************/
/*  EnterCritSect() - Enters critical section referenced by the  specified  */
/*  handle.  If  another  thread  is already in the critical section, this  */
/*  thread will wait until it can  enter.  If  more  than  one  thread  is  */
/*  waiting  to  enter, this thread will be placed at the end of the queue  */
/*  of waiting threads ("back of the line, buddy").                         */
/****************************************************************************/

BOOL EnterCritSect( HCRITSECT hCritSect );

/****************************************************************************/
/*  LeaveCritSect() - Leaves critical section referenced by the  specified  */
/*  handle.  If  thread(s) are waiting to enter this critical section, the  */
/*  one that has been waiting the longest gets to enter.                    */
/****************************************************************************/

BOOL LeaveCritSect( HCRITSECT hCritSect );

#pragma pack()

#ifdef __cplusplus
}
#endif

#endif // ndef _CRITSECT_H

