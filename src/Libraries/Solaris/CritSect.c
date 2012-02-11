/****************************************************************************/
/*                                                                          */
/*  Module:         CritSect.c                                              */
/*                                                                          */
/*  Description:    Implements support for the use of critical sections in  */
/*                  the Linux environment.                                  */
/*                                                                          */
/*  Notes:      1.  In the Linux  environment,  we  use  a  global  binary  */
/*                  semaphore (implemented using a counting semaphore with  */
/*                  a single unit) to implement the critical section.  The  */
/*                  critical  section type value is used as the identifier  */
/*                  for the semaphore (so make it unique!!).                */
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

#ifndef __sun__
#error This source module intended for use in Solaris environments only
#endif

#include <errno.h>

#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/time.h>

#include "CritSect.h"

/****************************************************************************/
/*  semun - Union used to manipulate Linux Semaphores. We have to  declare  */
/*  this ourselves if the Linux include files don't do so. It's silly that  */
/*  they so carefully document how to declare this union yet won't add the  */
/*  declaration to the include files...                                     */
/****************************************************************************/

#if defined(__GNU_LIBRARY__) && !defined(_SEM_SEMUN_UNDEFINED)
// union semun Was defined by including <sys/sem.h>
#else

union semun
{
    int                     val;            // value for SETVAL
    struct semid_ds         *buf;           // buffer for IPC_STAT, IPC_SET
    unsigned short int      *array;         // array for GETALL, SETALL
    struct seminfo          *__buf;         // buffer for IPC_INFO
};

#endif // def _SEM_SEMUN_UNDEFINED

/****************************************************************************/
/*  Declarations for our  semaphore  operation  specification  structures.  */
/*  Structure  stSemAdd requests the addition of a unit to a semaphore and  */
/*  structure stSemRem requests the removal of a unit  from  a  semaphore.  */
/*  Both  specify  SEM_UNDO  so  that  the semaphore is protected should a  */
/*  thread in ownership of the semaphore be deleted...                      */
/****************************************************************************/

static const struct sembuf stSemAdd = { 0,  1, SEM_UNDO };
static const struct sembuf stSemRem = { 0, -1, SEM_UNDO };

/****************************************************************************/
/*  LookupCritSect() - Returns a handle for the operator that is  used  to  */
/*  implement  the  specified critical section type. If this operator does  */
/*  not exist, the function will return NULL.                               */
/*                                                                          */
/*  On Linux, we attempt to look up the identifier for the semaphore using  */
/*  the specified critical section type.  We  add  1  to  this  identifier  */
/*  (since  0 is a valid identifier) and return this as the handle for our  */
/*  critical section. If the lookup fails, semget() returns -1.  Adding  1  */
/*  to  this  value  yields  0 (the NULL handle), which is perfect for our  */
/*  external needs...                                                       */
/****************************************************************************/

HCRITSECT LookupCritSect( U32 uSectionType )
{
    int iSemId = semget( (int)uSectionType, 1, 0 );
    return( (HCRITSECT)((long)(iSemId + 1)) );
}

/****************************************************************************/
/*  CreateCritSect() - Returns a handle for the operator that is  used  to  */
/*  implement  the  specified  critical section type. The bMustCreate flag  */
/*  indicates how the function should handle existing operators. If set to  */
/*  TRUE, the function will fail (return NULL) if the  operator  (already)  */
/*  exists.  If  set  to  FALSE, the function will return a handle for the  */
/*  existing operator.                                                      */
/*                                                                          */
/*  On Linux we create or lookup the appropriate semaphore using  function  */
/*  semget(),  depending upon the creation type requested. We add 1 to the  */
/*  semaphore's identifier (since 0 is a valid identifier) and return this  */
/*  as the handle for our critical section. If the lookup fails,  semget()  */
/*  returns  -1.  Adding 1 to this value yields 0 (the NULL Handle), which  */
/*  is perfect for our external (Handle) needs. We set errno to EEXIST  if  */
/*  the  semaphore  already existed and creation was requested. Otherwise,  */
/*  errno is set by the semaphore primities.                                */
/****************************************************************************/

HCRITSECT CreateCritSect( U32 uSectionType, BOOL bMustCreate )
{
    int iSemId = semget( (int)uSectionType, 1, IPC_CREAT | IPC_EXCL | 0666 );

    if( iSemId == -1 )
    {
        // if they didn't say must create, try looking up semaphore

        if( !bMustCreate )
            iSemId = semget( (int)uSectionType, 1, 0 );
    }
    else
    {
        // We created; initialize to have one unit (indicating available)

        union semun uSemun;
        uSemun.val = 1;

        if( semctl( iSemId, 0, SETVAL, uSemun ) == -1 )
        {
            int iSave = errno;
            semctl( iSemId, 0, IPC_RMID, 0 );
            errno = iSave;

            iSemId = -1;
        }
    }

    return( (HCRITSECT)((long)(iSemId + 1)) );
}

/****************************************************************************/
/*  CloseCritSect() - Dereferences the handle for  the  operator  that  is  */
/*  used  to implement the specified critical section type. If this is the  */
/*  last handle that was referencing the operator, the  operator  will  be  */
/*  deleted.                                                                */
/*                                                                          */
/*  On Linux, we have nothing to do.  When  all  applications  referencing  */
/*  the  semaphore  are  deleted,  the  semaphore  will  be  automatically  */
/*  deleted...                                                              */
/****************************************************************************/

BOOL CloseCritSect( HCRITSECT hCritSect )
{
    return( TRUE );
}

/****************************************************************************/
/*  EnterCritSect() - Enters critical section referenced by the  specified  */
/*  handle.  If  another  thread  is already in the critical section, this  */
/*  thread will wait until it can  enter.  If  more  than  one  thread  is  */
/*  waiting  to  enter, this thread will be placed at the end of the queue  */
/*  of waiting threads ("back of the line, buddy"). The  function  returns  */
/*  TRUE if successful; FALSE otherwise.                                    */
/*                                                                          */
/*  On Linux, we use function semop() to remove a unit from the semaphore.  */
/*  This  will  cause  a  wait  if no units are available. On failure, the  */
/*  errno setting comes from function semop().                              */
/****************************************************************************/

BOOL EnterCritSect( HCRITSECT hCritSect )
{
    int iSemId = (int)(*((long*)&hCritSect)) - 1;
    return( semop( iSemId, (struct sembuf *)&stSemRem, 1 ) != -1 );
}

/****************************************************************************/
/*  LeaveCritSect() - Leaves critical section referenced by the  specified  */
/*  handle.  If  thread(s) are waiting to enter this critical section, the  */
/*  one that has been waiting the longest  gets  to  enter.  The  function  */
/*  returns TRUE if successful; FALSE otherwise.                            */
/*                                                                          */
/*  On Linux, we use function semop() to add a unit to the  semaphore  for  */
/*  the  specified  critical section. This wakes up longest waiting thread  */
/*  (if any are, in fact, waiting for  a  unit).  On  failure,  the  errno  */
/*  setting comes from function semop().                                    */
/****************************************************************************/

BOOL LeaveCritSect( HCRITSECT hCritSect )
{
    int iSemId = (int)(*((long*)&hCritSect)) - 1;
    return( semop( iSemId, (struct sembuf *)&stSemAdd, 1 ) != -1 );
}

