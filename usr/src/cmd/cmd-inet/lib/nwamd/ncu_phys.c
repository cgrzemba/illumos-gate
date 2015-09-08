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

#include <assert.h>
#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <execinfo.h>
#include <kstat.h>
#include <libdllink.h>
#include <libdlwlan.h>
#include <libnwam.h>
#include <libdlstat.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <libdlpi.h>
#include <ucontext.h>

#include "events.h"
#include "llp.h"
#include "objects.h"
#include "known_wlans.h"
#include "ncp.h"
#include "ncu.h"
#include "util.h"

/*
 * ncu_phys.c - contains routines that are physical-link specific.
 * Mostly WiFi code.
 */

#define	NEED_ENC(sec)	(sec > DLADM_WLAN_SECMODE_NONE && \
			sec <= DLADM_WLAN_SECMODE_EAP)

/*
 * We need to ensure scan or connect threads do not run concurrently
 * on any links - otherwise we get radio interference.  Acquire this
 * lock on entering scan/connect threads to prevent this.
 */
pthread_mutex_t wireless_mutex = PTHREAD_MUTEX_INITIALIZER;

static void
scanconnect_entry(void)
{
	(void) pthread_mutex_lock(&wireless_mutex);
}

static void
scanconnect_exit(void)
{
	(void) pthread_mutex_unlock(&wireless_mutex);
}

/*
 * Get link state from kstats. Used to determine initial link state for
 * cases where drivers do not support DL_NOTE_LINK_UP/DOWN.  If link
 * state is LINK_STATE_UNKNOWN and link is wired, we assume the link is up and
 * the IP NCU timeout will cause us to move on to other links.
 */
link_state_t
nwamd_get_link_state(const char *name)
{
	kstat_ctl_t *kcp;
	kstat_t *ksp;
	char module[DLPI_LINKNAME_MAX];
	uint_t instance;
	link_state_t link_state = LINK_STATE_UNKNOWN;

	if ((kcp = kstat_open()) == NULL)
		return (link_state);

	if (dlpi_parselink(name, module, &instance) != DLPI_SUCCESS)
		goto out;

	if ((ksp = kstat_lookup(kcp, module, instance, "mac")) == NULL) {
		/*
		 * The kstat query could fail if the underlying MAC
		 * driver was already detached.
		 */
		goto out;
	}

	if (kstat_read(kcp, ksp, NULL) == -1)
		goto out;

	(void) dladm_kstat_value(ksp, "link_state", KSTAT_DATA_UINT32,
	    &link_state);

out:
	(void) kstat_close(kcp);

	return (link_state);
}

/*
 * Set/unset link propeties.  At present, these are MAC address, link MTU and
 * autopush modules.  We set MAC address last as setting it may cause a chip
 * reset which can prevent other device property setting succeeding.
 */
void
nwamd_set_unset_link_properties(nwamd_ncu_t *ncu, boolean_t set)
{
	dlpi_handle_t dh = ncu->ncu_link.nwamd_link_dhp;
	char *addr = set ? ncu->ncu_link.nwamd_link_mac_addr : NULL;
	uint64_t mtu = set ? ncu->ncu_link.nwamd_link_mtu : 0;
	char **autopush = set ? ncu->ncu_link.nwamd_link_autopush : NULL;
	uint_t num_autopush = set ? ncu->ncu_link.nwamd_link_num_autopush : 0;
	uchar_t *hwaddr = NULL, curraddr[DLPI_PHYSADDR_MAX];
	size_t hwaddrlen = DLPI_PHYSADDR_MAX;
	int retval;
	dladm_status_t status;
	char mtustr[DLADM_PROP_VAL_MAX];
	char *cp;
	char errmsg[DLADM_STRSIZE];
	uint_t cnt = 1;

	/*
	 * Set MTU here - either default value (if mtu == 0 indicating it has
	 * not been set) or specified value.
	 */
	if (mtu == 0) {
		cp = mtustr;
		status = dladm_get_linkprop(dld_handle,
		    ncu->ncu_link.nwamd_link_id, DLADM_PROP_VAL_DEFAULT, "mtu",
		    &cp, &cnt);
		if (status != DLADM_STATUS_OK) {
			nlog(LOG_ERR, "nwamd_set_unset_link_properties: "
			    "dladm_get_linkprop failed: %s",
			    dladm_status2str(status, errmsg));
			return;
		}
	} else {
		(void) snprintf(mtustr, DLADM_PROP_VAL_MAX, "%lld", mtu);
	}

	cp = mtustr;

	nlog(LOG_DEBUG, "nwamd_set_unset_link_properties: setting MTU of %s "
	    "for link %s", mtustr, ncu->ncu_name);
	status = dladm_set_linkprop(dld_handle, ncu->ncu_link.nwamd_link_id,
	    "mtu", &cp, 1, DLADM_OPT_ACTIVE);
	if (status != DLADM_STATUS_OK) {
		nlog(LOG_ERR, "nwamd_set_unset_link_properties(%s): "
		    "dladm_set_linkprop failed: %s", ncu->ncu_name,
		    dladm_status2str(status, errmsg));
	}

	nlog(LOG_DEBUG, "nwamd_set_unset_link_properties: setting %d "
	    "autopush module for link %s", num_autopush, ncu->ncu_name);
	status = dladm_set_linkprop(dld_handle, ncu->ncu_link.nwamd_link_id,
	    "autopush", autopush, num_autopush, DLADM_OPT_ACTIVE);
	if (status != DLADM_STATUS_OK) {
		nlog(LOG_ERR, "nwamd_set_unset_link_properties: "
		    "dladm_set_linkprop failed for autopush property: %s",
		    dladm_status2str(status, errmsg));
	}

	/*
	 * Set physical address - either factory (if link_mac_addr is NULL
	 * or we are unsetting properties) or specified MAC address string.
	 */
	if (addr == NULL) {
		if ((hwaddr = calloc(1, DLPI_PHYSADDR_MAX)) == NULL) {
			nlog(LOG_ERR,
			    "nwamd_set_unset_link_properties: malloc() failed");
			return;
		}
		if ((retval = dlpi_get_physaddr(dh, DL_FACT_PHYS_ADDR,
		    hwaddr, &hwaddrlen)) != DLPI_SUCCESS) {
			nlog(LOG_ERR, "nwamd_set_unset_link_properties: "
			    "could not get physical address for %s: %s",
			    ncu->ncu_name, dlpi_strerror(retval));
			free(hwaddr);
			return;
		}
	} else {
		int addrlen = hwaddrlen;
		if ((hwaddr = _link_aton(addr, &addrlen)) == NULL) {
			if (addrlen == -1) {
				nlog(LOG_ERR,
				    "nwamd_set_unset_link_properties: "
				    "%s: bad address for %s",
				    addr, ncu->ncu_name);
				return;
			} else {
				nlog(LOG_ERR, "nwamd_set_unset_link_properties:"
				    " malloc() failed");
				return;
			}
		}
		hwaddrlen = addrlen;
	}

	/*
	 * Only set physical address if desired address differs from current -
	 * this avoids unnecessary chip resets for some drivers.
	 */
	retval = dlpi_get_physaddr(dh, DL_CURR_PHYS_ADDR, curraddr,
	    &hwaddrlen);
	if (retval != DLPI_SUCCESS || bcmp(curraddr, hwaddr, hwaddrlen) != 0) {
		retval = dlpi_set_physaddr(dh, DL_CURR_PHYS_ADDR, hwaddr,
		    hwaddrlen);
		if (retval != DLPI_SUCCESS) {
			nlog(LOG_ERR, "nwamd_set_unset_link_properties:"
			    "failed setting mac address on %s: %s",
			    ncu->ncu_name, dlpi_strerror(retval));
		}
	}
	free(hwaddr);
}

/*
 * Checks if a wireless network can be selected or not.  A wireless network
 * CANNOT be selected if the NCU is DISABLED, or the NCU is OFFLINE or
 * ONLINE* and has lower priority than the currently active priority-group.
 * Called with object lock held.
 */
static boolean_t
wireless_selection_possible(nwamd_object_t object)
{
	nwamd_ncu_t *ncu = object->nwamd_object_data;

	if (ncu->ncu_link.nwamd_link_media != DL_WIFI)
		return (B_FALSE);

	(void) pthread_mutex_lock(&active_ncp_mutex);
	if (object->nwamd_object_state == NWAM_STATE_DISABLED ||
	    object->nwamd_object_state == NWAM_STATE_UNINITIALIZED ||
	    ((object->nwamd_object_state == NWAM_STATE_OFFLINE ||
	    object->nwamd_object_state == NWAM_STATE_ONLINE_TO_OFFLINE) &&
	    ncu->ncu_link.nwamd_link_activation_mode ==
	    NWAM_ACTIVATION_MODE_PRIORITIZED &&
	    (current_ncu_priority_group == INVALID_PRIORITY_GROUP ||
	    ncu->ncu_link.nwamd_link_priority_group >
	    current_ncu_priority_group))) {
		(void) pthread_mutex_unlock(&active_ncp_mutex);
		return (B_FALSE);
	}
	(void) pthread_mutex_unlock(&active_ncp_mutex);

	return (B_TRUE);
}

nwam_error_t
nwamd_wlan_select(const char *linkname, const nwam_wlan_t *mywlan,
    const dladm_wlan_key_t *key_data, const dladm_wlan_eap_t *eap_data)
{
	nwamd_object_t ncu_obj;
	nwamd_ncu_t *ncu;
	nwamd_link_t *link;
	char tmpbuf[DLADM_WLAN_BSSID_LEN * 3];

	if ((ncu_obj = nwamd_ncu_object_find(NWAM_NCU_TYPE_LINK, linkname))
	    == NULL) {
		nlog(LOG_ERR, "nwamd_wlan_select: could not find object  "
		    "for link %s", linkname);
		return (NWAM_ENTITY_NOT_FOUND);
	}
	ncu = ncu_obj->nwamd_object_data;
	link = &ncu->ncu_link;

	/*
	 * If wireless selection is not possible because of the current
	 * state or priority-group, then stop.
	 */
	if (!wireless_selection_possible(ncu_obj)) {
		nlog(LOG_ERR, "nwamd_wlan_select: wireless selection not "
		    "possible. Invalid state for link %s", linkname);
		nwamd_object_release(ncu_obj);
		return (NWAM_ENTITY_INVALID_STATE);
	}

	(void) memcpy(&link->nwamd_link_wifi_wlan, mywlan,
	    sizeof (nwam_wlan_t));

	if (link->nwamd_link_wifi_key != NULL) {
		free(link->nwamd_link_wifi_key);
		link->nwamd_link_wifi_key = NULL;
	}
	if (key_data != NULL) {
		link->nwamd_link_wifi_key = malloc(sizeof (dladm_wlan_key_t));
		(void) memcpy(link->nwamd_link_wifi_key, key_data,
		    sizeof (dladm_wlan_key_t));
	}

	if (link->nwamd_link_wifi_eap != NULL) {
		free(link->nwamd_link_wifi_eap);
		link->nwamd_link_wifi_eap = NULL;
	}
	if (eap_data != NULL) {
		link->nwamd_link_wifi_eap = malloc(sizeof (dladm_wlan_eap_t));
		(void) memcpy(link->nwamd_link_wifi_eap, eap_data,
		    sizeof (dladm_wlan_eap_t));
	}

	nlog(LOG_DEBUG, "nwamd_wlan_select: selecting target wlan (essid %s, "
	    "bssid %s) for link %s",
	    link->nwamd_link_wifi_wlan.nww_essid,
	    dladm_wlan_bssid2str(link->nwamd_link_wifi_wlan.nww_bssid, tmpbuf),
	    linkname);

	nwamd_object_release(ncu_obj);

	/* start connect thread */
	nwamd_wlan_connect(linkname);

	return (NWAM_SUCCESS);
}

/*
 * WLAN scan code for WIFI link NCUs.
 */

static boolean_t
fill_wifi_scan(void *arg, dladm_wlan_attr_t *attrp)
{
	nwamd_wifi_scan_t *s = arg;
	uint_t index = 0;
	int wlanid = -1;
	char tmpbuf[DLADM_WLAN_BSSID_LEN * 3];

	if (s == NULL || attrp == NULL) {
		nlog(LOG_ERR, "fill_wifi_scan: invalid args");
		return (B_FALSE);
	}

	index = s->nwamd_wifi_sres_num;
	s->nwamd_wifi_sres = realloc(s->nwamd_wifi_sres,
	    sizeof (nwam_wlan_t) * (index + 1));

	if (s->nwamd_wifi_sres == NULL) {
		nlog(LOG_ERR, "fill_wifi_scan: no memory");
		return (B_FALSE);
	}
	(void) memset(&s->nwamd_wifi_sres[index], 0, sizeof (nwam_wlan_t));

	if (attrp->wa_valid & DLADM_WLAN_ATTR_ESSID) {
		(void) memcpy(s->nwamd_wifi_sres[index].nww_essid,
		    attrp->wa_essid.we_bytes, attrp->wa_essid.we_length);
		s->nwamd_wifi_sres[index].nww_esslen =
		    attrp->wa_essid.we_length;
	}

	(void) memcpy(s->nwamd_wifi_sres[index].nww_bssid,
	    attrp->wa_bssid.wb_bytes, DLADM_WLAN_BSSID_LEN);

	s->nwamd_wifi_sres[index].nww_security_mode = attrp->wa_secmode;
	s->nwamd_wifi_sres[index].nww_freq = attrp->wa_freq;
	s->nwamd_wifi_sres[index].nww_strength = attrp->wa_strength;
	s->nwamd_wifi_sres[index].nww_rate = attrp->wa_rates.wr_rates[0];
	s->nwamd_wifi_sres[index].nww_scanid = s->nwamd_wifi_sres_num;
	(void) strncpy(s->nwamd_wifi_sres[index].nww_ietxt, attrp->wa_ietxt,
	    sizeof (attrp->wa_ietxt));

	nlog(LOG_DEBUG, "fill_wifi_scan(%d): ESSID %s, BSSID %s", index,
	    s->nwamd_wifi_sres[index].nww_essid, dladm_wlan_bssid2str(
	    s->nwamd_wifi_sres[index].nww_bssid, tmpbuf));

	s->nwamd_wifi_sres_num++;

	if (nwam_walk_known_wlans(find_matching_wlan_cb,
	    &s->nwamd_wifi_sres[index], &wlanid) != NWAM_WALK_HALTED) {
		s->nwamd_wifi_sres[index].nww_wlanid = 0;
		return (B_TRUE);
	}

	if (wlanid > 0 && wlanid == s->nwamd_wifi_sres[index].nww_wlanid)
		nlog(LOG_DEBUG, "fill_wifi_scan(%d): found known wlan %d",
		    index, wlanid);

	return (B_TRUE);
}

/*
 * Check if we're connected to the expected WLAN,
 */
boolean_t
nwamd_wlan_connected(nwamd_object_t ncu_obj)
{
	nwamd_ncu_t *ncu = ncu_obj->nwamd_object_data;
	nwamd_link_t *link = &ncu->ncu_link;
	nwam_wlan_t currwlan;
	boolean_t connected;

	if (dladm_wlan_get_linkstatus(dld_handle, link->nwamd_link_id,
	    &connected, sizeof (connected)) != DLADM_STATUS_OK)
		return (B_FALSE);

	if (connected) {
		char tmpbuf[DLADM_WLAN_BSSID_LEN * 3];
		(void) memset(&currwlan, 0, sizeof (nwam_wlan_t));
		if (dladm_wlan_get_essid(dld_handle, link->nwamd_link_id,
		    currwlan.nww_essid, &currwlan.nww_esslen))
			return (B_FALSE);
		if (dladm_wlan_get_bssid(dld_handle, link->nwamd_link_id,
		    currwlan.nww_bssid))
			return (B_FALSE);
		nlog(LOG_DEBUG, "nwamd_wlan_connected(%s): associated to %s %s",
		    ncu->ncu_name, currwlan.nww_essid,
		    dladm_wlan_bssid2str(currwlan.nww_bssid, tmpbuf));
	} else
		return (B_FALSE);

	/*
	 * Are we connected to expected WLAN?
	 */
	if (link->nwamd_link_wifi_wlan.nww_esslen == currwlan.nww_esslen &&
	    memcmp(currwlan.nww_essid, link->nwamd_link_wifi_wlan.nww_essid,
	    link->nwamd_link_wifi_wlan.nww_esslen) == 0 &&
	    memcmp(currwlan.nww_bssid, link->nwamd_link_wifi_wlan.nww_bssid,
	    DLADM_WLAN_BSSID_LEN) == 0) {
		return (B_TRUE);
	} else {
		(void) nlog(LOG_ERR,
		    "nwamd_wlan_connected: wrong AP on %s; expected %s %s",
		    ncu->ncu_name, link->nwamd_link_wifi_wlan.nww_essid,
		    link->nwamd_link_wifi_wlan.nww_bssid);
		return (B_FALSE);
	}
}

nwam_error_t
nwamd_wlan_scan(const char *linkname)
{
	struct wpa_ctrl *ctrl_conn = NULL;
	char *scancmd[] = {"SCAN"};
	int rc = 0;
	dladm_status_t status;

	nwamd_object_t ncu_obj;
	nwamd_ncu_t *ncu;
	nwamd_link_t *link;

	if (linkname == NULL) {
		nlog(LOG_ERR, "nwamd_wlan_scan: bad arg");
		return (NWAM_INVALID_ARG);
	}

	nlog(LOG_DEBUG, "nwamd_wlan_scan: WLAN scan for %s", linkname);

	if ((ncu_obj = nwamd_ncu_object_find(NWAM_NCU_TYPE_LINK, linkname))
	    == NULL) {
		nlog(LOG_ERR, "nwamd_wlan_scan: could not find object "
		    "for link %s", linkname);
		return (NWAM_ENTITY_NOT_FOUND);
	}

	ncu = ncu_obj->nwamd_object_data;
	link = &ncu->ncu_link;

	status = dladm_wlan_validate(dld_handle, link->nwamd_link_id,
	    &ctrl_conn, NULL);
	if (status != DLADM_STATUS_OK) {
		nlog(LOG_ERR, "nwamd_wlan_scan(%s): failed connecting to wpa_s "
		    "ctrlif. (err %d)", linkname, status);
		nwamd_object_release(ncu_obj);
		return (NWAM_ERROR_BIND);
	}

	rc = wpa_request(ctrl_conn, 1, scancmd);

	nwamd_object_release(ncu_obj);
	wpa_ctrl_close(ctrl_conn);

	if (rc) {
		nlog(LOG_ERR, "nwamd_wlan_scan: send scan request to wpa_s "
		    "error %d", rc);
		return (NWAM_ERROR_INTERNAL);
	}

	return (NWAM_SUCCESS);
}

/*
 * WLAN parse scan results thread. Called with the per-link WiFi mutex held.
 */
static void *
wlan_parse_scanres_thread(void *arg)
{
	char *linkname = arg;

	nwamd_object_t ncu_obj;
	nwamd_ncu_t *ncu;
	nwamd_link_t *link;
	nwamd_event_t scan_event;

	dladm_status_t status;

	if ((ncu_obj = nwamd_ncu_object_find(NWAM_NCU_TYPE_LINK, linkname))
	    == NULL) {
		nlog(LOG_ERR, "wlan_parse_scanres_thread: could not find object"
		    " for link %s", linkname);
		free(linkname);
		return (NULL);
	}

	ncu = ncu_obj->nwamd_object_data;
	link = &ncu->ncu_link;

	if (link->nwamd_link_wifi_scan.nwamd_wifi_sres != NULL)
		free(link->nwamd_link_wifi_scan.nwamd_wifi_sres);
	link->nwamd_link_wifi_scan.nwamd_wifi_sres_num = 0;
	link->nwamd_link_wifi_scan.nwamd_wifi_sres = NULL;

	nlog(LOG_DEBUG, "wlan_parse_scanres_thread: parsing scan results on %s",
	    ncu->ncu_name);

	scanconnect_entry();
	status = dladm_wlan_parse_esslist(dld_handle, link->nwamd_link_id,
	    &link->nwamd_link_wifi_scan, fill_wifi_scan);
	scanconnect_exit();

	if (status != DLADM_STATUS_OK) {
		char errbuf[DLADM_STRSIZE];
		nlog(LOG_ERR, "wlan_parse_scanres_thread: cannot scan link %s"
		    " error(%d): %s", ncu->ncu_name, status,
		    dladm_status2str(status, errbuf));
		nwamd_object_release(ncu_obj);
		free(linkname);
		return (NULL);
	}

	if (link->nwamd_link_wifi_scan.nwamd_wifi_sres_num != 0) {
		scan_event = nwamd_event_init_wlan
		    (ncu->ncu_name, NWAM_EVENT_TYPE_WLAN_SCAN_REPORT, NULL,
		    link->nwamd_link_wifi_scan.nwamd_wifi_sres_num);
		if (scan_event == NULL) {
			nlog(LOG_ERR,
			    "wlan_parse_scanres_thread: failed to init"
			    "wlan scan report event");
		}
		nwamd_event_enqueue(scan_event);
	}

	nwamd_object_release(ncu_obj);
	free(linkname);
	return (NULL);
}

nwam_error_t
nwamd_wlan_parse_scanres(const char *linkname)
{
	pthread_t wifi_thread;
	char *link = strdup(linkname);

	if (link == NULL) {
		nlog(LOG_ERR, "nwamd_wlan_parse_scanres: out of memory");
		return (NWAM_NO_MEMORY);
	}

	nlog(LOG_DEBUG, "nwamd_wlan_parse_scanres: got scanres for %s", link);

	if (pthread_create(&wifi_thread, NULL, wlan_parse_scanres_thread,
	    link) != 0) {
		nlog(LOG_ERR, "nwamd_wlan_parse_scanres: could not start"
		    " wlan_parse_scanres_thread");
		free(link);
		return (NWAM_ERROR_INTERNAL);
	}
	/* detach thread so that it doesn't become a zombie */
	(void) pthread_detach(wifi_thread);
	return (NWAM_SUCCESS);
}

/*
 * WLAN connection code.
 */
static void *
wlan_connect_thread(void *arg)
{
	char *linkname = arg;
	nwamd_object_t ncu_obj;
	nwamd_ncu_t *ncu;
	nwamd_link_t *link;
	dladm_wlan_attr_t attr;
	dladm_status_t status;
	char errbuf[DLADM_STRSIZE];

	if ((ncu_obj = nwamd_ncu_object_find(NWAM_NCU_TYPE_LINK, linkname))
	    == NULL) {
		nlog(LOG_ERR, "wlan_connect_thread: could not find object "
		    "for link %s", linkname);
		free(linkname);
		return (NULL);
	}

	ncu = ncu_obj->nwamd_object_data;
	link = &ncu->ncu_link;

	if (!wireless_selection_possible(ncu_obj)) {
		nlog(LOG_ERR, "wlan_connect_thread: %s in invalid state or "
		    "has lower priority", linkname);
		goto done;
	}

	/* we are connecting to a known wlan */
	if (link->nwamd_link_wifi_wlan.nww_wlanid != 0) {
		char *select_request[] = {"SELECT_NETWORK", NULL};
		char wlanid[16];
		int idlen;
		struct wpa_ctrl *ctrl_conn = NULL;

		idlen = snprintf(wlanid, sizeof (wlanid), "%u",
		    link->nwamd_link_wifi_wlan.nww_wlanid);
		if (idlen < 1 || idlen >= sizeof (wlanid)) {
			nlog(LOG_ERR, "wlan_connect_thread: invalid wlan id");
			goto done;
		}
		select_request[1] = wlanid;

		status = dladm_wlan_validate(dld_handle, link->nwamd_link_id,
		    &ctrl_conn, NULL);
		if (status != DLADM_STATUS_OK) {
			nlog(LOG_ERR, "wlan_connect_thread: failed connecting"
			    "to wpa_s interface (err: %d)", status);
			goto done;
		}

		if (wpa_request(ctrl_conn, 2, select_request))
			nlog(LOG_ERR, "wlan_connect_thread: failed selecting "
			    "known wlan %u", wlanid);
		else
			nlog(LOG_DEBUG, "wlan_connect_thread: connecting to "
			    "known wlan %u", wlanid);

		wpa_ctrl_close(ctrl_conn);
		goto done;
	}

	(void) memset(&attr, 0, sizeof (attr));
	if (link->nwamd_link_wifi_wlan.nww_esslen != 0) {
		(void) memcpy(attr.wa_essid.we_bytes,
		    link->nwamd_link_wifi_wlan.nww_essid,
		    link->nwamd_link_wifi_wlan.nww_esslen);
		attr.wa_essid.we_length = link->nwamd_link_wifi_wlan.nww_esslen;
		attr.wa_valid = DLADM_WLAN_ATTR_ESSID;
	} else if (NEED_ENC(link->nwamd_link_wifi_wlan.nww_security_mode)) {
		nlog(LOG_ERR, "wlan_connect_thread: ESSID is mandatory "
		    "for non-plaintext AP!");
		goto done;
	}

	if (link->nwamd_link_wifi_wlan.nww_bssid[0] != 0) {
		(void) memcpy(attr.wa_bssid.wb_bytes,
		    link->nwamd_link_wifi_wlan.nww_bssid, DLADM_WLAN_BSSID_LEN);
		attr.wa_valid = DLADM_WLAN_ATTR_BSSID;
	}

	attr.wa_secmode = link->nwamd_link_wifi_wlan.nww_security_mode;
	attr.wa_valid |= DLADM_WLAN_ATTR_SECMODE;
	if (attr.wa_secmode != DLADM_WLAN_SECMODE_NONE)
		nlog(LOG_DEBUG, "wlan_connect_thread: retrieved key");

	scanconnect_entry();
	status = dladm_wlan_connect(dld_handle, link->nwamd_link_id, &attr,
	    link->nwamd_link_wifi_key, link->nwamd_link_wifi_eap);
	scanconnect_exit();

	if (status != DLADM_STATUS_OK) {
		nlog(LOG_ERR, "wlan_connect_thread: connect failed for %s"
		    " error:'%s'", linkname, dladm_status2str(status, errbuf));
	} else {
		nlog(LOG_DEBUG, "wlan_connect_thread: connecting to %s for %s",
		    attr.wa_essid.we_bytes, linkname);
	}

done:
	nwamd_object_release(ncu_obj);
	free(linkname);

	return (NULL);
}

void
nwamd_wlan_connect(const char *linkname)
{
	pthread_t wifi_thread;
	char *link = strdup(linkname);

	if (link == NULL) {
		nlog(LOG_ERR, "nwamd_wlan_connect: out of memory");
		return;
	}

	nlog(LOG_DEBUG, "nwamd_wlan_connect: WLAN connect for %s",
	    link);

	if (pthread_create(&wifi_thread, NULL, wlan_connect_thread, link) != 0)
		nlog(LOG_ERR, "nwamd_wlan_connect: could not start connect");

	/* detach thread so that it doesn't become a zombie */
	(void) pthread_detach(wifi_thread);
}

void
nwamd_ncu_handle_link_state_event(nwamd_event_t event)
{
	nwam_event_t evm;
	nwamd_object_t ncu_obj;
	nwamd_ncu_t *ncu;
	nwamd_link_t *link;

	ncu_obj = nwamd_object_find(NWAM_OBJECT_TYPE_NCU, event->event_object);
	if (ncu_obj == NULL) {
		nlog(LOG_INFO, "nwamd_ncu_handle_link_state_event: no object "
		    "%s", event->event_object);
		nwamd_event_do_not_send(event);
		return;
	}
	ncu = ncu_obj->nwamd_object_data;
	link = &ncu->ncu_link;
	evm = event->event_msg;

	/*
	 * If it's a link up event and we're not disabled, go online.
	 */
	if (evm->nwe_data.nwe_link_state.nwe_link_up &&
	    ncu_obj->nwamd_object_state != NWAM_STATE_DISABLED) {

		boolean_t wifi = (link->nwamd_link_media == DL_WIFI);

		if (link->nwamd_link_activation_mode ==
		    NWAM_ACTIVATION_MODE_PRIORITIZED) {
			int64_t priority_group;

			(void) pthread_mutex_lock(&active_ncp_mutex);
			priority_group = current_ncu_priority_group;
			(void) pthread_mutex_unlock(&active_ncp_mutex);

			/* compare priority groups */
			if (link->nwamd_link_priority_group > priority_group) {
				nlog(LOG_DEBUG,
				    "nwamd_ncu_handle_link_state_event: "
				    "got LINK UP event for priority group "
				    "%lld, less preferred than current %lld, "
				    "ignoring",
				    link->nwamd_link_priority_group,
				    priority_group);

			} else if (link->nwamd_link_priority_group ==
			    priority_group) {
				nlog(LOG_DEBUG,
				    "nwamd_ncu_handle_link_state_event: "
				    "got LINK UP event for priority group "
				    "%lld, same as current %lld",
				    link->nwamd_link_priority_group,
				    priority_group);
				/*
				 * Change link state to UP.  It will be
				 * propagated to IP state machine.  Only do
				 * the NCU check if and when the interface
				 * NCU is online.
				 */
				nwamd_object_set_state(NWAM_OBJECT_TYPE_NCU,
				    event->event_object,
				    NWAM_STATE_OFFLINE_TO_ONLINE,
				    wifi ? NWAM_AUX_STATE_LINK_WIFI_ASSOCIATED :
				    NWAM_AUX_STATE_UP);
			} else {
				nlog(LOG_DEBUG,
				    "nwamd_ncu_handle_link_state_event: "
				    "got LINK UP event for priority group "
				    "%lld, more preferred than current %lld",
				    link->nwamd_link_priority_group,
				    priority_group);

				/*
				 * We need to mark the link as up so that when
				 * it is activated we will bring the interface
				 * up.
				 */
				nwamd_object_set_state(NWAM_OBJECT_TYPE_NCU,
				    event->event_object,
				    NWAM_STATE_OFFLINE_TO_ONLINE,
				    wifi ? NWAM_AUX_STATE_LINK_WIFI_ASSOCIATED :
				    NWAM_AUX_STATE_UP);
				nwamd_object_release(ncu_obj);
				nwamd_ncp_deactivate_priority_group
				    (priority_group);
				nwamd_ncp_activate_priority_group
				    (link->nwamd_link_priority_group);
				return;
			}

		} else if (link->nwamd_link_activation_mode ==
		    NWAM_ACTIVATION_MODE_MANUAL) {
			nlog(LOG_DEBUG, "nwamd_ncu_handle_link_state_event: "
			    "got LINK UP event for manual NCU %s",
			    ncu_obj->nwamd_object_name);

			nwamd_object_set_state(NWAM_OBJECT_TYPE_NCU,
			    event->event_object, NWAM_STATE_OFFLINE_TO_ONLINE,
			    wifi ? NWAM_AUX_STATE_LINK_WIFI_ASSOCIATED :
			    NWAM_AUX_STATE_UP);
		}
	}

	/*
	 * If the link is down then start or continue transition down.
	 */
	if (!evm->nwe_data.nwe_link_state.nwe_link_up &&
	    (ncu_obj->nwamd_object_state == NWAM_STATE_ONLINE ||
	    ncu_obj->nwamd_object_state == NWAM_STATE_OFFLINE_TO_ONLINE)) {

		if (link->nwamd_link_activation_mode ==
		    NWAM_ACTIVATION_MODE_PRIORITIZED) {
			nlog(LOG_DEBUG,
			    "nwamd_ncu_handle_link_state_event: "
			    "got LINK DOWN for priority group %lld",
			    link->nwamd_link_priority_group);
			/* Moving to offline checks priority group */
		} else {
			nlog(LOG_DEBUG, "nwamd_ncu_handle_link_state_event: "
			    "got LINK DOWN event for manual NCU %s",
			    ncu_obj->nwamd_object_name);
		}
		nwamd_object_set_state(NWAM_OBJECT_TYPE_NCU,
		    event->event_object, NWAM_STATE_ONLINE_TO_OFFLINE,
		    NWAM_AUX_STATE_DOWN);
	}

	nwamd_object_release(ncu_obj);
}
