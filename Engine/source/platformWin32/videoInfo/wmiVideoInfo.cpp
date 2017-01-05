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

#define _WIN32_DCOM

//#include <comdef.h>
#include <wbemidl.h>
//#include <atlconv.h>
#include <DXGI.h>
#pragma comment(lib, "comsuppw.lib") 
#pragma comment(lib, "wbemuuid.lib")

#include "platformWin32/videoInfo/wmiVideoInfo.h"
#include "core/util/safeRelease.h"
#include "console/console.h"

// http://www.spectranaut.net/sourcecode/WMI.cpp

// Add constructor to GUID.
struct MYGUID : public GUID
{
   MYGUID( DWORD a, SHORT b, SHORT c, BYTE d, BYTE e, BYTE f, BYTE g, BYTE h, BYTE i, BYTE j, BYTE k )
   {
      Data1       = a;
      Data2       = b;
      Data3       = c;
      Data4[ 0 ]  = d;
      Data4[ 1 ]  = e;
      Data4[ 2 ]  = f;
      Data4[ 3 ]  = g;
      Data4[ 4 ]  = h;
      Data4[ 5 ]  = i;
      Data4[ 6 ]  = j;
      Data4[ 7 ]  = k;
   }
};


//------------------------------------------------------------------------------
// DXDIAG declarations.

struct DXDIAG_INIT_PARAMS
{
   DWORD    dwSize;
   DWORD    dwDxDiagHeaderVersion;
   BOOL     bAllowWHQLChecks;
   LPVOID   pReserved;
};

struct IDxDiagContainer : public IUnknown
{
   virtual HRESULT   STDMETHODCALLTYPE GetNumberOfChildContainers( DWORD* pdwCount ) = 0;
   virtual HRESULT   STDMETHODCALLTYPE EnumChildContainerNames( DWORD dwIndex, LPWSTR pwszContainer, DWORD cchContainer ) = 0;
   virtual HRESULT   STDMETHODCALLTYPE GetChildContainer( LPCWSTR pwszContainer, IDxDiagContainer** ppInstance ) = 0;
   virtual HRESULT   STDMETHODCALLTYPE GetNumberOfProps( DWORD* pdwCount ) = 0;
   virtual HRESULT   STDMETHODCALLTYPE EnumPropNames( DWORD dwIndex, LPWSTR pwszPropName, DWORD cchPropName ) = 0;
   virtual HRESULT   STDMETHODCALLTYPE GetProp( LPCWSTR pwszPropName, VARIANT* pvarProp ) = 0;
};

struct IDxDiagProvider : public IUnknown
{
   virtual HRESULT   STDMETHODCALLTYPE Initialize( DXDIAG_INIT_PARAMS* pParams ) = 0;
   virtual HRESULT   STDMETHODCALLTYPE GetRootContainer( IDxDiagContainer** ppInstance ) = 0;
};

static MYGUID CLSID_DxDiagProvider( 0xA65B8071, 0x3BFE, 0x4213, 0x9A, 0x5B, 0x49, 0x1D, 0xA4, 0x46, 0x1C, 0xA7 );
static MYGUID IID_IDxDiagProvider( 0x9C6B4CB0, 0x23F8, 0x49CC, 0xA3, 0xED, 0x45, 0xA5, 0x50, 0x00, 0xA6, 0xD2 );
static MYGUID IID_IDxDiagContainer( 0x7D0F462F, 0x4064, 0x4862, 0xBC, 0x7F, 0x93, 0x3E, 0x50, 0x58, 0xC1, 0x0F );

//------------------------------------------------------------------------------

WCHAR *WMIVideoInfo::smPVIQueryTypeToWMIString [] =
{
   L"MaxNumberControlled",    //PVI_NumDevices
   L"Description",            //PVI_Description
   L"Name",                   //PVI_Name
   L"VideoProcessor",         //PVI_ChipSet
   L"DriverVersion",          //PVI_DriverVersion
   L"AdapterRAM",             //PVI_VRAM
};

//------------------------------------------------------------------------------

WMIVideoInfo::WMIVideoInfo()
   : PlatformVideoInfo(),
     mLocator( NULL ),
     mServices( NULL ),
     mComInitialized( false ),
     mDXGIModule( NULL ),
     mDXGIFactory( NULL ),
     mDxDiagProvider( NULL )
{

}

//------------------------------------------------------------------------------

WMIVideoInfo::~WMIVideoInfo()
{
   SAFE_RELEASE( mLocator );
   SAFE_RELEASE( mServices );

   if( mDxDiagProvider )
      SAFE_RELEASE( mDxDiagProvider );

   if( mDXGIFactory )
      SAFE_RELEASE( mDXGIFactory );
   if( mDXGIModule )
      FreeLibrary( ( HMODULE ) mDXGIModule );

   if( mComInitialized )
      CoUninitialize();
}

//------------------------------------------------------------------------------

String WMIVideoInfo::_lookUpVendorId(U32 vendorId)
{
   String vendor;
   switch (vendorId)
   {
   case 0x10DE:
      vendor = "NVIDIA";
      break;
   case 0x1002:
      vendor = "AMD";
      break;
   case 0x8086:
      vendor = "INTEL";
      break;
   }
   return vendor;
}

//------------------------------------------------------------------------------

bool WMIVideoInfo::_initialize()
{
   // Init COM
   HRESULT hr = CoInitialize( NULL );
   mComInitialized = SUCCEEDED( hr );

   if( !mComInitialized )
      return false;

   bool success = false;

   success |= _initializeDXGI();
   success |= _initializeDxDiag();
   success |= _initializeWMI();

   return success;
}

bool WMIVideoInfo::_initializeWMI()
{
   //// Set security levels 
   //hr = CoInitializeSecurity(
   //   NULL, 
   //   -1,                          // COM authentication
   //   NULL,                        // Authentication services
   //   NULL,                        // Reserved
   //   RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication 
   //   RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation  
   //   NULL,                        // Authentication info
   //   EOAC_NONE,                   // Additional capabilities 
   //   NULL                         // Reserved
   //   );

   //if( FAILED( hr ) )
   //{
   //   Con::errorf( "WMIVideoInfo: Failed to initialize com security." );
   //   return false;
   //}

   // Obtain the locator to WMI 
   HRESULT hr = CoCreateInstance(
      CLSID_WbemLocator,             
      0, 
      CLSCTX_INPROC_SERVER, 
      IID_IWbemLocator, 
      (void**)&mLocator
      );

   if( FAILED( hr ) )
   {
      Con::errorf( "WMIVideoInfo: Failed to create instance of IID_IWbemLocator." );
      return false;
   }

   // Connect to the root\cimv2 namespace with
   // the current user and obtain pointer pSvc
   // to make IWbemServices calls.
   hr = mLocator->ConnectServer(
      BSTR(L"ROOT\\CIMV2"), // Object path of WMI namespace
      NULL,                    // User name. NULL = current user
      NULL,                    // User password. NULL = current
      0,                       // Locale. NULL indicates current
      NULL,                    // Security flags.
      0,                       // Authority (e.g. Kerberos)
      0,                       // Context object 
      &mServices               // pointer to IWbemServices proxy
      );

   if( FAILED( hr ) )
   {
      Con::errorf( "WMIVideoInfo: Connect server failed." );
      return false;
   }


   // Set security levels on the proxy 
   hr = CoSetProxyBlanket(
      mServices,                   // Indicates the proxy to set
      RPC_C_AUTHN_WINNT,           // RPC_C_AUTHN_xxx
      RPC_C_AUTHZ_NONE,            // RPC_C_AUTHZ_xxx
      NULL,                        // Server principal name 
      RPC_C_AUTHN_LEVEL_CALL,      // RPC_C_AUTHN_LEVEL_xxx 
      RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
      NULL,                        // client identity
      EOAC_NONE                    // proxy capabilities 
      );

   if( FAILED( hr ) )
   {
      Con::errorf( "WMIVideoInfo: CoSetProxyBlanket failed" );
      return false;
   }

   return true;
}

bool WMIVideoInfo::_initializeDXGI()
{
   // Try using for DXGI 1.1, will only succeed on Windows 7+.
   mDXGIModule = ( HMODULE ) LoadLibrary( L"dxgi.dll" );
   if( mDXGIModule != 0 )
   {
      typedef HRESULT (WINAPI* CreateDXGIFactoryFuncType )( REFIID, void** );
      CreateDXGIFactoryFuncType factoryFunction =
         ( CreateDXGIFactoryFuncType ) GetProcAddress( ( HMODULE ) mDXGIModule, "CreateDXGIFactory1" );

      if( factoryFunction && factoryFunction( IID_IDXGIFactory1, ( void** ) &mDXGIFactory ) == S_OK )
         return true;
      else
      {
         FreeLibrary( ( HMODULE ) mDXGIModule );
         mDXGIModule = 0;
      }
   }

   return false;
}

bool WMIVideoInfo::_initializeDxDiag()
{
   if( CoCreateInstance( CLSID_DxDiagProvider, NULL, CLSCTX_INPROC_SERVER, IID_IDxDiagProvider, ( void** ) &mDxDiagProvider ) == S_OK )
   {
      DXDIAG_INIT_PARAMS params;
      dMemset( &params, 0, sizeof( DXDIAG_INIT_PARAMS ) );

      params.dwSize                 = sizeof( DXDIAG_INIT_PARAMS );
      params.dwDxDiagHeaderVersion  = 111;
      params.bAllowWHQLChecks       = false;

      HRESULT result = mDxDiagProvider->Initialize( &params );
      if( result != S_OK )
      {
         Con::errorf( "WMIVideoInfo: DxDiag initialization failed (%i)", result );
         SAFE_RELEASE( mDxDiagProvider );
         return false;
      }
      else
      {
         Con::printf( "WMIVideoInfo: DxDiag initialized" );
         return true;
      }
   }

   return false;
}

//------------------------------------------------------------------------------
// http://msdn2.microsoft.com/en-us/library/aa394512.aspx
// 
// The Win32_VideoController WMI class represents the capabilities and management capacity of the 
// video controller on a computer system running Windows.
//
// Starting with Windows Vista, hardware that is not compatible with Windows Display Driver Model (WDDM) 
// returns inaccurate property values for instances of this class.
//
// Windows Server 2003, Windows XP, Windows 2000, and Windows NT 4.0:  This class is supported.
//------------------------------------------------------------------------------

bool WMIVideoInfo::_queryProperty( const PVIQueryType queryType, const U32 adapterId, String *outValue )
{
   if( _queryPropertyDXGI( queryType, adapterId, outValue ) )
      return true;
   else if( _queryPropertyDxDiag( queryType, adapterId, outValue ) )
      return true;
   else
      return _queryPropertyWMI( queryType, adapterId, outValue );
}

bool WMIVideoInfo::_queryPropertyDxDiag( const PVIQueryType queryType, const U32 adapterId, String *outValue )
{
   if( mDxDiagProvider != 0 )
   {
      IDxDiagContainer* rootContainer = 0;
      IDxDiagContainer* displayDevicesContainer = 0;
      IDxDiagContainer* deviceContainer = 0;

      // Special case to deal with PVI_NumAdapters
      if(queryType == PVI_NumAdapters)
      {
         DWORD count = 0;
         String value;

         if( mDxDiagProvider->GetRootContainer( &rootContainer ) == S_OK
            && rootContainer->GetChildContainer( L"DxDiag_DisplayDevices", &displayDevicesContainer ) == S_OK
            && displayDevicesContainer->GetNumberOfChildContainers( &count ) == S_OK )
         {
            value = String::ToString("%d", count);
         }

         if( rootContainer )
            SAFE_RELEASE( rootContainer );
         if( displayDevicesContainer )
            SAFE_RELEASE( displayDevicesContainer );

         *outValue = value;
         return true;
      }

      WCHAR adapterIdString[ 2 ];
      adapterIdString[ 0 ] = L'0' + adapterId;
      adapterIdString[ 1 ] = L'\0';

      String value;
      if( mDxDiagProvider->GetRootContainer( &rootContainer ) == S_OK
         && rootContainer->GetChildContainer( L"DxDiag_DisplayDevices", &displayDevicesContainer ) == S_OK
         && displayDevicesContainer->GetChildContainer( adapterIdString, &deviceContainer ) == S_OK )
      {
         const WCHAR* propertyName = 0;

         switch( queryType )
         {
         case PVI_Description:
            propertyName = L"szDescription";
            break;

         case PVI_Name:
            propertyName = L"szDeviceName";
            break;

         case PVI_ChipSet:
            propertyName = L"szChipType";
            break;

         case PVI_DriverVersion:
            propertyName = L"szDriverVersion";
            break;

         // Don't get VRAM via DxDiag as that won't tell us about the actual amount of dedicated
         // video memory but rather some dedicated+shared RAM value.
         }

         if( propertyName )
         {
            VARIANT val;
            if( deviceContainer->GetProp( propertyName, &val ) == S_OK )
               switch( val.vt )
               {
               case VT_BSTR:
                  value = String( val.bstrVal );
                  break;

               default:
                  AssertWarn( false, avar( "WMIVideoInfo: property type '%i' not implemented", val.vt ) );
               }
         }
      }

      if( rootContainer )
         SAFE_RELEASE( rootContainer );
      if( displayDevicesContainer )
         SAFE_RELEASE( displayDevicesContainer );
      if( deviceContainer )
         SAFE_RELEASE( deviceContainer );

      if( value.isNotEmpty() )
      {
         // Try to get the DxDiag data into some canonical form.  Otherwise, we
         // won't be giving the card profiler much opportunity for matching up
         // its data with profile scripts.

         switch( queryType )
         {
         case PVI_ChipSet:
            if( value.compare( "ATI", 3, String::NoCase ) == 0 )
               value = "ATI Technologies Inc.";
            else if( value.compare( "NVIDIA", 6, String::NoCase ) == 0 )
               value = "NVIDIA";
            else if( value.compare( "INTEL", 5, String::NoCase ) == 0 )
               value = "INTEL";
            else if( value.compare( "MATROX", 6, String::NoCase ) == 0 )
               value = "MATROX";
            break;

         case PVI_Description:
            if( value.compare( "ATI ", 4, String::NoCase ) == 0 )
            {
               value = value.substr( 4, value.length() - 4 );
               if( value.compare( " Series", 7, String::NoCase | String::Right ) == 0 )
                  value = value.substr( 0, value.length() - 7 );
            }
            else if( value.compare( "NVIDIA ", 7, String::NoCase ) == 0 )
               value = value.substr( 7, value.length() - 7 );
            else if( value.compare( "INTEL ", 6, String::NoCase ) == 0 )
               value = value.substr( 6, value.length() - 6 );
            else if( value.compare( "MATROX ", 7, String::NoCase ) == 0 )
               value = value.substr( 7, value.length() - 7 );
            break;
         }

         *outValue = value;
         return true;
      }
   }
   return false;
}

bool WMIVideoInfo::_queryPropertyDXGI( const PVIQueryType queryType, const U32 adapterId, String *outValue )
{

   if( mDXGIFactory )
   {
      // Special case to deal with PVI_NumAdapters
      if (queryType == PVI_NumAdapters)
      {
         U32 count = 0;
         IDXGIAdapter1 *adapter;
         while (mDXGIFactory->EnumAdapters1(count, &adapter) != DXGI_ERROR_NOT_FOUND)
         {
            ++count;
            adapter->Release();
         }

         String value = String::ToString("%d", count);
         *outValue = value;
         return true;
      }

      IDXGIAdapter1* adapter;
      if( mDXGIFactory->EnumAdapters1( adapterId, &adapter ) != S_OK )
         return false;

      DXGI_ADAPTER_DESC1 desc;
      if( adapter->GetDesc1( &desc ) != S_OK )
      {
         adapter->Release();
         return false;
      }
 
      String value;
      switch( queryType )
      {
      case PVI_Description:
         value = String( desc.Description );
         break;

      case PVI_Name:
         value = String( avar( "%i", desc.DeviceId ) );
         break;

      case PVI_VRAM:
         value = String( avar( "%i", desc.DedicatedVideoMemory / 1048576 ) );
         break;
      case PVI_ChipSet:
         value = _lookUpVendorId(desc.VendorId);
         break;
      //TODO PVI_DriverVersion
      }

      adapter->Release();
      *outValue = value;
      return true;
   }

   return false;
}

bool WMIVideoInfo::_queryPropertyWMI( const PVIQueryType queryType, const U32 adapterId, String *outValue )
{
   if( mServices == NULL )
      return false;

   BSTR bstrWQL  = SysAllocString(L"WQL");
   BSTR bstrPath = SysAllocString(L"select * from Win32_VideoController");
   IEnumWbemClassObject* enumerator;
   
   // Use the IWbemServices pointer to make requests of WMI
   HRESULT hr = mServices->ExecQuery(bstrWQL, bstrPath, WBEM_FLAG_FORWARD_ONLY, NULL, &enumerator);

   if( FAILED( hr ) )
      return false;

   IWbemClassObject *adapter = NULL;   
   ULONG uReturned;

   // Get the appropriate adapter.
   for ( S32 i = 0; i <= adapterId; i++ )
   {
      hr = enumerator->Next(WBEM_INFINITE, 1, &adapter, &uReturned );

      if ( FAILED( hr ) || uReturned == 0 )
      {
         enumerator->Release();
         return false;         
      }
   }

   // Now get the property
   VARIANT v;
   hr = adapter->Get( smPVIQueryTypeToWMIString[queryType], 0, &v, NULL, NULL );

   bool result = SUCCEEDED( hr );

   if ( result )
   {
      switch( v.vt )
      {
      case VT_I4:
         {
            LONG longVal = v.lVal;

            if( queryType == PVI_VRAM )
            {
               longVal = longVal >> 20; // Convert to megabytes

               // While this value is reported as a signed integer, it is possible
               // for video cards to have 2GB or more.  In those cases the signed
               // bit is set and will give us a negative number.  Treating this
               // as unsigned will allows us to handle video cards with up to
               // 4GB of memory.  After that we'll need a new solution from Microsoft.
               *outValue = String::ToString( (U32)longVal );
            }
            else
            {
               *outValue = String::ToString( (S32)longVal );
            }
            break;
         }

      case VT_UI4:
         {
            *outValue = String::ToString( (U32)v.ulVal );
            break;
         }

      case VT_BSTR:
         {
            *outValue = String( v.bstrVal );
            break;
         }            
      case VT_LPSTR:
      case VT_LPWSTR:
         break;
      }


   }                  

   // Cleanup      
   adapter->Release();   
   enumerator->Release();

   return result;
}