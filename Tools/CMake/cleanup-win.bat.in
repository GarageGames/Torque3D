@ECHO off

:: Delete procedural shaders
echo shaders
del /q /a:-R shaders\procedural\*.*
:: Delete dumped shader disassembly files
for /R %%a IN (*._dis.txt) do IF EXIST "%%a._dis.txt" del "%%a._dis.txt"

:: Delete fonts
echo fonts
del /q /a:-R core\fonts\*.*

:: CEF cache
echo browser cache
del /q /a:-R cache


:: the cached meshes and alike
echo meshes and alike
for /R %%a IN (*.dae) do IF EXIST "%%~pna.cached.dts" del "%%~pna.cached.dts"
for /R %%a IN (*.imposter*.dds) do del "%%a"

:: the torque script compilations
echo compiled script
for /R %%a IN (*.cs) do IF EXIST "%%a.dso" del "%%a.dso"
for /R %%a IN (*.cs) do IF EXIST "%%a.edso" del "%%a.edso"
for /R %%a IN (*.gui) do IF EXIST "%%a.dso" del "%%a.dso"
for /R %%a IN (*.gui) do IF EXIST "%%a.edso" del "%%a.edso"
for /R %%a IN (*.ts) do IF EXIST "%%a.dso" del "%%a.dso"
for /R %%a IN (*.ts) do IF EXIST "%%a.edso" del "%%a.edso"

:: the user settings and alike
echo settings
IF EXIST "prefs.cs" del /s prefs.cs
IF EXIST "core\prefs.cs" del /s core\prefs.cs
::IF EXIST "scripts\client\prefs.cs" del /s scripts\client\prefs.cs
IF EXIST "scripts\server\banlist.cs" del /s scripts\server\banlist.cs
IF EXIST "scripts\server\prefs.cs" del /s scripts\server\prefs.cs
IF EXIST "client\config.cs" del /s client\config.cs
IF EXIST "config.cs" del /s config.cs
IF EXIST "tools\settings.xml" del /s tools\settings.xml
IF EXIST "banlist.cs" del /s banlist.cs

:: logs
echo logs
IF EXIST "torque3d.log" del /s torque3d.log
echo DONE!
