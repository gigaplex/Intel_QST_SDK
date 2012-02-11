/****************************************************************************/
/*                                                                          */
/*  Module:         QstSensor.cpp                                           */
/*                                                                          */
/*  Description:    Implements the  Sensor  monitoring  component  of  the  */
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
#include "QstSensor.h"
#include "QstInst.h"

#define ATLERROR(x,y) AtlReportError( CLSID_QstSensor, x, GUID_NULL, HRESULT_FROM_WIN32( y ) )

// CQstSensor

STDMETHODIMP CQstSensor::InterfaceSupportsErrorInfo( REFIID riid )
{
    static const IID* arr[] = { &IID_IQstSensor };

    for( int i=0; i < sizeof(arr) / sizeof(arr[0]); i++ )
    {
        if( InlineIsEqualGUID( *arr[i], riid ) )
            return( S_OK );
    }

    return( S_FALSE );
}

/****************************************************************************/
/* CQstSensor::GetCount() - Returns a count of the number of sensors of the */
/* specified type that are being monitored.                                 */
/****************************************************************************/

STDMETHODIMP CQstSensor::GetCount( BYTE byType, BYTE* pbyCount )
{
    QST_SENSOR_TYPE eType;
    int             iCount;

    if( (byType >= TEMPERATURE_SENSOR) && (byType <= CURRENT_SENSOR) )
        eType = (QST_SENSOR_TYPE)byType;
    else
        return( ATLERROR( L"Invalid Sensor Type specified", ERROR_BAD_ARGUMENTS ) );

    if( QstGetSensorCount( eType, &iCount ) )
    {
        *pbyCount = (BYTE)iCount;
        return( S_OK );
    }
    else
        return( ATLERROR( L"Unexpected result from QST Instrumentation Library", GetLastError() ) );
}

/****************************************************************************/
/* CQstSensor::GetCritHighThresh() - Returns the critical high (over-)      */
/* threshold for the specified sensor. This is only valid for temperature,  */
/* voltage and current sensors...                                           */
/****************************************************************************/

STDMETHODIMP CQstSensor::GetCritHighThresh( BYTE byType, BYTE bySensor, FLOAT* pfCrit )
{
    QST_SENSOR_TYPE eType;
    int             iCount;
    float           fNonCrit, fCrit, fNonRecov;

    switch( byType )
    {
    case TEMPERATURE_SENSOR:
    case VOLTAGE_SENSOR:
    case CURRENT_SENSOR:

        eType = (QST_SENSOR_TYPE)byType;
        break;

    case FAN_SPEED_SENSOR:
    default:

        return( ATLERROR( L"Invalid Sensor Type specified", ERROR_BAD_ARGUMENTS ) );
    }

    if( !QstGetSensorCount( eType, &iCount ) )
        return( ATLERROR( L"Unexpected result from QST Instrumentation Library", GetLastError() ) );

    if( bySensor >= (BYTE)iCount )
        return( ATLERROR( L"Invalid Sensor Index specified", ERROR_BAD_ARGUMENTS ) );

    if( QstGetSensorThresholdsHigh( eType, (int)bySensor, &fNonCrit, &fCrit, &fNonRecov ) )
    {
        *pfCrit = fCrit;
        return( S_OK );
    }
    else
        return( ATLERROR( L"Unexpected result from QST Instrumentation Library", GetLastError() ) );
}

/****************************************************************************/
/* CQstSensor::GetCritLowThresh() - Returns the critical low (under-)       */
/* threshold for the specified sensor. This is only valid for fan speed,    */
/* voltage and current sensors...                                           */
/****************************************************************************/

STDMETHODIMP CQstSensor::GetCritLowThresh( BYTE byType, BYTE bySensor, FLOAT* pfCrit )
{
    QST_SENSOR_TYPE eType;
    int             iCount;
    float           fNonCrit, fCrit, fNonRecov;

    switch( byType )
    {
    case FAN_SPEED_SENSOR:
    case VOLTAGE_SENSOR:
    case CURRENT_SENSOR:

        eType = (QST_SENSOR_TYPE)byType;
        break;

    case TEMPERATURE_SENSOR:
    default:

        return( ATLERROR( L"Invalid Sensor Type specified", ERROR_BAD_ARGUMENTS ) );
    }

    if( !QstGetSensorCount( eType, &iCount ) )
        return( ATLERROR( L"Unexpected result from QST Instrumentation Library", GetLastError() ) );

    if( bySensor >= (BYTE)iCount )
        return( ATLERROR( L"Invalid Sensor Index specified", ERROR_BAD_ARGUMENTS ) );

    if( QstGetSensorThresholdsLow( eType, (int)bySensor, &fNonCrit, &fCrit, &fNonRecov ) )
    {
        *pfCrit = fCrit;
        return( S_OK );
    }
    else
        return( ATLERROR( L"Unexpected result from QST Instrumentation Library", GetLastError() ) );
}

/****************************************************************************/
/* CQstSensor::GetFunction() - Returns the function (usage) indicator for   */
/* the specified sensor                                                     */
/****************************************************************************/

STDMETHODIMP CQstSensor::GetFunction( BYTE byType, BYTE bySensor, BYTE* pbyFunction )
{
    QST_SENSOR_TYPE eType;
    int             iCount;
    QST_FUNCTION    eFunction;
    BOOL            bRelative;
    float           fNominal;

    if( (byType >= TEMPERATURE_SENSOR) && (byType <= CURRENT_SENSOR) )
        eType = (QST_SENSOR_TYPE)byType;
    else
        return( ATLERROR( L"Invalid Sensor Type specified", ERROR_BAD_ARGUMENTS ) );

    if( !QstGetSensorCount( eType, &iCount ) )
        return( ATLERROR( L"Unexpected result from QST Instrumentation Library", GetLastError() ) );

    if( bySensor >= (BYTE)iCount )
        return( ATLERROR( L"Invalid Sensor Index specified", ERROR_BAD_ARGUMENTS ) );

    if( QstGetSensorConfiguration( eType, (int)bySensor, &eFunction, &bRelative, &fNominal ) )
    {
        *pbyFunction = (BYTE)eFunction;
        return( S_OK );
    }
    else
        return( ATLERROR( L"Unexpected result from QST Instrumentation Library", GetLastError() ) );
}

/****************************************************************************/
/* CQstSensor::GetHealth() - Returns the health status (byte) for the       */
/* specified sensor.                                                        */
/****************************************************************************/

STDMETHODIMP CQstSensor::GetHealth( BYTE byType, BYTE bySensor, BYTE* pbyHealth )
{
    QST_SENSOR_TYPE eType;
    int             iCount;
    QST_HEALTH      stHealth;

    if( (byType >= TEMPERATURE_SENSOR) && (byType <= CURRENT_SENSOR) )
        eType = (QST_SENSOR_TYPE)byType;
    else
        return( ATLERROR( L"Invalid Sensor Type specified", ERROR_BAD_ARGUMENTS ) );

    if( !QstGetSensorCount( eType, &iCount ) )
        return( ATLERROR( L"Unexpected result from QST Instrumentation Library", GetLastError() ) );

    if( bySensor >= (BYTE)iCount )
        return( ATLERROR( L"Invalid Sensor Index specified", ERROR_BAD_ARGUMENTS ) );

    if( QstGetSensorHealth( eType, (int)bySensor, &stHealth ) )
    {
        *pbyHealth = *(BYTE *)&stHealth;
        return( S_OK );
    }
    else
        return( ATLERROR( L"Unexpected result from QST Instrumentation Library", GetLastError() ) );
}

/****************************************************************************/
/* CQstSensor::GetNominal() - Returns the nominal reading for the specied   */
/* sensor.                                                                  */
/****************************************************************************/

STDMETHODIMP CQstSensor::GetNominal( BYTE byType, BYTE bySensor, FLOAT* pfNominal )
{
    QST_SENSOR_TYPE eType;
    int             iCount;
    QST_FUNCTION    eFunction;
    BOOL            bRelative;
    float           fNominal;

    if( (byType >= TEMPERATURE_SENSOR) && (byType <= CURRENT_SENSOR) )
        eType = (QST_SENSOR_TYPE)byType;
    else
        return( ATLERROR( L"Invalid Sensor Type specified", ERROR_BAD_ARGUMENTS ) );

    if( !QstGetSensorCount( eType, &iCount ) )
        return( ATLERROR( L"Unexpected result from QST Instrumentation Library", GetLastError() ) );

    if( bySensor >= (BYTE)iCount )
        return( ATLERROR( L"Invalid Sensor Index specified", ERROR_BAD_ARGUMENTS ) );

    if( QstGetSensorConfiguration( eType, (int)bySensor, &eFunction, &bRelative, &fNominal ) )
    {
        *pfNominal  = fNominal;
        return( S_OK );
    }
    else
        return( ATLERROR( L"Unexpected result from QST Instrumentation Library", GetLastError() ) );
}

/****************************************************************************/
/* CQstSensor::GetNonCritHighThresh() - Returns the non-critical high       */
/* (over-) threshold for the specified sensor. This is only valid for       */
/* temperature, voltage and current sensors...                              */
/****************************************************************************/

STDMETHODIMP CQstSensor::GetNonCritHighThresh( BYTE byType, BYTE bySensor, FLOAT* pfNonCrit )
{
    QST_SENSOR_TYPE eType;
    int             iCount;
    float           fNonCrit, fCrit, fNonRecov;

    switch( byType )
    {
    case TEMPERATURE_SENSOR:
    case VOLTAGE_SENSOR:
    case CURRENT_SENSOR:

        eType = (QST_SENSOR_TYPE)byType;
        break;

    case FAN_SPEED_SENSOR:
    default:

        return( ATLERROR( L"Invalid Sensor Type specified", ERROR_BAD_ARGUMENTS ) );
    }

    if( !QstGetSensorCount( eType, &iCount ) )
        return( ATLERROR( L"Unexpected result from QST Instrumentation Library", GetLastError() ) );

    if( bySensor >= (BYTE)iCount )
        return( ATLERROR( L"Invalid Sensor Index specified", ERROR_BAD_ARGUMENTS ) );

    if( QstGetSensorThresholdsHigh( eType, (int)bySensor, &fNonCrit, &fCrit, &fNonRecov ) )
    {
        *pfNonCrit = fNonCrit;
        return( S_OK );
    }
    else
        return( ATLERROR( L"Unexpected result from QST Instrumentation Library", GetLastError() ) );
}

/****************************************************************************/
/* CQstSensor::GetNonCritLowThresh() - Returns the non-critical low (under-)*/
/* threshold for the specified sensor. This is only valid for fan speed,    */
/* voltage and current sensors...                                           */
/****************************************************************************/

STDMETHODIMP CQstSensor::GetNonCritLowThresh( BYTE byType, BYTE bySensor, FLOAT* pfNonCrit )
{
    QST_SENSOR_TYPE eType;
    int             iCount;
    float           fNonCrit, fCrit, fNonRecov;

    switch( byType )
    {
    case FAN_SPEED_SENSOR:
    case VOLTAGE_SENSOR:
    case CURRENT_SENSOR:

        eType = (QST_SENSOR_TYPE)byType;
        break;

    case TEMPERATURE_SENSOR:
    default:

        return( ATLERROR( L"Invalid Sensor Type specified", ERROR_BAD_ARGUMENTS ) );
    }

    if( !QstGetSensorCount( eType, &iCount ) )
        return( ATLERROR( L"Unexpected result from QST Instrumentation Library", GetLastError() ) );

    if( bySensor >= (BYTE)iCount )
        return( ATLERROR( L"Invalid Sensor Index specified", ERROR_BAD_ARGUMENTS ) );

    if( QstGetSensorThresholdsLow( eType, (int)bySensor, &fNonCrit, &fCrit, &fNonRecov ) )
    {
        *pfNonCrit = fNonCrit;
        return( S_OK );
    }
    else
        return( ATLERROR( L"Unexpected result from QST Instrumentation Library", GetLastError() ) );
}

/****************************************************************************/
/* CQstSensor::GetNonRecovHighThresh() - Returns the non-recoverable high   */
/* (over-) threshold for the specified sensor. This is only valid for       */
/* temperature, voltage and current sensors...                              */
/****************************************************************************/

STDMETHODIMP CQstSensor::GetNonRecovHighThresh( BYTE byType, BYTE bySensor, FLOAT* pfNonRecov )
{
    QST_SENSOR_TYPE eType;
    int             iCount;
    float           fNonCrit, fCrit, fNonRecov;

    switch( byType )
    {
    case TEMPERATURE_SENSOR:
    case VOLTAGE_SENSOR:
    case CURRENT_SENSOR:

        eType = (QST_SENSOR_TYPE)byType;
        break;

    case FAN_SPEED_SENSOR:
    default:

        return( ATLERROR( L"Invalid Sensor Type specified", ERROR_BAD_ARGUMENTS ) );
    }

    if( !QstGetSensorCount( eType, &iCount ) )
        return( ATLERROR( L"Unexpected result from QST Instrumentation Library", GetLastError() ) );

    if( bySensor >= (BYTE)iCount )
        return( ATLERROR( L"Invalid Sensor Index specified", ERROR_BAD_ARGUMENTS ) );

    if( QstGetSensorThresholdsHigh( eType, (int)bySensor, &fNonCrit, &fCrit, &fNonRecov ) )
    {
        *pfNonRecov = fNonRecov;
        return( S_OK );
    }
    else
        return( ATLERROR( L"Unexpected result from QST Instrumentation Library", GetLastError() ) );
}

/****************************************************************************/
/* CQstSensor::GetNonRecovLowThresh() - Returns the non-recoverable low     */
/* (under-) threshold for the specified sensor. This is only valid for fan  */
/* speed, voltage and current sensors...                                    */
/****************************************************************************/

STDMETHODIMP CQstSensor::GetNonRecovLowThresh( BYTE byType, BYTE bySensor, FLOAT* pfNonRecov )
{
    QST_SENSOR_TYPE eType;
    int             iCount;
    float           fNonCrit, fCrit, fNonRecov;

    switch( byType )
    {
    case FAN_SPEED_SENSOR:
    case VOLTAGE_SENSOR:
    case CURRENT_SENSOR:

        eType = (QST_SENSOR_TYPE)byType;
        break;

    case TEMPERATURE_SENSOR:
    default:

        return( ATLERROR( L"Invalid Sensor Type specified", ERROR_BAD_ARGUMENTS ) );
    }

    if( !QstGetSensorCount( eType, &iCount ) )
        return( ATLERROR( L"Unexpected result from QST Instrumentation Library", GetLastError() ) );

    if( bySensor >= (BYTE)iCount )
        return( ATLERROR( L"Invalid Sensor Index specified", ERROR_BAD_ARGUMENTS ) );

    if( QstGetSensorThresholdsLow( eType, (int)bySensor, &fNonCrit, &fCrit, &fNonRecov ) )
    {
        *pfNonRecov = fNonRecov;
        return( S_OK );
    }
    else
        return( ATLERROR( L"Unexpected result from QST Instrumentation Library", GetLastError() ) );
}

/****************************************************************************/
/* CQstSensor::GetReading() - Returns the current reading for the specified */
/* sensor.                                                                  */
/****************************************************************************/

STDMETHODIMP CQstSensor::GetReading( BYTE byType, BYTE bySensor, FLOAT* pfReading )
{
    QST_SENSOR_TYPE eType;
    int             iCount;
    float           fReading;

    if( (byType >= TEMPERATURE_SENSOR) && (byType <= CURRENT_SENSOR) )
        eType = (QST_SENSOR_TYPE)byType;
    else
        return( ATLERROR( L"Invalid Sensor Type specified", ERROR_BAD_ARGUMENTS ) );

    if( !QstGetSensorCount( eType, &iCount ) )
        return( ATLERROR( L"Unexpected result from QST Instrumentation Library", GetLastError() ) );

    if( bySensor >= (BYTE)iCount )
        return( ATLERROR( L"Invalid Sensor Index specified", ERROR_BAD_ARGUMENTS ) );

    if( QstGetSensorReading( eType, (int)bySensor, &fReading ) )
    {
        *pfReading = fReading;
        return( S_OK );
    }
    else
        return( ATLERROR( L"Unexpected result from QST Instrumentation Library", GetLastError() ) );
}

/****************************************************************************/
/* CQstSensor::IsRelative() - Returns an indication of whether or not the   */
/* specified sensor returns readings in relative (vs. absolute) form. This  */
/* is typically only applicable to temperature sensors...                   */
/****************************************************************************/

STDMETHODIMP CQstSensor::IsRelative( BYTE byType, BYTE bySensor, VARIANT_BOOL* pbRelative )
{
    QST_SENSOR_TYPE eType;
    int             iCount;
    QST_FUNCTION    eFunction;
    BOOL            bRelative;
    float           fNominal;

    if( (byType >= TEMPERATURE_SENSOR) && (byType <= CURRENT_SENSOR) )
        eType = (QST_SENSOR_TYPE)byType;
    else
        return( ATLERROR( L"Invalid Sensor Type specified", ERROR_BAD_ARGUMENTS ) );

    if( !QstGetSensorCount( eType, &iCount ) )
        return( ATLERROR( L"Unexpected result from QST Instrumentation Library", GetLastError() ) );

    if( bySensor >= (BYTE)iCount )
        return( ATLERROR( L"Invalid Sensor Index specified", ERROR_BAD_ARGUMENTS ) );

    if( byType != TEMPERATURE_SENSOR )
    {
        *pbRelative = VARIANT_FALSE;
        return( S_OK );
    }

    if( QstGetSensorConfiguration( eType, (int)bySensor, &eFunction, &bRelative, &fNominal ) )
    {
        *pbRelative = (bRelative)? VARIANT_TRUE : VARIANT_FALSE;
        return( S_OK );
    }
    else
        return( ATLERROR( L"Unexpected result from QST Instrumentation Library", GetLastError() ) );
}

/****************************************************************************/
/* CQstSensor::SetHighThresh() - Sets the high (over-) thresholds for the   */
/* specified sensor. This is only valid for temperature, voltage and        */
/* current sensors...                                                       */
/****************************************************************************/

STDMETHODIMP CQstSensor::SetHighThresh( BYTE byType, BYTE bySensor, FLOAT fNonCrit, FLOAT fCrit, FLOAT fNonRecov, VARIANT_BOOL* pbSuccess )
{
    QST_SENSOR_TYPE eType;
    int             iCount;

    *pbSuccess = VARIANT_FALSE;

    switch( byType )
    {
    case TEMPERATURE_SENSOR:
    case VOLTAGE_SENSOR:
    case CURRENT_SENSOR:

        eType = (QST_SENSOR_TYPE)byType;
        break;

    case FAN_SPEED_SENSOR:
    default:

        return( ATLERROR( L"Invalid Sensor Type specified", ERROR_BAD_ARGUMENTS ) );
    }

    if( !QstGetSensorCount( eType, &iCount ) )
        return( ATLERROR( L"Unexpected result from QST Instrumentation Library", GetLastError() ) );

    if( bySensor >= (BYTE)iCount )
        return( ATLERROR( L"Invalid Sensor Index specified", ERROR_BAD_ARGUMENTS ) );

    if( QstSetSensorThresholdsHigh( eType, (int)bySensor, (float)fNonCrit, (float)fCrit, (float)fNonRecov ) )
    {
        *pbSuccess = VARIANT_TRUE;
        return( S_OK );
    }
    else
        return( ATLERROR( L"Unexpected result from QST Instrumentation Library", GetLastError() ) );
}

/****************************************************************************/
/* CQstSensor::SetLowThresh() - Sets the low (under-) thresholds for the    */
/* specified sensor. This is only valid for fan speed, voltage and current  */
/* sensors...                                                               */
/****************************************************************************/

STDMETHODIMP CQstSensor::SetLowThresh( BYTE byType, BYTE bySensor, FLOAT fNonCrit, FLOAT fCrit, FLOAT fNonRecov, VARIANT_BOOL* pbSuccess )
{
    QST_SENSOR_TYPE eType;
    int             iCount;

    *pbSuccess = VARIANT_FALSE;

    switch( byType )
    {
    case FAN_SPEED_SENSOR:
    case VOLTAGE_SENSOR:
    case CURRENT_SENSOR:

        eType = (QST_SENSOR_TYPE)byType;
        break;

    case TEMPERATURE_SENSOR:
    default:

        return( ATLERROR( L"Invalid Sensor Type specified", ERROR_BAD_ARGUMENTS ) );
    }

    if( !QstGetSensorCount( eType, &iCount ) )
        return( ATLERROR( L"Unexpected result from QST Instrumentation Library", GetLastError() ) );

    if( bySensor >= (BYTE)iCount )
        return( ATLERROR( L"Invalid Sensor Index specified", ERROR_BAD_ARGUMENTS ) );

    if( QstSetSensorThresholdsLow( eType, (int)bySensor, (float)fNonCrit, (float)fCrit, (float)fNonRecov ) )
    {
        *pbSuccess = VARIANT_TRUE;
        return( S_OK );
    }
    else
        return( ATLERROR( L"Unexpected result from QST Instrumentation Library", GetLastError() ) );
}

