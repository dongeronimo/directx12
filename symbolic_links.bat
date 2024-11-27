@echo off
:: Save the current directory path
for /f "delims=" %%i in ('powershell -NoProfile -Command "(Get-Location).Path"') do set pwd=%%i

:: Define the target and link paths
set linkPathDebug=%pwd%\x64\debug\assets
set linkPathRelease=%pwd%\x64\release\assets
set targetPath=%pwd%\assets

:: Create the symbolic link for debug
powershell -NoProfile -Command "New-Item -ItemType SymbolicLink -Path '%linkPathDebug%' -Target '%targetPath%'"

:: Optional: Confirm success
if %errorlevel% equ 0 (
    echo Symbolic link created successfully.
) else (
    echo Failed to create symbolic link.
)

:: Define the target and link paths
set linkPathDebug=%pwd%\x64\debug\assets
set linkPathRelease=%pwd%\x64\release\assets
set targetPath=%pwd%\assets

:: Create the symbolic link for debug
powershell -NoProfile -Command "New-Item -ItemType SymbolicLink -Path '%linkPathRelease%' -Target '%targetPath%'"

:: Optional: Confirm success
if %errorlevel% equ 0 (
    echo Symbolic link created successfully.
) else (
    echo Failed to create symbolic link.
)