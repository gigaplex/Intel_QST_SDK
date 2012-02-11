/****************************************************************************/
/*                                                                          */
/*  Module:         SmartTemp.c                                             */
/*                                                                          */
/*  Description:    Implements support for obtaining temperature  readings  */
/*                  from  hard  drives  that  expose their temperature via  */
/*                  S.M.A.R.T. technology.                                  */
/*                                                                          */
/*  Note:       1.  This module relies on the libSMART DLL for  access  to  */
/*                  HDD  temperatures.  This  DLL  provides  the functions  */
/*                  necessary to identify  all  HDDs  that  support  Self-  */
/*                  Monitoring,    Analysis   and   Reporting   Technology  */
/*                  (S.M.A.R.T.)  and  expose  the   failure   prediction,  */
/*                  attribute/threshold,  temperature  and other data that  */
/*                  is provided via this technology.                        */
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
#include <stdarg.h>
#include <string.h>

#include <windows.h>

#include "QstDiskServ.h"
#include "SMART.h"

/****************************************************************************/
/* Variables                                                                */
/****************************************************************************/

static int      iDrives;                        // Number of HDs exposing temp
static int      iDriveIndex[SMART_MAX_DRIVES];  // Physical indexes for these HDs
static DWORD    dwInitError;                    // Failure code from Init

/****************************************************************************/
/* InitSmartTemp() - Initializes support for accessing temperature readings */
/* from Hard Drives that expose their temperature via S.M.A.R.T.            */
/****************************************************************************/

int InitSmartTemp( void )
{
    BOOL bCurr, bWorst;
    int  iDrive;

    dwInitError = NO_ERROR;

#ifdef DYNAMIC_DLL_LOADING

    if( !SMARTInitialize() )
    {
        dwInitError = GetLastError();
        return( iDrives = NO_RESULT_AVAIL );
    }

#endif

    for( iDrive = iDrives = 0; iDrive < SMART_MAX_DRIVES; iDrive++ )
    {
        if( SMARTGetDriveCapabilities( iDrive, &bCurr, &bWorst ) && bCurr )
            iDriveIndex[iDrives++] = iDrive;
    }

    return( iDrives );
}

/****************************************************************************/
/* DoneSmartTemp() - Cleans up the support for accessing temperature        */
/* readings from Hard Drives that expose their temperature via S.M.A.R.T.   */
/****************************************************************************/

void DoneSmartTemp( void )
{

#ifdef DYNAMIC_DLL_LOADING

    if( iDrives != NO_RESULT_AVAIL )
        SMARTCleanup();

#endif

}

/****************************************************************************/
/* GetSmartIndex() - Returns the physical index of the specified drive      */
/****************************************************************************/

int GetSmartIndex( int iIndex )
{
    if( iDrives == NO_RESULT_AVAIL )
        SetLastError( ( dwInitError )? dwInitError : ERROR_NOT_FOUND );
    else
        if( (iIndex < 0) || (iIndex >= iDrives) )
            SetLastError( ERROR_INVALID_PARAMETER );
        else
            return( iDriveIndex[iIndex] );

    return( NO_RESULT_AVAIL );
}

/****************************************************************************/
/* GetSmartModel() - Returns model string for the specified drive           */
/****************************************************************************/

BOOL GetSmartModel( int iIndex, char *pszModel )
{
    if( iDrives == NO_RESULT_AVAIL )
        SetLastError( ( dwInitError )? dwInitError : ERROR_NOT_FOUND );
    else
        if( (iIndex < 0) || (iIndex >= iDrives) )
            SetLastError( ERROR_INVALID_PARAMETER );
        else
            return( SMARTGetDriveInformation( iDriveIndex[iIndex], pszModel, NULL, NULL ) );

    return( FALSE );
}

/****************************************************************************/
/* GetSmartTemp() - Returns the current temperature for the specified drive */
/****************************************************************************/

int GetSmartTemp( int iIndex )
{
    int iTemp;

    if( iDrives == NO_RESULT_AVAIL )
        SetLastError( ( dwInitError )? dwInitError : ERROR_NOT_FOUND );
    else
        if( (iIndex < 0) || (iIndex >= iDrives) )
            SetLastError( ERROR_INVALID_PARAMETER );
        else
            if( SMARTGetDriveTemperatures( iDriveIndex[iIndex], &iTemp, NULL ) )
                return( iTemp );

    return( NO_TEMP_AVAIL );
}

