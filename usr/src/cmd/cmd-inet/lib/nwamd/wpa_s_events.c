/*
 * This file and its contents are supplied under the terms of the
 * Common Development and Distribution License ("CDDL"), version 1.0.
 * You may only use this file in accordance with the terms of version
 * 1.0 of the CDDL.
 *
 * A full copy of the text of the CDDL should have accompanied this
 * source.  A copy is of the CDDL is also available via the Internet
 * at http://www.illumos.org/license/CDDL.
 */

/*
 * Copyright (c) 2012, Enrico Papi <enricop@computer.org>. All rights reserved.
 */

/*
 * routines to retrieve wpa_supplicant control events
 * from the wpa_supplicant instance control interface
 */

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <libdllink.h>
#include <libdlwlan.h>

#include "events.h"
#include "ncp.h"
#include "ncu.h"
#include "objects.h"
#include "util.h"

/*
 * events we handle:
 * WPA_EVENT_CONNECTED   "CTRL-EVENT-CONNECTED "
 * WPA_EVENT_DISCONNECTED   "CTRL-EVENT-DISCONNECTED "
 * WPA_EVENT_ASSOC_REJECT   "CTRL-EVENT-ASSOC-REJECT "
 * WPA_EVENT_TERMINATING   "CTRL-EVENT-TERMINATING "
 * WPA_EVENT_EAP_TLS_CERT_ERROR   "CTRL-EVENT-EAP-TLS-CERT-ERROR "
 * WPA_EVENT_EAP_FAILURE   "CTRL-EVENT-EAP-FAILURE "
 * WPA_EVENT_SCAN_RESULTS   "CTRL-EVENT-SCAN-RESULTS "
 *
 */

static void
wpa_s_process_ctrl_event(const char *ev_id, const char *ev_extra,
    const char *ifname)
{
	nwamd_event_t link_event = NULL;
	nwam_error_t err;
	char bssid_str[DLADM_WLAN_BSSID_LEN*3];
	char *object_name;

	if (strcmp(ev_id, WPA_EVENT_SCAN_RESULTS) == 0) {
		/* this event occurs when at least 1 node is in nodes table */
		nlog(LOG_DEBUG, "wpa_s_event_handler: new scan "
		    "results for link %s", ifname);
		/* async */
		if (nwamd_wlan_parse_scanres(ifname))
			nlog(LOG_ERR, "wpa_s_event_handler(%s):"
			    " failed to invoke"
			    " nwamd_wlan_parse_scanres",
			    ifname);
		return;

	} else if (strcmp(ev_id, WPA_EVENT_CONNECTED) == 0) {

		nwam_wlan_t enst_wlan;

		if ((err = nwam_ncu_name_to_typed_name(ifname,
		    NWAM_NCU_TYPE_LINK, &object_name)) != NWAM_SUCCESS) {
			nlog(LOG_ERR, "wpa_s_event_handler(%s): "
			    "nwam_ncu_name_to_typed_name: %s", ifname,
			    nwam_strerror(err));
			return;
		}
		nwamd_object_set_state(NWAM_OBJECT_TYPE_NCU, object_name,
		    NWAM_STATE_OFFLINE_TO_ONLINE,
		    NWAM_AUX_STATE_LINK_WIFI_CONNECTED);

		free(object_name);

		if (sscanf(ev_extra, "- Connection to %17s completed (auth) "
		    "[id=%15u id_str=%32s]", bssid_str, &enst_wlan.nww_wlanid,
		    enst_wlan.nww_essid) == 3) {
			(void) dladm_wlan_str2bssid(bssid_str,
			    enst_wlan.nww_bssid);
		} else if (sscanf(ev_extra, "- Connection to %17s completed "
		    "(reauth) [id=%15u id_str=%32s]", bssid_str,
		    &enst_wlan.nww_wlanid, enst_wlan.nww_essid) == 3) {
			(void) dladm_wlan_str2bssid(bssid_str,
			    enst_wlan.nww_bssid);
		} else {
			nlog(LOG_ERR, "wpa_s_event_handler(%s):"
			    " Failed parsing connect event", ifname);
		}

		link_event = nwamd_event_init_wlan(ifname,
		    NWAM_EVENT_TYPE_WLAN_CONNECTION_REPORT, &enst_wlan, 1);

	} else if (strcmp(ev_id, WPA_EVENT_DISCONNECTED) == 0) {

		nwam_wlan_t disc_wlan;

		if ((err = nwam_ncu_name_to_typed_name(ifname,
		    NWAM_NCU_TYPE_LINK, &object_name)) != NWAM_SUCCESS) {
			nlog(LOG_ERR, "wpa_s_event_handler(%s): "
			    "nwam_ncu_name_to_typed_name: %s", ifname,
			    nwam_strerror(err));
			return;
		}
		nwamd_object_set_state(NWAM_OBJECT_TYPE_NCU, object_name,
		    NWAM_STATE_ONLINE_TO_OFFLINE, NWAM_AUX_STATE_DOWN);

		free(object_name);

		if (sscanf(ev_extra, "bssid=%17s reason=%u", bssid_str,
		    &disc_wlan.nww_wlanid) != 2)
			nlog(LOG_ERR, "wpa_s_event_handler(%s):"
			    " Failed parsing disassoc event");

		(void) dladm_wlan_str2bssid(bssid_str, disc_wlan.nww_bssid);

		link_event = nwamd_event_init_wlan(ifname,
		    NWAM_EVENT_TYPE_WLAN_DISASSOCIATION_REPORT, &disc_wlan, 1);

	} else if ((strcmp(ev_id, WPA_EVENT_ASSOC_REJECT) == 0) ||
	    (strcmp(ev_id, WPA_EVENT_EAP_TLS_CERT_ERROR) == 0) ||
	    (strcmp(ev_id, WPA_EVENT_EAP_FAILURE) == 0)) {

		nwam_wlan_t rej_wlan;

		if (strcmp(ev_id, WPA_EVENT_ASSOC_REJECT) == 0 &&
		    sscanf(ev_extra, "bssid=%17s status_code=%u", bssid_str,
		    &rej_wlan.nww_wlanid) != 2) {
			nlog(LOG_ERR, "wpa_s_event_handler(%s):"
			    " Failed parsing reject event");
			return;
		}

		(void) dladm_wlan_str2bssid(bssid_str, rej_wlan.nww_bssid);
		link_event = nwamd_event_init_wlan(ifname,
		    NWAM_EVENT_TYPE_WLAN_WRONG_KEY, &rej_wlan, 1);

	} else if (strcmp(ev_id, WPA_EVENT_TERMINATING) == 0) {

		nwam_error_t err;
		char *object_name;
		if ((err = nwam_ncu_name_to_typed_name(ifname,
		    NWAM_NCU_TYPE_LINK, &object_name)) != NWAM_SUCCESS) {
			nlog(LOG_ERR, "wpa_s_event_handler(%s): "
			    "nwam_ncu_name_to_typed_name: %s", ifname,
			    nwam_strerror(err));
			return;
		}
		nwamd_object_set_state(NWAM_OBJECT_TYPE_NCU, object_name,
		    NWAM_STATE_ONLINE_TO_OFFLINE, NWAM_AUX_STATE_DOWN);

		free(object_name);

	} else
		return;

	/* Create event for link */
	if (link_event != NULL)
		nwamd_event_enqueue(link_event);
	else
		nlog(LOG_ERR, "wpa_s_event_handler(%s): failed initializing "
		    "wlan report event (%s)", ifname, ev_id);

	nlog(LOG_DEBUG, "wpa_s_event_handler(%s): %s (%s)", ifname, ev_id,
	    ev_extra);
}

static void *
wpa_s_event_handler(void *arg)
{
	dladm_status_t status;
	datalink_id_t linkid;

	struct wpa_ctrl *ctrl_conn = NULL;
	char ifname[MAXLINKNAMELEN];

	nlog(LOG_DEBUG, "wpa_s_event_handler: wakeup");
	if (arg != NULL) {
		(void) memcpy(&linkid, arg, sizeof (linkid));
		free(arg);
	} else {
		nlog(LOG_ERR, "wpa_s_event_handler: invalid args");
		return (NULL);
	}
retry:
	(void) memset(ifname, 0, sizeof (ifname));
	status = dladm_wlan_validate(dld_handle, linkid, &ctrl_conn, ifname);
	if (status != DLADM_STATUS_OK) {
		if (status != DLADM_STATUS_LINKINVAL) {
			nlog(LOG_ERR, "wpa_s_event_handler: failed "
			    "validating wpa_s instance (err %d)", status);
			if (status == DLADM_STATUS_NOTFOUND) {
				nlog(LOG_ERR, "install wpa_supplicant pkg!!!");
				return (NULL);
			}
			goto retry;
		} else {
			nlog(LOG_ERR, "wpa_s_event_handler: "
			    "invalid link type (err %d)", status);
			return (NULL);
		}
	}

	if (wpa_ctrl_attach(ctrl_conn)) {
		nlog(LOG_ERR, "wpa_s_event_handler: failed attaching to "
		    "wpa_s instance ctrl_if socket");
		wpa_ctrl_close(ctrl_conn);
		goto retry;
	}

	if (wpa_ctrl_pending(ctrl_conn) < 0) {
		nlog(LOG_ERR, "wpa_s_event_handler: "
		    "Connection to wpa_supplicant lost. Returning");
		(void) wpa_ctrl_detach(ctrl_conn);
		wpa_ctrl_close(ctrl_conn);
		goto retry;
	}

	nlog(LOG_DEBUG, "wpa_s_event_handler(%s): waiting wpa_s ctrl events",
	    ifname);

	for (;;) {
		char *ev_id = NULL;
		char *ev_extra = NULL;
		int rc;

		if ((rc = wpa_ctrl_pending(ctrl_conn)) < 0) {
			nlog(LOG_ERR, "wpa_s_event_handler(%s): "
			    "Connection to wpa_supplicant lost. Returning",
			    ifname);
			(void) wpa_ctrl_detach(ctrl_conn);
			wpa_ctrl_close(ctrl_conn);
			break;
		} else if (rc == 0)
			continue;

		if (!wpa_get_event(ctrl_conn, &ev_id, &ev_extra))
			continue;

		(void) wpa_s_process_ctrl_event(ev_id, ev_extra, ifname);

		free(ev_id);
		if (ev_extra != NULL)
			free(ev_extra);
	}
	goto retry;
} /* THREAD */

static int
/* LINTED E_FUNC_ARG_UNUSED */
nwamd_wpa_s_monitor_init(dladm_handle_t handle, datalink_id_t linkid, void *arg)
{
	int rc;
	pthread_attr_t attr;
	datalink_id_t *thp_linkid = NULL;

	nlog(LOG_DEBUG, "nwamd_wpa_s_monitor_init");

	rc = pthread_attr_init(&attr);
	if (rc != 0) {
		nlog(LOG_ERR, "nwamd_wpa_s_monitor_init: "
		    "pthread_attr_init failed: %s", strerror(rc));
		return (DLADM_WALK_CONTINUE);
	}

	rc = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	if (rc != 0) {
		nlog(LOG_ERR, "nwamd_wpa_s_monitor_init: "
		    "pthread_attr_setdetachstate failed: %s",
		    strerror(rc));
		(void) pthread_attr_destroy(&attr);
		return (DLADM_WALK_CONTINUE);
	}

	thp_linkid = malloc(sizeof (linkid));
	if (thp_linkid == NULL)
		return (DLADM_WALK_CONTINUE);

	(void) memcpy(thp_linkid, &linkid, sizeof (linkid));

	rc = pthread_create(NULL, &attr, wpa_s_event_handler, thp_linkid);
	if (rc != 0) {
		nlog(LOG_ERR, "nwamd_wpa_s_monitor_init: couldn't start "
		    "wpa_s_event_handler thread: %s", strerror(rc));
		(void) pthread_attr_destroy(&attr);
		return (DLADM_WALK_CONTINUE);
	}

	return (DLADM_WALK_CONTINUE);
}

void
nwamd_wpa_s_events_init(void)
{
	dladm_status_t status;

	nlog(LOG_DEBUG, "nwamd_wpa_s_events_init");

	status = dladm_walk_datalink_id(nwamd_wpa_s_monitor_init, dld_handle,
	    NULL, DATALINK_CLASS_PHYS, DL_WIFI, DLADM_OPT_PERSIST);
	if (status != DLADM_STATUS_OK)
		nlog(LOG_ERR, "nwamd_wpa_s_events_init: failed while walking"
		    " wifi links");
}

static int
/* LINTED E_FUNC_ARG_UNUSED */
nwamd_wpa_s_monitor_fini(dladm_handle_t handle, datalink_id_t linkid, void *arg)
{
	dladm_status_t	status;

	nlog(LOG_DEBUG, "nwamd_wpa_s_monitor_fini");

	status = dladm_wlan_disconnect(handle, linkid);
	if (status != DLADM_STATUS_OK && status != DLADM_STATUS_LINKINVAL) {
		char errbuf[DLADM_STRSIZE];
		nlog(LOG_ERR, "nwamd_wpa_s_monitor_fini: dladm_wlan_disconnect "
		    "returned. err(%d) %s", status,
		    dladm_status2str(status, errbuf));
		return (DLADM_WALK_CONTINUE);
	}

	return (DLADM_WALK_CONTINUE);
}

void
nwamd_wpa_s_events_fini(void)
{
	dladm_status_t status;

	nlog(LOG_DEBUG, "nwamd_wpa_s_events_fini");

	status = dladm_walk_datalink_id(nwamd_wpa_s_monitor_fini, dld_handle,
	    NULL, DATALINK_CLASS_PHYS, DL_WIFI, DLADM_OPT_PERSIST);
	if (status != DLADM_STATUS_OK)
		nlog(LOG_ERR, "nwamd_wpa_s_events_fini: failed to walk"
		    " wifi links");
}
