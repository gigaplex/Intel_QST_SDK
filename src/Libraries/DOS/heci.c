/****************************************************************************/
/*                                                                          */
/*  Module:         heci.c                                                  */
/*                                                                          */
/*  Description:    Implements  support  for  DOS-based  applications   to  */
/*                  communicate  with  firmware  entities  running  on the  */
/*                  Intel(R) Management Engine (ME).                        */
/*                                                                          */
/*  Notes:      1.  This code was derived from the sample code  originally  */
/*                  distributed  to  BIOS developers. It differs from that  */
/*                  code only with its (completed) support for timeouts.    */
/*                                                                          */
/*              2.  The Intel(R) Management Engine Interface (MEI),  which  */
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

/*** Include Files
 **************************/

#include <stdio.h>
#include <conio.h>
#include <time.h>

#include "typedef.h"
#include "heci.h"
#include "HeciMsg.h"


/*** Global Declarations
 **************************/

//
// Begin HECI Workaround
//

#define PCI_VID_DID_OFFSET       0x00
#define HECI_BROADWATER_A0_VID   0x8086
#define HECI_BROADWATER_A0_DID   0x2984

#define PCI_REVID_OFFSET         0x08
#define PCI_REVID_MSK            0x000000FF
#define HECI_BROADWATER_A0_REVID 0x00

#define PCI_FWSTATUS_OFFSET      0x40

typedef union _HECI_FWS_REGISTER
{
   UINT32   ul;
   struct
   {
      UINT32  CurrentState:4;
      UINT32  Reserved:6;
      UINT32  MeIsrCount:2;
      UINT32  ErrorCode:4;
      UINT32  ExtendedData:16;
   } r;
} HECI_FWS_REGISTER;

//
// End HECI Workaround
//


/*** Local Declarations
 **************************/
int HECIPacketRead(U32 Blocking, U32 HECIMemBar, CBM_HECI_MSG_HEADER * MessageHeader, U32 * MessageData, U32 * Length);
int HECIPacketWrite(U32 HECIMemBar, CBM_HECI_MSG_HEADER * MessageHeader, U32 * MessageData);
U8 FilledSlots(U32 ReadPointer, U32 WritePointer);
int OverflowCB(U32 ReadPointer, U32 WritePointer, U32 BufferDepth);
void StartTimer(U32 * start, U32 * end, U32 time);
int Timeout(U32 start, U32 end);
U32 HECIReadPCI(U8 offset);

/*** Global Functions
 **************************/

/*++
********************************************************************************
**
** FUNCTION:
**   HECIInitialize
**
** DESCRIPTION:
**   Prepare the host side of the HECI interface to send/receive HECI messages.
**   Corresponds to HECI HPS (part of) section 4.1.1.1
**
** ARGUMENTS:
**   none
**
** RETURNS:
**   0 - FAILURE.  Function timed out while waiting on ME_RDY_HRA
         to be set
**   1 - SUCCESS. The HECI Interface was successfully initialized
**
********************************************************************************
--*/

int HECIInitialize(U32 HECIMemBar)
{
    U32 timer_start, timer_end;
    HECIHostControlReg heci_reg_H_CSR;

    volatile HECIHostControlReg * heci_reg_H_CSR_ptr = (void *)(HECIMemBar + HECI_HOST_CONTROL_REG);
    volatile HECIMEControlReg * heci_reg_ME_CSR_HA_ptr = (void *)(HECIMemBar + HECI_ME_CONTROL_HA_REG);

    // Start 5 second timeout
    StartTimer(&timer_start, &timer_end, HECI_INIT_TIMEOUT);
    while (heci_reg_ME_CSR_HA_ptr->r.ME_RDY_HRA == 0)
    {
        // If 5 second timeout has expired, return fail
        if (Timeout(timer_start, timer_end) == 1)
            return 0;
    }

    heci_reg_H_CSR.ul = heci_reg_H_CSR_ptr->ul;
    // Set H_RDY if it is not set
    if (heci_reg_H_CSR_ptr->r.H_RDY == 0)
    {
        // Clear the host reset, set the host ready bit
        heci_reg_H_CSR.r.H_RST = 0;
        heci_reg_H_CSR.r.H_RDY = 1;
        heci_reg_H_CSR.r.H_IG = 1;
        heci_reg_H_CSR_ptr->ul = heci_reg_H_CSR.ul;
    }

    return 1;
}

/*++
********************************************************************************
**
** FUNCTION:
**   HECISendwACK
**
** DESCRIPTION:
**   Function sends one messsage through the HECI circular buffer, and waits
**   for the corresponding ACK message
**
** ARGUMENTS:
**   [IN] Message - pointer to a HECI message (including command) to send
**   [IN] Length - Length (in bytes) of message including command field
**
** RETURNS:
**   0 - FAILURE.  This will return failure on h/w error or timeout (no room in CB)
**   1 - SUCCESS. A message was successfully sent and acknowledge was received
**
********************************************************************************
--*/
int HECISendwACK(U32 * Message, U32 HECIMemBar, U32 SendLength, U32 * RecLength, U8 HostAddress, U8 MEAddress)
{
    U32 BufferLength;

    BufferLength = SendLength;
    if (HECISend(Message, HECIMemBar, BufferLength, BIOS_FIXED_HOST_ADDR, PREBOOT_FIXED_ME_ADDR) == 0)
        return 0;

    return HECIReceive(BLOCKING, HECIMemBar, Message, RecLength, HostAddress, MEAddress);
}

/*++
********************************************************************************
**
** FUNCTION:
**   HECIReceive
**
** DESCRIPTION:
**   Function to read one message (single or multi-part) from the HECI Interface
**
** ARGUMENTS:
**   [IN] Blocking - set to BLOCKING to not return until one message is received
**                 - set to NON_BLOCKING to return one message, but only if the
**                   circular buffer is non-empty when called
**   [IN] MessageBody - pointer to address of a buffer to store the HECI message
**         which corresponds to the first instance of a message read from the
**         HECI circular buffer (starting from the read pointer).
**   [IN/OUT] Length - pointer to an unsigned long int which on calling indicates
**         the size of the caller's buffer (in bytes).  On return, it will indicate the size
**         of the message (in bytes) received.
**
** RETURNS:
**   0 - FAILURE.  On BLOCKING, this will only return failure on h/w error,
**            timeout (message started, but did not complete), or if the caller's
**            buffer was too small. On NON_BLOCKING a 0 will also indicate if the
**            circular buffer was empty.
**   1 - SUCCESS. A message was successfully returned in the caller's buffer
**
********************************************************************************
--*/

int HECIReceive(U32 Blocking, U32 HECIMemBar, U32 * MessageBody, U32 * Length, U8 HostAddress, U8 MEAddress)
{
    CBM_HECI_MSG_HEADER PacketHeader;
    U32 CurrentLength = 0;      // in Bytes
    U32 MessageComplete = 0;
    U32 ReadError, PacketBuffer, timer_start, timer_end;

    StartTimer(&timer_start, &timer_end, HECI_READ_TIMEOUT);
    while ((CurrentLength < *Length) && (MessageComplete == 0))
    {
        // If 1 second timeout has expired, return fail as we have not yet received a full message
        if (Timeout(timer_start, timer_end) == 1)
            return 0;

        PacketBuffer = *Length - CurrentLength;
        ReadError = HECIPacketRead(Blocking, HECIMemBar, &PacketHeader, (U32 *) &MessageBody[CurrentLength/4], &PacketBuffer);

        // Check for error condition, pass forward to caller
        if (ReadError == 0)
        {
            *Length = 0;
            return 0;
        }

        MessageComplete = PacketHeader.h.MessageComplete;

        // Check for zero length messages
        if (PacketBuffer == 0)
        {
            // if we are not in the middle of a message, and we see Message Complete,
            // this is a valid zero-length message
            if ((CurrentLength == 0) && (MessageComplete == 1))
            {
                *Length = 0;
                return 1;
            }
            // we should not expect a zero-length message packet except in the circumstance
            // described above, return error here
            else
            {
                *Length = 0;
                return 0;
            }
        }

        // Track the length of what we have read so far
        CurrentLength += PacketBuffer;

    }

    // if we get here the message should be complete, if it is not
    // the caller's buffer was not large enough--return error
    if (MessageComplete == 0)
    {
        *Length = 0;
        return 0;
    }

    *Length = CurrentLength;
    return 1;

}

/*++
********************************************************************************
**
** FUNCTION:
**   HECIPacketRead
**
** DESCRIPTION:
**   Function to pull one messsage packet off the HECI circular buffer
**   Corresponds to HECI HPS (part of) section 4.2.4
**
** ARGUMENTS:
**   [IN] Blocking - set to BLOCKING to not return until one message packet is received
**                 - set to NON_BLOCKING to return one message, but only if the
**                   circular buffer is non-empty when called
**   [OUT] MessageHeader - pointer to address of a buffer to store the HECI message header
**   [OUT] MessageData - pointer to address of a buffer to store the HECI message packet
**                 NOTE: Caller must ensure this is the size of largest message
**                       or the circular buffer (whichever is smaller)
**   [IN/OUT] Length - pointer to an unsigned long int which on calling indicates
**         the size of the caller's buffer.  On return, it will indicate the size
**         of the message (in bytes) received. This buffer cannot be larger than
**         the circular buffer
**
** RETURNS:
**   0 - FAILURE.  On BLOCKING, this will only return failure on h/w error,
**            timeout (message started, but did not complete), or if the caller's
**            buffer was too small. On NON_BLOCKING a 0 will also indicate if the
**            circular buffer was empty.
**   1 - SUCCESS. A message was successfully returned in the caller's buffer
**
********************************************************************************
--*/

int HECIPacketRead(U32 Blocking, U32 HECIMemBar, CBM_HECI_MSG_HEADER * MessageHeader, U32 * MessageData, U32 * Length)
{
    int gotMessage = FALSE;
    U32 timer_start, timer_end, timer_start1, timer_end1;
    U32 i;
    HECIMEControlReg heci_reg_ME_CSR_HA;

    volatile HECIHostControlReg * heci_reg_H_CSR_ptr = (void *)(HECIMemBar + HECI_HOST_CONTROL_REG);
    volatile HECIMEControlReg * heci_reg_ME_CSR_HA_ptr = (void *)(HECIMemBar + HECI_ME_CONTROL_HA_REG);
    volatile U32 * heci_reg_ME_CBRW_ptr = (void *)(HECIMemBar + HECI_ME_CB_READ_WINDOW_REG);

    // for HECI workaround
    HECI_FWS_REGISTER FwStatus;
    int OldMeIsrCount;
    // end HECI Workaround

    // clear Interrupt Status bit
    heci_reg_H_CSR_ptr->r.H_IS = 1;

    // test for circular buffer overflow
    heci_reg_ME_CSR_HA.ul = heci_reg_ME_CSR_HA_ptr->ul;
    if (OverflowCB(heci_reg_ME_CSR_HA.r.ME_CBRP_HRA, heci_reg_ME_CSR_HA.r.ME_CBWP_HRA, heci_reg_ME_CSR_HA.r.ME_CBD_HRA) == 1)
    {
        // if we get here, the circular buffer is overflowed
        *Length = 0;
        return 0;
        // NOTE: HPS says to do a Host Initiated HECI Interface reset (as outlined in HPS 4.1.1.1)
        // *FB* We should not bother, f/w won't expect it at this point and is probably dead
        //      if we can get here
        //   - Talked to Will, he agrees we don't need to do this
    }

    // If NON_BLOCKING, exit if the circular buffer is empty
    heci_reg_ME_CSR_HA.ul = heci_reg_ME_CSR_HA_ptr->ul;;
    if ((FilledSlots(heci_reg_ME_CSR_HA.r.ME_CBRP_HRA, heci_reg_ME_CSR_HA.r.ME_CBWP_HRA) == 0) // out of messages in buffer if RP == WP
          && (Blocking == NON_BLOCKING))
    {
        *Length = 0;
        return 0;
    }

    // Start timeout counter
    StartTimer(&timer_start, &timer_end, HECI_READ_TIMEOUT);

    // loop until we get a message packet
    while (!gotMessage)
    {
        // If 1 second timeout has expired, return fail as we have not yet received a full message
        if (Timeout(timer_start, timer_end) == 1)
        {
            *Length = 0;
            return 0;
        }

        // read one message from HECI buffer and advance read pointer
        heci_reg_ME_CSR_HA.ul = heci_reg_ME_CSR_HA_ptr->ul;;
        if (FilledSlots(heci_reg_ME_CSR_HA.r.ME_CBRP_HRA, heci_reg_ME_CSR_HA.r.ME_CBWP_HRA) > 0)   // make sure we don't pass write pointer
        {
            //
            // Workaround for HECI Dropped DWORD hardware bug
            //

            if ((HECIReadPCI(PCI_VID_DID_OFFSET) == HECI_BROADWATER_A0_VID + (HECI_BROADWATER_A0_DID << 16)) &&
                ((HECIReadPCI(PCI_REVID_OFFSET) & PCI_REVID_MSK) == HECI_BROADWATER_A0_REVID))
            {

               // Wait for hardware to clear H_IG
               while (heci_reg_H_CSR_ptr->r.H_IG == 1) {}

               // Get the old MeIsrCount value from PCI cfg space
               FwStatus.ul = HECIReadPCI(PCI_FWSTATUS_OFFSET);
               OldMeIsrCount = FwStatus.r.MeIsrCount;

               // Generate an interrupt to ME. This will cause the ME ISR to write to
               // the ME_CSR register (clearing the bad hardware flag). It will then
               // update the count value so we know its done.
               heci_reg_H_CSR_ptr->r.H_IG = 1;

               do
               {
                  FwStatus.ul = HECIReadPCI(PCI_FWSTATUS_OFFSET);
               } while (FwStatus.r.MeIsrCount == OldMeIsrCount);
            }

            //
            // End Workaround
            //

            // eat the HECI Message header
            MessageHeader->ul = *heci_reg_ME_CBRW_ptr;

            // Just return success if Length is 0
            if  (MessageHeader->h.Length == 0)
            {
                // Set Interrupt Generate bit
                heci_reg_H_CSR_ptr->r.H_IG = 1;

                *Length = 0;
                return 1;
            }

            // Test for a valid packet size--cannot be larger than the circular buffer
            if ((MessageHeader->h.Length + 4) > (heci_reg_ME_CSR_HA_ptr->r.ME_CBD_HRA * 4))
            {
                //*FB* need to work out what to do here. This is an error by the ME firmware
                //    - Just let host continue as the ME firmware is probably dead

                *Length = 0;
                return 0;
            }

            // Test for a valid packet size--cannot be larger than the caller's buffer or the circular buffer
            if  ((MessageHeader->h.Length) <= *Length)
            {
                // Start timeout counter for inner loop
                StartTimer(&timer_start1, &timer_end1, HECI_READ_TIMEOUT);

                // wait here until entire message is present in ciruclar buffer
                heci_reg_ME_CSR_HA.ul = heci_reg_ME_CSR_HA_ptr->ul;
                while (((MessageHeader->h.Length + 3) / 4) > FilledSlots(heci_reg_ME_CSR_HA.r.ME_CBRP_HRA, heci_reg_ME_CSR_HA.r.ME_CBWP_HRA))
                {
                    // wait until message is complete in buffer
                    // If 1 second timeout has expired, return fail as we have not yet received a full message
                    if (Timeout(timer_start1, timer_end1) == 1)
                    {
                        *Length = 0;
                        return 0;
                    }
                    heci_reg_ME_CSR_HA.ul = heci_reg_ME_CSR_HA_ptr->ul;
                }

                //
                // Workaround for HECI Dropped DWORD hardware bug
                //

                if ((HECIReadPCI(PCI_VID_DID_OFFSET) == HECI_BROADWATER_A0_VID + (HECI_BROADWATER_A0_DID << 16)) &&
                    ((HECIReadPCI(PCI_REVID_OFFSET) & PCI_REVID_MSK) == HECI_BROADWATER_A0_REVID))
                {

                   // Wait for hardware to clear H_IG
                   while (heci_reg_H_CSR_ptr->r.H_IG == 1) {}

                   // Get the old MeIsrCount value from PCI cfg space
                   FwStatus.ul = HECIReadPCI(PCI_FWSTATUS_OFFSET);
                   OldMeIsrCount = FwStatus.r.MeIsrCount;

                   // Generate an interrupt to ME. This will cause the ME ISR to write to
                   // the ME_CSR register (clearing the bad hardware flag). It will then
                   // update the count value so we know its done.
                   heci_reg_H_CSR_ptr->r.H_IG = 1;

                   do
                   {
                      FwStatus.ul = HECIReadPCI(PCI_FWSTATUS_OFFSET);
                   } while (FwStatus.r.MeIsrCount == OldMeIsrCount);
                }

                //
                // End Workaround
                //

                // copy rest of message
                for (i= 0; i<((MessageHeader->h.Length + 3) / 4); i++)
                {
                    MessageData[i]  = *heci_reg_ME_CBRW_ptr;
                }

                gotMessage = TRUE;
                *Length = MessageHeader->h.Length;

            }
            // Message packet is larger than caller's buffer
            else
            {
                //*FB* need to work out what to do here. This could be an error by the caller
                //    and not the host
                //    - Just let host continue
                *Length = 0;
                return 0;
            }
        }
    }

    // Read ME_CSR_HA.  If the ME_RDY bit is 0, then an ME reset occurred during the
    // transaction and the message should be discarded as bad data may have been retrieved
    // from the host's circular buffer
    if (heci_reg_ME_CSR_HA_ptr->r.ME_RDY_HRA == 0)
    {
        *Length = 0;
        return 0;
    }

    // Set Interrupt Generate bit
    heci_reg_H_CSR_ptr->r.H_IG = 1;

    return 1;
}

/*++
********************************************************************************
**
** FUNCTION:
**   HECISend
**
** DESCRIPTION:
**   Function sends one messsage (of any length) through the HECI circular buffer
**
** ARGUMENTS:
**   [IN] Message - pointer to a HECI message (including command) to send
**   [IN] Length - Length (in bytes) of message including command field
**
** RETURNS:
**   0 - FAILURE.  This will return failure on h/w error or timeout (no room in CB)
**   1 - SUCCESS. A message was successfully sent
**
********************************************************************************
--*/

int HECISend(U32 * Message, U32 HECIMemBar, U32 Length, U8 HostAddress, U8 MEAddress)
{
    U32 CBLength;           // DWORDs
    U32 SendLength;         // DWORDs
    U32 CurrentLength = 0;  // bytes
    CBM_HECI_MSG_HEADER MessageHeader;

    volatile HECIHostControlReg * heci_reg_H_CSR_ptr = (void *)(HECIMemBar + HECI_HOST_CONTROL_REG);

    // Grab Circular Buffer length
    CBLength = heci_reg_H_CSR_ptr->r.H_CBD;

    // Prepare message header
    MessageHeader.ul = 0;
    MessageHeader.h.MeAddress = MEAddress;
    MessageHeader.h.HostAddress = HostAddress;

    // Break message up into CB-sized packets and loop until completely sent
    while (Length > CurrentLength)
    {
        // Set the Message Complete bit if this is our last packet in the message
        if ((((Length - CurrentLength) + 3) / 4) < CBLength)  // needs to be 'less than' to account for the header
        {
            MessageHeader.h.MessageComplete = 1;
        }

        // Calculate length for Message Header
        //    header length == smaller of circular buffer or remaining message (both account for the size of the header)
        SendLength = ((CBLength < (((Length - CurrentLength) + 3) / 4)) ? ((CBLength - 1) * 4) : (Length - CurrentLength));
        MessageHeader.h.Length = SendLength;

        // send the current packet (CurrentLength can be treated as the index into the message buffer)
        if (HECIPacketWrite(HECIMemBar, &MessageHeader, (U32 *)((U32)Message + CurrentLength)) == 0)
            return 0;

        CurrentLength += SendLength;
    }
    return 1;
}

/*++
********************************************************************************
**
** FUNCTION:
**   HECIPacketWrite
**
** DESCRIPTION:
**   Function sends one messsage packet through the HECI circular buffer
**   Corresponds to HECI HPS (part of) section 4.2.3
**
** ARGUMENTS:
**   [IN] MessageHeader - pointer to a HECI message header that has been already
**         loaded with the correct fields (ME/Host address, length, message complete)
**                 NOTE: Caller must ensure the overall length is less than or
**                       equal to the length of the circular buffer
**   [IN] MessageData - Buffer to send through the circular buffer (of size
**         described in the header)
**
** RETURNS:
**   0 - FAILURE.  This will return failure on h/w error or timeout (no room in CB)
**   1 - SUCCESS. A message was successfully sent
**
********************************************************************************
--*/

int HECIPacketWrite(U32 HECIMemBar, CBM_HECI_MSG_HEADER * MessageHeader, U32 * MessageData)
{
    U32 length; //in DWORDs
    U32 timer_start, timer_end;
    U32 i;
    HECIHostControlReg heci_reg_H_CSR;

    volatile HECIHostControlReg * heci_reg_H_CSR_ptr = (void *)(HECIMemBar + HECI_HOST_CONTROL_REG);
    volatile HECIMEControlReg * heci_reg_ME_CSR_HA_ptr = (void *)(HECIMemBar + HECI_ME_CONTROL_HA_REG);
    volatile U32 * heci_reg_H_CBWW_ptr = (void *)(HECIMemBar + HECI_HOST_CB_WRITE_WINDOW_REG);

    // Start timeout counter
    StartTimer(&timer_start, &timer_end, HECI_SEND_TIMEOUT);

    // Wait until ME f/w is ready
    while (heci_reg_ME_CSR_HA_ptr->r.ME_RDY_HRA == 0)
    {
         // If 1 second timeout has expired, return fail
         if (Timeout(timer_start, timer_end) == 1)
         {
            return 0;
         }
    }

    // Wait until there is sufficient room in the circular buffer
    heci_reg_H_CSR.ul = heci_reg_H_CSR_ptr->ul;
    while ( ((MessageHeader->h.Length + 3) / 4) >
         (heci_reg_H_CSR.r.H_CBD - FilledSlots(heci_reg_H_CSR.r.H_CBRP, heci_reg_H_CSR.r.H_CBWP)))
   {
         // If 1 second timeout has expired, return fail as the circular buffer never emptied
         if (Timeout(timer_start, timer_end) == 1)
         {
            return 0;
         }
         heci_reg_H_CSR.ul = heci_reg_H_CSR_ptr->ul;
    }

    // Write Message Header
    *heci_reg_H_CBWW_ptr = MessageHeader->ul;

    // Write Message Body
    length = (MessageHeader->h.Length + 3) / 4;
    for (i=0; i<length; i++)
    {
        *heci_reg_H_CBWW_ptr = MessageData[i];

    }

    // Set Interrupt Generate bit
    heci_reg_H_CSR_ptr->r.H_IG = 1;

    // Test if ME Ready bit is set to 1, if set to 0 a fatal error occured during
    // the transmission of this message.
    if (heci_reg_ME_CSR_HA_ptr->r.ME_RDY_HRA == 0)
    {
        // NOTE: HPS says to do a Host Initiated HECI Interface reset (as outlined in HPS 4.1.1.1)
        // *FB* We should not bother, f/w won't expect it at this point and is probably dead
        //      if we can get here
        //   - Talked to Will, he agrees we don't need to do this
        return 0;
    }

    return 1;
}

/*++
********************************************************************************
**
** FUNCTION:
**   FilledSlots
**
** DESCRIPTION:
**   Calculate if the circular buffer has overflowed
**   Corresponds to HECI HPS (part of) section 4.2.1
**
** ARGUMENTS:
**   [IN] ReadPointer - Value read from host/me read pointer
**   [IN] ReadPointer - Value read from host/me write pointer
**
** RETURNS:
**   Filled slots
**
********************************************************************************
--*/

U8 FilledSlots(U32 ReadPointer, U32 WritePointer)
{
    U8 FilledSlots;

    // Calculation documented in HECI HPS 0.68 section 4.2.1
    FilledSlots = (((signed char) WritePointer) - ((signed char) ReadPointer));

    return FilledSlots;
}

/*++
********************************************************************************
**
** FUNCTION:
**   OverflowCB
**
** DESCRIPTION:
**   Calculate if the circular buffer has overflowed
**   Corresponds to HECI HPS (part of) section 4.2.1
**
** ARGUMENTS:
**   [IN] ReadPointer - Value read from host/me read pointer
**   [IN] ReadPointer - Value read from host/me write pointer
**   [IN] BufferDepth - Value read from buffer depth register
**
** RETURNS:
**   0 - not overflowed
**   1 - overflowed
**
********************************************************************************
--*/

int OverflowCB(U32 ReadPointer, U32 WritePointer, U32 BufferDepth)
{
    U8 FilledSlots;

    // Calculation documented in HECI HPS 0.68 section 4.2.1
    FilledSlots = (((signed char) WritePointer) - ((signed char) ReadPointer));

    // test for overflow
    if(FilledSlots > ((U8) BufferDepth))
        return 1;

    return 0;
}

/*++
********************************************************************************
**
** FUNCTION:
**   StartTimer
**
** DESCRIPTION:
**   Used for implementing timeouts - returns a snapshot of the clock as the
**   current (start) time and calculates the end time
**
** ARGUMENTS:
**   [OUT] puStart  - snapshot of the clock
**   [OUT] puEnd    - calculated time when timeout period will be done
**   [IN]  uTime    - timeout period (in milliseconds)
**
** RETURNS:
**   none
**
********************************************************************************
--*/

void StartTimer(U32 *puStart, U32 *puEnd, U32 uTime)
{
    *puStart = clock();
    *puEnd   = *puStart + uTime;
}

/*++
********************************************************************************
**
** FUNCTION:
**   Timeout
**
** DESCRIPTION:
**   Used for implementing timeouts - returns indication of whether or not
**   the end time has been reached
**
** ARGUMENTS:
**   [IN] uStart    - snapshot of the clock when the timeout period started
**   [IN] uEnd      - calculated time when timeout period will be done
**
** RETURNS:
**   false - timeout has not occurred
**   true  - timeout has occurred
**
********************************************************************************
--*/

int Timeout( U32 uStart, U32 uEnd )
{
    U32 uCurr = clock();

    return(    ((uStart < uEnd) && (uEnd <= uCurr))                             // basic test; no wrapped variables
            || ((uStart < uEnd) && (uCurr < uStart))                            // handle current time wrapping
            || ((uStart > uEnd) && ((uCurr < uStart) && (uCurr > uEnd)))        // handle end and current time wrapping
            || (uStart == uEnd)                                           );    // test for zero-length interval
}



//
// Begin HECI Workaround
//
/*++
********************************************************************************
**
** FUNCTION:
**   HECIReadPCI
**
** DESCRIPTION:
**   Perform a PCI read through PCI data/index pair
**
** ARGUMENTS:
**   [IN] offset - Offset into the HECI devices PCI config space
**
** RETURNS:
**   U32 of data from selected PCI address
**
********************************************************************************
--*/
U32 HECIReadPCI(U8 offset)
{
   //
   //
   // IMPORTANT NOTE - This function is a hack and was only added as a
   // workaround for a HECI hardware issue. This cannot be used in a generic way
   // to support HECI 2 as currently written
   //
   //

   U32 address;

   // always reads from HECI device at Bus0, Device3, Func 0
   address = (0x80000000 | 0 << 16 | 3 << 11 | 0 << 8 | offset);
   outpd(0xcf8, address);

   return (inpd(0xcfc));
}
//
// End HECI Workaround
//
