/****************************************************************************/
/*                                                                          */
/*  Module:         QstController.cpp                                       */
/*                                                                          */
/*  Description:    Implements the Fan Speed Controller component  of  the  */
/*                  ActiveX  encapsulation  of  the  Intel(R) Quiet System  */
/*                  Technology (QST) Instrumentation Library.               */
/*                                                                          */
/****************************************************************************/

/****************************************************************************/
/*                                                                          */
/*        Copyright (c) 2009, Intel Corporation. All Rights Reserved.       */
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

#include "stdafx.h"
#include "QstController.h"
#include "QstInst.h"

#define ATLERROR(x,y) AtlReportError( CLSID_QstSensor, x, GUID_NULL, HRESULT_FROM_WIN32( y ) )

// CQstController

STDMETHODIMP CQstController::InterfaceSupportsErrorInfo( REFIID riid )
{
    static const IID* arr[] =
    {
        &IID_IQstController
    };

    for( int i=0; i < sizeof(arr) / sizeof(arr[0]); i++ )
    {
        if( InlineIsEqualGUID( *arr[i], riid ) )
            return( S_OK );
    }
    return( S_FALSE );
}

/****************************************************************************/
/* CQstController::GetCount() - Returns a count of the number of fan speed  */
/* controllers that are being managed by QST.                               */
/****************************************************************************/

STDMETHODIMP CQstController::GetCount( BYTE* pbyCount )
{
    int iCount;

    if( QstGetControllerCount( &iCount ) )
    {
        *pbyCount = (BYTE)iCount;
        return( S_OK );
    }
    else
        return( ATLERROR( L"Unexpected result from QST Instrumentation Library", GetLastError() ) );
}

/****************************************************************************/
/* CQstController::GetFunction() - Returns the function (usage) indicator   */
/* for the specified fan speed controller                                   */
/****************************************************************************/

STDMETHODIMP CQstController::GetFunction( BYTE byController, BYTE* pbyFunction )
{
    int                 iCount;
    QST_FUNCTION        eFunction;

    if( !QstGetControllerCount( &iCount ) )
        return( ATLERROR( L"Unexpected result from QST Instrumentation Library", GetLastError() ) );

    if( byController >= (BYTE)iCount )
        return( ATLERROR( L"Invalid Controller Index specified", ERROR_BAD_ARGUMENTS ) );

    if( QstGetControllerConfiguration( (int)byController, &eFunction ) )
    {
        *pbyFunction = (BYTE)eFunction;
        return( S_OK );
    }
    else
        return( ATLERROR( L"Unexpected result from QST Instrumentation Library", GetLastError() ) );
}

/****************************************************************************/
/* CQstController::GetHealth() - Returns the health indicator (byte) for    */
/* the specified fan speed controller                                       */
/****************************************************************************/

STDMETHODIMP CQstController::GetHealth( BYTE byController, BYTE* pbyHealth )
{
    int                 iCount;
    QST_HEALTH          eHealth;
    QST_CONTROL_STATE   eState;

    if( !QstGetControllerCount( &iCount ) )
        return( ATLERROR( L"Unexpected result from QST Instrumentation Library", GetLastError() ) );

    if( byController >= (BYTE)iCount )
        return( ATLERROR( L"Invalid Controller Index specified", ERROR_BAD_ARGUMENTS ) );

    if( QstGetControllerState( (int)byController, &eHealth, &eState ) )
    {
        *pbyHealth   = (BYTE)eHealth;
        return( S_OK );
    }
    else
        return( ATLERROR( L"Unexpected result from QST Instrumentation Library", GetLastError() ) );
}

/****************************************************************************/
/* CQstController::GetState() - Returns the state of the specified fan      */
/* speed controller                                                         */
/****************************************************************************/

STDMETHODIMP CQstController::GetState( BYTE byController, BYTE* pbyState )
{
    int                 iCount;
    QST_HEALTH          eHealth;
    QST_CONTROL_STATE   eState;

    if( !QstGetControllerCount( &iCount ) )
        return( ATLERROR( L"Unexpected result from QST Instrumentation Library", GetLastError() ) );

    if( byController >= (BYTE)iCount )
        return( ATLERROR( L"Invalid Controller Index specified", ERROR_BAD_ARGUMENTS ) );

    if( QstGetControllerState( (int)byController, &eHealth, &eState ) )
    {
        *pbyState = (BYTE)eState;
        return( S_OK );
    }
    else
        return( ATLERROR( L"Unexpected result from QST Instrumentation Library", GetLastError() ) );
}

/****************************************************************************/
/* CQstController::GetDuty() - Returns the current duty cycle being output  */
/* by the specified fan speed controller                                    */
/****************************************************************************/

STDMETHODIMP CQstController::GetDuty( BYTE byController, FLOAT* pfDuty )
{
    int                 iCount;
    float               fDuty;

    if( !QstGetControllerCount( &iCount ) )
        return( ATLERROR( L"Unexpected result from QST Instrumentation Library", GetLastError() ) );

    if( byController >= (BYTE)iCount )
        return( ATLERROR( L"Invalid Controller Index specified", ERROR_BAD_ARGUMENTS ) );

    if( QstGetControllerDutyCycle( (int)byController, &fDuty ) )
    {
        *pfDuty = fDuty;
        return( S_OK );
    }
    else
        return( ATLERROR( L"Unexpected result from QST Instrumentation Library", GetLastError() ) );
}
