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

#include "stdafx.h"
#include "IEWebGameCtrl.h"

#include "../common/webCommon.h"

// CIEWebGameCtrl (one and only one instance please)
CIEWebGameCtrl* CIEWebGameCtrl::sInstance = NULL;

// Javascript accessible methods

// plugin.getVariable("$MyVariable"); - get a Torque 3D console variable
STDMETHODIMP CIEWebGameCtrl::getVariable(BSTR name, BSTR* value)
{
	std::wstring wstr;
	std::string sstr;
	const char* astr;

	wstr.assign(name);
	sstr = WebCommon::WStringToString(wstr);
	astr = sstr.c_str();

   const char* avalue = NULL;
   char vinfo[256];
   vinfo[0] = 0;

   // requesting the version information
   if (!_stricmp(astr, "$version"))
   {
      char plugin[4096];

      GetModuleFileNameA(WebCommon::gPluginModule, plugin, 4096);
      DWORD dwHandle = 0;
      DWORD dwSize = GetFileVersionInfoSizeA(plugin, &dwHandle);
      if (dwSize >= 0)
      {
         LPBYTE lpInfo = new BYTE[dwSize];
         ZeroMemory(lpInfo, dwSize);
         if(GetFileVersionInfoA(plugin, 0, dwSize, lpInfo))
         {
            UINT valLen = MAX_PATH;
            LPVOID valPtr = NULL;
            if(::VerQueryValue(lpInfo, 
               TEXT("\\"),
               &valPtr,
               &valLen))
            {
               VS_FIXEDFILEINFO* pFinfo = (VS_FIXEDFILEINFO*)valPtr;

               sprintf(vinfo, "%i.%i", (pFinfo->dwProductVersionMS >> 16) & 0xFF, (pFinfo->dwFileVersionMS) & 0xFF);

            }
         }
         delete[] lpInfo;
      }

      if (!vinfo[0])
         strcpy(vinfo, "-1");

      
      avalue = vinfo;

   }
   else
      avalue = WebCommon::GetVariable(astr);

	sstr = avalue;
	wstr = WebCommon::StringToWString(sstr);

	*value = SysAllocString(wstr.c_str());

	return S_OK;
}


// plugin.setVariable("$MyVariable", 42); - set a Torque 3D console variable
STDMETHODIMP CIEWebGameCtrl::setVariable(BSTR name, BSTR value)
{
	std::wstring wstr;
	std::string nstr, vstr;
	const char* vname;
	const char* vvalue;

	wstr.assign(name);
	nstr = WebCommon::WStringToString(wstr);
	vname = nstr.c_str();

	wstr.assign(value);
	vstr = WebCommon::WStringToString(wstr);
	vvalue = vstr.c_str();

   WebCommon::SetVariable(vname, vvalue);

	return S_OK;
}


// plugin.startup(); - called once web page is fully loaded and plugin (including Torque 3D) is initialized
STDMETHODIMP CIEWebGameCtrl::startup()
{

	mInitialized = true;
	std::vector<JavasScriptExport>::iterator i;
	for (i = mJavaScriptExports.begin(); i != mJavaScriptExports.end();i++)
	{
		internalExportFunction(*i);
	}

	WebCommon::AddSecureFunctions();

	return S_OK;
}

// var result = plugin.callScript("mySecureFunction('one', 'two', 'three');"); - call a TorqueScript function marked as secure in webConfig.h with supplied arguments
// includes function parser
STDMETHODIMP CIEWebGameCtrl::callScript(BSTR code, BSTR* value)
{
	std::wstring wcode;
	std::string scode;
	wcode.assign(code);
	scode = WebCommon::WStringToString(wcode);
	const char* sig = scode.c_str();

	// do not allow large strings which could be used maliciously
	if (scode.length() > 255 || !mInitialized)
	{
		*value = SysAllocString(L"");
		return E_INVALIDARG;
	}

   // data buffers for laying out data in a Torque 3D console friendly manner
	char  nameSpace[256];
	char  fname[256];
	char  argv[256][256];
	char* argvv[256];
	int argc = 0;
	unsigned int argBegin = 0;

	memset(nameSpace, 0, 256);
	memset(fname, 0, 256);
	memset(argv, 0, 256 * 256);

	for (unsigned int i = 0; i < scode.length(); i++)
	{
		if (sig[i] == ')' || sig[i] == ';')
		{
			//scan out last arg is any
			char dummy[256];
			memset(dummy, 0, 256);

			WebCommon::StringCopy(dummy, &sig[argBegin], i - argBegin);

			if (strlen(dummy))
			{
				strcpy_s(argv[argc], dummy);
				argvv[argc] = argv[argc];
				argc++;
			}

			break; // done
		}

      // namespace
		if (sig[i]==':')
		{
			if (nameSpace[0] || fname[0])
			{
				*value = SysAllocString(L"");
				return E_INVALIDARG;
			}

			if (i > 0 && sig[i-1] == ':')
			{
				if (i - 2 > 0)
					WebCommon::StringCopy(nameSpace, sig, i - 1);
			}

			continue;
		}

      // args begin
		if (sig[i] == '(' )
		{
			if (fname[0] || i < 1)
			{
				*value = SysAllocString(L"");
				return E_INVALIDARG;
			}

			//everything before this is function name, minus nameSpace
			if (nameSpace[0])
			{
				int nlen = strlen(nameSpace);
				WebCommon::StringCopy(fname, &sig[nlen + 2], i - nlen - 2);
			}
			else
			{
				WebCommon::StringCopy(fname, sig, i);
			}

			WebCommon::StringCopy(argv[0], fname, strlen(fname)+1);
			argvv[0] = argv[0];
			argc++;

			argBegin = i + 1;
		}

      // args
		if (sig[i] == ',' )
		{
			if (argBegin >= i || argc == 255) 
			{
				*value = SysAllocString(L"");
				return E_INVALIDARG;
			}

			WebCommon::StringCopy(argv[argc], &sig[argBegin], i - argBegin);
			argvv[argc] = argv[argc];

			argc++;
			argBegin = i + 1;
		}

	}

	const char* retVal;
	std::string sretVal;
	std::wstring wretVal;

	if (fname[0])
	{
      // call into the Torque 3D shared library (console system) and get return value
		retVal = torque_callsecurefunction(nameSpace, fname, argc, (const char **) argvv);

		sretVal= retVal;
		wretVal = WebCommon::StringToWString(sretVal);

		*value = SysAllocString(wretVal.c_str());
	}
	else
	{
		*value = SysAllocString(L"");
		return E_INVALIDARG;
	}

	return S_OK;
}

// the sole entry point for Torque 3D console system into our browser plugin (handed over as a function pointer)
static const char * MyStringCallback(void *obj, int argc, const char* argv[])
{
	static char ret[4096];
	strcpy_s(ret,CIEWebGameCtrl::sInstance->callFunction(argv[0], argc, argv));
	return ret;
}

// Get the location we're loading the plugin from (http://, file://) including address
// this is used by the domain locking feature to ensure that your plugin is only
// being used from your web site
bool CIEWebGameCtrl::checkDomain()
{
	HRESULT hrResult	= S_FALSE;
	IMoniker* pMoniker	= NULL;
	LPOLESTR sDisplayName;

	hrResult = m_spClientSite->GetMoniker(OLEGETMONIKER_TEMPFORUSER,
		OLEWHICHMK_CONTAINER,
		&pMoniker);

	if(SUCCEEDED(hrResult))
	{
		hrResult = pMoniker->GetDisplayName(NULL,
			NULL,
			&sDisplayName);
		pMoniker->Release();

		std::wstring wstr;
		std::string sstr;

		wstr.assign(sDisplayName);
		sstr = WebCommon::WStringToString(wstr);

		return WebCommon::CheckDomain(sstr.c_str());
	}

	return false;
}

// handles TorqueScript -> Javascript calling including return value
const char* CIEWebGameCtrl::callFunction(const char* name, LONG numArguments, const char* argv[])
{

	//sanity
	if (numArguments > 200)
		return "";

   // A bunch of COM'esque stuff to which ultimately boils down to finding a Javascript function on the page

	HRESULT hr;

	LPOLECONTAINER pContainer;
	IHTMLDocument* pHTML = NULL;
	CComPtr<IDispatch> pScript;
	CComQIPtr<IHTMLWindow2> pWin;


	if (!m_spClientSite)
		return "";

	hr = m_spClientSite->GetContainer(&pContainer);

	if (FAILED(hr))
	{
		return "";
	}

	hr = pContainer->QueryInterface(IID_IHTMLDocument, (void
		**)&pHTML); 

	if (FAILED(hr))
	{
		pContainer->Release();
		return "";
	}

	hr = pHTML->get_Script(&pScript);

	if (FAILED(hr))
	{
		pContainer->Release();
		pHTML->Release();
		return "";
	}

	DISPID idMethod = 0;
	std::string smethod = name;
	std::wstring wmethod = WebCommon::StringToWString(smethod);
	OLECHAR FAR* sMethod = (OLECHAR FAR*)wmethod.c_str();
	hr = pScript->GetIDsOfNames(IID_NULL, &sMethod, 1, LOCALE_SYSTEM_DEFAULT,&idMethod);

	if (FAILED(hr))
	{
		pContainer->Release();
		pHTML->Release();
		return "";
	}

   // setup arguments and return value variants

	VARIANT pVarRet = {0};

	VariantInit(&pVarRet);

	if (numArguments <= 1)
	{
		DISPPARAMS dpNoArgs = {NULL, NULL, 0, 0};
		hr = pScript->Invoke(idMethod, IID_NULL, LOCALE_SYSTEM_DEFAULT, DISPATCH_METHOD,
			&dpNoArgs, &pVarRet, NULL, NULL);
	}
	else
	{
		DISPPARAMS params;
		VARIANTARG args[256];
		std::wstring wargs[256];

		for (LONG i = 0; i < numArguments - 1; i++ )
		{
			VariantInit(&args[i]);
			// Invoke wants these in reverse order 
			std::string s = argv[numArguments - i - 1];
			wargs[i] = WebCommon::StringToWString(s);
			args[i].vt = VT_BSTR;
			args[i].bstrVal = SysAllocString(wargs[i].c_str());
		}

		params.cArgs = numArguments - 1;
		params.rgdispidNamedArgs = NULL;
		params.cNamedArgs = 0;
		params.rgvarg = args;

      // whew, actually call the Javascript
		hr = pScript->Invoke(idMethod, IID_NULL, LOCALE_SYSTEM_DEFAULT, DISPATCH_METHOD,
			&params, &pVarRet, NULL, NULL);

		for (LONG i = 0; i < numArguments - 1; i++ )
		{
			SysFreeString(args[i].bstrVal);
		}

	}

	if (FAILED(hr))
	{
		pContainer->Release();
		pHTML->Release();
		return "";
	}

	VariantChangeType(&pVarRet, &pVarRet, 0, VT_BSTR);

	std::wstring wstr;
	std::string sstr;
	static char ret[4096];

	wstr.assign(pVarRet.bstrVal);
	sstr = WebCommon::WStringToString(wstr);
	strcpy_s(ret, sstr.c_str());

	pContainer->Release();
	pHTML->Release();

	return ret;

}

// handle the actual export (once we're actually all ready to go)
void CIEWebGameCtrl::internalExportFunction(const JavasScriptExport& jsexport)
{
	torque_exportstringcallback(MyStringCallback,"JS",jsexport.jsCallback.c_str(),"",jsexport.numArguments,jsexport.numArguments);
}

// plugin.exportFunction("MyJavascriptFunction",3); - export a Javascript function to the Torque 3D console system via its name and argument count
// If we haven't initialized Torque 3D yet, cache it
STDMETHODIMP CIEWebGameCtrl::exportFunction(BSTR callback, LONG numArguments)
{
	JavasScriptExport jsexport;
	std::wstring wstr;

	wstr.assign(callback);
	jsexport.jsCallback = WebCommon::WStringToString(wstr);
	jsexport.numArguments = numArguments;

	if (!mInitialized)
	{
		//queue it up
		mJavaScriptExports.push_back(jsexport);
	}
	else
	{
		internalExportFunction(jsexport);
	}

	return S_OK;
}


// Our web deployment is installer based, no code signing necessary
STDMETHODIMP CIEWebGameCtrl::GetInterfaceSafetyOptions(REFIID riid,
													   DWORD *pdwSupportedOptions,DWORD *pdwEnabledOptions)
{
	return S_OK;
}

STDMETHODIMP CIEWebGameCtrl::SetInterfaceSafetyOptions(REFIID riid,
													   DWORD dwOptionSetMask,DWORD dwEnabledOptions)
{
	return S_OK;
}



