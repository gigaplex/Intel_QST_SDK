/****************************************************************************/
/*                                                                          */
/*  Module:         InstTest.c                                              */
/*                                                                          */
/*  Description:    Implements sample program InstTest, which demonstrates  */
/*                  how  to  utilize  the  services of the Instrumentation  */
/*                  Layer (IL) for Intel(R) Quiet System Technology (QST).  */
/*                                                                          */
/*  Notes:      1.  The program runs as a console  application.  Once  per  */
/*                  second,  it collects and displays Fan Speed Controller  */
/*                  Duty Cycle Settings  and  Temperature  and  Fan  Speed  */
/*                  Sensor  Readings.  The  program is terminated when the  */
/*                  <enter> key is pressed. Before exiting, a  display  of  */
/*                  minimum,  maximum  and  average  readings/settings for  */
/*                  each of the Sensors/Controllers is displayed.           */
/*                                                                          */
/*              2.  The  source  code  contains  conditional  support  for  */
/*                  building  executables  for  DOS, Windows and Linux. In  */
/*                  the case of Windows, conditional support  is  provided  */
/*                  for  static (loadtime) or dynamic (runtime) binding of  */
/*                  the QST IL DLL. Currently, the Visual  Studio  project  */
/*                  for  this  program  is  set up for dynamic binding. To  */
/*                  utilize dynamic binding, which allows the  program  to  */
/*                  determine  if  and  when  the DLL is to be loaded into  */
/*                  memory, modify the  project  to  include  source  file  */
/*                  ..\Support\QstInst.c and symbol DYNAMIC_DLL_LOADING.    */
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
#include <ctype.h>
#include <time.h>
#include <errno.h>

#if defined(__WIN32__) || defined(_WIN32) || defined(_WIN64) || defined(__WINDOWS__)
#ifndef __WIN32__
#define __WIN32__
#endif
#include <windows.h>
#include <tchar.h>
#include <conio.h>
#elif defined(__SOLARIS__) || defined(__sun__)
#ifndef __SOLARIS__
#define __SOLARIS__
#endif
#include <sys/select.h>
#elif defined(__LINUX__) || defined(__linux__)
#ifndef __LINUX__
#define __LINUX__
#endif
#include <sys/select.h>
#elif defined(__MSDOS__) || defined(MSDOS) || defined(_MSDOS) || defined(__DOS__)
#ifndef __MSDOS__
#define __MSDOS__
#endif
#include <conio.h>
#endif

#include "QstInst.h"

/****************************************************************************/
/* Literals                                                                 */
/****************************************************************************/

#define TEMP_READING        0
#define FAN_READING         1
#define DUTY_SETTING        2

#define MONITOR_TYPES       3

/****************************************************************************/
/* Variables                                                                */
/****************************************************************************/

static int                  iTempReadings,
                            iFanReadings,
                            iDutySettings;
static float                fTempReading,
                            fTempReadingMax[QST_MAX_TEMPERATURE_SENSORS],
                            fTempReadingMin[QST_MAX_TEMPERATURE_SENSORS],
                            fTempReadingTotal[QST_MAX_TEMPERATURE_SENSORS],

                            fFanReading,
                            fFanReadingMax[QST_MAX_FAN_SPEED_SENSORS],
                            fFanReadingMin[QST_MAX_FAN_SPEED_SENSORS],
                            fFanReadingTotal[QST_MAX_FAN_SPEED_SENSORS],

                            fDutySetting,
                            fDutySettingMax[QST_MAX_FAN_SPEED_CONTROLLERS],
                            fDutySettingMin[QST_MAX_FAN_SPEED_CONTROLLERS],
                            fDutySettingTotal[QST_MAX_FAN_SPEED_CONTROLLERS];

static DWORD                dwCount;

static time_t               tStart;

static char                 szInput[128],
                            szLine[128],
                            szMessage[128];

/****************************************************************************/
/* Constants                                                                */
/****************************************************************************/

static const char * const   szDevice[MONITOR_TYPES] =
{
   "Temperature Sensor",
   "Fan Speed Sensor",
   "Fan Speed Controller"
};

/****************************************************************************/
/* Delay() - Implements an 'n' millisecond delay.                           */
/****************************************************************************/

static void Delay( int iMilliseconds )
{
   if( iMilliseconds )
   {

#if defined(__WIN32__)

      Sleep( iMilliseconds );

#elif defined(__MSDOS__)

#if (CLOCKS_PER_SEC != 1000)
#error clock() implementation does not support millisecond resolution!!
#endif

      clock_t tStart, tCurrent, tEnd;

      tCurrent = tStart = clock();
      tEnd = tStart + (clock_t)iMilliseconds;

      // Handle wraparound

      if( tEnd < tCurrent )
      {
         while( tCurrent )
            tCurrent = clock();
      }

      // Complete the delay

      while( tCurrent < tEnd )
         tCurrent = clock();

#elif defined(__LINUX__) || defined(__SOLARIS__)

      struct timespec stTime;

      stTime.tv_sec   = (time_t)(iMilliseconds / 1000);
      stTime.tv_nsec  = 1000000L * (iMilliseconds % 1000);    // 1,000,000 ns = 1 ms

      while( (nanosleep( &stTime, &stTime ) == -1) && (errno == EINTR) );

#endif

   }
}

#if defined(__LINUX__) || defined(__SOLARIS__)
/****************************************************************************/
/* kbhit() - Checks the console for keyboard input. This is a standard      */
/* function in DOS and Windows libraries, but we need to implement our own  */
/* for Linux                                                                */
/****************************************************************************/

int kbhit( void )
{
   struct timeval stTime = { 0, 0 };
   int            iFileStdin = fileno( stdin );
   fd_set         stRead;

   FD_ZERO( &stRead );
   FD_SET( iFileStdin, &stRead );

   // Check if input availability is being signalled

   if( select( iFileStdin + 1, &stRead, NULL, NULL, &stTime ) == -1 )
      return( FALSE );

   // Return indication of whether this input is from our stdin

   return( FD_ISSET( iFileStdin, &stRead ) );
}

#endif

/****************************************************************************/
/* Cleanup() - Cleans up for exit.                                          */
/****************************************************************************/

static void Cleanup( void )
{
   // Terminate when user is ready (presses <enter> key)

   printf( "\n\nPress <enter> to terminate program..." );

   tStart = time( NULL );
   (void)fgets( szInput, 80, stdin );

   if( tStart == time( NULL ) )
      (void)fgets( szInput, 80, stdin );

   // Cleanup

#if defined(DYNAMIC_DLL_LOADING) || defined(__MSDOS__)
   QstInstCleanup();
#endif

}

/****************************************************************************/
/* DisplayError() - Displays an appropriate error message.                  */
/****************************************************************************/

static void DisplayError( char *pszMessage, BOOL bInErrno )
{
   printf( "\n*** %s!!\n", pszMessage );

   if( bInErrno )
      printf( "    errno = %d (%s)!!\n", pszMessage, errno, strerror(errno) );
   else
   {

#if defined(__WIN32__)

      char   *pszMsgBuf;
      size_t tLen;
      DWORD  dwError = GetLastError();

      FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                     NULL, dwError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&pszMsgBuf, 0, NULL );

      if( (tLen = strlen(pszMsgBuf)) > 2 )
      {
         if( iscntrl(pszMsgBuf[tLen - 2]) )
            pszMsgBuf[tLen - 2] = '\0';
         else if( iscntrl(pszMsgBuf[tLen - 1]) )
            pszMsgBuf[tLen - 1] = '\0';

         printf( "   Windows Error = 0x%08X (%s)\n", dwError, pszMsgBuf );
      }
      else
         printf( "   Windows Error = 0x%08X (Unknown Error)\n", dwError );

      LocalFree( pszMsgBuf );

#endif

   }

   putchar( '\n' );
}

/****************************************************************************/
/* DisplayValue() - Displays a sensor's reading using field format          */
/* definition appropriate for the type of reading/setting.                  */
/****************************************************************************/

static void DisplayValue( int iType, int iValue )
{
   switch( iType )
   {
   case TEMP_READING:

      printf( "%3d ", iValue );
      break;

   case FAN_READING:

      printf( " %04d ", (iValue > 9999)? 9999 : iValue );
      break;

   case DUTY_SETTING:

      printf( " %02d ", (iValue > 99)? 99 : iValue );
      break;
   }
}

/****************************************************************************/
/* GetValue() - Gets a reading/setting from the specified sensor/controller */
/****************************************************************************/

static float GetValue( int iType, int iIndex )
{
   float fValue = 0;

   switch( iType )
   {
   case TEMP_READING:

      if( !QstGetSensorReading( TEMPERATURE_SENSOR, iIndex, &fValue ) )
         fValue = -1;

      break;

   case FAN_READING:

      if( !QstGetSensorReading( FAN_SPEED_SENSOR, iIndex, &fValue ) )
         fValue = -1;

      break;

   case DUTY_SETTING:

      if( !QstGetControllerDutyCycle( iIndex, &fValue ) )
         fValue = -1;

      break;
   }

   if( fValue == -1 )
   {
      sprintf( szMessage, "Unable to obtain reading from %s %d", szDevice[iType], iIndex );
      DisplayError( szMessage, FALSE );
      Cleanup();
      exit( 1 );
   }

   return( fValue );
}

/****************************************************************************/
/* main() - Mainline for the application                                    */
/****************************************************************************/

int main( int iArgs, char *pszArg[] )
{
   int iIndex;

   puts( "\nIntel(R) Quiet System Technology Instrumentation Layer Demo" );
   puts( "Copyright (C) 2007-2009, Intel Corporation. All Rights Reserved.\n" );

   // Initialize Readings Support

#if defined(DYNAMIC_DLL_LOADING) || defined(__MSDOS__)

   if( !QstInstInitialize() )
   {
      DisplayError( "Unable to Initialize Monitoring Support", FALSE );
      return( -1 );
   }

#endif

   QstGetSensorCount( TEMPERATURE_SENSOR, &iTempReadings );
   QstGetSensorCount( FAN_SPEED_SENSOR, &iFanReadings );
   QstGetControllerCount( &iDutySettings );

   // Initialize Statistics Support and data display

#if defined(__LINUX__) || defined(__SOLARIS__)
   printf( "Press <enter> to terminate monitoring and display statistics...\n\n        " );
#else
   printf( "Press any key to terminate monitoring and display statistics...\n\n        " );
#endif

   strcpy( szLine,  "--------" );

   // ...for Temperature Sensors

   for( iIndex = 0; iIndex < iTempReadings; iIndex++ )
   {
      fTempReadingMax[iIndex]     = -200;
      fTempReadingMin[iIndex]     =  200;
      fTempReadingTotal[iIndex]   =  0;

      strcat( szLine, "----" );
      printf( (iIndex >= 9)? "T%d " : " T%d ", iIndex + 1 );
   }

   // For Fan Speed Sensors

   for( iIndex = 0; iIndex < iFanReadings; iIndex++ )
   {
      fFanReadingMax[iIndex]      = 0;
      fFanReadingMin[iIndex]      = 50000;
      fFanReadingTotal[iIndex]    = 0;

      strcat( szLine, "------" );
      printf( (iIndex >= 9)? " F%d  " : "  F%d  ", iIndex + 1 );
   }

   // For Fan Speed Controllers

   for( iIndex = 0; iIndex < iDutySettings; iIndex++ )
   {
      fDutySettingMax[iIndex]     = 0;
      fDutySettingMin[iIndex]     = 101;
      fDutySettingTotal[iIndex]   = 0;

      strcat( szLine, "----" );
      printf( (iIndex >= 9)? "P%d " : " P%d ", iIndex + 1 );
   }

   printf( "\n%s\n", szLine );

   // display (and log, if selected) data until told to stop

   dwCount = 0;

   for( ; ; )
   {
      // Terminate sampling if a key is pressed

      if( kbhit() )
         break;

      // Display/Log sequence information

      ++dwCount;
      fputs( "\rCurrent ", stdout );

      // Display Temperatures

      for( iIndex = 0; iIndex < iTempReadings; iIndex++ )
      {
         fTempReading = GetValue( TEMP_READING, iIndex );

         if( fTempReading > fTempReadingMax[iIndex] )
            fTempReadingMax[iIndex] = fTempReading;

         if( fTempReading < fTempReadingMin[iIndex] )
            fTempReadingMin[iIndex] = fTempReading;

         fTempReadingTotal[iIndex] += fTempReading;

         DisplayValue( TEMP_READING, (int)fTempReading );
      }

      // Display Fan Speeds

      for( iIndex = 0; iIndex < iFanReadings; iIndex++ )
      {
         fFanReading = GetValue( FAN_READING, iIndex );

         if( fFanReading > fFanReadingMax[iIndex] )
            fFanReadingMax[iIndex] = fFanReading;

         if( fFanReading < fFanReadingMin[iIndex] )
            fFanReadingMin[iIndex] = fFanReading;

         fFanReadingTotal[iIndex] += fFanReading;

         DisplayValue( FAN_READING, (int)fFanReading );
      }

      // Display Duty Cycles

      for( iIndex = 0; iIndex < iDutySettings; iIndex++ )
      {
         fDutySetting = GetValue( DUTY_SETTING, iIndex );

         if( fDutySetting > fDutySettingMax[iIndex] )
            fDutySettingMax[iIndex] = fDutySetting;

         if( fDutySetting < fDutySettingMin[iIndex] )
            fDutySettingMin[iIndex] = fDutySetting;

         fDutySettingTotal[iIndex] += fDutySetting;

         DisplayValue( DUTY_SETTING, (int)fDutySetting );
      }

      // Flush display

      fflush( stdout );

      // Wait a sec

      Delay( 1 );
   }

   // Display statistics

   printf( "\n%s\nMinimum ", szLine );

   for( iIndex = 0; iIndex < iTempReadings; iIndex++ )
      DisplayValue( TEMP_READING, (int)fTempReadingMin[iIndex] );

   for( iIndex = 0; iIndex < iFanReadings; iIndex++ )
   {
      if( fFanReadingMin[iIndex] == 50000 )
         fFanReadingMin[iIndex] = 0;

      DisplayValue( FAN_READING, (int)fFanReadingMin[iIndex] );
   }

   for( iIndex = 0; iIndex < iDutySettings; iIndex++ )
      DisplayValue( DUTY_SETTING, (int)fDutySettingMin[iIndex] );

   printf( "\nAverage " );

   for( iIndex = 0; iIndex < iTempReadings; iIndex++ )
      DisplayValue( TEMP_READING, (int)(fTempReadingTotal[iIndex] / dwCount) );

   for( iIndex = 0; iIndex < iFanReadings; iIndex++ )
      DisplayValue( FAN_READING, (int)(fFanReadingTotal[iIndex] / dwCount) );

   for( iIndex = 0; iIndex < iDutySettings; iIndex++ )
      DisplayValue( DUTY_SETTING, (int)(fDutySettingTotal[iIndex] / dwCount) );

   printf( "\nMaximum " );

   for( iIndex = 0; iIndex < iTempReadings; iIndex++ )
      DisplayValue( TEMP_READING, (int)fTempReadingMax[iIndex] );

   for( iIndex = 0; iIndex < iFanReadings; iIndex++ )
      DisplayValue( FAN_READING, (int)fFanReadingMax[iIndex] );

   for( iIndex = 0; iIndex < iDutySettings; iIndex++ )
      DisplayValue( DUTY_SETTING, (int)fDutySettingMax[iIndex] );

   // Terminate when user is ready (presses <enter> key)

   Cleanup();
   return( 0 );

#if _MSC_VER > 1000
   UNREFERENCED_PARAMETER( iArgs );
   UNREFERENCED_PARAMETER( pszArg );
#endif

}

