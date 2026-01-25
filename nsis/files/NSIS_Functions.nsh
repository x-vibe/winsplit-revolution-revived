Function .onVerifyInstDir
	StrCmp $InstallInProcess "1" verify_end
	IfFileExists $INSTDIR\WinSplit.exe path_is_not_good
	Goto verify_end
	path_is_not_good:
		Abort
	verify_end:
FunctionEnd

Function InstallModePageEnter
	StrCmp $PortableModeForced "1" 0 show_this_page
		StrCpy $InstallMode "2"
		Goto end_installmodepageenter
	show_this_page:
		!insertmacro MUI_HEADER_TEXT "$(InstModPage_Title)" "$(InstModPage_SubTitle)"
		# Disable the "Next" button until a mode has been selected
		StrCmp $InstallMode "0" 0 +3
		GetDlgItem $0 $HWNDPARENT 1
		EnableWindow $0 0
		StrCmp $InstallMode "1" 0 +2
		WriteIniStr $PageInstallModeIniFile 'Field 2' 'state' '1'
		StrCmp $InstallMode "2" 0 +2
		WriteIniStr $PageInstallModeIniFile 'Field 3' 'state' '1'
		WriteIniStr $PageInstallModeIniFile 'Field 1' 'Text' $(InstModPage_Text)
		WriteIniStr $PageInstallModeIniFile 'Field 2' 'Text' $(InstModPage_Mode1)
		WriteIniStr $PageInstallModeIniFile 'Field 3' 'Text' $(InstModPage_Mode2)
		InstallOptions::dialog "$PageInstallModeIniFile"
	end_installmodepageenter:
FunctionEnd

Function InstallModePageLeave
	ReadINIStr $1 $PageInstallModeIniFile "Field 2" "state"
	StrCmp $1 "1" 0 +3
		StrCpy $2 "1"
		Goto CheckNext
		StrCpy $2 "2"
CheckNext:
	StrCmp $2 $InstallMode +5
	StrCpy $InstallMode $2
	GetDlgItem $1 $HWNDPARENT 1
	EnableWindow $1 1
	Abort
FunctionEnd
/*
Function DecimalBugPageEnter
		!insertmacro MUI_HEADER_TEXT "$(DecimalBugPage_Title)" "$(DecimalBugPage_SubTitle)"
		WriteIniStr $DecimalBugIniFile 'Field 1' 'Text' $(DecimalBugPage_Text1)
		WriteIniStr $DecimalBugIniFile 'Field 2' 'Text' $(DecimalBugPage_Text2)
		WriteIniStr $DecimalBugIniFile 'Field 3' 'Text' $(DecimalBugPage_Text3)
		WriteIniStr $DecimalBugIniFile 'Field 4' 'Text' $(DecimalBugPage_Text4)
		InstallOptions::dialog "$DecimalBugIniFile"
FunctionEnd

Function DecimalBugPageLeave
	; Nothing to do: this is just an info page
FunctionEnd
*/
Function GetPathOnly
	Exch $0
	Push $1
	Push $2

	StrCpy $2 $0 1 -1
	StrCmp $2 '\' 0 +3
	StrCpy $0 $0 -1
	goto -3

	StrCpy $1 0
	IntOp $1 $1 - 1
	StrCpy $2 $0 1 $1
	StrCmp $2 '\' +2
	StrCmp $2 '' 0 -3
	StrCpy $0 $0 $1

	Pop $2
	Pop $1
	Exch $0
FunctionEnd

!macro CreateInternetShortcut FILENAME URL ICONFILE ICONINDEX
WriteINIStr "${FILENAME}.url" "InternetShortcut" "URL" "${URL}"
WriteINIStr "${FILENAME}.url" "InternetShortcut" "IconFile" "${ICONFILE}"
WriteINIStr "${FILENAME}.url" "InternetShortcut" "IconIndex" "${ICONINDEX}"
!macroend

Function RemoveVersion1_9
	;HideWindow
	;SetSilent silent
	Processes::FindProcess "winsplit.exe"
	StrCmp $R0 "1" process_founded1 process_done1
	process_founded1:
		Processes::KillProcess "winsplit.exe"
	process_done1:
		ReadRegStr $R0 HKLM "${UNINSTALL_REGKEY}" "UninstallString"
		StrCmp $R0 "" all_done1
		Push $R0`
		Call GetPathOnly
		Pop $R1
		SetOutPath $R1
		File "files\uninst.exe"
		SetOutPath $EXEDIR
		StrCpy $R1 "$R0 /S"
		ExecWait $R1
		# Remove the "autorun" registry key from version 1.9 if present
		ReadRegStr $R0 HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Run" "Winsplit"
		StrCmp $R0 "" all_done1
		DeleteRegValue HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Run" "Winsplit"
		# Indicate that this key needs to be recreated for the new version
		StrCpy $OldVersionAutoRun "1"
	all_done1:
FunctionEnd

Function RemoveVersion8_xx
	Processes::FindProcess "winsplit.exe"
	StrCmp $R0 "1" process_founded2 process_done2
	process_founded2:
		Processes::KillProcess "winsplit.exe"
	process_done2:
		ReadRegStr $R0 HKLM "${UNINSTALL_REGKEY}" "UninstallString"
		StrCmp $R0 "" all_done2
		SetOutPath $EXEDIR
		StrCpy $R1 "$R0 /S"
		ExecWait $R1
		ReadRegStr $R0 HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Run" "Winsplit"
		StrCmp $R0 "" all_done2
		DeleteRegValue HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Run" "Winsplit"
		# Indicate that this key needs to be recreated for the new version
		StrCpy $OldVersionAutoRun "1"
	all_done2:
FunctionEnd

Function FinishPageRunFunction
	# Function used to create the "auto-run" registry key
	WriteRegStr HKCU "SOFTWARE\Microsoft\Windows\CurrentVersion\Run" "Winsplit" "$INSTDIR\WinSplit.exe"
FunctionEnd

Function FinishPageShowReadmeFunction
	# Function used to launch WinSplit at the end of installation
	Exec "$INSTDIR\WinSplit.exe"
FunctionEnd

Function MyOnGuiInit
	Call DetectInstalledVersion
	Pop $R0
	StrCmp $R0 "ok" onInit_End
	StrCpy $PortableModeForced "1"
	onInit_End:
FunctionEnd

Function un.MyOnGuiInit
	Processes::FindProcess "winsplit.exe"
	StrCmp $R0 "1" 0 canContinue
		MessageBox MB_OKCANCEL $(KillProcessBeforeUninstall) /SD IDOK IDCANCEL 0 IDOK KillIt
		Abort
		Goto canContinue
	KillIt:
		Processes::KillProcess "winsplit.exe"
	canContinue:
FunctionEnd
