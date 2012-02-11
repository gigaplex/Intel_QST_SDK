/****************************************************************************/
/*                                                                          */
/*  Module:         GlobMem.c                                               */
/*                                                                          */
/*  Description:    Implements  support  for  the  use  of  global  memory  */
/*                  segments in the Linux environment.                      */
/*                                                                          */
/*  Notes:      1.  For Linux, we utilize the shm (shared memory) facility  */
/*                  to manage our global memory segments. The segment type  */
/*                  is  used  as  the  identifier  for  the  shared memory  */
/*                  segments (so make them unique!!).                       */
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

#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/time.h>

#include "GlobMem.h"

/****************************************************************************/
/*  LookupGlobMem() - Returns a handle for the global memory segment  with  */
/*  the  specified  segment  id.  If  no segment exists with this id, NULL  */
/*  is returned. Note: in some environments, both the segment id and  size  */
/*  are  used  in  the search; thus, the operation may fail if the size is  */
/*  incorrect.                                                              */
/*                                                                          */
/*  On Linux, we use shmget() to determine whether a global memory segment  */
/*  exists with the specified segment id. We add 1 to the  identifier  for  */
/*  this  segment  (since  0 is a valid identifier) and return this as the  */
/*  handle for our critical section. If shmget() fails, -1 is returned  as  */
/*  the  identifier.  Adding  1  to this value yields 0 (the NULL handle),  */
/*  which is perfect for our external needs...                              */
/****************************************************************************/

HGLOBMEM LookupGlobMem( U32 uSegmentId, size_t tSegmentSize )
{
    int iShmId = shmget( (key_t)uSegmentId, tSegmentSize, 0 );
    return( (HGLOBMEM)((long)(iShmId + 1)) );
}

/****************************************************************************/
/*  CreateGlobMem() - Returns a handle for the global memory segment  with  */
/*  the  specified segment id. The bMustCreate parameter indicates how the  */
/*  function should handle the segment  existing.  If  set  to  TRUE,  the  */
/*  function  will  fail (return NULL). If set to FALSE, the function will  */
/*  return a handle for the existing segment. If no segment exists, one is  */
/*  created. If the creation attempt fails, the function returns NULL.      */
/*                                                                          */
/*  On Linux, we use shmget() to attempt the creation of the global memory  */
/*  segment.  The  IPC_EXCL  flag  says  fail  the  request if the segment  */
/*  already exists. If the operation  is  successful,  we  add  1  to  the  */
/*  identifier for this segment (since 0 is a valid identifier) and return  */
/*  this as the handle for our critical section. If the creation operation  */
/*  fails  and  the  caller  has  not  said  we  must  create,  we  invoke  */
/*  LookupGlobMem() to attempt the lookup of an existing segment. In  this  */
/*  case,  we  let it establish the handle and exception code to return to  */
/*  the caller...                                                           */
/****************************************************************************/

HGLOBMEM CreateGlobMem( U32 uSegmentId, size_t tSegmentSize, BOOL bMustCreate )
{
    int  iShmId = shmget( (key_t)uSegmentId, tSegmentSize, IPC_CREAT | IPC_EXCL | 0666 );

    if( iShmId != -1 )
        return( (HGLOBMEM)((long)(iShmId + 1)) );
    else
        return( (bMustCreate)? NULL : LookupGlobMem( uSegmentId, tSegmentSize ) );
}

/****************************************************************************/
/*  CloseGlobMem() - Frees the specified global memory segment handle.  If  */
/*  this is the last handle to the global memory segment, the segment will  */
/*  be deleted. The function returns a boolean success indicator.           */
/*                                                                          */
/*  On Linux, we use shmctl() to validate the handle and to indicate  that  */
/*  the  segment  can  be  deleted  when  its  last  attachment is removed  */
/*  (unmapped). We do not remove the attachment for this process, however;  */
/*  we don't have the pointer with which to do so and  the  caller  should  */
/*  have already done this before calling us anyway.                        */
/****************************************************************************/

BOOL CloseGlobMem( HGLOBMEM hSegment )
{
    int iShmId = (int)(*((long*)&hSegment)) - 1;
    return( shmctl( iShmId, IPC_RMID, NULL ) != -1 );
}

/****************************************************************************/
/*  MapGlobMem() - Maps the  specified  global  memory  segment  into  the  */
/*  address  space of the calling process. If successful, a pointer to the  */
/*  base of the area assigned to the segment is returned. If anything goes  */
/*  wrong during the attempt, NULL is returned.                             */
/*                                                                          */
/*  On Linux, we use shmat() to map the segment...                          */
/****************************************************************************/

void *MapGlobMem( HGLOBMEM hSegment )
{
    int iShmId = (int)(*((long*)&hSegment)) - 1;
    return( (void *)shmat( iShmId, NULL, 0 ) );
}

/****************************************************************************/
/*  UnmapGlobMem() - Dereferences the specified mapping of a global memory  */
/*  segment. If this is the last reference  to  the  segment  within  this  */
/*  process,  the  segment  will  be  unmapped  from  the calling process'  */
/*  address space. The function returns a boolean success indicator.        */
/*                                                                          */
/*  On Linux, we use shmdt() to unmap the segment...                        */
/****************************************************************************/

BOOL UnmapGlobMem( void *pvSegment )
{
    return( shmdt( pvSegment ) != -1 );
}

