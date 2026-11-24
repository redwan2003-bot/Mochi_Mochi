' ======================================================================
' Mochi Bot V1 - Silent Desktop Companion Launcher (No Terminal Window)
' ======================================================================
' Double-clicking this file starts the Mochi Desktop Companion in the 
' Windows System Tray completely silently with zero flashing CMD windows.

Set WshShell = CreateObject("WScript.Shell")
Set fso = CreateObject("Scripting.FileSystemObject")
strCurrentDir = fso.GetParentFolderName(WScript.ScriptFullName)

' Check if standalone executable exists first
If fso.FileExists(strCurrentDir & "\MochiCompanion.exe") Then
    WshShell.Run Chr(34) & strCurrentDir & "\MochiCompanion.exe" & Chr(34), 0, False
' Check for virtual environment pythonw
ElseIf fso.FileExists(strCurrentDir & "\.venv\Scripts\pythonw.exe") Then
    WshShell.Run Chr(34) & strCurrentDir & "\.venv\Scripts\pythonw.exe" & Chr(34) & " " & Chr(34) & strCurrentDir & "\tools\windows_companion_tray.py" & Chr(34), 0, False
' Fallback to Launch_Companion.bat hidden
Else
    WshShell.Run Chr(34) & strCurrentDir & "\Launch_Companion.bat" & Chr(34), 0, False
End If

Set WshShell = Nothing
Set fso = Nothing
