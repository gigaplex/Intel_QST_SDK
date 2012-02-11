/****************************************************************************/
/*                                                                          */
/*  Module:         BFileIO.h                                               */
/*                                                                          */
/*  Description:    Defines the routines used to perform I/O on files that  */
/*                  need  to be buffered in memory. This includes (and was  */
/*                  specifically implemented to support) INI files,  which  */
/*                  are  repeatedly  scanned  for  occurrences of specific  */
/*                  sections and parameters.                                */
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
/*              3.  Since buffered files that are modified won't  actually  */
/*                  get  written to disk until bfclose() is invoked, it is  */
/*                  important to check its return code!!                    */
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

#ifndef _BFILEIO_H
#define _BFILEIO_H

#include <stdio.h>
#include <time.h>

#include "typedef.h"

/****************************************************************************/
/* Definitions                                                              */
/****************************************************************************/

// File size support

#define BF_SEG_NUM      32                      // Max file segments
#define BF_SEG_SIZE     16384                   // Size of segments

// File mode/status bits

#define BF_READ         0x0001                  // Opened for reading
#define BF_WRITE        0x0002                  // Opened for writing
#define BF_APPEND       0x0004                  // Always write at EOF
#define BF_BINARY       0x0008                  // Binary file
#define BF_BUFFER       0x0010                  // Need to buffer file
#define BF_ERROR        0x0020                  // I/O Error has occurred
#define BF_DIRTY        0x0040                  // File needs to be saved

/****************************************************************************/
/* Structures                                                               */
/****************************************************************************/

#pragma pack(1)

typedef struct _BFILE
{
   unsigned int         uVerify;                // Verification Field
   struct _BFILE *      pBFileNext;             // Next BFILE in list
   struct _BFILE *      pBFilePrev;             // Previous BFILE in list
   FILE *               pFFile;                 // Actual File Handle
   time_t               tOpenTime;              // Time file was opened
   unsigned long        ulFlags;                // Mode/status flags
   unsigned long        ulFileLen;              // Amount of data in file
   unsigned long        ulFilePos;              // File Pointer
   char *               pFileBuf[BF_SEG_NUM];   // Buffer pointers

}  BFILE;

#pragma pack()

/****************************************************************************/
/* Function Prototypes                                                      */
/****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

BFILE * bfopen   ( const char *pszFilePath, const char *pszOpenMode );
int     bfclose  ( BFILE *pBFile );

int     bfeof    ( BFILE *pBFile );
long    bftell   ( BFILE *pBFile );

void    bfclrerr ( BFILE *pBFile );
int     bferror  ( BFILE *pBFile );

int     bfseek   ( BFILE *pBFile, long lOffset, int iOrigin );
void    bfrewind ( BFILE *pBFile );

time_t  btime    ( BFILE *pBFile );

int     bfputc   ( int iChar, BFILE *pBFile );
int     bfputs   ( const char *szString, BFILE *pBFile );
size_t  bfwrite  ( const void *pvBuffer, size_t tChars, size_t tItems, BFILE *pBFile );
int     bfprintf ( BFILE *pBFile, const char *pszFormat, ... );

int     bfgetc   ( BFILE *pBFile );
char *  bfgets   ( char *szBuffer, int iChars, BFILE *pBFile );
size_t  bfread   ( void *pvBuffer, size_t tChars, size_t tItems, BFILE *pBFile );

#ifdef __cplusplus
}
#endif

#endif // ndef _BFILEIO_H

