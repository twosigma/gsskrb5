@echo off
set BASE=ntlm

rem **** removing -DNDEBUG will add tracing facilities to the code
rem **** (you also have to edit dbg_fn.c:dbg_init())
set DEFINES=-DNDEBUG

set PREFIX=gss
set CC=CL /nologo
set OBJ=.obj
set SHARED=.dll
set LIBE=.lib
set EXE=.exe
set RANLIB=@rem
rem *** NMAKE is silly -- if it gets one of the special macros $* $@
rem *** on the command line, it will issue a warning (it works though).
rem *** so I have to pass them with WIN_ environment variables instead
set WIN_AR=lib /NOLOGO /OUT:$@
set AR="$(WIN_AR)"
set DEBUG=
set CMDSEP="&&"
set REMOVE="del 2>NUL"
set MAKELIB=%AR%
set MAKELIB_END=@rem
set OPTSFILE=%TMP%\MKOPTS
set MKARGS=@%OPTSFILE%
set EXTRA_LIBS="kernel32.lib user32.lib advapi32.lib wsock32.lib"
set MAKELIB_SHARED_END=%EXTRA_LIBS%
set RCDEFS=
set XTRA_FLAGS=SSPI_INCLUDE=
set SHLD=LINK /nologo
set LINK_EXE=LINK /nologo /OUT:$@
set DLLFLAGS=/LD
set POSTLINK_EXE=
set POSTLINK_DLL=

if "%1" == "vc6" shift && goto :assume_vs98
if "%1" == "VC6" shift && goto :assume_vs98
if "%1" == "vc7" shift && goto :assume_vs2003
if "%1" == "VC7" shift && goto :assume_vs2003
if "%1" == "vc8" shift && goto :assume_vs2005
if "%1" == "VC8" shift && goto :assume_vs2005
if "%1" == "sdk2003" shift && goto :assume_win2003sdk

IF EXIST "%MSVCDIR%\..\VC98\BIN\CL.EXE" goto :assume_vs98
IF EXIST "%VCINSTALLDIR%\bin\mt.exe" goto :assume_vs2005
IF NOT "%CPU%." == "i386." goto :assume_win2003sdk
  goto assume_vs2003

:assume_vs98
  rem *** when no current PlatformSDK is available use XTRA_FLAGS
  set XTRA_FLAGS=SSPI_INCLUDE=-I../win32inc
  set BUILDENV_LABEL=VS98
  set RUNTIME_DEBUG_CHECKS=/GZ

  set DLLFLAGS=/GD /LD 
  set CFLAGS_OPT=/Og /Oy
  set CFLAGS_DBG=
  set CFLAGS_OPT_32=/YX /G6 /Zi
  set CFLAGS_DBG_32=/YX /G6 /Z7
  set PLATFORM_CFLAGS=
  set PLATFORM_LFLAGS=/DEBUG /PDBTYPE:CON /PDB:$*.pdb
  goto :common_MSC_definitions

:assume_vs2003
  set BUILDENV_LABEL=VS2003
  set RUNTIME_DEBUG_CHECKS=/GZ
  set PLATFORM_CFLAGS=/Zi
  set PLATFORM_LFLAGS=/DEBUG /PDB:$*.pdb
  set CFLAGS_OPT=/Og /Oy
  set CFLAGS_OPT_32=/YX /G6
  set CFLAGS_DBG_32=/YX /G6
  goto :common_MSC_definitions

:assume_win2003sdk
  set BUILDENV_LABEL=Win2003-SDK
  rem Microsoft totally fubar'ed /GZ in Win2003-SDK (aka /RTCs)
  set RUNTIME_DEBUG_CHECKS=
  set PLATFORM_CFLAGS=/Zi
  set PLATFORM_LFLAGS=/DEBUG /PDB:$*.pdb
  set CFLAGS_OPT=
  set CFLAGS_DBG=
  goto :common_MSC_definitions

:assume_vs2005
  set BUILDENV_LABEL=VS2005
  set RUNTIME_DEBUG_CHECKS=/RTCs
  set PLATFORM_CFLAGS=/Zi /D_CRT_SECURE_NO_WARNINGS=1 /wd4996
  set PLATFORM_LFLAGS=/MANIFEST /DEBUG /PDB:$*.pdb
  set CFLAGS_OPT=/Oy
  set CFLAGS_OPT_32=-D_USE_32BIT_TIME_T
  set CFLAGS_DBG_32=-D_USE_32BIT_TIME_T
  set POSTLINK_EXE=mt.exe -manifest "$(@).manifest" -outputresource:"$(@)";1
  set POSTLINK_DLL=mt.exe -manifest "$(@).manifest" -outputresource:"$(@)";2

:common_MSC_definitions
rem Microsoft Intrisics optimization (implied by /Ox) gives a pig-slow malloc
set CFLAGS_OPT=%CFLAGS_OPT% /Ob1 /Gs /Gy /Os
set CFLAGS_DBG=%CFLAGS_DBG% /Od /Zi /Yd %RUNTIME_DEBUG_CHECKS%
set LFLAGS_OPT=%LFLAGS_OPT%
set LFLAGS_DBG=%LFLAGS_DBG%

if %CPU%. == IA64.  goto :is_ia64
if %CPU%. == AMD64. goto :is_amd64
if %CPU%. == i386.  goto :is_i386
if %CPU%. == .      goto :is_i386

echo !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
echo !! ABORTING --                                !!
echo !! I don't know the "%CPU%" build environment !!
echo !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
goto :done

:is_i386
REM *************************************************
REM ** Intel x86 (traditional Win32)               **
REM *************************************************
REM --- Compiler/Linker flags specific to Opimization

set ARCH_LABEL=Win32 on ia32/Intel x86
set RCDEFS=-D_WIN32
set PREFIX=gss

set CFLAGS_OPT=%CFLAGS_OPT% %CFLAGS_OPT_32%
set LFLAGS_OPT=%LFLAGS_OPT% %LFLAGS_OPT_32%
REM --- Compiler/Linker flags specific to Debug
set CFLAGS_DBG=%CFLAGS_DBG%
set LFLAGS_DBG=%LFLAGS_DBG%

goto :make_it_so

:is_ia64
REM *************************************************
REM ** Intel IA64 (Win64)                          **
REM *************************************************
rem Microsoft Fubar: without /GS- you must link bufferoverflowu.lib

set ARCH_LABEL=Win64 on Intel ia64/Itanic
set RCDEFS=-D_WIN64 -D_M_IA64
set PREFIX=gi64

set PLATFORM_CFLAGS=%PLATFORM_CFLAGS% /Wp64 /GS-
REM --- Compiler/Linker flags specific to Opimization
set CFLAGS_OPT=%CFLAGS_OPT%
set LFLAGS_OPT=%LFLAGS_OPT%
REM --- Compiler/Linker flags specific to Debug
set CFLAGS_DBG=%CFLAGS_DBG%
set LFLAGS_DBG=%LFLAGS_DBG%

goto :make_it_so

:is_amd64
REM *************************************************
REM ** AMD64 (Win64)                               **
REM *************************************************
rem Microsoft Fubar: without /GS- you must link bufferoverflowu.lib

set ARCH_LABEL=Win64 on x86_64/AMD64/x64
set RCDEFS=-D_WIN64 -D_M_AMD64
set PREFIX=gx64

set PLATFORM_CFLAGS=%PLATFORM_CFLAGS% /Wp64 /GS-
REM --- Compiler/Linker flags specific to Opimization
set CFLAGS_OPT=%CFLAGS_OPT% /Oy
set LFLAGS_OPT=%LFLAGS_OPT%
REM --- Compiler/Linker flags specific to Debug
set CFLAGS_DBG=%CFLAGS_DBG%
set LFLAGS_DBG=%LFLAGS_DBG%

goto :make_it_so

:make_it_so
echo *******************************************************
echo ** Assumed build environment: %BUILDENV_LABEL%
echo **   Target Architecture/CPU: %ARCH_LABEL%
echo *******************************************************
REM --- default is optimized build

if "%1" == "dbg"   shift && goto :compile_dbg
if "%1" == "debug" shift && goto :compile_dbg
if "%1" == "opt"   shift && goto :compile_opt
goto :compile_opt

:compile_opt
set CFLAGS=%CFLAGS_OPT% %PLATFORM_CFLAGS%
set LFLAGS=%LFLAGS_OPT% %PLATFORM_LFLAGS%
set RCDEFS=%RCDEFS% -DNDEBUG

goto :finalize_flags
 
:compile_dbg
set CFLAGS=%CFLAGS_DBG% %PLATFORM_CFLAGS%
set LFLAGS=%LFLAGS_DBG% %PLATFORM_LFLAGS%

goto :finalize_flags

:finalize_flags
REM --- add generic Compiler and Linker flags
set CFLAGS=   %CFLAGS% /MD /EHs-c- /TC /GF /W3 /D _CONSOLE /D Seperate_Make
set LDFLAGS=  %LFLAGS% /OPT:REF /subsystem:console
set SHLDFLAGS=%LFLAGS% /OPT:REF /DLL /OUT:$@ /DEF:$*.def


if "%1" == "pdb" shift
if "%1" == "prerelease" shift && set RCDEFS=%RCDEFS% -DPRERELEASE_DLL


:finish_up

set DEFINES=-DWIN32 %DEFINES%
set WIN_MAKELIB_SHARED=%SHLD% /DLL %SHLDFLAGS% /OUT:$@ /DEF:$*.def
set WIN_COMPILE_EXE=%CC% %CFLAGS% %DEFINES% %/Fe$@
set RCDEFS=%RCDEFS% %DEFINES%

rem *********
rem add quotes so that they get passed through the Make-Options file correctly

set CC="%CC%"
set CFLAGS="%CFLAGS%"
set DEFINES="%DEFINES%"
set DLLFLAGS="%DLLFLAGS%"
set SHLD="%SHLD%"
set SHLDFLAGS="%SHLDFLAGS%"
set MAKELIB_SHARED="$(WIN_MAKELIB_SHARED)"
set COMPILE_EXE="$(WIN_COMPILE_EXE)"
set RCDEFS="%RCDEFS%"

echo CC=%CC% OBJ=%OBJ% SHARED=%SHARED% LIBE=%LIBE% EXE=%EXE% RANLIB=%RANLIB% AR=%AR% CFLAGS=%CFLAGS% DEBUG=%DEBUG% DEFINES=%DEFINES% REMOVE=%REMOVE% CMDSEP=%CMDSEP% MKARGS=%MKARGS% MAKELIB=%MAKELIB% MAKELIB_END=%MAKELIB_END% MAKELIB_SHARED=%MAKELIB_SHARED% MAKELIB_SHARED_END=%MAKELIB_SHARED_END% EXTRA_LIBS=%EXTRA_LIBS% COMPILE_EXE=%COMPILE_EXE% DLLFLAGS=%DLLFLAGS% BASE=%BASE% PREFIX=%PREFIX% RCDEFS=%RCDEFS% %XTRA_FLAGS% >%OPTSFILE%
nmake -f Makefile.%BASE% %MKARGS% %1 %2 %3
