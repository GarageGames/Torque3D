//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
USERES("MngView.res");
USEFORM("Main.cpp", MainForm);
USEFILE("README.txt");
USELIB("..\win32dll\libmng.lib");
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
  try
  {
    Application->Initialize();
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
