/****************************************************************************/
/*                                                                          */
/*  Module:         SMART.h                                                 */
/*                                                                          */
/*  Description:    Defines the DLL routines that  are  used  to  identify  */
/*                  Hard  Drives that support the Self-Monitoring Analysis  */
/*                  and Reporting Technology (S.M.A.R.T.)  and  to  expose  */
/*                  the failure prediction and drive temperature reporting  */
/*                  capabilities provided by this technology.               */
/*                                                                          */
/*  Usage Model:    The DLL allows you to index your way through  the  set  */
/*                  of drives supporting S.M.A.R.T. and obtain information  */
/*                  about each's S.M.A.R.T. capabilities and  status.  The  */
/*                  indexing sequentially presents the drives that support  */
/*                  S.M.A.R.T.; this is distinct from the physical indexes  */
/*                  for  all  storage volumes enumerated by Windows, since  */
/*                  some  drives  do  not  support  S.M.A.R.T.  and   some  */
/*                  interfaces  (like  USB)  don't support the exposure of  */
/*                  S.M.A.R.T.  data.  Function  SMARTGetDriveIndex()   is  */
/*                  provided  to allow the physical index for a particular  */
/*                  hard drive to be obtained...                            */
/*                                                                          */
/*  Functions:      SMARTGetDriveCapabilities() - Indicates whether  drive  */
/*                      exposes current and worst-case temperatures.        */
/*                                                                          */
/*                  SMARTGetDrivePrediction() - Indicates  if   drive   is  */
/*                      predicting its demise.                              */
/*                                                                          */
/*                  SMARTGetDriveTemperatures() - Gets a  drive's  current  */
/*                      and/or worst-case temperatures.                     */
/*                                                                          */
/*                  SMARTGetDriveInformation() - Gets a drives's Model and  */
/*                      Serial Number string and/or size (in GB).           */
/*                                                                          */
/*                  SMARTGetDriveIndex() - Gets the drive's physical index  */
/*                                                                          */
/*                  SMARTDisplayDriveAttributes() - Debug function formats  */
/*                      output for displaying a drive's attributes. Caller  */
/*                      provides function to perform actual output.         */
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

#ifndef _SMART_H
#define _SMART_H

#include <windows.h>

#ifndef BASIC_DATA_TYPES
#define BASIC_DATA_TYPES
/****************************************************************************/
/* Basic Data Types                                                         */
/****************************************************************************/

typedef unsigned char           BYTE;
typedef unsigned short          WORD;
typedef unsigned long           DWORD;
typedef int                     BOOL;

#define FALSE                   0
#define False                   0
#define false                   0

#define TRUE                    1
#define True                    1
#define true                    1

#endif // ndef BASIC_DATA_TYPES

/****************************************************************************/
/* Definitions                                                              */
/****************************************************************************/

#define SMART_MAX_DRIVES        16      // Maximum drives supported by the DLL

#define SMART_MAX_MODEL         40      // Maximum characters in model number string
                                        // (not counting NULL byte)
#define SMART_MAX_SERIAL        20      // Maximum characters in serial number string
                                        // (not counting NULL byte)

/****************************************************************************/
/* Function Prototypes for implicit DLL loading (static binding)            */
/****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

#ifdef DYNAMIC_DLL_LOADING
BOOL SMARTInitialize( void );
void SMARTCleanup( void );
#endif

BOOL APIENTRY SMARTGetDriveCapabilities(   int iDrive, BOOL *pbCurrentTemp, BOOL *pbWorstTemp         );
BOOL APIENTRY SMARTGetDrivePrediction(     int iDrive, BOOL *pbFailing                                );
BOOL APIENTRY SMARTGetDriveTemperatures(   int iDrive, int *piCurrentTemp, int *piWorstTemp           );
BOOL APIENTRY SMARTGetDriveInformation(    int iDrive, char *pszModel, char *pszSerial, int *piSize   );
BOOL APIENTRY SMARTGetDriveIndex(          int iDrive, int *piIndex                                   );
BOOL APIENTRY SMARTDisplayDriveAttributes( int iDrive, void (APIENTRY *PutStrCallBack)( char *szStr ) );

/****************************************************************************/
/* Function Prototypes for explicit DLL loading                             */
/****************************************************************************/

typedef BOOL (APIENTRY * PFN_SMART_GET_DRIVE_CAPABILITIES)(   int iDrive, BOOL *pbCurrentTemp, BOOL *pbWorstTemp         );
typedef BOOL (APIENTRY * PFN_SMART_GET_DRIVE_PREDICTION)(     int iDrive, BOOL *pbFailing                                );
typedef BOOL (APIENTRY * PFN_SMART_GET_DRIVE_TEMPERATURES)(   int iDrive, int *piCurrentTemp, int *piWorstTemp           );
typedef BOOL (APIENTRY * PFN_SMART_GET_DRIVE_INFORMATION)(    int iDrive, char *pszModel, char *pszSerial, int *piSize   );
typedef BOOL (APIENTRY * PFN_SMART_GET_DRIVE_INDEX)(          int iDrive, int *piIndex                                   );
typedef BOOL (APIENTRY * PFN_SMART_DISPLAY_DRIVE_ATTRIBUTES)( int iDrive, void (APIENTRY *PutStrCallBack)( char *szStr ) );

#ifdef __cplusplus
}
#endif

/****************************************************************************/
/* Function Ordinals for explicit DLL loading                               */
/****************************************************************************/

#define SMART_DLL_NAME                          __TEXT("libSMART.dll")

#define SMART_ORD_GET_DRIVE_CAPABILITIES        1
#define SMART_ORD_GET_DRIVE_PREDICTION          2
#define SMART_ORD_GET_DRIVE_TEMPERATURES        3
#define SMART_ORD_GET_DRIVE_INFORMATION         4
#define SMART_ORD_GET_DRIVE_INDEX               5
#define SMART_ORD_DISPLAY_DRIVE_ATTRIBUTES      6

#endif // ndef _SMART_H

