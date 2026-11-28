; Build a Unicode installer so every bundled language renders correctly.
Unicode true

; Set search path
!addincludedir "${__FILEDIR__}"

; Installer initial constants
!define PRODUCT_NAME "CrossDesk"
!define PRODUCT_VERSION "${VERSION}"
!define PRODUCT_VERSION_NUMERIC "${VERSION_NUMERIC}"
!define PRODUCT_PUBLISHER "CrossDesk"
!define PRODUCT_WEB_SITE "https://github.com/thuongtin/crossdesk"
!define APP_NAME "CrossDesk"
!define UNINSTALL_REG_KEY "CrossDesk"
!define PRODUCT_SERVICE_NAME "CrossDeskService"

; Installer and uninstaller icon path
!define MUI_ICON "${__FILEDIR__}\..\..\resources\windows\crossdesk.ico"
!define MUI_UNICON "${__FILEDIR__}\..\..\resources\windows\crossdesk.ico"

; Compression settings
SetCompressor /FINAL lzma

; Request admin privileges (needed to write HKLM)
RequestExecutionLevel admin

; Replace the "Nullsoft Install System" footer with the product name.
BrandingText "${PRODUCT_NAME}"

; ------ MUI Modern UI Definition ------
!include "MUI.nsh"
!define MUI_ABORTWARNING
!insertmacro MUI_PAGE_WELCOME
; Upgrades stay in the folder of the existing installation, see
; LocateExistingInstall.
!define MUI_PAGE_CUSTOMFUNCTION_PRE SkipDirectoryPageOnUpgrade
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES

; Add run-after-install option
!define MUI_FINISHPAGE_RUN
!define MUI_FINISHPAGE_RUN_TEXT "$(CD_RunApp)"
!define MUI_FINISHPAGE_RUN_FUNCTION LaunchApp

!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

; Every language the app ships, plus Turkish and Hebrew, which the app does not
; translate yet. English must stay first: NSIS falls back to the first language
; when the Windows display language matches none of them.
!insertmacro MUI_LANGUAGE "English"
!insertmacro MUI_LANGUAGE "Vietnamese"
!insertmacro MUI_LANGUAGE "SimpChinese"
!insertmacro MUI_LANGUAGE "TradChinese"
!insertmacro MUI_LANGUAGE "Russian"
!insertmacro MUI_LANGUAGE "Japanese"
!insertmacro MUI_LANGUAGE "Korean"
!insertmacro MUI_LANGUAGE "German"
!insertmacro MUI_LANGUAGE "French"
!insertmacro MUI_LANGUAGE "Spanish"
!insertmacro MUI_LANGUAGE "PortugueseBR"
!insertmacro MUI_LANGUAGE "Turkish"
!insertmacro MUI_LANGUAGE "Hebrew"
!insertmacro MUI_RESERVEFILE_INSTALLOPTIONS
; ------ End of MUI Definition ------

; Include LogicLib for process handling
!include "LogicLib.nsh"
!include "FileFunc.nsh"
!include /CHARSET=UTF8 "installer_strings.nsh"
Var UpdateMode
Var ExistingInstall

; NSIS picks the language matching the Windows display language exactly,
; then by primary language, so zh-HK and zh-MO would get Simplified Chinese.
; Map them to Traditional Chinese like the app does.
!macro CROSSDESK_SELECT_LANGUAGE
    System::Call 'kernel32::GetUserDefaultUILanguage() i .r0'
    IntOp $1 $0 & 0x3FF
    ${If} $1 = 0x04
        ${If} $0 = 0x0404
        ${OrIf} $0 = 0x0C04
        ${OrIf} $0 = 0x1404
        ${OrIf} $0 = 0x7C04
            StrCpy $LANGUAGE ${LANG_TRADCHINESE}
        ${Else}
            StrCpy $LANGUAGE ${LANG_SIMPCHINESE}
        ${EndIf}
    ${EndIf}
    ; Translation review builds only, for example
    ; makensis /DCROSSDESK_FORCE_LANGUAGE=1066 forces Vietnamese.
    !ifdef CROSSDESK_FORCE_LANGUAGE
        StrCpy $LANGUAGE ${CROSSDESK_FORCE_LANGUAGE}
    !endif
!macroend

Function .onInit
    ${GetParameters} $0
    ClearErrors
    ${GetOptions} $0 "/CROSSDESK_UPDATE" $1
    StrCpy $UpdateMode 0
    ${IfNot} ${Errors}
        StrCpy $UpdateMode 1
    ${EndIf}
    !insertmacro CROSSDESK_SELECT_LANGUAGE
    Call LocateExistingInstall
FunctionEnd

Function un.onInit
    !insertmacro CROSSDESK_SELECT_LANGUAGE
FunctionEnd

Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"
OutFile "crossdesk-win-x64-${PRODUCT_VERSION}.exe"
VIProductVersion "${PRODUCT_VERSION_NUMERIC}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "CompanyName" "CrossDesk"
VIAddVersionKey /LANG=${LANG_ENGLISH} "FileDescription" "CrossDesk Installer"
VIAddVersionKey /LANG=${LANG_ENGLISH} "FileVersion" "${PRODUCT_VERSION}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "LegalCopyright" "Copyright (C) CrossDesk contributors"
VIAddVersionKey /LANG=${LANG_ENGLISH} "OriginalFilename" "crossdesk-win-x64-${PRODUCT_VERSION}.exe"
VIAddVersionKey /LANG=${LANG_ENGLISH} "ProductName" "${PRODUCT_NAME}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "ProductVersion" "${PRODUCT_VERSION}"
; CrossDesk is a 64-bit application. Existing installations, including
; v1.7.7-3 and older in Program Files (x86), are found by LocateExistingInstall.
InstallDir "$PROGRAMFILES64\${PRODUCT_NAME}"
ShowInstDetails show

Section "MainSection" SEC_MAIN
    ; The updater has already coordinated shutdown. Never kill unrelated
    ; instances by name during an unattended upgrade.
    ${If} $UpdateMode = 1
        Goto installApp
    ${EndIf}
    ; Check if CrossDesk is running
    StrCpy $1 "CrossDesk.exe"
    
    nsProcess::_FindProcess "$1"
    Pop $R0
    ${If} $R0 = 0  ;
        MessageBox MB_ICONQUESTION|MB_YESNO "$(CD_CloseRunningInstall)" IDYES closeApp IDNO cancelInstall
    ${Else}
        Goto installApp
    ${EndIf}

closeApp:
    nsProcess::_KillProcess "$1"
    Pop $R0
    Sleep 500
    Goto installApp

cancelInstall:
    SetDetailsPrint both
    MessageBox MB_ICONEXCLAMATION|MB_OK "$(CD_InstallAborted)"
    Abort

installApp:
    Call StopInstalledService

    SetOutPath "$INSTDIR"
    ; The signed release determines file versions, not filesystem timestamps.
    SetOverwrite on

    ; Main application executable path
    File /oname=CrossDesk.exe "..\..\..\..\build\windows\x64\release\crossdesk.exe"
    File "..\..\..\..\build\windows\x64\release\crossdesk_updater.exe"
    ; Bundle service-side binaries required by the Windows service flow
    File "..\..\..\..\build\windows\x64\release\crossdesk_service.exe"
    File "..\..\..\..\build\windows\x64\release\crossdesk_session_helper.exe"
    File "..\..\..\..\build\windows\x64\release\crossdesk_privacy_cursor_helper.exe"
    File "..\..\..\..\build\windows\x64\release\crossdesk_privacy_window.dll"
    ; Require the Slint runtime so an incomplete build cannot be packaged.
    File "..\..\..\..\build\windows\x64\release\slint_cpp.dll"
    ; Bundle the remaining runtime DLLs from the release output directory
    File /x crossdesk_privacy_window.dll /x slint_cpp.dll "..\..\..\..\build\windows\x64\release\*.dll"
    ; Amyuni usbmmidd_v2 virtual display driver for headless hosts. xmake
    ; copies apps\desktop\resources\windows\usbmmidd_v2 into the release
    ; output directory after building crossdesk; its License.txt ships with it.
    File /r "..\..\..\..\build\windows\x64\release\usbmmidd_v2"

    Call RegisterInstalledService

    ; Import the previous user's startup preference after installing service
    ; binaries. The migration CLI also runs during a silent updater install.
    ClearErrors
    ExecWait '"$INSTDIR\CrossDesk.exe" --autostart-migrate' $0
    ${If} ${Errors}
    ${OrIf} $0 != 0
        DetailPrint "$(CD_AutostartRetry)"
        IfSilent +2
        MessageBox MB_ICONEXCLAMATION|MB_OK "$(CD_AutostartRetry)"
    ${EndIf}

    ; Write uninstall information. The entry stays in the 32-bit registry view
    ; where earlier releases wrote it: the in-app updater reads InstallLocation
    ; from WOW6432Node, and a second entry would duplicate the program in
    ; Installed apps.
    WriteUninstaller "$INSTDIR\uninstall.exe"

    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${UNINSTALL_REG_KEY}" "DisplayName" "${PRODUCT_NAME}"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${UNINSTALL_REG_KEY}" "UninstallString" "$INSTDIR\uninstall.exe"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${UNINSTALL_REG_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${UNINSTALL_REG_KEY}" "InstallLocation" "$INSTDIR"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${UNINSTALL_REG_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${UNINSTALL_REG_KEY}" "URLInfoAbout" "${PRODUCT_WEB_SITE}"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${UNINSTALL_REG_KEY}" "DisplayIcon" "$INSTDIR\CrossDesk.exe"
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${UNINSTALL_REG_KEY}" "NoModify" 1
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${UNINSTALL_REG_KEY}" "NoRepair" 1
    WriteRegStr HKCU "Software\${PRODUCT_NAME}" "InstallDir" "$INSTDIR"
SectionEnd

Section -AdditionalIcons SEC_ICONS
    ; Desktop shortcut
    CreateShortCut "$DESKTOP\${PRODUCT_NAME}.lnk" "$INSTDIR\CrossDesk.exe" "" "$INSTDIR\CrossDesk.exe"

    ; Start menu shortcut
    CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME}.lnk" "$INSTDIR\CrossDesk.exe" "" "$INSTDIR\CrossDesk.exe"
SectionEnd

Section "Uninstall"
    ; Check if CrossDesk is running
    StrCpy $1 "CrossDesk.exe"
    
    nsProcess::_FindProcess "$1"
    Pop $R0
    ${If} $R0 = 0
        MessageBox MB_ICONQUESTION|MB_YESNO "$(CD_CloseRunningUninstall)" IDYES closeApp IDNO cancelUninstall
    ${Else}
        Goto uninstallApp
    ${EndIf}

closeApp:
    nsProcess::_KillProcess "$1"
    Pop $R0
    Sleep 500
    Goto uninstallApp

cancelUninstall:
    SetDetailsPrint both
    MessageBox MB_ICONEXCLAMATION|MB_OK "$(CD_UninstallAborted)"
    Abort

uninstallApp:
    ClearErrors
    ExecWait '"$INSTDIR\CrossDesk.exe" --autostart-uninstall' $0
    ${If} ${Errors}
    ${OrIf} $0 != 0
        MessageBox MB_ICONEXCLAMATION|MB_OK "$(CD_AutostartRemoveFailed)"
    ${EndIf}
    Call un.UnregisterInstalledService
    Call un.RemoveUsbmmiddDriver

    ; Delete main executable and uninstaller
    Delete "$INSTDIR\CrossDesk.exe"
    Delete "$INSTDIR\crossdesk_updater.exe"
    Delete "$INSTDIR\crossdesk_service.exe"
    Delete "$INSTDIR\crossdesk_session_helper.exe"
    Delete "$INSTDIR\crossdesk_privacy_cursor_helper.exe"
    Delete "$INSTDIR\crossdesk_privacy_window.dll"
    Delete "$INSTDIR\uninstall.exe"

    ; Leave the install directory before removing it
    SetOutPath "$TEMP"

    ; Recursively delete installation directory
    RMDir /r "$INSTDIR"

    ; Delete desktop and start menu shortcuts
    Delete "$DESKTOP\${PRODUCT_NAME}.lnk"
    Delete "$SMPROGRAMS\${PRODUCT_NAME}.lnk"

    ; Delete registry uninstall entry
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${UNINSTALL_REG_KEY}"

    ; Delete remembered install dir
    DeleteRegKey HKCU "Software\${PRODUCT_NAME}"

    ; Recursively delete CrossDesk folder in user AppData
    RMDir /r "$APPDATA\CrossDesk"
    RMDir /r "$LOCALAPPDATA\CrossDesk"
SectionEnd

; ------ Functions ------
Function LaunchApp
    ${If} $UpdateMode = 1
        Return
    ${EndIf}
    Exec "$INSTDIR\CrossDesk.exe"
FunctionEnd

Function .onInstFailed
    SetErrorLevel 1
FunctionEnd

; New installations go to the 64-bit Program Files folder. An existing
; installation is upgraded or repaired where it is, even in Program Files
; (x86): the updater relaunches CrossDesk.exe from its old path, and the
; service, the per-user auto-start tasks and the firewall rules all point at
; that path. Moving it would also need the old uninstaller, which deletes user
; data. The uninstall entry marks a registered installation, so its folder is
; kept even when CrossDesk.exe was deleted or quarantined. The per-user
; InstallDir survives an uninstall run from another account, so it only
; counts while CrossDesk.exe is still there.
; A folder passed with /D always wins, even when it is the default one. The
; updater always passes /D, which NSIS applies before .onInit, so a
; non-default $INSTDIR means the caller chose the folder. For the default
; folder, NSIS cuts its copy of the command line ($CMDLINE) at " /D=", so a
; $CMDLINE shorter than the process command line means /D named it.
Function LocateExistingInstall
    StrCpy $ExistingInstall 0
    ${If} $INSTDIR != "$PROGRAMFILES64\${PRODUCT_NAME}"
        Return
    ${EndIf}
    System::Call 'kernel32::GetCommandLineW() w .r0'
    StrLen $0 $0
    StrLen $1 $CMDLINE
    ${If} $1 < $0
        Return
    ${EndIf}
    ReadRegStr $0 HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${UNINSTALL_REG_KEY}" "InstallLocation"
    ${If} $0 == ""
        ReadRegStr $1 HKCU "Software\${PRODUCT_NAME}" "InstallDir"
        ${If} $1 != ""
        ${AndIf} ${FileExists} "$1\CrossDesk.exe"
            StrCpy $0 $1
        ${EndIf}
    ${EndIf}
    ${If} $0 != ""
        StrCpy $INSTDIR $0
        StrCpy $ExistingInstall 1
    ${EndIf}
FunctionEnd

; Upgrades skip the directory page, and with it the check there that keeps
; Install disabled while the drive lacks room for the files. Repeat that check
; so an interactive upgrade on a full drive stops before CrossDesk is closed,
; its service is stopped or any file is overwritten. Silent installs never
; showed the page, so the updater keeps its earlier behavior.
Function SkipDirectoryPageOnUpgrade
    ${If} $ExistingInstall <> 1
        Return
    ${EndIf}
    ; Section sizes are in KB; compare in MB so drives over 2 TB do not
    ; overflow 32-bit integers.
    SectionGetSize ${SEC_MAIN} $0
    SectionGetSize ${SEC_ICONS} $1
    IntOp $0 $0 + $1
    IntOp $0 $0 + 1023
    IntOp $0 $0 / 1024
    ; Ask about the folder itself, as the directory page does, not the drive
    ; root: a folder can be the mount point of another volume. Only a folder
    ; that does not exist (yet) is checked through its nearest existing
    ; parent. Any other failure, such as access denied on an existing mount
    ; point, skips the check: the parent may be on a different volume.
    StrCpy $1 $INSTDIR
    ${Do}
        ; Always pass a trailing backslash. GetParent turns D:\CrossDesk into
        ; D:, which names the current directory on drive D: (that can be a
        ; mount point of another volume), not the root of D:.
        StrCpy $7 $1 "" -1
        ${If} $7 == "\"
            StrCpy $7 $1
        ${Else}
            StrCpy $7 "$1\"
        ${EndIf}
        System::Call 'kernel32::GetDiskFreeSpaceExW(w r7, *l .r2, *l .r4, *l .r5) i .r3 ?e'
        Pop $6
        ${If} $3 <> 0
            ${ExitDo}
        ${EndIf}
        ; ERROR_FILE_NOT_FOUND and ERROR_PATH_NOT_FOUND.
        ${If} $6 <> 2
        ${AndIf} $6 <> 3
            ${ExitDo}
        ${EndIf}
        ${GetParent} $1 $1
    ${LoopUntil} $1 == ""
    ${If} $3 <> 0
        System::Int64Op $2 / 1048576
        Pop $2
        ${If} $2 < $0
            MessageBox MB_ICONSTOP|MB_OK "$(CD_UpgradeDiskFull)$\r$\n$\r$\n$(^SpaceRequired)$0$(^Mega)$(^Byte)$\r$\n$(^SpaceAvailable)$2$(^Mega)$(^Byte)"
            SetErrorLevel 1
            Quit
        ${EndIf}
    ${EndIf}
    Abort
FunctionEnd

Function StopInstalledService
    IfFileExists "$INSTDIR\CrossDesk.exe" 0 stop_with_sc
    IfFileExists "$INSTDIR\crossdesk_service.exe" 0 stop_with_sc

    DetailPrint "$(CD_StoppingService)"
    ExecWait '"$INSTDIR\CrossDesk.exe" --service-stop' $0
    ${If} $0 = 0
        Return
    ${EndIf}

stop_with_sc:
    DetailPrint "$(CD_StoppingServiceSc)"
    ExecWait '"$SYSDIR\sc.exe" stop ${PRODUCT_SERVICE_NAME}' $0
    ${If} $0 != 0
    ${AndIf} $0 != 1060
    ${AndIf} $0 != 1062
        MessageBox MB_ICONSTOP|MB_OK "$(CD_StopServiceFailed)"
        Abort
    ${EndIf}
    Sleep 1500
FunctionEnd

Function RegisterInstalledService
    IfFileExists "$INSTDIR\CrossDesk.exe" 0 missing_service_binary
    IfFileExists "$INSTDIR\crossdesk_service.exe" 0 missing_service_binary
    IfFileExists "$INSTDIR\crossdesk_session_helper.exe" 0 missing_service_binary

    DetailPrint "$(CD_RegisteringService)"
    ExecWait '"$INSTDIR\CrossDesk.exe" --service-install' $0
    ${If} $0 != 0
        MessageBox MB_ICONSTOP|MB_OK "$(CD_RegisterServiceFailed)"
        Abort
    ${EndIf}

    ; Preserve an existing unattended opt-in and resume it after upgrades.
    ; A fresh install still registers the ordinary on-demand lock-screen service.
    SetRegView 64
    ReadRegDWORD $1 HKLM "Software\CrossDesk\Unattended" "Enabled"
    SetRegView 32
    ${If} $1 = 1
        ExecWait '"$INSTDIR\CrossDesk.exe" --service-start' $0
        ${If} $0 != 0
            MessageBox MB_ICONEXCLAMATION|MB_OK "$(CD_UnattendedRestartFailed)"
        ${EndIf}
    ${EndIf}
    DetailPrint "$(CD_ServiceRegistered)"

    Return

missing_service_binary:
    MessageBox MB_ICONSTOP|MB_OK "$(CD_ServiceFilesMissing)"
    Abort
FunctionEnd

Function un.UnregisterInstalledService
    ; Also disarm unattended startup when removal must fall back to sc.exe.
    SetRegView 64
    DeleteRegKey HKLM "Software\CrossDesk\Unattended"
    SetRegView 32
    IfFileExists "$INSTDIR\CrossDesk.exe" 0 unregister_with_sc

    DetailPrint "$(CD_StoppingService)"
    ExecWait '"$INSTDIR\CrossDesk.exe" --service-stop' $0
    ${If} $0 = 0
        DetailPrint "$(CD_RemovingService)"
        ExecWait '"$INSTDIR\CrossDesk.exe" --service-uninstall' $0
        ${If} $0 = 0
            Return
        ${EndIf}
    ${EndIf}

unregister_with_sc:
    DetailPrint "$(CD_RemovingServiceSc)"
    ExecWait '"$SYSDIR\sc.exe" stop ${PRODUCT_SERVICE_NAME}' $0
    ${If} $0 != 0
    ${AndIf} $0 != 1060
    ${AndIf} $0 != 1062
        MessageBox MB_ICONSTOP|MB_OK "$(CD_UnStopServiceFailed)"
        Abort
    ${EndIf}
    Sleep 1500

    ExecWait '"$SYSDIR\sc.exe" delete ${PRODUCT_SERVICE_NAME}' $0
    ${If} $0 != 0
    ${AndIf} $0 != 1060
        MessageBox MB_ICONSTOP|MB_OK "$(CD_UnRemoveServiceFailed)"
        Abort
    ${EndIf}
FunctionEnd

; The session helper (or an elevated CrossDesk) installs the bundled usbmmidd
; driver on first headless use and records HKLM\SOFTWARE\CrossDesk\
; UsbmmiddDriverInstalled in the 64-bit registry view. Only a driver CrossDesk
; installed is removed here; one set up by the user for other software stays.
; deviceinstaller64.exe resolves usbmmIdd.dll relative to the working
; directory, so run it from the driver folder. Failures are logged, not fatal:
; the driver can still be removed from Device Manager.
Function un.RemoveUsbmmiddDriver
    IfFileExists "$INSTDIR\usbmmidd_v2\deviceinstaller64.exe" 0 done

    SetRegView 64
    ClearErrors
    ReadRegDWORD $0 HKLM "Software\${PRODUCT_NAME}" "UsbmmiddDriverInstalled"
    ${If} ${Errors}
    ${OrIf} $0 != 1
        SetRegView lastused
        Return
    ${EndIf}

    DetailPrint "$(CD_RemovingDriver)"
    SetOutPath "$INSTDIR\usbmmidd_v2"
    ; Unplug every virtual monitor first (the driver allows up to four).
    StrCpy $1 0
unplug_loop:
    nsExec::ExecToLog '"$INSTDIR\usbmmidd_v2\deviceinstaller64.exe" enableidd 0'
    Pop $2
    IntOp $1 $1 + 1
    ${If} $2 = 0
    ${AndIf} $1 < 4
        Goto unplug_loop
    ${EndIf}
    nsExec::ExecToLog '"$INSTDIR\usbmmidd_v2\deviceinstaller64.exe" stop usbmmidd'
    Pop $2
    nsExec::ExecToLog '"$INSTDIR\usbmmidd_v2\deviceinstaller64.exe" remove usbmmidd'
    Pop $2
    ${If} $2 != 0
        DetailPrint "$(CD_DriverRemoveFailed) $2"
    ${EndIf}
    SetOutPath "$TEMP"

    DeleteRegValue HKLM "Software\${PRODUCT_NAME}" "UsbmmiddDriverInstalled"
    DeleteRegKey /ifempty HKLM "Software\${PRODUCT_NAME}"
    SetRegView lastused

done:
FunctionEnd
