/****************************************************************************/
/*                                                                          */
/*  Module:         QstConfigManip.h                                        */
/*                                                                          */
/*  Description:    Provides common information used to compact and expand  */
/*                  the QST configuration.                                  */
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

#include "QstCfg.h"

#ifndef _QST_CONFIG_MANIP_COMMON_H
#define _QST_CONFIG_MANIP_COMMON_H

/****************************************************************************/
/* Declarations for compatibility with ME F/W environment                   */
/****************************************************************************/

#ifdef _ARC
#include "QstCommonDefs.h"
#else
#include <string.h>
#endif

/****************************************************************************/
/* Required Configuration version information to use this functionality.    */
/****************************************************************************/
#define MANIP_MIN_CFG_MAJOR_VERSION 2
#define MANIP_MIN_CFG_MINOR_VERSION 0

/****************************************************************************/
/* Macro to determine size of the configuration based on number of entities */
/****************************************************************************/
#define GET_QST_CONFIG_SIZE(TempMons, FanMons, VoltMons, CurrMons, RspUnits, FanCtrls) \
   (sizeof(QST_PAYLOAD_HEADER_STRUCT) + \
    (QST_TEMP_MONITOR_SIZE * (TempMons)) + \
    (QST_FAN_MONITOR_SIZE * (FanMons)) + \
    (QST_VOLT_MONITOR_SIZE * (VoltMons)) + \
    (QST_CURR_MONITOR_SIZE * (CurrMons)) + \
    (QST_TEMP_RESPONSE_SIZE * (RspUnits)) + \
    ((QST_FAN_CONTROLLER_SIZE(RspUnits)) * (FanCtrls)))

/****************************************************************************/
/* Status information                                                       */
/****************************************************************************/
typedef enum {
   MANIP_SUCCESS = 0,
   MANIP_INVALID_PARAMETER,
   MANIP_BUFFER_TOO_SMALL,
   MANIP_TOO_MANY_ENTITIES,
   MANIP_INVALID_HEADER,
   MANIP_INVALID_CFG_FORMAT

} MANIP_STATUS;

#define MANIP_ERROR(Status) ((Status) != MANIP_SUCCESS)

/****************************************************************************/
/* Structure information for passing expanded structure counts.             */
/****************************************************************************/

typedef struct {
   UINT8    TempMons;
   UINT8    FanMons;
   UINT8    VoltMons;
   UINT8    CurrMons;
   UINT8    TempRsps;
   UINT8    FanCtrls;

} QST_CFG_COUNTS;

//
// Need to special case the Fan Controller structure as it has a
// variable length
//
typedef struct {
   QST_PAYLOAD_HEADER_STRUCT  *Header;
   QST_TEMP_MONITOR_STRUCT    *TempMon;
   QST_FAN_MONITOR_STRUCT     *FanMon;
   QST_VOLT_MONITOR_STRUCT    *VoltMon;
   QST_CURR_MONITOR_STRUCT    *CurrMon;
   QST_TEMP_RESPONSE_STRUCT   *TempRsp;
   void                       *FanCtrl;

} QST_DYNAMIC_PAYLOAD;

#endif

