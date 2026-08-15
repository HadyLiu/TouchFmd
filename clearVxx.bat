@echo off

::::::::::::::::::::::::::::::
:: 1)清理PIC生成的中间文件
:: 2)清理FMD生成的中间文件
:: 3)清理HC生成的中间文件
:: *使用方法* 
:: PIC、FMD拷入文件夹内，双击即可
:: HC使用需要将本文件夹拷入工程目录
::

::::::::::::::::::::::::::::::::::::::::::
::一键清理FMD产生的中间文件
rd /s /q output
del /f /s /q funclist
del /f /s /q *.p1
del /f /s /q *.pre


::EEPROM.bin不删除
for /f "delims=" %%i in ('dir /b /s /a-d "*.bin"') do (
    if "%%~nxi" neq "EEPROM.bin" (
        del /f /q "%%i"
    )
)

del /f /s /q *.as
del /f /s /q *.lst
del /f /s /q *.rlf
del /f /s /q *.as
del /f /s /q *.asm
del /f /s /q *.cof
del /f /s /q *.hxl
del /f /s /q *.map
del /f /s /q *.sdb
del /f /s /q *.sym
del /f /s /q *.obj
del /f /s /q *.d
del /f /s /q *.cmf


::::::::::::::::::::::::::::::::::::::::::
::一键清理PIC中间文件
rd /s /q build
rd /s /q debug

::只保留xml文件
cd ./nbproject
rd /s /q private
del /f /s /q *.mk
del /f /s /q *.properties
del /f /s /q *.bash

::只保留hex文件
cd ./dist/default/production
del *.d
del *.i
del *.p1
del *.xml
del *.cmf
del *.elf
del *.hxl
del *.lst
del *.map
del *.mum
del *.o
del *.rlf
del *.sdb
del *.sym

::::::::::::::::::::::::::::::::::::::::::
::一键清理HC产生的中间文件
::EEPROM.bin不删除
for /f "delims=" %%i in ('dir /b /s /a-d "*.bin"') do (
    if "%%~nxi" neq "EEPROM.bin" (
        del /f /q "%%i"
    )
)

del /f /s /q *.obj
del /f /s /q *.pre
del /f /s /q *.err
del /f /s /q *.as
del /f /s /q *.aslib
del /f /s /q log.txt


::::::::::::::::::::::::::::::::::::::::::
::一键清理HC产生的中间文件
::删除output文件夹
del /f /s /q *.ram