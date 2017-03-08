REM Delete procedural shaders

del /q /a:-R game\shaders\procedural\*.*

REM Delete dumped shader disassembly files

del /q /s /a:-R *_dis.txt