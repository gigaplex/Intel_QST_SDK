/****************************************************************************/
/*                                                                          */
/*  Module:         GlobMem.h                                               */
/*                                                                          */
/*  Description:    Provides function prototypes for using the module that  */
/*                  implements support for using global memory segments.    */
/*                                                                          */
/*  Notes:      1.  We use segment id values to uniquely  identify  global  */
/*                  memory segments.                                        */
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

#ifndef _GLOBMEM_H
#define _GLOBMEM_H

#include "typedef.h"

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(1)

/****************************************************************************/
/* Definitions                                                              */
/****************************************************************************/

#ifndef HGLOBMEM_DEFINED
#define HGLOBMEM_DEFINED
typedef void * HGLOBMEM;
#endif

/****************************************************************************/
/*  LookupGlobMem() - Returns a handle for the global memory segment  with  */
/*  the  specified  segment  id.  If  no segment exists with this id, NULL  */
/*  is returned. Note: in some environments, both the segment id and  size  */
/*  are  used  in  the search; thus, the operation may fail if the size is  */
/*  incorrect.                                                              */
/****************************************************************************/

HGLOBMEM LookupGlobMem( U32 uSegmentId, size_t tSegmentSize );

/****************************************************************************/
/*  CreateGlobMem() - Returns a handle for the global memory segment  with  */
/*  the  specified segment id. The bMustCreate parameter indicates how the  */
/*  function should handle the segment  existing.  If  set  to  TRUE,  the  */
/*  function  will  fail (return NULL). If set to FALSE, the function will  */
/*  return a handle for the existing segment. If no segment exists, one is  */
/*  created. If the creation attempt fails, the function returns NULL.      */
/****************************************************************************/

HGLOBMEM CreateGlobMem( U32 uSegmentId, size_t tSegmentSize, BOOL bMustCreate );

/****************************************************************************/
/*  CloseGlobMem() - Frees the specified global memory segment handle.  If  */
/*  this is the last handle to the global memory segment, the segment will  */
/*  be deleted. The function returns a boolean success indicator.           */
/****************************************************************************/

BOOL CloseGlobMem( HGLOBMEM hSegment );

/****************************************************************************/
/*  MapGlobMem() - Maps the  specified  global  memory  segment  into  the  */
/*  address  space of the calling process. If successful, a pointer to the  */
/*  base of the area assigned to the segment is returned. If anything goes  */
/*  wrong during the attempt, NULL is returned.                             */
/****************************************************************************/

void *MapGlobMem( HGLOBMEM hSegment );

/****************************************************************************/
/*  UnmapGlobMem() - Dereferences the specified mapping of a global memory  */
/*  segment. If this is the last reference  to  the  segment  within  this  */
/*  process,  the  segment  will  be  unmapped  from  the calling process'  */
/*  address space. The function returns a boolean success indicator.        */
/****************************************************************************/

BOOL UnmapGlobMem( void *pvSegment );

#pragma pack()

#ifdef __cplusplus
}
#endif

#endif // ndef _GLOBMEM_H

