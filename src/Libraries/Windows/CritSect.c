/****************************************************************************/
/*                                                                          */
/*  Module:         CritSect.c                                              */
/*                                                                          */
/*  Description:    Implements support for the use of critical sections in  */
/*                  the Windows environments.                               */
/*                                                                          */
/*  Notes:      1.  In the Windows environment, we use a global  semaphore  */
/*                  with a single unit to implement each critical section.  */
/*                  Since semaphores are identified by name,  we  use  the  */
/*                  section type values to build the unique strings we use  */
/*                  for the semaphores.                                     */
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

#if !defined(_WIN32) && !defined(__WIN32__)
#error This source module intended for use in Windows environments only
#endif

#include <stdio.h>
#include <tchar.h>
#include <windows.h>

#include "CritSect.h"

/****************************************************************************/
/*  LookupCritSect() - Returns a handle for the operator that is  used  to  */
/*  implement  the  specified critical section type. If this operator does  */
/*  not exist, the function will return NULL.                               */
/*                                                                          */
/*  On Windows, we use OpenSemaphore() to search for existing  semaphores.  */
/*  This  function  returns  the  handle for existing semaphores (which we  */
/*  cast to be our critical section handles). If no semaphore exists  with  */
/*  the  specified (section type-based) name, OpenSemaphore() returns NULL  */
/*  and sets the Last-Error variable for us...                              */
/****************************************************************************/

HCRITSECT LookupCritSect( U32 uSectionType )
{
    TCHAR tszName[32];

    _stprintf( tszName, _T("Global\\QST_SEM_%08X"), uSectionType );
    return( (HCRITSECT)OpenSemaphore( SEMAPHORE_ALL_ACCESS, FALSE, tszName ) );
}

/****************************************************************************/
/*  CreateCritSect() - Returns a handle for the operator that is  used  to  */
/*  implement  the  specified  critical section type. The bMustCreate flag  */
/*  indicates how the function should handle existing operators. If set to  */
/*  TRUE, the function will fail (return NULL) if the  operator  (already)  */
/*  exists.  If  set  to  FALSE, the function will return a handle for the  */
/*  existing operator.                                                      */
/*                                                                          */
/*  On  Windows,  we  use  CreateSemaphore()  to  create  or  lookup   the  */
/*  appropriate  semaphore, depending upon the creation type requested. If  */
/*  we created or looked it up (and lookup is acceptable), we  return  its  */
/*  handle  as  our Critical Section handle. Otherwise, the NULL handle is  */
/*  returned. In this case, the Last-Error variable is  set  appropriately  */
/*  by function CreateSemaphore().                                          */
/****************************************************************************/

HCRITSECT CreateCritSect( U32 uSectionType, BOOL bMustCreate )
{
    TCHAR   tszName[32];
    HANDLE  hSem;

    // Attempt semaphore creation

    _stprintf( tszName, _T("Global\\QST_SEM_%08X"), uSectionType );
    hSem = CreateSemaphore( NULL, 1, 1, tszName );

    if( hSem )
    {
        if( GetLastError() == ERROR_ALREADY_EXISTS )
        {
            // Already existed; reject if user wanted us to create it

            if( bMustCreate )
            {
                CloseHandle( hSem );
                hSem = NULL;
            }
        }
        else
        {
            // We created; need to grant others access to it

            SECURITY_DESCRIPTOR stSecDescr;
            InitializeSecurityDescriptor( &stSecDescr, SECURITY_DESCRIPTOR_REVISION );
            SetSecurityDescriptorDacl( &stSecDescr, TRUE, NULL, FALSE );
            SetKernelObjectSecurity( hSem, DACL_SECURITY_INFORMATION, &stSecDescr );
        }
    }

    return( (HCRITSECT)hSem );
}

/****************************************************************************/
/*  CloseCritSect() - Dereferences the handle for  the  operator  that  is  */
/*  used  to implement the specified critical section type. If this is the  */
/*  last handle that was referencing the operator, the  operator  will  be  */
/*  deleted.                                                                */
/*                                                                          */
/*  On Windows, we simply close the semaphore handle.  If this is the last  */
/*  open handle for the semaphore, the semaphore will also be deleted.      */
/****************************************************************************/

BOOL CloseCritSect( HCRITSECT hCritSect )
{
    return( CloseHandle( (HANDLE)hCritSect ) );
}

/****************************************************************************/
/*  EnterCritSect() - Enters critical section referenced by the  specified  */
/*  handle.  If  another  thread  is already in the critical section, this  */
/*  thread will wait until it can  enter.  If  more  than  one  thread  is  */
/*  waiting  to  enter, this thread will be placed at the end of the queue  */
/*  of waiting threads ("back of the line, buddy").                         */
/*                                                                          */
/*  On Windows, we use WaitForSingleObject() to implement a wait  until  a  */
/*  unit can be removed from the semaphore. On failure, this function sets  */
/*  up the Last-Error variable for us.                                      */
/****************************************************************************/

BOOL EnterCritSect( HCRITSECT hCritSect )
{
    switch( WaitForSingleObject( (HANDLE)hCritSect, INFINITE ) )
    {
    case WAIT_ABANDONED:
    case WAIT_OBJECT_0:

        return( TRUE );

    case WAIT_TIMEOUT:
    case WAIT_FAILED:
    default:

        return( FALSE );
    }
}

/****************************************************************************/
/*  LeaveCritSect() - Leaves critical section referenced by the  specified  */
/*  handle.  If  thread(s) are waiting to enter this critical section, the  */
/*  one that has been waiting the longest gets to enter.                    */
/*                                                                          */
/*  On Windows, we use ReleaseSemaphore() to add a unit to the  semaphore.  */
/*  On failure, thise function sets up the Last-Error variable for us.      */
/****************************************************************************/

BOOL LeaveCritSect( HCRITSECT hCritSect )
{
    return( ReleaseSemaphore( (HANDLE)hCritSect, 1, NULL ) );
}

