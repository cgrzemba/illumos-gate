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

#include <strings.h>
#include <ctype.h>
#include <unistd.h>
#include <stropts.h>
#include <errno.h>
#include <sys/stat.h>
#include <libintl.h>
#include <secobj.h>
#include <netinet/inetutil.h>
#include <sys/dld_ioc.h>
#include <libdladm_impl.h>
#include <kmfapi.h>

static dladm_status_t	i_dladm_set_secobj_db(dladm_handle_t,
			    dladm_wlan_key_t *);
static dladm_status_t	i_dladm_get_secobj_db(dladm_handle_t,
			    dladm_wlan_key_t *);
static dladm_status_t	i_dladm_unset_secobj_db(dladm_handle_t, const char *);
static dladm_status_t	i_dladm_walk_secobj_db(dladm_handle_t, void *,
			    boolean_t (*)(dladm_handle_t, void *,
			    const char *, dladm_secobj_class_t));

static const secobj_class_info_t secobj_class_table[] = {
	{"wep",		DLADM_SECOBJ_CLASS_WEP},
	{"psk", 	DLADM_SECOBJ_CLASS_PSK},
	{"eap-tls",	DLADM_SECOBJ_CLASS_TLS},
	{"eap-ttls",	DLADM_SECOBJ_CLASS_TTLS},
	{"peap",	DLADM_SECOBJ_CLASS_PEAP}
};

static const val_desc_t p2ttls_vals[] = {
	{ "pap",	DLADM_EAP_P2TTLS_PAP	},
	{ "chap",	DLADM_EAP_P2TTLS_CHAP	},
	{ "mschap",	DLADM_EAP_P2TTLS_MS	},
	{ "mschapv2",	DLADM_EAP_P2TTLS_MSV2	},
	{ "eap-md5",	DLADM_EAP_P2TTLS_EAPMD5	},
	{ "eap-gtc",	DLADM_EAP_P2TTLS_EAPGTC	},
	{ "eap-mschapv2", DLADM_EAP_P2TTLS_EAPMSV2}
};

static const val_desc_t p2peap_vals[] = {
	{ "md5",	DLADM_WLAN_P2PEAP_MD5	},
	{ "gtc",	DLADM_WLAN_P2PEAP_MSV2	},
	{ "mschapv2",	DLADM_WLAN_P2PEAP_GTC	}
/*	{ "tls",	DLADM_WLAN_P2PEAP_TLS	} unsupported config */
};

static const char *key_prompts[] = {
	NULL,
	"WEP Password (5/13 ASCII or 10/26 Hex characters)\n",
	"WPA PSK (from 8, up to 63 ASCII chars or 64 Hex characters)\n",
	"EAP Passphrase (up to 256 ASCII characters)\n",
	"EAP Passphrase (up to 256 ASCII characters)\n",
	"Private Key File Passphrase (up to 256 ASCII characters)\n",
};

#define	SECOBJ_MAXBUFSZ	65536
#define	NSECOBJCLASS \
	(sizeof (secobj_class_table) / sizeof (secobj_class_info_t))

boolean_t
dladm_valid_secobjclass(dladm_secobj_class_t class)
{
	return (class > 0 && class <= NSECOBJCLASS);
}

boolean_t
dladm_valid_secobj_name(const char *secobj_name)
{
	const char *cp;
	size_t len = strnlen(secobj_name, DLADM_SECOBJ_NAME_MAX);

	if (len == DLADM_SECOBJ_NAME_MAX || len == 0)
		return (B_FALSE);

	/*
	 * The legal characters in a secobj name are:
	 * alphanumeric (a-z, A-Z, 0-9), '.', '_', '-', ':'
	 */
	for (cp = secobj_name; *cp != '\0'; cp++) {
		if (!isalnum(*cp) && (*cp != ':') &&
		    (*cp != '.') && (*cp != '_') && (*cp != '-'))
			return (B_FALSE);
	}

	return (B_TRUE);
}

dladm_status_t
dladm_str2secobjclass(const char *str, dladm_secobj_class_t *class)
{
	int i;
	const secobj_class_info_t *sp;

	for (i = 0; i < NSECOBJCLASS; i++) {
		sp = &secobj_class_table[i];
		if (strcasecmp(str, sp->sc_name) == 0) {
			*class = i + 1;
			return (DLADM_STATUS_OK);
		}
	}
	return (DLADM_STATUS_BADARG);
}

const char *
dladm_secobjclass2str(dladm_secobj_class_t class, char *buf)
{
	const char		*s;

	if (!dladm_valid_secobjclass(class))
		s = "";
	else
		s = secobj_class_table[class-1].sc_name;

	(void) snprintf(buf, 10, "%s", s);
	return (buf);
}

dladm_status_t
dladm_str2identity(char *input_str, char *eap_usern)
{
	int flen;
	int i;
	if (input_str == NULL || eap_usern == NULL || *eap_usern != '\0')
		return (DLADM_STATUS_BADARG);

	flen = strnlen(input_str, DLADM_STRSIZE - 2);
	if (flen < 1 || flen == DLADM_STRSIZE - 2)
		return (DLADM_STATUS_BADVAL);

	if (input_str[flen - 1] == '\n') {
		flen--;
		if (flen == 0)
			return (DLADM_STATUS_BADVAL);
		input_str[flen] = '\0';
	}

	for (i = 0; i < flen; i++)
		if (iscntrl(input_str[i]))
			return (DLADM_STATUS_BADVAL);

	eap_usern[0] = '"';
	(void) memcpy(&eap_usern[1], input_str, flen);
	eap_usern[flen + 1] = '"';
	eap_usern[flen + 2] = '\0';
	return (DLADM_STATUS_OK);
}

dladm_status_t
dladm_str2crtname(char *option, char *eap_filename) {
	struct stat stbuf;
	int flen;
	int i;
	char str_tmp[PATH_MAX];
	KMF_ENCODE_FORMAT kfmt = 0;

	if (option == NULL || eap_filename == NULL || *eap_filename != '\0')
		return (DLADM_STATUS_BADARG);

	flen = strnlen(option, PATH_MAX);
	if (flen < 1 || flen >= PATH_MAX)
		return (DLADM_STATUS_BADVAL);

	if (option[flen - 1] == '\n') {
		flen--;
		if (flen == 0)
			return (DLADM_STATUS_BADVAL);
		option[flen] = '\0';
	}

	for (i = 0; i < flen; i++)
		if (iscntrl(option[i]))
			return (DLADM_STATUS_BADVAL);

	if (realpath(option, str_tmp) == NULL)
		return (DLADM_STATUS_BADVAL);
	if (stat(str_tmp, &stbuf) == -1)
		return (DLADM_STATUS_BADVAL);
	if (!(S_ISREG(stbuf.st_mode)))
		return (DLADM_STATUS_BADVAL);

	switch (kmf_get_file_format(str_tmp, &kfmt)) {
		case KMF_ERR_BAD_PARAMETER:
			return (DLADM_STATUS_BADARG);
		case KMF_ERR_OPEN_FILE:
			return (DLADM_STATUS_NOTFOUND);
		case KMF_OK:
			if (kfmt == KMF_FORMAT_ASN1 || kfmt == KMF_FORMAT_PEM ||
			    kfmt == KMF_FORMAT_PKCS12 ||
			    kfmt == KMF_FORMAT_PEM_KEYPAIR)
				break;
		/* FALLTHROUGH */
		case KMF_ERR_ENCODING:
			return (DLADM_STATUS_NOTSUP);
		default:
			return (DLADM_STATUS_FAILED);
	}

	(void) snprintf(eap_filename, PATH_MAX, "\"%s\"", str_tmp);
	return (DLADM_STATUS_OK);
}

static uint_t
print_p2names_by_mask(uint8_t val, const val_desc_t *vdp, uint_t cnt,
    char *buf, boolean_t namesonly)
{
	uint_t i = 0, len;
	int j;

	if (buf == NULL || buf[0] != '\0')
		return (0);

	buf[0] = '"';
	for (j = 0; j < cnt; j++) {
		if (val & vdp[j].vd_val) {
			char value[DLADM_STRSIZE];
			if (namesonly) {
				(void) snprintf(value, DLADM_STRSIZE, "%s ",
				    vdp[j].vd_name);
			} else if (strncmp(vdp[j].vd_name, "eap-", 4) == 0) {
				/* removes 'eap-' prefix */
				(void) snprintf(value, DLADM_STRSIZE,
				    "autheap=%s ", vdp[j].vd_name + 4);
			} else {
				(void) snprintf(value, DLADM_STRSIZE,
				    "auth=%s ", vdp[j].vd_name);
			}
			(void) strlcat(buf, value, DLADM_STRSIZE);
			i++;
		}
	}
	len = strnlen(buf, DLADM_STRSIZE);
	buf[len - 1] = '"';
	buf[len] = '\0';

	return (i);
}

uint_t
dladm_p2peap_2_str(uint8_t mask, char *phase2str, boolean_t namesonly)
{
	return (print_p2names_by_mask(mask, p2peap_vals, VALCNT(p2peap_vals),
	    phase2str, namesonly));
}

uint_t
dladm_p2ttls_2_str(uint8_t mask, char *phase2str, boolean_t namesonly)
{
	return (print_p2names_by_mask(mask, p2ttls_vals, VALCNT(p2ttls_vals),
	    phase2str, namesonly));
}

static uint_t
set_mask_by_string(char *str, const val_desc_t *vdp, uint_t cnt,
    uint8_t *valp)
{
	char *tokenp;
	uint_t i = 0;
	int j;

	*valp = 0;

	if ((tokenp = strtok(str, " \n\t")) == NULL)
		return (0);

	do {
		for (j = 0; j < cnt; j++) {
			if (strncasecmp(tokenp, vdp[j].vd_name,
			    DLADM_STRSIZE) == 0) {
				*valp |= vdp[j].vd_val;
				i++;
				break;
			}
		}
	} while ((tokenp = strtok(NULL, " \n\t")) != NULL);

	return (i);
}

uint_t
dladm_str_2_p2peap(char *str, uint8_t *p2peap_mask)
{
	return (set_mask_by_string(str, p2peap_vals, VALCNT(p2peap_vals),
	    p2peap_mask));
}

uint_t
dladm_str_2_p2ttls(char *str, uint8_t *p2ttls_mask)
{
	return (set_mask_by_string(str, p2ttls_vals, VALCNT(p2ttls_vals),
	    p2ttls_mask));
}

dladm_status_t
dladm_str2secobjval(char *buf, uint8_t *obj_val, uint16_t *obj_lenp,
    dladm_secobj_class_t class)
{
	int i;
	uint16_t buf_len;

	if (buf == NULL || obj_val == NULL || obj_lenp == NULL)
		return (DLADM_STATUS_BADARG);

	buf_len = strnlen(buf, DLADM_SECOBJ_VAL_MAX);
	if (buf_len == DLADM_SECOBJ_VAL_MAX || buf_len == 0)
		return (DLADM_STATUS_BADARG);

	if (iscntrl(buf[buf_len - 1])) {
		buf[buf_len - 1] = '\0';
		buf_len--;
	}

	for (i = 0; i < buf_len; i++)
		if (!(isprint(buf[i])))
			return (DLADM_STATUS_BADVAL);

	(void) memset(obj_val, 0, DLADM_SECOBJ_VAL_MAX);

	if (class == DLADM_SECOBJ_CLASS_PSK) {
		if (buf_len >= 8 && buf_len <= 63) {
			*obj_val = '"';
			(void) memcpy(obj_val + 1, buf, buf_len);
			*(obj_val+buf_len + 1) = '"';
			*obj_lenp = buf_len + 2;
		} else if (buf_len == 64) {
			for (i = 0; i < buf_len; i++)
				if (!(isxdigit(buf[i])))
					return (DLADM_STATUS_BADVAL);
			(void) memcpy(obj_val, buf, buf_len);
			*obj_lenp = buf_len;
		} else
			return (DLADM_STATUS_BADVALCNT);
	} else if (class == DLADM_SECOBJ_CLASS_WEP) {
		switch (buf_len) {
		case 5:
		case 13:
			/* ASCII key sizes  */
			*obj_val = '"';
			(void) memcpy(obj_val + 1, buf, buf_len);
			*(obj_val + buf_len + 1) = '"';
			*obj_lenp = buf_len + 2;
			break;

		case 10:
		case 26:
			/* Hex key sizes, not preceded by 0x (64bit/128bit) */
			for (i = 0; i < buf_len; i++)
				if (!(isxdigit(buf[i])))
					return (DLADM_STATUS_BADVAL);
			(void) memcpy(obj_val, buf, buf_len);
			*obj_lenp = buf_len;
			break;

		case 12:
		case 28:
			/* Hex key sizes, preceded by 0x (64bit/128bit) */
			if (strncmp(buf, "0x", 2) != 0) {
				return (DLADM_STATUS_BADVAL);
			} else {
				for (i = 2; i < buf_len; i++)
					if (!(isxdigit(buf[i])))
						return (DLADM_STATUS_BADVAL);
			}
			(void) memcpy(obj_val + 2, buf, buf_len - 2);
			*obj_lenp = buf_len - 2;
			break;
		default:
			return (DLADM_STATUS_BADVALCNT);
		}
	} else if (class == DLADM_SECOBJ_CLASS_TTLS ||
	    class == DLADM_SECOBJ_CLASS_PEAP ||
	    class == DLADM_SECOBJ_CLASS_TLS) {
		/*
		 * When using ENGINE, Private Key Password (eap-tls)
		 * needs to be unquoted. This is done in eap_pk11.
		 */
		*obj_val = '"';
		(void) memcpy(obj_val + 1, buf, buf_len);
		*(obj_val + buf_len + 1) = '"';
		*obj_lenp = buf_len + 2;
	} else
		return (DLADM_STATUS_NOTSUP);

	return (DLADM_STATUS_OK);
}

dladm_status_t
dladm_secobj_prompt(dladm_wlan_key_t *kdata, FILE *filep)
{
	dladm_status_t status = DLADM_STATUS_OK;
	char tmpbuf[DLADM_STRSIZE];

	switch (kdata->wk_class) {
	case DLADM_SECOBJ_CLASS_PSK:
		break;
	case DLADM_SECOBJ_CLASS_WEP:
		do {
			tmpbuf[0] = '\0';
			(void) printf(gettext("\nEnter WEP Key Slot [1-4]: "));
			(void) fflush(stdin);
			(void) fgets(tmpbuf, 2, stdin);
		} while (tmpbuf[0] != '1' && tmpbuf[0] != '2' &&
		    tmpbuf[0] != '3' && tmpbuf[0] != '4');
		kdata->wk_idx = tmpbuf[0];
		break;
	case DLADM_SECOBJ_CLASS_TTLS:
		do {
			(void) memset(tmpbuf, 0, DLADM_STRSIZE);
			(void) printf(gettext("\nEnter one or more "
			    "space-separated inner authentication methods "
			    "among these\n"));
			(void) dladm_p2ttls_2_str(0xFF, tmpbuf, B_TRUE);
			(void) printf("\n%s", tmpbuf);
			(void) memset(tmpbuf, 0, DLADM_STRSIZE);
			(void) printf(gettext("\n\nEnter EAP-TTLS Phase2 : "));
			(void) fflush(stdin);
			(void) fgets(tmpbuf, DLADM_STRSIZE, stdin);
		} while (dladm_str_2_p2ttls(tmpbuf, &kdata->wk_p2mask) == 0);
		break;
	case DLADM_SECOBJ_CLASS_PEAP:
		do {
			(void) memset(tmpbuf, 0, DLADM_STRSIZE);
			(void) printf(gettext("\nEnter one or more "
			    "space-separated inner authentication methods "
			    "among these\n"));
			(void) dladm_p2peap_2_str(0xFF, tmpbuf, B_TRUE);
			(void) printf("\n%s", tmpbuf);
			(void) memset(tmpbuf, 0, DLADM_STRSIZE);
			(void) printf(gettext("\n\nEnter EAP-PEAP Phase2 : "));
			(void) fflush(stdin);
			(void) fgets(tmpbuf, DLADM_STRSIZE, stdin);
		} while (dladm_str_2_p2peap(tmpbuf, &kdata->wk_p2mask) == 0);
		break;
	case DLADM_SECOBJ_CLASS_TLS:
		for (;;) {
			tmpbuf[0] = '\0';
			(void) printf(gettext("\nUse PKCS#11 keystore to manage"
			    " EAP-TLS private key and client certificate? "
			    "[y/n]:"));
			(void) fflush(stdin);
			(void) fgets(tmpbuf, 2, stdin);
			if (tmpbuf[0] == 'y' || tmpbuf[0] == 'Y') {
				(void) printf(gettext("\nPrivate key passphrase"
				    " won't be stored permanently in secobj "
				    "db\n"));
				kdata->wk_engine = B_TRUE;
				break;
			} else if (tmpbuf[0] == 'n' || tmpbuf[0] == 'N') {
				(void) printf(gettext("The private key "
				    "passphrase will be stored permanently "
				    "in secobj db\n"));
				kdata->wk_engine = B_FALSE;
				break;
			}
		}
		break;
	default:
		return (DLADM_STATUS_NOTSUP);
	}

	if (filep == NULL) {
		char *buf = NULL;
		uint8_t *buf2 = NULL;
		uint16_t len = 0;

		(void) memset(tmpbuf, 0, sizeof (tmpbuf));

		(void) snprintf(tmpbuf, sizeof (tmpbuf), gettext("\nProvide "
		    "value for %s\n  > "), key_prompts[kdata->wk_class]);

		buf = getpassphrase(tmpbuf);
		if (buf == NULL) {
			status = DLADM_STATUS_FAILED;
			goto err;
		}

		status = dladm_str2secobjval(buf, kdata->wk_val,
		    &kdata->wk_len, kdata->wk_class);
		if (status != DLADM_STATUS_OK)
			goto err;

		(void) snprintf(tmpbuf, sizeof (tmpbuf), gettext("\nConfirm "
		    "value for %s\n  > "), key_prompts[kdata->wk_class]);

		buf = getpassphrase(tmpbuf);
		if (buf == NULL) {
			status = DLADM_STATUS_FAILED;
			goto err;
		}

		buf2 = malloc(DLADM_SECOBJ_VAL_MAX);
		if (buf2 == NULL) {
			status = DLADM_STATUS_FAILED;
			goto err;
		}

		status = dladm_str2secobjval(buf, buf2, &len, kdata->wk_class);
		if (status != DLADM_STATUS_OK)
			goto err;

		if (len != kdata->wk_len || memcmp(buf2, kdata->wk_val,
		    kdata->wk_len) != 0)
			status = DLADM_STATUS_BADPROP;

		free(buf2);
	} else {
		char buf[DLADM_SECOBJ_VAL_MAX];
		int len;

		if (fgets(buf, sizeof (buf), filep) == NULL) {
			(void) fclose(filep);
			return (DLADM_STATUS_DENIED);
		}

		(void) fclose(filep);

		len = strnlen(buf, DLADM_SECOBJ_VAL_MAX);
		if ((len - 1) == '\n')
			buf[len - 1] = '\0';
		status = dladm_str2secobjval(buf, kdata->wk_val, &kdata->wk_len,
		    kdata->wk_class);
	}

err:
	if (!(isatty(fileno(stdout))))
		return (status);

	switch (status) {
		case DLADM_STATUS_BADVALCNT:
			(void) printf(gettext("Invalid secure object "
			    "length\n"));
			break;
		case DLADM_STATUS_TOOSMALL:
			(void) printf(gettext("Zero-length secure object\n"));
			break;
		case DLADM_STATUS_NOTSUP:
			(void) printf(gettext("Invalid secure object class\n"));
			break;
		case DLADM_STATUS_BADVAL:
			(void) printf(gettext("Invalid secure object "
			    "characters\n"));
			break;
		case DLADM_STATUS_BADPROP:
			(void) printf(gettext("Verification failed. Secure "
			    "objects do not match\n"));
			break;
		case DLADM_STATUS_OK:
			break;
		default:
			break;
	}

	return (status);
}

/*
 * Walk security objects looking for one that matches the keyname.
 * Store the keyclass if a match is found - we use the last match
 * since it is the most recently updated.
 */
boolean_t
/* LINTED E_FUNC_ARG_UNUSED */
find_matching_secobj(dladm_handle_t dh, void *arg, const char *secobjname,
    dladm_secobj_class_t secobjclass)
{
	secobj_class_info_t *winfo = arg;

	if (strnlen(secobjname, DLADM_SECOBJ_NAME_MAX) ==
	    strnlen(winfo->sc_name, DLADM_SECOBJ_NAME_MAX) &&
	    strncmp(winfo->sc_name, secobjname, DLADM_SECOBJ_NAME_MAX) == 0) {
		winfo->sc_dladmclass = secobjclass;
		return (B_FALSE);
	}
	return (B_TRUE);
}

static void
get_first_byte(dladm_wlan_key_t *data, uint8_t fbyte)
{
	switch (data->wk_class) {
	case DLADM_SECOBJ_CLASS_WEP:
		data->wk_idx = fbyte;
		break;
	case DLADM_SECOBJ_CLASS_PSK:
		break;
	case DLADM_SECOBJ_CLASS_TTLS:
	case DLADM_SECOBJ_CLASS_PEAP:
		data->wk_p2mask = fbyte;
		break;
	case DLADM_SECOBJ_CLASS_TLS:
		data->wk_engine = (fbyte == 1) ? B_TRUE : B_FALSE;
		break;
	default:
		break;
	}
}

static void
set_first_byte(dladm_wlan_key_t *data, uint8_t *fbyte)
{
	*fbyte = 0;
	switch (data->wk_class) {
	case DLADM_SECOBJ_CLASS_WEP:
		*fbyte = data->wk_idx;
		break;
	case DLADM_SECOBJ_CLASS_PSK:
		break;
	case DLADM_SECOBJ_CLASS_TTLS:
	case DLADM_SECOBJ_CLASS_PEAP:
		*fbyte = data->wk_p2mask;
		break;
	case DLADM_SECOBJ_CLASS_TLS:
		if (data->wk_engine)
			*fbyte = 0x31;
		break;
	default:
		break;
	}
}

dladm_status_t
dladm_set_secobj(dladm_handle_t handle, dladm_wlan_key_t *data, uint_t flags)
{
	secobj_class_info_t	key_if;

	if (data == NULL || flags == 0 || data->wk_len == 0 ||
	    !dladm_valid_secobj_name(data->wk_name) ||
	    !dladm_valid_secobjclass(data->wk_class))
		return (DLADM_STATUS_BADARG);

	/* check if this name has already been used */
	key_if.sc_name = data->wk_name;
	key_if.sc_dladmclass = 0;
	if (dladm_walk_secobj(handle, &key_if, find_matching_secobj,
	    DLADM_OPT_PERSIST | DLADM_OPT_ACTIVE))
		return (DLADM_STATUS_FAILED);

	if (dladm_valid_secobjclass(key_if.sc_dladmclass))
		return (DLADM_STATUS_EXIST);

	if (flags & DLADM_OPT_ACTIVE) {
		dld_ioc_secobj_set_t	secobj_set;
		dld_secobj_t		*objp;

		bzero(&secobj_set, sizeof (secobj_set));
		objp = &secobj_set.ss_obj;

		set_first_byte(data, &objp->so_val[0]);

		objp->so_class = (dld_secobj_class_t)data->wk_class;
		(void) strlcpy(objp->so_name, data->wk_name,
		    DLADM_SECOBJ_NAME_MAX);
		bcopy(data->wk_val, &objp->so_val[1], data->wk_len);
		objp->so_len = data->wk_len + 1;

		if (flags & DLADM_OPT_CREATE)
			secobj_set.ss_flags = DLD_SECOBJ_OPT_CREATE;

		if (ioctl(dladm_dld_fd(handle),
		    DLDIOC_SECOBJ_SET, &secobj_set) < 0)
			return (dladm_errno2status(errno));
	}

	if (flags & DLADM_OPT_PERSIST)
		return (i_dladm_set_secobj_db(handle, data));

	return (DLADM_STATUS_OK);
}

dladm_status_t
dladm_get_secobj(dladm_handle_t handle, dladm_wlan_key_t *data, uint_t flags)
{
	if (data == NULL || flags == 0 || data->wk_name[0] == '\0')
		return (DLADM_STATUS_BADARG);

	if (flags & DLADM_OPT_PERSIST) {
		dladm_status_t status = i_dladm_get_secobj_db(handle, data);
		if (status == DLADM_STATUS_OK ||
		    status != DLADM_STATUS_NOTFOUND)
			return (status);
	}

	if (flags & DLADM_OPT_ACTIVE) {
		dld_ioc_secobj_get_t	secobj_get;
		dld_secobj_t		*objp;

		bzero(&secobj_get, sizeof (secobj_get));
		objp = &secobj_get.sg_obj;
		(void) strlcpy(objp->so_name, data->wk_name,
		    DLADM_SECOBJ_NAME_MAX);

		secobj_get.sg_size = sizeof (secobj_get);

		if (ioctl(dladm_dld_fd(handle),
		    DLDIOC_SECOBJ_GET, &secobj_get) < 0)
			return (dladm_errno2status(errno));

		data->wk_class = (dladm_secobj_class_t)objp->so_class;
		get_first_byte(data, objp->so_val[0]);
		data->wk_len = objp->so_len - 1;
		bcopy(&objp->so_val[1], data->wk_val, data->wk_len);
	}

	return (DLADM_STATUS_OK);
}

dladm_status_t
dladm_unset_secobj(dladm_handle_t handle, const char *obj_name, uint_t flags)
{
	if (obj_name == NULL || strlen(obj_name) >= DLADM_SECOBJ_NAME_MAX ||
	    flags == 0)
		return (DLADM_STATUS_BADARG);

	if (flags & DLADM_OPT_ACTIVE) {
		dld_ioc_secobj_unset_t	secobj_unset;
		bzero(&secobj_unset, sizeof (secobj_unset));

		(void) strlcpy(secobj_unset.su_name, obj_name,
		    DLADM_SECOBJ_NAME_MAX);

		if (ioctl(dladm_dld_fd(handle), DLDIOC_SECOBJ_UNSET,
		    &secobj_unset) < 0)
			return (dladm_errno2status(errno));
	}

	if (flags & DLADM_OPT_PERSIST)
		return (i_dladm_unset_secobj_db(handle, obj_name));

	return (DLADM_STATUS_OK);
}

dladm_status_t
dladm_walk_secobj(dladm_handle_t handle, void *arg,
    boolean_t (*func)(dladm_handle_t, void *, const char *,
    dladm_secobj_class_t), uint_t flags)
{
	dladm_status_t		status = DLADM_STATUS_OK;
	dld_ioc_secobj_get_t	*secobj_getp;
	dld_secobj_t		*objp;
	size_t			secobj_bufsz;

	if (flags == 0 || arg == NULL || func == NULL)
		return (DLADM_STATUS_BADARG);

	if (flags & DLADM_OPT_PERSIST)
		status = i_dladm_walk_secobj_db(handle, arg, func);

	if (status != DLADM_STATUS_OK || !(flags & DLADM_OPT_ACTIVE))
		return (status);

	/* Start with enough room for 10 objects, increase if necessary. */
	secobj_bufsz = sizeof (*secobj_getp) + (10 * sizeof (*objp));
	secobj_getp = calloc(1, secobj_bufsz);
	if (secobj_getp == NULL) {
		status = dladm_errno2status(errno);
		goto done;
	}

tryagain:
	secobj_getp->sg_size = secobj_bufsz;
	if (ioctl(dladm_dld_fd(handle), DLDIOC_SECOBJ_GET, secobj_getp) < 0) {
		if (errno == ENOSPC) {
			/* Increase the buffer size and try again. */
			secobj_bufsz *= 2;
			if (secobj_bufsz > SECOBJ_MAXBUFSZ) {
				status = dladm_errno2status(errno);
				goto done;
			}
			secobj_getp = realloc(secobj_getp, secobj_bufsz);
			if (secobj_getp == NULL) {
				status = dladm_errno2status(errno);
				goto done;
			}
			bzero(secobj_getp, secobj_bufsz);
			goto tryagain;
		}
		status = dladm_errno2status(errno);
		goto done;
	}

	objp = (dld_secobj_t *)(secobj_getp + 1);
	while (secobj_getp->sg_count > 0) {
		if (!func(handle, arg, objp->so_name,
		    (dladm_secobj_class_t)objp->so_class))
			goto done;
		secobj_getp->sg_count--;
		objp++;
	}
done:
	free(secobj_getp);
	return (status);
}

/*
 * Private data structures used for persistent secure objects back-end
 */
typedef struct secobj_info {
	const char		*si_name;
	dladm_secobj_class_t	si_class;
	uint8_t			*si_val;
	uint16_t		si_len;
} secobj_info_t;

typedef struct secobj_name {
	char			*sn_name;
	dladm_secobj_class_t    sn_class;
	struct secobj_name	*sn_next;
} secobj_name_t;

typedef struct secobj_db_state	secobj_db_state_t;

typedef boolean_t (*secobj_db_op_t)(dladm_handle_t, struct secobj_db_state *,
    char *, secobj_info_t *, dladm_status_t *);

struct secobj_db_state {
	secobj_db_op_t		ss_op;
	secobj_info_t		ss_info;
	secobj_name_t		**ss_namelist;
};

/*
 * Update or generate a secobj entry using the info in ssp->ss_info.
 */
/* ARGSUSED */
static boolean_t
process_secobj_set(dladm_handle_t handle, secobj_db_state_t *ssp, char *buf,
    secobj_info_t *sip, dladm_status_t *statusp)
{
	char	tmpbuf[MAXLINELEN];
	char	classbuf[DLADM_STRSIZE];
	char	*ptr = tmpbuf, *lim = tmpbuf + MAXLINELEN;
	int	i;

	sip = &ssp->ss_info;

	ptr += snprintf(ptr, BUFLEN(lim, ptr), "%s\t", sip->si_name);
	ptr += snprintf(ptr, BUFLEN(lim, ptr), "%s\t",
	    dladm_secobjclass2str(sip->si_class, classbuf));

	/* new syntax lines begin with 1x, old ones with 0x */
	ptr += snprintf(ptr, BUFLEN(lim, ptr), "1x");

	for (i = 0; i < sip->si_len; i++) {
		ptr += snprintf(ptr, BUFLEN(lim, ptr), "%02x",
		    sip->si_val[i] & 0xff);
	}

	if (ptr > lim) {
		*statusp = DLADM_STATUS_TOOSMALL;
		return (B_FALSE);
	}
	(void) snprintf(buf, MAXLINELEN, "%s\n", tmpbuf);
	return (B_FALSE);
}

/* ARGSUSED */
static boolean_t
process_secobj_get(dladm_handle_t handle, secobj_db_state_t *ssp, char *buf,
    secobj_info_t *sip, dladm_status_t *statusp)
{
	bcopy(sip->si_val, ssp->ss_info.si_val, sip->si_len);
	ssp->ss_info.si_len = sip->si_len;
	ssp->ss_info.si_class = sip->si_class;
	return (B_FALSE);
}

/* ARGSUSED */
static boolean_t
process_secobj_unset(dladm_handle_t handle, secobj_db_state_t *ssp, char *buf,
    secobj_info_t *sip, dladm_status_t *statusp)
{
	/*
	 * Delete line.
	 */
	buf[0] = '\0';
	return (B_FALSE);
}

/* ARGSUSED */
static boolean_t
process_secobj_walk(dladm_handle_t handle, secobj_db_state_t *ssp, char *buf,
    secobj_info_t *sip, dladm_status_t *statusp)
{
	secobj_name_t	*snp;

	if ((snp = malloc(sizeof (*snp))) == NULL)
		return (B_TRUE);

	if ((snp->sn_name = strdup(sip->si_name)) == NULL) {
		free(snp);
		return (B_TRUE);
	}

	snp->sn_class = sip->si_class;
	snp->sn_next = NULL;
	*ssp->ss_namelist = snp;
	ssp->ss_namelist = &snp->sn_next;
	return (B_TRUE);
}

static int
parse_secobj_val(char *buf, secobj_info_t *sip)
{
	uint_t tlen = DLD_SECOBJ_VAL_MAX, ret = 0;

	/* new syntax lines begin with 1x, old ones with 0x */
	if (strncmp(buf, "1x", 2) != 0)
		return (EINVAL);
	ret = hexascii_to_octet(buf + 2, strlen(buf) - 2, sip->si_val, &tlen);
	sip->si_len = tlen;
	return (ret);
}

static boolean_t
process_secobj_line(dladm_handle_t handle, secobj_db_state_t *ssp, char *buf,
    dladm_status_t *statusp)
{
	secobj_info_t		sinfo;
	uint8_t			val[DLD_SECOBJ_VAL_MAX];

	int			i, len, nlen;
	char			*str, *lasts;

	/*
	 * Skip leading spaces, blank lines, and comments.
	 */
	len = strlen(buf);
	for (i = 0; i < len; i++) {
		if (!isspace(buf[i]))
			break;
	}
	if (i == len || buf[i] == '#')
		return (B_TRUE);

	str = buf + i;
	if (ssp->ss_info.si_name != NULL) {
		/*
		 * Skip objects we're not interested in.
		 */
		nlen = strlen(ssp->ss_info.si_name);
		if (strncmp(str, ssp->ss_info.si_name, nlen) != 0 ||
		    !isspace(str[nlen]))
			return (B_TRUE);

		sinfo.si_name = ssp->ss_info.si_name;
	} else {
		/*
		 * If an object is not specified, find the object name
		 * and assign it to sinfo.si_name.
		 */
		if (strtok_r(str, " \n\t", &lasts) == NULL)
			goto fail;

		nlen = strlen(str);
		sinfo.si_name = str;
	}
	str += nlen + 1;
	if (str >= buf + len)
		goto fail;

	/*
	 * Find the class name.
	 */
	if ((str = strtok_r(str, " \n\t", &lasts)) == NULL)
		goto fail;

	*statusp = dladm_str2secobjclass(str, &sinfo.si_class);
	if (*statusp != DLADM_STATUS_OK)
		goto fail;

	/*
	 * Find the object value.
	 */
	if ((str = strtok_r(NULL, " \n\t", &lasts)) == NULL)
		goto fail;

	sinfo.si_val = val;
	if (parse_secobj_val(str, &sinfo) != 0)
		goto fail;

	return ((*ssp->ss_op)(handle, ssp, buf, &sinfo, statusp));

fail:
	/*
	 * Delete corrupted line.
	 */
	buf[0] = '\0';
	return (B_TRUE);
}

static dladm_status_t
process_secobj_db(dladm_handle_t handle, void *arg, FILE *fp, FILE *nfp)
{
	secobj_db_state_t	*ssp = arg;
	dladm_status_t		status = DLADM_STATUS_OK;
	char			buf[MAXLINELEN];
	boolean_t		cont = B_TRUE;

	(void) memset(buf, 0, sizeof (buf));
	/*
	 * This loop processes each line of the configuration file.
	 * buf can potentially be modified by process_secobj_line().
	 * If this is a write operation and buf is not truncated, buf will
	 * be written to disk. process_secobj_line() will no longer be
	 * called after it returns B_FALSE; at which point the remainder
	 * of the file will continue to be read and, if necessary, written
	 * to disk as well.
	 */
	while (fgets(buf, MAXLINELEN, fp) != NULL) {
		if (cont)
			cont = process_secobj_line(handle, ssp, buf, &status);

		if (nfp != NULL && buf[0] != '\0' && fputs(buf, nfp) == EOF) {
			status = dladm_errno2status(errno);
			break;
		}
	}
	if (status != DLADM_STATUS_OK || !cont)
		return (status);

	if (ssp->ss_op == process_secobj_set) {
		/*
		 * If the specified object is not found above, we add the
		 * object to the configuration file.
		 */
		(void) (*ssp->ss_op)(handle, ssp, buf, NULL, &status);
		if (status == DLADM_STATUS_OK && fputs(buf, nfp) == EOF)
			status = dladm_errno2status(errno);
	}

	if (ssp->ss_op == process_secobj_unset ||
	    ssp->ss_op == process_secobj_get)
		status = DLADM_STATUS_NOTFOUND;

	return (status);
}

#define	SECOBJ_RW_DB(handle, statep, writeop)			\
	(i_dladm_rw_db(handle, "/etc/dladm/secobj.conf",	\
	S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP,			\
	process_secobj_db, (statep), (writeop)))

static dladm_status_t
i_dladm_set_secobj_db(dladm_handle_t handle, dladm_wlan_key_t *keyp)
{
	secobj_db_state_t	state;
	uint8_t 		tmpval[DLD_SECOBJ_VAL_MAX];

	(void) memset(tmpval, 0, sizeof (tmpval));
	set_first_byte(keyp, &tmpval[0]);
	(void) memcpy(&tmpval[1], keyp->wk_val, keyp->wk_len);
	keyp->wk_len += 1;

	state.ss_op = process_secobj_set;
	state.ss_info.si_name = keyp->wk_name;
	state.ss_info.si_class = keyp->wk_class;
	state.ss_info.si_val = tmpval;
	state.ss_info.si_len = keyp->wk_len;
	state.ss_namelist = NULL;

	return (SECOBJ_RW_DB(handle, &state, B_TRUE));
}

static dladm_status_t
i_dladm_get_secobj_db(dladm_handle_t handle, dladm_wlan_key_t *keyp)
{
	secobj_db_state_t	state;
	dladm_status_t		status;
	uint8_t 		tmpval[DLD_SECOBJ_VAL_MAX];

	state.ss_op = process_secobj_get;
	state.ss_info.si_name = keyp->wk_name;
	state.ss_info.si_val = tmpval;
	state.ss_namelist = NULL;

	(void) memset(tmpval, 0, sizeof (tmpval));

	status = SECOBJ_RW_DB(handle, &state, B_FALSE);
	if (status == DLADM_STATUS_OK) {
		keyp->wk_len = state.ss_info.si_len;
		keyp->wk_class = state.ss_info.si_class;
		get_first_byte(keyp, *state.ss_info.si_val);
		keyp->wk_len--;
		bcopy(state.ss_info.si_val + 1, keyp->wk_val, keyp->wk_len);
		keyp->wk_val[keyp->wk_len] = 0;
	} else if (status == DLADM_STATUS_NOTSUP) {
		/* delete corrupted/old format entries */
		state.ss_op = process_secobj_walk;
		state.ss_info.si_name = NULL;
		state.ss_info.si_class = 0;
		state.ss_info.si_val = NULL;
		state.ss_info.si_len = 0;
		state.ss_namelist = NULL;
		(void) SECOBJ_RW_DB(handle, &state, B_TRUE);
		return (DLADM_STATUS_OK);
	}

	return (status);
}

static dladm_status_t
i_dladm_unset_secobj_db(dladm_handle_t handle, const char *obj_name)
{
	secobj_db_state_t	state;

	state.ss_op = process_secobj_unset;
	state.ss_info.si_name = obj_name;
	state.ss_info.si_class = 0;
	state.ss_info.si_val = NULL;
	state.ss_info.si_len = 0;
	state.ss_namelist = NULL;

	return (SECOBJ_RW_DB(handle, &state, B_TRUE));
}

static dladm_status_t
i_dladm_walk_secobj_db(dladm_handle_t handle, void *arg,
    boolean_t (*func)(dladm_handle_t, void *, const char *,
    dladm_secobj_class_t))
{
	secobj_db_state_t	state;
	secobj_name_t		*snp = NULL, *fsnp;
	dladm_status_t		status;
	boolean_t		cont = B_TRUE;

	state.ss_op = process_secobj_walk;
	state.ss_info.si_name = NULL;
	state.ss_info.si_class = 0;
	state.ss_info.si_val = NULL;
	state.ss_info.si_len = 0;
	state.ss_namelist = &snp;

	status = SECOBJ_RW_DB(handle, &state, B_FALSE);
	if (status == DLADM_STATUS_NOTSUP) {
		/* delete corrupted/old format entries */
		(void) SECOBJ_RW_DB(handle, &state, B_TRUE);
		return (DLADM_STATUS_OK);
	}

	while (snp != NULL) {
		fsnp = snp;
		snp = snp->sn_next;
		if (cont)
			cont = func(handle, arg, fsnp->sn_name, fsnp->sn_class);
		free(fsnp->sn_name);
		free(fsnp);
	}
	return (status);
}
