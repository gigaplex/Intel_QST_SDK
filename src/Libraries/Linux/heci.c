/****************************************************************************/
/*                                                                          */
/*  Module:         heci.c                                                  */
/*                                                                          */
/*  Description:    Provides the base functions necessary to support Linux  */
/*                  applications  communicating  with  firmware subsystems  */
/*                  running on the Management Engine. The services of  the  */
/*                  Host  to  Embedded  Controller Interface (HECI) driver  */
/*                  are used to perform this communication.                 */
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

#ifndef __linux__
#error This source module intended for use in Linux environments only
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdint.h>
#include <aio.h>

#include "heci.h"

/****************************************************************************/
/* Configuration Variables                                                  */
/****************************************************************************/

#define SEND_TIMEOUT    1000                // One second

/****************************************************************************/
/* Definitions for delivering IOCTLs to the HECI driver                     */
/****************************************************************************/

#pragma pack(1)

// Client connection data

typedef struct _HECI_CLIENT_DATA
{
    UINT32              dwMaxMessageSize;
    UINT8               byProtocolVersion;
    UINT8               _reserved[3];
} HECI_CLIENT_DATA;

// IOCTL data

typedef struct _HECI_IOCTL_DATA
{
    union
    {
        GUID             in_client_guid;
        HECI_CLIENT_DATA out_client_data;
    };
} HECI_IOCTL_DATA;

// IOCTL Definitions

#define IOCTL_HECI_GET_VERSION    _IOWR( 'H', 0x800, HECI_IOCTL_DATA )
#define IOCTL_HECI_CONNECT_CLIENT _IOWR( 'H', 0x801, HECI_IOCTL_DATA )

#pragma pack(0)

/****************************************************************************/
/* Persistent Variables                                                     */
/****************************************************************************/

static int              hDriver;                // Handle for driver connection
static void *           pvReceiveBuff;          // Pointer to receive buffer
static size_t           tMaxReceive;            // Size of receive buffer

/****************************************************************************/
/* HeciDisconnect() - Disconnects from the ME Subsystem                     */
/****************************************************************************/

void HeciDisconnect( void )
{
    if( pvReceiveBuff )
    {
        free( pvReceiveBuff );
        pvReceiveBuff = NULL;
        tMaxReceive = 0;
    }

    if( hDriver != -1 )
    {
        close( hDriver );
        hDriver = -1;
    }
}

/****************************************************************************/
/* HeciConnect() - Connects to a ME Subsystem                               */
/****************************************************************************/

size_t HeciConnect( const GUID *pSubsysGUID )
{
    char                byData[sizeof(GUID)];
    HECI_CLIENT_DATA    *pstClient = (HECI_CLIENT_DATA *)byData;
    HECI_IOCTL_DATA     stIOCTL;

    HeciDisconnect();

    // Open a connection to the driver

    hDriver = open( "/dev/mei", O_RDWR );

    if( hDriver == -1 )
        return( -1 );

    // Send a subsystem connection request

    memcpy( &stIOCTL.in_client_guid, pSubsysGUID, sizeof(GUID) );

    if( ioctl( hDriver, IOCTL_HECI_CONNECT_CLIENT, &stIOCTL ) )
    {
        HeciDisconnect();
        return( -1 );
    }

    // Create a buffer for receiving response packets

    tMaxReceive = (size_t)pstClient->dwMaxMessageSize;

    pvReceiveBuff = malloc( tMaxReceive );

    if( !pvReceiveBuff )
    {
        HeciDisconnect();
        return( -1 );
    }

    // Let user know maximum size for command/response packets

    return( tMaxReceive );
}

/****************************************************************************/
/* HeciReceive() - Receive a packet from the ME Subsystem                   */
/****************************************************************************/

int HeciReceive( void *pvBuff, size_t tBuffMax )
{
    // Get the next response packet

    int iLen = read( hDriver, pvReceiveBuff, tMaxReceive );

    if( iLen < 0 )
        return( -1 );

    // If it doesn't fit, let user know

    if( iLen > tBuffMax )
    {
        errno = ENOSPC;
        return( -1 );
    }

    // Fits; copy into user's buffer and tell them its size

    memcpy( pvBuff, pvReceiveBuff, iLen );
    return( iLen );
}

/****************************************************************************/
/* HeciSend() - Sends a packet to the ME Subsystem                          */
/****************************************************************************/

BOOL HeciSend( void *pvBuff, size_t tBuffLen )
{
    // Send the packet to the driver for transmission

    int iLen = write( hDriver, pvBuff, (int)tBuffLen );

    if( iLen < 0 )
        return( FALSE );
    else
    {
        // Driver accepted packet; wait for transmission to actually complete

        fd_set          stFDSet;
        struct timeval  stTime;
        int             iNumFD;

        stTime.tv_sec  = SEND_TIMEOUT / 1000;
        stTime.tv_usec = (SEND_TIMEOUT % 1000) * 1000000;

        FD_ZERO( &stFDSet );
        FD_SET( hDriver, &stFDSet );

        iNumFD = select( hDriver + 1, &stFDSet, NULL, NULL, &stTime );

        return( (iNumFD > 0) && FD_ISSET( hDriver, &stFDSet ) );
    }
}

/****************************************************************************/
/* HeciInitialize() - Initializes the module                                */
/****************************************************************************/

BOOL HeciInitialize( void )
{
    hDriver       = -1;
    pvReceiveBuff = NULL;
    tMaxReceive   = 0;

    return( TRUE );
}

/****************************************************************************/
/* HeciCleanup() - Cleans up after the module                               */
/****************************************************************************/

void HeciCleanup( void )
{
    HeciDisconnect();
}

