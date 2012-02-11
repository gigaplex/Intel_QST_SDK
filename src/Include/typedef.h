/****************************************************************************/
/*                                                                          */
/*  Module:       typedef.h                                                 */
/*                                                                          */
/*  Description:  Provides basic type and macro definitions.                */
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

#ifndef _TYPEDEF_H_
#define _TYPEDEF_H_

#if _MSC_VER > 1000
#pragma once
#pragma warning( disable: 4142 )    /* Ignore equivalent redefinitions */
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _BASETSD_H_
/****************************************************************************/
/* Nothing pre-defined by Windows header files...                           */
/****************************************************************************/

/****************************************************************************/
/* C_ASSERT() - Macro for a compile time Assert. It can be used to perform  */
/* many compile-time assertions: type sizes, field offsets, etc. An         */
/* assertion failure results in error C2118: negative subscript.            */
/****************************************************************************/

#define C_ASSERT(e) typedef char __C_ASSERT__[(e)?1:-1]

/****************************************************************************/
/* Definitions for basic scalar types (8-, 16- and 32-bit)                  */
/****************************************************************************/

typedef char                    INT8;
typedef unsigned char           UINT8;

typedef short                   INT16;
typedef unsigned short          UINT16;

#if defined(_WIN32) || defined(__MSDOS__) || defined(MSDOS) || defined(_MSDOS) || defined(__DOS__)

typedef long                    INT32;
typedef unsigned long           UINT32;

#ifndef _SIZE_T_DEFINED
#define _SIZE_T_DEFINED
#define _SIZE_T_DEFINED_
#if defined(_WIN64)
typedef unsigned __int64        size_t;
#else
typedef unsigned int            size_t;
typedef size_t                  _w_size_t;
#endif

#endif

#ifndef _CLOCK_T_DEFINED
#define _CLOCK_T_DEFINED
typedef long                    clock_t;
#endif

#elif defined(__linux__) || defined(__sun__)

typedef int                     INT32;
typedef unsigned int            UINT32;

#if !defined(__size_t) && !defined(_SIZE_T)
#define __size_t
#define _SIZE_T
typedef unsigned int            size_t;
#endif

#if !defined(__clock_t_defined) && !defined(_CLOCK_T)
#define __clock_t_defined
#define _CLOCK_T
typedef unsigned int            clock_t;
#endif

#endif

/****************************************************************************/
/* Definitions for alternate scalars                                        */
/****************************************************************************/

/* 32-bit scalars */

typedef UINT32 *                UINT32_PTR;     /* Pointer to unsigned 32-bit scalar */
typedef UINT32 *                PUINT32;        /* Pointer to unsigned 32-bit scalar */

typedef INT32 *                 INT32_PTR;      /* Pointer to signed 32-bit scalar   */
typedef INT32 *                 PINT32;         /* Pointer to signed 32-bit scalar   */

typedef UINT32                  ULONG;          /* Unsigned 32-bit scalar            */
typedef ULONG *                 PULONG;         /* Pointer to unsigned 32-bit scalar */
typedef ULONG *                 ULONG_PTR;      /* Pointer to unsigned 32-bit scalar */

typedef INT32                   LONG;           /* Signed 32-bit scalar              */
typedef LONG *                  PLONG;          /* Pointer to signed 32-bit scalar   */
typedef LONG *                  LONG_PTR;       /* Pointer to signed 32-bit scalar   */

typedef UINT32                  DWORD;          /* Unsigned 32-bit scalar            */
typedef DWORD *                 PDWORD;         /* Pointer to unsigned 32-bit scalar */
typedef DWORD *                 DWORD_PTR;      /* Pointer to unsigned 32-bit scalar */

typedef UINT32                  U32;            /* Unsigned 32 bit                   */

/* 16-bit scalars */

typedef UINT16 *                UINT16_PTR;     /* Pointer to unsigned 16-bit scalar */
typedef UINT16 *                PUINT16;        /* Pointer to unsigned 16-bit scalar */

typedef INT16 *                 INT16_PTR;      /* Pointer to signed 16-bit scalar   */
typedef INT16 *                 PINT16;         /* Pointer to signed 16-bit scalar   */

typedef UINT16                  USHORT;         /* Unsigned 16-bit scalar            */
typedef USHORT *                PUSHORT;        /* Pointer to unsigned 16-bit scalar */
typedef USHORT *                USHORT_PTR;     /* Pointer to unsigned 16-bit scalar */

typedef INT16                   SHORT;          /* Signed 16-bit scalar              */
typedef SHORT *                 PSHORT;         /* Pointer to signed 16-bit scalar   */
typedef SHORT *                 SHORT_PTR;      /* Pointer to signed 16-bit scalar   */

typedef UINT16                  WORD;           /* Unsigned 16-bit scalar            */
typedef WORD *                  PWORD;          /* Pointer to unsigned 16-bit scalar */
typedef WORD *                  WORD_PTR;       /* Pointer to unsigned 16-bit scalar */

typedef UINT16                  U16;            /* Unsigned 16-bit scalar            */
typedef INT16                   CHAR16;         /* Signed 16-bit scalar              */

/* 8-bit scalars */

typedef UINT8 *                 UINT8_PTR;      /* Pointer to unsigned 8-bit scalar  */
typedef UINT8 *                 PUINT8;         /* Pointer to unsigned 8-bit scalar  */

typedef INT8 *                  INT8_PTR;       /* Pointer to signed 8-bit scalar    */
typedef INT8 *                  PINT8;          /* Pointer to signed 8-bit scalar    */

typedef UINT8                   UBYTE;          /* Unsigned 8-bit scalar             */
typedef UBYTE *                 PUBYTE;         /* Pointer to unsigned 8-bit scalar  */
typedef UBYTE *                 UBYTE_PTR;      /* Pointer to unsigned 8-bit scalar  */

typedef UINT8                   BYTE;           /* Unsigned 8-bit scalar             */
typedef BYTE *                  PBYTE;          /* Pointer to unsigned 8-bit scalar  */
typedef BYTE *                  BYTE_PTR;       /* Pointer to unsigned 8-bit scalar  */

typedef UINT8                   U8;             /* Unsigned 8-bit scalar             */
typedef INT8                    CHAR;           /* Unsigned 8-bit scalar             */

/* open length scalars */

typedef unsigned int            UINT;           /* Unsigned Integer                  */


/****************************************************************************/
/* Miscellaneous definitions not provided by Windows header files           */
/****************************************************************************/

/** Declares a FAR pointer */
#define FAR _far

/** Moves a pointer from one base to another base */
#define AdjustPointer(ptr, orig_base, new_base) \
    ptr = (void *)((U32)ptr - (U32)orig_base + (U32)new_base)

/** Determine if a bit within a bit mask is set */
#define TestBits(x, bits) ((x) & (bits)) == (bits)

/**  CONTAINING_RECORD - returns a pointer to the structure from one of it's elements. */
#define ContainedRecord(Record, TYPE, Field)  \
    ((TYPE *) ( (UINT8 *)(Record) - (UINT8 *) &(((TYPE *) 0)->Field)))

/** Definition of an unsigned 64-bit Structure */
typedef struct _U64
{
   U32 l;  /**< Lower 32-bits of 64 bit structure */
   U32 h;  /**< Upper 32-bits of 64 bit structure */
} U64;

/** Definition of an unsigned 64-bit Structure */
typedef struct _uint64Split
{
   UINT32    Bits31_0;   /**< Bits 31 - 0  of 64 bit structure */
   UINT32    Bits63_32;  /**< BIts 63 - 32 of 64 bit structure */
}UINT64SPLIT;

// INT64 and long long cannot be used in tools compiled by watcom
#ifdef ARC
typedef union _uint64
{
   unsigned long long val;
   UINT64SPLIT split;
} UINT64;

typedef UINT64SPLIT INT64; /**< Signed 64 bit integer */
#endif

/** Character Definition */
typedef char TCHAR;

/** Defines a globally unique ID (GUID) */
typedef struct
{
   UINT32 Data1;       /**< DWORD 1 */
   UINT16 Data2;       /**< WORD  2 */
   UINT16 Data3;       /**< WORD  3 */
   UINT8  Data4[8];    /**< BYTE  4 Array */
} GUID, *PGUID;

/** Defines a GUID that is also accesibly as DWORDS */
typedef union {
   GUID     Guid;     /**< GUID Format Member */
   UINT32   Raw[4];   /**< RAW DWORD Member */
}GUID_UNION;

/** Generic HANDLE definition */
typedef void * HANDLE;

/** BOOLEAN definition, using natural variable */
typedef int BOOLEAN;
/** BOOLEAN definition, using natural variable */
typedef int BOOL;
//typedef int bool;

#ifndef TRUE
#define TRUE  1     /**< True value for a BOOLEAN */
#endif
#ifndef FALSE
#define FALSE 0     /**< False value for a BOOLEAN */
#endif

/* When in CPP don't compile these macros since they are natively keywords*/
#ifndef __cplusplus
#define true  1     /**< True value for a BOOLEAN */
#define false 0     /**< False value for a BOOLEAN */
#endif

#define True  1     /**< True value for a BOOLEAN */
#define False 0     /**< False value for a BOOLEAN */

#define CONST const       /**< Defines a CONST */

typedef void VOID;        /**< Defines a VOID */
typedef void * PVOID;     /**< Defines a VOID Pointer */

#ifdef INITGUID
/** When INITGUID is defined, the GUID is declared as a global,constant variable */
#define DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
        const GUID name \
                = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }
#else
/** When INITGUID is not defined, the GUID is declared as an external constant variable */
#define DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
    extern "C" const GUID name
#endif /* INITGUID */

/****************************************************************************/
/* If using Windows header files, just a few things of the things above are */
/* missing                                                                  */
/****************************************************************************/

#else /* _BASETSD_H_ */

typedef unsigned long           U32;
typedef unsigned short          U16;
typedef U16                     UINT16;
typedef short                   INT16;
typedef unsigned char           U8;
typedef U8                      UINT8;
typedef signed char             INT8;

#endif /* _BASETSD_H_ */

/****************************************************************************/
/* Miscellaneous definitions                                                */
/****************************************************************************/

/** Declaration of a Function pointer.  Function pointers are /4 on ARC */
typedef void (*FUNC_PTR)();

/** Declare a structure before it is used to allow of structure pointers within the structure */
#define FORWARD_DECLARATION(x)    typedef struct _##x x

/** Declare an interface structure. */
#define INTERFACE_DECL(x)         typedef struct x

/** Determine the number of elements within a array based on the size and the size of a single element */
#define NELEMENTS(a) (sizeof(a)/sizeof((a)[0]))

/** Calculate number of functions from protocol structure */
#define NUM_OF_FUNCS(protocol)      ((sizeof(protocol) / sizeof(PVOID)) - 2)

/** Determine the offset of a member within a structure */
#define OffsetOf(s,m) ((UINT)&(((s *)0)->m))

/** Determine the size of a member within a structure */
#define SizeOf(s,m) ((UINT)sizeof(((s *)0)->m))

typedef INT8            CHAR8;          /**< Signed 8 bit */
typedef int             INT;            /**< Integer (INT) */
typedef INT8*           CHAR_PTR;       /**< Signed 8 bit pointer */
typedef void *          VOID_PTR;       /**< Void Pointer */

#define IN                              /**< Defines an input parameter */
#define OUT                             /**< Defines an output parameter */
#define IO                              /**< Defines an input/output parameter */

/** Defines a DWORD, so that it can be accessed in U32, U16, or U8 */
typedef union
{
    UINT32 dw;    /**< DWORD Access */
    UINT16 w[2];  /**< WORD Access */
    UINT8  b[4];  /**< BYTE Access */

}S_DWORD;

#ifdef _WIN32
#pragma warning( disable : 4214 4200 )
/** Defines variable argument list on WIN32 */
typedef char * VA_LIST;
#else
/** Defines a variable argument list */
typedef void * VA_LIST;
#endif

/** Links the VA_ARG to stdarg.h va_arg */
#define VA_ARG(a, b) va_arg(a, b)

/** Links the VA_START to stdarg.h va_start */
#define VA_START(a, b) va_start(a, b)

/** Links the VA_END to stdarg.h va_end */
#define VA_END(a) va_end(a)

/** Definition of a Callback pointer */
typedef void * CALLBK;

#define VOLATILE volatile  /**< Definition of a Volatile variable */
#define STATIC   static    /**< Definition of a static variable */
#define CONST    const     /**< Definition of a const variable */
#define EXTERN   extern    /**< Definition of a extern variable */

#ifndef CLSID
typedef GUID CLSID;  /**< Class ID GUID Definition */
#endif

//Bit Definitions
#define BIT0   0x00000001   /**< Bit 0  Mask */
#define BIT1   0x00000002   /**< Bit 1  Mask */
#define BIT2   0x00000004   /**< Bit 2  Mask */
#define BIT3   0x00000008   /**< Bit 3  Mask */
#define BIT4   0x00000010   /**< Bit 4  Mask */
#define BIT5   0x00000020   /**< Bit 5  Mask */
#define BIT6   0x00000040   /**< Bit 6  Mask */
#define BIT7   0x00000080   /**< Bit 7  Mask */
#define BIT8   0x00000100   /**< Bit 8  Mask */
#define BIT9   0x00000200   /**< Bit 9  Mask */
#define BIT10  0x00000400   /**< Bit 10 Mask */
#define BIT11  0x00000800   /**< Bit 11 Mask */
#define BIT12  0x00001000   /**< Bit 12 Mask */
#define BIT13  0x00002000   /**< Bit 13 Mask */
#define BIT14  0x00004000   /**< Bit 14 Mask */
#define BIT15  0x00008000   /**< Bit 15 Mask */
#define BIT16  0x00010000   /**< Bit 16 Mask */
#define BIT17  0x00020000   /**< Bit 17 Mask */
#define BIT18  0x00040000   /**< Bit 18 Mask */
#define BIT19  0x00080000   /**< Bit 19 Mask */
#define BIT20  0x00100000   /**< Bit 20 Mask */
#define BIT21  0x00200000   /**< Bit 21 Mask */
#define BIT22  0x00400000   /**< Bit 22 Mask */
#define BIT23  0x00800000   /**< Bit 23 Mask */
#define BIT24  0x01000000   /**< Bit 24 Mask */
#define BIT25  0x02000000   /**< Bit 25 Mask */
#define BIT26  0x04000000   /**< Bit 26 Mask */
#define BIT27  0x08000000   /**< Bit 27 Mask */
#define BIT28  0x10000000   /**< Bit 28 Mask */
#define BIT29  0x20000000   /**< Bit 29 Mask */
#define BIT30  0x40000000   /**< Bit 30 Mask */
#define BIT31  0x80000000   /**< Bit 31 Mask */

#ifdef __cplusplus
}

#ifndef NULL
#define NULL                    0               /* Define NULL pointer for C++ */
#endif

#else /* ndef __cplusplus */

#ifndef NULL
#define NULL                    ((void *)0)     /* Define NULL pointer for C */
#endif

#endif /* ndef __cplusplus */

#endif /* ndef _TYPEDEF_H_ */

