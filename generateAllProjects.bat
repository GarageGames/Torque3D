@echo off
setlocal

for /F "delims=^" %%a in (allProjects.txt) do if exist "%%a"\generateProjects.bat (

   setlocal

   echo.
   echo.
   echo === %%a =========
   echo.

   cd %%a
   call generateProjects.bat noPause

   endlocal
)

endlocal

echo.
pause