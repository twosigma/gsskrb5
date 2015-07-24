##########################################################################
#  $Id: Makefile,v 1.1 2001/01/24 18:42:55 d019080 Exp $
##########################################################################
#
# Copyright (c) 1997-2000  SAP AG.  All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer. 
#
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in
#    the documentation and/or other materials provided with the
#    distribution.
#
# 3. All advertising materials mentioning features or use of this
#    software must display the following acknowledgment:
#    "This product includes software developed by SAP AG"
#
# 4. The name "SAP AG" must not be used to endorse or promote products
#    derived from this software without prior written permission.
#    For written permission, please contact www.press@sap.com
#
# 5. Redistributions of any form whatsoever must retain the following
#    acknowledgment:
#    "This product includes software developed by SAP AG"
#
# THIS SOFTWARE IS PROVIDED BY SAP AG ''AS IS'' AND ANY EXPRESSED
# OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED. SAP AG SHALL BE LIABLE FOR ANY DAMAGES
# ARISING OUT OF THE USE OF THIS SOFTWARE ONLY IF CAUSED BY SAP AG'S
# INTENT OR GROSS NEGLIGENCE. IN CASE SAP AG IS LIABLE UNDER THIS
# AGREEMENT FOR DAMAGES CAUSED BY SAP AG´S GROSS NEGLIGENCE SAP AG
# FURTHER SHALL NOT BE LIABLE FOR ANY INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
# OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT, AND SHALL NOT BE LIABLE IN EXCESS OF THE AMOUNT OF
# DAMAGES TYPICALLY FORESEEABLE FOR SAP AG, WHICH SHALL IN NO EVENT
# EXCEED US$ 500.000.- 
#
##########################################################################

OBJ=.o
LIBE=.a
EXE=
CMDSEP=;
CC=cc
AR=ar rcs
RANLIB=ranlib
DEBUG=-g
DLLFLAGS=-fpic
CFLAGS=-ansi -Wall
INCDIR=-I..
MAKELIB=$(AR) $@
MAKELIB_END=$(RANLIB) $@
REMOVE=rm -f
MECH_LIB=   ../gsskrb5$(LIBE)

XDEFS = -I.

SRCS=	gss-misc.c	   gss-client.c 	gss-server.c	oid-func.c

COMMONOBJS=	gss-misc$(OBJ) 	oid-func$(OBJ)
CLIENTOBJS=	gss-client$(OBJ)
SERVEROBJS=	gss-server$(OBJ)

CLIOBJS=	$(CLIENTOBJS)	$(COMMONOBJS)
SRVOBJS=	$(SERVEROBJS)	$(COMMONOBJS)

OBJS=		$(COMMONOBJS)	$(CLIENTOBJS)	$(SERVEROBJS)

HDRS=	../gssapi_2.h	../platform.h		gss-misc.h	oid-func.h

CLITARGET = gss-client$(EXE)
SRVTARGET = gss-server$(EXE)

TARGETS = $(CLITARGET) $(SRVTARGET)

all: $(TARGETS)

$(OBJS):
	$(CC) $(CFLAGS) $(INCDIR) $(DEFINES) $(DEBUG) $(XDEFS) -c $*.c

$(CLITARGET):	$(CLIOBJS)
	@echo "-------------------------------------------------------------------"
	@echo "Linking OpenVision/MIT's '$@' with DLL '$(MECH_LIB)'"
	@echo "-------------------------------------------------------------------"
	LINK $(LDFLAGS) /OUT:$@ $(CLIOBJS) $(MECH_LIB) $(EXTRA_LIBS)
	$(POSTLINK_EXE)

$(SRVTARGET):	$(SRVOBJS)
	@echo "-------------------------------------------------------------------"
	@echo "Linking OpenVision/MIT's '$@' with DLL '$(MECH_LIB)'"
	@echo "-------------------------------------------------------------------"
	LINK $(LDFLAGS) /OUT:$@ $(SRVOBJS) $(MECH_LIB) $(EXTRA_LIBS)
	$(POSTLINK_EXE)

clean:
	-$(REMOVE) $(OBJS)
	-$(REMOVE) $(OBJS:.obj=.sbr)
	-$(REMOVE) $(TARGETS)
	-$(REMOVE) $(TARGETS:.exe=.pdb)
	-$(REMOVE) $(TARGETS:.exe=.ilk)
	-$(REMOVE) vc*.pdb
	-$(REMOVE) *.pch
        -$(REMOVE) *.manifest


## Dependencies:
oid-func$(OBJ):	  oid-func.c	$(HDRS)
gss-misc$(OBJ):	  gss-misc.c	$(HDRS)
gss-client$(OBJ): gss-client.c	$(HDRS)
gss-server$(OBJ): gss-server.c	$(HDRS)

