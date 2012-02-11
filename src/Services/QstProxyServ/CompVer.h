/****************************************************************************/
/*                                                                          */
/*  Module:         CompVer.h                                               */
/*                                                                          */
/*  Description:    Defines component version information for the Intel(R)  */
/*                  Quiet System Technology (QST) Instrumentation Service.  */
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

#ifndef _COMPVER_H_
#define _COMPVER_H_

#include "../../Include/Build.h"

// Version updated to 2.0.0.xxxx for IbexPeak

#define COMP_VER_MAJOR           2
#define COMP_VER_MAJOR_STR      "2"
#define COMP_VER_MINOR           0
#define COMP_VER_MINOR_STR      "0"
#define COMP_VER_HOTFIX          0
#define COMP_VER_HOTFIX_STR     "0"

#define COMP_VERSION            COMP_VER_MAJOR, COMP_VER_MINOR, COMP_VER_HOTFIX, VER_BUILD

#ifdef DBG
#define VER_DEBUG_TAG           " (DBG)"
#else
#define VER_DEBUG_TAG
#endif

#define COMP_VERSION_STR        COMP_VER_MAJOR_STR "." COMP_VER_MINOR_STR "." COMP_VER_HOTFIX_STR "." VER_BUILD_STR VER_DEBUG_TAG

#endif // _COMPVER_H_

