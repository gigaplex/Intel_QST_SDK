/****************************************************************************/
/*                                                                          */
/*  Module:         CritSect.c                                              */
/*                                                                          */
/*  Description:    Implements support for the use of critical sections in  */
/*                  the DOS environment.                                    */
/*                                                                          */
/*  Notes:      1.  Because  everything  is  single-threaded  in  the  DOS  */
/*                  environment,  we  don't  really need a synchronization  */
/*                  operator. For robustness, however, we need to  provide  */
/*                  support  for  validation  of usage and parameters. For  */
/*                  each critical section type  referenced,  we  create  a  */
/*                  descriptor  to  track  the  type.  The descriptors are  */
/*                  managed in a linked list.                               */
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

#if !defined(_DOS) && !defined(__DOS__) && !defined(MSDOS)
#error This source module intended for use in DOS environments only
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "CritSect.h"

/****************************************************************************/
/* Definitions for the descriptors that we use to track the active critical */
/* section types. We use a linked-list to track the descriptors             */
/****************************************************************************/

typedef struct _CRITSECT_DESCR
{
    U32                         uSignature;
    U32                         uSectionType;
    struct _CRITSECT_DESCR *    pstPrevDescr;
    struct _CRITSECT_DESCR *    pstNextDescr;

} CRITSECT_DESCR;

#define CRITSECT_SIGNATURE      'CRIT'                  // Signature for validating descriptors

static  CRITSECT_DESCR *        pstFirstDescr = NULL;   // Head of our linked list

/****************************************************************************/
/*  LookupCritSect() - Returns a handle for the operator that is  used  to  */
/*  implement  the  specified critical section type. If this operator does  */
/*  not exist, the function will return NULL.                               */
/*                                                                          */
/*  On DOS, we search our descriptor linked-list for the entry  containing  */
/*  the  specified  critical  section type. If we find an entry, we return  */
/*  the descriptor's address as the handle for the operator. If  we  can't  */
/*  find  an entry with the specified critical section type, we return the  */
/*  NULL handle and set errno to ENOENT.                                    */
/****************************************************************************/

HCRITSECT LookupCritSect( U32 uSectionType )
{
    CRITSECT_DESCR *pstDescr;

    for( pstDescr = pstFirstDescr; pstDescr; pstDescr = pstDescr->pstNextDescr )
        if( pstDescr->uSectionType == uSectionType )
            break;

    if( !pstDescr )
        errno = ENOENT;

    return( (HCRITSECT)pstDescr );
}

/****************************************************************************/
/*  CreateCritSect() - Returns a handle for the operator that is  used  to  */
/*  implement  the  specified  critical section type. The bMustCreate flag  */
/*  indicates how the function should handle existing operators. If set to  */
/*  TRUE, the function will fail (return NULL) if the  operator  (already)  */
/*  exists.  If  set  to  FALSE, the function will return a handle for the  */
/*  existing operator.                                                      */
/*                                                                          */
/*  On DOS, for critical section creation, we allocate  a  descriptor  and  */
/*  put the critical section type into it. We place this descriptor at the  */
/*  head  of  our linked list of descriptors and return its address as the  */
/*  handle for the operator. If we cannot create the  descriptor,  we  set  */
/*  errno  to  ENOMEM  and return the NULL handle. If the critical section  */
/*  type already exists and creation is not required, we return  a  handle  */
/*  for  the existing descriptor. If creation is required, we set errno to  */
/*  EEXIST and return the NULL handle.                                      */
/****************************************************************************/

HCRITSECT CreateCritSect( U32 uSectionType, BOOL bMustCreate )
{
    HCRITSECT hCritSect = LookupCritSect( uSectionType );

    if( hCritSect )
    {
        if( bMustCreate )
        {
            errno = EEXIST;
            hCritSect = NULL;
        }
    }
    else
    {
        CRITSECT_DESCR *pstDescr = (CRITSECT_DESCR *)malloc( sizeof(CRITSECT_DESCR) );

        if( pstDescr )
        {
            hCritSect = (HCRITSECT)pstDescr;

            // Initialize descriptor

            pstDescr->uSignature            = CRITSECT_SIGNATURE;
            pstDescr->uSectionType          = uSectionType;

            // Put it at the beginning of the list

            pstDescr->pstPrevDescr          = NULL;
            pstDescr->pstNextDescr          = pstFirstDescr;

            if( pstFirstDescr )
                pstFirstDescr->pstPrevDescr = pstDescr;

            pstFirstDescr                   = pstDescr;
        }
        else // malloc() failed
        {
            errno = ENOMEM;
            hCritSect = NULL;
        }
    }

    return( hCritSect );
}

/****************************************************************************/
/* ValidCritSect() - Determines whether a critical section handle is valid  */
/****************************************************************************/

static BOOL ValidCritSect( HCRITSECT hCritSect )
{
    if( hCritSect && (((CRITSECT_DESCR *)hCritSect)->uSignature == CRITSECT_SIGNATURE) )
        return( TRUE );

    return( FALSE );
}

/****************************************************************************/
/*  CloseCritSect() - Dereferences the handle for  the  operator  that  is  */
/*  used  to implement the specified critical section type. If this is the  */
/*  last handle that was referencing the operator, the  operator  will  be  */
/*  deleted.                                                                */
/*                                                                          */
/*  On DOS, closing the critical section means deleting the descriptor. We  */
/*  pull it out of our linked-list and then free its memory.                */
/****************************************************************************/

BOOL CloseCritSect( HCRITSECT hCritSect )
{
    if( !ValidCritSect( hCritSect ) )
    {
        errno = EINVAL;
        return( FALSE );
    }
    else
    {
        CRITSECT_DESCR *pstDescr = (CRITSECT_DESCR *)hCritSect;

        // Invalidate the descriptor so it won't match if used subsequently

        pstDescr->uSignature    = 0;
        pstDescr->uSectionType  = 0;

        // Remove segment from the list of segments

        if( pstDescr->pstPrevDescr )
            pstDescr->pstPrevDescr->pstNextDescr = pstDescr->pstNextDescr;
        else
            pstFirstDescr = pstDescr->pstNextDescr;

        if( pstDescr->pstNextDescr )
            pstDescr->pstNextDescr->pstPrevDescr = pstDescr->pstPrevDescr;

        // Get rid of the descriptor

        free( pstDescr );
        return( TRUE );
    }
}

/****************************************************************************/
/*  EnterCritSect() - Enters critical section referenced by the  specified  */
/*  handle.  If  another  thread  is already in the critical section, this  */
/*  thread will wait until it can  enter.  If  more  than  one  thread  is  */
/*  waiting  to  enter, this thread will be placed at the end of the queue  */
/*  of waiting threads ("back of the line, buddy").                         */
/*                                                                          */
/*  On DOS, since we're single-threaded and  don't  need  to  do  anything  */
/*  real, all we do is validate the handle passed.                          */
/****************************************************************************/

BOOL EnterCritSect( HCRITSECT hCritSect )
{
    return( ValidCritSect( hCritSect ) );
}

/****************************************************************************/
/*  LeaveCritSect() - Leaves critical section referenced by the  specified  */
/*  handle.  If  thread(s) are waiting to enter this critical section, the  */
/*  one that has been waiting the longest gets to enter.                    */
/*                                                                          */
/*  On DOS, since we're single-threaded and  don't  need  to  do  anything  */
/*  real, all we do is validate the handle passed.                          */
/****************************************************************************/

BOOL LeaveCritSect( HCRITSECT hCritSect )
{
    return( ValidCritSect( hCritSect ) );
}

