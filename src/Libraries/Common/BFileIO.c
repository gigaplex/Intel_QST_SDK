/****************************************************************************/
/*                                                                          */
/*  Module:         BFileIO.c                                               */
/*                                                                          */
/*  Description:    Implements the  routines used to  perform I/O on files  */
/*                  that need to be buffered in memory. This includes (and  */
/*                  was specifically implemented to  support)  INI  files,  */
/*                  which   are  repeatedly  scanned  for  occurrences  of  */
/*                  specific sections and parameters.                       */
/*                                                                          */
/*  Note:       1.  Operations for all implemented functions follows the C  */
/*                  Library Standard. The definitions for  EOF,  SEEK_CUR,  */
/*                  SEEK_SET,  SEEK_END, EBADF, EINVAL, EACCES and ENOSPC,  */
/*                  come from the header files of the tools being compiled  */
/*                  with.                                                   */
/*                                                                          */
/*              2.  Support for binary files (and <CR><LF> translation) is  */
/*                  performed in the real file I/O layer. While  the  file  */
/*                  is buffered in memory, no translation will occur. This  */
/*                  means  that  all modes of bfseek() usage will have the  */
/*                  expected, predictable results.                          */
/*                                                                          */
/*              3.  Since buffered files that are modified don't  actually  */
/*                  get  written  to disk until bfclose() is called, it is  */
/*                  important to verify its return code!!                   */
/*                                                                          */
/*              4.  Support is provided for  automatically  closing  files  */
/*                  (and  flushing  them  to  disk  if  modified) when the  */
/*                  application exits. This is done via C Library function  */
/*                  atexit(). Consequently, if you abnormally  terminate a  */
/*                  program  using  this facility, file modifications will  */
/*                  be lost!!                                               */
/*                                                                          */
/*              5.  Variable errno is set as follows:                       */
/*                                                                          */
/*                      EBADF   An invalid BFILE pointer was passed to one  */
/*                              of the functions.                           */
/*                                                                          */
/*                      EINVAL  Invalid parameter passed to a function.     */
/*                                                                          */
/*                      EACCES  Called an output function when file opened  */
/*                              exclusively for input or an input function  */
/*                              when file opened exclusively for output.    */
/*                                                                          */
/*                      ENOSPC  Called an output function and the buffered  */
/*                              file could not be grown (reached file size  */
/*                              limit or malloc() failed).                  */
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

#include <stdlib.h>
#include <stddef.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include "BFileIO.h"

/****************************************************************************/
/* Definitions                                                              */
/****************************************************************************/

#define LINE_BUF_SIZE       256             // line size supported

#if _MSC_VER > 800
#define BF_VERIFY           'BFIO'          // Verification field value
#else
#define BF_VERIFY           'BF'            // Verification field value
#endif

// Macros for manipulating mode/status bits

#define FLAGSET(a,b)        (((a)->ulFlags & (b)) == (b))

#define IS_READABLE(x)      FLAGSET(x,BF_READ)
#define IS_WRITEABLE(x)     FLAGSET(x,BF_WRITE)
#define IS_APPEND(x)        FLAGSET(x,BF_APPEND)
#define IS_ERROR(x)         FLAGSET(x,BF_ERROR)
#define IS_DIRTY(x)         FLAGSET(x,BF_DIRTY)
#define IS_BINARY(x)        FLAGSET(x,BF_BINARY)

#define FLAGRESET(a,b)      (((a)->ulFlags & (b)) != (b))

#define NOT_READABLE(x)     FLAGRESET(x,BF_READ)
#define NOT_WRITEABLE(x)    FLAGRESET(x,BF_WRITE)
#define NOT_APPEND(x)       FLAGRESET(x,BF_APPEND)
#define NOT_ERROR(x)        FLAGRESET(x,BF_ERROR)
#define NOT_DIRTY(x)        FLAGRESET(x,BF_DIRTY)
#define NOT_BINARY(x)       FLAGRESET(x,BF_BINARY)

#define SETFLAG(a,b)        ((a)->ulFlags |= (b))

#define SET_ERROR(x)        SETFLAG(x,BF_ERROR)
#define SET_DIRTY(x)        SETFLAG(x,BF_DIRTY)

#define RESETFLAG(a,b)      ((a)->ulFlags &= ~(b))

#define RESET_ERROR(x)      RESETFLAG(x,BF_ERROR)
#define RESET_DIRTY(x)      RESETFLAG(x,BF_DIRTY)

/****************************************************************************/
/* Global Variables                                                         */
/****************************************************************************/

static BFILE *              pBFileFirst = NULL;
static BFILE *              pBFileLast  = NULL;
static BOOL                 bRegistered = FALSE;

/****************************************************************************/
/* bferror() - Indicates is error has occurred on buffered file             */
/****************************************************************************/

int bferror( BFILE *pBFile )
{
   if( !pBFile || (pBFile->uVerify != BF_VERIFY) )
      return( 1 );

   return( IS_ERROR( pBFile ) );
}

/****************************************************************************/
/* bfclrerr() - Clears error indicator for buffered file                    */
/****************************************************************************/

void bfclrerr( BFILE *pBFile )
{
   if( !pBFile || (pBFile->uVerify != BF_VERIFY) )
      return;

   RESET_ERROR( pBFile );
}

/****************************************************************************/
/* bfeof() - Indicates if buffered file's file pointer is at EOF            */
/****************************************************************************/

int bfeof( BFILE *pBFile )
{
   // Function has no error return; we will say at EOF for bad BFile pointer

   if( !pBFile || (pBFile->uVerify != BF_VERIFY) )
      return( 1 );

   // Indicate if at EOF

   if( (pBFile->ulFilePos >= pBFile->ulFileLen) )
      return( 1 );
   else
      return( 0 );
}

/****************************************************************************/
/* _bfseek() - Internal routine to set buffered file's file pointer         */
/****************************************************************************/

static int _bfseek( BFILE *pBFile, long lOffset, int iOrigin )
{
   long lFilePos;

   switch( iOrigin )
   {
   case SEEK_CUR:

      lFilePos = pBFile->ulFilePos + lOffset;
      break;

   case SEEK_END:

      lFilePos = pBFile->ulFileLen + lOffset;
      break;

   case SEEK_SET:

      lFilePos = lOffset;
      break;

   default:

      errno = EINVAL;
      return( 1 );
   }

   if( lFilePos < 0 )
      pBFile->ulFilePos = 0;
   else
   {
      pBFile->ulFilePos = lFilePos;

      if( pBFile->ulFilePos > pBFile->ulFileLen )
          pBFile->ulFilePos = pBFile->ulFileLen;
   }

   return( 0 );
}

/****************************************************************************/
/* bfseek() - Sets buffered file's file pointer                             */
/****************************************************************************/

int bfseek( BFILE *pBFile, long lOffset, int iOrigin )
{
   if( !pBFile || (pBFile->uVerify != BF_VERIFY) )
   {
      errno = EBADF;
      return( 0 );
   }

   return( _bfseek( pBFile, lOffset, iOrigin ) );
}

/****************************************************************************/
/* bfrewind() - Rewinds the buffered file to the beginning                  */
/****************************************************************************/

void bfrewind( BFILE *pBFile )
{
   if( !pBFile || (pBFile->uVerify != BF_VERIFY) )
      return; // no error return

   pBFile->ulFilePos = 0;
   RESET_ERROR( pBFile );           // strange but required by C Spec.
}

/****************************************************************************/
/* bftell() - Returns position of file pointer within a buffered file.      */
/****************************************************************************/

long bftell( BFILE *pBFile )
{
   if( !pBFile || (pBFile->uVerify != BF_VERIFY) )
   {
      errno = EBADF;
      return( -1L );
   }

   return( pBFile->ulFilePos );
}

/****************************************************************************/
/* _bfgetc() - Internal routine to get a character from a buffered file     */
/****************************************************************************/

static int _bfgetc( BFILE *pBFile )
{
   // Validate input parameters

   if( pBFile->ulFilePos >= pBFile->ulFileLen )
      return( EOF );
   else
   {
      // Calculate where character should be obtained

      unsigned long ulBuffer = pBFile->ulFilePos / BF_SEG_SIZE;
      unsigned long ulOffset = pBFile->ulFilePos % BF_SEG_SIZE;

      // Update file pointer

      ++pBFile->ulFilePos;

      // Return character

      return( pBFile->pFileBuf[ulBuffer][ulOffset] );
   }
}

/****************************************************************************/
/* bfgetc() - Gets a character from a buffered file                         */
/****************************************************************************/

int bfgetc( BFILE *pBFile )
{
   // Validate input parameters

   if( !pBFile || (pBFile->uVerify != BF_VERIFY) )
   {
      errno = EBADF;
      return( EOF );
   }

   // If reading from file is allowed, read a character

   if( IS_READABLE( pBFile ) )
      return( _bfgetc(  pBFile ) );

   // Otherwise, indicate error

   errno = EACCES;
   SET_ERROR( pBFile );
   return( EOF );
}

/****************************************************************************/
/* bfgets() - Gets a line from a buffered file                              */
/****************************************************************************/

char *bfgets( char *pszBuffer, int iChars, BFILE *pBFile )
{
   int  iIndex;
   char iChar;

   // Validate input parameters

   if( !pBFile || (pBFile->uVerify != BF_VERIFY) )
   {
      errno = EBADF;
      return( NULL );
   }

   if( NOT_READABLE( pBFile ) )
   {
      errno = EACCES;
      SET_ERROR( pBFile );
      return( NULL );
   }

   if( --iChars <= 0 )
      return( NULL );

   // Input allowable number of chars, stopping at newline

   for( iIndex = 0; iIndex < iChars; iIndex++ )
   {
      int iRes = _bfgetc( pBFile );
      pszBuffer[iIndex] = iChar = (char)iRes;

      // if at EOF, return NULL to indicate EOF condition

      if( iRes == EOF )
         return( NULL );

      if( iChar == '\n' )
      {
         ++iIndex;
         break;
      }
   }

   // Terminate the string

   pszBuffer[iIndex] = '\0';
   return( pszBuffer );
}

/****************************************************************************/
/* bfread() - Gets blocks of data (items) from a buffer file                */
/****************************************************************************/

size_t bfread( void *pvBuffer, size_t tChars, size_t tItems, BFILE *pBFile )
{
   // Verify parameters

   if( !pBFile || (pBFile->uVerify != BF_VERIFY) )
   {
      errno = EBADF;
      return( 0 );
   }

   // Fail if not opened for reading

   if( NOT_READABLE( pBFile ) )
   {
      errno = EACCES;
      SET_ERROR( pBFile );
      return( 0 );
   }
   else
   {
      size_t tItem, tChar;
      char   iChar, *pChar = (char *)pvBuffer;

      // Read requested number of items

      for( tItem = 0; tItem < tItems; tItem++ )
      {
         // Read requested number of characters for each item

         for( tChar = 0; tChar < tChars; tChar++ )
         {
            int iRes = _bfgetc( pBFile );
            *pChar++ = iChar = (char)iRes;

            // If at EOF, return num items completely input

            if( iRes == EOF )
               return( tItem );
         }
      }

      // Got all items requested

      return( tItems );
   }
}

/****************************************************************************/
/* _bfputc() - Internal routine to put a character into a buffered file     */
/****************************************************************************/

static int _bfputc( int iChar, BFILE *pBFile )
{
   unsigned long ulBuffer, ulOffset;

   // If appending, always write at EOF

   if( IS_APPEND( pBFile ) )
      pBFile->ulFilePos = pBFile->ulFileLen;

   // Calculate where character should be saved

   ulBuffer = pBFile->ulFilePos / BF_SEG_SIZE;
   ulOffset = pBFile->ulFilePos % BF_SEG_SIZE;

   // Check if file is too big to add this character

   if( ulBuffer >= BF_SEG_NUM )
   {
      errno = ENOSPC;
      SET_ERROR( pBFile );
      return( EOF );
   }

   // Allocate storage buffer if growth needed

   if( !pBFile->pFileBuf[ulBuffer] )
   {
      pBFile->pFileBuf[ulBuffer] = calloc( 1, BF_SEG_SIZE );

      if( !pBFile->pFileBuf[ulBuffer] )
      {
         errno = ENOMEM;
         SET_ERROR( pBFile );
         return( EOF );
      }
   }

   // Store character in buffer

   pBFile->pFileBuf[ulBuffer][ulOffset] = (char)iChar;

   // Update file pointer (and possibly file size)

   if( ++pBFile->ulFilePos > pBFile->ulFileLen )
      pBFile->ulFileLen = pBFile->ulFilePos;

   SET_DIRTY( pBFile );
   return( iChar );
}

/****************************************************************************/
/* bfputc() - Puts a character into a buffered file                         */
/****************************************************************************/

int bfputc( int iChar, BFILE *pBFile )
{
   if( !pBFile || (pBFile->uVerify != BF_VERIFY) )
   {
      errno = EBADF;
      return( EOF );
   }

   // If writing not allowed, signal error

   if( NOT_WRITEABLE( pBFile ) )
   {
      errno = EACCES;
      SET_ERROR( pBFile );
      return( 0 );
   }

   // Otherwise write character

   return( _bfputc( iChar, pBFile ) );
}

/****************************************************************************/
/* _bfwrite() - Internal routine for put data blocks into a buffered file   */
/****************************************************************************/

static size_t _bfwrite( const void *pvBuffer, size_t tChars, size_t tItems, BFILE *pBFile )
{
   size_t   tItem, tChar;
   char     *pChar = (char *)pvBuffer;

   // Handle requested number of items

   for( tItem = 0; tItem < tItems; tItem++ )
   {
      // Handle requested number of characters per item

      for( tChar = 0; tChar < tChars; tChar++ )
      {
         // Put a character, aborting if problem develops

         if( _bfputc( *pChar++, pBFile ) == EOF )
         {
            // When aborting, indicate how many items we did successfully

            return( tItem );
         }
      }
   }

   // Indicate all items done successfully!

   return( tItems );
}

/****************************************************************************/
/* bfwrite() - Puts data blocks into a buffered file                        */
/****************************************************************************/

size_t bfwrite( const void *pvBuffer, size_t tChars, size_t tItems, BFILE *pBFile )
{
   if( !pBFile || (pBFile->uVerify != BF_VERIFY) )
   {
      errno = EBADF;
      return( 0 );
   }

   if( NOT_WRITEABLE( pBFile ) )
   {
      errno = EACCES;
      SET_ERROR( pBFile );
      return( 0 );
   }

   return( _bfwrite( pvBuffer, tChars, tItems, pBFile ) );
}

/****************************************************************************/
/* bfputs() - Puts a string into a buffered file                            */
/****************************************************************************/

int bfputs( const char *szString, BFILE *pBFile )
{
   // Verify parameters

   if( !pBFile || (pBFile->uVerify != BF_VERIFY) )
   {
      errno = EBADF;
      return( EOF );
   }

   // If not writeable, signal error

   if( NOT_WRITEABLE( pBFile ) )
   {
      errno = EACCES;
      SET_ERROR( pBFile );
      return( EOF );
   }

   // Otherwise, write the requested items

   return( _bfwrite( szString, 1, strlen( szString ), pBFile )? strlen( szString ) : EOF );
}

/****************************************************************************/
/* bfprintf() - Puts formatted data into a buffered file                    */
/****************************************************************************/

int bfprintf( BFILE *pBFile, const char *pszFormat, ... )
{
   char    szLineBuf[LINE_BUF_SIZE];
   va_list vaList;
   int     iLen;

   // Verify parameters

   if( !pBFile || (pBFile->uVerify != BF_VERIFY) )
   {
      errno = EBADF;
      return( 0 );
   }

   if( !pszFormat )
   {
      errno = EINVAL;
      return( 0 );
   }

   if( !pszFormat[0] )
      return( 0 );

   // If not writeable, signal error

   if( NOT_WRITEABLE( pBFile ) )
   {
      errno = EACCES;
      SET_ERROR( pBFile );
      return( 0 );
   }

   // Use sprintf() to build output string

   va_start( vaList, pszFormat );
   iLen = vsprintf( szLineBuf, pszFormat, vaList );
   va_end( vaList );

   // And then bfwrite() to do the actual output

   return( _bfwrite( szLineBuf, 1, (size_t)iLen, pBFile ) );
}

/****************************************************************************/
/* bfclose() - Closes buffered file, writing contents to media if modified  */
/****************************************************************************/

int bfclose( BFILE *pBFile )
{
   unsigned uBuffer;
   int      iRetCode = 0;

   if( !pBFile || (pBFile->uVerify != BF_VERIFY) )
   {
      errno = EBADF;
      return( 1 );
   }

   // If modified, write file back to media; free buffers as they're written

   if( IS_DIRTY( pBFile ) )
   {
      unsigned long ulLeft, ulWrite, ulWritten;

      fseek( pBFile->pFFile, 0, SEEK_SET );

      for( uBuffer = 0, ulLeft = pBFile->ulFileLen; ulLeft ; uBuffer++ )
      {
         ulWrite = ( ulLeft >= BF_SEG_SIZE )? BF_SEG_SIZE : ulLeft;
         ulLeft -= ulWritten = fwrite( pBFile->pFileBuf[uBuffer], 1, (size_t)ulWrite, pBFile->pFFile );

         if( ulWritten != ulWrite )
            iRetCode = errno;   // Save this to return

         free( pBFile->pFileBuf[uBuffer] );
         pBFile->pFileBuf[uBuffer] = NULL;
      }
   }

   // Otherwise, just free up the buffers used

   else
   {
      for( uBuffer = 0; pBFile->pFileBuf[uBuffer]; uBuffer++ )
      {
         free( pBFile->pFileBuf[uBuffer] );
         pBFile->pFileBuf[uBuffer] = NULL;
      }
   }

   // Unlink from our BFILE list

   if( pBFile->pBFilePrev )
      pBFile->pBFilePrev->pBFileNext = pBFile->pBFileNext;
   else
      pBFileFirst = pBFile->pBFileNext;

   if( pBFile->pBFileNext )
      pBFile->pBFileNext->pBFilePrev = pBFile->pBFilePrev;
   else
      pBFileLast = pBFile->pBFilePrev;

   // Free up resources

   pBFile->uVerify = 0;
   fclose( pBFile->pFFile );
   free( pBFile );

   // Indicate success

   if( iRetCode )
   {
      errno = iRetCode;
      return( 1 );
   }

   return( 0 );
}

/****************************************************************************/
/* ExitHandler() - Cleans up any open files at program termination          */
/****************************************************************************/

static void ExitHandler( void )
{
   while( pBFileFirst )
      bfclose( pBFileFirst );
}

/****************************************************************************/
/* BufferFile() - Reads the content of a physical file into the specified   */
/* buffered file                                                            */
/****************************************************************************/

static BOOL BufferFile( BFILE *pBFile, FILE *pFFile )
{
   char szLineBuf[LINE_BUF_SIZE];

   // Read entire disk file and write into buffered file; terminate if any problems

   pBFile->ulFilePos = 0;

   while( !feof( pFFile ) && !ferror( pFFile ) && !bferror( pBFile ) )
   {
      if( fgets( szLineBuf, LINE_BUF_SIZE, pFFile ) )
         _bfwrite( szLineBuf, strlen( szLineBuf ), 1, pBFile );
   }

   if( ferror( pFFile ) || bferror( pBFile ) )
      return( FALSE );

   // Writes to buffered file will have set dirty bit; reset

   RESET_DIRTY( pBFile );

   // Even for files opened in append mode, the file pointer must start at BOF

   pBFile->ulFilePos = 0;
   return( TRUE );
}

/****************************************************************************/
/* btime() - Returns the time when the specified file was read into memory  */
/****************************************************************************/

time_t btime( BFILE *pBFile )
{
   return( pBFile->tOpenTime );
}

/****************************************************************************/
/* bfopen() - Opens and buffers a file                                      */
/****************************************************************************/

typedef struct
{
   char             *pszOpenMode;
   char             *pszFileOpenMode;
   unsigned         uMode;

}  OPEN_MODE;

static OPEN_MODE    stOpenMode[] =
{
   { "r",   "r",    BF_READ                                    | BF_BUFFER },
   { "rb",  "rb",   BF_READ                        | BF_BINARY | BF_BUFFER },
   { "w",   "w",              BF_WRITE                                     },
   { "wb",  "wb",             BF_WRITE             | BF_BINARY             },
   { "a",   "r+",             BF_WRITE | BF_APPEND             | BF_BUFFER },
   { "ab",  "rb+",            BF_WRITE | BF_APPEND | BF_BINARY | BF_BUFFER },
   { "r+",  "r+",   BF_READ | BF_WRITE                         | BF_BUFFER },
   { "r+b", "rb+",  BF_READ | BF_WRITE             | BF_BINARY | BF_BUFFER },
   { "rb+", "rb+",  BF_READ | BF_WRITE             | BF_BINARY | BF_BUFFER },
   { "w+",  "w+",   BF_READ | BF_WRITE                                     },
   { "w+b", "wb+",  BF_READ | BF_WRITE             | BF_BINARY             },
   { "wb+", "wb+",  BF_READ | BF_WRITE             | BF_BINARY             },
   { "a+",  "r+",   BF_READ | BF_WRITE | BF_APPEND             | BF_BUFFER },
   { "a+b", "rb+",  BF_READ | BF_WRITE | BF_APPEND | BF_BINARY | BF_BUFFER },
   { "ab+", "rb+",  BF_READ | BF_WRITE | BF_APPEND | BF_BINARY | BF_BUFFER },
   { NULL,  NULL,   0                                                      }
};

BFILE *bfopen( const char *pszFilePath, const char *pszOpenMode )
{
   BFILE    *pBFile;
   FILE     *pFFile;
   int      iIndex;
   unsigned uMode;

   // Cannot handle NULL

   if( !pszFilePath || !pszFilePath[0] )
   {
      errno = EINVAL;
      return( NULL );
   }

   // Ascertain requirements for setup the buffered file

   for( iIndex = 0; ; iIndex++ )
   {
      if( !stOpenMode[iIndex].pszOpenMode )
      {
         errno = EINVAL;
         return( NULL );
      }

      if( !strcmp( stOpenMode[iIndex].pszOpenMode, pszOpenMode ) )
         break;
   }

   uMode = stOpenMode[iIndex].uMode;

   // Attempt to open the file

   pFFile = fopen( pszFilePath, stOpenMode[iIndex].pszFileOpenMode );

   if( !pFFile )
   {
      // Failed; file may not exist...

      if( uMode & BF_APPEND )
      {
         // Ifappending but file doesn't exist, try creating...

         pFFile = fopen( pszFilePath, ( uMode & BF_BINARY )? "wb+" : "w+" );
         uMode &= ~BF_BUFFER; // Won't be anything to buffer...
      }

      if( !pFFile )
         return( NULL );
   }

   // Allocate BFILE object

   pBFile = calloc( 1, sizeof(BFILE) );

   if( !pBFile )
   {
      fclose( pFFile );
      errno = ENOMEM;
      return( NULL );
   }

   pBFile->uVerify   = BF_VERIFY;
   pBFile->pFFile    = pFFile;
   pBFile->ulFlags   = uMode;
   pBFile->tOpenTime = time( NULL );

   // Buffer the file (if necessary)

   if( uMode & BF_BUFFER )
   {
      if( !BufferFile( pBFile, pFFile ) )
      {
         // Buffering failed, cleanup

         int iErrnoSave = errno;
         fclose( pFFile );

         for( iIndex = 0; pBFile->pFileBuf[iIndex]; iIndex++ )
            free( pBFile->pFileBuf[iIndex] );

         free( pBFile );
         errno = iErrnoSave;
         return( NULL );
      }
   }

   // Place BFILE object in list

   if( pBFileFirst )
   {
      // Multiple BFILEs, add this one to the end of the list

      pBFileLast->pBFileNext = pBFile;
      pBFile->pBFilePrev = pBFileLast;
      pBFileLast = pBFile;
   }
   else
   {
      // First BFILE, make it the whole list

      pBFileFirst = pBFileLast = pBFile;
   }

   // If we haven't already, register exit handler

   if( !bRegistered )
   {
      atexit( ExitHandler );
      bRegistered = TRUE;
   }

   return( pBFile );
}


