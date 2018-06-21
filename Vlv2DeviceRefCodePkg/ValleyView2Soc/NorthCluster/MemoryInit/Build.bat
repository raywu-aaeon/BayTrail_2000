@echo off
SET PATH="C:\Program Files\Microsoft Visual Studio 9.0\VC\bin";%PATH%
CALL vcvars32.bat

nmake clean
nmake
