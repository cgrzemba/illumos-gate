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
#
# Copyright 2009 Sun Microsystems, Inc.  All rights reserved.
# Use is subject to license terms.
#

include ../../Makefile.cmd

SBINPROG =	print-service
BINPROG =	desktop-print-management-applet
LPBINLINKS =	lp lpstat cancel enable disable
LPRBINLINKS =	lpr lpq lprm lpc
DESKTOPLINKS =  desktop-print-management desktop-print-management-prefs
BINLINKS =	$(LPBINLINKS) $(LPRBINLINKS) $(DESKTOPLINKS)
SBINLINKS =	accept reject lpmove lpadmin

ROOTUSRSBINPROG =	$(SBINPROG:%=$(ROOTUSRSBIN)/%)
ROOTUSRBINPROG =	$(BINPROG:%=$(ROOTBIN)/%)
ROOTUSRBINLINKS =	$(BINLINKS:%=$(ROOTBIN)/%)
ROOTUSRSBINLINKS =	$(SBINLINKS:%=$(ROOTUSRSBIN)/%)

FILEMODE =	0555

$(ROOTUSRBINLINKS):
	$(RM) $@; $(SYMLINK) ../sbin/$(SBINPROG) $@

$(ROOTUSRSBINLINKS):
	$(RM) $@; $(SYMLINK) $(SBINPROG) $@

.KEEP_STATE:

all check clean clobber lint:

install:	$(ROOTUSRSBINPROG) $(ROOTUSRBINPROG) $(ROOTUSRBINLINKS) $(ROOTUSRSBINLINKS)

