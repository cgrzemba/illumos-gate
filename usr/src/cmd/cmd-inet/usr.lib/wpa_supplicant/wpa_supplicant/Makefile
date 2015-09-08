#
# This file and its contents are supplied under the terms of the
# Common Development and Distribution License ("CDDL"), version 1.0.
# You may only use this file in accordance with the terms of version
# 1.0 of the CDDL.
#
# A full copy of the text of the CDDL should have accompanied this
# source.  A copy of the CDDL is also available via the Internet at
# http://www.illumos.org/license/CDDL.
#

#
# Copyright (c) 2012, Enrico Papi <enricop@computer.org>. All rights reserved.
#

PROG =		wpa_supplicant
OBJS =		config.o notify.o bss.o eap_register.o ../src/utils/common.o \
../src/utils/wpa_debug.o ../src/utils/wpabuf.o ../src/utils/os_unix.o \
../src/utils/eloop.o config_solaris.o ../src/rsn_supp/wpa.o \
../src/rsn_supp/preauth.o ../src/rsn_supp/pmksa_cache.o \
../src/rsn_supp/peerkey.o ../src/rsn_supp/wpa_ie.o ../src/common/wpa_common.o \
../src/eap_peer/eap_tls.o ../src/eap_peer/eap_peap.o \
../src/eap_common/eap_peap_common.o ../src/eap_peer/eap_ttls.o \
../src/eap_peer/eap_md5.o ../src/eap_peer/eap_mschapv2.o ../src/eap_peer/eap_gtc.o \
../src/eap_peer/mschapv2.o ../src/eapol_supp/eapol_supp_sm.o \
../src/eap_peer/eap.o ../src/eap_peer/eap_methods.o ../src/crypto/ms_funcs.o \
../src/eap_common/chap.o ../src/eap_peer/eap_tls_common.o \
../src/crypto/tls_openssl.o ../src/crypto/crypto_openssl.o \
../src/crypto/aes-unwrap.o ../src/crypto/md5.o ../src/crypto/random.o \
ctrl_iface.o ctrl_iface_unix.o ../src/utils/base64.o \
../src/eap_common/eap_common.o ../src/crypto/sha1-prf.o \
../src/crypto/sha1-tlsprf.o ../src/drivers/driver_common.o wpa_supplicant.o \
events.o blacklist.o wpas_glue.o scan.o main.o ../src/drivers/driver_solaris.o \
../src/drivers/drivers.o ../src/l2_packet/l2_packet_solaris.o

SRCS = 		$(OBJS:%.o=%.c)

include	../../../../Makefile.cmd

MSGFILES=	$(OBJS)

CERRWARN +=	-erroff=E_USELESS_DECLARATION
CERRWARN +=	-erroff=E_ZERO_OR_NEGATIVE_SUBSCRIPT
CERRWARN +=	-erroff=E_EMPTY_MBR_DECLARATION
CERRWARN +=	-erroff=E_ZERO_SIZED_STRUCT_UNION
CERRWARN +=	-erroff=E_DUBIOUS_ESCAPE_CHAR

include ./Makefile.cflags

#Illumos-gate Cflags
C99MODE = $(C99_ENABLE)
CFLAGS += -D_WPASILLUMOS
CFLAGS += -_cc=-features=extensions

#Wpa_s CFlags
CFLAGS += $(WPAFLAGS) -I../src -I../src/utils

LDLIBS += 	 -ldladm -ldlpi -lnwam -lsocket

all install := LDLIBS += -lcrypto -lssl

$(ROOTLIBINETPROG) := FILEMODE = 0555

.KEEP_STATE:

all:		$(PROG)

%.o:		%.c
		@echo "CFLAGS: $(CFLAGS)"
		@echo "CTF_FLAGS: $(CTF_FLAGS)"
		@echo "COMPILE.c: $(COMPILE.c)"
		@echo
		$(COMPILE.c) -o $@ $<

$(PROG):	$(OBJS)
		$(LINK.c) -o $@ $(OBJS) $(LDLIBS)
		$(POST_PROCESS)

include ../../Makefile.lib

install:	all $(ROOTLIBINETPROG)

clean:
		$(RM) $(OBJS)

include ../../../../Makefile.targ
