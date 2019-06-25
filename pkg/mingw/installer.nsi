!include MUI2.nsh

; General
Name "DivvyDroid"
OutFile "DivvyDroidSetup.exe"

SetCompressor /SOLID lzma

InstallDir "$PROGRAMFILES\DivvyDroid"
InstallDirRegKey HKCU "Software\DivvyDroid" ""
RequestExecutionLevel admin

; Interface
!define MUI_ABORTWARNING

; Pages
!insertmacro MUI_PAGE_LICENSE "../LICENSE"
;!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

; Languages
!insertmacro MUI_LANGUAGE "English"
!define MUI_PAGE_HEADER_TEXT "DivvyDroid Installation"
!define MUI_PAGE_HEADER_SUBTEXT "Application to screencast and remote control Android devices"

; Installer
Section "StartMenu" SecStartMenu
	CreateDirectory "$SMPROGRAMS\DivvyDroid"
	CreateShortcut "$SMPROGRAMS\DivvyDroid\DivvyDroid.lnk" "$INSTDIR\DivvyDroid.exe"
	CreateShortcut "$SMPROGRAMS\DivvyDroid\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
SectionEnd

Section "Installation" SecInstall
	SetOutPath "$INSTDIR"
	File "src/divvydroid.exe"
	; Following will be replaced by nsi-installer.sh
	InstallDeps
	
	SetOutPath "$INSTDIR\platforms"
	File "/usr/i686-w64-mingw32/lib/qt/plugins/platforms/qwindows.dll"
	SetOutPath "$INSTDIR\audio"
	File "/usr/i686-w64-mingw32/lib/qt/plugins/audio/qtaudio_windows.dll"
	SetOutPath "$INSTDIR\printsupport"
	File "/usr/i686-w64-mingw32/lib/qt/plugins/printsupport/windowsprintersupport.dll"
	SetOutPath "$INSTDIR\styles"
	File "/usr/i686-w64-mingw32/lib/qt/plugins/styles/qwindowsvistastyle.dll"
	SetOutPath "$INSTDIR\iconengines"
	File "/usr/i686-w64-mingw32/lib/qt/plugins/iconengines/qsvgicon.dll"
	SetOutPath "$INSTDIR\imageformats"
	File "/usr/i686-w64-mingw32/lib/qt/plugins/imageformats/qico.dll"
	File "/usr/i686-w64-mingw32/lib/qt/plugins/imageformats/qjpeg.dll"
	File "/usr/i686-w64-mingw32/lib/qt/plugins/imageformats/qsvg.dll"
	File "/usr/i686-w64-mingw32/lib/qt/plugins/imageformats/qtiff.dll"

	; Store installation folder
	WriteRegStr HKCU "Software\DivvyDroid" "" $INSTDIR
	
	; Create uninstaller
	WriteUninstaller "$INSTDIR\Uninstall.exe"
SectionEnd

LangString DESC_SecStartMenu ${LANG_ENGLISH} "Start Menu Shortcuts"
LangString DESC_SecInstall ${LANG_ENGLISH} "DivvyDroid Application"
!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
!insertmacro MUI_DESCRIPTION_TEXT ${SecStartMenu} $(DESC_SecStartMenu)
!insertmacro MUI_DESCRIPTION_TEXT ${SecInstall} $(DESC_SecInstall)
!insertmacro MUI_FUNCTION_DESCRIPTION_END

; Uninstaller
Section "Uninstall"
	RMDir /r "$INSTDIR"
	RMDir /r "$SMPROGRAMS\DivvyDroid"
	DeleteRegKey HKCU "Software\DivvyDroid"
SectionEnd
