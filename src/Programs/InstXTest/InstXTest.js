/****************************************************************************/
/*                                                                          */
/*  Module:         QstCommXTest.js                                         */
/*                                                                          */
/*  Description:    Sample Javascript module that demonstrates how to make  */
/*                  use of the functions provided  by  the  QstInstX  DLL,  */
/*                  which  implements  an  ActiveX Control Wrapper for the  */
/*                  Intel(R) Quiet System Technology (QST) Instrumentation  */
/*                  DLL.                                                    */
/*                                                                          */
/*  Notes:      1.  This sample makes use of System.Console.WriteLine() to  */
/*                  format its output. As a result, attempts to execute it  */
/*                  using the Windows Scripting Host will fail  (a  syntax  */
/* 					error  at  line  56). Instead, you need to compile the  */
/* 					script into a .EXE using jsc, the Microsoft .NET Java-  */
/*                  script  compiler.  The command-line necessary to do so  */
/*                  is "jsc QstCommXTest.js"...                             */
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

import System;

/****************************************************************************/
/* Global Variables                                                         */
/****************************************************************************/

var cSensor     = new ActiveXObject( "QstInstX.QstSensor"     );
var cController = new ActiveXObject( "QstInstX.QstController" );

/****************************************************************************/
/* GetSensorType() - Returns descriptive string for specified sensor type   */
/****************************************************************************/

function GetSensorType( iType )
{
    switch( iType )
    {
    case 0:  return( "Temperature" );
    case 1:  return( "Voltage" );
    case 2:  return( "Fan Speed" );
    case 3:  return( "Current" );
    default: return( "Unknown" );
    }
}

/****************************************************************************/
/* GetSensorUsage() - Returns usage string for specified sensor             */
/****************************************************************************/

function GetSensorUsage( iType, iSensor )
{
    var iUsage = cSensor.GetFunction( iType, iSensor );

    switch( iType )
    {
    case 0:     // Temperature Sensors

        switch( iUsage )
        {
        case 1:  return( "Processor Temperature" );
        case 2:  return( "Processor Temperature" );
        case 3:  return( "I/O Controller Hub Temperature" );
        case 4:  return( "Memory Controller Hub Temperature" );
        case 5:  return( "Voltage Regulator Temperature" );
        case 6:  return( "Memory DIMM Temperature" );
        case 7:  return( "Motherboard Temperature" );
        case 8:  return( "Ambient Air Temperature" );
        case 9:  return( "Processor Fan Inlet Air Temperature" );
        case 10: return( "System Inlet Air Temperature" );
        case 11: return( "System Outlet Air Temperature" );
        case 12: return( "Power Supply Internal/Hotspot Temperature" );
        case 13: return( "Power Supply Inlet Air Temperature" );
        case 14: return( "Power Supply Outlet Air Temperature" );
        case 15: return( "Hard Drive Temperature" );
        case 16: return( "Graphics Processor Temperature" );
        case 17: return( "I/O Hub Temperature" );
        case 18: return( "Peripheral Controller Hub (PCH) Temperature" );
        default: return( "Other/Unknown Temperature" );
        }

    case 1:     // Voltage Sensors

        switch( iUsage )
        {
        case 1:  return( "+12 Volts" );
        case 2:  return( "-12 Volts" );
        case 3:  return( "+5 Volts" );
        case 4:  return( "+5 Volt Backup" );
        case 5:  return( "-5 Volts" );
        case 6:  return( "+3.3 Volts" );
        case 7:  return( "+2.5 Volts" );
        case 8:  return( "+1.5 Volts" );
        case 9:  return( "Processor Vcc" );
        case 10: return( "Processor 2 Vcc" );
        case 11: return( "Processor 3 Vcc" );
        case 12: return( "Processor 4 Vcc" );
        case 13: return( "AC Input Voltage" );
        case 14: return( "Memory Controller Hub Vcc" );
        case 15: return( "+3.3 Volt Standby" );
        case 16: return( "Processor Vtt" );
        case 17: return( "+1.8 Volts" );
        case 18: return( "Peripheral Controller Hub (PCH) Vcc" );
        default: return( "Other/Unknown Voltage" );
        }

    case 2:     // Fan Speed Sensors

        switch( iUsage )
        {
        case 1:  return( "Processor Fan" );
        case 2:  return( "Processor Thermal Module Fan" );
        case 3:  return( "Memory Controller Hub Fan" );
        case 4:  return( "Voltage Regulator Fan" );
        case 5:  return( "Chassis Fan" );
        case 6:  return( "Chassis Inlet Fan" );
        case 7:  return( "Chassis Outlet Fan" );
        case 8:  return( "Power Supply Fan" );
        case 9:  return( "Power Supply Inlet Fan" );
        case 10: return( "Power Supply Outlet Fan" );
        case 11: return( "Hard Drive Fan" );
        case 12: return( "Graphics Processor fan" );
        case 13: return( "Auxiliary Fan" );
        case 14: return( "I/O Hub Fan" );
        case 15: return( "Peripheral Controller Hub (PCH) Fan" );
        default: return( "Other/Unknown Fan" );
        }

    case 3:     // Current Sensors

        switch( iUsage )
        {
        case 1:  return( "+12V Current" );
        case 2:  return( "-12V Current" );
        case 3:  return( "+5V Current" );
        case 4:  return( "+5V Backup Current" );
        case 5:  return( "-5V Current" );
        case 6:  return( "+3.3V Current" );
        case 7:  return( "+2.5V Current" );
        case 8:  return( "+1.5V Current" );
        case 9:  return( "Processor Current" );
        case 10: return( "Processor 2 Current" );
        case 11: return( "Processor 3 Current" );
        case 12: return( "Processor 4 Current" );
        case 13: return( "AC Input Current" );
        case 14: return( "Memory Controller Hub Current" );
        case 15: return( "+3.3V Standby Current" );
        case 16: return( "+1.8V Current" );
        case 17: return( "Peripheral Controller Hub (PCH) Current" );
        default: return( "Other/Unknown Current" );
        }
    }

    return( "Other/Unknown Usage" );
}

/****************************************************************************/
/* GetSensorHealth() - Returns health string for specified sensor           */
/****************************************************************************/

function GetSensorHealth( iType, iSensor )
{
    var iStatus = cSensor.GetHealth( iType, iSensor );

    switch( iStatus )
    {
    case 0:  return( "Normal" );
    case 1:  return( "Non-Critical" );
    case 2:  return( "Critical" );
    case 3:  return( "Non-Recoverable" );
    }

    return( "Unknown" );
}

/****************************************************************************/
/* DisplaySensors() - Displays info about each sensor of a specified type   */
/****************************************************************************/

function DisplaySensors( iType )
{
    var iSensors = cSensor.GetCount( iType );

    for( var iSensor = 0; iSensor < iSensors; iSensor++ )
    {
        System.Console.WriteLine( "  {0} Sensor {1}:", GetSensorType( iType ), iSensor );
        System.Console.WriteLine( "" );
        System.Console.WriteLine( "    Health:          {0}", GetSensorHealth( iType, iSensor ) );
        System.Console.WriteLine( "    Usage:           {0}", GetSensorUsage( iType, iSensor ) );

        switch( iType )
        {
        case 0:     // Temperature Sensors

            System.Console.WriteLine( "    Reading:         {0:0.00}", cSensor.GetReading( iType, iSensor ) );
            System.Console.WriteLine( "" );
            System.Console.WriteLine( "    NonCrit:         {0:0.00}", cSensor.GetNonCritHighThresh( iType, iSensor ) );
            System.Console.WriteLine( "    Crit:            {0:0.00}", cSensor.GetCritHighThresh( iType, iSensor ) );
            System.Console.WriteLine( "    NonRecov:        {0:0.00}", cSensor.GetNonRecovHighThresh( iType, iSensor ) );
            break;

        case 1:     // Voltage Sensors

            System.Console.WriteLine( "    Reading:         {0:0.000}", cSensor.GetReading( iType, iSensor ) );
            System.Console.WriteLine( "" );
            System.Console.WriteLine( "    NonCrit Low:     {0:0.000}", cSensor.GetNonCritLowThresh( iType, iSensor ) );
            System.Console.WriteLine( "    Crit Low:        {0:0.000}", cSensor.GetCritLowThresh( iType, iSensor ) );
            System.Console.WriteLine( "    NonRecov Low:    {0:0.000}", cSensor.GetNonRecovLowThresh( iType, iSensor ) );
            System.Console.WriteLine( "" );
            System.Console.WriteLine( "    NonCrit High:    {0:0.000}", cSensor.GetNonCritHighThresh( iType, iSensor ) );
            System.Console.WriteLine( "    Crit High:       {0:0.000}", cSensor.GetCritHighThresh( iType, iSensor ) );
            System.Console.WriteLine( "    NonRecov High:   {0:0.000}", cSensor.GetNonRecovHighThresh( iType, iSensor ) );
            break;

        case 2:     // Fan Speed Sensors

            System.Console.WriteLine( "    Reading:         {0:0.}", cSensor.GetReading( iType, iSensor ) );
            System.Console.WriteLine( "" );
            System.Console.WriteLine( "    NonCrit:         {0:0.}", cSensor.GetNonCritLowThresh( iType, iSensor ) );
            System.Console.WriteLine( "    Crit:            {0:0.}", cSensor.GetCritLowThresh( iType, iSensor ) );
            System.Console.WriteLine( "    NonRecov:        {0:0.}", cSensor.GetNonRecovLowThresh( iType, iSensor ) );
            break;

        case 3:     // Current Sensors

            System.Console.WriteLine( "    Reading:         {0:0.000}", cSensor.GetReading( iType, iSensor ) );
            System.Console.WriteLine( "" );
            System.Console.WriteLine( "    NonCrit Low:     {0:0.000}", cSensor.GetNonCritLowThresh( iType, iSensor ) );
            System.Console.WriteLine( "    Crit Low:        {0:0.000}", cSensor.GetCritLowThresh( iType, iSensor ) );
            System.Console.WriteLine( "    NonRecov Low:    {0:0.000}", cSensor.GetNonRecovLowThresh( iType, iSensor ) );
            System.Console.WriteLine( "" );
            System.Console.WriteLine( "    NonCrit High:    {0:0.000}", cSensor.GetNonCritHighThresh( iType, iSensor ) );
            System.Console.WriteLine( "    Crit High:       {0:0.000}", cSensor.GetCritHighThresh( iType, iSensor ) );
            System.Console.WriteLine( "    NonRecov High:   {0:0.000}", cSensor.GetNonRecovHighThresh( iType, iSensor ) );
            break;
        }

        System.Console.WriteLine( "" );
    }
}

/****************************************************************************/
/* GetControllerUsage() - Returns usage string for specified controller     */
/****************************************************************************/

function GetControllerUsage( iController )
{
    var iUsage = cController.GetFunction( iController );

    switch( iUsage )
    {
    case 1:  return( "Processor Fan Controller" );
    case 2:  return( "Processor Thermal Module Fan Controller" );
    case 3:  return( "Memory Controller Hub Fan Controller" );
    case 4:  return( "Voltage Regulator Fan Controller" );
    case 5:  return( "Chassis Fan Controller" );
    case 6:  return( "Chassis Inlet Fan Controller" );
    case 7:  return( "Chassis Outlet Fan Controller" );
    case 8:  return( "Power Supply Fan Controller" );
    case 9:  return( "Power Supply Inlet Fan Controller" );
    case 10: return( "Power Supply Outlet Fan Controller" );
    case 11: return( "Hard Drive Fan Controller" );
    case 12: return( "Graphics Processor fan Controller" );
    case 13: return( "Auxiliary Fan Controller" );
    case 14: return( "I/O Hub Fan Controller" );
    case 15: return( "Peripheral Controller Hub (PCH) Fan Controller" );
    default: return( "Other/Unknown Fan Controller" );
    }
}

/****************************************************************************/
/* GetControllerHealth() - Returns health string for specified controller   */
/****************************************************************************/

function GetControllerHealth( iController )
{
    var iStatus = cController.GetHealth( iController );

    switch( iStatus )
    {
    case 0:  return( "Normal" );
    case 1:  return( "Non-Critical" );
    case 2:  return( "Critical" );
    case 3:  return( "Non-Recoverable" );
    }

    return( "Unknown" );
}

/****************************************************************************/
/* GetControllerState() - Returns state string for specified controller     */
/****************************************************************************/

function GetControllerState( iController )
{
    var iState = cController.GetState( iController );

    switch( iState )
    {
    case 0:  return( "Automatic" );
    case 1:  return( "Manual" );
    case 2:  return( "Overridden Due to Sensor Error" );
    case 3:  return( "Overridden Due to Controller Error" );
    }
}

/****************************************************************************/
/* DisplayControllers() - Displays info about each controller               */
/****************************************************************************/

function DisplayControllers()
{
    var iControllers = cController.GetCount();

    for( var iController = 0; iController < iControllers; iController++ )
    {
        System.Console.WriteLine( "  Fan Speed Controller {0}:", iController );
        System.Console.WriteLine( "" );
        System.Console.WriteLine( "    Health:          {0}", GetControllerHealth( iController ) );
        System.Console.WriteLine( "    Usage:           {0}", GetControllerUsage( iController ) );
        System.Console.WriteLine( "    Control:         {0}", GetControllerState( iController ) );
        System.Console.WriteLine( "    Duty:            {0:0.00}", cController.GetDuty( iController ) );
        System.Console.WriteLine( "" );
    }
}

/****************************************************************************/
/* Mainline for demo program - Displays Sensor/Controller report similar to */
/* that produced by the QstStat tool.                                       */
/****************************************************************************/

System.Console.WriteLine( "" );
System.Console.WriteLine( "Sensor Configuration/Status Summary:" );
System.Console.WriteLine( "" );

for( var iType = 0; iType < 4; iType++ )
{
    DisplaySensors( iType );
}

System.Console.WriteLine( "Fan Speed Controller Configuration/Status Summary:" );
System.Console.WriteLine( "" );

DisplayControllers();

System.Console.WriteLine( "End of Report" );
System.Console.WriteLine( "" );


