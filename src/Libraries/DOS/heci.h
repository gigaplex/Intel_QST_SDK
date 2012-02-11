/****************************************************************************/
/*                                                                          */
/*  Module:         heci.h                                                  */
/*                                                                          */
/*  Description:    Provides all necessary definitions for the support  of  */
/*                  communication  with  firmware  entities running on the  */
/*                  Intel(R) Management Engine (ME).                        */
/*                                                                          */
/*  Notes:      1.  The Intel(R) Management Engine Interface (MEI),  which  */
/*                  is   used   to   facilitate  this  communication,  was  */
/*                  originally known as the Host  to  Embedded  Controller  */
/*                  Interface (HECI).                                       */
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

#ifndef _HECI_H_
#define _HECI_H_

#include <time.h>

#define HECI_INIT_TIMEOUT               (5 * CLOCKS_PER_SEC)
#define HECI_READ_TIMEOUT               (1 * CLOCKS_PER_SEC)
#define HECI_SEND_TIMEOUT               (1 * CLOCKS_PER_SEC)

#include "typedef.h"

#define MAX_HECI_BUFFER_SIZE            128 // Largest Circular Buffer allowed in Broadwater

#define NON_BLOCKING                    0
#define BLOCKING                        1

#define HECI_HOST_CB_WRITE_WINDOW_REG   0x00
#define HECI_HOST_CONTROL_REG           0x04
#define HECI_ME_CB_READ_WINDOW_REG      0x08
#define HECI_ME_CONTROL_HA_REG          0x0C

#define BIOS_FIXED_HOST_ADDR            1
#define PREBOOT_FIXED_ME_ADDR           1

#define MAX_NUM_OF_DRAM_CHANNELS        2
#define MAX_DIMMS_PER_CHANNEL           2


/****************** REGISTER EQUATES ****************************************************/

//ME_CSR_HA - ME Control Status Host Access

typedef union
{
    U32 ul;
    struct
    {
        U32  ME_IE_HRA              :1; // 0 - ME Interrupt Enable (Host Read Access)
        U32  ME_IS_HRA              :1; // 1 - ME Interrupt Status (Host Read Access)
        U32  ME_IG_HRA              :1; // 2 - ME Interrupt Generate (Host Read Access)
        U32  ME_RDY_HRA             :1; // 3 - ME Ready (Host Read Access)
        U32  ME_RST_HRA             :1; // 4 - ME Reset (Host Read Access)
        U32  Reserved               :3; // 7:5
        U32  ME_CBRP_HRA            :8; // 15:8 - ME CB Read Pointer (Host Read Access)
        U32  ME_CBWP_HRA            :8; // 23:16 - ME CB Write Pointer (Host Read Access)
        U32  ME_CBD_HRA             :8; // 31:24 - ME Circular Buffer Depth (Host Read Access)
    } r;
} HECIMEControlReg, * HECIMEControlRegPtr;
C_ASSERT(sizeof(HECIMEControlReg) == 4);

//H_CSR - Host Control Status

typedef union
{
    U32 ul;
    struct
    {
        U32  H_IE                   :1; // 0 - Host Interrupt Enable ME
        U32  H_IS                   :1; // 1 - Host Interrupt Status ME
        U32  H_IG                   :1; // 2 - Host Interrupt Generate
        U32  H_RDY                  :1; // 3 - Host Ready
        U32  H_RST                  :1; // 4 - Host Reset
        U32  Reserved               :3; // 7:5
        U32  H_CBRP                 :8; // 15:8 - Host CB Read Pointer
        U32  H_CBWP                 :8; // 23:16 - Host CB Write Pointer
        U32  H_CBD                  :8; // 31:24 - Host Circular Buffer Depth
    } r;
} HECIHostControlReg, * HECIHostControlRegPtr;
C_ASSERT(sizeof(HECIHostControlReg) == 4);

/****************** PROTOTYPES ******************************************/

int HECIInitialize(U32 HECIMemBar);
int HECISend(U32 * Message, U32 HECIMemBar, U32 Length, U8 HostAddress, U8 MEAddress);
int HECISendwACK(U32 * Message, U32 HECIMemBar, U32 SendLength, U32 * RecLength, U8 HostAddress, U8 MEAddress);

//Note:BIOS should not call HECIReceive directly
int HECIReceive(U32 Blocking, U32 HECIMemBar, U32 * MessageBody, U32 * Length, U8 HostAddress, U8 MEAddress);

#endif /* End of Sentry Header */
