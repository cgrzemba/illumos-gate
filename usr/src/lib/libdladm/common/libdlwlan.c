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
 * Copyright 2009 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 * Copyright (c) 2012, Enrico Papi <enricop@computer.org>. All rights reserved.
 */

#include <stddef.h>
#include <errno.h>
#include <stropts.h>
#include <string.h>
#include <unistd.h>
#include <libscf.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <secobj.h>
#include <wpa_defs.h>
#include <libdllink.h>
#include <libdlwlan.h>
#include <libdladm_impl.h>
#include <libdlwlan_impl.h>
#include <sys/net80211_crypto.h>
#include <libnwam.h>

static dladm_status_t	wpa_instance_create(struct wpa_ctrl *, char *);
static dladm_status_t	wpa_instance_delete(struct wpa_ctrl *, char *);
static dladm_status_t	wpa_network_config(struct wpa_ctrl *,
    dladm_wlan_attr_t *attrp, const dladm_wlan_key_t *key,
    dladm_wlan_eap_t *identity);

static dladm_status_t 	do_get_signal(dladm_handle_t, datalink_id_t, void *,
			    size_t);
static dladm_status_t 	do_get_rate(dladm_handle_t, datalink_id_t, void *,
			    size_t);
static dladm_status_t	do_get_mode(dladm_handle_t, datalink_id_t, void *,
			    size_t);
static dladm_status_t	do_set_wep(dladm_handle_t, datalink_id_t);

static boolean_t	find_val_by_name(const char *, const val_desc_t *,
			    uint_t, uint_t *);
static boolean_t	find_name_by_val(uint_t, const val_desc_t *, uint_t,
			    char **);
static void		generate_essid(dladm_wlan_essid_t *);

static dladm_status_t	dladm_wlan_wlresult2status(wldp_t *);

static const val_desc_t	linkstatus_vals[] = {
	{ "disconnected", B_FALSE	},
	{ "connected",    B_TRUE	}
};

static const val_desc_t 	secmode_vals[] = {
	{ "none",	DLADM_WLAN_SECMODE_NONE	},
	{ "wep",	DLADM_WLAN_SECMODE_WEP	},
	{ "wpa-psk",	DLADM_WLAN_SECMODE_PSK	},
	{ "wpa-eap",	DLADM_WLAN_SECMODE_EAP	}
};

static const val_desc_t	mode_vals[] = {
	{ "a",		DLADM_WLAN_MODE_80211A		},
	{ "b",		DLADM_WLAN_MODE_80211B		},
	{ "g",		DLADM_WLAN_MODE_80211G		},
	{ "n",		DLADM_WLAN_MODE_80211GN		},
	{ "n",		DLADM_WLAN_MODE_80211AN		}
};

/* capital chars */
static const val_desc_t auth_vals[] = {
	{ "OPEN",	WPA_AUTH_ALG_OPEN		},
	{ "SHARED",	WPA_AUTH_ALG_SHARED		}
};

static const val_desc_t bsstype_vals[] = {
	{ "ESS",	IEEE80211_MODE_INFRA		},
	{ "IBSS",	IEEE80211_MODE_IBSS		}
};

static dladm_status_t
dladm_wlan_wlresult2status(wldp_t *gbuf)
{
	switch (gbuf->wldp_result) {
	case WL_SUCCESS:
		return (DLADM_STATUS_OK);

	case WL_NOTSUPPORTED:
	case WL_LACK_FEATURE:
		return (DLADM_STATUS_NOTSUP);

	case WL_READONLY:
		return (DLADM_STATUS_PROPRDONLY);

	default:
		break;
	}

	return (DLADM_STATUS_FAILED);
}

static dladm_wlan_mode_t
do_convert_mode(wl_phy_conf_t *phyp)
{
	wl_erp_t *wlep = &phyp->wl_phy_erp_conf;
	wl_ofdm_t *wlop = &phyp->wl_phy_ofdm_conf;

	switch (phyp->wl_phy_fhss_conf.wl_fhss_subtype) {
	case WL_ERP:
		return (wlep->wl_erp_ht_enabled ?
		    DLADM_WLAN_MODE_80211GN : DLADM_WLAN_MODE_80211G);
	case WL_OFDM:
		return (wlop->wl_ofdm_ht_enabled ?
		    DLADM_WLAN_MODE_80211AN : DLADM_WLAN_MODE_80211A);
	case WL_DSSS:
	case WL_FHSS:
		return (DLADM_WLAN_MODE_80211B);
	default:
		break;
	}

	return (DLADM_WLAN_MODE_NONE);
}

boolean_t
i_dladm_wlan_convert_chan(wl_phy_conf_t *phyp, uint16_t *channelp)
{
	wl_fhss_t *wlfp = &phyp->wl_phy_fhss_conf;
	wl_ofdm_t *wlop = &phyp->wl_phy_ofdm_conf;

	switch (wlfp->wl_fhss_subtype) {
	case WL_FHSS:
	case WL_DSSS:
	case WL_IRBASE:
	case WL_HRDS:
	case WL_ERP:
		*channelp = wlfp->wl_fhss_frequency;
		break;
	case WL_OFDM:
		*channelp = wlop->wl_ofdm_frequency;
		break;
	default:
		return (B_FALSE);
	}
	return (B_TRUE);
}

static void
fill_wlan_attr(wl_ess_conf_t *wlp, dladm_wlan_attr_t *attrp)
{
	int i;
	int keymgmt = -1;

	/*
	 * Note: SSID is an array of octets, i.e.,
	 * it is not NULL terminated and can, at least in theory,
	 * contain control characters (including NULL) and as such,
	 * should be processed as binary data, not a printable string.
	 */
	if (wlp->wl_ess_conf_essid.wl_essid_length != 0) {
		attrp->wa_essid.we_length =
		    wlp->wl_ess_conf_essid.wl_essid_length;
		(void) memcpy(attrp->wa_essid.we_bytes,
		    wlp->wl_ess_conf_essid.wl_essid_essid,
		    attrp->wa_essid.we_length);
		attrp->wa_valid |= DLADM_WLAN_ATTR_ESSID;
	}

	(void) memcpy(attrp->wa_bssid.wb_bytes,
	    wlp->wl_ess_conf_bssid.wl_bssid_bssid, DLADM_WLAN_BSSID_LEN);
	attrp->wa_valid |= DLADM_WLAN_ATTR_BSSID;

	/*
	 * Open/Shared is simply not present in the AP's beacon.  It is a
	 * deficiency of WEP, which WPA fixed by only allowing open
	 * authentication for WPA connections. There is simply no way to tell
	 * whether an AP is using shared auth or open system auth because a WEP
	 * AP never broadcasts that information.
	 * [net80211 sets it to 1 (OPEN) by default for scanned nodes]
	 *
	 * If wpa_ie is not present and IEEE80211_CAP_PRIVACY is on => WEP
	 * If wpa_ie is not present and IEEE80211_CAP_PRIVACY is off => NONE
	 */
	if (wpa_supplicant_capsie2txt(wlp->wl_ess_conf_caps,
	    wlp->wl_ess_conf_wpa_ie.wpa_ie, wlp->wl_ess_conf_wpa_ie.wpa_ie_len,
	    attrp->wa_ietxt, sizeof (attrp->wa_ietxt), &keymgmt) == 0)
		attrp->wa_valid |= DLADM_WLAN_ATTR_IEDETAIL;

	if (wlp->wl_ess_conf_wpa_ie.wpa_ie_len == 0) {
		if (wlp->wl_ess_conf_caps & IEEE80211_CAP_PRIVACY) {
			attrp->wa_auth = WPA_AUTH_ALG_SHARED;
			attrp->wa_secmode = DLADM_WLAN_SECMODE_WEP;
		} else {
			attrp->wa_auth = WPA_AUTH_ALG_OPEN;
			attrp->wa_secmode = DLADM_WLAN_SECMODE_NONE;
		}
		attrp->wa_valid |= DLADM_WLAN_ATTR_SECMODE;
	} else if (keymgmt != -1) {
		/*
		 * check wpa_ie.c defines
		 */
		attrp->wa_auth = WPA_AUTH_ALG_OPEN;
		if (keymgmt == WPA_KEY_MGMT_IEEE8021X) {
			attrp->wa_secmode = DLADM_WLAN_SECMODE_EAP;
			attrp->wa_valid |= DLADM_WLAN_ATTR_SECMODE;
		} else if (keymgmt == WPA_KEY_MGMT_PSK) {
			attrp->wa_secmode = DLADM_WLAN_SECMODE_PSK;
			attrp->wa_valid |= DLADM_WLAN_ATTR_SECMODE;
		}
	}
	attrp->wa_valid |= DLADM_WLAN_ATTR_AUTH;

	/* caps values are not mutually-exclusive here */
	if (wlp->wl_ess_conf_caps & IEEE80211_CAP_IBSS)
		attrp->wa_bsstype = IEEE80211_MODE_IBSS;
	if (wlp->wl_ess_conf_caps & IEEE80211_CAP_ESS)
		attrp->wa_bsstype = IEEE80211_MODE_INFRA;
	attrp->wa_valid |= DLADM_WLAN_ATTR_BSSTYPE;

	/* signal strength (0->127) */
	attrp->wa_strength = wlp->wl_ess_conf_sl;
	attrp->wa_valid |= DLADM_WLAN_ATTR_STRENGTH;

	/* speed rates */
	for (i = 0; i < wlp->wl_ess_conf_rates.wl_rates_num; i++)
		attrp->wa_rates.wr_rates[i] =
		    wlp->wl_ess_conf_rates.wl_rates_rates[i];
	attrp->wa_rates.wr_cnt = wlp->wl_ess_conf_rates.wl_rates_num;
	attrp->wa_valid |= DLADM_WLAN_ATTR_RATES;

	/* 802.11 mode + frequency */
	attrp->wa_mode = do_convert_mode(&wlp->wl_ess_conf_phys);
	attrp->wa_valid |= DLADM_WLAN_ATTR_MODE;

	if (i_dladm_wlan_convert_chan(&wlp->wl_ess_conf_phys, &attrp->wa_freq))
		attrp->wa_valid |= DLADM_WLAN_ATTR_CHANNEL;
}

dladm_status_t
dladm_wlan_scan(dladm_handle_t handle, datalink_id_t linkid)
{
	struct wpa_ctrl *ctrl_conn = NULL;
	char *cmd_scan[] = {"SCAN"};

	dladm_status_t	status;

	if ((status = dladm_wlan_validate(handle, linkid, &ctrl_conn, NULL)) !=
	    DLADM_STATUS_OK)
		return (status);

	/*
	 * using wpa ctrl_if to request scan we can wait for
	 * WPA_EVENT_SCAN_RESULTS before processing scan results
	 */
	if (wpa_request(ctrl_conn, 1, cmd_scan)) {
		wpa_ctrl_close(ctrl_conn);
		return (DLADM_STATUS_FAILED);
	}

	if (wpa_ctrl_attach(ctrl_conn)) {
		wpa_ctrl_close(ctrl_conn);
		return (DLADM_STATUS_IOERR);
	}

	do {
		char *ev_id = NULL;
		char *ev_extra = NULL;

		status = DLADM_STATUS_NOTFOUND;

		if (wpa_ctrl_pending(ctrl_conn) < 0) {
			status = DLADM_STATUS_IOERR;
			break;
		} else if (wpa_ctrl_pending(ctrl_conn) == 0) {
			continue;
		}

		if (!wpa_get_event(ctrl_conn, &ev_id, &ev_extra))
			continue;

		if (strcmp(ev_id, WPA_EVENT_SCAN_RESULTS) == 0)
			status = DLADM_STATUS_OK;

		free(ev_id);
		free(ev_extra);
	} while (status != DLADM_STATUS_OK);

	(void) wpa_ctrl_detach(ctrl_conn);

	wpa_ctrl_close(ctrl_conn);

	return (status);
}

dladm_status_t
dladm_wlan_parse_esslist(dladm_handle_t handle, datalink_id_t linkid, void *arg,
    boolean_t (*func)(void *, dladm_wlan_attr_t *))
{
	int			i;
	wl_ess_conf_t		*wlp;
	wl_ess_list_t		*wls;
	dladm_wlan_attr_t 	*wlattr;
	dladm_status_t		status;

	if (func == NULL)
		return (DLADM_STATUS_BADARG);

	if ((status = dladm_wlan_validate(handle, linkid, NULL, NULL)) !=
	    DLADM_STATUS_OK)
		return (status);

	/*
	 * WLDP_BUFSIZE can contain more than 2800 wl_ess_conf_t.
	 * We should initially use a smaller buffer. We start with 25.
	 */
	wls = calloc(25, sizeof (wl_ess_conf_t));
	if (wls == NULL)
		return (DLADM_STATUS_NOMEM);

	if ((status = dladm_wlan_get_esslist(handle, linkid, wls,
	    sizeof (wl_ess_conf_t) * 25)) != DLADM_STATUS_OK) {
		if (status == DLADM_STATUS_TOOSMALL) {
			wls = realloc(wls, WLDP_BUFSIZE);
			if (wls == NULL) {
				free(wls);
				return (DLADM_STATUS_NOMEM);
			}
			(void) memset(wls, 0, WLDP_BUFSIZE);
			if ((status = dladm_wlan_get_esslist(handle, linkid,
			    wls, WLDP_BUFSIZE)) != DLADM_STATUS_OK) {
				free(wls);
				return (status);
			}
		} else {
			free(wls);
			return (status);
		}
	}

	if (wls->wl_ess_list_num == 0) {
		free(wls);
		return (DLADM_STATUS_NOTFOUND);
	}

	wlattr = malloc(sizeof (dladm_wlan_attr_t));
	if (wlattr == NULL) {
		free(wls);
		return (DLADM_STATUS_NOMEM);
	}

	wlp = wls->wl_ess_list_ess;
	for (i = 0; i < wls->wl_ess_list_num; i++, wlp++) {
		(void) memset(wlattr, 0, sizeof (dladm_wlan_attr_t));
		fill_wlan_attr(wlp, wlattr);
		if (!func(arg, wlattr))
			break;
	}
	free(wlattr);
	free(wls);
	return (status);
}

dladm_status_t
dladm_wlan_connect(dladm_handle_t handle, datalink_id_t linkid,
    dladm_wlan_attr_t *attrp, const void *key, void *eap_attr)
{
	struct wpa_ctrl *ctrlif = NULL;
	dladm_status_t status;

	if ((status = dladm_wlan_validate(handle, linkid, &ctrlif, NULL)) !=
	    DLADM_STATUS_OK)
		return (status);

	status = wpa_network_config(ctrlif, attrp, key, eap_attr);
	if (status != DLADM_STATUS_OK) {
		wpa_ctrl_close(ctrlif);
		return (status);
	}

	if (isatty(fileno(stdout))) {
		if (wpa_ctrl_attach(ctrlif)) {
			wpa_ctrl_close(ctrlif);
			return (DLADM_STATUS_IOERR);
		}

		do {
			char *ev_id = NULL;
			char *ev_extra = NULL;

			status = DLADM_STATUS_NOTFOUND;

			if (wpa_ctrl_pending(ctrlif) < 0) {
				status = DLADM_STATUS_IOERR;
				break;
			} else if (wpa_ctrl_pending(ctrlif) == 0) {
				continue;
			}

			if (!wpa_get_event(ctrlif, &ev_id, &ev_extra))
				continue;

			if (strcmp(ev_id, WPA_EVENT_CONNECTED) == 0 ||
			    strcmp(ev_id, WPA_EVENT_DISCONNECTED) == 0 ||
			    strcmp(ev_id, WPA_EVENT_ASSOC_REJECT) == 0 ||
			    strcmp(ev_id, WPA_EVENT_TERMINATING) == 0 ||
			    strcmp(ev_id, WPA_EVENT_EAP_FAILURE) == 0 ||
			    strcmp(ev_id, WPA_EVENT_EAP_TLS_CERT_ERROR) == 0)
				status = DLADM_STATUS_OK;

			if (strcmp(ev_id, WPA_EVENT_SCAN_RESULTS) != 0 &&
			    strcmp(ev_id, WPA_EVENT_BSS_ADDED) != 0)
				(void) printf("%s\n%s\n", ev_id, ev_extra);

			free(ev_id);
			if (ev_extra != NULL)
				free(ev_extra);
		} while (status != DLADM_STATUS_OK);

		(void) wpa_ctrl_detach(ctrlif);
	}

	wpa_ctrl_close(ctrlif);

	return (status);
}

dladm_status_t
dladm_wlan_disconnect(dladm_handle_t handle, datalink_id_t linkid)
{
	dladm_status_t	status;
	wl_linkstatus_t	wl_status;
	char linkname[MAXLINKNAMELEN];

	if ((status = dladm_wlan_validate(handle, linkid, NULL, linkname)) !=
	    DLADM_STATUS_OK)
		return (status);

	if ((status = dladm_wlan_get_linkstatus(handle, linkid, &wl_status,
	    sizeof (wl_status))) != DLADM_STATUS_OK)
		return (status);

	if (wl_status) {
		/* associated */
		struct wpa_ctrl *ctrl_conn = NULL;

		if ((status = dladm_wlan_validate(handle, linkid, &ctrl_conn,
		    NULL)) == DLADM_STATUS_OK) {
			if ((status = wpa_instance_delete(ctrl_conn,
			    linkname)) == DLADM_STATUS_OK)
				return (status);
		}

		/* force disassociation in case something has failed */
		status = dladm_wlan_cmd(linkname, WL_DISASSOCIATE);
	} else {
		/* not associated */
		(void) wpa_instance_delete(NULL, linkname);
	}

	return (status);
}

dladm_status_t
dladm_wlan_get_linkattr(dladm_handle_t handle, datalink_id_t linkid,
    dladm_wlan_linkattr_t *attrp)
{
	wl_ess_conf_t		confd;
	dladm_status_t		status;

	if (attrp == NULL)
		return (DLADM_STATUS_BADARG);

	/* link status */
	if ((status = dladm_wlan_get_linkstatus(handle, linkid,
	    &attrp->la_connected, sizeof (attrp->la_connected)))
	    != DLADM_STATUS_OK)
		return (status);

	if (!attrp->la_connected)
		return (DLADM_STATUS_OK);

	(void) memset(&confd, 0, sizeof (confd));

	/* signal level */
	if (do_get_signal(handle, linkid, &confd.wl_ess_conf_sl,
	    sizeof (wl_rssi_t)))
		return (status);

	/* speed */
	if (do_get_rate(handle, linkid, &confd.wl_ess_conf_rates,
	    sizeof (wl_rates_t)))
		return (status);

	/* 802.11 mode + freq */
	if ((status = do_get_mode(handle, linkid, &confd.wl_ess_conf_phys,
	    sizeof (wl_phy_conf_t))) != DLADM_STATUS_OK)
		return (status);

	fill_wlan_attr(&confd, &attrp->la_wlan_attr);

	return (status);
}

#define	WPA_SUPPLICANT_SVC	"svc:/network/wpa_supplicant:default"
#define	CTRL_IFACE_GLOBAL 	"/var/run/wpa_supplicant-global"

/*
 * Check to see if the link is wireless.
 * If caller passes ctrl_conn, check wpa_s ctrl interface too.
 */
dladm_status_t
dladm_wlan_validate(dladm_handle_t handle, datalink_id_t linkid,
    struct wpa_ctrl **ctrl_conn, char *linkname)
{
	dladm_status_t	status = DLADM_STATUS_OK;

	uint32_t	flags;
	uint32_t	media;
	char 		ifname[MAXLINKNAMELEN];
	char		ctrlif[DLADM_STRSIZE];

	if ((status = dladm_datalink_id2info(handle, linkid, &flags, NULL,
	    &media, ifname, MAXLINKNAMELEN)) != DLADM_STATUS_OK)
		return (status);

	if (media != DL_WIFI || !(flags & DLADM_OPT_ACTIVE))
		return (DLADM_STATUS_LINKINVAL);
	if (linkname != NULL)
		(void) strlcpy(linkname, ifname, MAXLINKNAMELEN);
	if (ctrl_conn == NULL)
		return (status);
	if (wpa_get_ctrlname(ifname, ctrlif) == NULL)
		return (DLADM_STATUS_FAILED);

	/* Loops until wpa_s ctrl socket accepts commands */
	for (;;) {
		struct wpa_ctrl	*global_conn = NULL;
		struct stat statbf;
		char *ping[] = {"PING"};
		char *state = NULL;

		/*
		 * ctrl_open tries to unlink its argument if it already exists.
		 * It could be an inactive socket left by improper termination.
		 */
		if ((*ctrl_conn = wpa_ctrl_open(ctrlif)) == NULL &&
		    stat(CTRL_IFACE_GLOBAL, &statbf) == 0 &&
		    S_ISSOCK(statbf.st_mode) &&
		    (global_conn = wpa_ctrl_open(CTRL_IFACE_GLOBAL)) != NULL) {

			status = wpa_instance_create(global_conn, ifname);
			if (status == DLADM_STATUS_OK &&
			    (*ctrl_conn = wpa_ctrl_open(ctrlif)) != NULL) {
				char *no_auto_connect[] =
				    {"DISABLE_NETWORK", "all"};
				char *no_auto_reconnect[] =
				    {"STA_AUTOCONNECT", "0"};
				(void) wpa_request(*ctrl_conn, 2,
				    no_auto_connect);
				(void) wpa_request(*ctrl_conn, 2,
				    no_auto_reconnect);
			}
		}

		if (*ctrl_conn != NULL &&
		    wpa_request(*ctrl_conn, 1, ping) == 0) {
			wpa_ctrl_close(global_conn);
			return (DLADM_STATUS_OK);
		}

		if ((state = smf_get_state(WPA_SUPPLICANT_SVC)) == NULL) {
			if (isatty(fileno(stdout)))
				(void) printf("Install 'wpa_supplicant' "
				    "package!\n");
			return (DLADM_STATUS_NOTFOUND);
		}

		if (strcmp(state, SCF_STATE_STRING_ONLINE) == 0 &&
		    *ctrl_conn != NULL) {
			(void) wpa_instance_delete(*ctrl_conn, ifname);
			*ctrl_conn = NULL;
			wpa_ctrl_close(global_conn);
			(void) unlink(ctrlif);
		} else if (strcmp(state, SCF_STATE_STRING_ONLINE) == 0 &&
		    global_conn != NULL) {
			char *term[] = {"TERMINATE"};
			struct timespec ts;
			/* safest way to kill wpa_s process. smf will restart */
			(void) wpa_request(global_conn, 1, term);

			wpa_ctrl_close(global_conn);
			*ctrl_conn = NULL;
			(void) unlink(ctrlif);

			ts.tv_sec = 0;
			ts.tv_nsec = 10000000;
			(void) nanosleep(&ts, NULL);
		} else if (strcmp(state, SCF_STATE_STRING_ONLINE) == 0 &&
		    global_conn == NULL) {
			if (smf_restart_instance(WPA_SUPPLICANT_SVC) == -1)
				return (DLADM_STATUS_FAILED);
		} else if (strcmp(state, SCF_STATE_STRING_DISABLED) == 0) {
			if (smf_enable_instance(WPA_SUPPLICANT_SVC,
			    SMF_TEMPORARY) == -1)
				return (DLADM_STATUS_FAILED);
		} else if (strcmp(state, SCF_STATE_STRING_MAINT) == 0 ||
		    strcmp(state, SCF_STATE_STRING_DEGRADED) == 0) {
			if (smf_disable_instance(WPA_SUPPLICANT_SVC,
			    SMF_TEMPORARY) == -1)
				return (DLADM_STATUS_FAILED);
		} else if (strcmp(state, SCF_STATE_STRING_UNINIT) == 0 ||
		    strcmp(state, SCF_STATE_STRING_OFFLINE) == 0) {
			/* Loop until it's ready after waiting 10msec */
			struct timespec ts;
			ts.tv_sec = 0;
			ts.tv_nsec = 10000000;
			(void) nanosleep(&ts, NULL);
		} else {
			free(state);
			return (DLADM_STATUS_FAILED);
		}
		free(state);
	}
}

static boolean_t
find_val_by_name(const char *str, const val_desc_t *vdp, uint_t cnt,
    uint_t *valp)
{
	int	i;

	for (i = 0; i < cnt; i++) {
		if (strcasecmp(str, vdp[i].vd_name) == 0) {
			*valp = vdp[i].vd_val;
			return (B_TRUE);
		}
	}
	return (B_FALSE);
}

static boolean_t
find_name_by_val(uint_t val, const val_desc_t *vdp, uint_t cnt, char **strp)
{
	int	i;

	for (i = 0; i < cnt; i++) {
		if (val == vdp[i].vd_val) {
			*strp = vdp[i].vd_name;
			return (B_TRUE);
		}
	}
	return (B_FALSE);
}

/* data to string */

#define	MAC2STR(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
#define	MACSTR "%02x:%02x:%02x:%02x:%02x:%02x"

const char *
dladm_wlan_bssid2str(const uint8_t *bssid_bytes, char *bssid_str)
{
	(void) snprintf(bssid_str, 3 * DLADM_WLAN_BSSID_LEN, MACSTR,
	    MAC2STR(bssid_bytes));
	return (bssid_str);
}

static const char *
dladm_wlan_val2str(uint_t val, const val_desc_t *vdp, uint_t cnt, char *buf)
{
	char	*s;

	if (!find_name_by_val(val, vdp, cnt, &s))
		s = "";

	(void) snprintf(buf, DLADM_STRSIZE, "%s", s);
	return (buf);
}

const char *
dladm_wlan_secmode2str(dladm_wlan_secmode_t secmode, char *buf)
{
	return (dladm_wlan_val2str(secmode, secmode_vals, VALCNT(secmode_vals),
	    buf));
}

const char *
dladm_wlan_strength2str(const uint8_t strength, char *buf)
{
	(void) snprintf(buf, DLADM_STRSIZE, "%u/127", strength);
	return (buf);
}

const char *
dladm_wlan_mode2str(dladm_wlan_mode_t mode, char *buf)
{
	return (dladm_wlan_val2str(mode, mode_vals, VALCNT(mode_vals), buf));
}

/*
 * Logic Here:
 *
 * WL_RATE_1M 2
 * WL_RATE_2M 4
 * WL_RATE_5_5M 11
 * WL_RATE_6M 12
 * WL_RATE_9M 18
 * WL_RATE_11M 22
 * WL_RATE_12M 24
 * WL_RATE_18M 36
 * WL_RATE_22M 44
 * WL_RATE_24M 48
 * WL_RATE_33M 66
 * WL_RATE_36M 72
 * WL_RATE_48M 96
 * WL_RATE_54M 108
 *
 */
#define	IEEE80211_RATE	0x7f

const char *
dladm_wlan_rate2str(const uint8_t rate, char *buf)
{
	(void) snprintf(buf, DLADM_STRSIZE, "%uMb",
	    (rate & IEEE80211_RATE) / 2);
	return (buf);
}

const char *
dladm_wlan_auth2str(const uint8_t auth, char *buf)
{
	return (dladm_wlan_val2str(auth, auth_vals, VALCNT(auth_vals), buf));
}

/*
 * Convert MHz frequency to IEEE channel number.
 */
const char *
dladm_wlan_freq2channel(const uint16_t freq, char *buf)
{
	int channel;
	if (freq == 2484) {
		channel = 14;
	} else if (freq < 2484) {
		channel = ((int)freq - 2407) / 5;
	} else if (freq < 5000) {
		if (freq > 4900)
			channel = (freq - 4000) / 5;
		else
			channel = 15 + ((freq - 2512) / 20);
	} else
		channel = (freq - 5000) / 5;
	(void) sprintf(buf, "%d", channel);
	if (channel > 0)
		return (buf);
	else
		return (NULL);
}

const char *
dladm_wlan_bsstype2str(const uint8_t bsstype, char *buf)
{
	return (dladm_wlan_val2str(bsstype, bsstype_vals, VALCNT(bsstype_vals),
	    buf));
}

const char *
dladm_wlan_linkstatus2str(uint_t linkstatus, char *buf)
{
	return (dladm_wlan_val2str(linkstatus, linkstatus_vals,
	    VALCNT(linkstatus_vals), buf));
}


/* string to data */

dladm_status_t
dladm_wlan_str2bssid(const char *str, uint8_t *bssid)
{
	int	len = 0;
	uchar_t	*buf;

	if (str == NULL || bssid == NULL)
		return (DLADM_STATUS_BADARG);

	buf = _link_aton(str, &len);
	if (buf == NULL)
		return (DLADM_STATUS_BADARG);

	if (len != DLADM_WLAN_BSSID_LEN) {
		free(buf);
		return (DLADM_STATUS_BADARG);
	}

	(void) memcpy(bssid, buf, len);
	free(buf);
	return (DLADM_STATUS_OK);
}

dladm_status_t
dladm_wlan_str2secmode(const char *str, dladm_wlan_secmode_t *secmode)
{
	uint_t	val;

	if (!find_val_by_name(str, secmode_vals, VALCNT(secmode_vals), &val))
		return (DLADM_STATUS_BADARG);

	*secmode = (dladm_wlan_secmode_t)val;
	return (DLADM_STATUS_OK);
}

dladm_status_t
dladm_wlan_str2mode(const char *str, dladm_wlan_mode_t *mode)
{
	uint_t	val;

	if (!find_val_by_name(str, mode_vals, VALCNT(mode_vals), &val))
		return (DLADM_STATUS_BADARG);

	*mode = (dladm_wlan_mode_t)val;
	return (DLADM_STATUS_OK);
}

dladm_status_t
dladm_wlan_str2bsstype(const char *str, uint8_t *bsstype)
{
	uint_t	val;

	if (!find_val_by_name(str, bsstype_vals, VALCNT(bsstype_vals), &val))
		return (DLADM_STATUS_BADARG);

	*bsstype = val;
	return (DLADM_STATUS_OK);
}

/* legacy ioctl for WL_SCAN and WL_DISASSOCIATE */

dladm_status_t
dladm_wlan_cmd(const char *ifname, uint_t id)
{
	int			fd, rc;
	struct strioctl		stri;
	dladm_status_t		status = DLADM_STATUS_OK;
	wldp_t			*gbuf;
	char			linkpath[MAXLINKNAMELEN*2];

	if (ifname == NULL || id == 0 || ifname[0] == '\0')
		return (DLADM_STATUS_BADARG);

	if ((gbuf = malloc(sizeof (wldp_t))) == NULL)
		return (DLADM_STATUS_NOMEM);

	if (memset(gbuf, 0, sizeof (wldp_t)) == NULL) {
		free(gbuf);
		return (DLADM_STATUS_BADVALCNT);
	}
	(void) memset(linkpath, 0, sizeof (linkpath));
	/*
	 * dlpi_open() is not used here because libdlpi depends on libdladm,
	 * and we do not want to introduce recursive dependencies.
	 */
	rc = snprintf(linkpath, sizeof (linkpath), "/dev/net/%s", ifname);
	if (rc < 1 || rc >= sizeof (linkpath)) {
		free(gbuf);
		return (DLADM_STATUS_FAILED);
	}
	if ((fd = open(linkpath, O_RDWR)) < 0) {
		free(gbuf);
		return (dladm_errno2status(errno));
	}

	gbuf->wldp_type = 80211;
	gbuf->wldp_id	= id;
	gbuf->wldp_length = sizeof (uint32_t);

	stri.ic_timout	= 0;
	stri.ic_dp	= (char *)gbuf;
	stri.ic_cmd	= WLAN_COMMAND;
	stri.ic_len	= sizeof (wldp_t);

	if ((rc = ioctl(fd, I_STR, &stri)) != 0) {
		if (rc > 0) {
			/*
			 * Non-negative return value indicates the specific
			 * operation failed and the reason for the failure
			 * was stored in gbuf->wldp_result.
			 */
			status = dladm_wlan_wlresult2status(gbuf);
		} else {
			/*
			 * Negative return value indicates the ioctl failed.
			 */
			status = dladm_errno2status(errno);
		}
	}

	(void) close(fd);
	free(gbuf);
	return (status);
}

/* GET */
dladm_status_t
dladm_wlan_get_esslist(dladm_handle_t handle, datalink_id_t linkid, void *buf,
    size_t buflen)
{
	return (i_dladm_wlan_param(handle, linkid, buf, MAC_PROP_WL_ESS_LIST,
	    buflen, B_FALSE));
}

dladm_status_t
dladm_wlan_get_bssid(dladm_handle_t handle, datalink_id_t linkid,
    uint8_t *sta_bssid)
{
	wl_bssid_t ieee_bssid;
	dladm_status_t status;

	status = i_dladm_wlan_param(handle, linkid, &ieee_bssid,
	    MAC_PROP_WL_BSSID, sizeof (ieee_bssid), B_FALSE);

	if (status == DLADM_STATUS_OK)
		(void) memcpy(sta_bssid, ieee_bssid.wl_bssid_bssid,
		    DLADM_WLAN_BSSID_LEN);
	return (status);
}

dladm_status_t
dladm_wlan_get_essid(dladm_handle_t handle, datalink_id_t linkid,
    uint8_t *sta_essid, uint8_t *sta_essid_len)
{
	wl_essid_t ieee_essid;
	dladm_status_t status;

	status = i_dladm_wlan_param(handle, linkid, &ieee_essid,
	    MAC_PROP_WL_ESSID, sizeof (ieee_essid), B_FALSE);

	if (status == DLADM_STATUS_OK) {
		(void) memset(sta_essid, 0, DLADM_WLAN_MAX_ESSID_LEN);
		(void) memcpy(sta_essid, ieee_essid.wl_essid_essid,
		    ieee_essid.wl_essid_length);
		*sta_essid_len = ieee_essid.wl_essid_length;
	}
	return (status);
}

dladm_status_t
dladm_wlan_get_linkstatus(dladm_handle_t handle, datalink_id_t linkid,
    void *buf, size_t buflen)
{
	return (i_dladm_wlan_param(handle, linkid, buf, MAC_PROP_WL_LINKSTATUS,
	    buflen, B_FALSE));
}

static dladm_status_t
do_get_rate(dladm_handle_t handle, datalink_id_t linkid, void *buf,
    size_t buflen)
{
	return (i_dladm_wlan_param(handle, linkid, buf,
	    MAC_PROP_WL_DESIRED_RATES, buflen, B_FALSE));
}

static dladm_status_t
do_get_signal(dladm_handle_t handle, datalink_id_t linkid, void *buf,
    size_t buflen)
{
	return (i_dladm_wlan_param(handle, linkid, buf, MAC_PROP_WL_RSSI,
	    buflen, B_FALSE));
}

static dladm_status_t
do_get_mode(dladm_handle_t handle, datalink_id_t linkid, void *buf,
    size_t buflen)
{
	return (i_dladm_wlan_param(handle, linkid, buf, MAC_PROP_WL_PHY_CONFIG,
	    buflen, B_FALSE));
}

/* SET */
dladm_status_t
dladm_wlan_set_bsstype(dladm_handle_t handle, datalink_id_t linkid,
    uint8_t bsstype)
{
	wl_bss_type_t ibsstype = bsstype;

	return (i_dladm_wlan_param(handle, linkid, &ibsstype,
	    MAC_PROP_WL_BSSTYPE, sizeof (ibsstype), B_TRUE));
}

dladm_status_t
dladm_wlan_set_authmode(dladm_handle_t handle, datalink_id_t linkid,
    uint8_t authalg)
{
	wl_authmode_t auth_mode = authalg;

	return (i_dladm_wlan_param(handle, linkid, &auth_mode,
	    MAC_PROP_WL_AUTH_MODE, sizeof (auth_mode), B_TRUE));
}

static dladm_status_t
do_set_wep(dladm_handle_t handle, datalink_id_t linkid)
{
	wl_encryption_t	encryption = WL_ENC_WEP;

	return (i_dladm_wlan_param(handle, linkid, &encryption,
	    MAC_PROP_WL_ENCRYPTION, sizeof (encryption), B_TRUE));
}

#define	DLADM_WLAN_WEPKEY64_LEN		5	/* per WEP spec */
#define	DLADM_WLAN_WEPKEY128_LEN	13	/* per WEP spec */

static dladm_status_t
do_set_static_wep(dladm_handle_t handle, datalink_id_t linkid,
    const uint8_t *wep_key, size_t wep_key_len, int wep_tx_keyidx)
{
	int			i;
	dladm_status_t		status;
	wl_wep_key_tab_t	wepkey_tab;

	if ((status = do_set_wep(handle, linkid)) != DLADM_STATUS_OK)
		return (status);

	if (wep_key_len == 0 || wep_key == NULL)
		return (DLADM_STATUS_BADARG);

	(void) memset(wepkey_tab, 0, sizeof (wepkey_tab));
	for (i = 0; i < IEEE80211_KEY_MAX; i++)
		wepkey_tab[i].wl_wep_operation = WL_NUL;

	if (wep_tx_keyidx == 0 || wep_tx_keyidx > IEEE80211_KEY_MAX)
		return (DLADM_STATUS_BADARG);
	if (wep_key_len != DLADM_WLAN_WEPKEY64_LEN &&
	    wep_key_len != DLADM_WLAN_WEPKEY128_LEN)
		return (DLADM_STATUS_BADARG);
	wepkey_tab[wep_tx_keyidx - 1].wl_wep_operation = WL_ADD;
	wepkey_tab[wep_tx_keyidx - 1].wl_wep_length = wep_key_len;
	(void) memcpy(wepkey_tab[wep_tx_keyidx - 1].wl_wep_keyval, wep_key,
	    wep_key_len);

	return (i_dladm_wlan_param(handle, linkid, &wepkey_tab,
	    MAC_PROP_WL_KEY_TAB, sizeof (wepkey_tab), B_TRUE));
}

static void
generate_essid(dladm_wlan_essid_t *essid)
{
	char ssid_temp[DLADM_WLAN_MAX_ESSID_LEN];

	srandom(gethrtime());
	(void) snprintf(ssid_temp, DLADM_WLAN_MAX_ESSID_LEN, "illumos-%s",
	    random());
	/* truncate final ESSID */
	(void) memcpy(essid->we_bytes, ssid_temp, DLADM_WLAN_MAX_ESSID_LEN / 2);
	essid->we_length = DLADM_WLAN_MAX_ESSID_LEN / 2;
}

dladm_status_t
dladm_wlan_set_essid(dladm_handle_t handle, datalink_id_t linkid,
    const uint8_t *essid, uint8_t essid_len)
{
	wl_essid_t	iessid;

	(void) memset(&iessid, 0, sizeof (iessid));

	if (essid != NULL && essid_len != 0) {
		iessid.wl_essid_length = essid_len;
		(void) memcpy(iessid.wl_essid_essid, essid,
		    iessid.wl_essid_length);
	} else {
		return (DLADM_STATUS_BADARG);
	}
	return (i_dladm_wlan_param(handle, linkid, &iessid, MAC_PROP_WL_ESSID,
	    sizeof (iessid), B_TRUE));
}

dladm_status_t
dladm_wlan_set_bssid(dladm_handle_t handle, datalink_id_t linkid,
    const uint8_t *bssid)
{
	wl_bssid_t	ibssid;

	if (bssid != NULL) {
		(void) memcpy(ibssid.wl_bssid_bssid, bssid,
		    DLADM_WLAN_BSSID_LEN);
	} else {
		return (DLADM_STATUS_BADARG);
	}
	return (i_dladm_wlan_param(handle, linkid, &ibssid, MAC_PROP_WL_BSSID,
	    sizeof (ibssid), B_TRUE));
}

/*
 * this is actually used only in wpa_s driver interface when setting IBSS
 * mode channel. The user provides a channel number in this case and not
 * a frequency value
 */
dladm_status_t
dladm_wlan_set_channel(dladm_handle_t handle, datalink_id_t linkid,
    uint16_t channel)
{
	wl_phy_conf_t phy_conf;

	(void) memset(&phy_conf, 0xff, sizeof (phy_conf));
	phy_conf.wl_phy_ofdm_conf.wl_ofdm_frequency = channel;

	return (i_dladm_wlan_param(handle, linkid, &phy_conf,
	    MAC_PROP_WL_PHY_CONFIG, sizeof (phy_conf), B_TRUE));
}

dladm_status_t
dladm_wlan_createibss(dladm_handle_t handle, datalink_id_t linkid)
{
	wl_create_ibss_t cr = B_TRUE;

	return (i_dladm_wlan_param(handle, linkid, &cr, MAC_PROP_WL_CREATE_IBSS,
	    sizeof (cr), B_TRUE));
}

dladm_status_t
dladm_wlan_get_capability(dladm_handle_t handle, datalink_id_t linkid,
    void *buf, size_t buflen)
{
	return (i_dladm_wlan_param(handle, linkid, buf, MAC_PROP_WL_CAPABILITY,
	    buflen, B_FALSE));
}

/* WPA support routines */

dladm_status_t
dladm_wlan_get_wpa_ie(dladm_handle_t handle, datalink_id_t linkid, void *buf,
    size_t buflen)
{
	wl_wpa_ie_t ie;
	dladm_status_t	status;

	(void) memset(&ie, 0, sizeof(wl_wpa_ie_t));

	status = i_dladm_wlan_param(handle, linkid, &ie, MAC_PROP_WL_OPTIE,
	    sizeof (wl_wpa_ie_t), B_FALSE);
	if (status != DLADM_STATUS_OK)
		return (status);

	if (buflen >= sizeof (wl_wpa_ie_t))
		(void) memcpy(buf, &ie, sizeof (wl_wpa_ie_t));
	else
		return (DLADM_STATUS_BADARG);

	return (status);
}

dladm_status_t
dladm_wlan_set_wpa_ie(dladm_handle_t handle, datalink_id_t linkid,
    const uint8_t *wpa_ie, size_t wpa_ie_len)
{
	wl_wpa_ie_t *ie;
	dladm_status_t	status;

	if (wpa_ie_len > IEEE80211_MAX_WPA_IE)
		return (DLADM_STATUS_BADARG);
	ie = malloc(sizeof (wl_wpa_ie_t));
	if (ie == NULL)
		return (DLADM_STATUS_NOMEM);

	(void) memset(ie, 0, sizeof (wl_wpa_ie_t));
	ie->wpa_ie_len = wpa_ie_len;
	(void) memcpy(ie->wpa_ie, wpa_ie, wpa_ie_len);

	status = i_dladm_wlan_param(handle, linkid, ie, MAC_PROP_WL_OPTIE,
	    sizeof (wl_wpa_ie_t), B_TRUE);
	free(ie);

	return (status);
}

dladm_status_t
dladm_wlan_set_wpa(dladm_handle_t handle, datalink_id_t linkid)
{
	wl_wpa_t wpa = B_TRUE;

	return (i_dladm_wlan_param(handle, linkid, &wpa, MAC_PROP_WL_WPA,
	    sizeof (wpa), B_TRUE));
}

dladm_status_t
dladm_wlan_set_counterm(dladm_handle_t handle, datalink_id_t linkid,
    boolean_t flag)
{
	boolean_t counterm = flag;

	return (i_dladm_wlan_param(handle, linkid, &counterm,
	    MAC_PROP_WL_COUNTERM, sizeof (wl_counterm_t), B_TRUE));
}

/*
 * set_key - Configure encryption key
 * @alg: encryption algorithm (%WPA_ALG_NONE, %WPA_ALG_WEP,
 *	%WPA_ALG_TKIP, %WPA_ALG_CCMP, %WPA_ALG_IGTK, %WPA_ALG_PMK);
 *	%WPA_ALG_NONE clears the key.
 * @addr: Address of the peer STA (BSSID of the current AP when setting
 *	pairwise key in station mode), ff:ff:ff:ff:ff:ff for
 *	broadcast keys, %NULL for default keys that are used both for
 *	broadcast and unicast; when clearing keys, %NULL is used to
 *	indicate that both the broadcast-only and default key of the
 *	specified key index is to be cleared
 * @key_idx: key index (0..3), usually 0 for unicast keys; 0..4095 for
 *	IGTK
 * @set_tx: configure this key as the default Tx key (only used when
 *	driver does not support separate unicast/individual key
 * @seq: sequence number/packet number, seq_len octets, the next
 *	packet number to be used for in replay protection; configured
 *	for Rx keys (in most cases, this is only used with broadcast
 *	keys and set to zero for unicast keys); %NULL if not set
 * @seq_len: length of the seq, depends on the algorithm:
 *	TKIP: 6 octets, CCMP: 6 octets, IGTK: 6 octets
 * @key: key buffer; TKIP: 16-byte temporal key, 8-byte Tx Mic key,
 *	8-byte Rx Mic Key
 * @key_len: length of the key buffer in octets (WEP: 5 or 13,
 *	TKIP: 32, CCMP: 16, IGTK: 16)
 *
 * Returns: 0 on success, -1 on failure
 *
 * Configure the given key for the kernel driver. If the driver
 * supports separate individual keys (4 default keys + 1 individual),
 * addr can be used to determine whether the key is default or
 * individual. If only 4 keys are supported, the default key with key
 * index 0 is used as the individual key. STA must be configured to use
 * it as the default Tx key (set_tx is set) and accept Rx for all the
 * key indexes. In most cases, WPA uses only key indexes 1 and 2 for
 * broadcast keys, so key index 0 is available for this kind of
 * configuration.
 *
 * Please note that TKIP keys include separate TX and RX MIC keys and
 * some drivers may expect them in different order than wpa_supplicant
 * is using. If the TX/RX keys are swapped, all TKIP encrypted packets
 * will trigger Michael MIC errors. This can be fixed by changing the
 * order of MIC keys by swapping te bytes 16..23 and 24..31 of the key
 * in driver_*.c set_key() implementation, see driver_ndis.c for an
 * example on how this can be done.
 *
 * Check also bsd_set_key.c in driver_bsd.c in wpa_s tree
 */
dladm_status_t
dladm_wlan_set_key(dladm_handle_t handle, datalink_id_t linkid,
    uint_t alg, const uint8_t *addr, int key_idx, int set_tx,
    const uint8_t *seq, size_t seq_len, const uint8_t *key, size_t key_len)
{
	wl_key_t	wks;
	wl_del_key_t	wkd;

	(void) memset(&wks, 0, sizeof (wl_key_t));

	switch (alg) {
	case WPA_ALG_NONE:
		(void) memset(&wkd, 0, sizeof (wl_del_key_t));
		wkd.idk_keyix = key_idx;
		if (addr != NULL)
			(void) memcpy(wkd.idk_macaddr, addr,
			    DLADM_WLAN_BSSID_LEN);
		return (i_dladm_wlan_param(handle, linkid, &wkd,
		    MAC_PROP_WL_DELKEY, sizeof (wkd), B_TRUE));
	case WPA_ALG_TKIP:
		wks.ik_type = IEEE80211_CIPHER_TKIP;
		break;
	case WPA_ALG_CCMP:
		wks.ik_type = IEEE80211_CIPHER_AES_CCM;
		break;
	case WPA_ALG_WEP:
		return (do_set_static_wep(handle, linkid, key, key_len,
		    key_idx));
	case WPA_ALG_IGTK:
	case WPA_ALG_PMK:
	default:
		return (DLADM_STATUS_NOTSUP);
	}

	/* keylen + keydata */
	if (key_len > sizeof (wks.ik_keydata) || key == NULL)
		return (DLADM_STATUS_BADARG);

	wks.ik_keylen = key_len;
	(void) memcpy(wks.ik_keydata, key, key_len);

	/* rx seq */
	if (seq != NULL) {
		if (seq_len > sizeof (wks.ik_keyrsc))
			return (DLADM_STATUS_BADARG);
#ifdef _BIG_ENDIAN
		/*
		 * wk.ik_keyrsc is in host byte order (big endian), need to
		 * swap it to match with the byte order used in WPA.
		 */
		{
			int i;
			uint8_t *keyrsc = (uint8_t *)&wks.ik_keyrsc;
			for (i = 0; i < seq_len; i++)
				keyrsc[sizeof (wks.ik_keyrsc) - i - 1] = seq[i];
		}
#else /* _BIG_ENDIAN */
		(void) memcpy(&wks.ik_keyrsc, seq, seq_len);
#endif /* _BIG_ENDIAN */
	}

	/* flags */
	wks.ik_flags = IEEE80211_KEY_RECV;

	/* tx key */
	if (set_tx)
		wks.ik_flags |= IEEE80211_KEY_XMIT;

	/* key_idx */
	if (key_idx > sizeof (wks.ik_keyix))
		return (DLADM_STATUS_BADARG);

	wks.ik_keyix = key_idx;

	if (addr == NULL) {
		(void) memset(wks.ik_macaddr, 0xff, DLADM_WLAN_BSSID_LEN);
	} else {
		/*
		 * Deduce whether group/global or unicast key by checking
		 * the address (yech).  Note also that we can only mark global
		 * keys default; doing this for a unicast key is an error.
		 */
		if ((addr[0] & addr[1] & addr[2] & addr[3] & addr[4] & addr[5])
		    == 0xff) {
			wks.ik_flags |= IEEE80211_KEY_GROUP;
			wks.ik_keyix = key_idx;
			(void) memset(wks.ik_macaddr, 0xff,
			    DLADM_WLAN_BSSID_LEN);
		} else {
			(void) memcpy(wks.ik_macaddr, addr,
			    DLADM_WLAN_BSSID_LEN);
			if (key_idx == 0) {
				wks.ik_keyix = IEEE80211_KEYIX_NONE;
				wks.ik_flags |= IEEE80211_KEY_DEFAULT;
			} else
				wks.ik_keyix = key_idx;
		}
	}

	return (i_dladm_wlan_param(handle, linkid, &wks, MAC_PROP_WL_KEY,
	    sizeof (wks), B_TRUE));
}

dladm_status_t
dladm_wlan_set_mlme(dladm_handle_t handle, datalink_id_t linkid,
    boolean_t op, int reason, const uint8_t *bssid)
{
	wl_mlme_t mlme;

	if (bssid == NULL)
		return (DLADM_STATUS_BADARG);

	(void) memset(&mlme, 0, sizeof (wl_mlme_t));

	mlme.im_op = op ? IEEE80211_MLME_ASSOC : IEEE80211_MLME_DISASSOC;
	mlme.im_reason = reason;
	(void) memcpy(mlme.im_macaddr, bssid, DLADM_WLAN_BSSID_LEN);

	return (i_dladm_wlan_param(handle, linkid, &mlme, MAC_PROP_WL_MLME,
	    sizeof (mlme), B_TRUE));
}

/*
 * Opens a control interface connection to wpa_supplicant
 * global interface ctrl_path, /var/run/wpa_supplicant-global. This path
 * is configured in network/wpa_supplicant service manifest.
 * network/wpa_supplicant service must be running.
 */
static dladm_status_t
wpa_instance_create(struct wpa_ctrl *ctrl_global, char *ifname)
{
	int res;
	boolean_t nwam;

	char interface_add[DLADM_STRSIZE];
	char *interface_add_cmd[1];
	char *state;

	if (ctrl_global == NULL || ifname == NULL)
		return (DLADM_STATUS_BADARG);

	interface_add_cmd[0] = interface_add;

	state = smf_get_state(NWAM_FMRI);
	nwam = (strcmp(state, SCF_STATE_STRING_DISABLED) != 0);
	free(state);

	res = snprintf(interface_add, sizeof (interface_add),
	    "INTERFACE_ADD %s\t%s\t%s\t%s",
	    ifname, nwam ? ifname : "", "solaris", CTRL_IFACE_DIR);

	if (res <= 0 || res >= sizeof (interface_add))
		return (DLADM_STATUS_FAILED);

	/* wpa_s now executes synchronously wpa_driver_solaris_init */
	if (wpa_request(ctrl_global, 1, interface_add_cmd))
		return (DLADM_STATUS_IOERR);

	return (DLADM_STATUS_OK);
}

static dladm_status_t
wpa_instance_delete(struct wpa_ctrl *ctrl_conn, char *ifname)
{
	dladm_status_t status = DLADM_STATUS_OK;
	struct stat statbuf;

	if (ifname == NULL || ifname[0] == '\0') {
		if (ctrl_conn != NULL)
			wpa_ctrl_close(ctrl_conn);
		return (DLADM_STATUS_BADARG);
	}

	if (ctrl_conn != NULL) {
		char *disable_network[] = {"DISABLE_NETWORK", "all"};
		char *remove_network[] = {"REMOVE_NETWORK", "0"};
		/*
		 * disable network causes driver to send disassoc frame
		 * and to load default values
		 */
		if (wpa_request(ctrl_conn, 2, disable_network))
			status = DLADM_STATUS_IOERR;
		(void) wpa_request(ctrl_conn, 2, remove_network);
		wpa_ctrl_close(ctrl_conn);
	}

	if (stat(CTRL_IFACE_GLOBAL, &statbuf) == 0 &&
	    S_ISSOCK(statbuf.st_mode)) {

		struct wpa_ctrl *ctrl_global = NULL;
		char *state = smf_get_state(NWAM_FMRI);

		if (state != NULL && strcmp(state,
		    SCF_STATE_STRING_DISABLED) == 0 &&
		    (ctrl_global = wpa_ctrl_open(CTRL_IFACE_GLOBAL)) != NULL) {

			char *interface_remove[] = {"INTERFACE_REMOVE", NULL};
			if (ctrl_conn != NULL) {
				/*
				 * This prevents disassoc event from being
				 * received when interface is being removed
				 */
				struct timespec ts;
				ts.tv_sec = 0;
				ts.tv_nsec = 100000000;
				(void) nanosleep(&ts, NULL);
			}
			/*
			 * remove wpa_s instance interface and
			 * deinit device driver interface
			 */
			interface_remove[1] = ifname;
			if (wpa_request(ctrl_global, 2, interface_remove))
				status = DLADM_STATUS_IOERR;
			wpa_ctrl_close(ctrl_global);
		}
		if (state != NULL)
			free(state);
		if (ctrl_global == NULL)
			status = DLADM_STATUS_DENIED;
	} else {
		status = DLADM_STATUS_DENIED;
	}

	return (status);
}

static dladm_status_t wpa_network_config(struct wpa_ctrl *ctrl_conn,
    dladm_wlan_attr_t *attrp, const dladm_wlan_key_t *key,
    dladm_wlan_eap_t *eapp)
{
	int i = 0;
	dladm_status_t status = DLADM_STATUS_OK;

	char *set_network[] = {"SET_NETWORK", "0", NULL, NULL};
	char *common_props[] = {"mode", "auth_alg", "key_mgmt"};
	char *common_vals[] = {"0", "OPEN", "NONE"};

	char *add_network[] = {"ADD_NETWORK"};
	char *remove_network[] = {"REMOVE_NETWORK", "0"};

	if (ctrl_conn == NULL)
		return (DLADM_STATUS_BADARG);

	(void) wpa_request(ctrl_conn, 2, remove_network);

	/* add empty wifi network block */
	if (wpa_request(ctrl_conn, 1, add_network))
		status = DLADM_STATUS_BADVAL;

	/* attrp can be NULL */
	if (attrp == NULL)
		goto enable;

	if ((attrp->wa_valid & DLADM_WLAN_ATTR_SECMODE) &&
	    attrp->wa_secmode != DLADM_WLAN_SECMODE_NONE && key == NULL)
		status = DLADM_STATUS_BADVAL;

	if ((attrp->wa_valid & DLADM_WLAN_ATTR_SECMODE) &&
	    attrp->wa_secmode == DLADM_WLAN_SECMODE_EAP && eapp == NULL)
		status = DLADM_STATUS_BADVAL;

	/* bssid */
	/* we always set bssid if present */
	if (attrp->wa_valid & DLADM_WLAN_ATTR_BSSID) {
		char ap_bssid[DLADM_WLAN_BSSID_LEN * 3];
		char *cmd_bssid[] = { "bssid", "0", NULL };
		cmd_bssid[2] = ap_bssid;
		(void) dladm_wlan_bssid2str(attrp->wa_bssid.wb_bytes, ap_bssid);
		if (wpa_request(ctrl_conn, 3, cmd_bssid))
			status = DLADM_STATUS_BADVAL;
	}

	/* Ad-Hoc */
	if ((attrp->wa_valid & DLADM_WLAN_ATTR_BSSTYPE) &&
	    (attrp->wa_bsstype == IEEE80211_MODE_IBSS)) {
		/*
		 * TODO:
		 * add frequency to dladm connect-wifi
		 * when adhoc mode is supported
		 */
		set_network[2] = "frequency";
		set_network[3] = "1";
		common_vals[0] = "1";
		if (wpa_request(ctrl_conn, 4, set_network))
			status = DLADM_STATUS_BADVAL;
		if ((!(attrp->wa_valid & DLADM_WLAN_ATTR_ESSID))) {
			generate_essid(&attrp->wa_essid);
			attrp->wa_valid |= DLADM_WLAN_ATTR_ESSID;
		}
		if (attrp->wa_secmode == DLADM_WLAN_SECMODE_PSK)  {
			char *secure_ibss_props[] = {"proto", "pairwise",
			    "group"};
			char *secure_ibss_vals[] = {"WPA", "NONE", "TKIP"};
			common_vals[2] = "WPA-NONE";
			/*
			 * wpa_supplicant.conf specify this is the only
			 * supported configuration mode if cleartext is not used
			 *
			 * TODO:
			 * when adhoc mode is supported we should check that
			 * the net80211 or the device driver supports this
			 * security mode
			 */
			for (i = 0; i < 3; i++) {
				set_network[2] = secure_ibss_props[i];
				set_network[3] = secure_ibss_vals[i];
				if (wpa_request(ctrl_conn, 4, set_network))
					status = DLADM_STATUS_BADVAL;
			}
		}
	}

	/* Essid */
	if (attrp->wa_valid & DLADM_WLAN_ATTR_ESSID) {
		set_network[2] = "ssid";
		/* ssid string needs to be quoted */
		if (asprintf(&set_network[3], "\"%s\"",
		    attrp->wa_essid.we_bytes) == -1)
			status = DLADM_STATUS_BADVAL;
		if (wpa_request(ctrl_conn, 4, set_network))
			status = DLADM_STATUS_BADVAL;
		/* required for nwam connection report */
		set_network[2] = "id_str";
		if (wpa_request(ctrl_conn, 4, set_network))
			status = DLADM_STATUS_BADVAL;
		free(set_network[3]);
	} else {
		/* Essid is mandatory for non-plaintext APs */
		if (!(attrp->wa_valid & DLADM_WLAN_ATTR_SECMODE) ||
		    attrp->wa_secmode == DLADM_WLAN_SECMODE_NONE)
			goto enable;
	}

	switch (attrp->wa_secmode) {
	case DLADM_WLAN_SECMODE_WEP:
		common_vals[1] = "SHARED";
		if (asprintf(&set_network[2], "wep_key%c", key->wk_idx) == -1)
			status = DLADM_STATUS_BADVAL;
		set_network[3] = (char *)key->wk_val;
		if (wpa_request(ctrl_conn, 4, set_network))
			status = DLADM_STATUS_BADVAL;
		free(set_network[2]);
		set_network[2] = "wep_tx_keyidx";
		if (asprintf(&set_network[3], "%c", key->wk_idx) == -1)
			status = DLADM_STATUS_BADVAL;
		if (wpa_request(ctrl_conn, 4, set_network))
			status = DLADM_STATUS_BADVAL;
		free(set_network[3]);
		break;
	case DLADM_WLAN_SECMODE_PSK:
		common_vals[2] = "WPA-PSK";
		set_network[2] = "psk";
		set_network[3] = (char *)key->wk_val;
		if (wpa_request(ctrl_conn, 4, set_network))
			status = DLADM_STATUS_BADVAL;
		break;
	case DLADM_WLAN_SECMODE_EAP:
	/*
	 * TODO: when we add PCKS#11 cert. reference support
	 * move to a separate function  and fix checks for each eap_valid flags
	 */
		{
		int eap_param = 2;
		char *eap_props[] = {"eap", "identity", "ca_path", "ca_cert"};
		char *eap_vals[] = {NULL, NULL, "\"/etc/certs/CA\"", NULL };

		eap_vals[1] = eapp->eap_user;
		eap_vals[3] = eapp->eap_ca_cert;

		if (eapp->eap_valid & DLADM_EAP_ATTR_CACERT)
			eap_param += 2;

		common_vals[2] = "WPA-EAP";

		/* < TLS > */
		if (key->wk_class == DLADM_SECOBJ_CLASS_TLS) {
			int clicrt = (eapp->eap_valid & DLADM_EAP_ATTR_CLICERT)
			    ? 4 : 3;
			char *tls_props[] = {"engine", NULL, NULL,
			    "client_cert"};
			char *tls_vals[] = {"0", NULL, NULL, NULL};

			tls_vals[3] = eapp->eap_cli_cert;
			eap_vals[0] = "TLS";
			/*
			 * FIX ME when we can retrieve crt using engine
			 * the "client_cert" string in tls_props must be
			 * replaced with "cert_id"
			 * The same is for ca_cert -> "ca_cert_id"
			 * Both values are the key_data->wk_name string
			 */
			if (key->wk_engine) {
				tls_props[1] = "engine_id";
				tls_props[2] = "key_id";
				tls_vals[0] = "1";
				tls_vals[1] = "\"pkcs11\"";
				if (asprintf(&tls_vals[2],
				    "\"pkcs11:object=%s;passphrasedialog=exec:"
				    "%s\"", key->wk_name, PIN_FILE) <= 0)
					status = DLADM_STATUS_BADVAL;
			} else {
				tls_props[1] = "private_key";
				tls_props[2] = "private_key_passwd";
				tls_vals[1] = eapp->eap_priv;
				tls_vals[2] = (char *)key->wk_val;
			}
			for (i = 0; i < clicrt; i++) {
				set_network[2] = tls_props[i];
				set_network[3] = tls_vals[i];
				if (wpa_request(ctrl_conn, 4, set_network))
					status = DLADM_STATUS_BADVAL;
			}
			if (key->wk_engine)
				free(tls_vals[2]);
		} else {
			char *tunnel_props[] = {"password", "phase2",
			    "anonymous_identity"};
			char *tunnel_vals[3] = {NULL, NULL, NULL};
			char inauthtyp[DLADM_STRSIZE];
			int f = (eapp->eap_valid & DLADM_EAP_ATTR_ANON) ? 3 : 2;

			tunnel_vals[0] = (char *)key->wk_val;
			tunnel_vals[1] = inauthtyp;
			tunnel_vals[2] = eapp->eap_anon;

			(void) memset(inauthtyp, 0, sizeof (inauthtyp));

			if (key->wk_class == DLADM_SECOBJ_CLASS_TTLS) {
				if (dladm_p2ttls_2_str(key->wk_p2mask,
				    inauthtyp, B_FALSE) < 1)
					status = DLADM_STATUS_BADVAL;
				eap_vals[0] = "TTLS";
			} else if (key->wk_class == DLADM_SECOBJ_CLASS_PEAP) {
				if (dladm_p2peap_2_str(key->wk_p2mask,
				    inauthtyp, B_FALSE) < 1)
					status = DLADM_STATUS_BADVAL;
				eap_vals[0] = "PEAP";
			}
			for (i = 0; i < f; i++) {
				set_network[2] = tunnel_props[i];
				set_network[3] = tunnel_vals[i];
				if (wpa_request(ctrl_conn, 4, set_network))
					status = DLADM_STATUS_BADVAL;
			}
		}
		for (i = 0; i < eap_param; i++) {
			set_network[2] = eap_props[i];
			set_network[3] = eap_vals[i];
			if (wpa_request(ctrl_conn, 4, set_network))
				status = DLADM_STATUS_BADVAL;
		}
		}
		break;
	case DLADM_WLAN_SECMODE_NONE:
		break;
	default:
		status = DLADM_STATUS_BADVAL;
		break;
	}

enable:
	for (i = 0; i < 3; i++) {
		set_network[2] = common_props[i];
		set_network[3] = common_vals[i];
		if (wpa_request(ctrl_conn, 4, set_network))
			status = DLADM_STATUS_BADVAL;
	}

	/* disable other networks and enable network with id 0 */
	if (status == DLADM_STATUS_OK) {
		char *select_network[] = {"SELECT_NETWORK", "0"};
		if (wpa_request(ctrl_conn, 2, select_network))
			status = DLADM_STATUS_BADVAL;
	} else {
		(void) wpa_request(ctrl_conn, 2, remove_network);
	}

	/*
	 * the wpa_s driver interface starts interacting with net80211 module
	 * and requests a network scan
	 */

	return (status);
}

boolean_t
wpa_get_event(struct wpa_ctrl *ctrlif, char **ev_type, char **ev_info)
{
	char evdata[DLADM_STRSIZE];
	size_t evlen = sizeof (evdata) - 1;
	size_t evtype_len;
	char *pos = evdata, *pos2;

	if (ctrlif == NULL || *ev_type != NULL || *ev_info != NULL)
		return (B_FALSE);

	(void) memset(evdata, 0, sizeof (evdata));

	if (wpa_ctrl_recv(ctrlif, evdata, &evlen))
		return (B_FALSE);

	if (evlen >= sizeof (evdata))
		return (B_FALSE);

	evdata[evlen] = '\0';

	if (*pos == '<') {
		/* skip priority */
		pos++;
		pos = memchr(pos, '>', evlen);
		if (pos)
			pos++;
		else
			return (B_FALSE);
	}

	if (evlen < 14 || strncmp(pos, "CTRL-EVENT-", 11) != 0)
		return (B_FALSE);

	pos2 = memchr(pos, ' ', evlen - (pos - evdata));
	if (pos2)
		pos2++;
	else
		return (B_FALSE);


	evtype_len = pos2 - pos;

	*ev_type = strndup(pos, evtype_len);
	if (*ev_type == NULL)
		return (B_FALSE);

	if (*pos2 != '\0')
		*ev_info = strndup(pos2, evlen - evtype_len - (pos - evdata));

	return (B_TRUE);
}

char *
wpa_get_ctrlname(const char *ifname, char *ctrl_path)
{
	int flen, namelen, res;

	if (ctrl_path == NULL || ifname == NULL)
		return (NULL);

	namelen = strnlen(ifname, MAXLINELEN);
	if (namelen <= 0 || namelen >= MAXLINELEN)
		return (NULL);

	flen = strlen(CTRL_IFACE_DIR) + namelen + 2;

	res = snprintf(ctrl_path, flen, "%s/%s", CTRL_IFACE_DIR, ifname);
	if (res <= 0 || res >= flen)
		return (NULL);

	return (ctrl_path);
}

/*
 * msg_cb can be used to register a callback function that will be called for
 * unsolicited messages received while waiting for the command response.
 */
static void
wpa_msg_cb(char *msg, size_t len)
{
	if (isatty(fileno(stdout)) && len != 0)
		(void) printf("%s", msg);
}

static int
wpa_ctrl_command(struct wpa_ctrl *ctrl, char *cmd, size_t cmdlen)
{
	char *replybuf;
	size_t len;
	int ret;
	boolean_t verbose;

	/* with these two commands we need to print reply msg to stdout */
	verbose = (strncmp(cmd, "STATUS VERBOSE", cmdlen) == 0 ||
	    strncmp(cmd, "MIB", cmdlen) == 0);

	len = verbose ? 2047 : 31;

	replybuf = calloc(1, len + 1);
	if (replybuf == NULL)
		return (-1);

	ret = wpa_ctrl_request(ctrl, cmd, cmdlen, replybuf, &len, wpa_msg_cb);

	/* remove verbose condition to enable debugging */
	if (verbose && isatty(fileno(stdout)) && len != 0)
		(void) printf("%s", replybuf);

	free(replybuf);

	return (ret);
}

int
wpa_request(struct wpa_ctrl *ctrl, int argc, char *argv[])
{
	char *cmdbuf;
	size_t cmdlen = 0;
	size_t bufsize = 0;
	int ret = 0;
	int i;

	if (ctrl == NULL || argc < 1 || argc > 4 || argv == NULL)
		return (-1);

	for (i = 0; i < argc; i++) {
		size_t len;
		if (argv[i] == NULL)
			return (-1);
		len = strnlen(argv[i], DLADM_STRSIZE);
		if (len > 0 && len < DLADM_STRSIZE)
			bufsize += len;
		else
			return (-1);
	}

	bufsize += 8;
	cmdbuf = calloc(1, bufsize);
	if (cmdbuf == NULL)
		return (-1);

	for (i = 0; i < argc; i++) {
		cmdlen = strlcat(cmdbuf, argv[i], bufsize);
		if (cmdlen == 0 && cmdlen >= bufsize) {
			free(cmdbuf);
			return (-1);
		}
		if (i < (argc - 1))
			cmdbuf[cmdlen] = ' ';
	}

	ret = wpa_ctrl_command(ctrl, cmdbuf, cmdlen);
	free(cmdbuf);
	return (ret);
}
