/****************************************************************************/
/*                                                                          */
/*  Module:         QstSensor.h                                             */
/*                                                                          */
/*  Description:    Defines the implementation of the Sensors component of  */
/*                  the   ActiveX  encapsulation  of  the  Intel(R)  Quiet  */
/*                  System Technology (QST) Instrumentation Library.        */
/*                                                                          */
/****************************************************************************/

/****************************************************************************/
/*                                                                          */
/*        Copyright (c) 2009, Intel Corporation. All Rights Reserved.       */
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

#pragma once
#include "resource.h"       // main symbols

#include "QstInstX.h"

#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif

// CQstSensor

class ATL_NO_VTABLE CQstSensor :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CQstSensor, &CLSID_QstSensor>,
    public ISupportErrorInfo,
    public IDispatchImpl<IQstSensor, &IID_IQstSensor, &LIBID_QstInstXLib, /*wMajor =*/ 1, /*wMinor =*/ 0>
{
public:
    CQstSensor()
    {
    }

DECLARE_REGISTRY_RESOURCEID(IDR_QSTSENSOR)


BEGIN_COM_MAP(CQstSensor)
    COM_INTERFACE_ENTRY(IQstSensor)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

// ISupportsErrorInfo
    STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);


    DECLARE_PROTECT_FINAL_CONSTRUCT()

    HRESULT FinalConstruct()
    {
        return S_OK;
    }

    void FinalRelease()
    {
    }

public:

    STDMETHOD(GetCount)(BYTE byType, BYTE* pbyCount);
    STDMETHOD(GetCritHighThresh)(BYTE byType, BYTE bySensor, FLOAT* pfCrit);
    STDMETHOD(GetCritLowThresh)(BYTE byType, BYTE bySensor, FLOAT* pfCrit);
    STDMETHOD(GetFunction)(BYTE byType, BYTE bySensor, BYTE* pbyFunction);
    STDMETHOD(GetHealth)(BYTE byType, BYTE bySensor, BYTE* pbyHealth);
    STDMETHOD(GetNominal)(BYTE byType, BYTE bySensor, FLOAT* pfNominal);
    STDMETHOD(GetNonCritHighThresh)(BYTE byType, BYTE bySensor, FLOAT* pfNonCrit);
    STDMETHOD(GetNonCritLowThresh)(BYTE byType, BYTE bySensor, FLOAT* pfNonCrit);
    STDMETHOD(GetNonRecovHighThresh)(BYTE byType, BYTE bySensor, FLOAT* pfNonRecov);
    STDMETHOD(GetNonRecovLowThresh)(BYTE byType, BYTE bySensor, FLOAT* pfNonRecov);
    STDMETHOD(GetReading)(BYTE byType, BYTE bySensor, FLOAT* pfReading);
    STDMETHOD(IsRelative)(BYTE byType, BYTE bySensor, VARIANT_BOOL* pbRelative);
    STDMETHOD(SetHighThresh)(BYTE byType, BYTE bySensor, FLOAT fNonCrit, FLOAT fCrit, FLOAT fNonRecov, VARIANT_BOOL* pbSuccess);
    STDMETHOD(SetLowThresh)(BYTE byType, BYTE bySensor, FLOAT fNonCrit, FLOAT fCrit, FLOAT fNonRecov, VARIANT_BOOL* pbSuccess);
};

OBJECT_ENTRY_AUTO(__uuidof(QstSensor), CQstSensor)
