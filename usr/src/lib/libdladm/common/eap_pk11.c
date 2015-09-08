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
 * Copyright (c) 2004, 2010, Oracle and/or its affiliates. All rights reserved.
 * Copyright 2012 Enrico Papi <enricop@computer.org>.  All rights reserved.
 */

/*
 * PKCS#11 keystore keys/certificates import support routines.
 * Required for using OpenSSL ENGINE when connecting to EAP-TLS protected
 * network with wpa_supplicant.
 *
 * The code is based on 'pktool'
 */

#include <errno.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <cryptoutil.h>
#include <kmfapi.h>
#include <secobj.h>

#define	PRINT_PIN_FILE	"#!/bin/sh\n/usr/gnu/bin/echo -n "
#define	EMPTYSTRING(s)	(s == NULL || !strlen(s))

static KMF_RETURN
select_token_pk11(void *kmfhandle, int readonly)
{
	KMF_ATTRIBUTE attlist[10];
	KMF_KEYSTORE_TYPE kstype = KMF_KEYSTORE_PK11TOKEN;
	KMF_RETURN rv;

	kmf_set_attr_at_index(attlist, 0, KMF_KEYSTORE_TYPE_ATTR, &kstype,
	    sizeof (kstype));

	kmf_set_attr_at_index(attlist, 1, KMF_TOKEN_LABEL_ATTR,
	    SOFT_TOKEN_LABEL, strlen(SOFT_TOKEN_LABEL));

	kmf_set_attr_at_index(attlist, 2, KMF_READONLY_ATTR, &readonly,
	    sizeof (readonly));

	rv = kmf_configure_keystore(kmfhandle, 3, attlist);
	if (rv == KMF_ERR_TOKEN_SELECTED)
		rv = KMF_OK;
	return (rv);
}

static int
setpin_pkcs11(KMF_HANDLE_T handle, char *new_pin, uint32_t new_pin_len)
{
	ulong_t		slot_id;
	int		rv;
	KMF_CREDENTIAL	newcred = {NULL, 0};
	KMF_CREDENTIAL	oldcred = {NULL, 0};
	KMF_KEYSTORE_TYPE kstype = KMF_KEYSTORE_PK11TOKEN;
	KMF_ATTRIBUTE	attrlist[6];
	char 		*token_spec = SOFT_TOKEN_LABEL ":" SOFT_MANUFACTURER_ID;

	rv = kmf_pk11_token_lookup(NULL, token_spec, &slot_id);
	if (rv != KMF_OK)
		return (-1);

	if ((oldcred.cred = strdup(SOFT_DEFAULT_PIN)) == NULL)
		return (-1);
	oldcred.credlen = strlen(SOFT_DEFAULT_PIN);

	kmf_set_attr_at_index(attrlist, 0, KMF_KEYSTORE_TYPE_ATTR,
	    &kstype, sizeof (kstype));

	kmf_set_attr_at_index(attrlist, 1, KMF_TOKEN_LABEL_ATTR,
	    SOFT_TOKEN_LABEL, strlen(SOFT_TOKEN_LABEL));

	kmf_set_attr_at_index(attrlist, 2, KMF_CREDENTIAL_ATTR, &oldcred,
	    sizeof (oldcred));

	kmf_set_attr_at_index(attrlist, 3, KMF_SLOT_ID_ATTR, &slot_id,
	    sizeof (slot_id));

	newcred.cred = new_pin;
	newcred.credlen = new_pin_len;
	kmf_set_attr_at_index(attrlist, 4, KMF_NEWPIN_ATTR, &newcred,
	    sizeof (newcred));

	rv = kmf_set_token_pin(handle, 5, attrlist);

	/* Clean up. */
	if (oldcred.cred != NULL)
		free(oldcred.cred);

	return (rv);
}

static KMF_RETURN
destroy_keys(void *handle, KMF_ATTRIBUTE *attrlist, int numattr)
{
	int i;
	KMF_RETURN rv;
	uint32_t *numkeys;
	KMF_KEY_HANDLE *keys;
	int del_num = 0;
	KMF_ATTRIBUTE delete_attlist[16];
	KMF_KEYSTORE_TYPE kstype = KMF_KEYSTORE_PK11TOKEN;
	boolean_t destroy = B_TRUE;
	KMF_CREDENTIAL cred;

	kmf_set_attr_at_index(delete_attlist, del_num,
	    KMF_KEYSTORE_TYPE_ATTR, &kstype, sizeof (kstype));
	del_num++;

	kmf_set_attr_at_index(delete_attlist, del_num,
	    KMF_DESTROY_BOOL_ATTR, &destroy, sizeof (boolean_t));
	del_num++;

	rv = kmf_get_attr(KMF_CREDENTIAL_ATTR, attrlist, numattr, &cred, NULL);
	if (rv == KMF_OK) {
		if (cred.credlen > 0) {
			kmf_set_attr_at_index(delete_attlist, del_num,
			    KMF_CREDENTIAL_ATTR, &cred,
			    sizeof (KMF_CREDENTIAL));
			del_num++;
		}
	}

	numkeys = kmf_get_attr_ptr(KMF_COUNT_ATTR, attrlist, numattr);
	if (numkeys == NULL)
		return (KMF_ERR_KEY_NOT_FOUND);

	keys = kmf_get_attr_ptr(KMF_KEY_HANDLE_ATTR, attrlist, numattr);
	if (keys == NULL)
		return (KMF_ERR_KEY_NOT_FOUND);

	for (i = 0; rv == KMF_OK && i < *numkeys; i++) {
		int num = del_num;

		kmf_set_attr_at_index(delete_attlist, num,
		    KMF_KEY_HANDLE_ATTR, &keys[i], sizeof (KMF_KEY_HANDLE));
		num++;

		rv = kmf_delete_key_from_keystore(handle, num, delete_attlist);
	}
	return (rv);
}

static KMF_RETURN
delete_keys(KMF_HANDLE_T kmfhandle, KMF_ATTRIBUTE *attlist, int numattr,
    int *keysdeleted)
{
	KMF_RETURN rv;
	uint32_t numkeys;
	int num = numattr;

	*keysdeleted = 0;
	numkeys = 0;

	kmf_set_attr_at_index(attlist, num, KMF_COUNT_ATTR, &numkeys,
	    sizeof (uint32_t));
	num++;

	rv = kmf_find_key(kmfhandle, num, attlist);

	if (rv == KMF_OK && numkeys > 0) {
		KMF_KEY_HANDLE *keys;
		keys = (KMF_KEY_HANDLE *)malloc(numkeys *
		    sizeof (KMF_KEY_HANDLE));
		if (keys == NULL)
			return (KMF_ERR_MEMORY);
		(void) memset(keys, 0, numkeys *sizeof (KMF_KEY_HANDLE));

		kmf_set_attr_at_index(attlist, num, KMF_KEY_HANDLE_ATTR, keys,
		    sizeof (KMF_KEY_HANDLE));

		rv = kmf_find_key(kmfhandle, num, attlist);
		if (rv == KMF_OK)
			rv = destroy_keys(kmfhandle, attlist, num);

		free(keys);
	}

	*keysdeleted = numkeys;
	return (rv);
}

static KMF_RETURN
delete_pk11_keys(KMF_HANDLE_T kmfhandle, KMF_CREDENTIAL *tokencred,
    char *objlabel)
{
	KMF_RETURN rv = KMF_OK;
	int nk;
	KMF_KEYSTORE_TYPE kstype = KMF_KEYSTORE_PK11TOKEN;
	int numattr = 0;
	KMF_ATTRIBUTE attrlist[16];
	/* User is supposed to import only private keys */
	KMF_KEY_CLASS keyclass = KMF_ASYM_PRI;
	boolean_t token_bool = B_TRUE;
	boolean_t private = B_TRUE;
	/*
	 * Symmetric keys and RSA/DSA private keys are always
	 * created with the "CKA_PRIVATE" field == TRUE, so
	 * make sure we search for them with it also set.
	 */

	rv = select_token_pk11(kmfhandle, FALSE);
	if (rv != KMF_OK) {
		return (rv);
	}

	kmf_set_attr_at_index(attrlist, numattr, KMF_KEYSTORE_TYPE_ATTR,
	    &kstype, sizeof (kstype));
	numattr++;

	kmf_set_attr_at_index(attrlist, numattr, KMF_KEYLABEL_ATTR, objlabel,
	    strlen(objlabel));
	numattr++;

	if (tokencred != NULL && tokencred->credlen > 0) {
		kmf_set_attr_at_index(attrlist, numattr, KMF_CREDENTIAL_ATTR,
		    tokencred, sizeof (KMF_CREDENTIAL));
		numattr++;
	}

	kmf_set_attr_at_index(attrlist, numattr, KMF_PRIVATE_BOOL_ATTR,
	    &private, sizeof (private));
	numattr++;

	kmf_set_attr_at_index(attrlist, numattr, KMF_TOKEN_BOOL_ATTR,
	    &token_bool, sizeof (token_bool));
	numattr++;

	kmf_set_attr_at_index(attrlist, numattr, KMF_KEYCLASS_ATTR,
	    &keyclass, sizeof (keyclass));
	numattr++;

	rv = delete_keys(kmfhandle, attrlist, numattr, &nk);

	if (rv == KMF_OK && nk == 0)
		rv = KMF_ERR_KEY_NOT_FOUND;

	return (rv);
}

static KMF_RETURN
delete_certs(KMF_HANDLE_T kmfhandle, KMF_ATTRIBUTE *attlist, const int numattr)
{
	KMF_RETURN rv;
	uint32_t numcerts = 0;
	int num = numattr;

	kmf_set_attr_at_index(attlist, num, KMF_COUNT_ATTR, &numcerts,
	    sizeof (uint32_t));
	num++;

	rv = kmf_find_cert(kmfhandle, num, attlist);
	if (rv == KMF_OK && numcerts > 0) {
		/*
		 * Use numattr because delete cert does not require
		 * KMF_COUNT_ATTR attribute.
		 */
		rv = kmf_delete_cert_from_keystore(kmfhandle, numattr, attlist);
	}

	return (rv);
}

static KMF_RETURN
delete_pk11_certs(KMF_HANDLE_T kmfhandle, KMF_CREDENTIAL *tokencred,
    char *objlabel)
{
	KMF_RETURN kmfrv;
	KMF_KEYSTORE_TYPE kstype = KMF_KEYSTORE_PK11TOKEN;
	int numattr = 0;
	KMF_ATTRIBUTE attrlist[16];

	kmfrv = select_token_pk11(kmfhandle, FALSE);

	if (kmfrv != KMF_OK) {
		return (kmfrv);
	}

	kmf_set_attr_at_index(attrlist, numattr, KMF_KEYSTORE_TYPE_ATTR,
	    &kstype, sizeof (kstype));
	numattr++;

	if (tokencred != NULL && tokencred->credlen > 0) {
		kmf_set_attr_at_index(attrlist, numattr, KMF_CREDENTIAL_ATTR,
		    tokencred, sizeof (KMF_CREDENTIAL));
		numattr++;
	}

	kmf_set_attr_at_index(attrlist, numattr, KMF_CERT_LABEL_ATTR, objlabel,
	    strlen(objlabel));
	numattr++;

	kmfrv = delete_certs(kmfhandle, attrlist, numattr);

	return (kmfrv);
}

/*
 * Delete token objects.
 */
dladm_status_t
dladm_eap_delete(char *label, dladm_secobj_class_t obj_type)
{
	struct stat	statbuf;
	KMF_HANDLE_T	kmfhandle;
	KMF_CREDENTIAL	tokencred = {NULL, 0};

	KMF_RETURN	keyrv = KMF_ERR_KEY_NOT_FOUND;
	KMF_RETURN	certrv = KMF_ERR_KEY_NOT_FOUND;
	dladm_status_t  status = DLADM_STATUS_OK;

	if (obj_type == 0)
		return (DLADM_STATUS_BADARG);

	if (label == NULL)
		return (DLADM_STATUS_BADARG);

	if (stat(PIN_FILE, &statbuf) < 0)
		return (DLADM_STATUS_BADVAL);

	/* GET PIN */
	if (S_ISREG(statbuf.st_mode)) {
		char random_pin[DLADM_SECOBJ_VAL_MAX];
		FILE *fp;
		if ((fp = fopen(PIN_FILE, "r")) == NULL)
			return (DLADM_STATUS_FAILED);
		if ((fscanf(fp, PRINT_PIN_FILE "%255s\n", random_pin)) == EOF)
			return (DLADM_STATUS_FAILED);
		if (fclose(fp))
			return (DLADM_STATUS_FAILED);
		tokencred.credlen = strnlen(random_pin, DLADM_SECOBJ_VAL_MAX);
		tokencred.cred = strndup(random_pin, tokencred.credlen);
		if (tokencred.cred == NULL)
			return (DLADM_STATUS_FAILED);
	} else
		return (DLADM_STATUS_FAILED);

	if (setenv("METASLOT_ENABLED", "false", 1) < 0)
		return (DLADM_STATUS_FAILED);
	if (setenv("SOFTTOKEN_DIR", "/etc/dladm", 1) < 0)
		return (DLADM_STATUS_FAILED);

	if ((kmf_initialize(&kmfhandle, NULL, NULL)) != KMF_OK)
		return (DLADM_STATUS_FAILED);

	if (obj_type == DLADM_SECOBJ_CLASS_TLS)
		keyrv = delete_pk11_keys(kmfhandle, &tokencred, label);

	certrv = delete_pk11_certs(kmfhandle, &tokencred, label);

	/*
	 * Logic here:
	 *    If searching for more than just one class of object (key or cert)
	 *    and only 1 of the classes was not found, it is not an error.
	 *    If searching for just one class of object, that failure should
	 *    be reported.
	 *
	 *    Any error other than "KMF_ERR_[key/cert]_NOT_FOUND" should
	 *    be reported either way.
	 */
	if (keyrv != KMF_ERR_KEY_NOT_FOUND && keyrv != KMF_OK)
		status = DLADM_STATUS_FAILED;
	else if (certrv != KMF_OK && certrv != KMF_ERR_CERT_NOT_FOUND)
		status = DLADM_STATUS_FAILED;

	/*
	 * If nothing was found, return error.
	 */
	if (keyrv == KMF_ERR_KEY_NOT_FOUND && certrv == KMF_ERR_CERT_NOT_FOUND)
		status = DLADM_STATUS_NOTFOUND;

	(void) kmf_finalize(kmfhandle);
	free(tokencred.cred);
	return (status);
}

static KMF_RETURN
import_cert_pk11(KMF_HANDLE_T kmfhandle, KMF_CREDENTIAL *tokencred,
    char *filename, char *label)
{
	KMF_RETURN rv;
	KMF_ATTRIBUTE attrlist[32];
	KMF_KEYSTORE_TYPE kstype = KMF_KEYSTORE_PK11TOKEN;
	int i = 0;

	rv = select_token_pk11(kmfhandle, FALSE);

	if (rv != KMF_OK)
		return (rv);

	kmf_set_attr_at_index(attrlist, i,
	    KMF_KEYSTORE_TYPE_ATTR, &kstype, sizeof (KMF_KEYSTORE_TYPE));
	i++;

	kmf_set_attr_at_index(attrlist, i, KMF_CERT_FILENAME_ATTR,
	    filename, strlen(filename));
	i++;

	kmf_set_attr_at_index(attrlist, i, KMF_CERT_LABEL_ATTR,
	    label, strlen(label));
	i++;

	kmf_set_attr_at_index(attrlist, i, KMF_CREDENTIAL_ATTR, &tokencred,
	    sizeof (KMF_CREDENTIAL));
	i++;

	rv = kmf_import_cert(kmfhandle, i, attrlist);

	return (rv);
}

#define	NEW_ATTRLIST(a, n) \
{ \
	a = (KMF_ATTRIBUTE *)malloc(n * sizeof (KMF_ATTRIBUTE)); \
	if (a == NULL) { \
		rv = KMF_ERR_MEMORY; \
		goto end; \
	} \
	(void) memset(a, 0, n * sizeof (KMF_ATTRIBUTE));  \
}

static KMF_RETURN
import_pk12_pk11(KMF_HANDLE_T kmfhandle, KMF_CREDENTIAL *p12cred,
    KMF_CREDENTIAL *tokencred, char *filename, char *label)
{
	KMF_RETURN rv;
	KMF_X509_DER_CERT *certs;
	KMF_RAW_KEY_DATA *keys;
	int ncerts = 0;
	int nkeys = 0;
	int i;
	KMF_KEYSTORE_TYPE kstype = KMF_KEYSTORE_PK11TOKEN;
	KMF_ATTRIBUTE *attrlist;
	int numattr = 0;

	rv = select_token_pk11(kmfhandle, FALSE);

	if (rv != KMF_OK)
		return (rv);

	rv = kmf_import_objects(kmfhandle, filename, p12cred, &certs,
	    &ncerts, &keys, &nkeys);

	if (rv == KMF_OK) {
		NEW_ATTRLIST(attrlist, (3 + (2 * nkeys)));

		kmf_set_attr_at_index(attrlist, numattr, KMF_KEYSTORE_TYPE_ATTR,
		    &kstype, sizeof (kstype));
		numattr++;

		kmf_set_attr_at_index(attrlist, numattr, KMF_KEYLABEL_ATTR,
		    label, strlen(label));
			numattr++;

		if (tokencred != NULL && tokencred->credlen > 0) {
			kmf_set_attr_at_index(attrlist, numattr,
			    KMF_CREDENTIAL_ATTR, tokencred,
			    sizeof (KMF_CREDENTIAL));
			numattr++;
		}

		/* The order of certificates and keys should match */
		for (i = 0; i < nkeys; i++) {
			int num = numattr;

			if (i < ncerts) {
				kmf_set_attr_at_index(attrlist, num,
				    KMF_CERT_DATA_ATTR, &certs[i].certificate,
				    sizeof (KMF_DATA));
				num++;
			}

			kmf_set_attr_at_index(attrlist, num,
			    KMF_RAW_KEY_ATTR, &keys[i],
			    sizeof (KMF_RAW_KEY_DATA));
			num++;

			rv = kmf_store_key(kmfhandle, num, attrlist);

		}
		free(attrlist);
	}

	if (rv == KMF_OK) {
		numattr = 0;
		NEW_ATTRLIST(attrlist, (1 + (2 * ncerts)));

		kmf_set_attr_at_index(attrlist, numattr,
		    KMF_KEYSTORE_TYPE_ATTR, &kstype, sizeof (kstype));
		numattr++;

		for (i = 0; rv == KMF_OK && i < ncerts; i++) {
			int num = numattr;
			if (certs[i].kmf_private.label != NULL) {
				kmf_set_attr_at_index(attrlist, num,
				    KMF_CERT_LABEL_ATTR,
				    certs[i].kmf_private.label,
				    strlen(certs[i].kmf_private.label));
				num++;
			} else if (i == 0 && label != NULL) {
				kmf_set_attr_at_index(attrlist, num,
				    KMF_CERT_LABEL_ATTR, label, strlen(label));
				num++;
			}

			kmf_set_attr_at_index(attrlist, num,
			    KMF_CERT_DATA_ATTR, &certs[i].certificate,
			    sizeof (KMF_DATA));
			num++;

			rv = kmf_store_cert(kmfhandle, num, attrlist);
		}
		free(attrlist);
	}

end:
	/*
	 * Cleanup memory.
	 */
	if (certs) {
		for (i = 0; i < ncerts; i++)
			kmf_free_kmf_cert(kmfhandle, &certs[i]);
		free(certs);
	}
	if (keys) {
		for (i = 0; i < nkeys; i++)
			kmf_free_raw_key(&keys[i]);
		free(keys);
	}

	return (rv);
}

/*ARGSUSED*/
static KMF_RETURN
import_keys_pk11(KMF_HANDLE_T kmfhandle, KMF_CREDENTIAL *cred,
    char *filename, char *label)
{
	KMF_RETURN rv;
	KMF_ATTRIBUTE attrlist[16];
	int numattr = 0;
	KMF_KEY_HANDLE key;
	KMF_RAW_KEY_DATA rawkey;
	KMF_KEY_CLASS class = KMF_ASYM_PRI;
	int numkeys = 1;
	KMF_KEYSTORE_TYPE kstype = KMF_KEYSTORE_PK11TOKEN;

	rv = select_token_pk11(kmfhandle, FALSE);

	if (rv != KMF_OK)
		return (rv);
	/*
	 * First, set up to read the keyfile using the FILE plugin
	 * mechanisms.
	 */
	kmf_set_attr_at_index(attrlist, numattr, KMF_COUNT_ATTR,
	    &numkeys, sizeof (numkeys));
	numattr++;

	kmf_set_attr_at_index(attrlist, numattr, KMF_KEY_HANDLE_ATTR,
	    &key, sizeof (key));
	numattr++;

	kmf_set_attr_at_index(attrlist, numattr, KMF_RAW_KEY_ATTR,
	    &rawkey, sizeof (rawkey));
	numattr++;

	kmf_set_attr_at_index(attrlist, numattr, KMF_KEYCLASS_ATTR,
	    &class, sizeof (class));
	numattr++;

	kmf_set_attr_at_index(attrlist, numattr, KMF_KEY_FILENAME_ATTR,
	    filename, strlen(filename));
	numattr++;

	rv = kmf_find_key(kmfhandle, numattr, attrlist);
	if (rv == KMF_OK) {
		numattr = 0;

		kmf_set_attr_at_index(attrlist, numattr, KMF_KEYSTORE_TYPE_ATTR,
		    &kstype, sizeof (kstype));
		numattr++;

		if (cred != NULL && cred->credlen > 0) {
			kmf_set_attr_at_index(attrlist, numattr,
			    KMF_CREDENTIAL_ATTR, cred, sizeof (KMF_CREDENTIAL));
			numattr++;
		}

		kmf_set_attr_at_index(attrlist, numattr, KMF_KEYLABEL_ATTR,
		    label, strlen(label));
		numattr++;

		kmf_set_attr_at_index(attrlist, numattr,
		    KMF_RAW_KEY_ATTR, &rawkey, sizeof (rawkey));
		numattr++;

		rv = kmf_store_key(kmfhandle, numattr, attrlist);
		if (rv == KMF_OK)
			(void) printf("Importing %d keys\n", numkeys);

		kmf_free_kmf_key(kmfhandle, &key);
		kmf_free_raw_key(&rawkey);
	} else {
		/* to be removed */
		(void) printf("Failed importing key from %s\n", filename);
	}
	return (rv);
}

/*
 * Import objects from into KMF repositories.
 */
dladm_status_t
dladm_eap_import(dladm_handle_t handle, char *filename, char *label,
    const char *secobjname, const dladm_secobj_class_t obj_type)
{
	struct	stat		statbuf;
	dladm_status_t		status = DLADM_STATUS_OK;
	KMF_ENCODE_FORMAT	kfmt = 0;
	KMF_RETURN		rv;
	KMF_CREDENTIAL		filecred = { NULL, 0 };
	KMF_CREDENTIAL		tokencred = { NULL, 0 };
	KMF_HANDLE_T		kmfhandle;
	char 			random_bytes[DLADM_SECOBJ_VAL_MAX];
	int			len;

	/* Filename arg is required. */
	if (EMPTYSTRING(filename))
		return (DLADM_STATUS_BADARG);

	/* Label arg is required. */
	if (EMPTYSTRING(label))
		return (DLADM_STATUS_BADARG);

	if (obj_type == 0)
		return (DLADM_STATUS_BADARG);

	len = strnlen(filename, FILENAME_MAX);
	if (filename[0] == '"' && filename[len - 1] == '"') {
		filename[len - 1] = '\0';
		filename++;
	}

	switch (kmf_get_file_format(filename, &kfmt)) {
		case KMF_OK:
			break;
		case KMF_ERR_BAD_PARAMETER:
			return (DLADM_STATUS_BADARG);
		case KMF_ERR_OPEN_FILE:
			return (DLADM_STATUS_NOTFOUND);
		case KMF_ERR_ENCODING:
			return (DLADM_STATUS_NOTSUP);
		default:
			return (DLADM_STATUS_FAILED);
	}

	/*
	 * Get Token PIN. Initialize if it is not set
	 */
	if (stat(PIN_FILE, &statbuf) < 0) {
		int i, fd;
		mode_t mode = S_IRUSR | S_IXUSR;
		if (pkcs11_get_random(random_bytes, DLADM_SECOBJ_VAL_MAX) != 0)
			return (DLADM_STATUS_FAILED);
		for (i = 0; i < DLADM_SECOBJ_VAL_MAX; i++) {
			if (isalnum(random_bytes[i])) {
				random_bytes[tokencred.credlen] =
				    random_bytes[i];
				tokencred.credlen++;
			}
		}
		if ((fd = creat(PIN_FILE, mode)) == -1)
			return (DLADM_STATUS_FAILED);
		if (write(fd, PRINT_PIN_FILE, strlen(PRINT_PIN_FILE)) !=
		    strlen(PRINT_PIN_FILE))
			return (DLADM_STATUS_FAILED);
		if (write(fd, random_bytes, tokencred.credlen) !=
		    tokencred.credlen)
			return (DLADM_STATUS_FAILED);
		if (write(fd, "\n", 1) != 1)
			return (DLADM_STATUS_FAILED);
		if (close(fd))
			return (DLADM_STATUS_FAILED);
		tokencred.cred = &random_bytes[0];
	} else if (S_ISREG(statbuf.st_mode)) {
		FILE *fp;
		if ((fp = fopen(PIN_FILE, "r")) == NULL)
			return (DLADM_STATUS_FAILED);
		if ((fscanf(fp, PRINT_PIN_FILE "%255s\n", random_bytes)) == EOF)
			return (DLADM_STATUS_FAILED);
		if (fclose(fp))
			return (DLADM_STATUS_FAILED);
		tokencred.credlen = strnlen(random_bytes, DLADM_SECOBJ_VAL_MAX);
		tokencred.cred = &random_bytes[0];
	} else
		return (DLADM_STATUS_FAILED);

	if (setenv("METASLOT_ENABLED", "false", 1) < 0)
		return (DLADM_STATUS_FAILED);
	if (setenv("SOFTTOKEN_DIR", "/etc/dladm", 1) < 0)
		return (DLADM_STATUS_FAILED);

	/* Retrieve temporary secobj value, containg private key passphrase */
	if (!(EMPTYSTRING(secobjname))) {
		dladm_wlan_key_t keydata;
		(void) strlcpy(keydata.wk_name, secobjname,
		    DLADM_SECOBJ_NAME_MAX);
		status = dladm_get_secobj(handle, &keydata,
		    DLADM_OPT_ACTIVE | DLADM_OPT_PERSIST);
		if (status != DLADM_STATUS_OK) {
			if (status == DLADM_STATUS_NOTFOUND)
				return (DLADM_STATUS_BADARG);
			else
				return (DLADM_STATUS_FAILED);
		}
		filecred.cred = malloc(keydata.wk_len);
		if (filecred.cred == NULL)
			return (DLADM_STATUS_FAILED);
		(void) memcpy(filecred.cred, keydata.wk_val, keydata.wk_len);
		filecred.credlen = keydata.wk_len;
	}

	if ((rv = kmf_initialize(&kmfhandle, NULL, NULL)) != KMF_OK)
		/* "Error initializing KMF" */
		return (DLADM_STATUS_FAILED);

	/*
	 * this means that we have just generated the new PIN and we have to
	 * overwrite the default PIN value
	 */
	if (!(S_ISREG(statbuf.st_mode)) && tokencred.cred != NULL &&
	    tokencred.credlen != 0) {
		rv = setpin_pkcs11(kmfhandle, tokencred.cred,
		    tokencred.credlen);
		if (rv == KMF_ERR_AUTH_FAILED)
			/* Incorrect passphrase */
			status = DLADM_STATUS_BADVAL;
		else if (rv)
			/* Failed to change passphrase */
			status = (DLADM_STATUS_FAILED);
	} else
		status = (DLADM_STATUS_FAILED);

	if (status != DLADM_STATUS_OK) {
		(void) kmf_finalize(kmfhandle);
		if (filecred.cred != NULL)
			free(filecred.cred);
	}

	if (kfmt == KMF_FORMAT_PKCS12 && obj_type == DLADM_SECOBJ_CLASS_TLS) {
		rv = import_pk12_pk11(kmfhandle, &filecred, &tokencred,
		    filename, label);
	} else if (kfmt == KMF_FORMAT_PEM_KEYPAIR &&
	    obj_type == DLADM_SECOBJ_CLASS_TLS) {
		rv = import_keys_pk11(kmfhandle, &tokencred, filename, label);
	} else if (kfmt == KMF_FORMAT_ASN1 || kfmt == KMF_FORMAT_PEM) {
		if (kmf_is_cert_file(kmfhandle, filename, &kfmt) !=
		    KMF_OK && obj_type == DLADM_SECOBJ_CLASS_TLS) {
			rv = import_keys_pk11(kmfhandle, &tokencred, filename,
			    label);
		} else if (kfmt == KMF_FORMAT_ASN1 || kfmt == KMF_FORMAT_PEM) {
			rv = import_cert_pk11(kmfhandle, &tokencred, filename,
			    label);
		} else
			status = DLADM_STATUS_BADVAL;
	} else
		status = DLADM_STATUS_BADARG;

	if (rv != KMF_OK)
		/* "Error importing objects" */
		status = DLADM_STATUS_FAILED;

	(void) kmf_finalize(kmfhandle);

	if (filecred.cred != NULL)
		free(filecred.cred);

	return (status);
}
