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
# Copyright 2006 Sun Microsystems, Inc.  All rights reserved.
# Use is subject to license terms.
#

#
#	This makefile drives the production of assym.h through genconst/stabs.
#
#	sun4u opl implementation architecture dependent
#

#
#	Path to the base of the uts directory tree (usually /usr/src/uts).
#
UTSBASE	= ../../..

ASSYM_H			= $(DSF_DIR)/$(OBJS_DIR)/assym.h
GENCONST		= $(DSF_DIR)/$(OBJS_DIR)/genconst

#
#	Include common rules.
#
include $(UTSBASE)/sun4u/opl/Makefile.opl

#
#	Define targets
#
ALL_TARGET	= $(ASSYM_H)

#
# This is DSF_DIR.  Use a short path.
#
DSF_DIR		= .

#
#	Overrides
#
CLEANFILES	= $(GENCONST) Nothing_to_remove
CLOBBERFILES	= $(ASSYM_H) $(CLEANFILES) Nothing_to_remove
ALL_BUILDS      = $(ALL_BUILDSONLY64)
DEF_BUILDS      = $(DEF_BUILDSONLY64)

#
#	Default build targets.
#
.KEEP_STATE:

all:		$(ALL_DEPS)

def:		$(DEF_DEPS)

clean:		$(CLEAN_DEPS)

clobber:	$(CLOBBER_DEPS)

install:	def

#
#	create assym.h
#
$(GENCONST):    $(GENCONST_SRC)
	$(NATIVECC) -w $(ALWAYS_DEFS) $(GENCONST_DEFS) $(NATIVE_INC_PATH) \
	    -o $(GENCONST) $(GENCONST_SRC)

$(ASSYM_H):	$(GENCONST) $(OFFSETS) $(PLATFORM_OFFSETS)
	$(OFFSETS_CREATE) <$(OFFSETS) >$@
	$(OFFSETS_CREATE) <$(PLATFORM_OFFSETS) >>$@
	$(GENCONST) >>$@

#
#	Include common targets.
#
include $(UTSBASE)/sun4u/opl/Makefile.targ
