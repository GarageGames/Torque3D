;NSIS Modern User Interface version 1.70

;--------------------------------
;Include Modern UI

!include "MUI.nsh"

;--------------------------------
;General

!define  REG_KEY  "<RegKey>"

;Name and file
Name "<Name>"
OutFile "<OutFile>"

;Default installation folder
InstallDir "<InstallDir>"
InstallDirRegKey HKLM "${REG_KEY}" "Install_Dir"
DirText "This will install the $(^Name) on your computer. Please choose a directory"
BrandingText "<BrandingText>"

;--------------------------------
;Interface Settings

!define MUI_ABORTWARNING
!define MUI_ICON "<TemplatePath>\main.ico"
!define MUI_UNICON "<TemplatePath>\main.ico"
!define MUI_COMPONENTSPAGE_SMALLDESC
!define MUI_HEADERIMAGE
!define MUI_WELCOMEFINISHPAGE_BITMAP "<PanelImage>"
!define MUI_HEADERIMAGE_BITMAP "<HeaderImage>"
<FinishLaunch>


;--------------------------------
;Pages

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH


;--------------------------------
;Languages

!insertmacro MUI_LANGUAGE "English"

;--------------------------------
; Game section

Section "!$(^Name) (required)" SecGame
  SectionIn RO
  SetOutPath "$INSTDIR"
  File "<TorqueDir>\Engine\bin\tools\getD3DXVer\app\getD3DXVer.exe"
  File "<TorqueDir>\Engine\bin\tools\dxwebsetup\dxwebsetup.exe"
  File /r "<StagingFiles>"

  ; Start Menu
  SetOutPath "$SMPROGRAMS\$(^Name)"
  <StartMenuShortcuts>
  
  SetOutPath "$INSTDIR"
  <UninstallShortcuts>

  ; Web registry settings
  <WebRegistryInstall>

  ; Registry uninstall
  WriteRegStr HKLM "${REG_KEY}" "" $INSTDIR
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\$(^Name)" \
                 "DisplayName" "$(^Name) (remove only)"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\$(^Name)" \
                 "UninstallString" '"$INSTDIR\uninst-$(^Name).exe"'
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\$(^Name)" \
                 "DisplayIcon" "$INSTDIR\uninst-$(^Name).exe"
  

  ; Check for DX and D3DX version
  ExecWait "$INSTDIR\getD3DXVer.exe"
  IfErrors AskDXInstall NoDXInstall

  AskDXInstall:
    MessageBox MB_YESNO|MB_ICONQUESTION \
             "$(^Name) requires a newer version of DirectX than you have installed on your computer.  Would you like to launch the Microsoft DirectX web installer?" IDNO NoDXInstall

  ; Install DirectX from the web
  ExecWait "$INSTDIR\dxwebsetup.exe"
  IfErrors NoDXInstall NoDXInstall
  
  NoDXInstall:
  ; Do nothing and go onto the next step.
  
  ;Create uninstaller
  WriteUninstaller "$INSTDIR\uninst-$(^Name).exe"
SectionEnd

;--------------------------------
; Desktop Shortcut

Section "Desktop Shortcut" SecDesktop
  ; Desktop
  SetOutPath "$INSTDIR"
  <DesktopShortcuts>
SectionEnd

;--------------------------------
; Readme at the end

<LoadGame>

;--------------------------------
; Uninstaller

Section Uninstall
  MessageBox MB_YESNO|MB_ICONQUESTION \
           "Everything in the $(^Name) directory will be deleted. Are you sure you wish to uninstall the $(^Name)?" \
           IDNO Removed

  ; Shortcuts
  <ShortcutsRemove> 
  RMDir "$SMPROGRAMS\$(^Name)"

  ; Registry
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\$(^Name)"
  DeleteRegKey HKLM "${REG_KEY}"

  ; Web registry uninstall settings
  <WebRegistryUninstall>

  ; SDK Source and Binaries
  RMDir /r $INSTDIR
  IfFileExists $INSTDIR 0 Removed
    MessageBox MB_OK|MB_ICONEXCLAMATION \
               "Note: $INSTDIR could not be removed."
  Removed:
SectionEnd

;--------------------------------
;Descriptions

!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
!insertmacro MUI_DESCRIPTION_TEXT ${SecGame} "Install $(^Name). This component is required."
!insertmacro MUI_DESCRIPTION_TEXT ${SecDesktop} "Create a shortcut on your desktop"
<loadGameInsert>
!insertmacro MUI_FUNCTION_DESCRIPTION_END
