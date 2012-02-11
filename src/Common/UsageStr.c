/****************************************************************************/
/*                                                                          */
/*  Module:         UsageStr.c                                              */
/*                                                                          */
/*  Description:    Implements the routines that provide usage description  */
/*                  strings for sensors/controllers.                        */
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

#include "QstCmd.h"
#include "UsageStr.h"

/****************************************************************************/
/* GetTempUsageStr() - Returns description string for specified temperature */
/* sensor.                                                                  */
/****************************************************************************/

static const char * const pszTempMonStr[] =
{
   "Other/Unknown Temperature",
   "Processor Temperature",
   "Processor Temperature",
   "I/O Controller Hub Temperature",
   "Memory Controller Hub Temperature",
   "Voltage Regulator Temperature",
   "Memory DIMM Temperature",
   "Motherboard Temperature",
   "Ambient Air Temperature",
   "Processor Fan Inlet Air Temperature",
   "System Inlet Air Temperature",
   "System Outlet Air Temperature",
   "Power Supply Internal/Hotspot Temperature",
   "Power Supply Inlet Air Temperature",
   "Power Supply Outlet Air Temperature",
   "Hard Drive Temperature",
   "Graphics Processor Temperature",
   "I/O Hub Temperature",
   "Peripheral Controller Hub (PCH) Temperature"
};

char *GetTempUsageStr( int iIndex )
{
   if( (iIndex >= 0) && (iIndex <= QST_LAST_TEMP_USAGE) )
      return( (char *)pszTempMonStr[iIndex] );
   else
      return( (char *)pszTempMonStr[0] );
}

/****************************************************************************/
/* GetFanUsageStr() - Returns description string for specified fan speed    */
/* sensor.                                                                  */
/****************************************************************************/

static const char * const pszFanMonStr[] =
{
   "Other/Unknown Fan",
   "Processor Fan",
   "Processor Thermal Module Fan",
   "Memory Controller Hub Fan",
   "Voltage Regulator Fan",
   "Chassis Fan",
   "Chassis Inlet Fan",
   "Chassis Outlet Fan",
   "Power Supply Fan",
   "Power Supply Inlet Fan",
   "Power Supply Outlet Fan",
   "Hard Drive Fan",
   "Graphics Processor fan",
   "Auxiliary Fan",
   "I/O Hub Fan",
   "Peripheral Controller Hub (PCH) Fan",
   "Memory Fan",
};

char *GetFanUsageStr( int iIndex )
{
   if( (iIndex >= 0) && (iIndex <= QST_LAST_FAN_USAGE) )
      return( (char *)pszFanMonStr[iIndex] );
   else
      return( (char *)pszFanMonStr[0] );
}

/****************************************************************************/
/* GetCtrlUsageStr() - Returns description string for specified fan speed   */
/* controller.                                                              */
/****************************************************************************/

static const char * const pszFanCtrlStr[] =
{
   "Other/Unknown Fan Controller",
   "Processor Fan Controller",
   "Processor Thermal Module Fan Controller",
   "Memory Controller Hub Fan Controller",
   "Voltage Regulator Fan Controller",
   "Chassis Fan Controller",
   "Chassis Inlet Fan Controller",
   "Chassis Outlet Fan Controller",
   "Power Supply Fan Controller",
   "Power Supply Inlet Fan Controller",
   "Power Supply Outlet Fan Controller",
   "Hard Drive Fan Controller",
   "Graphics Processor fan Controller",
   "Auxiliary Fan Controller",
   "I/O Hub Fan Controller",
   "Peripheral Controller Hub (PCH) Fan Controller",
   "Memory Fan Controller",
};

char *GetCtrlUsageStr( int iIndex )
{
   if( (iIndex >= 0) && (iIndex <= QST_LAST_FAN_USAGE) )
      return( (char *)pszFanCtrlStr[iIndex] );
   else
      return( (char *)pszFanCtrlStr[0] );
}

/****************************************************************************/
/* GetVoltUsageStr() - Returns description string for specified voltage     */
/* sensor.                                                                  */
/****************************************************************************/

static const char * const pszVoltMonStr[] =
{
   "Other/Unknown Voltage",
   "+12 Volts",
   "-12 Volts",
   "+5 Volts",
   "+5 Volt Backup",
   "-5 Volts",
   "+3.3 Volts",
   "+2.5 Volts",
   "+1.5 Volts",
   "Processor Vcc",
   "Processor 2 Vcc",
   "Processor 3 Vcc",
   "Processor 4 Vcc",
   "AC Input Voltage",
   "Memory Controller Hub Vcc",
   "+3.3 Volt Standby",
   "Processor Vtt",
   "+1.8 Volts",
   "Peripheral Controller Hub (PCH) Vcc",
   "SDRAM Vcc",
};

char *GetVoltUsageStr( int iIndex )
{
   if( (iIndex >= 0) && (iIndex <= QST_LAST_VOLT_USAGE) )
      return( (char *)pszVoltMonStr[iIndex] );
   else
      return( (char *)pszVoltMonStr[0] );
}

/****************************************************************************/
/* GetCurrUsageStr() - Returns description string for specified current     */
/* sensor.                                                                  */
/****************************************************************************/

static const char * const pszCurrMonStr[] =
{
   "Other/Unknown Current",
   "+12V Current",
   "-12V Current",
   "+5V Current",
   "+5V Backup Current",
   "-5V Current",
   "+3.3V Current",
   "+2.5V Current",
   "+1.5V Current",
   "Processor Current",
   "Processor 2 Current",
   "Processor 3 Current",
   "Processor 4 Current",
   "AC Input Current",
   "Memory Controller Hub Current",
   "+3.3V Standby Current",
   "+1.8V Current",
   "Peripheral Controller Hub (PCH) Current",
   "SDRAM Current",
};

char *GetCurrUsageStr( int iIndex )
{
   if( iIndex <= QST_LAST_CURR_USAGE )
      return( (char *)pszCurrMonStr[iIndex] );
   else
      return( (char *)pszCurrMonStr[0] );
}

