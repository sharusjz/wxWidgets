///////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/ole/oleutils.h
// Purpose:     OLE helper routines, OLE debugging support &c
// Author:      Vadim Zeitlin
// Modified by:
// Created:     19.02.1998
// Copyright:   (c) 1998 Vadim Zeitlin <zeitlin@dptmaths.ens-cachan.fr>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef   _WX_OLEUTILS_H
#define   _WX_OLEUTILS_H

#include "wx/defs.h"

#if wxUSE_OLE

// ole2.h includes windows.h, so include wrapwin.h first
#include "wx/msw/wrapwin.h"
// get IUnknown, REFIID &c
#include <ole2.h>
#include "wx/intl.h"
#include "wx/log.h"
#include "wx/variant.h"

#include "wx/msw/ole/comimpl.h"

// ============================================================================
// General purpose functions and macros
// ============================================================================

// ----------------------------------------------------------------------------
// initialize/cleanup OLE
// ----------------------------------------------------------------------------

// call OleInitialize() or CoInitialize[Ex]() depending on the platform
//
// return true if ok, false otherwise
inline bool wxOleInitialize()
{
    HRESULT
    hr = ::OleInitialize(NULL);

    // RPC_E_CHANGED_MODE indicates that OLE had been already initialized
    // before, albeit with different mode. Don't consider it to be an error as
    // we don't actually care ourselves about the mode used so this allows the
    // main application to call OleInitialize() on its own before we do if it
    // needs non-default mode.
    if ( hr != RPC_E_CHANGED_MODE && FAILED(hr) )
    {
        wxLogError(wxGetTranslation("Cannot initialize OLE"));

        return false;
    }

    return true;
}

inline void wxOleUninitialize()
{
    ::OleUninitialize();
}

#if wxUSE_VARIANT
// Convert variants
class WXDLLIMPEXP_FWD_BASE wxVariant;

// wrapper for CURRENCY type used in VARIANT (VARIANT.vt == VT_CY)
class WXDLLIMPEXP_CORE wxVariantDataCurrency : public wxVariantData
{
public:
    wxVariantDataCurrency() { VarCyFromR8(0.0, &m_value); }
    wxVariantDataCurrency(CURRENCY value) { m_value = value; }

    CURRENCY GetValue() const { return m_value; }
    void SetValue(CURRENCY value) { m_value = value; }

    virtual bool Eq(wxVariantData& data) const wxOVERRIDE;

#if wxUSE_STD_IOSTREAM
    virtual bool Write(wxSTD ostream& str) const wxOVERRIDE;
#endif
    virtual bool Write(wxString& str) const wxOVERRIDE;

    wxVariantData* Clone() const wxOVERRIDE { return new wxVariantDataCurrency(m_value); }
    virtual wxString GetType() const wxOVERRIDE { return wxS("currency"); }

    DECLARE_WXANY_CONVERSION()

private:
    CURRENCY m_value;
};


// wrapper for SCODE type used in VARIANT (VARIANT.vt == VT_ERROR)
class WXDLLIMPEXP_CORE wxVariantDataErrorCode : public wxVariantData
{
public:
    wxVariantDataErrorCode(SCODE value = S_OK) { m_value = value; }

    SCODE GetValue() const { return m_value; }
    void SetValue(SCODE value) { m_value = value; }

    virtual bool Eq(wxVariantData& data) const wxOVERRIDE;

#if wxUSE_STD_IOSTREAM
    virtual bool Write(wxSTD ostream& str) const wxOVERRIDE;
#endif
    virtual bool Write(wxString& str) const wxOVERRIDE;

    wxVariantData* Clone() const wxOVERRIDE { return new wxVariantDataErrorCode(m_value); }
    virtual wxString GetType() const wxOVERRIDE { return wxS("errorcode"); }

    DECLARE_WXANY_CONVERSION()

private:
    SCODE m_value;
};

// wrapper for SAFEARRAY, used for passing multidimensional arrays in wxVariant
class WXDLLIMPEXP_CORE wxVariantDataSafeArray : public wxVariantData
{
public:
    explicit wxVariantDataSafeArray(SAFEARRAY* value = NULL)
    {
        m_value = value;
    }

    SAFEARRAY* GetValue() const { return m_value; }
    void SetValue(SAFEARRAY* value) { m_value = value; }

    virtual bool Eq(wxVariantData& data) const wxOVERRIDE;

#if wxUSE_STD_IOSTREAM
    virtual bool Write(wxSTD ostream& str) const wxOVERRIDE;
#endif
    virtual bool Write(wxString& str) const wxOVERRIDE;

    wxVariantData* Clone() const wxOVERRIDE { return new wxVariantDataSafeArray(m_value); }
    virtual wxString GetType() const wxOVERRIDE { return wxS("safearray"); }

    DECLARE_WXANY_CONVERSION()

private:
    SAFEARRAY* m_value;
};

// Used by wxAutomationObject for its wxConvertOleToVariant() calls.
enum wxOleConvertVariantFlags
{
    wxOleConvertVariant_Default = 0,

    // If wxOleConvertVariant_ReturnSafeArrays  flag is set, SAFEARRAYs
    // contained in OLE VARIANTs will be returned as wxVariants
    // with wxVariantDataSafeArray type instead of wxVariants
    // with the list type containing the (flattened) SAFEARRAY's elements.
    wxOleConvertVariant_ReturnSafeArrays = 1
};

WXDLLIMPEXP_CORE
bool wxConvertVariantToOle(const wxVariant& variant, VARIANTARG& oleVariant);

WXDLLIMPEXP_CORE
bool wxConvertOleToVariant(const VARIANTARG& oleVariant, wxVariant& variant,
                           long flags = wxOleConvertVariant_Default);

#endif // wxUSE_VARIANT

// Convert string to Unicode
WXDLLIMPEXP_CORE BSTR wxConvertStringToOle(const wxString& str);

// Convert string from BSTR to wxString
WXDLLIMPEXP_CORE wxString wxConvertStringFromOle(BSTR bStr);

// A thin RAII wrapper for BSTR, which can also create BSTR
// from wxString as well as return the owned BSTR as wxString.
// Unlike _b_str_t, wxBSTR is NOT reference counted.
class WXDLLIMPEXP_CORE wxBSTR
{
public:
    // Creates with the owned BSTR set to NULL
    wxBSTR() : m_bstr(NULL) {}

    // If copy is true then a copy of bstr is created,
    // if not then ownership of bstr is taken.
    wxBSTR(BSTR bstr, bool copy);

    // Creates the owned BSTR from wxString str
    wxBSTR(const wxString& str) : m_bstr(::SysAllocString(str.wc_str(*wxConvCurrent))) {}

    // Creates a copy of BSTR owned by wxbstr
    wxBSTR(const wxBSTR& wxbstr) : m_bstr(wxbstr.GetCopy()) {}

    // Frees the owned BSTR
    ~wxBSTR() { Free(); }

    // Creates a copy of the BSTR owned by wxbstr
    wxBSTR& operator=(const wxBSTR& wxbstr);

    // Takes ownership of bstr
    void Attach(BSTR bstr);

    // Returns the owned BSTR and gives up its ownership
    BSTR Detach();

    // Frees the owned BSTR
    void Free();

    // Returns the owned BSTR while keeping its ownership
    BSTR GetBSTR() const { return m_bstr; }
    operator BSTR() const { return GetBSTR(); }

    // Returns the address of owned BSTR
    BSTR* GetAddress() { return &m_bstr; }

    // Returns a copy of owned BSTR
    BSTR GetCopy() const {  return ::SysAllocString(m_bstr); }

    // Returns the owned BSTR converted to wxString
    wxString GetwxString() const { return wxConvertStringFromOle(m_bstr); }
private:
    BSTR m_bstr;
};

#else // !wxUSE_OLE

// ----------------------------------------------------------------------------
// stub functions to avoid #if wxUSE_OLE in the main code
// ----------------------------------------------------------------------------

inline bool wxOleInitialize() { return false; }
inline void wxOleUninitialize() { }

#endif // wxUSE_OLE/!wxUSE_OLE

// RAII class initializing OLE in its ctor and undoing it in its dtor.
class wxOleInitializer
{
public:
    wxOleInitializer()
        : m_ok(wxOleInitialize())
    {
    }

    bool IsOk() const
    {
        return m_ok;
    }

    ~wxOleInitializer()
    {
        if ( m_ok )
            wxOleUninitialize();
    }

private:
    const bool m_ok;

    wxDECLARE_NO_COPY_CLASS(wxOleInitializer);
};

#endif  //_WX_OLEUTILS_H
