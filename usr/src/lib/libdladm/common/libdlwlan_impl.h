/*
 * CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License (the "License").
 * You may not use this file except in compliance with the License.
 *
 * You can obtain a copy of the license at usr/src/OPENSOLARIS.LICENSE
 * or http://www.opensolaris.org/os/licensing.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the License file at usr/src/OPENSOLARIS.LICENSE.
 * If applicable, add the following below this CDDL HEADER, with the
 * fields enclosed by brackets "[]" replaced with your own identifying
 * information: Portions Copyright [yyyy] [name of copyright owner]
 *
 * CDDL HEADER END
 */
/*
 * Copyright 2008 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 * Copyright (c) 2012, Enrico Papi <enricop@computer.org>. All rights reserved.
 */

#ifndef _LIBDLWLAN_IMPL_H
#define	_LIBDLWLAN_IMPL_H

#include <inet/wifi_ioctl.h>
#include <sys/mac.h>

/*
 * Implementation-private data structures, macros, and constants.
 */

#ifdef	__cplusplus
extern "C" {
#endif

/*
 * These settings are not supported by current device drivers:
 *
 * powermode: Specifies the power management mode of the WiFi link.
 * Possible values are:
 * 	off (disable power management),
 * 	max (maximum power savings), and
 * 	fast (performance sensitive power management).
 * Default is off.
 *
 * radio: Specifies whether the radio is on or off;
 * default is on.
 */

typedef enum {
	DLADM_WLAN_RADIO_ON = 1,
	DLADM_WLAN_RADIO_OFF
} dladm_wlan_radio_t;

typedef enum {
	DLADM_WLAN_PM_OFF = 1,
	DLADM_WLAN_PM_MAX,
	DLADM_WLAN_PM_FAST
} dladm_wlan_powermode_t;

extern dladm_status_t	i_dladm_wlan_param(dladm_handle_t, datalink_id_t,
			    void *, mac_prop_id_t, size_t, boolean_t);
extern boolean_t	i_dladm_wlan_convert_chan(wl_phy_conf_t *, uint16_t *);

#ifdef	__cplusplus
}
#endif

#endif	/* _LIBDLWLAN_IMPL_H */
