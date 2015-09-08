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
 * Copyright 2010 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 * Copyright (c) 2012, Enrico Papi <enricop@computer.org>. All rights reserved.
 */

#include <assert.h>
#include <ctype.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <libdlwlan.h>

#include "libnwam_impl.h"
#include <libnwam_priv.h>

/*
 * Functions to support creating, modifying and destroying
 * known WLAN objects. These represent the WiFi connection history,
 * and are used by nwamd to identify and connect to known WLANs in
 * scan results.
 */
static nwam_error_t nwam_valid_ssid(nwam_value_t);
static nwam_error_t nwam_valid_secobj(nwam_value_t);
static nwam_error_t nwam_valid_priority(nwam_value_t);
static nwam_error_t nwam_valid_eapuser(nwam_value_t);
static nwam_error_t nwam_valid_certfile(nwam_value_t);
static nwam_error_t nwam_valid_priv(nwam_value_t);

struct nwam_prop_table_entry known_wlan_prop_table_entries[] = {
	{NWAM_KNOWN_WLAN_PROP_SSID, NWAM_VALUE_TYPE_STRING, B_FALSE,
	    1, 1, nwam_valid_ssid,
	    "Service set identifier (network name) [max:32chars]",
	    NWAM_TYPE_ANY, NWAM_CLASS_ANY},
	{NWAM_KNOWN_WLAN_PROP_BSSID, NWAM_VALUE_TYPE_STRING, B_FALSE,
	    0, 1, nwam_valid_mac_addr,
	    "BSSID (mac-addr in the form aa:bb:cc:dd:ee:ff) (default=any)",
	    NWAM_TYPE_ANY, NWAM_CLASS_ANY},
	{NWAM_KNOWN_WLAN_PROP_KEYNAME, NWAM_VALUE_TYPE_STRING, B_FALSE,
	    0, 1, nwam_valid_secobj,
	    "Existing and Valid secure object name [max:32chars] \n"
	    "Wlan Security Policy follows secure object class \n"
	    "(default=plaintext)",
	    NWAM_TYPE_ANY, NWAM_CLASS_ANY},
	{NWAM_KNOWN_WLAN_PROP_PRIORITY, NWAM_VALUE_TYPE_UINT64, B_FALSE,
	    0, 1, nwam_valid_priority,
	    "Wlans with higher priority values are matched earlier \n"
	    "Valid Range is [0->2147483647] (default=1)",
	    NWAM_TYPE_ANY, NWAM_CLASS_ANY},
	{NWAM_KNOWN_WLAN_PROP_DISABLED, NWAM_VALUE_TYPE_BOOLEAN, B_FALSE,
	    0, 1, nwam_valid_boolean,
	    "If known WLAN should be ignored during selection (default=false)",
	    NWAM_TYPE_ANY, NWAM_CLASS_ANY},
	{NWAM_KNOWN_WLAN_PROP_EAP_USER, NWAM_VALUE_TYPE_STRING, B_FALSE,
	    0, 1, nwam_valid_eapuser,
	    "Identity string for EAP (Network Access Identifier) \n"
	    "[required for EAP-TLS/TTLS/PEAP] [max:256chars]",
	    NWAM_TYPE_ANY, NWAM_CLASS_ANY},
	{NWAM_KNOWN_WLAN_PROP_EAP_ANON, NWAM_VALUE_TYPE_STRING, B_FALSE,
	    0, 1, nwam_valid_eapuser,
	    "Anonymous Identity string for EAP \n"
	    "[EAP-TLS/TTLS/PEAP only] [max:256chars] (default=empty)",
	    NWAM_TYPE_ANY, NWAM_CLASS_ANY},
	{NWAM_KNOWN_WLAN_PROP_CA_CERT, NWAM_VALUE_TYPE_STRING, B_FALSE,
	    0, 1, nwam_valid_certfile,
	    "File path to CA certificate (PEM/DER) \n"
	    "[EAP-TLS/TTLS/PEAP only] (default=empty)",
	    NWAM_TYPE_ANY, NWAM_CLASS_ANY},
	{NWAM_KNOWN_WLAN_PROP_PRIV, NWAM_VALUE_TYPE_STRING, B_FALSE,
	    0, 1, nwam_valid_priv,
	    "File path to Client Private Key (PEM/DER/PKCS#12) \n"
	    "[required for EAP-TLS] ",
	    NWAM_TYPE_ANY, NWAM_CLASS_ANY},
	{NWAM_KNOWN_WLAN_PROP_CLI_CERT, NWAM_VALUE_TYPE_STRING, B_FALSE,
	    0, 1, nwam_valid_certfile,
	    "File path to Client Certificate (PEM/DER) \n"
	    "[required if Client Private Key file does not contain both]",
	    NWAM_TYPE_ANY, NWAM_CLASS_ANY},
};

#define	NWAM_NUM_KNOWN_WLAN_PROPS	\
		(sizeof (known_wlan_prop_table_entries) / \
		sizeof (*known_wlan_prop_table_entries))

struct nwam_prop_table known_wlan_prop_table =
	{ NWAM_NUM_KNOWN_WLAN_PROPS, known_wlan_prop_table_entries };

static int
find_highest_id(nwam_known_wlan_handle_t kwh, void *data)
{
	uint32_t *topid = data;
	int currid = -1;
	char *curridstr;

	if (nwam_known_wlan_get_name(kwh, &curridstr) != NWAM_SUCCESS)
		return (-1);

	currid = atoi(curridstr);

	if (currid > 0 && currid < INT_MAX - 1 && currid > *topid)
		*topid = currid;

	free(curridstr);
	return (0);
}

static boolean_t
nwam_valid_wlanid(const char *name)
{
	long wlanid;
	char *endptr;

	wlanid = strtol(name, &endptr, 10);
	if (wlanid == 0 && endptr == name)
		return (B_FALSE);
	if (wlanid <= 0 || wlanid >= INT_MAX)
		return (B_FALSE);

	if (*endptr == '\0')
		return (B_TRUE);

	return (B_FALSE);
}

nwam_error_t
nwam_known_wlan_read(const char *name, uint64_t flags,
    nwam_known_wlan_handle_t *kwhp)
{
	return (nwam_read(NWAM_OBJECT_TYPE_KNOWN_WLAN,
	    NWAM_KNOWN_WLAN_CONF_FILE, name, flags, kwhp));
}

nwam_error_t
nwam_known_wlan_create(const char *name, nwam_known_wlan_handle_t *kwhp)
{
	nwam_error_t err;

	assert(kwhp != NULL && name != NULL);

	if (!nwam_valid_wlanid(name))
		return (NWAM_INVALID_ARG);

	if ((err = nwam_create(NWAM_OBJECT_TYPE_KNOWN_WLAN,
	    NWAM_KNOWN_WLAN_CONF_FILE, name, kwhp)) != NWAM_SUCCESS)
		return (err);

	/*
	 * Create new object list for known WLAN.
	 */
	err = nwam_alloc_object_list(&((*kwhp)->nwh_data));

	if (err != NWAM_SUCCESS) {
		nwam_known_wlan_free(*kwhp);
		*kwhp = NULL;
	}
	return (err);
}

nwam_error_t
nwam_known_wlan_get_name(nwam_known_wlan_handle_t kwh, char **namep)
{
	return (nwam_get_name(kwh, namep));
}

nwam_error_t
nwam_known_wlan_set_name(nwam_known_wlan_handle_t kwh, const char *name)
{
	assert(kwh != NULL && name != NULL);

	if (!nwam_valid_wlanid(name))
		return (NWAM_INVALID_ARG);

	return (nwam_set_name(kwh, name));
}

boolean_t
nwam_known_wlan_can_set_name(nwam_known_wlan_handle_t kwh)
{
	return (!kwh->nwh_committed);
}

/*
 * Some recursion is required here, since if _WALK_PRIORITY_ORDER is specified,
 * we need to first walk the list of known WLANs to retrieve names
 * and priorities, then utilize that list to carry out an in-order walk.
 */
nwam_error_t
nwam_walk_known_wlans(int(*cb)(nwam_known_wlan_handle_t, void *), void *data,
    int *retp)
{
	assert(cb != NULL);

	return (nwam_walk(NWAM_OBJECT_TYPE_KNOWN_WLAN,
	    NWAM_KNOWN_WLAN_CONF_FILE, cb, data, 0, retp, NULL));
}

void
nwam_known_wlan_free(nwam_known_wlan_handle_t kwh)
{
	nwam_free(kwh);
}

nwam_error_t
nwam_known_wlan_copy(nwam_known_wlan_handle_t oldkwh, const char *newname,
    nwam_known_wlan_handle_t *newkwhp)
{
	return (nwam_copy(NWAM_KNOWN_WLAN_CONF_FILE, oldkwh, newname, newkwhp));
}

nwam_error_t
nwam_known_wlan_delete_prop(nwam_known_wlan_handle_t kwh, const char *propname)
{
	nwam_error_t err;
	void *olddata;

	assert(kwh != NULL && propname != NULL);

	/*
	 * Duplicate data, remove property and validate. If validation
	 * fails, revert to data duplicated prior to remove.
	 */
	if ((err = nwam_dup_object_list(kwh->nwh_data, &olddata))
	    != NWAM_SUCCESS)
		return (err);
	if ((err = nwam_delete_prop(kwh->nwh_data, propname)) != NWAM_SUCCESS) {
		nwam_free_object_list(kwh->nwh_data);
		kwh->nwh_data = olddata;
		return (err);
	}
	if ((err = nwam_known_wlan_validate(kwh, NULL)) != NWAM_SUCCESS) {
		nwam_free_object_list(kwh->nwh_data);
		kwh->nwh_data = olddata;
		return (err);
	}
	nwam_free_object_list(olddata);

	return (NWAM_SUCCESS);
}

nwam_error_t
nwam_known_wlan_set_prop_value(nwam_known_wlan_handle_t kwh,
    const char *propname, nwam_value_t value)
{
	nwam_error_t err;

	assert(kwh != NULL && propname != NULL && value != NULL);

	if ((err = nwam_known_wlan_validate_prop(kwh, propname, value))
	    != NWAM_SUCCESS)
		return (err);

	return (nwam_set_prop_value(kwh->nwh_data, propname, value));
}

nwam_error_t
nwam_known_wlan_get_prop_value(nwam_known_wlan_handle_t kwh,
    const char *propname, nwam_value_t *valuep)
{
	return (nwam_get_prop_value(kwh->nwh_data, propname, valuep));
}

nwam_error_t
nwam_known_wlan_walk_props(nwam_known_wlan_handle_t kwh,
    int (*cb)(const char *, nwam_value_t, void *),
    void *data, uint64_t flags, int *retp)
{
	return (nwam_walk_props(kwh, cb, data, flags, retp));
}

nwam_error_t
nwam_known_wlan_commit(nwam_known_wlan_handle_t kwh)
{
	nwam_error_t err;

	assert(kwh != NULL && kwh->nwh_data != NULL);

	if ((err = nwam_known_wlan_validate(kwh, NULL)) != NWAM_SUCCESS)
		return (err);

	return (nwam_commit(NWAM_KNOWN_WLAN_CONF_FILE, kwh,
	    NWAM_FLAG_ENTITY_KNOWN_WLAN));
}

nwam_error_t
nwam_known_wlan_destroy(nwam_known_wlan_handle_t kwh, uint64_t flags)
{
	nwam_value_t keynval;

	if (nwam_known_wlan_get_prop_value(kwh, NWAM_KNOWN_WLAN_PROP_KEYNAME,
	    &keynval) == NWAM_SUCCESS) {
		dladm_handle_t adm_handle;
		char *keyname;

		if (nwam_value_get_string(keynval, &keyname) == NWAM_SUCCESS &&
		    dladm_open(&adm_handle) == DLADM_STATUS_OK) {
			(void) dladm_unset_secobj(adm_handle, keyname,
			    DLADM_OPT_PERSIST);
			dladm_close(adm_handle);
		}
		nwam_value_free(keynval);
	}
	return (nwam_destroy(NWAM_KNOWN_WLAN_CONF_FILE, kwh,
	    flags | NWAM_FLAG_ENTITY_KNOWN_WLAN));
}

nwam_error_t
nwam_known_wlan_get_prop_description(const char *propname,
    const char **descriptionp)
{
	return (nwam_get_prop_description(known_wlan_prop_table, propname,
	    descriptionp));
}

/* Property-specific value validation functions should go here. */

static nwam_error_t
nwam_valid_ssid(nwam_value_t value)
{
	char *essid;
	uint8_t len;

	if (nwam_value_get_string(value, &essid) != NWAM_SUCCESS)
		return (NWAM_ENTITY_INVALID_VALUE);

	len = strnlen(essid, DLADM_WLAN_MAX_ESSID_LEN);
	/* ssid can contain control chars */
	if (len == 0 || len == DLADM_WLAN_MAX_ESSID_LEN)
		return (NWAM_ENTITY_INVALID_VALUE);

	return (NWAM_SUCCESS);
}

static nwam_error_t
nwam_valid_secobj(nwam_value_t value)
{
	dladm_handle_t adm_handle;
	secobj_class_info_t key_if;
	char *keyname;

	if (nwam_value_get_string(value, &keyname) != NWAM_SUCCESS)
		return (NWAM_ENTITY_INVALID_VALUE);

	if (!dladm_valid_secobj_name(keyname))
		return (NWAM_ENTITY_INVALID_VALUE);

	if (dladm_open(&adm_handle) != DLADM_STATUS_OK)
		return (NWAM_ENTITY_INVALID_VALUE);

	key_if.sc_name = keyname;
	key_if.sc_dladmclass = 0;
	if (dladm_walk_secobj(adm_handle, &key_if, find_matching_secobj,
	    DLADM_OPT_PERSIST)) {
		dladm_close(adm_handle);
		return (NWAM_ENTITY_INVALID_VALUE);
	}

	dladm_close(adm_handle);

	/* not found */
	if (!dladm_valid_secobjclass(key_if.sc_dladmclass))
		return (NWAM_ENTITY_INVALID_VALUE);

	return (NWAM_SUCCESS);
}

static nwam_error_t
nwam_valid_priority(nwam_value_t value)
{
	uint64_t priv_val;

	if (nwam_valid_uint64(value) != NWAM_SUCCESS)
		return (NWAM_ENTITY_INVALID_VALUE);

	if (nwam_value_get_uint64(value, &priv_val) != NWAM_SUCCESS)
		return (NWAM_ENTITY_INVALID_VALUE);

	if (priv_val >= INT_MAX)
		return (NWAM_ENTITY_INVALID_VALUE);

	return (NWAM_SUCCESS);
}

static nwam_error_t
nwam_valid_eapuser(nwam_value_t value)
{
	char *user;
	char testbuf[DLADM_STRSIZE];

	(void) memset(testbuf, 0, sizeof (testbuf));
	if (nwam_value_get_string(value, &user) != NWAM_SUCCESS)
		return (NWAM_ENTITY_INVALID_VALUE);

	if (dladm_str2identity(user, testbuf) != DLADM_STATUS_OK)
		return (NWAM_ENTITY_INVALID_VALUE);

	return (NWAM_SUCCESS);
}

static nwam_error_t
nwam_valid_certfile(nwam_value_t value)
{
	char *cert;
	char testbuf[DLADM_STRSIZE];

	if (nwam_value_get_string(value, &cert) != NWAM_SUCCESS)
		return (NWAM_ENTITY_INVALID_VALUE);

	if (dladm_str2crtname(cert, testbuf) != DLADM_STATUS_OK)
		return (NWAM_ENTITY_INVALID_VALUE);

	return (NWAM_SUCCESS);
}

static nwam_error_t
nwam_valid_priv(nwam_value_t value)
{
	char *priv;
	char testbuf[DLADM_STRSIZE];

	if (nwam_value_get_string(value, &priv) != NWAM_SUCCESS)
		return (NWAM_ENTITY_INVALID_VALUE);

	if (dladm_str2crtname(priv, testbuf) != DLADM_STATUS_OK)
		return (NWAM_ENTITY_INVALID_VALUE);

	/*
	 * TODO: when we add PKCS#11 cert. by reference support,
	 * restrict success only to pk12 files *
	 */
	return (NWAM_SUCCESS);
}

nwam_error_t
nwam_known_wlan_validate(nwam_known_wlan_handle_t kwh, const char **errpropp)
{
	/*
	 * TODO: when we add PKCS#11 cert. by reference support,
	 * evaluating known_wlan_prop_table is not enough.
	 */
	return (nwam_validate(known_wlan_prop_table, kwh, errpropp));
}

nwam_error_t
nwam_known_wlan_validate_prop(nwam_known_wlan_handle_t kwh,
    const char *propname, nwam_value_t value)
{
	return (nwam_validate_prop(known_wlan_prop_table, kwh, propname,
	    value));
}

/*
 * Given a property, return expected property data type
 */
nwam_error_t
nwam_known_wlan_get_prop_type(const char *propname, nwam_value_type_t *typep)
{
	return (nwam_get_prop_type(known_wlan_prop_table, propname, typep));
}

nwam_error_t
nwam_known_wlan_get_default_proplist(const char ***prop_list,
    uint_t *numvaluesp)
{
	return (nwam_get_default_proplist(known_wlan_prop_table,
	    NWAM_TYPE_ANY, NWAM_CLASS_ANY, prop_list, numvaluesp));
}

/*
 * Add the given ESSID, BSSID, key name to known WLANs.
 * BSSID and keyname can be NULL.
 * Default Priority Group is 1 for known wlans
 * By default new Known Wlans are enabled
 *
 * We always add a new wlan unless ESSID, BSSID and secobj class are the same
 */
nwam_error_t
nwam_known_wlan_add_to_known_wlans(dladm_handle_t dh, nwam_wlan_t *mywlan,
    dladm_wlan_key_t *key_data, dladm_wlan_eap_t *eap_data,
    const char **invalid_prop)
{
	nwam_known_wlan_handle_t kwh;
	nwam_error_t err;

	nwam_value_t keynameval = NULL, bssidval = NULL, ssidval = NULL;
	nwam_value_t identityval = NULL, anonval = NULL, ca_certval = NULL;
	nwam_value_t cli_certval = NULL, privval = NULL;

	int ret = -1;
	uint32_t freewlanid = 1;
	char wname[16];
	char bssidstr[DLADM_WLAN_BSSID_LEN * 3];

	if (mywlan == NULL)
		return (NWAM_INVALID_ARG);

	err = nwam_walk_known_wlans(find_highest_id, &freewlanid, &ret);
	if (err != NWAM_SUCCESS)
		return (err);

	if (ret == -1)
		return (NWAM_ENTITY_NOT_FOUND);

	freewlanid++;

	(void) snprintf(wname, sizeof (wname), "%u", freewlanid);

	if ((err = nwam_known_wlan_create(wname, &kwh)) != NWAM_SUCCESS)
		return (err);

	/* NWAM_KNOWN_WLAN_PROP_SSID	"ssid" */

	if ((err = nwam_value_create_string((char *)mywlan->nww_essid,
	    &ssidval)) != NWAM_SUCCESS) {
		nwam_known_wlan_free(kwh);
		return (err);
	}
	err = nwam_known_wlan_set_prop_value(kwh, NWAM_KNOWN_WLAN_PROP_SSID,
	    ssidval);
	nwam_value_free(ssidval);
	if (err != NWAM_SUCCESS) {
		nwam_known_wlan_free(kwh);
		return (err);
	}

	/* NWAM_KNOWN_WLAN_PROP_BSSID	"bssid" */

	(void) dladm_wlan_bssid2str(mywlan->nww_bssid, bssidstr);
	if ((err = nwam_value_create_string(bssidstr, &bssidval)) !=
	    NWAM_SUCCESS) {
		nwam_known_wlan_free(kwh);
		return (err);
	}
	err = nwam_known_wlan_set_prop_value(kwh, NWAM_KNOWN_WLAN_PROP_BSSID,
	    bssidval);
	nwam_value_free(bssidval);
	if (err != NWAM_SUCCESS) {
		nwam_known_wlan_free(kwh);
		return (err);
	}

	/* NWAM_KNOWN_WLAN_PROP_KEYNAME	"keyname" */

	if (key_data == NULL)
		goto commit;

	(void) snprintf(key_data->wk_name, DLADM_SECOBJ_NAME_MAX, "nwam-%s",
	    wname);

	{
		dladm_status_t status = DLADM_STATUS_OK;
		status = dladm_set_secobj(dh, key_data, DLADM_OPT_PERSIST);
		if (status != DLADM_STATUS_OK) {
			nwam_known_wlan_free(kwh);
			return (NWAM_ERROR_INTERNAL);
		}
	}

	if ((err = nwam_value_create_string(key_data->wk_name, &keynameval))
	    != NWAM_SUCCESS) {
		nwam_known_wlan_free(kwh);
		return (err);
	}
	err = nwam_known_wlan_set_prop_value(kwh, NWAM_KNOWN_WLAN_PROP_KEYNAME,
	    keynameval);
	nwam_value_free(keynameval);
	if (err != NWAM_SUCCESS) {
		nwam_known_wlan_free(kwh);
		return (err);
	}

	if (eap_data == NULL)
		goto commit;

	/* NWAM_KNOWN_WLAN_PROP_EAP_USER	"identity" */

	if ((err = nwam_value_create_string(eap_data->eap_user, &identityval))
	    != NWAM_SUCCESS) {
		nwam_known_wlan_free(kwh);
		return (err);
	}
	err = nwam_known_wlan_set_prop_value(kwh, NWAM_KNOWN_WLAN_PROP_EAP_USER,
	    identityval);
	nwam_value_free(identityval);
	if (err != NWAM_SUCCESS) {
		nwam_known_wlan_free(kwh);
		return (err);
	}

	/* NWAM_KNOWN_WLAN_PROP_EAP_ANON	"anonymous_identity" */

	if (eap_data->eap_valid & DLADM_EAP_ATTR_ANON) {
		if ((err = nwam_value_create_string(eap_data->eap_anon,
		    &anonval)) != NWAM_SUCCESS) {
			nwam_known_wlan_free(kwh);
			return (err);
		}
		err = nwam_known_wlan_set_prop_value(kwh,
		    NWAM_KNOWN_WLAN_PROP_EAP_ANON, anonval);
		nwam_value_free(anonval);
		if (err != NWAM_SUCCESS) {
			nwam_known_wlan_free(kwh);
			return (err);
		}
	}

	/* NWAM_KNOWN_WLAN_PROP_CA_CERT	"ca_cert" */

	if (eap_data->eap_valid & DLADM_EAP_ATTR_CACERT) {
		if ((err = nwam_value_create_string(eap_data->eap_ca_cert,
		    &ca_certval)) != NWAM_SUCCESS) {
			nwam_known_wlan_free(kwh);
			return (err);
		}
		err = nwam_known_wlan_set_prop_value(kwh,
		    NWAM_KNOWN_WLAN_PROP_CA_CERT, ca_certval);
		nwam_value_free(ca_certval);
		if (err != NWAM_SUCCESS) {
			nwam_known_wlan_free(kwh);
			return (err);
		}
	}

	/* NWAM_KNOWN_WLAN_PROP_PRIV	"private_key" */

	if (eap_data->eap_valid & DLADM_EAP_ATTR_PRIV) {
		if ((err = nwam_value_create_string(eap_data->eap_priv,
		    &privval)) != NWAM_SUCCESS) {
			nwam_known_wlan_free(kwh);
			return (err);
		}
		err = nwam_known_wlan_set_prop_value(kwh,
		    NWAM_KNOWN_WLAN_PROP_PRIV, privval);
		nwam_value_free(privval);
		if (err != NWAM_SUCCESS) {
			nwam_known_wlan_free(kwh);
			return (err);
		}
	}

	/* NWAM_KNOWN_WLAN_PROP_CLI_CERT	"client_cert" */

	if (eap_data->eap_valid & DLADM_EAP_ATTR_PRIV) {
		if ((err = nwam_value_create_string(eap_data->eap_cli_cert,
		    &cli_certval)) != NWAM_SUCCESS) {
			nwam_known_wlan_free(kwh);
			return (err);
		}
		err = nwam_known_wlan_set_prop_value(kwh,
		    NWAM_KNOWN_WLAN_PROP_CLI_CERT, cli_certval);
		nwam_value_free(cli_certval);
		if (err != NWAM_SUCCESS) {
			nwam_known_wlan_free(kwh);
			return (err);
		}
	}

commit:
	if (nwam_known_wlan_validate(kwh, invalid_prop) != NWAM_SUCCESS) {
		nwam_known_wlan_free(kwh);
		return (err);
	}

	err = nwam_known_wlan_commit(kwh);
	nwam_known_wlan_free(kwh);

	mywlan->nww_wlanid = freewlanid;

	return (err);
}
