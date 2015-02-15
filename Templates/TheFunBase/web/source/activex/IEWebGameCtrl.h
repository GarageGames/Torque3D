//-----------------------------------------------------------------------------
// Copyright (c) 2012 GarageGames, LLC
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------

// Torque 3D web deployment for Internet Explorer (ActiveX)

#pragma once
#include "resource.h"       // main symbols
#include <atlctl.h>
#include "IEWebGamePlugin_i.h"
#include "IEWebGameWindow.h"

#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif

// The heavy lifting is done by our game control (which inherits from WebGameWindow)

// CIEWebGameCtrl
class ATL_NO_VTABLE CIEWebGameCtrl :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CStockPropImpl<CIEWebGameCtrl, IIEWebGameCtrl>,
	public IPersistStreamInitImpl<CIEWebGameCtrl>,
	public IOleControlImpl<CIEWebGameCtrl>,
	public IOleObjectImpl<CIEWebGameCtrl>,
	public IOleInPlaceActiveObjectImpl<CIEWebGameCtrl>,
	public IViewObjectExImpl<CIEWebGameCtrl>,
	public IOleInPlaceObjectWindowlessImpl<CIEWebGameCtrl>,
	public IObjectSafetyImpl<CIEWebGameCtrl, INTERFACESAFE_FOR_UNTRUSTED_CALLER>,
	public CComCoClass<CIEWebGameCtrl, &CLSID_IEWebGameCtrl>,
	public CComControl<CIEWebGameCtrl, WebGameWindow>
{
public:



	DECLARE_OLEMISC_STATUS(OLEMISC_RECOMPOSEONRESIZE |
	OLEMISC_CANTLINKINSIDE |
		OLEMISC_INSIDEOUT |
		OLEMISC_ACTIVATEWHENVISIBLE |
		OLEMISC_SETCLIENTSITEFIRST
		)

		DECLARE_REGISTRY_RESOURCEID(IDR_IEWEBGAMECTRL)


	BEGIN_COM_MAP(CIEWebGameCtrl)
		COM_INTERFACE_ENTRY(IIEWebGameCtrl)
		COM_INTERFACE_ENTRY(IDispatch)
		COM_INTERFACE_ENTRY(IViewObjectEx)
		COM_INTERFACE_ENTRY(IViewObject2)
		COM_INTERFACE_ENTRY(IViewObject)
		COM_INTERFACE_ENTRY(IOleInPlaceObjectWindowless)
		COM_INTERFACE_ENTRY(IOleInPlaceObject)
		COM_INTERFACE_ENTRY2(IOleWindow, IOleInPlaceObjectWindowless)
		COM_INTERFACE_ENTRY(IOleInPlaceActiveObject)
		COM_INTERFACE_ENTRY(IOleControl)
		COM_INTERFACE_ENTRY(IOleObject)
		COM_INTERFACE_ENTRY(IPersistStreamInit)
		COM_INTERFACE_ENTRY2(IPersist, IPersistStreamInit)
		COM_INTERFACE_ENTRY_IID(IID_IObjectSafety, IObjectSafety)

	END_COM_MAP()

	BEGIN_PROP_MAP(CIEWebGameCtrl)
		PROP_DATA_ENTRY("_cx", m_sizeExtent.cx, VT_UI4)
		PROP_DATA_ENTRY("_cy", m_sizeExtent.cy, VT_UI4)
		// Example entries
		// PROP_ENTRY_TYPE("Property Name", dispid, clsid, vtType)
		// PROP_PAGE(CLSID_StockColorPage)
	END_PROP_MAP()


	BEGIN_MSG_MAP(WebGameWindow)
		CHAIN_MSG_MAP(WebGameWindow)
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()

	// Handler prototypes:
	//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

	// IViewObjectEx
	DECLARE_VIEW_STATUS(0)

	// IIEWebGameCtrl
public:

	static CIEWebGameCtrl* sInstance;

	CIEWebGameCtrl()
	{
		m_bWindowOnly = TRUE;
		sInstance = this;
		mInitialized = false;
	}

	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}

   // the javascript accessible methods which can be called our on plugin object

   // plugin.getVariable("$MyVariable"); - get a Torque 3D console variable
	STDMETHOD(getVariable)(BSTR name, BSTR* value);

   // plugin.setVariable("$MyVariable", 42); - set a Torque 3D console variable
	STDMETHOD(setVariable)(BSTR name, BSTR value);

   // var result = plugin.callScript("mySecureFunction('one', 'two', 'three');"); - call a TorqueScript function marked as secure in webConfig.h with supplied arguments
	STDMETHOD(callScript)(BSTR code, BSTR* retValue);

   // plugin.exportFunction("MyJavascriptFunction",3); - export a Javascript function to the Torque 3D console system via its name and argument count
	STDMETHOD(exportFunction)(BSTR name, LONG numArguments);

   // plugin.startup(); - called once web page is fully loaded and plugin (including Torque 3D) is initialized
	STDMETHOD(startup)();

   // TorqueScript -> Javascript call handling
	const char* callFunction(const char* name, LONG numArguments, const char* argv[]);

   // our plugin requires no signing as it is installer based
   STDMETHOD(GetInterfaceSafetyOptions)(REFIID riid, DWORD *pdwSupportedOptions, DWORD *pdwEnabledOptions);
   STDMETHOD(SetInterfaceSafetyOptions)(REFIID riid, DWORD dwOptionSetMask, DWORD dwEnabledOptions);


protected:

	// these can be added on the page before we're initialized, so we cache them at startup
	typedef struct JavasScriptExport
	{ 
		std::string jsCallback; //javascript function name
		UINT        numArguments;  //the number of arguments it takes
	};

	std::vector<JavasScriptExport> mJavaScriptExports;

   // actually handle the export (once Torque 3D is fully initialized)
   void internalExportFunction(const JavasScriptExport& jsexport);	


	BOOL mInitialized;

	// checks a given domain against the allowed domains in webConfig.h
	bool checkDomain();
   

};


OBJECT_ENTRY_AUTO(__uuidof(IEWebGameCtrl), CIEWebGameCtrl)
