/****************************************************************************/
/*                                                                          */
/*  Module:         libSMART.h                                              */
/*                                                                          */
/*  Description:    Provides definitions  for  handling  information  from  */
/*                  hard  drives that support Self-Monitoring Analysis and  */
/*                  Reporting Technology (S.M.A.R.T.).                      */
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

#ifndef _LIBSMART_H
#define _LIBSMART_H

#if _MSC_VER > 1000
#pragma once
#pragma warning( disable: 4142 4214 )
#endif

#pragma pack(1)

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

#ifndef BIT_FIELD_IN_BYTE
/****************************************************************************/
/* Bit field counting is slightly buggy in 16-bit VC++ compiler. We handle  */
/* this by using its fixed field length extension feature. Since most of    */
/* the other compilers that we use (i.e., Microsoft and Borland) also       */
/* support this feature (the GNU compiler being the only exception), we     */
/* will use this extension everywhere that we can. Note: These extensions   */
/* are enabled by default; if you disable, compilation errors will abound!  */
/****************************************************************************/

#ifdef __GNUC__

#define BIT_FIELD_IN_BYTE       unsigned
#define BIT_FIELD_IN_WORD       unsigned
#define BIT_FIELD_IN_DWORD      unsigned

#else // ndef __GNUC__

#define BIT_FIELD_IN_BYTE       unsigned char
#define BIT_FIELD_IN_WORD       unsigned short
#define BIT_FIELD_IN_DWORD      unsigned long

#endif // ndef __GNUC__

#endif // ndef BIT_FIELD_IN_BYTE

/****************************************************************************/
/* Miscellaneous Definitions                                                */
/****************************************************************************/

#define SMART_MAX_ATTRIBS       30                  // Max attributes supported

#define SMART_TEMP_ATTRIB_1     194                 // First Temperature Attribute Id
#define SMART_TEMP_ATTRIB_2     231                 // Second Temperature Attribute Id

#define SMART_BUFFER_SIZE       512                 // Size of a sector

/****************************************************************************/
/* Cylinder values returned when indicating threshold exceeded attribute    */
/* These are a nibble reversal of the input value.                          */
/****************************************************************************/

#define SMART_CYL_LOW_EXC       0xF4                // SMART_CYL_LOW = 0x4F
#define SMART_CYL_HI_EXC        0x2C                // SMART_CYL_HI = 0xC2

/****************************************************************************/
/* Word offsets into the IDENTIFY DEVICE data block for the fields that we  */
/* have an interest in.                                                     */
/****************************************************************************/

#define ID_SERIAL_NO_OFFSET     10                  // Drive Serial Number
#define ID_SERIAL_NO_WORDS      10

#define ID_MODEL_NO_OFFSET      27                  // Drive Model Number
#define ID_MODEL_NO_WORDS       20

#define ID_CMD_SET_SUPP_OFFSET  83                  // Supported Command Set
#define ID_CMD_SET_SUPP_48BITS  0x400               // Bit 10: bits of addressing: 0=28-bit, 1=48-bit)

#define ID_SECT28_WORD0_OFFSET  60                  // Word 0 of Sector Count if 28-bit addressing
#define ID_SECT28_WORD1_OFFSET  61                  // Word 1 of Sector Count if 28-bit addressing

#define ID_SECT48_WORD0_OFFSET  100                 // Word 0 of Sector Count if 48-bit addressing
#define ID_SECT48_WORD1_OFFSET  101                 // Word 1 of Sector Count if 48-bit addressing
#define ID_SECT48_WORD2_OFFSET  102                 // Word 2 of Sector Count if 48-bit addressing
#define ID_SECT48_WORD3_OFFSET  103                 // Word 3 of Sector Count if 48-bit addressing

#define ID_SECTINFO_OFFSET      106                 // Sector size information
#define ID_SECTINFO_VALID_MASK  0xC000              // Check bits 14 and 15
#define ID_SECTINFO_VALID       0x4000              // Bit 14 set and bit 15 reset
                                                    // If it isn't valid, sector size is 512
#define ID_SECTINFO_SIZE_SPEC   0x1000              // Indicates if sector size specified

#define ID_SECTSIZE_WORD0_OFFSET 117                // Word 0 of Sector Size word count
#define ID_SECTSIZE_WORD1_OFFSET 118                // Word 1 of Sector Size word count

/****************************************************************************/
/* SMART_ATTRIB - Structure for a S.M.A.R.T. Attribute                      */
/****************************************************************************/

typedef struct _SMART_ATTRIB
{
    BYTE                        byAttribId;         // Attribute ID
    WORD                        wFlags;             // Flags
    BYTE                        byValue;            // Attribute value
    BYTE                        byWorst;            // Worst-case value
    BYTE                        byRaw[6];           // Raw data
    BYTE                        byReserved;         // Vendor-specific data

} SMART_ATTRIB, *P_SMART_ATTRIB;

/****************************************************************************/
/* SMART_ATTRIBS - Structure for the S.M.A.R.T. Attribute data block        */
/****************************************************************************/

typedef struct _SMART_ATTRIBS
{
    WORD                        wRevisionNumber;    // Table revision number
    SMART_ATTRIB                Attribute[SMART_MAX_ATTRIBS];// Attribute table
    BYTE                        byReserved[6];      // Reserved bytes
    WORD                        wSMARTCapability;   // Capabilities word
    BYTE                        byReserved2[16];    // Reserved bytes
    BYTE                        byVendorSpecific[125];// Vendor specific data
    BYTE                        byCheckSum;         // Checksum for structure

} SMART_ATTRIBS;

/****************************************************************************/
/* SMART_THRESH - Structure for a S.M.A.R.T. Threshold                      */
/****************************************************************************/

typedef struct _SMART_THRESH
{
    BYTE                        byAttribId;         // Attribute ID
    BYTE                        byValue;            // Threshold value
    BYTE                        byReserved[10];     // Vendor-specific data

} SMART_THRESH, *P_SMART_THRESH;

/****************************************************************************/
/* SMART_THRESHS - Structure for the S.M.A.R.T. Threshold data block        */
/****************************************************************************/

typedef struct _SMART_THRESHS
{
    WORD                        wRevisionNumber;    // Table revision number
    SMART_THRESH                Threshold[SMART_MAX_ATTRIBS];// Threshold table
    BYTE                        byReserved[18];     // Reserved bytes
    BYTE                        VendorSpecific[131];// Vendor-specific data
    BYTE                        byCheckSum;         // Checksum for structure

} SMART_THRESHS;

#pragma pack()

#endif // ndef _LIBSMART_H

