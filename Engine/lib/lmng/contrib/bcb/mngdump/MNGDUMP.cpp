//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
USERES("MngDump.res");
USEFORM("Main.cpp", MainForm);
USEUNIT("Chunks.cpp");
USEUNIT("About.cpp");
USEUNIT("Callback.cpp");
USEUNIT("Help.cpp");
USELIB("..\..\..\bcb\win32dll\libmng.lib");
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
  try
  {
    Application->Initialize();
    Application->Title = "MngDump";
    Application->CreateForm(__classid(TMainForm), &MainForm);
        Application->Run();
  }
  catch (Exception &exception)
  {
    Application->ShowException(&exception);
  }
  return 0;
}
//---------------------------------------------------------------------------
