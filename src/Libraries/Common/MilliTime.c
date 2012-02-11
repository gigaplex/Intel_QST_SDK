/****************************************************************************/
/*                                                                          */
/*  Module:         MilliTime.c                                             */
/*                                                                          */
/*  Description:    Implements a  set  of  portable  time  functions  with  */
/*                  millisecond resolution.                                 */
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

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>

#if defined(__WIN32__) || defined(_WIN32) || defined(_WIN64) || defined(__WINDOWS__)
#include <windows.h>
#include <tchar.h>
#include "sys/timeb.h"
#elif defined(__SOLARIS__) || defined(__sun__) || defined(__LINUX__) || defined(__linux__)
#include <sys/time.h>
#elif defined(__MSDOS__) || defined(MSDOS) || defined(_MSDOS) || defined(__DOS__)
#if (CLOCKS_PER_SEC != 1000)
#error clock() implementation does not support millisecond resolution!!
#endif
#endif

#include "MilliTime.h"

/****************************************************************************/
/* Delay() - Implements an 'n' millisecond delay.                           */
/****************************************************************************/

static void Delay( unsigned long uMilliseconds )
{
   if( uMilliseconds )
   {

#if defined(__LINUX__) || defined(__SOLARIS__)

      struct timespec stTS;

      stTS.tv_sec  = (time_t)(uMilliseconds / 1000);
      stTS.tv_nsec = 1000000L * (uMilliseconds % 1000);   // 1,000,000 ns = 1 ms

      while( (nanosleep( &stTS, &stTS ) == -1) && (errno == EINTR) );

#elif defined(_WIN32) || defined(__WIN32__)

      Sleep( uMilliseconds );

#elif defined(_DOS) || defined(__DOS__) || defined(MSDOS)

      clock_t tStart, tCurr, tEnd;

      tCurr = tStart = clock();
      tEnd  = tStart + (clock_t)uMilliseconds;

      // Handle wraparound

      if( tEnd < tCurr )
      {
         while( tCurr )
            tCurr = clock();
      }

      // Complete the delay

      while( tCurr < tEnd )
         tCurr = clock();

#endif

   }
}

/****************************************************************************/
/* CurrMTime() - Stores current time into specified MILLITIME buffer.       */
/****************************************************************************/

void CurrMTime( P_MILLITIME pMTime )
{

#if defined(__LINUX__) || defined(__SOLARIS__)

   struct timeval stTV;
   gettimeofday( &stTV, NULL );

   pMTime->tTimeS  = stTV.tv_sec;
   pMTime->uTimeMS = stTV.tv_usec / 1000;

#elif defined(_WIN32) || defined(__WIN32__)

   struct _timeb st;

   _ftime( &st );

   pMTime->tTimeS  = st.time;
   pMTime->uTimeMS = st.millitm;

#elif defined(_DOS) || defined(__DOS__) || defined(MSDOS)

   pMTime->uTimeMS = clock() % 1000;
   pMTime->tTimeS  = time( NULL );

#endif

}

/****************************************************************************/
/* SetMTime() - Sets a MILLITIME variable with specific time values         */
/****************************************************************************/

void SetMTime( P_MILLITIME pMTime, time_t tSecs, unsigned long wMillisecs )
{
   pMTime->tTimeS  = tSecs;
   pMTime->uTimeMS = wMillisecs;
}

/****************************************************************************/
/* CopyMTime() - Copies one MILLITIME variable to another                   */
/****************************************************************************/

void CopyMTime( P_MILLITIME pMTimeOut, P_MILLITIME pMTimeIn )
{
   pMTimeOut->tTimeS  = pMTimeIn->tTimeS;
   pMTimeOut->uTimeMS = pMTimeIn->uTimeMS;
}

/****************************************************************************/
/* AddMTime() - Adds the specified number of seconds and milliseconds to a  */
/* MILLITIME value.                                                         */
/****************************************************************************/

void AddMTime( P_MILLITIME pMTime, time_t tSecs, unsigned long uMillisecs )
{
   DWORD dwMS = pMTime->uTimeMS + uMillisecs;

   pMTime->tTimeS  += (dwMS / 1000) + tSecs;
   pMTime->uTimeMS  = dwMS % 1000;
}

/****************************************************************************/
/* PastMTime() - Compares specified MILLITIME values and returns a boolean  */
/* indicating if the first is previous to (older than) the second.          */
/****************************************************************************/

BOOL PastMTime( P_MILLITIME pMTime1, P_MILLITIME pMTime2 )
{
   if( pMTime1->tTimeS > pMTime2->tTimeS )
      return( FALSE );

   if( pMTime1->tTimeS < pMTime2->tTimeS )
      return( TRUE );

   // pMtime1->tTimeS == pMTime2->tTimeS...

   if( pMTime1->uTimeMS < pMTime2->uTimeMS )
      return( TRUE );

   return( FALSE );
}

/****************************************************************************/
/* WaitMTime() - Delays execution until the specified time occurs.          */
/****************************************************************************/

void WaitMTime( P_MILLITIME pMTime )
{
   MILLITIME stNow;
   long      iMS;

   CurrMTime( &stNow );

   iMS = (long)((((double)pMTime->tTimeS - (double)stNow.tTimeS) * 1000.0) + (double)pMTime->uTimeMS - (double)stNow.uTimeMS);

   if( iMS > 0 )
      Delay( (unsigned long)iMS );
}

