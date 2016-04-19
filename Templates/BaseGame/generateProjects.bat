@echo off
setlocal

SET TORQUEDIR=%2
IF NOT DEFINED TORQUEDIR SET TORQUEDIR=..\..

%TORQUEDIR%\engine\bin\php\php %TORQUEDIR%\Tools\projectGenerator\projectGenerator.php buildFiles/config/project.conf %TORQUEDIR%

endlocal

if X%1 == X (

   echo.
   echo REMEMBER: Restart VisualStudio if you are running it to be sure the new
   echo           project files are loaded! See docs for more info!
   echo.

   pause
)
