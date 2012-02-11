/****************************************************************************/
/*                                                                          */
/*  Module:         heci.h                                                  */
/*                                                                          */
/*  Description:    Provides all necessary definitions for the support  of  */
/*                  the communication protocol established between runtime  */
/*                  software and firmware entities running on the Intel(R)  */
/*                  Management Engine (ME).                                 */
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

#ifndef  HECI_MSG_H
#define  HECI_MSG_H

#pragma pack(1)
//
//HECI Message Header.
//    Should be defined at a common place. Probably in hecihw.h. Until then including
//    _CBM prefix.
//
typedef union _CBM_HECI_MSG_HEADER
{
   UINT32   ul;
   struct
   {
      UINT32   MeAddress         :8;
      UINT32   HostAddress       :8;
      UINT32   Length            :9;
      UINT32   Reserved          :6;
      UINT32   MessageComplete   :1;
   } h;
}CBM_HECI_MSG_HEADER;

//
//CBM_Command
//    Command identifier for Core BIOS messages.
typedef union _CBM_COMMAND
{
   UINT8       Data;
   struct
   {
      UINT8    Command     :7;
      UINT8    IsResponse  :1;
   }Fields;
}CBM_COMMAND;

//
// HECI Client Address - Core Messages
//    Core messages to coordinate memory initialization and UMA allocation with ME
//    as well as to inform ME of the end of POST event
#define HECI_CLIENT_CORE_MSG_DISPATCHER                  0x01
#define HOST_FIXED_ADDRESS                               0x00

//
//CBM_MEM_INIT_STATUS
//    This message is sent by the system BIOS whenever it has completed memory
//    initialization after a S3 / S4 / S5 resume or during a G3-exit.  This message
//    will result in the HECI interface temporarily being unavailable (EP_RDY = 0)
//    while the ME system relocates the HECI buffers to ME UMA.  BIOS must not use
//    system memory until the DRAM Initialize Response message is received.
//    Note that this message describes the motherboard layout to ME and not necessarily
//    any information about actual population of these slots.
//
//Members:
//    Error             -  1 if BIOS encountered an error during memory initialization
//    Channel0          -  1 if Channel 0 is not populated
//    LeastCommonFreq   -  1 if the least common frequency supported is > DDR2-533

typedef union _CBM_MEM_INIT_STATUS
{
   U8  byte;
   struct
   {
      U8  Error               :1;
      U8  Channel0            :1;
      U8  LeastCommonFreq     :1;
      U8  Reserved            :5;
   } b;
} CBM_MEM_INIT_STATUS;

//
//CBM_DRAM_INITIALIZE_REQUEST
//    This message is sent by the system BIOS whenever it has completed memory
//    initialization after a S3 / S4 / S5 resume or during a G3-exit.  This message
//    will result in the HECI interface temporarily being unavailable (EP_RDY = 0)
//    while the ME system relocates the HECI buffers to ME UMA.  BIOS must not use
//    system memory until the DRAM Initialize Response message is received.
//    Note that this message describes the motherboard layout to ME and not necessarily
//    any information about actual population of these slots.
//
//Members:
//    Header            -  Heci message header: 0x800A0001
//                         ME Client ID   =  1
//                         Host Client ID =  0
//                         Length         =  A
//                         MessageComplete = 1
//    Command           -  0x08.
//    NumberOfChannels  -  One based number of DRAM channels supported by this motherboard.
//                         For 2006 GMCH devices this must be 1 or 2.
//    DimmsPerChannel   -  One based number of DIMM sockets present on the motherboard.
//                         For 2006 GMCH devices this must be 1 or 2.
//    MemInitStatus     -  BIOS communicates to ME whether there was any error in initializing
//                         memory or not. In case of no DIMM's. the code is set to ERROR.
//                         If only channel 0 is occupied then bit 0 is set to SUCCESS and bit 1
//                         reflects that channel 0 is not occupied.
//    MaxMemoryFreq     -  Maximum frequency supported in the system - this is the maximum common
//                         frequency of all DIMMs
//    MinMemoryFreq     -  Least common frequency supported in the system - this is the min common
//                         frequency of all DIMMs
//    SpdAddressMap    -  A doubly indexed array of bytes that describe each DIMM's SPD address.
//                         The SPD address is in bits 7:1 of the each byte, while bit 0 is reserved.
//
//    Refer to the BIOS Writer's Guide for more details.
//
#define MAX_NUM_OF_DRAM_CHANNELS    2
#define MAX_DIMMS_PER_CHANNEL       2

typedef struct _CBM_DRAM_INIT_RQ_DATA
{
   CBM_COMMAND          Command;
   UINT8                NumberOfChannels;
   UINT8                DimmsPerChannel;
   CBM_MEM_INIT_STATUS  MemInitStatus;
   UINT8                CurMemoryFreq;
   UINT8                MinCommonMemoryFreq;
   UINT8                SpdAddressMap[MAX_NUM_OF_DRAM_CHANNELS][MAX_DIMMS_PER_CHANNEL];
   UINT8                Reserved[2];
}CBM_DRAM_INIT_RQ_DATA;

typedef struct _CBM_DRAM_INITIALIZE_REQUEST
{
   CBM_HECI_MSG_HEADER     Header;
   CBM_DRAM_INIT_RQ_DATA   Data;
}CBM_DRAM_INITIALIZE_REQUEST;

//
//CBM_DRAM_INITIALIZE_RESPONSE
//    This message is sent by the ME to the host in response to the DRAM
//    Initialize Request message.  BIOS can start using the system DRAM only
//    after receiving this message.
//
//Members:
//    Header      -  Heci message header: 0x80030001
//                   ME Client ID      =  1
//                   Host Client ID    =  0
//                   Length            =  3
//                   MessageComplete   =  1
//    Command     -  0x88
//    BiosAction  -  What actions should BIOS take upon receiving this response
//                   message.
//                  DIR_CONTINUE_POST
//                   DIR_GO_TO_SX
//                   DIR_GO_TO_S3
//                   DIR_GO_TO_S4
//                   DIR_GO_TO_S5
//    MEFWFunctionalState - Describes if the ME subsystem is in a temporary
//                   disabled state
//                   CBM_DIR_ME_FW_ENABLED
//                   CBM_DIR_ME_FW_TEMP_DISABLED
//
//
typedef struct _CBM_DRAM_INIT_RP_DATA
{
   CBM_COMMAND             Command;
   UINT8                   BiosAction;
   UINT8                   MEFWFunctionalState;
   UINT8                   Reserved[1];
}CBM_DRAM_INIT_RP_DATA;

#define CBM_DRAM_INIT_RP_DATA_SIZE  3

typedef struct _CBM_DRAM_INITIALIZE_RESPONSE
{
   CBM_HECI_MSG_HEADER     Header;
   CBM_DRAM_INIT_RP_DATA   Data;
}CBM_DRAM_INITIALIZE_RESPONSE;

//
//CBM_DRAM_OUT_OF_SELF
//    This message is an indication from BIOS to ME that DRAM channels have been
//    brought out of self refresh by BIOS.
//
//Members:
//    Header      -  Heci message header: 0x80020001
//                   ME Client ID      =  1
//                   Host Client ID    =  0
//                   Length            =  2
//                   MessageComplete   =  1
//    Command     -  0x0A
//    Status      -  Indicates whether DRAM was successfully brought out of SR or
//                   not.
//                   SR_EXIT_SUCCESS
//                   SR_EXIT_FAILURE
//
typedef struct _CBM_DRAM_OUT_OF_SR_DATA
{
   CBM_COMMAND             Command;
   UINT8                   Status;
   UINT8                   Reserved[2];
}CBM_DRAM_OUT_OF_SR_DATA;

typedef struct _CBM_DRAM_OUT_OF_SELF_REFRESH
{
   CBM_HECI_MSG_HEADER     Header;
   CBM_DRAM_OUT_OF_SR_DATA Data;
}CBM_DRAM_OUT_OF_SELF_REFRESH;

//
//CBM_FLASH_ID_INFO_REQUEST
//    BIOS, after determining what Flash Vendor Specific Component is present
//    in the system, communicates this information to ME over HECI using message
//    defined below.  ME writes this info into the ME VSCC register as well as
//    into the ME region of the Flash.  On subsequent boots, ME will read the
//    Vendor Specific information from the ME Region of the Flash and write it
//    to the ME VSCC register directly.
//
//    Note that there is no ordering requirement on BIOS to send this message
//    in terms of placement within POST process except for that it must be sent
//    before END OF POST message is sent to ME. This message is sent by the
//    system BIOS after it is done with flash part identification process using
//    vendor specific algorithms.
//
//Members:
//    Header      -  Heci message header: 0x80080001
//                   ME Client ID      =  1
//                   Host Client ID    =  0
//                   Length            =  8
//                   MessageComplete   =  1
//    Command     -  0x09
//    VsccData    -  Data to be programmed in VSCC register
//
typedef struct _CBM_FLASH_ID_INFO_RQ_DATA
{
   CBM_COMMAND             Command;
   UINT8                   Reserved[3];
   UINT32                  VsccData;
}CBM_FLASH_ID_INFO_RQ_DATA;

typedef struct _CBM_FLASH_ID_INFO_REQUEST
{
   CBM_HECI_MSG_HEADER           Header;
   CBM_FLASH_ID_INFO_RQ_DATA     Data;
}CBM_FLASH_ID_INFO_REQUEST;

//
//CBM_END_OF_POST
//    At the end of BIOS POST (defined as the place right before calling int19h
//    or calling OS resume vector in case of S3 exit), BIOS sends a "END OF PREBOOT"
//    HECI message to ME declaring end of POST and start of OS load.
//
//    Sending this message at the right location is extremely important for maintaining
//    platform security and ensuring that manageability functions are not compromised
//    by rogue software running under OS's premise.
//
//    Note that this message needs to be sent during S3 exit also because ME might
//    be in Moff while host is in S3 and waking up along with the host.
//
//Members:
//    Header            -  Heci message header: 0x80080001
//                         ME Client ID      =  1
//                         Host Client ID    =  0
//                         Length            =  8
//                         MessageComplete   =  1
//    Command           -  0x0A
//    AmtXExitCode      -  If host is booting from S4/5/G3 then this field contains AMTx
//                         (aka PTBx exit code [31:0] that are returned from PTBx module.
//                         Else if host is exiting from S3 state, this field contains 0
//
typedef struct _CBM_EOP_DATA
{
   CBM_COMMAND             Command;
   UINT8                   Reserved[3];
   UINT32                  AmtXExitCode;
}CBM_EOP_DATA;

typedef struct _CBM_END_OF_POST
{
   CBM_HECI_MSG_HEADER     Header;
   CBM_EOP_DATA            Data;
}CBM_END_OF_POST;

//
//CBM_END_OF_POST_RESPONSE
//    This message is sent by the ME to the host in response to the END OF POST
//    message.  BIOS can proceed only after receiving this response.
//    BIOS doesn't wait for a response when making S3-exit
//    In S4/5-exit or G3-exit, BIOS waits to receive a response within 5 seconds.
//    If it doesn't receive a response, it should halt with a warning message to user.
//
//    BIOS must wait for a response for the END_OF_POST request message because there
//    is a boundary case where host s/w starts loading before ME has processed this
//    message. In this case, a rogue host s/w (e.g. boot virus) can potentially
//    overwrite the END OF POST message before it has been serviced by ME by
//    continuously writing content in HECI circular buffer. HECI H/W doesn't provide
//    protection against overrun and if the write pointer reaches the location in
//    HECI buffer where END OF POST message is written then it will be overwritten
//    by next write.
//
//    If successful this message take ME out of Pre-Boot mode and ME transitions
//    from Pre Boot to OS mode.  The following PTHI local host interface requests
//    are not allowed this point onwards (please refer to the latest PTHI spec for
//    details):
//    All STATE class messages
//    All HWAI class requests used for inventory table updating
//    All CFG class requests except CFG_GetInterfaceVersion and CFG_GetMaximumTransferUnit
//
//Members:
//    Header            -  Heci message header: 0x80010001
//                         ME Client ID      =  1
//                         Host Client ID    =  0
//                         Length            =  1
//                         MessageComplete   =  1
//    Command           -  0x8A
//
typedef struct _CBM_EOP_RP_DATA
{
   CBM_COMMAND             Command;
   UINT8                   Reserved[3];
}CBM_EOP_RP_DATA;

typedef struct _CBM_END_OF_POST_RESPONSE
{
   CBM_HECI_MSG_HEADER     Header;
   CBM_EOP_RP_DATA         Data;
}CBM_END_OF_POST_RESPONSE;

//
//CBM_RESET_REQUEST
//    This message is sent by the system BIOS, AMTBx binary or host, when it
//    needs to do a global reset of the system and either of following conditions
//    is true:
//    1. BIOS is in boot block and ME is in M1 (SX controller is initialized)
//    2. BIOS is in POST (has exited the boot block) and ME f/w status code is
//       initializing or initialized (doesn't reflect ERROR)
//    3. PTBx binary is requesting the global reset
//
//Members:
//    Header            -  Heci message header: 0x80030001
//                         ME Client ID      =  1
//                         Host Client ID    =  0
//                         Length            =  3
//                         MessageComplete   =  1
//    Command           -  0x0B
//    RequestOrigin     -  A constant describing where this request is originating
//                         from in the host.
//                            RR_REQ_ORIGIN_BIOS_MEMORY_INIT
//                            RR_REQ_ORIGIN_BIOS_POST
//                            RR_REQ_ORIGIN_AMTBX_ LAN_DISABLE
//    ResetType         -  Type of Reset to be performed.
//                            RR_GLOBAL_RESET
//                            RR_HOST_RESET_ONLY
typedef struct _CBM_RESET_RQ_DATA
{
   CBM_COMMAND             Command;
   UINT8                   RequestOrigin;
   UINT8                   ResetType;
   UINT8                   Reserved;
}CBM_RESET_RQ_DATA;

typedef struct _CBM_RESET_REQUEST
{
   CBM_HECI_MSG_HEADER     Header;
   CBM_RESET_RQ_DATA       Data;
}CBM_RESET_REQUEST;

//
//CBM_RESET_RESPONSE
//    This message is sent by the system BIOS, AMTBx binary or host, when it
//    needs to do a global reset of the system and either of following conditions
//    is true:
//    1. BIOS is in boot block and ME is in M1 (SX controller is initialized)
//    2. BIOS is in POST (has exited the boot block) and ME f/w status code is
//       initializing or initialized (doesn't reflect ERROR)
//    3. PTBx binary is requesting the global reset
//
//Members:
//    Header            -  Heci message header: 0x80020001
//                         ME Client ID      =  1
//                         Host Client ID    =  0
//                         Length            =  2
//                         MessageComplete   =  1
//    Command           -  0x0B
//    ReasonForRejection-  Reason for rejecting the global reset request:
//                            RES_REQ_NOT_ACCEPTED - PTHI is closed
//
typedef struct _CBM_RESET_RP_DATA
{
   CBM_COMMAND             Command;
   UINT8                   ReasonForRejection;
   UINT8                   Reserved[2];
}CBM_RESET_RP_DATA;

typedef struct _CBM_RESET_RESPONSE
{
   CBM_HECI_MSG_HEADER     Header;
   CBM_RESET_RP_DATA       Data;
}CBM_RESET_RESPONSE;

//
//HECI Header Definitions for Core BIOS Messages
//
#define CBM_DRAM_INITIALIZE_REQUEST_HECI_HDR             0x800A0001
#define CBM_DRAM_INITIALIZE_RESPONSE_HECI_HDR            0x80030001
#define CBM_DRAM_OUT_OF_SELF_REFRESH_HECI_HDR            0x80020001
#define CBM_DRAM_OUT_OF_SELF_REFRESH_ACK_HECI_HDR        0x80010001
#define CBM_FLASH_ID_INFO_REQUEST_HECI_HDR               0x80050001
#define CBM_END_OF_POST_HECI_HDR                         0x80080001
#define CBM_END_OF_POST_RESPONSE_HECI_HDR                0x80010001
#define CBM_RESET_REQUEST_HECI_HDR                       0x80030001
#define CBM_RESET_RESPONSE_HECI_HDR                      0x80020001

//
//Core Bios Message Command Identifiers.
//
#define CBM_DRAM_INITIALIZE_REQUEST_CMD                  0x08
#define CBM_DRAM_INITIALIZE_RESPONSE_CMD                 0x88
#define CBM_FLASH_ID_INFO_REQUEST_CMD                    0x09
#define CBM_DRAM_OUT_OF_SELF_REFRESH_CMD                 0x0A
#define CBM_DRAM_OUT_OF_SELF_REFRESH_ACK_CMD             0x8A
#define CBM_RESET_REQUEST_CMD                            0x0B
#define CBM_RESET_RESPONSE_CMD                           0x8B
#define CBM_END_OF_POST_CMD                              0x0C
#define CBM_END_OF_POST_RESPONSE_CMD                     0x8C

//
//Enumerations used in Core BIOS Messages
//

// MemInitStatus codes
#define CBM_DIS_ERROR                                    0x01
#define CBM_DIS_CH0_NOT_POPULATED                        0x02
#define CBM_DIS_MIN_FREQ_ABOVE_533                       0x04

// MaxMemoryFreq Enum
#define CBM_DIS_DRAM_400MHZ                              0x01
#define CBM_DIS_DRAM_533MHZ                              0x02
#define CBM_DIS_DRAM_667MHZ                              0x03
#define CBM_DIS_DRAM_800MHZ                              0x04

// MinMemoryFreq Enum
#define CBM_DIS_DRAM_400MHZ                              0x01
#define CBM_DIS_DRAM_533MHZ                              0x02
#define CBM_DIS_DRAM_667MHZ                              0x03
#define CBM_DIS_DRAM_800MHZ                              0x04

//DRAM Initiailization Response Codes.
#define CBM_DIR_CONTINUE_POST                            0x00
#define CBM_DIR_GO_TO_SX                                 0x01
#define CBM_DIR_RESERVED                                 0x02
#define CBM_DIR_GO_TO_S3                                 0x03
#define CBM_DIR_GO_TO_S4                                 0x04
#define CBM_DIR_GO_TO_S5                                 0x05
#define CBM_DIR_ME_FW_ENABLED                            0x00
#define CBM_DIR_ME_FW_TEMP_DISABLED                      0x01

//DRAM Out of Self Refresh Codes
#define CBM_SR_EXIT_SUCCESS                              0x01
#define CBM_SR_EXIT_FAILURE                              0x80

//End Of Post Codes.
#define CBM_EOP_EXITING_G3                               0x01
#define CBM_EOP_RESERVED                                 0x02
#define CBM_EOP_EXITING_S3                               0x03
#define CBM_EOP_EXITING_S4                               0x04
#define CBM_EOP_EXITING_S5                               0x05

//Reset Request Origin Codes.
#define CBM_RR_REQ_ORIGIN_BIOS_MEMORY_INIT               0x01
#define CBM_RR_REQ_ORIGIN_BIOS_POST                      0x02
#define CBM_RR_REQ_ORIGIN_AMTBX_ LAN_DISABLE             0x03

//Reset Type Codes.
#define CBM_HRR_GLOBAL_RESET                             0x01
#define CBM_HRR_HOST_RESET_ONLY                          0x02

//Reset Response Codes.
#define CBM_HRR_RES_REQ_NOT_ACCEPTED                     0x01

#pragma pack()

#endif
