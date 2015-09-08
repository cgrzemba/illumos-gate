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

#ifndef _LIBDLWLAN_H
#define	_LIBDLWLAN_H

/*
 * This file includes structures, macros and routines used by WLAN link
 * administration.
 */

#include <libdladm.h>
#include <wpa_ctrl.h>

/*
 * General libdlwlan definitions and functions.
 *
 * These interfaces are ON consolidation-private.
 * For documentation, refer to PSARC/2006/623.
 *
 */

#ifdef	__cplusplus
extern "C" {
#endif

#define	DLADM_WLAN_MAX_ESSID_LEN    	32	/* per 802.11 spec */
#define	DLADM_WLAN_BSSID_LEN		6	/* per 802.11 spec */
#define	DLADM_WLAN_MAX_RATES_NUM	15	/* IEEE80211_RATE_MAXSIZE */

/*
 * wa_valid is a bitfield used for indicating the validity of each attribute in
 * the dladm_wlan_attr data struct
 * wa_valid may have 0 or more of the following bits set:
 */
enum {
	DLADM_WLAN_ATTR_ESSID = 0x00000001,
	DLADM_WLAN_ATTR_BSSID = 0x00000002,
	DLADM_WLAN_ATTR_SECMODE = 0x00000004,
	DLADM_WLAN_ATTR_STRENGTH = 0x00000008,
	DLADM_WLAN_ATTR_MODE = 0x00000010,
	DLADM_WLAN_ATTR_RATES = 0x00000020,
	DLADM_WLAN_ATTR_AUTH = 0x00000040,
	DLADM_WLAN_ATTR_BSSTYPE = 0x00000080,
	DLADM_WLAN_ATTR_CHANNEL = 0x00000100,
	DLADM_WLAN_ATTR_IEDETAIL = 0x00000200
};

/* the type of key management used */
typedef enum {
	DLADM_WLAN_SECMODE_NONE = 0,
	DLADM_WLAN_SECMODE_WEP,
	DLADM_WLAN_SECMODE_PSK,
	DLADM_WLAN_SECMODE_EAP
} dladm_wlan_secmode_t;

/* 80211 mode, this is actually an informative field only */
typedef enum {
	DLADM_WLAN_MODE_NONE = 0,
	DLADM_WLAN_MODE_80211A,
	DLADM_WLAN_MODE_80211B,
	DLADM_WLAN_MODE_80211G,
	DLADM_WLAN_MODE_80211GN,
	DLADM_WLAN_MODE_80211AN
} dladm_wlan_mode_t;

typedef struct dladm_wlan_essid {
	uint8_t we_bytes[DLADM_WLAN_MAX_ESSID_LEN];
	uint8_t we_length;
} dladm_wlan_essid_t;

typedef struct dladm_wlan_bssid {
	uint8_t wb_bytes[DLADM_WLAN_BSSID_LEN];
} dladm_wlan_bssid_t;

/* link speed rates: wr_rates[0] should be the highest rate */
typedef struct dladm_wlan_rates {
	uint8_t	wr_rates[DLADM_WLAN_MAX_RATES_NUM]; /* 0->127 */
	uint8_t	wr_cnt;
} dladm_wlan_rates_t;


typedef struct dladm_wlan_attr {
	uint_t wa_valid;
	dladm_wlan_essid_t wa_essid;
	dladm_wlan_bssid_t wa_bssid;
	uint8_t wa_auth; /* shared/open */
	dladm_wlan_secmode_t wa_secmode;
	dladm_wlan_mode_t wa_mode;
	dladm_wlan_rates_t wa_rates;
	uint16_t wa_freq; /* 80211 channel (value is in Mhz) */
	uint8_t wa_strength; /* signal strength 0->127 */
	uint8_t wa_bsstype; /* ibss/bss */
	char wa_ietxt[48]; /* string with detailed information on wpaie props */
} dladm_wlan_attr_t;

typedef struct dladm_wlan_linkattr {
	boolean_t la_connected;
	dladm_wlan_attr_t la_wlan_attr;
} dladm_wlan_linkattr_t;

/* check wpa_s status and retrieves a valid handler for its ctrl interface */
extern dladm_status_t dladm_wlan_validate(dladm_handle_t, datalink_id_t,
			    struct wpa_ctrl **, char *);
/* start a network scan */
extern dladm_status_t dladm_wlan_scan(dladm_handle_t, datalink_id_t);
/* retrieve and parse scan results */
extern dladm_status_t dladm_wlan_parse_esslist(dladm_handle_t, datalink_id_t,
    void *, boolean_t (*)(void *, dladm_wlan_attr_t *));
/* connection management */
extern dladm_status_t dladm_wlan_connect(dladm_handle_t, datalink_id_t,
    dladm_wlan_attr_t *, const void *, void *);
extern dladm_status_t dladm_wlan_disconnect(dladm_handle_t, datalink_id_t);
extern dladm_status_t dladm_wlan_cmd(const char *, uint_t);

/* GET */
extern dladm_status_t dladm_wlan_get_linkattr(dladm_handle_t, datalink_id_t,
    dladm_wlan_linkattr_t *);
extern dladm_status_t dladm_wlan_get_essid(dladm_handle_t, datalink_id_t,
    uint8_t *, uint8_t *);
extern dladm_status_t dladm_wlan_get_bssid(dladm_handle_t, datalink_id_t,
    uint8_t *);
extern dladm_status_t dladm_wlan_get_esslist(dladm_handle_t, datalink_id_t,
    void *, size_t);
extern dladm_status_t dladm_wlan_get_capability(dladm_handle_t, datalink_id_t,
    void *, size_t);
extern dladm_status_t dladm_wlan_get_linkstatus(dladm_handle_t, datalink_id_t,
    void *, size_t);
extern dladm_status_t dladm_wlan_get_wpa_ie(dladm_handle_t, datalink_id_t,
    void *, size_t);

/* SET - the main consumer is wpa_s driver interface */
extern dladm_status_t dladm_wlan_set_bsstype(dladm_handle_t, datalink_id_t,
    uint8_t);
extern dladm_status_t dladm_wlan_set_authmode(dladm_handle_t, datalink_id_t,
    uint8_t);
extern dladm_status_t dladm_wlan_set_essid(dladm_handle_t, datalink_id_t,
    const uint8_t *, uint8_t);
extern dladm_status_t dladm_wlan_set_bssid(dladm_handle_t, datalink_id_t,
    const uint8_t *);
extern dladm_status_t dladm_wlan_set_counterm(dladm_handle_t, datalink_id_t,
    boolean_t);
extern dladm_status_t dladm_wlan_createibss(dladm_handle_t, datalink_id_t);
/*
 * this is actually used only in wpa_s driver interface when setting IBSS
 * mode channel. The user provides a channel number in this case and not
 * a frequency value
 */
extern dladm_status_t dladm_wlan_set_channel(dladm_handle_t, datalink_id_t,
    uint16_t);
extern dladm_status_t dladm_wlan_set_wpa_ie(dladm_handle_t, datalink_id_t,
    const uint8_t *, size_t);
extern dladm_status_t dladm_wlan_set_wpa(dladm_handle_t, datalink_id_t);
/* same signature of the set_key function in wpa_s driver interface */
extern dladm_status_t dladm_wlan_set_key(dladm_handle_t handle,
    datalink_id_t linkid,
    uint_t alg, const uint8_t *addr, int key_idx, int set_tx,
    const uint8_t *seq, size_t seq_len,
    const uint8_t *key, size_t key_len);
extern dladm_status_t dladm_wlan_set_mlme(dladm_handle_t, datalink_id_t,
    boolean_t, int, const uint8_t *);

/* VAL2STR */
extern const char *dladm_wlan_bssid2str(const uint8_t *, char *);
extern const char *dladm_wlan_secmode2str(dladm_wlan_secmode_t, char *);
extern const char *dladm_wlan_strength2str(const uint8_t, char *);
extern const char *dladm_wlan_mode2str(dladm_wlan_mode_t, char *);
extern const char *dladm_wlan_rate2str(const uint8_t, char *);
extern const char *dladm_wlan_auth2str(const uint8_t, char *);
extern const char *dladm_wlan_bsstype2str(const uint8_t, char *);
extern const char *dladm_wlan_linkstatus2str(uint_t, char *);
extern const char *dladm_wlan_freq2channel(const uint16_t, char *);

/* STR2VAL */
extern dladm_status_t dladm_wlan_str2bssid(const char *, uint8_t *);
extern dladm_status_t dladm_wlan_str2bsstype(const char *, uint8_t *);

/* obtains the full path of the control socket of a given wpa_s instance */
extern char *wpa_get_ctrlname(const char *, char *);
/* for sending control requests to wpa_supplicant */
extern int wpa_request(struct wpa_ctrl *, int, char *[]);
/* listens on the control sockets for control events */
extern boolean_t wpa_get_event(struct wpa_ctrl *, char **, char **);
/* parses wpa informazion element */
extern int wpa_supplicant_capsie2txt(const uint16_t ,
    const uint8_t *, const uint8_t , char *, size_t , int *);

#ifdef	__cplusplus
}
#endif

#endif	/* _LIBDLWLAN_H */
