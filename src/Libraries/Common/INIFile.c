/****************************************************************************/
/*                                                                          */
/*  Module:         INIFile.c                                               */
/*                                                                          */
/*  Description:    Implements the functions used to create, delete,  read  */
/*                  and update entries in INI files.                        */
/*                                                                          */
/*  Notes:      1.  This module is not reentrant and is  thus  not  thread  */
/*                  safe.  You can only use it serially on one INI file at  */
/*                  a time...                                               */
/*                                                                          */
/*              2.  The API provided by the module is as follows:           */
/*                                                                          */
/*                  CheckINIEntry   Returns a boolean  indicating  whether  */
/*                                  or  not  the specified entry exists in  */
/*                                  the specified Section.                  */
/*                                                                          */
/*                  GetINIEntry     Returns  the  text  of  the  specified  */
/*                                  Entry in the specified Section.         */
/*                                                                          */
/*                  PutINIEntry     Creates,  updates   or   deletes   the  */
/*                                  specified   Entry   in  the  specified  */
/*                                  Section. Deletion is indicated  by  an  */
/*                                  empty  text  assignment string. If not  */
/*                                  deleting, creates the  Section  and/or  */
/*                                  Entry if they do not exist.             */
/*                                                                          */
/*                  PutINIEntryAbs  Creates or updates the specified Entry  */
/*                                  in  the specified Section. Supports an  */
/*                                  entry  being  created  with  no   text  */
/*                                  assignment. Creates the Section and/or  */
/*                                  Entry if they do not exist.             */
/*                                                                          */
/*                  CloseINIFile    This function is normally only  called  */
/*                                  internal  to  the  module.  It  can be  */
/*                                  invoked externally  (with  its  bForce  */
/*                                  parameter  set  to  TRUE), to unbuffer  */
/*                                  the last-accessed INI file,  but  only  */
/*                                  in   very   close   proximity  to  the  */
/*                                  invocation of the other  functions  to  */
/*                                  access/update the INI file's contents.  */
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

/****************************************************************************/
/*                        Notes on INI File Handling                        */
/*                                                                          */
/*  1.  INI Files are made up of some number of Sections, each  containing  */
/*      some number of Entries.                                             */
/*                                                                          */
/*  2.  Blank lines may be inserted anywhere in the file.  These lines are  */
/*      ignored.                                                            */
/*                                                                          */
/*  3.  A semicolon (';')  indicates  the  beginning  of  a  comment.  All  */
/*      remaining  text  in the line will be ignored. Semicolons cannot be  */
/*      used in Section or Entry names. If a semicolon is necessary within  */
/*      an Entry's  Text,  all  or  part  of  the  Entry's  Text  must  be  */
/*      encapsulated  in quotes. PutINIEntry() will encapsulate the entire  */
/*      Entry's text if a raw semicolon is included within the string.  If  */
/*      an  Entry's  text  is  entirely  encapsulated within quotes, these  */
/*      quotes will be removed before text is returned by GetINIEntry().    */
/*                                                                          */
/*  4.  Section Names, Entry Names and Entry Text may  contain  whitespace  */
/*      characters.  No  quoting  is  necessary  if they do - unless it is  */
/*      important to retain leading and/or trailing whitespace characters.  */
/*                                                                          */
/*  5.  Syntax used within INI files is summarized as follows:              */
/*                                                                          */
/*      Section ::=                                                         */
/*                                                                          */
/*        [ ]* '[' [ ]* <SectionName> [ ]* ']' [<Ignored>]                  */
/*                                                                          */
/*      Entry ::=                                                           */
/*                                                                          */
/*        [ ]* <EntryName> [ ]* '=' [ ]* <EntryText> [<Ignored>]            */
/*                                                                          */
/*      Ignored ::=                                                         */
/*                                                                          */
/*        [ ]* [';' <Comment>] <EOL>                                        */
/*                                                                          */
/****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#if _MSC_VER > 1000
#pragma once
#pragma warning( disable: 4310 )    // Ignore truncation of constant values
#pragma warning( disable: 4706 )    // Ignore assignment in conditional expression
#pragma warning( disable: 4142 )    // Ignore equivalent redefinitions
#endif

#include "typedef.h"
#include "INIFile.h"
#include "BFileIO.h"

/****************************************************************************/
/* Simplify subsequent conditional compilation statements                   */
/****************************************************************************/

#if !defined(__MSDOS__) && (defined(MSDOS) || defined(_MSDOS) || defined(__DOS__))
#define __MSDOS__
#endif

#if !defined(__SOLARIS__) && defined(__sun__)
#define __SOLARIS__
#endif

#if !defined(__LINUX__) && defined(__linux__)
#define __LINUX__
#endif

#if !defined(__WIN32__) && (defined(_WIN32) || defined(_WIN64) || defined(__WINDOWS__))
#define __WIN32__
#endif

/****************************************************************************/
/* Environmental Declarations                                               */
/****************************************************************************/

#if defined(__LINUX__) || defined(__SOLARIS__)

#include <unistd.h>
typedef  struct stat    FILE_STAT;                  // File Status structure
#define  STAT_FUNC      stat                        // File Status function
#define  GLOB_PATH      "/etc/"                     // Global location for INI files on Linux
#define  stricmp        strcasecmp                  // Linux names function differently

#elif defined(__WIN32__)

#include <io.h>
typedef  struct _stat   FILE_STAT;                  // File Status structure
#define  STAT_FUNC      _stat                       // File Status function
#define  GLOB_PATH      "c:\\Windows\\"             // Global location for INI files on Windows

#elif defined(__MSDOS__)

#include <io.h>
typedef  struct _stat   FILE_STAT;                  // File Status structure
#define  STAT_FUNC      _stat                       // File Status function
#define  GLOB_PATH      ""                          // Global location for INI files on DOS

#else
#error Execution Environment Unsupported!!
#endif

/****************************************************************************/
/* Buffer Size Declarations                                                 */
/****************************************************************************/

#define MAX_PATH_LEN    255                         // Max characters in pathname
#define MAX_PATH_LEN0   (MAX_PATH_LEN + 1)          //   room for NULL byte

#define MAX_LINE_LEN    255                         // Max characters in file line
#define MAX_LINE_LEN0   (MAX_LINE_LEN + 1)          //   room for NULL byte

#define MAX_NAME_LEN    64                          // Max characters in field (section/entry) name
#define MAX_NAME_LEN0   (MAX_NAME_LEN + 1)          //   room for NULL byte

/****************************************************************************/
/* Global Variables                                                         */
/****************************************************************************/

static  BOOL            bINIOpen = FALSE;           // Indicates if INI file open
static  BFILE           *pBFINIOpen;                // Buffer for open INI file handle
static  char            szINIOpen[MAX_PATH_LEN0];   // Buffer for open INI file pathname

/****************************************************************************/
/* Trim() - Removes leading & trailing whitespace characters from the       */
/* specified string. Can handle being passed no string.                     */
/****************************************************************************/

static void Trim
(
   IO  char          *pszString             // String to be trimmed
){
   // Only need to continue if string has something in it...

   if( pszString && *pszString )
   {
      // Remove trailing whitespace characters

      int iIndex = strlen(pszString) - 1;

      while( iIndex && isspace(pszString[iIndex]) )
         pszString[iIndex--] = '\0';

      // Remove leading whitespace characters

      iIndex = 0;

      while( isspace(pszString[iIndex]) )
         iIndex++;

      if( iIndex )
         strcpy( pszString, &pszString[iIndex] );
   }
}

/****************************************************************************/
/* TrimDotINI() - Removes leading & trailing whitespace characters from the */
/* specified INI file pathname and, if it was included, removes the         */
/* trailing ".ini" file type designator.                                    */
/****************************************************************************/

static void TrimDotINI
(
   IO  char          *pszFilePath           // String to have ".INI" removed from it
){
   int               iDot;                  // pointer to where ".INI" appears in string

   Trim( pszFilePath );

   iDot = strlen(pszFilePath) - 4;

   if( !stricmp( &pszFilePath[iDot], ".ini" ) )
      pszFilePath[iDot] = '\0';
}

/****************************************************************************/
/* UnQuote() - Removes leading & trailing quote characters from the         */
/* specified string, provided they are the only quote characters present.   */
/****************************************************************************/

static void UnQuote
(
   IO  char          *pszString             // String to be unquoted
){
   int               iLast;                 // Offset of last char in string
   int               iQuotes;               // Count of quote characters found
   char              chCurr;                // Current character from string
   char              *pszSearch;            // Pointer into string

   Trim( pszString );

   // Only need to continue if trimmed string isn't empty...

   if( pszString && *pszString )
   {
      iLast = strlen(pszString) - 1;

      // Only need to continue is surrounded by quote characters...

      if( (pszString[0] == '\"') && (pszString[iLast] == '\"') )
      {
         iQuotes   = 0;
         pszSearch = pszString;

         // Count quote characters in string

         while( (chCurr = *pszSearch++) )
         {
            if( chCurr == '\"' )
               ++iQuotes;
         }

         // If there's only 2, can strip them off

         if( iQuotes == 2 )
         {
            pszString[iLast] = '\0';
            strcpy( pszString, &pszString[1] );
         }
      }
   }
}

/****************************************************************************/
/* RawSemi() - Returns a indication of whether or not the specified string  */
/* contains a semicolon that is not all or in part encapsulated within      */
/* quotes. The value returned is the index of this semicolon. If none is    */
/* found, 0 (zero) will be returned (so can use in BOOLEAN tests)...        */
/****************************************************************************/

static int RawSemi
(
   IO  char          *pszString             // String to be searched
){
   int               iIndex;                // Offset into string
   BOOL              bQuoted = FALSE;       // Indicates if currently in quoted [sub]string

   for( iIndex = 0; pszString[iIndex]; iIndex++ )
   {
      if( pszString[iIndex] == '\"' )
         bQuoted = !bQuoted;
      else
         if( !bQuoted && (pszString[iIndex] == ';') )
            return( iIndex );
   }

   return( 0 );
}

/****************************************************************************/
/* NextToken() - Parses the next string token in the INI file. See notes on */
/* parsing INI files earlier in this module for more information on how     */
/* this is done. If a target buffer is specified, the string token is       */
/* copied to this buffer. If no buffer is specified, the call still has the */
/* affect of skipping over a token. Source and target buffers may be the    */
/* same (allowing parsing in place). Returns a pointer to the remainder of  */
/* the line if a token remains (entry text, presumably) or NULL if no       */
/* content remains (or is a comment).                                       */
/****************************************************************************/

static char *NextToken
(
   IN  char          *pszLine,              // String to be tokenized
   OUT char          *pszToken,             // Buffer to save token string
   IN  int           iTokenMax              // Amount of room in the buffer
){
   BOOL              bQuoted = FALSE;       // Indicates if currently within a quoted [sub]string

   if( pszLine )
   {
      // Allow for NULL byte

      if( pszToken )
         iTokenMax--;

      // Process entire line

      while( *pszLine )
      {
         // Bare equal sign terminates entry name

         if( !bQuoted && (*pszLine == '=') )
         {
            pszLine++;
            break;
         }

         // Bare semi-colon terminates entry text (comment is discarded)

         if( !bQuoted && (*pszLine == ';') )
         {
            pszLine = NULL;
            break;
         }

         // Double-Quote brings us into or out of a quoted string (we discard the quote sign)

         if( *pszLine == '\"' )
         {
            bQuoted = !bQuoted;
            pszLine++;
            continue;
         }

         // Copy char to token buffer if possible and prepare to process next char

         if( pszToken && iTokenMax && *pszLine )
         {
            *pszToken++ = *pszLine++;
            iTokenMax--;
         }
         else
            pszLine++;
      }

      // Terminate token string

      if( pszToken )
         *pszToken = '\0';
   }

   // return pointer to beginning of next token or NULL if done with buffer

   return( ( pszLine && *pszLine )? pszLine : NULL );
}

/****************************************************************************/
/* FindINIEntry() - Finds the specified entry in an INI file and returns    */
/* its entry text (if a buffer is provided). Implementation presumes that   */
/* the correct section header has already been found and file pointer is    */
/* beyond this header. Search for entry is terminated at EOF or if another  */
/* section header is encountered. Returns TRUE if entry is found; FALSE     */
/* otherwise. Sets errno to EEXIST if entry is not found or E2BIG if entry  */
/* text won't fit in caller's buffer. If a variable is specified, the       */
/* position of the beginning of the matched entry (line) in the file is put */
/* into this variable (this is used to support the update of entries).      */
/****************************************************************************/

static BOOL FindINIEntry
(
   IN  BFILE         *pBFINIFile,           // Handle for open INI file
   IN  char          *szEntryName,          // Entry Name we are searching for
   OUT char          *pszEntryText,         // Buffer for returning entry text
   IN  int           iEntryTextMax,         // Amount of room in the buffer
   OUT long          *plFilePos             // Receives file position of matching line
){
   char              szLine[MAX_LINE_LEN0]; // Buffer for lines from the file
   char              *pszNext;              // Pointer for search result
   int               iLen;                  // Length of string in line buffer
   int               iSemi;                 // Count of semicolons found in line

   // Keep trying until we hit EOF

   while( !bfeof( pBFINIFile ) )
   {
      // Save file position if desired

      if ( plFilePos )
         *plFilePos = bftell( pBFINIFile );

      // Get a line from the INI file

      if( bfgets( szLine, MAX_LINE_LEN, pBFINIFile ) )
      {
         // Strip off any trailing control characters

         iLen = (int)(strlen(szLine) - 1);

         if( (szLine[iLen] == '\r') || (szLine[iLen] == '\n') )
            szLine[iLen] = '\0';

         // Strip off any leading/trailing whitespace characters

         Trim( szLine );

         // Isolate first token in line

         pszNext = NextToken( szLine, szLine, MAX_LINE_LEN );

         // If we've found a section header, the entry doesn't exist

         if( *szLine == '[' )
            break;

         // Otherwise, we have an entry name...

         if( *szLine )
         {
            // check if its the right one

            Trim( szLine );

            if( !stricmp( szLine, szEntryName ) )
            {
               // right entry; save off entry text if desired

               if( pszEntryText )
               {
                  // Save off text if there is any

                  if( pszNext )
                  {
                     // Strip off any trailing comment

                     if( (iSemi = RawSemi( pszNext )) )
                        pszNext[iSemi] = '\0';

                     // Remove quotes if bracketing entire text

                     UnQuote( pszNext );

                     // If text will fit in user's buffer, copy it there

                     if( iEntryTextMax > (int)strlen( pszNext ) )
                     {
                         strcpy( pszEntryText, pszNext );
                     }

                     // Otherwise, tell them it won't fit

                     else
                     {
                        errno = E2BIG;
                        return( FALSE );
                     }
                  }

                  // No entry text; give them NULL string

                  else
                     *pszEntryText = '\0';
               }

               // Tell caller entry found

               return( TRUE );
            }
         }
      }
   }

   // Indicate there's no match

   if( plFilePos )
      *plFilePos = 0;

   // Tell caller entry not found

   errno = EEXIST;
   return( FALSE );
}

/****************************************************************************/
/* FindINISection() - Finds the specified section (or, if pszSection is     */
/* specified as NULL, the next section (if one exists)) in an INI file.     */
/* Starts search from current file position. Returns TRUE if found, FALSE   */
/* otherwise (and sets errno to EEXIST, in this case). If a variable is     */
/* specified, the position within the file of the beginning of the line     */
/* that contains the section header is put into this variable (this is used */
/* to support the update of entries).                                       */
/****************************************************************************/

static BOOL FindINISection
(
   IN  BFILE         *pBFINIFile,           // Handle for open INI file
   IN  char          *pszSectionName,       // Section Name for the entry
   OUT long          *plFilePos             // Receives file position of matching line
){
   char              szLine[MAX_LINE_LEN0]; // Buffer for lines from the file
   char              *pszTerm;              // Search result pointer
   long              lFilePos = 0;          // Current file position
   BOOL              bFound = FALSE;        // Indicates of entry was found

   // keep trying until we hit EOF

   while( !bfeof( pBFINIFile ) )
   {
      // save file position

      lFilePos = bftell( pBFINIFile );

      // Get a line from the file

      if( bfgets( szLine, MAX_LINE_LEN, pBFINIFile ) != NULL )
      {
         // pull off the first token

         NextToken( szLine, szLine, MAX_LINE_LEN );
         Trim( szLine );

         // process if it's a Section header

         if( *szLine == '[' )
         {
            // we're done if they want any old Section

            if( !pszSectionName )
            {
               bFound = TRUE;
               break;
            }

            // strip off closing bracket

            pszTerm = strstr( szLine, "]" );

            if( pszTerm )
               *pszTerm = '\0';

            // remove whitespace around name

            Trim( szLine + 1 );

            // and check if we have the right one

            if( !stricmp( szLine + 1, pszSectionName ) )
            {
               bFound = TRUE;
               break;
            }
         }
      }
   }

   if( plFilePos )
      *plFilePos = lFilePos;

   if( !bFound )
      errno = EEXIST;

   return( bFound );
}

/****************************************************************************/
/* CloseINIFile() - Closes INI file. If bForce parameter is TRUE, actually  */
/* closes file. Otherwise, it leaves the file open (and cached!) for        */
/* subsequent searches/updates.                                             */
/****************************************************************************/

void CloseINIFile
(
   IN  BOOL          bForce                 // Force file closed
){
   if( bForce && bINIOpen )
   {
      bfclose( pBFINIOpen );
      bINIOpen = FALSE;
   }
}

/****************************************************************************/
/* OpenINIFile() - Opens the specified INI file. Implements support for     */
/* open file retention.                                                     */
/****************************************************************************/

static BFILE *OpenINIFile
(
   IN  const char    *pszINIPath            // Pathname for the INI file

){
   char              szINIPath[MAX_PATH_LEN0]; // Buffer for manipulating INI file pathname
   FILE_STAT         stStat;                // File status information

   // Check for common INI file wih previous invocations

   if( bINIOpen )
   {
      // Try matching with relative path

      strcpy( szINIPath, pszINIPath );
      TrimDotINI( szINIPath );
      strcat( szINIPath, ".ini" );

      if( stricmp( szINIPath, szINIOpen ) != 0 )
      {
         // Try matching with global path

         strcpy( szINIPath, GLOB_PATH );
         strcat( szINIPath, pszINIPath );
         TrimDotINI( szINIPath );
         strcat( szINIPath, ".ini" );
      }

      if( stricmp( szINIPath, szINIOpen ) == 0 )
      {
         // Same file; check for modifications

         if( STAT_FUNC( szINIPath, &stStat ) )
         {
            // Have modification time; verify against BFile open time

            if( stStat.st_mtime < btime( pBFINIOpen ) )
            {
               // Buffered file is ok; reset file pointer

               bfrewind( pBFINIOpen );
               return( pBFINIOpen );
            }
         }
      }

      // Different or stale file, flush and continue with reopen...

      CloseINIFile( TRUE );
   }

   // Attempt to open INI file as relative path

   strcpy( szINIPath, pszINIPath );
   TrimDotINI( szINIPath );
   strcat( szINIPath, ".ini" );

   pBFINIOpen = bfopen( szINIPath, "r" );

   if( pBFINIOpen )
   {
      // Successful, buffer path for subsequent invocations

      strcpy( szINIOpen, szINIPath );
      bINIOpen = TRUE;
      return( pBFINIOpen );
   }

   // Attempt to open INI file in global path

   strcpy( szINIPath, GLOB_PATH );
   strcat( szINIPath, pszINIPath );
   TrimDotINI( szINIPath );
   strcat( szINIPath, ".ini" );

   pBFINIOpen = bfopen( szINIPath, "r" );

   if( pBFINIOpen )
   {
      // Successful, buffer path for subsequent invocations

      strcpy( szINIOpen, szINIPath );
      bINIOpen = TRUE;
      return( pBFINIOpen );
   }

   return( NULL );
}

/****************************************************************************/
/* CheckINIEntry() - Returns a BOOLEAN indication of whether or not the     */
/* specified entry exists within the INI file.                              */
/****************************************************************************/

BOOL CheckINIEntry
(
   IN  const char    *pszINIPath,           // Pathname for the INI file
   IN  const char    *pszSectionName,       // Section Name for the entry
   IN  const char    *pszEntryName          // Entry Name
){
   BFILE             *pBFINIFile;           // Handle for open INI file
   char              szSectionName[MAX_NAME_LEN0]; // Buffer for manipulating Section Name
   char              szEntryName[MAX_NAME_LEN0]; // Buffer for manipulating Entry Name
   BOOL              bFound = FALSE;        // Indicates of entry was found

   pBFINIFile = OpenINIFile( pszINIPath );

   if( pBFINIFile )
   {
      strcpy( szSectionName, pszSectionName );
      Trim( szSectionName );
      strcpy( szEntryName, pszEntryName );
      Trim( szEntryName );

      // find the right section...

      if( FindINISection( pBFINIFile, szSectionName, NULL ) )
      {
         // find the right entry...

         bFound = FindINIEntry( pBFINIFile, szEntryName, NULL, 0, NULL );
      }

      CloseINIFile( FALSE );
   }

   return( bFound );
}

/****************************************************************************/
/* GetINIEntry() - Gets INI file entry with specified file, section and     */
/* entry names. Returns TRUE if entry found and fits in result buffer.      */
/* Returns FALSE on failure and sets errno to EEXIST if entry was not found */
/* or to E2BIG if entry won't fit in result buffer.                         */
/****************************************************************************/

BOOL GetINIEntry
(
   IN  const char    *pszINIPath,           // Pathname for the INI file
   IN  const char    *pszSectionName,       // Section Name for the entry
   IN  const char    *pszEntryName,         // Entry Name
   OUT char          *pszEntryText,         // Buffer for Entry text
   IN  int           iEntryTextMax          // Space in buffer for Entry text
){
   BFILE             *pBFINIFile;           // Handle for open INI file
   char              szSectionName[MAX_NAME_LEN0]; // Buffer for manipulating Section Name
   char              szEntryName[MAX_NAME_LEN0]; // Buffer for manipulating Entry Name
   BOOL              bFound = FALSE;        // Indicates of entry was found

   // Provide a default for the entry text

   if ( pszEntryText )
      *pszEntryText = '\0';

   // process if found INI file

   pBFINIFile = OpenINIFile( pszINIPath );

   if( pBFINIFile )
   {
      strcpy( szSectionName, pszSectionName );
      Trim( szSectionName );
      strcpy( szEntryName, pszEntryName );
      Trim( szEntryName );

      // find the right section...

      if( FindINISection( pBFINIFile, szSectionName, NULL ) )
      {
         // find the right entry (saving entry's text in user's buffer, if desired)...

         bFound = FindINIEntry( pBFINIFile, szEntryName, pszEntryText, iEntryTextMax, NULL );
      }

      CloseINIFile( FALSE );
   }

   return( bFound );
}

/****************************************************************************/
/* PutINIEntryImp() - Implements the support for creating, updating or      */
/* deleting INI file entries. Deletion is accomplished by setting the entry */
/* text parameter to NULL or "" and setting bSupportDelete to TRUE. If      */
/* bSupportDelete is set to FALSE, entry will be created/updated, even if   */
/* entry text parameter is set to NULL or "". Function returns TRUE if      */
/* successful; FALSE otherwise. During entry creation, the routine will     */
/* automatically create the section header if it doesn't exist. If INI file */
/* doesn't exist, it will be created. File creation always occurs in the    */
/* GLOB_PATH directory.                                                     */
/****************************************************************************/

static BOOL PutINIEntryImp
(
   IN  const char    *pszINIPath,           // Pathname for the INI file
   IN  const char    *pszSectionName,       // Section Name for the entry
   IN  const char    *pszEntryName,         // Entry Name
   IN  const char    *pszEntryText,         // Entry text
   IN  BOOL          bSupportDelete         // Indicates if deletion is supported
){
   BFILE             *pBFINIFile;           // Handle for open INI file
   BFILE             *pBFTempFile;          // Handle for temporary copy of the INI file
   long              lSavedFilePos;         // Saved file position
   long              lFilePos;              // Current file position

   char              szLinef[MAX_LINE_LEN0];        // Buffer for reading lines from the INI file
   char              szSectionName[MAX_NAME_LEN0];  // Buffer for manipulating Section Name
   char              szEntryName[MAX_NAME_LEN0];    // Buffer for manipulating Entry Name
   char              szINIPath[MAX_PATH_LEN0];      // Buffer for manipulating INI file pathname
   char              szINIPath2[MAX_PATH_LEN0];     // Buffer for manipulating INI file pathname
   char              szTMPPath[MAX_PATH_LEN0];      // Buffer for generating INI TMP file pathname

   CloseINIFile( TRUE );

   strcpy( szINIPath2, pszINIPath );
   TrimDotINI( szINIPath2 );

   // open the INI file, trying local directory first

   sprintf( szINIPath, "%s.ini", szINIPath2 );
   sprintf( szTMPPath, "%s.tmp", szINIPath2 );

   pBFINIFile = bfopen( szINIPath, "r+" );

   // if this failed, try global directory

   if( !pBFINIFile )
   {
      sprintf( szINIPath, GLOB_PATH "%s.ini", szINIPath2 );
      sprintf( szTMPPath, GLOB_PATH "%s.tmp", szINIPath2 );
      pBFINIFile = bfopen( szINIPath, "r+" );
   }

   // If it doesn't exist, try forcing its creation

   if( !pBFINIFile )
   {
      pBFINIFile = bfopen( szINIPath, "w" );

      if( pBFINIFile )
         bfclose( pBFINIFile );
      else
         return( FALSE );

      pBFINIFile = bfopen( szINIPath, "r+" );
   }

   // got it open, we're good to go...

   if( pBFINIFile )
   {
      strcpy( szSectionName, pszSectionName );
      Trim( szSectionName );
      strcpy( szEntryName, pszEntryName );
      Trim( szEntryName );

      // If section exists, adding, updating or deleting entry...

      if( FindINISection( pBFINIFile, szSectionName, NULL ) )
      {
         // Point beyond section header

         lSavedFilePos = bftell( pBFINIFile );

         // If entry exists, point at entry

         if( FindINIEntry( pBFINIFile, szEntryName, NULL, 0, &lFilePos ) )
            lSavedFilePos = lFilePos;

         // Open output file...

         pBFTempFile = bfopen( szTMPPath, "w" );

         if( !pBFTempFile )
            return( FALSE );

         // Write everything up to pointer from original file

         bfrewind( pBFINIFile );

         while( bftell( pBFINIFile ) < lSavedFilePos )
         {
            if( bfgets( szLinef, MAX_LINE_LEN, pBFINIFile ) )
               bfputs( szLinef, pBFTempFile );
         }

         // If entry exists, point beyond it within original file (effectively discarding it)

         if( lSavedFilePos == lFilePos )
            bfgets( szLinef, MAX_LINE_LEN, pBFINIFile );

         // Write the new/replacement entry if text is supplied. If bSupportDelete is TRUE and
         // pszEntryText is NULL or "" (i.e. no text supplied), desire is to delete entry so,
         // in this case, we don't write anything

         if( pszEntryText && *pszEntryText )
         {
            // Writing/replacing entry...
            // Handle strings containing comment characters

            if( RawSemi( (char *)pszEntryText ) )
               bfprintf( pBFTempFile, "%s=\"%s\"\n", szEntryName, pszEntryText );
            else
               bfprintf( pBFTempFile, "%s=%s\n", szEntryName, pszEntryText );
         }
         else
         {
            // Deleting/emptying entry...

            if( !bSupportDelete )
               bfprintf( pBFINIFile, "%s=\n", szEntryName );
         }

         // write the remainder of original file

         while( !bfeof( pBFINIFile ) )
         {
            if( bfgets( szLinef, MAX_LINE_LEN, pBFINIFile ) )
               bfputs( szLinef, pBFTempFile );
         }

         bfclose( pBFTempFile );
         bfclose( pBFINIFile );

         // finished with original file

         unlink( szINIPath );

         // replace with updated file

         rename( szTMPPath, szINIPath );
      }
      else
      {
         // Couldn't find section; so need to create section header and entry. If
         // bSupportDelete is TRUE, we do nothing if there is no entry text (indicating
         // desire was to delete entry)

         // (Note: file pointer is at EOF from search, so can just write where we are)

         if( pszEntryText && *pszEntryText )
         {
            bfprintf( pBFINIFile, "\n[%s]\n", szSectionName );

            if( RawSemi( (char *)pszEntryText ) )
               bfprintf( pBFINIFile, "%s=\"%s\"\n", szEntryName, pszEntryText );
            else
               bfprintf( pBFINIFile, "%s=%s\n", szEntryName, pszEntryText );
         }
         else
         {
            if( !bSupportDelete )
               bfprintf( pBFINIFile, "\n[%s]\n%s=\n", szSectionName, szEntryName );
         }

         // Close the file

         bfclose( pBFINIFile );
      }

      return( TRUE );
   }

   // Couldn't access/create file; indicate failure

   return( FALSE );
}

/****************************************************************************/
/* PutINIEntry() - Creates, updates or deletes INI file entries. Empty      */
/* entry text indicates deletion desired. Returns TRUE if successful; FALSE */
/* otherwise. During entry creation, the routine will automatically create  */
/* the section header if it doesn't exist. If INI file doesn't exist, it    */
/* will be created. File creation always occurs in the GLOB_PATH folder.    */
/****************************************************************************/

BOOL PutINIEntry
(
   IN  const char    *pszINIPath,         // Pathname for the INI file
   IN  const char    *pszSectionName,     // Section Name for the entry
   IN  const char    *pszEntryName,       // Entry Name
   IN  const char    *pszEntryText        // Entry text
){
   return( PutINIEntryImp( pszINIPath, pszSectionName, pszEntryName, pszEntryText, TRUE ) );
}

/****************************************************************************/
/* PutINIEntryAbs() - Creates or updates INI file entry. Allows entry to be */
/* created with no text assignment. Returns TRUE if successful; FALSE       */
/* otherwise. During entry creation, the routine will automatically create  */
/* the section header if it doesn't exist. If INI file doesn't exist, it    */
/* will be created. File creation always occurs in the GLOB_PATH folder.    */
/****************************************************************************/

BOOL PutINIEntryAbs
(
   IN  const char    *pszINIPath,         // Pathname for the INI file
   IN  const char    *pszSectionName,     // Section Name for the entry
   IN  const char    *pszEntryName,       // Entry Name
   IN  const char    *pszEntryText        // Entry text
){
    return( PutINIEntryImp( pszINIPath, pszSectionName, pszEntryName, pszEntryText, FALSE ) );
 }

