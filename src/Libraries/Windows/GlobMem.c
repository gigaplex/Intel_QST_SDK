/****************************************************************************/
/*                                                                          */
/*  Module:         GlobMem.c                                               */
/*                                                                          */
/*  Description:    Implements  support  for  the  use  of  global  memory  */
/*                  segments in the Windows environment.                    */
/*                                                                          */
/*  Notes:      1.  For Windows, we utilize the file mapping  facility  to  */
/*                  create  and  manage  our global memory segments. Since  */
/*                  file mappings are identified by name, we  use  segment  */
/*                  id  values  to build the unique string identifiers for  */
/*                  the shared memory segments.                             */
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

#include "GlobMem.h"

/****************************************************************************/
/*  LookupGlobMem() - Returns a handle for the global memory segment  with  */
/*  the  specified  segment  id.  If  no segment exists with this id, NULL  */
/*  is returned.                                                            */
/*                                                                          */
/*  On Windows, we use OpenFileMapping() to find existing  file  mappings.  */
/*  This  function returns the handle for the existing file mapping (which  */
/*  we cast to be our global memory segment handle). If  no  file  mapping  */
/*  exists  with  the specified (segment id-based) name, OpenFileMapping()  */
/*  returns NULL and sets the Last-Error variable for us...                 */
/****************************************************************************/

HGLOBMEM LookupGlobMem( U32 uSegmentId, size_t tSegmentSize )
{
    TCHAR tszName[32];

    _stprintf( tszName, _T("Global\\QST_MEM_%08X"), uSegmentId );

    return( (HGLOBMEM)OpenFileMapping( FILE_MAP_ALL_ACCESS, FALSE, tszName ) );

    UNREFERENCED_PARAMETER( tSegmentSize );
}

/****************************************************************************/
/*  CreateGlobMem() - Returns a handle for the global memory segment  with  */
/*  the  specified segment id. The bMustCreate parameter indicates how the  */
/*  function should handle the segment  existing.  If  set  to  TRUE,  the  */
/*  function  will  fail (return NULL). If set to FALSE, the function will  */
/*  return a handle for the existing segment. If no segment exists, one is  */
/*  created. If the creation attempt fails, the function returns NULL.      */
/*                                                                          */
/*  On Windows, we use CreateFileMapping() to create a new file mapping or  */
/*  obtain  a  handle  for  the existing one. The Last-Error variable will  */
/*  tell us whether a mapping was created or not. Finally, if  we  created  */
/*  the  segment,  we  must  set  its  security  descriptor to allow other  */
/*  processes to access it...                                               */
/****************************************************************************/

HGLOBMEM CreateGlobMem( U32 uSegmentId, size_t tSegmentSize, BOOL bMustCreate )
{
    TCHAR   tszName[32];
    HANDLE  hSharedSeg;

    // Attempt creation of segment

    _stprintf( tszName, _T("Global\\QST_MEM_%08X"), uSegmentId );

    hSharedSeg = CreateFileMapping( INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, tSegmentSize, tszName );

    if( hSharedSeg )
    {
        // If segment already exists, fail the operation if bMustCreate is set

        if( GetLastError() == ERROR_ALREADY_EXISTS )
        {
            if( bMustCreate )
            {
                CloseHandle( hSharedSeg );
                hSharedSeg = NULL;
            }
        }

        // If we created the segment; give the masses access to it

        else
        {
            SECURITY_DESCRIPTOR stSecDescr;

            InitializeSecurityDescriptor( &stSecDescr, SECURITY_DESCRIPTOR_REVISION );
            SetSecurityDescriptorDacl( &stSecDescr, TRUE, NULL, FALSE );
            SetKernelObjectSecurity( hSharedSeg, DACL_SECURITY_INFORMATION, &stSecDescr );
        }
    }

    return( (HGLOBMEM)hSharedSeg );
}

/****************************************************************************/
/*  CloseGlobMem() - Frees the specified global memory segment handle.  If  */
/*  this is the last handle to the global memory segment, the segment will  */
/*  be deleted. The function returns a boolean success indicator.           */
/*                                                                          */
/*  On Windows, we invoke CloseHandle to free up the handle...              */
/****************************************************************************/

BOOL CloseGlobMem( HGLOBMEM hSegment )
{
    return( CloseHandle( (HANDLE)hSegment ) );
}

/****************************************************************************/
/*  MapGlobMem() - Maps the  specified  global  memory  segment  into  the  */
/*  address  space of the calling process. If successful, a pointer to the  */
/*  base of the area assigned to the segment is returned. If anything goes  */
/*  wrong during the attempt, NULL is returned.                             */
/*                                                                          */
/*  On Windows, we use MapViewOfFile() to map the segment...                */
/****************************************************************************/

void *MapGlobMem( HGLOBMEM hSegment )
{
    return( MapViewOfFile( hSegment, FILE_MAP_WRITE, 0, 0, 0 ) );
}

/****************************************************************************/
/*  UnmapGlobMem() - Dereferences the specified mapping of a global memory  */
/*  segment. If this is the last reference  to  the  segment  within  this  */
/*  process,  the  segment  will  be  unmapped  from  the calling process'  */
/*  address space. The function returns a boolean success indicator.        */
/*                                                                          */
/*  On Windows, we use UnmapViewOfFile() to unmap the segment...            */
/****************************************************************************/

BOOL UnmapGlobMem( void *pvSegment )
{
    return( UnmapViewOfFile( pvSegment ) );
}

