#
# CDDL HEADER START
#
# The contents of this file are subject to the terms of the
# Common Development and Distribution License (the "License").
# You may not use this file except in compliance with the License.
#
# You can obtain a copy of the license at usr/src/OPENSOLARIS.LICENSE
# or http://www.opensolaris.org/os/licensing.
# See the License for the specific language governing permissions
# and limitations under the License.
#
# When distributing Covered Code, include this CDDL HEADER in each
# file and include the License file at usr/src/OPENSOLARIS.LICENSE.
# If applicable, add the following below this CDDL HEADER, with the
# fields enclosed by brackets "[]" replaced with your own identifying
# information: Portions Copyright [yyyy] [name of copyright owner]
#
# CDDL HEADER END
#
# Copyright 2008 Sun Microsystems, Inc.  All rights reserved.
# Use is subject to license terms.
# Copyright (c) 2012, Enrico Papi <enricop@computer.org>. All rights reserved.
#

include	../../../Makefile.cmd

SUBDIR =	wpa_supplicant
MANIFEST =	wpa_supplicant.xml
SVCMETHOD=	svc-wpa_supplicant

ROOTMANIFESTDIR =	$(ROOTSVCNETWORK)

$(ROOTMANIFEST):= FILEMODE = 0444
$(ROOTSVCMETHOD):= FILEMODE = 0555

WPADEFS =	defs.h
WPACTRL =	wpa_ctrl.h
COMMONDIR =	src/common
WPAHDRDIR =	$(ROOT)/usr/include

all:=		TARGET= all
install:=	TARGET= install
clean:=		TARGET= clean
clobber:=	TARGET= clobber

include ../Makefile.lib

install_h:	$(WPADEFS) $(WPACTRL)

all lint clean clobber:		$(SUBDIR)

install:	install_h $(SUBDIR) $(ROOTMANIFEST) $(ROOTSVCMETHOD)

check:		$(CHKMANIFEST)

$(SUBDIR):	FRC
		@cd $@; pwd; $(MAKE) $(TARGET)

FRC:

$(WPADEFS):
	cd $(COMMONDIR); $(RM) $(WPAHDRDIR)/wpa_defs.h; \
	$(CP) $@ $(WPAHDRDIR)/wpa_defs.h; cd ../../

$(WPACTRL):
	cd $(COMMONDIR); $(RM) $(WPAHDRDIR)/$@; \
	$(CP) $@ $(WPAHDRDIR); cd ../../

include ../../../Makefile.targ
