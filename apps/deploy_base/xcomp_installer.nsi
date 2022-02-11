#NSIS Modern User Interface
#Start Menu Folder Selection Example Script
#Written by Joost Verburg
#Modified by Davide Pasca
#Modified by Marco Azimonti

#--------------------------------
SetCompressor /SOLID lzma

#--------------------------------
#Include Modern UI

  !include "MUI.nsh"

#--------------------------------
#Configuration
#!define MUI_TEXT_FINISH_SHOWREADME "Run ${SUITE_NAME}"
!define MUI_COMPONENTSPAGE_NODESC

#--------------------------------
#General
    !define COMPANYNAME             "Gugen Studio Inc."
    !define ABOUTURL                "www.gugenstudio.co.jp"
    !define SUITE_NAME              "xComp"
    !define XCOMP_APP_NAME          "xComp"
    !define OPEN_APPDATA_DIR_XCOMP  "xComp"

    #!define MANUAL_LOCATION         https://SERVER/share/man/xComp_User_Manual.en.pdf

    Name "${SUITE_NAME}"

    # parse the version numbers from the source
    !searchparse /file ..\apps\src_common\GTVersions.h \
            `#define GTV_SUITE_VERSION "` VER_MAJOR `.` VER_MINOR `.` VER_REV `"`

    #!define VER_MAJOR 1
    #!define VER_MINOR 0
    #!define VER_REV   1

    !define XCOMP_EXE_FULLPATH "$INSTDIR\bin\xcomp.exe"

    !define REG_UNINSTALL_BASE \
                "Software\Microsoft\Windows\CurrentVersion\Uninstall\${SUITE_NAME}"

    OutFile "${SUITE_NAME}_${VER_MAJOR}.${VER_MINOR}.${VER_REV}_Setup.exe"

    # Default installation folder
    InstallDir "$PROGRAMFILES64\${SUITE_NAME}"

    # Get installation folder from registry if available
    InstallDirRegKey HKCU "Software\${SUITE_NAME}" ""

    Icon "..\apps\resources\xcomp_icon.ico"

#--------------------------------
#Variables

  Var MUI_TEMP
  Var STARTMENU_FOLDER

#--------------------------------
#Interface Settings

  !define MUI_ABORTWARNING

#--------------------------------
#Pages
    !insertmacro MUI_PAGE_WELCOME
    !insertmacro MUI_PAGE_LICENSE "xcomp\license.txt"

    !insertmacro MUI_PAGE_COMPONENTS

    !insertmacro MUI_PAGE_DIRECTORY

    !define MUI_STARTMENUPAGE_REGISTRY_ROOT "HKCU"
    !define MUI_STARTMENUPAGE_REGISTRY_KEY "Software\${SUITE_NAME}"
    !define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "Start Menu Folder"
    !insertmacro MUI_PAGE_STARTMENU Application $STARTMENU_FOLDER

    !insertmacro MUI_PAGE_INSTFILES


    # These indented statements modify settings for MUI_PAGE_FINISH
    #!define MUI_FINISHPAGE_NOAUTOCLOSE
    #!define MUI_FINISHPAGE_RUN      ${XCOMP_EXE_FULLPATH}
    !define MUI_FINISHPAGE_RUN
    !define MUI_FINISHPAGE_RUN_FUNCTION RunAtFinishFunc
    # TMPTMP
    #MessageBox MB_OK ${XCOMP_EXE_FULLPATH}

    !define MUI_FINISHPAGE_RUN_CHECKED
    #!define MUI_FINISHPAGE_RUN_TEXT "Start a shortcut"
    #!define MUI_FINISHPAGE_RUN_FUNCTION "LaunchLink"
    #!define MUI_FINISHPAGE_SHOWREADME_NOTCHECKED
    #!define MUI_FINISHPAGE_SHOWREADME ${MANUAL_LOCATION}
    !insertmacro MUI_PAGE_FINISH

Function RunAtFinishFunc
    SetOutPath $INSTDIR

    IfFileExists "$INSTDIR\bin\xcomp.exe" 0 +3
        Exec '"$INSTDIR\bin\xcomp.exe"'
        goto _end

_end:
FunctionEnd

    !insertmacro MUI_UNPAGE_CONFIRM
    !insertmacro MUI_UNPAGE_INSTFILES

#--------------------------------
#Languages

  !insertmacro MUI_LANGUAGE "English"
  #!insertmacro MUI_LANGUAGE "Italian"
  #!insertmacro MUI_LANGUAGE "Japanese"

#--------------------------------
#Installer Sections
Section "Core Files" SecCoreFiles
SectionIn RO
    SetOutPath "$INSTDIR"

    File /r /x "xcomp.exe" \
            /x "run.sh" \
            "xcomp\*"

    # Store installation folder
    WriteRegStr HKCU "Software\${SUITE_NAME}" "" $INSTDIR

    # Create uninstaller
    WriteUninstaller "$INSTDIR\Uninstall.exe"

    # write keys for the control panel
    WriteRegStr   HKLM ${REG_UNINSTALL_BASE} \
                            "DisplayName" \
                            "${SUITE_NAME} ${VER_MAJOR}.${VER_MINOR}.${VER_REV}"

    WriteRegStr   HKLM ${REG_UNINSTALL_BASE} "Publisher"   "${COMPANYNAME}"
    WriteRegStr   HKLM ${REG_UNINSTALL_BASE} "UninstallString" "$\"$INSTDIR\Uninstall.exe$\""

    WriteRegStr   HKLM ${REG_UNINSTALL_BASE} \
                            "QuietUninstallString" \
                            "$\"$INSTDIR\uninstall.exe$\" /S"

    WriteRegStr   HKLM ${REG_UNINSTALL_BASE} "URLInfoAbout" "$\"${ABOUTURL}$\""

    WriteRegStr   HKLM ${REG_UNINSTALL_BASE} \
                            "DisplayVersion" \
                            "${VER_MAJOR}.${VER_MINOR}.${VER_REV}"

    WriteRegDWORD HKLM ${REG_UNINSTALL_BASE} "VersionMajor" ${VER_MAJOR}
    WriteRegDWORD HKLM ${REG_UNINSTALL_BASE} "VersionMinor" ${VER_MINOR}

    WriteRegDWORD HKLM ${REG_UNINSTALL_BASE} "NoModify" 1
    WriteRegDWORD HKLM ${REG_UNINSTALL_BASE} "NoRepair" 1

    !insertmacro MUI_STARTMENU_WRITE_BEGIN Application

    # Create shortcuts
    CreateDirectory "$SMPROGRAMS\$STARTMENU_FOLDER"

    CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\Uninstall.lnk" \
                                                        "$INSTDIR\Uninstall.exe"

    #CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\User Manual.lnk" \
    #                                                    "${MANUAL_LOCATION}"

    !insertmacro MUI_STARTMENU_WRITE_END

SectionEnd

#==================================================================
SectionGroup "!xComp"

Section "Executables" SecClient
    #SectionIn RO
    SetOutPath "$INSTDIR"
    File "/oname=bin\xcomp.exe" "xcomp\bin\xcomp.exe"

    # Create shortcuts
    !insertmacro MUI_STARTMENU_WRITE_BEGIN Application

    CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\${XCOMP_APP_NAME}.lnk" \
                                                        "${XCOMP_EXE_FULLPATH}"

    !insertmacro MUI_STARTMENU_WRITE_END
SectionEnd

# Optional section (can be disabled by the user)
Section "Desktop Shortcut" SecClient_Desktop
    IfSilent +2
        CreateShortCut "$DESKTOP\${XCOMP_APP_NAME}.lnk" ${XCOMP_EXE_FULLPATH} ""
SectionEnd

# Optional section (can be disabled by the user)
Section "Quick Launch Shortcut" SecClient_Quick
    IfSilent +2
        CreateShortCut "$QUICKLAUNCH\${XCOMP_APP_NAME}.lnk" ${XCOMP_EXE_FULLPATH} ""
SectionEnd

# Optional section (can be disabled by the user)
#Section "Run After Login (suggested)" SecClient_RunLogin
#    IfSilent +2
#        WriteRegStr HKCU \
#            "SOFTWARE\Microsoft\Windows\CurrentVersion\Run" \
#            "${XCOMP_APP_NAME}" \
#            "$\"${XCOMP_EXE_FULLPATH}$\" --cd $\"$INSTDIR$\" --minimized"
#SectionEnd

SectionGroupEnd

#==================================================================
Section "-run after silent install"
    IfSilent 0 +2
        Exec ${XCOMP_EXE_FULLPATH}
SectionEnd

#==================================================================
SectionGroup "Run-time Components"

Section "VC++ Redistributable (required once)" SecVCRedist
    ExecWait '"$INSTDIR\other\VC_redist.x64.exe"  /passive /norestart'
SectionEnd

#Section /o "Mesa 3D (for systems with no GPU) " SecOpenGLSW
#    File "/oname=bin\opengl32.dll" "xcomp\other\opengl32.dll"
#SectionEnd

SectionGroupEnd

#--------------------------------
#Descriptions

#  #Language strings
#  LangString DESC_SecCoreFiles ${LANG_ENGLISH} "Required files to use ${SUITE_NAME}."
#
#  #Assign language strings to sections
#  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
#    !insertmacro MUI_DESCRIPTION_TEXT ${SecCoreFiles} $(DESC_SecCoreFiles)
#  !insertmacro MUI_FUNCTION_DESCRIPTION_END

#--------------------------------
#Uninstaller Section

Section "Uninstall"

    Delete "$DESKTOP\${XCOMP_APP_NAME}.lnk"
    Delete "$QUICKLAUNCH\${XCOMP_APP_NAME}.lnk"

    RMDir /r    "$INSTDIR"

    !insertmacro MUI_STARTMENU_GETFOLDER Application $MUI_TEMP

    Delete "$SMPROGRAMS\$MUI_TEMP\${XCOMP_APP_NAME}.lnk"
    Delete "$SMPROGRAMS\$MUI_TEMP\${XCOMP_APP_NAME}.cfg"
    Delete "$SMPROGRAMS\$MUI_TEMP\Uninstall.lnk"
    Delete "$SMPROGRAMS\$MUI_TEMP\User Manual.lnk"

    # Delete empty start menu parent diretories
    StrCpy $MUI_TEMP "$SMPROGRAMS\$MUI_TEMP"

    startMenuDeleteLoop:
    ClearErrors
    RMDir $MUI_TEMP
    GetFullPathName $MUI_TEMP "$MUI_TEMP\.."

    IfErrors startMenuDeleteLoopDone

    StrCmp $MUI_TEMP $SMPROGRAMS startMenuDeleteLoopDone startMenuDeleteLoop
    startMenuDeleteLoopDone:

    DeleteRegKey /ifempty HKCU "Software\${SUITE_NAME}"
    DeleteRegValue HKCU "SOFTWARE\Microsoft\Windows\CurrentVersion\Run" "${SUITE_NAME}"

    # erase keys for control panel
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${SUITE_NAME}"

    ExecShell "" "$APPDATA\${OPEN_APPDATA_DIR_XCOMP}"

SectionEnd

#==================================================================
Function .onInit

FunctionEnd

