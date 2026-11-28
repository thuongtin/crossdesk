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
!include "x64.nsh"
!include /CHARSET=UTF8 "installer_strings.nsh"
Var UpdateMode
Var ExistingInstall
; Set by the uninstall functions when CrossDesk data or the driver stays.
Var UninstallIncomplete

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
    StrCpy $UninstallIncomplete 0
    ClearErrors
    ExecWait '"$INSTDIR\CrossDesk.exe" --autostart-uninstall' $0
    ${If} ${Errors}
    ${OrIf} $0 != 0
        MessageBox MB_ICONEXCLAMATION|MB_OK "$(CD_AutostartRemoveFailed)"
    ${EndIf}
    Call un.UnregisterInstalledService
    Call un.RemoveUsbmmiddDriver
    Call un.RemoveMachineData

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

    ; The details list names everything the uninstall functions left behind;
    ; open it so the user does not see only "Completed".
    ${If} $UninstallIncomplete = 1
        SetDetailsView show
        MessageBox MB_ICONEXCLAMATION|MB_OK "$(CD_UninstallIncomplete)" /SD IDOK
    ${EndIf}
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
    SetOutPath "$TEMP"

    ; Keep the marker while the driver may still be installed: the app does
    ; not set it again for a driver that is already present, so without it
    ; no later uninstall would remove the driver. remove exits with 0 when no
    ; usbmmidd device is left, which also clears a marker that outlived a
    ; driver removed from Device Manager.
    ${If} $2 == "0"
        DeleteRegValue HKLM "Software\${PRODUCT_NAME}" "UsbmmiddDriverInstalled"
        DeleteRegKey /ifempty HKLM "Software\${PRODUCT_NAME}"
    ${Else}
        DetailPrint "$(CD_DriverRemoveFailed) $2"
        StrCpy $UninstallIncomplete 1
    ${EndIf}
    SetRegView lastused

done:
FunctionEnd

; Removes what CrossDesk keeps outside the install folder and the user
; profiles, and lists in the details what stays. Upgrades never run the
; uninstaller, so all of this survives them.
Function un.RemoveMachineData
    DetailPrint "$(CD_RemovingMachineData)"

    ; --service-uninstall writes Unattended\Enabled=0 again after
    ; un.UnregisterInstalledService deleted the key, so remove it only now.
    ; Keep the usbmmidd marker that un.RemoveUsbmmiddDriver left because the
    ; driver could not be removed, so that a later uninstall still removes it.
    SetRegView 64
    ClearErrors
    ReadRegDWORD $0 HKLM "Software\${PRODUCT_NAME}" "UsbmmiddDriverInstalled"
    ${If} ${Errors}
    ${OrIf} $0 <> 1
        DeleteRegKey HKLM "Software\${PRODUCT_NAME}"
    ${Else}
        DeleteRegKey HKLM "Software\${PRODUCT_NAME}\Unattended"
        DeleteRegValue HKLM "Software\${PRODUCT_NAME}" "LastAttachedDisplayModeV2"
        DeleteRegValue HKLM "Software\${PRODUCT_NAME}" "LastAttachedDisplayMode"
    ${EndIf}
    SetRegView lastused

    ; The service and the unattended host run as SYSTEM and create folders
    ; whose DACL allows only SYSTEM, such as CrossDeskUnattended\host, which
    ; an administrator can neither list nor delete without these privileges.
    Push 2
    Call un.SetBackupPrivileges
    Pop $0
    ${If} $0 <> 1
        DetailPrint "$(CD_BackupPrivilegesUnavailable)"
    ${EndIf}

    ; The identity and trusted devices published for the unattended host, and
    ; staging folders an interrupted elevated update left behind.
    SetShellVarContext all
    StrCpy $R6 $APPDATA
    SetShellVarContext current
    StrCpy $R9 "$R6\CrossDeskUnattended"
    Call un.RemovePrivateMachineDirectory
    FindFirst $R5 $R4 "$R6\CrossDesk-update-*"
    ${DoWhile} $R4 != ""
        StrCpy $R9 "$R6\$R4"
        Call un.RemovePrivateMachineDirectory
        FindNext $R5 $R4
    ${Loop}
    FindClose $R5

    ; The service and the unattended host run as SYSTEM and keep their logs,
    ; cache and settings in its profile. The uninstaller is a 32-bit process:
    ; reach System32 itself, not SysWOW64.
    ${DisableX64FSRedirection}
    StrCpy $R9 "$SYSDIR\config\systemprofile\AppData\Local\${PRODUCT_NAME}"
    Call un.RemoveMachineDirectory
    StrCpy $R9 "$SYSDIR\config\systemprofile\AppData\Roaming\${PRODUCT_NAME}"
    Call un.RemoveMachineDirectory
    ${EnableX64FSRedirection}

    ; Disabling privileges the token holds does not fail.
    Push 0
    Call un.SetBackupPrivileges
    Pop $0

    ; When a user answers the Windows Defender Firewall prompt for
    ; CrossDesk.exe, Windows adds a TCP and a UDP inbound rule whose IDs start
    ; with "TCP Query User{" and "UDP Query User{". Remove only those for this
    ; copy's path; rules an administrator or a policy created stay. netsh
    ; cannot select rules by ID, so PowerShell's NetSecurity module does it.
    ; The path goes through the environment, so no quoting can break it.
    System::Call 'kernel32::SetEnvironmentVariableW(w "CROSSDESK_FIREWALL_PROGRAM", w "$INSTDIR\CrossDesk.exe")'
    ${DisableX64FSRedirection}
    nsExec::ExecToLog `"$SYSDIR\WindowsPowerShell\v1.0\powershell.exe" -NoProfile -NonInteractive -ExecutionPolicy Bypass -Command "$$ErrorActionPreference = 'Stop'; try { Get-NetFirewallRule -PolicyStore PersistentStore -Name 'TCP Query User{*', 'UDP Query User{*' | Where-Object { ($$_ | Get-NetFirewallApplicationFilter).Program -eq $$env:CROSSDESK_FIREWALL_PROGRAM } | Remove-NetFirewallRule } catch { exit 1 }"`
    Pop $0
    ${EnableX64FSRedirection}
    System::Call 'kernel32::SetEnvironmentVariableW(w "CROSSDESK_FIREWALL_PROGRAM", p 0)'
    ${If} $0 != "0"
        DetailPrint "$(CD_FirewallRulesNotRemoved)"
        StrCpy $UninstallIncomplete 1
    ${EndIf}
FunctionEnd

; Removes the folder in $R9 and reports it when anything of it is left.
Function un.RemoveMachineDirectory
    RMDir /r "$R9"
    System::Call 'kernel32::GetFileAttributesW(w R9) i .r0'
    ${If} $0 <> -1
        DetailPrint "$(CD_MachineDataNotRemoved) $R9"
        StrCpy $UninstallIncomplete 1
    ${EndIf}
FunctionEnd

; Removes the folder in $R9 only if it is a real directory, not a junction
; or symbolic link, owned by SYSTEM or Administrators, whose DACL only allows
; those two: the folders CrossDesk creates, and what PrivateDirectory in
; unattended_config.cpp and OwnedJobDirectory in update_platform.cpp accept.
; Standard users can create folders in ProgramData and RMDir /r follows
; junctions it finds inside, so the elevated uninstaller must not empty a
; folder anyone else could write to. Such a folder is not CrossDesk's and is
; kept.
Function un.RemovePrivateMachineDirectory
    System::Call 'kernel32::GetFileAttributesW(w R9) i .r0'
    ${If} $0 = -1
        Return
    ${EndIf}
    StrCpy $5 0
    ; FILE_ATTRIBUTE_DIRECTORY set, FILE_ATTRIBUTE_REPARSE_POINT clear.
    IntOp $1 $0 & 0x410
    ${If} $1 = 0x10
        ; SE_FILE_OBJECT, OWNER_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION.
        System::Call 'advapi32::GetNamedSecurityInfoW(w R9, i 1, i 5, *p .r1, p 0, *p .r2, p 0, *p .r3) i .r0'
        ${If} $0 = 0
            StrCpy $R8 $1
            Call un.IsTrustedSid
            ; A NULL DACL grants everyone full access.
            ${If} $R7 = 1
            ${AndIf} $2 <> 0
                StrCpy $5 1
                ; ACL header: AclRevision, Sbz1, AclSize, AceCount.
                System::Call '*$2(&i1 .r6, &i1 .r6, &i2 .r6, &i2 .r4)'
                StrCpy $6 0
                ${DoWhile} $6 < $4
                    System::Call 'advapi32::GetAce(p r2, i r6, *p .r7) i .r0'
                    ${If} $0 = 0
                        StrCpy $5 0
                        ${ExitDo}
                    ${EndIf}
                    ; ACE header: AceType, AceFlags, AceSize. The SID of an
                    ; ACCESS_ALLOWED_ACE follows the header and the 4-byte mask.
                    System::Call '*$7(&i1 .r0, &i1 .r1)'
                    ${If} $0 <> 0
                        StrCpy $5 0
                        ${ExitDo}
                    ${EndIf}
                    IntPtrOp $R8 $7 + 8
                    Call un.IsTrustedSid
                    ${If} $R7 <> 1
                        StrCpy $5 0
                        ${ExitDo}
                    ${EndIf}
                    IntOp $6 $6 + 1
                ${Loop}
            ${EndIf}
            System::Call 'kernel32::LocalFree(p r3)'
        ${EndIf}
    ${EndIf}
    ${If} $5 = 1
        Call un.RemoveMachineDirectory
    ${Else}
        DetailPrint "$(CD_MachineDataKept) $R9"
        StrCpy $UninstallIncomplete 1
    ${EndIf}
FunctionEnd

; Enables (2, SE_PRIVILEGE_ENABLED) or disables (0) SeBackupPrivilege and
; SeRestorePrivilege in the uninstaller's token, per the value on the stack,
; and replaces that value with 1 when both were set, else 0. Administrators
; hold both, disabled, unless a policy took them away. While they are
; enabled, FindFirstFile, DeleteFile and RemoveDirectory, which open with
; backup intent, ignore DACLs, so they stay enabled only while RMDir /r
; removes the folders that un.RemovePrivateMachineDirectory and the SYSTEM
; profile path vouch for.
Function un.SetBackupPrivileges
    Exch $0
    Push $1
    Push $2
    Push $3
    Push $4
    Push $5
    StrCpy $5 0
    ; TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY on the current process.
    System::Call 'advapi32::OpenProcessToken(p -1, i 0x28, *p .r1) i .r4'
    ${If} $4 <> 0
        ; TOKEN_PRIVILEGES: PrivilegeCount, then two LUID_AND_ATTRIBUTES of
        ; LUID (two DWORDs) and Attributes.
        System::Call '*(i 2, i 0, i 0, i r0, i 0, i 0, i r0) p .r2'
        IntPtrOp $3 $2 + 4
        System::Call 'advapi32::LookupPrivilegeValueW(p 0, w "SeBackupPrivilege", p r3) i .r4'
        ${If} $4 <> 0
            IntPtrOp $3 $2 + 16
            System::Call 'advapi32::LookupPrivilegeValueW(p 0, w "SeRestorePrivilege", p r3) i .r4'
            ${If} $4 <> 0
                ; It also succeeds when the token lacks a privilege, leaves
                ; that one unchanged and sets ERROR_NOT_ALL_ASSIGNED.
                System::Call 'advapi32::AdjustTokenPrivileges(p r1, i 0, p r2, i 0, p 0, p 0) i .r4 ?e'
                Pop $3
                ${If} $4 <> 0
                ${AndIf} $3 = 0
                    StrCpy $5 1
                ${EndIf}
            ${EndIf}
        ${EndIf}
        System::Free $2
        System::Call 'kernel32::CloseHandle(p r1)'
    ${EndIf}
    StrCpy $0 $5
    Pop $5
    Pop $4
    Pop $3
    Pop $2
    Pop $1
    Exch $0
FunctionEnd

; Sets $R7 to 1 when the SID at address $R8 is LocalSystem or the
; Administrators group, else to 0.
Function un.IsTrustedSid
    ; WinLocalSystemSid, WinBuiltinAdministratorsSid.
    System::Call 'advapi32::IsWellKnownSid(p R8, i 22) i .R7'
    ${If} $R7 = 0
        System::Call 'advapi32::IsWellKnownSid(p R8, i 26) i .R7'
    ${EndIf}
    ${If} $R7 <> 0
        StrCpy $R7 1
    ${EndIf}
FunctionEnd
