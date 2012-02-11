/****************************************************************************/
/*                                                                          */
/*  Module:         RegAccess.c                                             */
/*                                                                          */
/*  Description:    Implements the routines used to read and write to  the  */
/*                  Registry.                                               */
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

#include "RegAccess.h"

/****************************************************************************/
/* Definitions                                                              */
/****************************************************************************/

#if defined(__linux__)
#define stricmp             strcasecmp      // GNU uses a silly name...
#endif


#define MAX_NAME_LEN        65              // Max length for variable names
#define MAX_VALUE_LEN       17              // Max length for variable string

/****************************************************************************/
/* Module Variables                                                         */
/****************************************************************************/

static  HKEY                hRegKey;        // Handle for registry key

/****************************************************************************/
/* FixName() - Builds indexed version of name string. If index specified is */
/* -1 (NO_INDEX), resulting string will be unindexed.                       */
/****************************************************************************/

static void FixName( const char *pszName, int iIndex, char *pszResult )
{
    if( iIndex == NO_INDEX )
        strcpy( pszResult, pszName );
    else
        sprintf( pszResult, "%s %d", pszName, iIndex );
}

/****************************************************************************/
/* OpenRegistry() - Opens the specified registry key to support read/write  */
/* of variables stored there.                                               */
/****************************************************************************/

BOOL OpenRegistry( const char *pszKeyPath )
{
    SECURITY_ATTRIBUTES     stSecAttribs;
    SECURITY_DESCRIPTOR     stSecDescr;
    DWORD                   dwDisposition = 0;

    // Initialize security attributes for ALL access (ie. empty ACL)

    InitializeSecurityDescriptor( &stSecDescr, SECURITY_DESCRIPTOR_REVISION );
    SetSecurityDescriptorDacl( &stSecDescr, TRUE, NULL, FALSE );

    memset( &stSecAttribs, 0, sizeof(SECURITY_ATTRIBUTES) );

    stSecAttribs.nLength              = sizeof(SECURITY_ATTRIBUTES);
    stSecAttribs.bInheritHandle       = FALSE;
    stSecAttribs.lpSecurityDescriptor = &stSecDescr;

    // Now that security is squared away, open the key.

    if( RegOpenKeyEx( HKEY_LOCAL_MACHINE, pszKeyPath, 0, KEY_ALL_ACCESS, &hRegKey ) != ERROR_SUCCESS )
    {
        // Create the key with completely open security since multiple apps will call this routine

        DWORD dwStatus = RegCreateKeyEx( HKEY_LOCAL_MACHINE, pszKeyPath, 0, __TEXT(""), REG_OPTION_NON_VOLATILE,
                                         KEY_ALL_ACCESS, &stSecAttribs, &hRegKey, &dwDisposition );

        return( dwStatus == ERROR_SUCCESS );
    }

    return( TRUE );
}

/****************************************************************************/
/* CloseRegistry() - Cleans up after registry access.                       */
/****************************************************************************/

void CloseRegistry( void )
{
    RegCloseKey( hRegKey );
}

/****************************************************************************/
/* byReadRegistry() - Reads binary byte array variable                      */
/****************************************************************************/

BOOL byReadRegistry( const char *pszName, int iIndex, BYTE *pbyDefault, BYTE *pbyResult, size_t tResultMax )
{
    char    szFixedName[MAX_NAME_LEN];
    DWORD   dwLength = tResultMax, dwType;

    FixName( pszName, iIndex, szFixedName );

    if( RegQueryValueEx( hRegKey, szFixedName, NULL, &dwType, pbyResult, &dwLength ) )
    {
        memcpy( pbyResult, pbyDefault, tResultMax );
        return( !RegSetValueEx( hRegKey, szFixedName, 0, REG_BINARY, pbyResult, tResultMax ) );
    }

    return( TRUE );
}

/****************************************************************************/
/* byWriteRegistry() - Writes binary byte array variable                    */
/****************************************************************************/

BOOL byWriteRegistry( const char *pszName, int iIndex, BYTE *pbyResult, int iResultSize )
{
    char    szFixedName[MAX_NAME_LEN];

    FixName( pszName, iIndex, szFixedName );
    return( !RegSetValueEx( hRegKey, szFixedName, 0, REG_BINARY, pbyResult, iResultSize ) );
}

/****************************************************************************/
/* szReadRegistry() - Reads string variable                                 */
/****************************************************************************/

BOOL szReadRegistry( const char *pszName, int iIndex, char *pszDefault, char *pszString, size_t tResultMax )
{
    char    szFixedName[MAX_NAME_LEN];
    DWORD    dwLength = tResultMax, dwType;

    FixName( pszName, iIndex, szFixedName );

    if( RegQueryValueEx( hRegKey, szFixedName, NULL, &dwType, (BYTE *)pszString, &dwLength ) )
    {
        strcpy( pszString, pszDefault );
        return( !RegSetValueEx( hRegKey, szFixedName, 0, REG_SZ, (BYTE *)pszDefault, strlen( pszString ) + 1 ) );
    }

    return( TRUE );
}

/****************************************************************************/
/* szWriteRegistry() - Writes String variable                               */
/****************************************************************************/

BOOL szWriteRegistry( const char *pszName, int iIndex, char *pszString )
{
    char    szFixedName[MAX_NAME_LEN];

    FixName( pszName, iIndex, szFixedName );
    return( !RegSetValueEx( hRegKey, szFixedName, 0, REG_SZ, (BYTE *)pszString, strlen( pszString ) + 1 ) );
}

/****************************************************************************/
/* iReadRegistry() - Reads integer variable                                 */
/****************************************************************************/

BOOL iReadRegistry( const char *pszName, int iIndex, int iDefault, int *piResult )
{
    char    szFixedName[MAX_NAME_LEN], szBuffer[MAX_VALUE_LEN];
    DWORD   dwLength = MAX_NAME_LEN, dwType;

    FixName( pszName, iIndex, szFixedName );

    if( RegQueryValueEx( hRegKey, szFixedName, NULL, &dwType, (BYTE *)szBuffer, &dwLength ) )
    {
        sprintf( szBuffer, "%d", *piResult = iDefault );
        return( !RegSetValueEx( hRegKey, szFixedName, 0, REG_SZ, (BYTE *)szBuffer, strlen( szBuffer ) + 1 ) );
    }

    *piResult = atoi( szBuffer );
    return( TRUE );
}

/****************************************************************************/
/* iWriteRegistry() - Writes integer variable                               */
/****************************************************************************/

BOOL iWriteRegistry( const char *pszName, int iIndex, int iValue )
{
    char    szFixedName[MAX_NAME_LEN], szBuffer[MAX_VALUE_LEN];

    FixName( pszName, iIndex, szFixedName );
    sprintf( szBuffer, "%d", iValue );
    return( !RegSetValueEx( hRegKey, szFixedName, 0, REG_SZ, (BYTE *)szBuffer, strlen( szBuffer ) + 1 ) );
}

/****************************************************************************/
/* fReadRegistry() - Reads floating point value                             */
/****************************************************************************/

BOOL fReadRegistry( const char *pszName, int iIndex, double fDefault, double *pfResult )
{
    char    szFixedName[MAX_NAME_LEN], szBuffer[MAX_VALUE_LEN];
    DWORD   dwLength = MAX_NAME_LEN, dwType;

    FixName( pszName, iIndex, szFixedName );

    if( RegQueryValueEx( hRegKey, szFixedName, NULL, &dwType, (BYTE *)szBuffer, &dwLength ) )
    {
        sprintf( szBuffer, "%f", *pfResult = fDefault );
        return( !RegSetValueEx( hRegKey, szFixedName, 0, REG_SZ, (BYTE *)szBuffer, strlen( szBuffer ) + 1 ) );
    }

    *pfResult = atof( szBuffer );
    return( TRUE );
}

/****************************************************************************/
/* fWriteRegistry() - Writes floating point variable                        */
/****************************************************************************/

BOOL fWriteRegistry( const char *pszName, int iIndex, double fValue )
{
    char    szFixedName[MAX_NAME_LEN], szBuffer[MAX_VALUE_LEN];

    FixName( pszName, iIndex, szFixedName );
    sprintf( szBuffer, "%f", fValue );
    return( !RegSetValueEx( hRegKey, szFixedName, 0, REG_SZ, (BYTE *)szBuffer, strlen( szBuffer ) + 1 ) );
}

/****************************************************************************/
/* bReadRegistry() - Reads boolean value                                    */
/****************************************************************************/

BOOL bReadRegistry( const char *pszName, int iIndex, BOOL bDefault, BOOL *pbResult )
{
    char    szFixedName[MAX_NAME_LEN], szBuffer[MAX_VALUE_LEN];
    DWORD   dwLength = MAX_NAME_LEN, dwType;

    FixName( pszName, iIndex, szFixedName );

    if( RegQueryValueEx( hRegKey, szFixedName, NULL, &dwType, (BYTE *)szBuffer, &dwLength ) )
    {
        *pbResult = bDefault;
        strcpy( szBuffer, ( bDefault )? "TRUE" : "FALSE" );
        return( !RegSetValueEx( hRegKey, szFixedName, 0, REG_SZ, (BYTE *)szBuffer, strlen( szBuffer ) + 1 ) );
    }

    *pbResult = (stricmp( szBuffer, "TRUE" ) == 0);
    return( TRUE );
}

/****************************************************************************/
/* bWriteRegistry() - Writes boolean variable                               */
/****************************************************************************/

BOOL bWriteRegistry( const char *pszName, int iIndex, BOOL bValue )
{
    char    szFixedName[MAX_NAME_LEN], szBuffer[MAX_VALUE_LEN];

    FixName( pszName, iIndex, szFixedName );
    strcpy( szBuffer, ( bValue )? "TRUE" : "FALSE" );
    return( !RegSetValueEx( hRegKey, szFixedName, 0, REG_SZ, (BYTE *)szBuffer, strlen( szBuffer ) + 1 ) );
}

