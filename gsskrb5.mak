# Microsoft Developer Studio Generated NMAKE File, Based on gsskrb5.dsp
!IF "$(CFG)" == ""
CFG=gsskrb5 - Win32 Debug
!MESSAGE No configuration specified. Defaulting to gsskrb5 - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "gsskrb5 - Win32 Release" && "$(CFG)" != "gsskrb5 - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
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
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "gsskrb5 - Win32 Release"

OUTDIR=.\gssapi32
INTDIR=.\gssapi32
# Begin Custom Macros
OutDir=.\gssapi32
# End Custom Macros

ALL : "$(OUTDIR)\gsskrb5.dll"


CLEAN :
	-@erase "$(INTDIR)\acpt_ctx.obj"
	-@erase "$(INTDIR)\acq_cred.obj"
	-@erase "$(INTDIR)\add_cred.obj"
	-@erase "$(INTDIR)\can_name.obj"
	-@erase "$(INTDIR)\cmp_name.obj"
	-@erase "$(INTDIR)\ctx_time.obj"
	-@erase "$(INTDIR)\dbg_fn.obj"
	-@erase "$(INTDIR)\del_ctx.obj"
	-@erase "$(INTDIR)\dsp_name.obj"
	-@erase "$(INTDIR)\dsp_stat.obj"
	-@erase "$(INTDIR)\dup_name.obj"
	-@erase "$(INTDIR)\exp_ctx.obj"
	-@erase "$(INTDIR)\exp_name.obj"
	-@erase "$(INTDIR)\get_mic.obj"
	-@erase "$(INTDIR)\gn_token.obj"
	-@erase "$(INTDIR)\gsskrb5.obj"
	-@erase "$(INTDIR)\gssmain.obj"
	-@erase "$(INTDIR)\gssmaini.obj"
	-@erase "$(INTDIR)\imp_ctx.obj"
	-@erase "$(INTDIR)\imp_name.obj"
	-@erase "$(INTDIR)\in_c_b_m.obj"
	-@erase "$(INTDIR)\in_n_f_m.obj"
	-@erase "$(INTDIR)\init_ctx.obj"
	-@erase "$(INTDIR)\inq_cred.obj"
	-@erase "$(INTDIR)\inq_ctx.obj"
	-@erase "$(INTDIR)\krb5cred.obj"
	-@erase "$(INTDIR)\krb5ctx.obj"
	-@erase "$(INTDIR)\krb5main.obj"
	-@erase "$(INTDIR)\krb5msg.obj"
	-@erase "$(INTDIR)\krb5name.obj"
	-@erase "$(INTDIR)\krb5oid.obj"
	-@erase "$(INTDIR)\ntlmcred.obj"
	-@erase "$(INTDIR)\ntlmctx.obj"
	-@erase "$(INTDIR)\ntlmmain.obj"
	-@erase "$(INTDIR)\ntlmmsg.obj"
	-@erase "$(INTDIR)\ntlmname.obj"
	-@erase "$(INTDIR)\ntlmoid.obj"
	-@erase "$(INTDIR)\oid_func.obj"
	-@erase "$(INTDIR)\proc_tok.obj"
	-@erase "$(INTDIR)\rel_buff.obj"
	-@erase "$(INTDIR)\rel_cred.obj"
	-@erase "$(INTDIR)\rel_name.obj"
	-@erase "$(INTDIR)\sy_funcs.obj"
	-@erase "$(INTDIR)\sy_shlib.obj"
	-@erase "$(INTDIR)\unwrap.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vrfy_mic.obj"
	-@erase "$(INTDIR)\wrap.obj"
	-@erase "$(INTDIR)\ws_limit.obj"
	-@erase "$(OUTDIR)\gsskrb5.dll"
	-@erase "$(OUTDIR)\gsskrb5.exp"
	-@erase "$(OUTDIR)\gsskrb5.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MT /W3 /GX /O2 /I "." /I "krb5" /I "ntlm" /I "win32inc" /D "WIN32" /D "NDEBUG" /Fp"$(INTDIR)\gsskrb5.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\gsskrb5.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib advapi32.lib wsock32.lib /nologo /dll /incremental:no /pdb:"$(OUTDIR)\gsskrb5.pdb" /machine:I386 /def:".\gsskrb5.def" /out:"$(OUTDIR)\gsskrb5.dll" /implib:"$(OUTDIR)\gsskrb5.lib" 
DEF_FILE= \
	".\gsskrb5.def"
LINK32_OBJS= \
	"$(INTDIR)\acpt_ctx.obj" \
	"$(INTDIR)\acq_cred.obj" \
	"$(INTDIR)\add_cred.obj" \
	"$(INTDIR)\can_name.obj" \
	"$(INTDIR)\cmp_name.obj" \
	"$(INTDIR)\ctx_time.obj" \
	"$(INTDIR)\dbg_fn.obj" \
	"$(INTDIR)\del_ctx.obj" \
	"$(INTDIR)\dsp_name.obj" \
	"$(INTDIR)\dsp_stat.obj" \
	"$(INTDIR)\dup_name.obj" \
	"$(INTDIR)\exp_ctx.obj" \
	"$(INTDIR)\exp_name.obj" \
	"$(INTDIR)\get_mic.obj" \
	"$(INTDIR)\gn_token.obj" \
	"$(INTDIR)\gssmain.obj" \
	"$(INTDIR)\gssmaini.obj" \
	"$(INTDIR)\imp_ctx.obj" \
	"$(INTDIR)\imp_name.obj" \
	"$(INTDIR)\in_c_b_m.obj" \
	"$(INTDIR)\in_n_f_m.obj" \
	"$(INTDIR)\init_ctx.obj" \
	"$(INTDIR)\inq_cred.obj" \
	"$(INTDIR)\inq_ctx.obj" \
	"$(INTDIR)\krb5cred.obj" \
	"$(INTDIR)\krb5ctx.obj" \
	"$(INTDIR)\krb5main.obj" \
	"$(INTDIR)\krb5msg.obj" \
	"$(INTDIR)\krb5name.obj" \
	"$(INTDIR)\krb5oid.obj" \
	"$(INTDIR)\ntlmcred.obj" \
	"$(INTDIR)\ntlmctx.obj" \
	"$(INTDIR)\ntlmmain.obj" \
	"$(INTDIR)\ntlmmsg.obj" \
	"$(INTDIR)\ntlmname.obj" \
	"$(INTDIR)\ntlmoid.obj" \
	"$(INTDIR)\oid_func.obj" \
	"$(INTDIR)\proc_tok.obj" \
	"$(INTDIR)\rel_buff.obj" \
	"$(INTDIR)\rel_cred.obj" \
	"$(INTDIR)\rel_name.obj" \
	"$(INTDIR)\sy_funcs.obj" \
	"$(INTDIR)\sy_shlib.obj" \
	"$(INTDIR)\unwrap.obj" \
	"$(INTDIR)\vrfy_mic.obj" \
	"$(INTDIR)\wrap.obj" \
	"$(INTDIR)\ws_limit.obj" \
	"$(INTDIR)\gsskrb5.obj"

"$(OUTDIR)\gsskrb5.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "gsskrb5 - Win32 Debug"

OUTDIR=.\gssapi32
INTDIR=.\gssapi32
# Begin Custom Macros
OutDir=.\gssapi32
# End Custom Macros

ALL : "$(OUTDIR)\gsskrb5.dll" "$(OUTDIR)\gsskrb5.bsc"


CLEAN :
	-@erase "$(INTDIR)\acpt_ctx.obj"
	-@erase "$(INTDIR)\acpt_ctx.sbr"
	-@erase "$(INTDIR)\acq_cred.obj"
	-@erase "$(INTDIR)\acq_cred.sbr"
	-@erase "$(INTDIR)\add_cred.obj"
	-@erase "$(INTDIR)\add_cred.sbr"
	-@erase "$(INTDIR)\can_name.obj"
	-@erase "$(INTDIR)\can_name.sbr"
	-@erase "$(INTDIR)\cmp_name.obj"
	-@erase "$(INTDIR)\cmp_name.sbr"
	-@erase "$(INTDIR)\ctx_time.obj"
	-@erase "$(INTDIR)\ctx_time.sbr"
	-@erase "$(INTDIR)\dbg_fn.obj"
	-@erase "$(INTDIR)\dbg_fn.sbr"
	-@erase "$(INTDIR)\del_ctx.obj"
	-@erase "$(INTDIR)\del_ctx.sbr"
	-@erase "$(INTDIR)\dsp_name.obj"
	-@erase "$(INTDIR)\dsp_name.sbr"
	-@erase "$(INTDIR)\dsp_stat.obj"
	-@erase "$(INTDIR)\dsp_stat.sbr"
	-@erase "$(INTDIR)\dup_name.obj"
	-@erase "$(INTDIR)\dup_name.sbr"
	-@erase "$(INTDIR)\exp_ctx.obj"
	-@erase "$(INTDIR)\exp_ctx.sbr"
	-@erase "$(INTDIR)\exp_name.obj"
	-@erase "$(INTDIR)\exp_name.sbr"
	-@erase "$(INTDIR)\get_mic.obj"
	-@erase "$(INTDIR)\get_mic.sbr"
	-@erase "$(INTDIR)\gn_token.obj"
	-@erase "$(INTDIR)\gn_token.sbr"
	-@erase "$(INTDIR)\gsskrb5.obj"
	-@erase "$(INTDIR)\gsskrb5.sbr"
	-@erase "$(INTDIR)\gssmain.obj"
	-@erase "$(INTDIR)\gssmain.sbr"
	-@erase "$(INTDIR)\gssmaini.obj"
	-@erase "$(INTDIR)\gssmaini.sbr"
	-@erase "$(INTDIR)\imp_ctx.obj"
	-@erase "$(INTDIR)\imp_ctx.sbr"
	-@erase "$(INTDIR)\imp_name.obj"
	-@erase "$(INTDIR)\imp_name.sbr"
	-@erase "$(INTDIR)\in_c_b_m.obj"
	-@erase "$(INTDIR)\in_c_b_m.sbr"
	-@erase "$(INTDIR)\in_n_f_m.obj"
	-@erase "$(INTDIR)\in_n_f_m.sbr"
	-@erase "$(INTDIR)\init_ctx.obj"
	-@erase "$(INTDIR)\init_ctx.sbr"
	-@erase "$(INTDIR)\inq_cred.obj"
	-@erase "$(INTDIR)\inq_cred.sbr"
	-@erase "$(INTDIR)\inq_ctx.obj"
	-@erase "$(INTDIR)\inq_ctx.sbr"
	-@erase "$(INTDIR)\krb5cred.obj"
	-@erase "$(INTDIR)\krb5cred.sbr"
	-@erase "$(INTDIR)\krb5ctx.obj"
	-@erase "$(INTDIR)\krb5ctx.sbr"
	-@erase "$(INTDIR)\krb5main.obj"
	-@erase "$(INTDIR)\krb5main.sbr"
	-@erase "$(INTDIR)\krb5msg.obj"
	-@erase "$(INTDIR)\krb5msg.sbr"
	-@erase "$(INTDIR)\krb5name.obj"
	-@erase "$(INTDIR)\krb5name.sbr"
	-@erase "$(INTDIR)\krb5oid.obj"
	-@erase "$(INTDIR)\krb5oid.sbr"
	-@erase "$(INTDIR)\ntlmcred.obj"
	-@erase "$(INTDIR)\ntlmcred.sbr"
	-@erase "$(INTDIR)\ntlmctx.obj"
	-@erase "$(INTDIR)\ntlmctx.sbr"
	-@erase "$(INTDIR)\ntlmmain.obj"
	-@erase "$(INTDIR)\ntlmmain.sbr"
	-@erase "$(INTDIR)\ntlmmsg.obj"
	-@erase "$(INTDIR)\ntlmmsg.sbr"
	-@erase "$(INTDIR)\ntlmname.obj"
	-@erase "$(INTDIR)\ntlmname.sbr"
	-@erase "$(INTDIR)\ntlmoid.obj"
	-@erase "$(INTDIR)\ntlmoid.sbr"
	-@erase "$(INTDIR)\oid_func.obj"
	-@erase "$(INTDIR)\oid_func.sbr"
	-@erase "$(INTDIR)\proc_tok.obj"
	-@erase "$(INTDIR)\proc_tok.sbr"
	-@erase "$(INTDIR)\rel_buff.obj"
	-@erase "$(INTDIR)\rel_buff.sbr"
	-@erase "$(INTDIR)\rel_cred.obj"
	-@erase "$(INTDIR)\rel_cred.sbr"
	-@erase "$(INTDIR)\rel_name.obj"
	-@erase "$(INTDIR)\rel_name.sbr"
	-@erase "$(INTDIR)\sy_funcs.obj"
	-@erase "$(INTDIR)\sy_funcs.sbr"
	-@erase "$(INTDIR)\sy_shlib.obj"
	-@erase "$(INTDIR)\sy_shlib.sbr"
	-@erase "$(INTDIR)\unwrap.obj"
	-@erase "$(INTDIR)\unwrap.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\vrfy_mic.obj"
	-@erase "$(INTDIR)\vrfy_mic.sbr"
	-@erase "$(INTDIR)\wrap.obj"
	-@erase "$(INTDIR)\wrap.sbr"
	-@erase "$(INTDIR)\ws_limit.obj"
	-@erase "$(INTDIR)\ws_limit.sbr"
	-@erase "$(OUTDIR)\gsskrb5.bsc"
	-@erase "$(OUTDIR)\gsskrb5.dll"
	-@erase "$(OUTDIR)\gsskrb5.exp"
	-@erase "$(OUTDIR)\gsskrb5.ilk"
	-@erase "$(OUTDIR)\gsskrb5.lib"
	-@erase "$(OUTDIR)\gsskrb5.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MTd /W3 /Gm /GX /ZI /Od /I "." /I "krb5" /I "ntlm" /I "win32inc" /D "WIN32" /D "NDEBUG" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\gsskrb5.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\gsskrb5.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\acpt_ctx.sbr" \
	"$(INTDIR)\acq_cred.sbr" \
	"$(INTDIR)\add_cred.sbr" \
	"$(INTDIR)\can_name.sbr" \
	"$(INTDIR)\cmp_name.sbr" \
	"$(INTDIR)\ctx_time.sbr" \
	"$(INTDIR)\dbg_fn.sbr" \
	"$(INTDIR)\del_ctx.sbr" \
	"$(INTDIR)\dsp_name.sbr" \
	"$(INTDIR)\dsp_stat.sbr" \
	"$(INTDIR)\dup_name.sbr" \
	"$(INTDIR)\exp_ctx.sbr" \
	"$(INTDIR)\exp_name.sbr" \
	"$(INTDIR)\get_mic.sbr" \
	"$(INTDIR)\gn_token.sbr" \
	"$(INTDIR)\gssmain.sbr" \
	"$(INTDIR)\gssmaini.sbr" \
	"$(INTDIR)\imp_ctx.sbr" \
	"$(INTDIR)\imp_name.sbr" \
	"$(INTDIR)\in_c_b_m.sbr" \
	"$(INTDIR)\in_n_f_m.sbr" \
	"$(INTDIR)\init_ctx.sbr" \
	"$(INTDIR)\inq_cred.sbr" \
	"$(INTDIR)\inq_ctx.sbr" \
	"$(INTDIR)\krb5cred.sbr" \
	"$(INTDIR)\krb5ctx.sbr" \
	"$(INTDIR)\krb5main.sbr" \
	"$(INTDIR)\krb5msg.sbr" \
	"$(INTDIR)\krb5name.sbr" \
	"$(INTDIR)\krb5oid.sbr" \
	"$(INTDIR)\ntlmcred.sbr" \
	"$(INTDIR)\ntlmctx.sbr" \
	"$(INTDIR)\ntlmmain.sbr" \
	"$(INTDIR)\ntlmmsg.sbr" \
	"$(INTDIR)\ntlmname.sbr" \
	"$(INTDIR)\ntlmoid.sbr" \
	"$(INTDIR)\oid_func.sbr" \
	"$(INTDIR)\proc_tok.sbr" \
	"$(INTDIR)\rel_buff.sbr" \
	"$(INTDIR)\rel_cred.sbr" \
	"$(INTDIR)\rel_name.sbr" \
	"$(INTDIR)\sy_funcs.sbr" \
	"$(INTDIR)\sy_shlib.sbr" \
	"$(INTDIR)\unwrap.sbr" \
	"$(INTDIR)\vrfy_mic.sbr" \
	"$(INTDIR)\wrap.sbr" \
	"$(INTDIR)\ws_limit.sbr" \
	"$(INTDIR)\gsskrb5.sbr"

"$(OUTDIR)\gsskrb5.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib advapi32.lib wsock32.lib /nologo /dll /incremental:yes /pdb:"$(OUTDIR)\gsskrb5.pdb" /debug /machine:I386 /def:".\gsskrb5.def" /out:"$(OUTDIR)\gsskrb5.dll" /implib:"$(OUTDIR)\gsskrb5.lib" /pdbtype:sept 
DEF_FILE= \
	".\gsskrb5.def"
LINK32_OBJS= \
	"$(INTDIR)\acpt_ctx.obj" \
	"$(INTDIR)\acq_cred.obj" \
	"$(INTDIR)\add_cred.obj" \
	"$(INTDIR)\can_name.obj" \
	"$(INTDIR)\cmp_name.obj" \
	"$(INTDIR)\ctx_time.obj" \
	"$(INTDIR)\dbg_fn.obj" \
	"$(INTDIR)\del_ctx.obj" \
	"$(INTDIR)\dsp_name.obj" \
	"$(INTDIR)\dsp_stat.obj" \
	"$(INTDIR)\dup_name.obj" \
	"$(INTDIR)\exp_ctx.obj" \
	"$(INTDIR)\exp_name.obj" \
	"$(INTDIR)\get_mic.obj" \
	"$(INTDIR)\gn_token.obj" \
	"$(INTDIR)\gssmain.obj" \
	"$(INTDIR)\gssmaini.obj" \
	"$(INTDIR)\imp_ctx.obj" \
	"$(INTDIR)\imp_name.obj" \
	"$(INTDIR)\in_c_b_m.obj" \
	"$(INTDIR)\in_n_f_m.obj" \
	"$(INTDIR)\init_ctx.obj" \
	"$(INTDIR)\inq_cred.obj" \
	"$(INTDIR)\inq_ctx.obj" \
	"$(INTDIR)\krb5cred.obj" \
	"$(INTDIR)\krb5ctx.obj" \
	"$(INTDIR)\krb5main.obj" \
	"$(INTDIR)\krb5msg.obj" \
	"$(INTDIR)\krb5name.obj" \
	"$(INTDIR)\krb5oid.obj" \
	"$(INTDIR)\ntlmcred.obj" \
	"$(INTDIR)\ntlmctx.obj" \
	"$(INTDIR)\ntlmmain.obj" \
	"$(INTDIR)\ntlmmsg.obj" \
	"$(INTDIR)\ntlmname.obj" \
	"$(INTDIR)\ntlmoid.obj" \
	"$(INTDIR)\oid_func.obj" \
	"$(INTDIR)\proc_tok.obj" \
	"$(INTDIR)\rel_buff.obj" \
	"$(INTDIR)\rel_cred.obj" \
	"$(INTDIR)\rel_name.obj" \
	"$(INTDIR)\sy_funcs.obj" \
	"$(INTDIR)\sy_shlib.obj" \
	"$(INTDIR)\unwrap.obj" \
	"$(INTDIR)\vrfy_mic.obj" \
	"$(INTDIR)\wrap.obj" \
	"$(INTDIR)\ws_limit.obj" \
	"$(INTDIR)\gsskrb5.obj"

"$(OUTDIR)\gsskrb5.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("gsskrb5.dep")
!INCLUDE "gsskrb5.dep"
!ELSE 
!MESSAGE Warning: cannot find "gsskrb5.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "gsskrb5 - Win32 Release" || "$(CFG)" == "gsskrb5 - Win32 Debug"
SOURCE=.\acpt_ctx.c

!IF  "$(CFG)" == "gsskrb5 - Win32 Release"


"$(INTDIR)\acpt_ctx.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "gsskrb5 - Win32 Debug"


"$(INTDIR)\acpt_ctx.obj"	"$(INTDIR)\acpt_ctx.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\acq_cred.c

!IF  "$(CFG)" == "gsskrb5 - Win32 Release"


"$(INTDIR)\acq_cred.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "gsskrb5 - Win32 Debug"


"$(INTDIR)\acq_cred.obj"	"$(INTDIR)\acq_cred.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\add_cred.c

!IF  "$(CFG)" == "gsskrb5 - Win32 Release"


"$(INTDIR)\add_cred.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "gsskrb5 - Win32 Debug"


"$(INTDIR)\add_cred.obj"	"$(INTDIR)\add_cred.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\can_name.c

!IF  "$(CFG)" == "gsskrb5 - Win32 Release"


"$(INTDIR)\can_name.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "gsskrb5 - Win32 Debug"


"$(INTDIR)\can_name.obj"	"$(INTDIR)\can_name.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\cmp_name.c

!IF  "$(CFG)" == "gsskrb5 - Win32 Release"


"$(INTDIR)\cmp_name.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "gsskrb5 - Win32 Debug"


"$(INTDIR)\cmp_name.obj"	"$(INTDIR)\cmp_name.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\ctx_time.c

!IF  "$(CFG)" == "gsskrb5 - Win32 Release"


"$(INTDIR)\ctx_time.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "gsskrb5 - Win32 Debug"


"$(INTDIR)\ctx_time.obj"	"$(INTDIR)\ctx_time.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\dbg_fn.c

!IF  "$(CFG)" == "gsskrb5 - Win32 Release"


"$(INTDIR)\dbg_fn.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "gsskrb5 - Win32 Debug"


"$(INTDIR)\dbg_fn.obj"	"$(INTDIR)\dbg_fn.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\del_ctx.c

!IF  "$(CFG)" == "gsskrb5 - Win32 Release"


"$(INTDIR)\del_ctx.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "gsskrb5 - Win32 Debug"


"$(INTDIR)\del_ctx.obj"	"$(INTDIR)\del_ctx.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\dsp_name.c

!IF  "$(CFG)" == "gsskrb5 - Win32 Release"


"$(INTDIR)\dsp_name.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "gsskrb5 - Win32 Debug"


"$(INTDIR)\dsp_name.obj"	"$(INTDIR)\dsp_name.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\dsp_stat.c

!IF  "$(CFG)" == "gsskrb5 - Win32 Release"


"$(INTDIR)\dsp_stat.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "gsskrb5 - Win32 Debug"


"$(INTDIR)\dsp_stat.obj"	"$(INTDIR)\dsp_stat.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\dup_name.c

!IF  "$(CFG)" == "gsskrb5 - Win32 Release"


"$(INTDIR)\dup_name.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "gsskrb5 - Win32 Debug"


"$(INTDIR)\dup_name.obj"	"$(INTDIR)\dup_name.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\exp_ctx.c

!IF  "$(CFG)" == "gsskrb5 - Win32 Release"


"$(INTDIR)\exp_ctx.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "gsskrb5 - Win32 Debug"


"$(INTDIR)\exp_ctx.obj"	"$(INTDIR)\exp_ctx.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\exp_name.c

!IF  "$(CFG)" == "gsskrb5 - Win32 Release"


"$(INTDIR)\exp_name.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "gsskrb5 - Win32 Debug"


"$(INTDIR)\exp_name.obj"	"$(INTDIR)\exp_name.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\get_mic.c

!IF  "$(CFG)" == "gsskrb5 - Win32 Release"


"$(INTDIR)\get_mic.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "gsskrb5 - Win32 Debug"


"$(INTDIR)\get_mic.obj"	"$(INTDIR)\get_mic.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\gn_token.c

!IF  "$(CFG)" == "gsskrb5 - Win32 Release"


"$(INTDIR)\gn_token.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "gsskrb5 - Win32 Debug"


"$(INTDIR)\gn_token.obj"	"$(INTDIR)\gn_token.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\gsskrb5.c

!IF  "$(CFG)" == "gsskrb5 - Win32 Release"


"$(INTDIR)\gsskrb5.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "gsskrb5 - Win32 Debug"


"$(INTDIR)\gsskrb5.obj"	"$(INTDIR)\gsskrb5.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\gssmain.c

!IF  "$(CFG)" == "gsskrb5 - Win32 Release"


"$(INTDIR)\gssmain.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "gsskrb5 - Win32 Debug"


"$(INTDIR)\gssmain.obj"	"$(INTDIR)\gssmain.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\gssmaini.c

!IF  "$(CFG)" == "gsskrb5 - Win32 Release"


"$(INTDIR)\gssmaini.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "gsskrb5 - Win32 Debug"


"$(INTDIR)\gssmaini.obj"	"$(INTDIR)\gssmaini.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\imp_ctx.c

!IF  "$(CFG)" == "gsskrb5 - Win32 Release"


"$(INTDIR)\imp_ctx.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "gsskrb5 - Win32 Debug"


"$(INTDIR)\imp_ctx.obj"	"$(INTDIR)\imp_ctx.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\imp_name.c

!IF  "$(CFG)" == "gsskrb5 - Win32 Release"


"$(INTDIR)\imp_name.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "gsskrb5 - Win32 Debug"


"$(INTDIR)\imp_name.obj"	"$(INTDIR)\imp_name.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\in_c_b_m.c

!IF  "$(CFG)" == "gsskrb5 - Win32 Release"


"$(INTDIR)\in_c_b_m.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "gsskrb5 - Win32 Debug"


"$(INTDIR)\in_c_b_m.obj"	"$(INTDIR)\in_c_b_m.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\in_n_f_m.c

!IF  "$(CFG)" == "gsskrb5 - Win32 Release"


"$(INTDIR)\in_n_f_m.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "gsskrb5 - Win32 Debug"


"$(INTDIR)\in_n_f_m.obj"	"$(INTDIR)\in_n_f_m.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\init_ctx.c

!IF  "$(CFG)" == "gsskrb5 - Win32 Release"


"$(INTDIR)\init_ctx.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "gsskrb5 - Win32 Debug"


"$(INTDIR)\init_ctx.obj"	"$(INTDIR)\init_ctx.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\inq_cred.c

!IF  "$(CFG)" == "gsskrb5 - Win32 Release"


"$(INTDIR)\inq_cred.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "gsskrb5 - Win32 Debug"


"$(INTDIR)\inq_cred.obj"	"$(INTDIR)\inq_cred.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\inq_ctx.c

!IF  "$(CFG)" == "gsskrb5 - Win32 Release"


"$(INTDIR)\inq_ctx.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "gsskrb5 - Win32 Debug"


"$(INTDIR)\inq_ctx.obj"	"$(INTDIR)\inq_ctx.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\krb5\krb5cred.c

!IF  "$(CFG)" == "gsskrb5 - Win32 Release"


"$(INTDIR)\krb5cred.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "gsskrb5 - Win32 Debug"


"$(INTDIR)\krb5cred.obj"	"$(INTDIR)\krb5cred.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\krb5\krb5ctx.c

!IF  "$(CFG)" == "gsskrb5 - Win32 Release"


"$(INTDIR)\krb5ctx.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "gsskrb5 - Win32 Debug"


"$(INTDIR)\krb5ctx.obj"	"$(INTDIR)\krb5ctx.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\krb5\krb5main.c

!IF  "$(CFG)" == "gsskrb5 - Win32 Release"


"$(INTDIR)\krb5main.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "gsskrb5 - Win32 Debug"


"$(INTDIR)\krb5main.obj"	"$(INTDIR)\krb5main.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\krb5\krb5msg.c

!IF  "$(CFG)" == "gsskrb5 - Win32 Release"


"$(INTDIR)\krb5msg.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "gsskrb5 - Win32 Debug"


"$(INTDIR)\krb5msg.obj"	"$(INTDIR)\krb5msg.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\krb5\krb5name.c

!IF  "$(CFG)" == "gsskrb5 - Win32 Release"


"$(INTDIR)\krb5name.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "gsskrb5 - Win32 Debug"


"$(INTDIR)\krb5name.obj"	"$(INTDIR)\krb5name.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\krb5\krb5oid.c

!IF  "$(CFG)" == "gsskrb5 - Win32 Release"


"$(INTDIR)\krb5oid.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "gsskrb5 - Win32 Debug"


"$(INTDIR)\krb5oid.obj"	"$(INTDIR)\krb5oid.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ntlm\ntlmcred.c

!IF  "$(CFG)" == "gsskrb5 - Win32 Release"


"$(INTDIR)\ntlmcred.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "gsskrb5 - Win32 Debug"


"$(INTDIR)\ntlmcred.obj"	"$(INTDIR)\ntlmcred.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ntlm\ntlmctx.c

!IF  "$(CFG)" == "gsskrb5 - Win32 Release"


"$(INTDIR)\ntlmctx.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "gsskrb5 - Win32 Debug"


"$(INTDIR)\ntlmctx.obj"	"$(INTDIR)\ntlmctx.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ntlm\ntlmmain.c

!IF  "$(CFG)" == "gsskrb5 - Win32 Release"


"$(INTDIR)\ntlmmain.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "gsskrb5 - Win32 Debug"


"$(INTDIR)\ntlmmain.obj"	"$(INTDIR)\ntlmmain.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ntlm\ntlmmsg.c

!IF  "$(CFG)" == "gsskrb5 - Win32 Release"


"$(INTDIR)\ntlmmsg.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "gsskrb5 - Win32 Debug"


"$(INTDIR)\ntlmmsg.obj"	"$(INTDIR)\ntlmmsg.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ntlm\ntlmname.c

!IF  "$(CFG)" == "gsskrb5 - Win32 Release"


"$(INTDIR)\ntlmname.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "gsskrb5 - Win32 Debug"


"$(INTDIR)\ntlmname.obj"	"$(INTDIR)\ntlmname.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ntlm\ntlmoid.c

!IF  "$(CFG)" == "gsskrb5 - Win32 Release"


"$(INTDIR)\ntlmoid.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "gsskrb5 - Win32 Debug"


"$(INTDIR)\ntlmoid.obj"	"$(INTDIR)\ntlmoid.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\oid_func.c

!IF  "$(CFG)" == "gsskrb5 - Win32 Release"


"$(INTDIR)\oid_func.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "gsskrb5 - Win32 Debug"


"$(INTDIR)\oid_func.obj"	"$(INTDIR)\oid_func.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\proc_tok.c

!IF  "$(CFG)" == "gsskrb5 - Win32 Release"


"$(INTDIR)\proc_tok.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "gsskrb5 - Win32 Debug"


"$(INTDIR)\proc_tok.obj"	"$(INTDIR)\proc_tok.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\rel_buff.c

!IF  "$(CFG)" == "gsskrb5 - Win32 Release"


"$(INTDIR)\rel_buff.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "gsskrb5 - Win32 Debug"


"$(INTDIR)\rel_buff.obj"	"$(INTDIR)\rel_buff.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\rel_cred.c

!IF  "$(CFG)" == "gsskrb5 - Win32 Release"


"$(INTDIR)\rel_cred.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "gsskrb5 - Win32 Debug"


"$(INTDIR)\rel_cred.obj"	"$(INTDIR)\rel_cred.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\rel_name.c

!IF  "$(CFG)" == "gsskrb5 - Win32 Release"


"$(INTDIR)\rel_name.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "gsskrb5 - Win32 Debug"


"$(INTDIR)\rel_name.obj"	"$(INTDIR)\rel_name.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\sy_funcs.c

!IF  "$(CFG)" == "gsskrb5 - Win32 Release"


"$(INTDIR)\sy_funcs.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "gsskrb5 - Win32 Debug"


"$(INTDIR)\sy_funcs.obj"	"$(INTDIR)\sy_funcs.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\sy_shlib.c

!IF  "$(CFG)" == "gsskrb5 - Win32 Release"


"$(INTDIR)\sy_shlib.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "gsskrb5 - Win32 Debug"


"$(INTDIR)\sy_shlib.obj"	"$(INTDIR)\sy_shlib.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\unwrap.c

!IF  "$(CFG)" == "gsskrb5 - Win32 Release"


"$(INTDIR)\unwrap.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "gsskrb5 - Win32 Debug"


"$(INTDIR)\unwrap.obj"	"$(INTDIR)\unwrap.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\vrfy_mic.c

!IF  "$(CFG)" == "gsskrb5 - Win32 Release"


"$(INTDIR)\vrfy_mic.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "gsskrb5 - Win32 Debug"


"$(INTDIR)\vrfy_mic.obj"	"$(INTDIR)\vrfy_mic.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\wrap.c

!IF  "$(CFG)" == "gsskrb5 - Win32 Release"


"$(INTDIR)\wrap.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "gsskrb5 - Win32 Debug"


"$(INTDIR)\wrap.obj"	"$(INTDIR)\wrap.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\ws_limit.c

!IF  "$(CFG)" == "gsskrb5 - Win32 Release"


"$(INTDIR)\ws_limit.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "gsskrb5 - Win32 Debug"


"$(INTDIR)\ws_limit.obj"	"$(INTDIR)\ws_limit.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 


!ENDIF 

