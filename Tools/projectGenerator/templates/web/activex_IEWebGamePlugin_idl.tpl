// IEWebGamePlugin.idl : IDL source for IEWebGamePlugin
//

// This file will be processed by the MIDL tool to
// produce the type library (IEWebGamePlugin.tlb) and marshalling code.

#include "olectl.h"
import "oaidl.idl";
import "ocidl.idl";

[
	object,
	uuid(__WEBGAME_UUID__),
	dual,
	nonextensible,
	helpstring("IIEWebGameCtrl Interface"),
	pointer_default(unique)
]
interface IIEWebGameCtrl : IDispatch{
	[propget, bindable, requestedit, id(DISPID_HWND)]
	HRESULT HWND([out, retval]LONG_PTR* pHWND);
    [id(1), helpstring("method getVariable")] HRESULT getVariable([in] BSTR name, [out, retval] BSTR* value);
    [id(2), helpstring("method setVariable")] HRESULT setVariable([in] BSTR name, [in] BSTR value);
    [id(3), helpstring("method export")] HRESULT exportFunction([in] BSTR callback, [in] LONG numArguments);
    [id(4), helpstring("method callScript")] HRESULT callScript([in] BSTR code, [out, retval] BSTR* retValue);
    [id(5), helpstring("method startup")] HRESULT startup();
};

[
	uuid(__IWEBGAMECTRL_UUID__),
	version(1.0),
	helpstring("__WEBGAME_PLUGINNAME__ 1.0 Type Library")
]
library __WEBGAME_PLUGINNAME__Lib
{
	importlib("stdole2.tlb");
	[
		uuid(__WEBGAMELIB_UUID__),
		control,
		helpstring("IEWebGameCtrl Class")
	]
	coclass IEWebGameCtrl
	{
		[default] interface IIEWebGameCtrl;
	};
};
