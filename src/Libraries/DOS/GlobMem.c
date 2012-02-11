/****************************************************************************/
/*                                                                          */
/*  Module:         GlobMem.h                                               */
/*                                                                          */
/*  Description:    Implements  support  for  the  use  of  global  memory  */
/*                  segments in the DOS environment.                        */
/*                                                                          */
/*  Notes:      1.  On DOS, we use simple memory allocation to fulfill all  */
/*                  requests. We  create  segments  with  some  additional  */
/*                  space  that  we  use to maintain information about the  */
/*                  segment. This includes linking them into a list, so we  */
/*                  can support lookup as well as creation.                 */
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

#include "GlobMem.h"

/****************************************************************************/
/* Definitions for the descriptors that we use to track the active global   */
/* memory segments. These descriptors preceed the user-accessible portion   */
/* of the segment in memory. We use a linked-list to track active segments  */
/****************************************************************************/

typedef struct _GLOBMEM_DESCR
{
    U32                         uSignature;
    U32                         uSegmentId;
    size_t                      tSegmentSize;           // size of user-accessible portion
    U32                         uReferences;
    U32                         uMappings;
    struct _GLOBMEM_DESCR *     pstPrevDescr;
    struct _GLOBMEM_DESCR *     pstNextDescr;


} GLOBMEM_DESCR;

#define GLOBMEM_DESCR_SIZE      32                      // Round it up for safety/alignment
#define GLOBMEM_SIGNATURE       'GLOB'                  // Signature for validating descriptors

static  GLOBMEM_DESCR *         pstFirstDescr = NULL;   // Head of our linked list

/****************************************************************************/
/*  LookupGlobMem() - Returns a handle for the global memory segment  with  */
/*  the  specified  segment  id.  If  no segment exists with this id, NULL  */
/*  is returned. Note: in some environments, both the segment id and  size  */
/*  are  used  in  the search; thus, the operation may fail if the size is  */
/*  incorrect.                                                              */
/*                                                                          */
/*  On DOS, we search our linked-list for a segment with the specified id.  */
/*  If  we  find an entry, we use the address of the descriptor portion of  */
/*  the segment as our handle. If we can't find a segment in our list with  */
/*  the specified type, we return the NULL handle and set errno to ENOENT.  */
/****************************************************************************/

HGLOBMEM LookupGlobMem( U32 uSegmentId, size_t tSegmentSize )
{
    GLOBMEM_DESCR *pstDescr;

    for( pstDescr = pstFirstDescr; pstDescr; pstDescr = pstDescr->pstNextDescr )
        if( (pstDescr->uSegmentId == uSegmentId) && (pstDescr->tSegmentSize == tSegmentSize) )
            break;

    if( pstDescr )
    {
        ++pstDescr->uReferences;
        return( pstDescr );
    }
    else
    {
        errno = ENOENT;
        return( NULL );
    }
}

/****************************************************************************/
/*  CreateGlobMem() - Returns a handle for the global memory segment  with  */
/*  the  specified segment id. The bMustCreate parameter indicates how the  */
/*  function should handle the segment  existing.  If  set  to  TRUE,  the  */
/*  function  will  fail (return NULL). If set to FALSE, the function will  */
/*  return a handle for the existing segment. If no segment exists, one is  */
/*  created. If the creation attempt fails, the function returns NULL.      */
/*                                                                          */
/*  On DOS, we first attempt to see whether there is an  existing  segment  */
/*  of the specified type. If there is, we process against bMustCreate. If  */
/*  creation  is  required,  we set errno to EEXIST and return NULL. If we  */
/*  have to create the segment, we allocate one that has  room  to  locate  */
/*  our  descriptor  (ahead  of  the user-accessible portion). We add this  */
/*  segment to the head our linked-list and  return  the  address  of  the  */
/*  descriptor  as  the  handle  for  the segment. If we cannot create the  */
/*  descriptor, we set errno to ENOMEM and return NULL.                     */
/****************************************************************************/

HGLOBMEM CreateGlobMem( U32 uSegmentId, size_t tSegmentSize, BOOL bMustCreate )
{
    GLOBMEM_DESCR *pstDescr = (GLOBMEM_DESCR *)LookupGlobMem( uSegmentId, tSegmentSize );

    if( pstDescr )
    {
        // Set errno if we are rejecting because segment already exists

        if( bMustCreate )
        {
            errno = EEXIST;
            --pstDescr->uReferences;                    // undo side-effect of calling LookupGlobMem()
            pstDescr = (GLOBMEM_DESCR *)NULL;
        }
    }
    else
    {
        // Doesn't exist; we need to create...

        pstDescr = (GLOBMEM_DESCR *)malloc( tSegmentSize + GLOBMEM_DESCR_SIZE );

        if( pstDescr )
        {
            // Initialize descriptor

            pstDescr->uSignature            = GLOBMEM_SIGNATURE;
            pstDescr->uSegmentId            = uSegmentId;
            pstDescr->tSegmentSize          = tSegmentSize;

            pstDescr->uReferences           = 1;    // first reference
            pstDescr->uMappings             = 0;    // no mappings

            // Put it at the beginning of the list

            pstDescr->pstPrevDescr          = (HGLOBMEM)NULL;
            pstDescr->pstNextDescr          = pstFirstDescr;

            if( pstFirstDescr )
                pstFirstDescr->pstPrevDescr = pstDescr;

            pstFirstDescr                   = pstDescr;
        }
    }

    return( (HGLOBMEM)pstDescr );
}

/****************************************************************************/
/*  CloseGlobMem() - Frees the specified global memory segment handle.  If  */
/*  this is the last handle to the global memory segment, the segment will  */
/*  be deleted. The function returns a boolean success indicator.           */
/*                                                                          */
/*  On DOS, after validating the handle, we decrement the reference  count  */
/*  for  the segment. If there are then no references and no mappings, the  */
/*  program is done with the segment and we can get rid of it. This  means  */
/*  invalidating  the descriptor (to avoid accidental reuse), unlinking it  */
/*  from our list and then freeing the memory used.                         */
/****************************************************************************/

BOOL CloseGlobMem( HGLOBMEM hSegment )
{
    GLOBMEM_DESCR *pstDescr = (GLOBMEM_DESCR *)hSegment;

    if( pstDescr && (pstDescr->uSignature == GLOBMEM_SIGNATURE) )
    {
        // Dereference segment and, if no more references or mappings, delete it

        if( (--pstDescr->uReferences == 0) && (pstDescr->uMappings == 0) )
        {
            // Invalidate the descriptor so it won't match if used subsequently

            pstDescr->uSignature    = 0;
            pstDescr->uSegmentId    = 0;
            pstDescr->tSegmentSize  = 0;

            // unlink the segment

            if( pstDescr->pstPrevDescr )
                pstDescr->pstPrevDescr->pstNextDescr = pstDescr->pstNextDescr;
            else
                pstFirstDescr = pstDescr->pstNextDescr;

            if( pstDescr->pstNextDescr )
                pstDescr->pstNextDescr->pstPrevDescr = pstDescr->pstPrevDescr;

            // Get rid of the descriptor

            free( pstDescr );
        }

        return( TRUE );
    }

    errno = EINVAL;
    return( FALSE );
}

/****************************************************************************/
/*  MapGlobMem() - Maps the  specified  global  memory  segment  into  the  */
/*  address  space of the calling process. If successful, a pointer to the  */
/*  base of the area assigned to the segment is returned. If anything goes  */
/*  wrong during the attempt, NULL is returned.                             */
/*                                                                          */
/*  On DOS, after validating the handle, we increment the  mappings  count  */
/*  for  the  segment.  We  then  build  a  pointer to the user-accessible  */
/*  portion of the memory and return it to the caller.                      */
/****************************************************************************/

void *MapGlobMem( HGLOBMEM hSegment )
{
    GLOBMEM_DESCR *pstDescr = (GLOBMEM_DESCR *)hSegment;

    if( pstDescr && (pstDescr->uSignature == GLOBMEM_SIGNATURE) )
    {
        ++pstDescr->uMappings;
        return( ((char *)pstDescr + GLOBMEM_DESCR_SIZE) );
    }

    errno = EINVAL;
    return( FALSE );
}

/****************************************************************************/
/*  UnmapGlobMem() - Dereferences the specified mapping of a global memory  */
/*  segment. If this is the last reference  to  the  segment  within  this  */
/*  process,  the  segment  will  be  unmapped  from  the calling process'  */
/*  address space. The function returns a boolean success indicator.        */
/*                                                                          */
/*  On DOS, after validating the handle, we decrement the  mappings  count  */
/*  for  the segment. If there are then no mappings and no references, the  */
/*  program is done with the segment and we can get rid of it. This  means  */
/*  invalidating  the descriptor (to avoid accidental reuse), unlinking it  */
/*  from our list and then freeing the memory used.                         */
/****************************************************************************/

BOOL UnmapGlobMem( void *pvSegment )
{
    if( pvSegment )
    {
        GLOBMEM_DESCR *pstDescr = (GLOBMEM_DESCR *)((char *)pvSegment - GLOBMEM_DESCR_SIZE);

        if( pstDescr->uSignature == GLOBMEM_SIGNATURE )
        {
            // Remove the mapping and, if no references or mappings, delete segment
            if( (--pstDescr->uMappings == 0) && (pstDescr->uReferences == 0) )
            {
                // Invalidate the descriptor so it won't match if used subsequently

                pstDescr->uSignature    = 0;
                pstDescr->uSegmentId    = 0;
                pstDescr->tSegmentSize  = 0;

                // unlink the segment

                if( pstDescr->pstPrevDescr )
                    pstDescr->pstPrevDescr->pstNextDescr = pstDescr->pstNextDescr;
                else
                    pstFirstDescr = pstDescr->pstNextDescr;

                if( pstDescr->pstNextDescr )
                    pstDescr->pstNextDescr->pstPrevDescr = pstDescr->pstPrevDescr;

                // Get rid of the descriptor

                free( pstDescr );
            }

            return( TRUE );
        }
    }

    errno = EINVAL;
    return( FALSE );
}

