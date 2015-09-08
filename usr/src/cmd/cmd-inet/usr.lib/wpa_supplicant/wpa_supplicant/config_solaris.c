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
 * Copyright 2012 Enrico Papi <enricop@computer.org>.  All rights reserved.
 */

/*
 * This file implements a configuration backend for nwam. The
 * configuration information is read from known_wlan.conf text file.
 */

#include "includes.h"
#include "common.h"
#include "config.h"

#include <libnwam.h>

#ifdef CONFIG_BACKEND_SOLARIS

void wpa_config_set_illumos_ssid_prefs(struct wpa_ssid *ssid) {
	wpa_config_set(ssid, "priority", "1", 0);
	wpa_config_set(ssid, "key_mgmt", "NONE", 0);
}

void wpa_config_set_illumos_global_pref(struct wpa_config *config) {

/*
 * config->sta_autoconnect = 0;
 * Add command sta_autoconnect for disabling automatic reconnection
 * on receiving disconnection event.
 * config->sta_autoconnect <0/1> = disable/enable automatic reconnection
 *
 * unsigned int wpa_config::bss_expiration_age
 * BSS entry age after which it can be expired.
 * This value controls the time in seconds after which a BSS entry gets removed
 * if it has not been updated or is not in use.
 *
 * unsigned int wpa_config::bss_expiration_scan_count
 * Expire BSS after number of scans.
 * If the BSS entry has not been seen in this many scans, it will be removed.
 * A value of 1 means that entry is removed after the first scan in which the
 * BSSID is not seen. Larger values can be used to avoid BSS entries
 * disappearing if they are not visible in every scan
 * (e.g., low signal quality or interference).
 */
	config->update_config = 0;
/*
 * Is wpa_supplicant allowed to update configuration.
 * This variable control whether wpa_supplicant is allow to re-write its
 * configuration with wpa_config_write(). If this is zero, configuration data is
 * only changed in memory and the external data is not overriden.
 * If this is non-zero, wpa_supplicant will update the configuration data
 * (e.g., a file) whenever configuration is changed. This update may replace the
 * old configuration which can remove comments from it in case of a text file
 * configuration.
 */
}

static int
wpa_config_read_prop(const char *propname, nwam_value_t propval, void *arg)
{
	struct wpa_ssid *ssid = arg;
	nwam_error_t err = NWAM_SUCCESS;
	dladm_status_t status = DLADM_STATUS_OK;
	char *chrbuf;

	if (propname == NULL || ssid == NULL)
		return (NWAM_INVALID_ARG);

	if (strcmp(propname, NWAM_KNOWN_WLAN_PROP_SSID) == 0) {
		err = nwam_value_get_string(propval, &chrbuf);
		if (err != NWAM_SUCCESS)
			goto perror;
		if (wpa_config_set_quoted(ssid, "id_str", chrbuf))
			goto perror;
		if (wpa_config_set_quoted(ssid, propname, chrbuf))
			goto perror;
	} else if (strcmp(propname, NWAM_KNOWN_WLAN_PROP_KEYNAME) == 0) {
		dladm_handle_t handle;
		dladm_wlan_key_t key_data;
		err = nwam_value_get_string(propval, &chrbuf);
		if (err != NWAM_SUCCESS)
			goto perror;
		strlcpy(key_data.wk_name, chrbuf, DLADM_SECOBJ_NAME_MAX);
		if (dladm_open(&handle) != DLADM_STATUS_OK) {
			wpa_printf(MSG_ERROR, "%s: FAILED to open dladm handle",
			    __func__);
			goto perror;
		}
		status = dladm_get_secobj(handle, &key_data,
		    DLADM_OPT_PERSIST | DLADM_OPT_ACTIVE);
		if (status != DLADM_STATUS_OK) {
			char errbuf[DLADM_STRSIZE];
			wpa_printf(MSG_ERROR, "%s: FAILED to get secobj '%s'"
			    ". error: %s", __func__, chrbuf,
			    dladm_status2str(status, errbuf));
			dladm_close(handle);
			goto perror;
		}
		dladm_close(handle);
		switch (key_data.wk_class) {
		case DLADM_SECOBJ_CLASS_WEP:
			{
			char wep_keystr[10];
			strcpy(wep_keystr, "wep_key");
			strlcat(wep_keystr, &key_data.wk_idx, 9);
			if (wpa_config_set(ssid, wep_keystr,
			    (char *)key_data.wk_val, 0))
				goto perror;
			if (wpa_config_set(ssid, "wep_tx_keyidx",
			    &key_data.wk_idx, 0))
				goto perror;
			if (wpa_config_set(ssid, "auth_alg", "SHARED", 0))
				goto perror;
			}
			break;
		case DLADM_SECOBJ_CLASS_PSK:
			if (wpa_config_set(ssid, "key_mgmt", "WPA-PSK", 0))
				goto perror;
			if (wpa_config_set(ssid, "psk", (char *)key_data.wk_val,
			    0))
				goto perror;
			break;
		case DLADM_SECOBJ_CLASS_TLS:
			{
			char *engine_keyid;
			if (wpa_config_set(ssid, "key_mgmt", "WPA-EAP", 0))
				goto perror;
			if (wpa_config_set(ssid, "eap", "TLS", 0))
				goto perror;
			if (wpa_config_set(ssid, "private_key_passwd",
			    (char *)key_data.wk_val, 0))
				goto perror;
			if (asprintf(&engine_keyid,
			    "\"pkcs11:object=%s;passphrasedialog=exec:"
			    "%s\"", key_data.wk_name, PIN_FILE) <= 0)
				goto perror;
			if (wpa_config_set(ssid, "key_id", engine_keyid, 0))
				goto perror;
			free(engine_keyid);
			if (wpa_config_set_quoted(ssid, "cert_id",
			    key_data.wk_name))
				goto perror;
			}
			break;
		case DLADM_SECOBJ_CLASS_TTLS:
			{
			char phase2str[DLADM_STRSIZE];
			memset(phase2str, 0, sizeof (phase2str));
			if (dladm_p2ttls_2_str(key_data.wk_p2mask, phase2str,
			    B_FALSE) < 1)
				goto perror;
			if (wpa_config_set(ssid, "phase2", phase2str, 0))
				goto perror;
			if (wpa_config_set(ssid, "key_mgmt", "WPA-EAP", 0))
				goto perror;
			if (wpa_config_set(ssid, "eap", "TTLS", 0))
				goto perror;
			if (wpa_config_set(ssid, "password",
			    (char*)key_data.wk_val, 0))
				goto perror;
			}
			break;
		case DLADM_SECOBJ_CLASS_PEAP:
			{
			char phase2str[DLADM_STRSIZE];
			memset(phase2str, 0, sizeof (phase2str));
			if (dladm_p2peap_2_str(key_data.wk_p2mask, phase2str,
			    B_FALSE) < 1)
				goto perror;
			if (wpa_config_set(ssid, "phase2", phase2str, 0))
				goto perror;
			if (wpa_config_set(ssid, "key_mgmt", "WPA-EAP", 0))
				goto perror;
			if (wpa_config_set(ssid, "eap", "PEAP", 0))
				goto perror;
			if (wpa_config_set(ssid, "password",
			    (char*)key_data.wk_val, 0))
				goto perror;
			}
			break;
		default:
			goto perror;
		}
	} else if (strcmp(propname, NWAM_KNOWN_WLAN_PROP_PRIORITY) == 0) {
		uint64_t decbuf;
		err = nwam_value_get_uint64(propval, &decbuf);
		if (err != NWAM_SUCCESS)
			goto perror;
		if (asprintf(&chrbuf, "%u", decbuf) <= 0)
			goto perror;
		if (wpa_config_set(ssid, propname, chrbuf, 0)) {
			free(chrbuf);
			goto perror;
		}
		free(chrbuf);
	} else if (strcmp(propname, NWAM_KNOWN_WLAN_PROP_DISABLED) == 0) {
		boolean_t disabled = B_FALSE;
		err = nwam_value_get_boolean(propval, &disabled);
		if (err != NWAM_SUCCESS)
			goto perror;
		if (wpa_config_set(ssid, propname, disabled ? "1" : "0", 0))
			goto perror;
	} else {
		/*
		 * case	NWAM_KNOWN_WLAN_PROP_BSSID
		 * case NWAM_KNOWN_WLAN_PROP_EAP_USER:
		 * case NWAM_KNOWN_WLAN_PROP_EAP_ANON:
		 * case NWAM_KNOWN_WLAN_PROP_CA_CERT:
		 * case NWAM_KNOWN_WLAN_PROP_PRIV:
		 * case NWAM_KNOWN_WLAN_PROP_CLI_CERT
		 */
		err = nwam_value_get_string(propval, &chrbuf);
		if (err != NWAM_SUCCESS)
			goto perror;
		if (wpa_config_set(ssid, propname, chrbuf, 0))
			goto perror;
	}

	return (0);

perror:
	wpa_printf(MSG_ERROR, "%s: failed to get property '%s' for known wlan"
	    " '%d' error: %d status: %d", __func__, propname, ssid->id, err,
	    status);

	return (err ? err : -1);
}

boolean_t wpa_config_validate_network(struct wpa_ssid *ssid)
{
	if (ssid->passphrase) {
		if (ssid->psk_set) {
			wpa_printf(MSG_ERROR, "Both PSK and passphrase set.");
			return (B_FALSE);
		}
		wpa_config_update_psk(ssid);
	}

	if ((ssid->key_mgmt & (WPA_KEY_MGMT_PSK | WPA_KEY_MGMT_FT_PSK |
	    WPA_KEY_MGMT_PSK_SHA256)) && !ssid->psk_set) {
		wpa_printf(MSG_ERROR, "WPA-PSK accepted for key "
		    "management, but no PSK configured.");
		return (B_FALSE);
	}

	if ((ssid->group_cipher & WPA_CIPHER_CCMP) &&
	    !(ssid->pairwise_cipher & WPA_CIPHER_CCMP) &&
	    !(ssid->pairwise_cipher & WPA_CIPHER_NONE)) {
		/* Group cipher cannot be stronger than the pairwise cipher. */
		wpa_printf(MSG_DEBUG, "Removed CCMP from group cipher"
		    " list since it was not allowed for pairwise cipher");
		ssid->group_cipher &= ~WPA_CIPHER_CCMP;
	}

	return (B_TRUE);
}

static int wpa_config_read_wlan(nwam_known_wlan_handle_t kwh, void *arg)
{
	struct wpa_config *config = arg;
	struct wpa_ssid *ssid;
	nwam_error_t err = NWAM_SUCCESS;
	int id = -1;
	int ret = 0;
	char *name;

	if (config == NULL) {
		wpa_printf(MSG_ERROR, "%s: config struct pointer is NULL",
		    __func__);
		return (-1);
	}

	/* the know wlan name will be used as network id */
	err = nwam_known_wlan_get_name(kwh, &name);
	if (err != NWAM_SUCCESS) {
		wpa_printf(MSG_ERROR, "%s: failed to get known wlan name",
		    __func__);
		return (-1);
	}

	wpa_printf(MSG_MSGDUMP, "%s: parsing wlan %s ", __func__, name);

	id = atoi(name);
	free(name);

	/* valid known wlan names are numeric values between 1 and INT_MAX */
	if (id <= 0 || id >= INT_MAX) {
		wpa_printf(MSG_ERROR, "%s: invalid wlan id %u", __func__, id);
		return (-1);
	}

	ssid = wpa_config_add_network(config);
	if (ssid == NULL) {
		wpa_printf(MSG_ERROR, "%s: failed allocating ssid config",
		    __func__);
		return (-1);
	}

	wpa_config_set_network_defaults(ssid);
	wpa_config_set_illumos_ssid_prefs(ssid);
	ssid->id = id;

	err = nwam_known_wlan_walk_props(kwh, wpa_config_read_prop, ssid, 0,
	    &ret);
	if (err != NWAM_SUCCESS || ret != 0) {
		wpa_printf(MSG_ERROR, "%s: failed parsing prop value(err %d,"
		    " ret %d)", __func__, err, ret);
		wpa_config_free_ssid(ssid);
		return (-1);
	}

	if (!wpa_config_validate_network(ssid))
		return (-1);

	return (0);
}

struct wpa_config * wpa_config_read(const char *name)
{
	nwam_error_t err = NWAM_SUCCESS;
	struct wpa_config *config;
	int ret = 0;

	if (name == NULL)
		return (NULL);
	wpa_printf(MSG_DEBUG, "%s(%s):Reading known-wlan.conf", name, __func__);

	config = wpa_config_alloc_empty(CTRL_IFACE_DIR, NULL);
	if (config == NULL)
		return (NULL);

	wpa_config_set_illumos_global_pref(config);

	err = nwam_walk_known_wlans(wpa_config_read_wlan, config, &ret);
	if (err != NWAM_SUCCESS) {
		wpa_printf(MSG_ERROR, "%s: nwam_walk_known_wlans failed (%d)",
		    __func__, err);
		wpa_printf(MSG_ERROR, "%s: Failed reading known wlan list"
		" error: %d", __func__, ret);
		wpa_config_free(config);
		return (NULL);
	}

	if (wpa_config_update_prio_list(config)) {
		wpa_printf(MSG_ERROR, "%s: Failed to add "
		   "network blocks to priority list.", __func__);
		return (NULL);
	}

	wpa_config_debug_dump_networks(config);

	return (config);
}

int wpa_config_write(const char *name, struct wpa_config *config)
{
	return (-1);
}

#endif /* CONFIG_BACKEND_SOLARIS */
