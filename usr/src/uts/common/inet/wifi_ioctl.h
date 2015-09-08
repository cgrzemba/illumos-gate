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

#ifndef	__WIFI_IOCTL_H
#define	__WIFI_IOCTL_H

#include <sys/types.h>

#ifdef	__cplusplus
extern "C" {
#endif

#define	WPA_DOOR		"/var/run/wpa_door"

#define	MAX_BUF_LEN		65536
/*
 * field_offset
 */
#define	WIFI_BUF_OFFSET		offsetof(wldp_t, wldp_buf)
#define	WLDP_BUFSIZE		(MAX_BUF_LEN - WIFI_BUF_OFFSET)

/*
 * ioctls
 */
#define	WLAN_IOCTL_BASE 0x1000
#define	WLAN_COMMAND (WLAN_IOCTL_BASE + 0x4)

/*
 * commands
 */
#define	WL_COMMAND_BASE 0x3000
#define	WL_SCAN (WL_COMMAND_BASE + 0x0)
#define	WL_DISASSOCIATE (WL_COMMAND_BASE + 0x1)

/*
 * power mode (not yet supported)
 */
#define	WL_PM_AM 0x0
#define	WL_PM_MPS 0x1
#define	WL_PM_FAST 0x2
#define	WL_PM_USER 0x3

/*
 * wep operations
 */
#define	WL_WEP_OPERATION_BASE 0x6000
#define	WL_ADD (WL_WEP_OPERATION_BASE + 0x0)
#define	WL_DEL (WL_WEP_OPERATION_BASE + 0x1)
#define	WL_NUL (WL_WEP_OPERATION_BASE + 0x2)
#define	WL_IND (WL_WEP_OPERATION_BASE + 0x3)

#define	WL_NOENCRYPTION 0x0
#define	WL_ENC_WEP 0x1
#define	WL_ENC_WPA 0x2

/*
 * return values
 */
#define	WL_SUCCESS 0x0
#define	WL_NOTSUPPORTED EINVAL
#define	WL_LACK_FEATURE ENOTSUP
#define	WL_HW_ERROR EIO
#define	WL_ACCESS_DENIED EACCES
#define	WL_RETURN_BASE	0x7000
#define	WL_READONLY (WL_RETURN_BASE + 0x1)
#define	WL_WRITEONLY (WL_RETURN_BASE + 0x2)
#define	WL_NOAP (WL_RETURN_BASE + 0x3)
/*
 * other values
 */
#define	WL_OTHER_BASE 0x8000
#define	WL_FHSS (WL_OTHER_BASE + 0x0)
#define	WL_DSSS (WL_OTHER_BASE + 0x1)
#define	WL_IRBASE (WL_OTHER_BASE + 0x2)
#define	WL_OFDM (WL_OTHER_BASE + 0x3)
#define	WL_HRDS (WL_OTHER_BASE + 0x4)
#define	WL_ERP (WL_OTHER_BASE + 0x5)

/* common definitions (as in wpa_s driver.h) */
#define	IEEE80211_MODE_INFRA	0
#define	IEEE80211_MODE_IBSS	1
#define	IEEE80211_MODE_AP	2

#define	IEEE80211_CAP_ESS	0x0001
#define	IEEE80211_CAP_IBSS	0x0002
#define	IEEE80211_CAP_PRIVACY	0x0010

/* space for both wpa tx+rx keys */
#define	IEEE80211I_KEY_SIZE	16
#define	IEEE80211I_MIC_SIZE	(8+8)

/* supported event types (as in wpa_s driver.h) */
enum wl_net80211_events {
	NET80211_EVENT_ASSOC = 0,
	NET80211_EVENT_DISASSOC = 1,
	NET80211_EVENT_SCAN_RESULTS = 3,
	NET80211_EVENT_INTERFACE_STATUS = 5,
	NET80211_EVENT_ASSOC_REJECT = 13,
	NET80211_EVENT_ASSOC_TIMED_OUT = 15
};

/*
 * type definitions
 */
typedef boolean_t wl_create_ibss_t;
typedef boolean_t wl_wpa_t;
typedef boolean_t wl_radio_t;
typedef boolean_t wl_counterm_t;
typedef boolean_t wl_linkstatus_t;
typedef uint8_t wl_rssi_t; /* 0->127 */ /* driver specific */
typedef uint8_t wl_bss_type_t;
typedef uint8_t wl_authmode_t;
typedef uint8_t wl_encryption_t;
typedef uint16_t wl_beacon_period_t;
typedef uint32_t wl_beacon_age_t;
typedef uint64_t wl_beacon_tsf_t;
/* ieee80211com */
typedef uint32_t wl_capability_t;
/* ieee80211node */
typedef uint16_t wl_node_caps_t;

typedef struct wl_bssid {
	uint8_t wl_bssid_bssid[8];
} wl_bssid_t;

typedef struct wl_essid {
	uint32_t wl_essid_length;
	uint8_t wl_essid_essid[32];
} wl_essid_t;

typedef struct wl_phy_supported {
	uint32_t wl_phy_support_num;
	uint32_t wl_phy_support_phy_types[1];
} wl_phy_supported_t;

typedef struct wl_fhss {
	uint32_t wl_fhss_subtype;
	uint32_t wl_fhss_frequency;
	uint32_t wl_fhss_hoptime;
	uint32_t wl_fhss_hoppattern;
	uint32_t wl_fhss_hopset;
	uint32_t wl_fhss_dwelltime;
} wl_fhss_t;

typedef struct wl_dsss {
	uint32_t wl_dsss_subtype;
	uint32_t wl_dsss_frequency;
	boolean_t wl_dsss_have_short_preamble;
	uint32_t wl_dsss_preamble_mode;
	boolean_t wl_dsss_agility_enabled;
	boolean_t wl_dsss_have_pbcc;
	boolean_t wl_dsss_pbcc_enable;
} wl_dsss_t;

typedef struct wl_ofdm {
	uint32_t wl_ofdm_subtype;
	uint32_t wl_ofdm_frequency;
	uint32_t wl_ofdm_freq_supported;
	boolean_t wl_ofdm_ht_enabled;
} wl_ofdm_t;

typedef struct wl_erp {
	uint32_t wl_erp_subtype;
	uint32_t wl_erp_frequency;
	boolean_t wl_erp_have_short_preamble;
	uint32_t wl_erp_preamble_mode;
	boolean_t wl_erp_have_agility;
	boolean_t wl_erp_agility_enabled;
	boolean_t wl_erp_have_pbcc;
	boolean_t wl_erp_pbcc_enabled;
	boolean_t wl_erp_have_dsss_ofdm;
	boolean_t wl_erp_dsss_ofdm_enabled;
	boolean_t wl_erp_have_sst;
	boolean_t wl_erp_sst_enabled;
	boolean_t wl_erp_ht_enabled;
} wl_erp_t;

typedef union wl_phy_conf {
	wl_fhss_t wl_phy_fhss_conf;
	wl_dsss_t wl_phy_dsss_conf;
	wl_ofdm_t wl_phy_ofdm_conf;
	wl_erp_t wl_phy_erp_conf;
} wl_phy_conf_t;

typedef struct wl_ps_mode {
	uint32_t wl_ps_mode;
	uint32_t wl_ps_max_sleep;
	uint32_t wl_ps_min_sleep;
	uint32_t wl_ps_max_awake;
	uint32_t wl_ps_min_awake;
	boolean_t wl_ps_nobroadcast;
} wl_ps_mode_t;

typedef struct wl_rates {
	uint8_t wl_rates_num;
	uint8_t wl_rates_rates[15];  /* 0->127 */
} wl_rates_t;

/*
 * Macro and data structures defined for 802.11i.
 */
typedef struct wl_wpa_ie {
	uint8_t		wpa_ie[40];
	uint8_t		wpa_ie_len;
} wl_wpa_ie_t;

/*
 * WPA/RSN get/set key request.
 * ik_type  : wep/tkip/aes
 * ik_keyix : should be between 0 and 3, 0 will be used as default key.
 * ik_keylen: key length in bytes.
 * ik_keydata and ik_keylen include the DATA key and MIC key.
 * ik_keyrsc/ik_keytsc: rx/tx seq number.
 */
#pragma pack(1)
typedef struct wl_key {
	uint8_t		ik_type;

	uint8_t		ik_keylen;
	uint16_t	ik_keyix;

	uint16_t	ik_flags;

	uint8_t		ik_macaddr[6];
	uint64_t	ik_keyrsc;
	uint64_t	ik_keytsc;

	uint8_t		ik_keydata[IEEE80211I_KEY_SIZE+IEEE80211I_MIC_SIZE];
} wl_key_t;
#pragma pack()

/* wpa only */
typedef struct wl_del_key {
	uint8_t		idk_keyix;
	uint8_t		idk_macaddr[6];
} wl_del_key_t;

/*
 * structure for WL_MLME state manipulation request.
 * im_op: operations include auth/deauth/assoc/disassoc,
 * im_reason: 802.11 reason code
 */
typedef struct wl_mlme {
	uint16_t	im_reason;
	uint8_t		im_macaddr[6];
	uint8_t		im_op;
} wl_mlme_t;

/*
 * beacon, probe response info
 */
#pragma pack(1)
typedef struct wl_ess_conf {
	wl_essid_t 		wl_ess_conf_essid;
	wl_bssid_t 		wl_ess_conf_bssid;
	wl_beacon_tsf_t		wl_ess_conf_beacon_tsf;
	wl_beacon_age_t		wl_ess_conf_beacon_age;
	wl_rates_t		wl_ess_conf_rates;
	wl_node_caps_t 		wl_ess_conf_caps;
	wl_beacon_period_t	wl_ess_conf_beacon_period;
	wl_phy_conf_t		wl_ess_conf_phys;
	wl_wpa_ie_t		wl_ess_conf_wpa_ie;
	wl_rssi_t 		wl_ess_conf_sl;
} wl_ess_conf_t;
#pragma pack()

typedef struct wl_ess_list {
	uint16_t wl_ess_list_size;
	uint16_t wl_ess_list_num;
	wl_ess_conf_t wl_ess_list_ess[1];
} wl_ess_list_t;

typedef struct wl_wep_key {
	uint16_t wl_wep_length;
	uint16_t wl_wep_operation;
	uint8_t	 wl_wep_keyval[26];
} wl_wep_key_t;

typedef wl_wep_key_t wl_wep_key_tab_t[4];

typedef struct wldp {
	uint32_t wldp_length;
	uint32_t wldp_type;
	uint32_t wldp_result;
	uint32_t wldp_id;
	uint32_t wldp_buf[1];
} wldp_t;

/* event data passed through wpa door */
typedef struct wl_event {
	uint32_t wpa_ev_type;
	uint16_t wpa_ev_reason;
	uint8_t	wpa_ev_beacon[6];
} wl_event_t;

#ifdef __cplusplus
}
#endif

#endif /* __WIFI_IOCTL_H */
