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
 * Wifi Security Policy Defines
 */

#ifndef _SECOBJ_H
#define	_SECOBJ_H

#include <libdladm.h>
#include <limits.h>

#ifdef	__cplusplus
extern "C" {
#endif

/*
 * Maximum size of secobj value. Note that it should not be greater than
 * DLD_SECOBJ_VAL_MAX.
 */
#define	DLADM_SECOBJ_VAL_MAX	256

/*
 * Maximum size of secobj name. Note that it should not be greater than
 * DLD_SECOBJ_NAME_MAX.
 */
#define	DLADM_SECOBJ_NAME_MAX	32

#define	PIN_FILE	"/etc/dladm/eap_pin.sh"

/* same as CONFIG_CTRL_IFACE_DIR in wpa_cli.c */
#define	CTRL_IFACE_DIR	"/var/run/wpa_supplicant"

/*
 * Do not reorder these
 */
typedef enum {
	DLADM_SECOBJ_CLASS_WEP = 1,
	DLADM_SECOBJ_CLASS_PSK,
	DLADM_SECOBJ_CLASS_TLS,
	DLADM_SECOBJ_CLASS_TTLS,
	DLADM_SECOBJ_CLASS_PEAP
} dladm_secobj_class_t;

typedef struct dladm_wlan_key {
	dladm_secobj_class_t wk_class;
	boolean_t wk_engine;
	char wk_name[DLADM_SECOBJ_NAME_MAX];
	uint8_t wk_val[DLADM_SECOBJ_VAL_MAX];
	uint16_t wk_len;
	uint8_t wk_p2mask;
	char wk_idx;
} dladm_wlan_key_t;

typedef struct secobj_class_info {
	const char		*sc_name;
	dladm_secobj_class_t	sc_dladmclass;
} secobj_class_info_t;

/*
 * Tunnel inner authentication
 * only for eap-ttls eap-peap
 */
typedef enum {
	DLADM_EAP_P2TTLS_PAP = 0x01,
	DLADM_EAP_P2TTLS_CHAP = 0x02,
	DLADM_EAP_P2TTLS_MS = 0x04,
	DLADM_EAP_P2TTLS_MSV2 = 0x08,
	DLADM_EAP_P2TTLS_EAPMD5 = 0x10,
	DLADM_EAP_P2TTLS_EAPGTC = 0x20,
	DLADM_EAP_P2TTLS_EAPMSV2 = 0x40
} dladm_wlan_p2ttls_t;

typedef enum {
	DLADM_WLAN_P2PEAP_MD5 = 0x01,
	DLADM_WLAN_P2PEAP_MSV2 = 0x02,
	DLADM_WLAN_P2PEAP_GTC = 0x04
/*	DLADM_WLAN_P2PEAP_TLS = 0x08 (unsupported configuration scheme) */
} dladm_wlan_p2peap_t;

enum {
	DLADM_EAP_ATTR_USER = 0x01,
	DLADM_EAP_ATTR_ANON = 0x02,
	DLADM_EAP_ATTR_CACERT = 0x04,
	DLADM_EAP_ATTR_CLICERT = 0x08,
	DLADM_EAP_ATTR_PRIV = 0x10
};

typedef struct dladm_wlan_eap {
	char eap_user[DLADM_STRSIZE]; /* email */
	char eap_anon[DLADM_STRSIZE]; /* email */
	char eap_ca_cert[PATH_MAX]; /*  filename */
	char eap_cli_cert[PATH_MAX]; /* filename */
	char eap_priv[PATH_MAX]; /* filename */
	uint8_t eap_valid;
} dladm_wlan_eap_t;

/* class */
extern const char	*dladm_secobjclass2str(dladm_secobj_class_t, char *);
extern dladm_status_t	dladm_str2secobjclass(const char *,
			    dladm_secobj_class_t *);
extern boolean_t	dladm_valid_secobjclass(dladm_secobj_class_t class);

/* keyname */
extern boolean_t	dladm_valid_secobj_name(const char *);

/* keyval */
extern dladm_status_t	dladm_str2secobjval(char *buf, uint8_t *obj_val,
			    uint16_t *obj_lenp, dladm_secobj_class_t class);
extern dladm_status_t	dladm_secobj_prompt(dladm_wlan_key_t *kdata,
			    FILE *filep);

/* secobj */
extern dladm_status_t	dladm_set_secobj(dladm_handle_t, dladm_wlan_key_t *,
			    uint_t);
extern dladm_status_t	dladm_get_secobj(dladm_handle_t, dladm_wlan_key_t *,
			    uint_t);
extern dladm_status_t	dladm_unset_secobj(dladm_handle_t, const char *,
			    uint_t);
extern dladm_status_t	dladm_walk_secobj(dladm_handle_t, void *,
			    boolean_t (*)(dladm_handle_t, void *, const char *,
			    dladm_secobj_class_t), uint_t);
extern boolean_t	find_matching_secobj(dladm_handle_t, void *,
			    const char *, dladm_secobj_class_t);

/* eap */
extern uint_t dladm_p2peap_2_str(uint8_t, char *, boolean_t);
extern uint_t dladm_p2ttls_2_str(uint8_t, char *, boolean_t);
extern uint_t dladm_str_2_p2peap(char *, uint8_t *);
extern uint_t dladm_str_2_p2ttls(char *, uint8_t *);

extern dladm_status_t dladm_str2crtname(char *option, char *eapst);
extern dladm_status_t dladm_str2identity(char *, char *);


/* pkcs11 routines */
extern dladm_status_t   dladm_eap_import(dladm_handle_t, char *,
			    char *, const char *, const dladm_secobj_class_t);
extern dladm_status_t   dladm_eap_delete(char *, dladm_secobj_class_t);

#ifdef	__cplusplus
}
#endif

#endif	/* _SECOBJ_H */
