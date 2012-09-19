@echo off
call bison.bat CMD CMDgram.c CMDgram.y . CMDgram.cpp
..\..\bin\flex\flex -PCMD -oCMDscan.cpp CMDscan.l
