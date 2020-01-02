@echo off
@rem 优先级 %%a > %var% >!var! > call %%var%%
setlocal enabledelayedexpansion

set /a GetStrLenRet=0
set /a FindStrRet=-1

set cppfilePath=D:\\Work\\ConsoleApp1\\ConsoleApp1\\MyStruct.cs

for /F "skip=1 delims=" %%i in (%cppfilePath%) do (
@rem echo %%i
call:FindFirstStr "%%i" struct
if !FindStrRet! NEQ -1 (
 set temp=%%i
 call set test=%%temp:~!FindStrRet!,-1%%
 echo !test!
)
if !existsEnumName! equ 1 echo %%i
if %%i == { goto ReadDone
)
:ReadDone
pause>nul
set strHead={ EGoods::YaoSH, "
set strTail=},
DIR *.bmp /b >tmp.data
for /F %%i in (tmp.data) do (
@rem echo %strHead%%%i"
set strResult=%%i
call:GetLength %%i
if !GetStrLenRet! LSS 20 (
for /l %%j in (!GetStrLenRet!,1,20) do (
@rem !strResult!空格键
set strResult=!strResult! 
)
)
set strResult=%strHead% !strResult! %strTail%
@rem echo !strResult!
)



echo Finnally.

pause>nul


:GetLength
SETLOCAL
set /a length=0
set str=%1
:GLStart
if defined str (
  set str=!str:~0,-1!
  set /a length+=1
  goto GLStart
)
(
ENDLOCAL
set /a GetStrLenRet=%length%
)
GOTO:EOF


:FindFirstStr
SETLOCAL
set srcStr=%1
set mchStr=%2
set /a mchPos=-1
call:GetLength %mchStr%
set /a mchLen=GetStrLenRet
set /a cur=0
@rem echo src str::%srcStr%
@rem echo find str:%mchStr%,length %mchLen%
:FFSStart
set tempStr=!srcStr:~%cur%,%mchLen%!
if defined tempStr (
 if !tempStr! neq %mchStr% (
   set /a cur+=1
   goto FFSStart
 ) else (
	set /a mchPos=%cur%
 )
)
(
  ENDLOCAL
  set /a FindStrRet=%mchPos%
)
GOTO:EOF









