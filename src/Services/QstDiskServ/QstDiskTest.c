/****************************************************************************/
/*                                                                          */
/*  Module:         DiskTest.c                                              */
/*                                                                          */
/*  Description:    Mainline for a program that tests for the presence  of  */
/*                  hard  drives  that  both support S.M.A.R.T. and expose  */
/*                  temperature readings. It sets the errorlevel  variable  */
/*                  to the number of drives supporting these capabilites.   */
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

#include "CompVer.h"
#include "SMART.h"

int main( int iArgs, char *pszArg[] )
{
    int  iDrive, iDrives, iCurr, iWorst, iSize;
    BOOL bCurr, bWorst;
    char szModel[SMART_MAX_MODEL+1], szSerial[SMART_MAX_SERIAL+1], szWorst[20];

    puts( "\nIntel(R) Quiet System Technology Hard Disk Verification Utility v" COMP_VERSION_STR );
    puts( "Copyright (C) 2007-2009, Intel Corporation. All Rights Reserved.\n" );

#ifdef DYNAMIC_DLL_LOADING

    if( !SMARTInitialize() )
    {
        printf( "*** Unable to initialize drive interface, ccode = 0x%08X\n\n", GetLastError() );
        return( 0 );
    }

#endif

    for( iDrive = iDrives = 0; iDrive < SMART_MAX_DRIVES; iDrive++ )
    {
        if( SMARTGetDriveCapabilities( iDrive, &bCurr, &bWorst ) && bCurr )
        {
            if(    !SMARTGetDriveInformation( iDrive, szModel, szSerial, &iSize )
                || !SMARTGetDriveTemperatures( iDrive, &iCurr, (bWorst)? &iWorst : NULL ) )
            {
                printf( "*** Error getting Drive Information, ccode = 0x%08X\n\n", GetLastError() );
                return( 0 );
            }

            if( bWorst )
                sprintf( szWorst, "%dc", iWorst );
            else
                strcpy( szWorst, "N/A" );

            printf( "\nCompliant Drive Detected: %s\n", szModel );
            printf(   "  Serial Number:          %s\n", szSerial );
            printf(   "  Capacity:               %dGB\n", iSize );
            printf(   "  Current Temperature:    %dc\n", iCurr );
            printf(   "  Worst-Case Temperature: %s\n", szWorst );

            ++iDrives;
        }
    }

    putchar( '\n' );

#ifdef DYNAMIC_DLL_LOADING
    SMARTCleanup();
#endif

    return( iDrives );

#if _MSC_VER > 1000
   UNREFERENCED_PARAMETER( iArgs );
   UNREFERENCED_PARAMETER( pszArg );
#endif

}

