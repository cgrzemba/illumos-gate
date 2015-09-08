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
 * Copyright (c) 2010, Oracle and/or its affiliates. All rights reserved.
 * Copyright (c) 2012, Enrico Papi <enricop@computer.org>. All rights reserved.
 */

#include <string.h>
#include <libdlwlan.h>
#include <syslog.h>

#include "events.h"
#include "known_wlans.h"
#include "ncu.h"
#include "objects.h"
#include "util.h"

int
find_matching_wlan_cb(nwam_known_wlan_handle_t kwh, void *data)
{
	nwam_wlan_t *mywlan = data;
	nwam_error_t err;

	nwam_value_t bssidval;
	char *bssidstr;
	nwam_value_t ssidval;
	char *ssidstr;
	nwam_value_t keynameval;
	char *keynamestr;

	char *currnamestr;
	int currid = -1;

	if (mywlan == NULL)
		return (-1);

	/*
	 * We return NWAM_SUCCESS if nothing is found or errors are encountered
	 * to not halt known wlans walk
	 */

	/*
	 * BSSID. Skip this check if the user added manually a known wlan
	 * without bssid field
	 */
	if ((err = nwam_known_wlan_get_prop_value(kwh,
	    NWAM_KNOWN_WLAN_PROP_BSSID, &bssidval)) == NWAM_SUCCESS) {
		uint8_t bssid[6];

		err = nwam_value_get_string(bssidval, &bssidstr);
		if (err != NWAM_SUCCESS) {
			nwam_value_free(bssidval);
			return (NWAM_SUCCESS);
		}
		if (dladm_wlan_str2bssid(bssidstr, bssid)) {
			nwam_value_free(bssidval);
			return (NWAM_SUCCESS);
		}
		if (memcmp(bssid, mywlan->nww_bssid,
		    DLADM_WLAN_BSSID_LEN) != 0) {
			nwam_value_free(bssidval);
			return (NWAM_SUCCESS);
		}
		nwam_value_free(bssidval);
	}

	/* ssid */
	if ((err = nwam_known_wlan_get_prop_value(kwh,
	    NWAM_KNOWN_WLAN_PROP_SSID, &ssidval)) != NWAM_SUCCESS)
		return (NWAM_SUCCESS);

	err = nwam_value_get_string(ssidval, &ssidstr);
	if (err != NWAM_SUCCESS) {
		nwam_value_free(ssidval);
		return (NWAM_SUCCESS);
	}

	if (memcmp(ssidstr, mywlan->nww_essid, mywlan->nww_esslen) != 0) {
		nwam_value_free(ssidval);
		return (NWAM_SUCCESS);
	}
	nwam_value_free(ssidval);

	/* secmode <> secobjclass */
	err = nwam_known_wlan_get_prop_value(kwh, NWAM_KNOWN_WLAN_PROP_KEYNAME,
	    &keynameval);
	if (mywlan->nww_security_mode == DLADM_WLAN_SECMODE_NONE &&
	    err != NWAM_SUCCESS) {
		goto valid;
	} else if (err != NWAM_SUCCESS) {
		return (NWAM_SUCCESS);
	}
	err = nwam_value_get_string(keynameval, &keynamestr);
	if (err != NWAM_SUCCESS) {
		nwam_value_free(keynameval);
		return (NWAM_SUCCESS);
	}

	{
		secobj_class_info_t key_if;

		key_if.sc_name = keynamestr;
		key_if.sc_dladmclass = 0;
		if (dladm_walk_secobj(dld_handle, &key_if, find_matching_secobj,
		    DLADM_OPT_PERSIST)) {
			nwam_value_free(keynameval);
			return (NWAM_SUCCESS);
		}

		if (key_if.sc_dladmclass == 0) {
			nwam_value_free(keynameval);
			return (NWAM_SUCCESS);
		}

		if (!((key_if.sc_dladmclass == DLADM_SECOBJ_CLASS_PSK &&
		    mywlan->nww_security_mode == DLADM_WLAN_SECMODE_PSK) ||
		    (key_if.sc_dladmclass == DLADM_SECOBJ_CLASS_WEP &&
		    mywlan->nww_security_mode == DLADM_WLAN_SECMODE_WEP) ||
		    (key_if.sc_dladmclass >= DLADM_SECOBJ_CLASS_TLS &&
		    mywlan->nww_security_mode == DLADM_WLAN_SECMODE_EAP))) {
			nwam_value_free(keynameval);
			return (NWAM_SUCCESS);
		}
		nwam_value_free(keynameval);
	}

valid:
	if ((err = nwam_known_wlan_get_name(kwh, &currnamestr)) !=
	    NWAM_SUCCESS)
		return (NWAM_SUCCESS);

	currid = atoi(currnamestr);
	free(currnamestr);

	if (currid <= 0 || currid >= INT_MAX)
		return (NWAM_SUCCESS);

	mywlan->nww_wlanid = currid;

	/* found known wlan, halt walk */
	return (currid);
}

/* ARGSUSED */
static int
nwamd_ncu_known_wlan_committed(nwamd_object_t object, void *data)
{
	nwamd_ncu_t *ncu_data = object->nwamd_object_data;
	dladm_status_t status;
	struct wpa_ctrl *ctrl_conn = NULL;
	char *reconf[] = {"RECONFIGURE"};

	if (ncu_data->ncu_type != NWAM_NCU_TYPE_LINK ||
	    ncu_data->ncu_link.nwamd_link_media != DL_WIFI ||
	    (object->nwamd_object_state != NWAM_STATE_OFFLINE &&
	    object->nwamd_object_state != NWAM_STATE_INITIALIZED))
		return (0);

	status = dladm_wlan_validate(dld_handle,
	    ncu_data->ncu_link.nwamd_link_id, &ctrl_conn, NULL);
	if (status != DLADM_STATUS_OK) {
		nlog(LOG_ERR, "nwamd_ncu_known_wlan_committed(%s): wpa_s ctrl "
		    "interface is down (%d)", ncu_data->ncu_name, status);
		return (-1);
	}

	if (wpa_request(ctrl_conn, 1, reconf)) {
		nlog(LOG_ERR, "nwamd_ncu_known_wlan_committed(%s): failed wpa_s"
		    "reconfiguration", ncu_data->ncu_name);
		wpa_ctrl_close(ctrl_conn);
		return (-1);
	}

	wpa_ctrl_close(ctrl_conn);
	return (0);
}

/* Handle known WLAN initialization/refresh event */
/* ARGSUSED */
void
nwamd_known_wlan_handle_init_event(nwamd_event_t known_wlan_event)
{
	/*
	 * Since the Known WLAN list has changed, do a reconfigure so that the
	 * best network is selected. If the known wlan is destroyed by nwamd or
	 * by the user we need to propagate new known wlan list to wpa_s
	 * configuration as well.
	 */
	(void) nwamd_walk_objects(NWAM_OBJECT_TYPE_NCU,
	    nwamd_ncu_known_wlan_committed, NULL);
}

void
nwamd_known_wlan_handle_action_event(nwamd_event_t known_wlan_event)
{
	switch (known_wlan_event->event_msg->nwe_data.nwe_object_action.
	    nwe_action) {
	case NWAM_ACTION_ADD:
	case NWAM_ACTION_REFRESH:
	case NWAM_ACTION_DESTROY:
	case NWAM_ACTION_ENABLE:
	case NWAM_ACTION_DISABLE:
		nwamd_known_wlan_handle_init_event(known_wlan_event);
		break;
	default:
		nlog(LOG_INFO, "nwam_known_wlan_handle_action_event: "
		    "unexpected action");
		break;
	}
}

int
nwamd_known_wlan_action(const char *known_wlan, nwam_action_t action)
{
	nwamd_event_t known_wlan_event = nwamd_event_init_object_action
		(NWAM_OBJECT_TYPE_KNOWN_WLAN, known_wlan, NULL, action);
	if (known_wlan_event == NULL)
		return (1);
	nwamd_event_enqueue(known_wlan_event);
	return (0);
}
