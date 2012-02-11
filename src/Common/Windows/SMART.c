/****************************************************************************/
/*                                                                          */
/*  Module:         SMART.c                                                 */
/*                                                                          */
/*  Description:    Provides the functions  necessary  for  utilizing  the  */
/*                  services  of  the SMART DLL. This DLL provides support  */
/*                  for accessing Hard Drive S.M.A.R.T. failure prediction  */
/*                  and temperature data.                                   */
/*                                                                          */
/*  Notes:      1.  This is a  helper  module,  designed  to  support  the  */
/*                  dynamic  linking of programs with the SMART DLL. It is  */
/*                  NOT needed for programs that statically link the DLL.   */
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
#include <string.h>

#include <windows.h>

#include "SMART.h"

/****************************************************************************/
/* Variables for DLL handling.                                              */
/****************************************************************************/

static  HMODULE                             hSmartDLL;

static  PFN_SMART_GET_DRIVE_CAPABILITIES    pfSMARTGetDriveCapabilities;
static  PFN_SMART_GET_DRIVE_PREDICTION      pfSMARTGetDrivePrediction;
static  PFN_SMART_GET_DRIVE_TEMPERATURES    pfSMARTGetDriveTemperatures;
static  PFN_SMART_GET_DRIVE_INFORMATION     pfSMARTGetDriveInformation;
static  PFN_SMART_GET_DRIVE_INDEX           pfSMARTGetDriveIndex;
static  PFN_SMART_DISPLAY_DRIVE_ATTRIBUTES  pfSMARTDisplayDriveAttributes;

/****************************************************************************/
/* SMARTInitialize() - Loads and initializes I/F with the SMART DLL.        */
/****************************************************************************/

BOOL SMARTInitialize( void )
{
    // Load the DLL

    hSmartDLL = LoadLibrary( SMART_DLL_NAME );

    if( hSmartDLL )
    {
        // Now build pointers to the DLL's functions

        pfSMARTGetDriveCapabilities   = (PFN_SMART_GET_DRIVE_CAPABILITIES)GetProcAddress( hSmartDLL, MAKEINTRESOURCE( SMART_ORD_GET_DRIVE_CAPABILITIES ) );
        pfSMARTGetDrivePrediction     = (PFN_SMART_GET_DRIVE_PREDICTION)GetProcAddress( hSmartDLL, MAKEINTRESOURCE( SMART_ORD_GET_DRIVE_PREDICTION ) );
        pfSMARTGetDriveTemperatures   = (PFN_SMART_GET_DRIVE_TEMPERATURES)GetProcAddress( hSmartDLL, MAKEINTRESOURCE( SMART_ORD_GET_DRIVE_TEMPERATURES ) );
        pfSMARTGetDriveInformation    = (PFN_SMART_GET_DRIVE_INFORMATION)GetProcAddress( hSmartDLL, MAKEINTRESOURCE( SMART_ORD_GET_DRIVE_INFORMATION ) );
        pfSMARTGetDriveIndex          = (PFN_SMART_GET_DRIVE_INDEX)GetProcAddress( hSmartDLL, MAKEINTRESOURCE( SMART_ORD_GET_DRIVE_INDEX ) );
        pfSMARTDisplayDriveAttributes = (PFN_SMART_DISPLAY_DRIVE_ATTRIBUTES)GetProcAddress( hSmartDLL, MAKEINTRESOURCE( SMART_ORD_DISPLAY_DRIVE_ATTRIBUTES ) );

        if(    pfSMARTGetDriveCapabilities
            && pfSMARTGetDrivePrediction
            && pfSMARTGetDriveTemperatures
            && pfSMARTGetDriveInformation
            && pfSMARTGetDriveIndex
            && pfSMARTDisplayDriveAttributes )
            return( TRUE );

        FreeLibrary( hSmartDLL );
        hSmartDLL = NULL;
    }

    return( FALSE );
}

/****************************************************************************/
/* SMARTCleanup() - Unloads the SMART DLL.                                  */
/****************************************************************************/

void SMARTCleanup( void )
{
    FreeLibrary( hSmartDLL );

    hSmartDLL                     = NULL;
    pfSMARTGetDriveCapabilities   = NULL;
    pfSMARTGetDrivePrediction     = NULL;
    pfSMARTGetDriveTemperatures   = NULL;
    pfSMARTGetDriveInformation    = NULL;
    pfSMARTGetDriveIndex          = NULL;
    pfSMARTDisplayDriveAttributes = NULL;
}

/****************************************************************************/
/* SMARTGetDriveCapabilities() - Gets a drive's S.M.A.R.T. Capabilities     */
/****************************************************************************/

BOOL APIENTRY SMARTGetDriveCapabilities( int iDrive, BOOL *pbCurrentTemp, BOOL *pbWorstTemp )
{
    return( pfSMARTGetDriveCapabilities( iDrive, pbCurrentTemp, pbWorstTemp ) );
}

/****************************************************************************/
/* SMARTGetDrivePrediction() - Gets a drive's Failure Prediction            */
/****************************************************************************/

BOOL APIENTRY SMARTGetDrivePrediction( int iDrive, BOOL *pbFailurePedicted )
{
    return( pfSMARTGetDrivePrediction( iDrive, pbFailurePedicted ) );
}

/****************************************************************************/
/* SMARTGetDriveTemperatures() - Gets a drive's current and/or worst-case   */
/* temperature. Specifying NULL for either destination pointer indicates    */
/* not interested in that particular temperature value. If the return of    */
/* the value(s) requested is not supported by the specified drive, the      */
/* function will return FALSE and set the last-error variable to            */
/* ERROR_NOT_SUPPORTED. Any other errors will also result in FALSE being    */
/* returned and the last-error variable set to an appropriate error code.   */
/****************************************************************************/

BOOL APIENTRY SMARTGetDriveTemperatures( int iDrive, int *piCurrentTemp, int *piWorstTemp )
{
    return( pfSMARTGetDriveTemperatures( iDrive, piCurrentTemp, piWorstTemp ) );
}

/****************************************************************************/
/* SMARTGetDriveInformation() - Gets information about the hard drive. This */
/* can include Model Number string, Serial Number string and/or size of the */
/* drive (in gigabytes). Specifying NULL for an item's destination pointer  */
/* indicates not interested in that particular item.                        */
/****************************************************************************/

BOOL APIENTRY SMARTGetDriveInformation( int iDrive, char *pszModel, char *pszSerial, int *piSize )
{
    return( pfSMARTGetDriveInformation( iDrive, pszModel, pszSerial, piSize ) );
}

/****************************************************************************/
/* SMARTGetDriveIndex() - Gets the physical index of the specified drive.   */
/****************************************************************************/

BOOL APIENTRY SMARTGetDriveIndex( int iDrive, int *piIndex )
{
    return( pfSMARTGetDriveIndex( iDrive, piIndex ) );
}

/****************************************************************************/
/* SMARTDisplayDriveAttributes - Displays a drive's S.M.A.R.T. Attributes.  */
/* NOTE: This routine is provided for debug purposes only. As stated, in    */
/* the S.M.A.R.T. Specification, APPLICATIONS SHOULD NOT INTERPRET, OR      */
/* PRESENT TO USERS, THE ATTRIBUTES OR THRESHOLD DATA VALUES.               */
/****************************************************************************/

BOOL APIENTRY SMARTDisplayDriveAttributes( int iDrive, void (APIENTRY *PutStrCallBack)( char *szStr ) )
{
    return( pfSMARTDisplayDriveAttributes( iDrive, PutStrCallBack ) );
}

