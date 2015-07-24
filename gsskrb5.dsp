# Microsoft Developer Studio Project File - Name="gsskrb5" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=gsskrb5 - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "gsskrb5.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "gsskrb5.mak" CFG="gsskrb5 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "gsskrb5 - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "gsskrb5 - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "gsskrb5"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "gsskrb5 - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "gssapi32"
# PROP Intermediate_Dir "gssapi32"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "GSSKRB5_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "." /I "krb5" /I "ntlm" /I "win32inc" /D "WIN32" /D "NDEBUG" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib advapi32.lib wsock32.lib /nologo /dll /machine:I386

!ELSEIF  "$(CFG)" == "gsskrb5 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "gssapi32"
# PROP Intermediate_Dir "gssapi32"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "GSSKRB5_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "." /I "krb5" /I "ntlm" /I "win32inc" /D "WIN32" /D "NDEBUG" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib advapi32.lib wsock32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "gsskrb5 - Win32 Release"
# Name "gsskrb5 - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\acpt_ctx.c
# End Source File
# Begin Source File

SOURCE=.\acq_cred.c
# End Source File
# Begin Source File

SOURCE=.\add_cred.c
# End Source File
# Begin Source File

SOURCE=.\can_name.c
# End Source File
# Begin Source File

SOURCE=.\cmp_name.c
# End Source File
# Begin Source File

SOURCE=.\ctx_time.c
# End Source File
# Begin Source File

SOURCE=.\dbg_fn.c
# End Source File
# Begin Source File

SOURCE=.\del_ctx.c
# End Source File
# Begin Source File

SOURCE=.\dsp_name.c
# End Source File
# Begin Source File

SOURCE=.\dsp_stat.c
# End Source File
# Begin Source File

SOURCE=.\dup_name.c
# End Source File
# Begin Source File

SOURCE=.\exp_ctx.c
# End Source File
# Begin Source File

SOURCE=.\exp_name.c
# End Source File
# Begin Source File

SOURCE=.\get_mic.c
# End Source File
# Begin Source File

SOURCE=.\gn_token.c
# End Source File
# Begin Source File

SOURCE=.\gsskrb5.c
# End Source File
# Begin Source File

SOURCE=.\gsskrb5.def
# End Source File
# Begin Source File

SOURCE=.\gssmain.c
# End Source File
# Begin Source File

SOURCE=.\gssmaini.c
# End Source File
# Begin Source File

SOURCE=.\imp_ctx.c
# End Source File
# Begin Source File

SOURCE=.\imp_name.c
# End Source File
# Begin Source File

SOURCE=.\in_c_b_m.c
# End Source File
# Begin Source File

SOURCE=.\in_n_f_m.c
# End Source File
# Begin Source File

SOURCE=.\init_ctx.c
# End Source File
# Begin Source File

SOURCE=.\inq_cred.c
# End Source File
# Begin Source File

SOURCE=.\inq_ctx.c
# End Source File
# Begin Source File

SOURCE=.\krb5\krb5cred.c
# End Source File
# Begin Source File

SOURCE=.\krb5\krb5ctx.c
# End Source File
# Begin Source File

SOURCE=.\krb5\krb5main.c
# End Source File
# Begin Source File

SOURCE=.\krb5\krb5msg.c
# End Source File
# Begin Source File

SOURCE=.\krb5\krb5name.c
# End Source File
# Begin Source File

SOURCE=.\krb5\krb5oid.c
# End Source File
# Begin Source File

SOURCE=.\ntlm\ntlmcred.c
# End Source File
# Begin Source File

SOURCE=.\ntlm\ntlmctx.c
# End Source File
# Begin Source File

SOURCE=.\ntlm\ntlmmain.c
# End Source File
# Begin Source File

SOURCE=.\ntlm\ntlmmsg.c
# End Source File
# Begin Source File

SOURCE=.\ntlm\ntlmname.c
# End Source File
# Begin Source File

SOURCE=.\ntlm\ntlmoid.c
# End Source File
# Begin Source File

SOURCE=.\oid_func.c
# End Source File
# Begin Source File

SOURCE=.\proc_tok.c
# End Source File
# Begin Source File

SOURCE=.\rel_buff.c
# End Source File
# Begin Source File

SOURCE=.\rel_cred.c
# End Source File
# Begin Source File

SOURCE=.\rel_name.c
# End Source File
# Begin Source File

SOURCE=.\sy_funcs.c
# End Source File
# Begin Source File

SOURCE=.\sy_shlib.c
# End Source File
# Begin Source File

SOURCE=.\unwrap.c
# End Source File
# Begin Source File

SOURCE=.\vrfy_mic.c
# End Source File
# Begin Source File

SOURCE=.\wrap.c
# End Source File
# Begin Source File

SOURCE=.\ws_limit.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\win32inc\BASETSD.H
# End Source File
# Begin Source File

SOURCE=.\dbg_defs.h
# End Source File
# Begin Source File

SOURCE=.\gn_spi.h
# End Source File
# Begin Source File

SOURCE=.\gss_argm.h
# End Source File
# Begin Source File

SOURCE=.\gssapi_2.h
# End Source File
# Begin Source File

SOURCE=.\gssmaini.h
# End Source File
# Begin Source File

SOURCE=.\win32inc\ISSPERR.H
# End Source File
# Begin Source File

SOURCE=.\krb5\krb5defs.h
# End Source File
# Begin Source File

SOURCE=.\mysccs.h
# End Source File
# Begin Source File

SOURCE=.\ntlm\ntlmdefs.h
# End Source File
# Begin Source File

SOURCE=.\platform.h
# End Source File
# Begin Source File

SOURCE=.\win32inc\SecExt.h
# End Source File
# Begin Source File

SOURCE=.\win32inc\security.h
# End Source File
# Begin Source File

SOURCE=.\win32inc\SSPI.H
# End Source File
# Begin Source File

SOURCE=.\win32inc\sspi_p1.h
# End Source File
# Begin Source File

SOURCE=.\sy_defs.h
# End Source File
# Begin Source File

SOURCE=.\test_api.h
# End Source File
# Begin Source File

SOURCE=.\version.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
