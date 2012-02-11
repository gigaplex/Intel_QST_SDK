/****************************************************************************/
/*                                                                          */
/*  Module:         INIFile.h                                               */
/*                                                                          */
/*  Description:    Provides definitions and function prototypes  for  the  */
/*                  module that is used to create, delete, read and update  */
/*                  entries in INI files.                                   */
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

#ifndef _INIFILE_H
#define _INIFILE_H

#include "typedef.h"

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************************/
/* CheckINIEntry() - Returns a BOOLEAN indication of whether or not the     */
/* specified entry exists within the INI file.                              */
/****************************************************************************/

BOOL CheckINIEntry
(
    IN  const char       *pszINIPath,        // Pathname for the INI file
    IN  const char       *pszSectionName,    // Section Name for the entry
    IN  const char       *pszEntryName       // Entry Name
);

/****************************************************************************/
/* GetINIEntry() - Gets INI file entry with specified file, section and     */
/* entry names. Returns TRUE if entry found and fits in result buffer.      */
/* Returns FALSE on failure and sets errno as follows:                      */
/*      EEXIST      Entry was not found.                                    */
/*      E2BIG       Entry won't fit in result buffer.                       */
/*      other       File I/O error.                                         */
/****************************************************************************/

BOOL GetINIEntry
(
   IN  const char       *pszINIPath,        // Pathname for the INI file
   IN  const char       *pszSectionName,    // Section Name for the entry
   IN  const char       *pszEntryName,      // Entry Name
   OUT char             *pszEntryText,      // Buffer for Entry text
   IN  int              iEntryTextMax       // Max characters buffer can hold
);

/****************************************************************************/
/* PutINIEntry() - Creates, updates or deletes INI file entries. Setting    */
/* pszEntryText to NULL or "" indicates entry deletion is desired. Returns  */
/* TRUE if successful; FALSE otherwise. During entry creation, the routine  */
/* will automatically create the section header if it doesn't exist. If INI */
/* file doesn't exist, it will be created. File creation always occurs in   */
/* the GLOB_PATH folder.                                                    */
/****************************************************************************/

BOOL PutINIEntry
(
   IN  const char       *pszINIPath,        // Pathname for the INI file
   IN  const char       *pszSectionName,    // Section Name for the entry
   IN  const char       *pszEntryName,      // Entry Name
   IN  const char       *pszEntryText       // Entry text
);

/****************************************************************************/
/* PutINIEntryAbs() - Creates or updates INI file entry. Allows entry to be */
/* created with nothing assigned to it. Returns TRUE if successful; FALSE   */
/* otherwise. During entry creation, the routine will automatically create  */
/* the section header if it doesn't exist. If INI file doesn't exist, it    */
/* will be created. File creation always occurs in the GLOB_PATH folder.    */
/****************************************************************************/

BOOL PutINIEntryAbs
(
   IN  const char       *pszINIPath,        // Pathname for the INI file
   IN  const char       *pszSectionName,    // Section Name for the entry
   IN  const char       *pszEntryName,      // Entry Name
   IN  const char       *pszEntryText       // Entry text
);

/****************************************************************************/
/* CloseINIFile() - Closes most-recently accessed INI file. If the bForce   */
/* parameter is TRUE, it closes the file; clearing its memory cache. Other- */
/* wise, the file will be left open (and cached!) for subsequent searches   */
/* and/or updates...                                                        */
/****************************************************************************/

void CloseINIFile
(
   IN  BOOL             bForce              // Force file closed
);

#ifdef __cplusplus
}
#endif

#endif // ndef _INIFILE_H

