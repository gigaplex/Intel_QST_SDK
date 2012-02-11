/****************************************************************************/
/*                                                                          */
/*  Module:         MilliTime.h                                             */
/*                                                                          */
/*  Description:    Provides prototypes and supporting definitions  for  a  */
/*                  set   of  portable  time  functions  with  millisecond  */
/*                  resolution.                                             */
/*                                                                          */
/*  Functions:      CurrMTime()   - Places  current  time  into  specified  */
/*                                  MILLITIME buffer.                       */
/*                                                                          */
/*                  CopyMTime()   - Copies  one  MILLITIME   variable   to  */
/*                                  another.                                */
/*                                                                          */
/*                  SetMTime()    - Sets  MILLITIME variable to a specific  */
/*                                  time value.                             */
/*                                                                          */
/*                  AddMTime()    - Adds the specified number  of  seconds  */
/*                                  and milliseconds to a MILLITIME value.  */
/*                                                                          */
/*                  PastMTime()   - Compares  specified  MILLITIME  values  */
/*                                  and returns a  boolean  indicating  if  */
/*                                  the  first is previous to (older than)  */
/*                                  the second.                             */
/*                                                                          */
/*                  WaitMTime()   - Delays execution until  the  specified  */
/*                                  MILLITIME time occurs.                  */
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

#ifndef _MILLITIME_H
#define _MILLITIME_H

#include <time.h>
#include "typedef.h"

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(1)

/****************************************************************************/
/* Definitions                                                              */
/****************************************************************************/

typedef struct _MILLITIME
{
   time_t         tTimeS;
   unsigned long  uTimeMS;

}  MILLITIME, *P_MILLITIME;

/****************************************************************************/
/* Function Prototypes                                                      */
/****************************************************************************/

void CurrMTime( P_MILLITIME pMTime );
void CopyMTime( P_MILLITIME pMTimeOut, P_MILLITIME pMTimeIn );
void SetMTime(  P_MILLITIME pMTime, time_t tSecs, unsigned long dwMillisecs );
void AddMTime(  P_MILLITIME pMTime, time_t tSecs, unsigned long dwMillisecs );
BOOL PastMTime( P_MILLITIME pMTime1, P_MILLITIME pMTime2 );
void WaitMTime( P_MILLITIME pMTime );

#pragma pack()

#ifdef __cplusplus
}
#endif

#endif /* ndef _MILLITIME_H */

