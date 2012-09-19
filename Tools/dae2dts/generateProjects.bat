@echo off

SET TORQUEDIR=%2

IF NOT DEFINED TORQUEDIR SET TORQUEDIR=..\..

@%TORQUEDIR%\engine\bin\php\php %TORQUEDIR%\Tools\projectGenerator\projectGenerator.php buildFiles/config/project.conf %TORQUEDIR%
@echo  ...
@echo  ...
@echo  ...
@echo REMEMBER: Restart Visual Studio if you are running it to be sure the new
@echo           project file(s) are loaded! See docs for more info!
@IF X%1 == X pause