/*
 * Copyright (c) 2002-2008, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See wpa_supplicant THIRDPARTYLICENSE for more details.
 */

/*
 * wpa_supplicant routines and defines for parsing wpa Information Element
 */

#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <wpa_defs.h>
#include <inet/wifi_ioctl.h>

/* IEEE 802.11i */
#define PMKID_LEN 16

#define WPA_SELECTOR_LEN 4
#define WPA_VERSION 1
#define RSN_SELECTOR_LEN 4
#define RSN_VERSION 1

/* Information Element IDs */
#define WLAN_EID_SSID 0
#define WLAN_EID_RSN 48
#define WLAN_EID_VENDOR_SPECIFIC 221

#define WPA_GET_LE16(a) ((uint16_t) (((a)[1] << 8) | (a)[0]))
#define WPA_GET_BE16(a) ((uint16_t) (((a)[0] << 8) | (a)[1]))
#define WPA_PUT_BE16(a, val)                    \
        do {                                    \
                (a)[0] = ((uint16_t) (val)) >> 8;    \
                (a)[1] = ((uint16_t) (val)) & 0xff;  \
        } while (0)


#define RSN_SELECTOR(a, b, c, d) \
	((((uint32_t) (a)) << 24) | (((uint32_t) (b)) << 16) | (((uint32_t) (c)) << 8) | \
	 (uint32_t) (d))

#define WPA_AUTH_KEY_MGMT_NONE RSN_SELECTOR(0x00, 0x50, 0xf2, 0)
#define WPA_AUTH_KEY_MGMT_UNSPEC_802_1X RSN_SELECTOR(0x00, 0x50, 0xf2, 1)
#define WPA_AUTH_KEY_MGMT_PSK_OVER_802_1X RSN_SELECTOR(0x00, 0x50, 0xf2, 2)
#define WPA_CIPHER_SUITE_NONE RSN_SELECTOR(0x00, 0x50, 0xf2, 0)
#define WPA_CIPHER_SUITE_WEP40 RSN_SELECTOR(0x00, 0x50, 0xf2, 1)
#define WPA_CIPHER_SUITE_TKIP RSN_SELECTOR(0x00, 0x50, 0xf2, 2)
#define WPA_CIPHER_SUITE_WRAP RSN_SELECTOR(0x00, 0x50, 0xf2, 3)
#define WPA_CIPHER_SUITE_CCMP RSN_SELECTOR(0x00, 0x50, 0xf2, 4)
#define WPA_CIPHER_SUITE_WEP104 RSN_SELECTOR(0x00, 0x50, 0xf2, 5)

#define RSN_AUTH_KEY_MGMT_UNSPEC_802_1X RSN_SELECTOR(0x00, 0x0f, 0xac, 1)
#define RSN_AUTH_KEY_MGMT_PSK_OVER_802_1X RSN_SELECTOR(0x00, 0x0f, 0xac, 2)

#define RSN_AUTH_KEY_MGMT_802_1X_SHA256 RSN_SELECTOR(0x00, 0x0f, 0xac, 5)
#define RSN_AUTH_KEY_MGMT_PSK_SHA256 RSN_SELECTOR(0x00, 0x0f, 0xac, 6)
#define RSN_AUTH_KEY_MGMT_TPK_HANDSHAKE RSN_SELECTOR(0x00, 0x0f, 0xac, 7)

#define RSN_CIPHER_SUITE_NONE RSN_SELECTOR(0x00, 0x0f, 0xac, 0)
#define RSN_CIPHER_SUITE_WEP40 RSN_SELECTOR(0x00, 0x0f, 0xac, 1)
#define RSN_CIPHER_SUITE_TKIP RSN_SELECTOR(0x00, 0x0f, 0xac, 2)
#define RSN_CIPHER_SUITE_WRAP RSN_SELECTOR(0x00, 0x0f, 0xac, 3
#define RSN_CIPHER_SUITE_CCMP RSN_SELECTOR(0x00, 0x0f, 0xac, 4)
#define RSN_CIPHER_SUITE_WEP104 RSN_SELECTOR(0x00, 0x0f, 0xac, 5)
#define RSN_CIPHER_SUITE_AES_128_CMAC RSN_SELECTOR(0x00, 0x0f, 0xac, 6)

#define WPA_OUI_TYPE RSN_SELECTOR(0x00, 0x50, 0xf2, 1)

#define RSN_SELECTOR_PUT(a, val) WPA_PUT_BE32((uint8_t *) (a), (val))
#define RSN_SELECTOR_GET(a) WPA_GET_BE32((const uint8_t *) (a))

#define RSN_NUM_REPLAY_COUNTERS_1 0
#define RSN_NUM_REPLAY_COUNTERS_2 1
#define RSN_NUM_REPLAY_COUNTERS_4 2
#define RSN_NUM_REPLAY_COUNTERS_16 3

/* IEEE 802.11, 7.3.2.25.3 RSN Capabilities */
#define WPA_CAPABILITY_PREAUTH BIT(0)
#define WPA_CAPABILITY_NO_PAIRWISE BIT(1)

#define WPA_GET_BE32(a) ((((uint32_t) (a)[0]) << 24) | (((uint32_t) (a)[1]) << 16) | \
			 (((uint32_t) (a)[2]) << 8) | ((uint32_t) (a)[3]))

struct wpa_ie_data {
	int proto;
	int pairwise_cipher;
	int group_cipher;
	int key_mgmt;
	int capabilities;
	size_t num_pmkid;
	const uint8_t *pmkid;
	int mgmt_group_cipher;
};

#pragma pack(1)
struct wpa_ie_hdr {
	uint8_t elem_id;
	uint8_t len;
	uint8_t oui[4]; /* 24-bit OUI followed by 8-bit OUI type */
	uint8_t version[2]; /* little endian */
};
#pragma pack()

#pragma pack(1)
struct rsn_ie_hdr {
	uint8_t elem_id; /* WLAN_EID_RSN */
	uint8_t len;
	uint8_t version[2]; /* little endian */
};
#pragma pack()

static int wpa_selector_to_bitfield(const uint8_t *s)
{
	if (RSN_SELECTOR_GET(s) == WPA_CIPHER_SUITE_NONE)
		return WPA_CIPHER_NONE;
	if (RSN_SELECTOR_GET(s) == WPA_CIPHER_SUITE_WEP40)
		return WPA_CIPHER_WEP40;
	if (RSN_SELECTOR_GET(s) == WPA_CIPHER_SUITE_TKIP)
		return WPA_CIPHER_TKIP;
	if (RSN_SELECTOR_GET(s) == WPA_CIPHER_SUITE_CCMP)
		return WPA_CIPHER_CCMP;
	if (RSN_SELECTOR_GET(s) == WPA_CIPHER_SUITE_WEP104)
		return WPA_CIPHER_WEP104;
	return 0;
}

static int wpa_key_mgmt_to_bitfield(const uint8_t *s)
{
	if (RSN_SELECTOR_GET(s) == WPA_AUTH_KEY_MGMT_UNSPEC_802_1X)
		return WPA_KEY_MGMT_IEEE8021X;
	if (RSN_SELECTOR_GET(s) == WPA_AUTH_KEY_MGMT_PSK_OVER_802_1X)
		return WPA_KEY_MGMT_PSK;
	if (RSN_SELECTOR_GET(s) == WPA_AUTH_KEY_MGMT_NONE)
		return WPA_KEY_MGMT_WPA_NONE;
	return 0;
}

static int rsn_key_mgmt_to_bitfield(const uint8_t *s)
{
	if (RSN_SELECTOR_GET(s) == RSN_AUTH_KEY_MGMT_UNSPEC_802_1X)
		return WPA_KEY_MGMT_IEEE8021X;
	if (RSN_SELECTOR_GET(s) == RSN_AUTH_KEY_MGMT_PSK_OVER_802_1X)
		return WPA_KEY_MGMT_PSK;
	return 0;
}

static int rsn_selector_to_bitfield(const uint8_t *s)
{
	if (RSN_SELECTOR_GET(s) == RSN_CIPHER_SUITE_NONE)
		return WPA_CIPHER_NONE;
	if (RSN_SELECTOR_GET(s) == RSN_CIPHER_SUITE_WEP40)
		return WPA_CIPHER_WEP40;
	if (RSN_SELECTOR_GET(s) == RSN_CIPHER_SUITE_TKIP)
		return WPA_CIPHER_TKIP;
	if (RSN_SELECTOR_GET(s) == RSN_CIPHER_SUITE_CCMP)
		return WPA_CIPHER_CCMP;
	if (RSN_SELECTOR_GET(s) == RSN_CIPHER_SUITE_WEP104)
		return WPA_CIPHER_WEP104;
	return 0;
}

static int wpa_parse_wpa_ie_wpa(const uint8_t *wpa_ie, size_t wpa_ie_len,
    struct wpa_ie_data *data)
{
	const struct wpa_ie_hdr *hdr;
	const uint8_t *pos;
	int left;
	int i, count;

	memset(data, 0, sizeof (*data));
	data->proto = WPA_PROTO_WPA;
	data->pairwise_cipher = WPA_CIPHER_TKIP;
	data->group_cipher = WPA_CIPHER_TKIP;
	data->key_mgmt = WPA_KEY_MGMT_IEEE8021X;
	data->capabilities = 0;
	data->pmkid = NULL;
	data->num_pmkid = 0;
	data->mgmt_group_cipher = 0;

	if (wpa_ie_len == 0) {
		/* No WPA IE - fail silently */
		return -1;
	}

	if (wpa_ie_len < sizeof (struct wpa_ie_hdr)) {
		printf("wpa_parse_wpa_ie_wpa: ie len too short %lu",
			   (unsigned long) wpa_ie_len);
		return -1;
	}

	hdr = (const struct wpa_ie_hdr *) wpa_ie;

	if (hdr->elem_id != WLAN_EID_VENDOR_SPECIFIC ||
	    hdr->len != wpa_ie_len - 2 ||
	    RSN_SELECTOR_GET(hdr->oui) != WPA_OUI_TYPE ||
	    WPA_GET_LE16(hdr->version) != WPA_VERSION) {
		printf("wpa_parse_wpa_ie_wpa: malformed ie or unknown version");
		return -2;
	}

	pos = (const uint8_t *) (hdr + 1);
	left = wpa_ie_len - sizeof (*hdr);

	if (left >= WPA_SELECTOR_LEN) {
		data->group_cipher = wpa_selector_to_bitfield(pos);
		pos += WPA_SELECTOR_LEN;
		left -= WPA_SELECTOR_LEN;
	} else if (left > 0) {
		printf("wpa_parse_wpa_ie_wpa: ie length mismatch, %u too much",
		    left);
		return -3;
	}

	if (left >= 2) {
		data->pairwise_cipher = 0;
		count = WPA_GET_LE16(pos);
		pos += 2;
		left -= 2;
		if (count == 0 || left < count * WPA_SELECTOR_LEN) {
			printf("wpa_parse_wpa_ie_wpa: ie count botch (pairwise)\
			    , count %u left %u", count, left);
			return -4;
		}
		for (i = 0; i < count; i++) {
			data->pairwise_cipher |= wpa_selector_to_bitfield(pos);
			pos += WPA_SELECTOR_LEN;
			left -= WPA_SELECTOR_LEN;
		}
	} else if (left == 1) {
		printf("wpa_parse_wpa_ie_wpa: ie too short (for key mgmt)");
		return -5;
	}

	if (left >= 2) {
		data->key_mgmt = 0;
		count = WPA_GET_LE16(pos);
		pos += 2;
		left -= 2;
		if (count == 0 || left < count * WPA_SELECTOR_LEN) {
			printf("wpa_parse_wpa_ie_wpa: ie count botch (key mgmt)\
			    , count %u left %u", count, left);
			return -6;
		}
		for (i = 0; i < count; i++) {
			data->key_mgmt |= wpa_key_mgmt_to_bitfield(pos);
			pos += WPA_SELECTOR_LEN;
			left -= WPA_SELECTOR_LEN;
		}
	} else if (left == 1) {
		printf("wpa_parse_wpa_ie_wpa: ie too short (for capabilities)");
		return -7;
	}

	if (left >= 2) {
		data->capabilities = WPA_GET_LE16(pos);
		pos += 2;
		left -= 2;
	}

	if (left > 0) {
		printf("wpa_parse_wpa_ie_wpa: ie has %u trailing bytes \
		    - ignored", left);
	}

	return 0;
}

/**
 * wpa_parse_wpa_ie_rsn - Parse RSN IE
 * @rsn_ie: Buffer containing RSN IE
 * @rsn_ie_len: RSN IE buffer length (including IE number and length octets)
 * @data: Pointer to structure that will be filled in with parsed data
 * Returns: 0 on success, <0 on failure
 */
static int wpa_parse_wpa_ie_rsn(const uint8_t *rsn_ie, size_t rsn_ie_len,
    struct wpa_ie_data *data)
{
	const struct rsn_ie_hdr *hdr;
	const uint8_t *pos;
	int left;
	int i, count;

	memset(data, 0, sizeof (*data));
	data->proto = WPA_PROTO_RSN;
	data->pairwise_cipher = WPA_CIPHER_CCMP;
	data->group_cipher = WPA_CIPHER_CCMP;
	data->key_mgmt = WPA_KEY_MGMT_IEEE8021X;
	data->capabilities = 0;
	data->pmkid = NULL;
	data->num_pmkid = 0;
	data->mgmt_group_cipher = 0;

	if (rsn_ie_len == 0) {
		/* No RSN IE - fail silently */
		return -1;
	}

	if (rsn_ie_len < sizeof (struct rsn_ie_hdr)) {
		printf("wpa_parse_wpa_ie_rsn: ie len too short %lu",
		    (unsigned long) rsn_ie_len);
		return -1;
	}

	hdr = (const struct rsn_ie_hdr *) rsn_ie;

	if (hdr->elem_id != WLAN_EID_RSN ||
	    hdr->len != rsn_ie_len - 2 ||
	    WPA_GET_LE16(hdr->version) != RSN_VERSION) {
		printf("wpa_parse_wpa_ie_rsn: malformed ie or unknown version");
		return -2;
	}

	pos = (const uint8_t *) (hdr + 1);
	left = rsn_ie_len - sizeof (*hdr);

	if (left >= RSN_SELECTOR_LEN) {
		data->group_cipher = rsn_selector_to_bitfield(pos);
		pos += RSN_SELECTOR_LEN;
		left -= RSN_SELECTOR_LEN;
	} else if (left > 0) {
		printf("wpa_parse_wpa_ie_rsn: ie length mismatch, %u too much",
		    left);
		return -3;
	}

	if (left >= 2) {
		data->pairwise_cipher = 0;
		count = WPA_GET_LE16(pos);
		pos += 2;
		left -= 2;
		if (count == 0 || left < count * RSN_SELECTOR_LEN) {
			printf("wpa_parse_wpa_ie_rsn: ie count botch (pairwise)\
			, count %u left %u", count, left);
			return -4;
		}
		for (i = 0; i < count; i++) {
			data->pairwise_cipher |= rsn_selector_to_bitfield(pos);
			pos += RSN_SELECTOR_LEN;
			left -= RSN_SELECTOR_LEN;
		}
	} else if (left == 1) {
		printf("wpa_parse_wpa_ie_rsn: ie too short (for key mgmt)");
		return -5;
	}

	if (left >= 2) {
		data->key_mgmt = 0;
		count = WPA_GET_LE16(pos);
		pos += 2;
		left -= 2;
		if (count == 0 || left < count * RSN_SELECTOR_LEN) {
			printf("wpa_parse_wpa_ie_rsn: ie count botch (key mgmt)\
			, count %u left %u", count, left);
			return -6;
		}
		for (i = 0; i < count; i++) {
			data->key_mgmt |= rsn_key_mgmt_to_bitfield(pos);
			pos += RSN_SELECTOR_LEN;
			left -= RSN_SELECTOR_LEN;
		}
	} else if (left == 1) {
		printf("wpa_parse_wpa_ie_rsn: ie too short (for capabilities)");
		return -7;
	}

	if (left >= 2) {
		data->capabilities = WPA_GET_LE16(pos);
		pos += 2;
		left -= 2;
	}

	if (left >= 2) {
		data->num_pmkid = WPA_GET_LE16(pos);
		pos += 2;
		left -= 2;
		if (left < (int) data->num_pmkid * PMKID_LEN) {
			printf("wpa_parse_wpa_ie_rsn: PMKID underflow "
				   "(num_pmkid=%lu left=%d)",
				   (unsigned long) data->num_pmkid,
				   left);
			data->num_pmkid = 0;
			return -9;
		} else {
			data->pmkid = pos;
			pos += data->num_pmkid * PMKID_LEN;
			left -= data->num_pmkid * PMKID_LEN;
		}
	}

	if (left > 0)
		printf("wpa_parse_wpa_ie_rsn: ie has %u trailing bytes \
		- ignored", left);

	return 0;
}

static int wpa_parse_wpa_ie(const uint8_t *wpa_ie, size_t wpa_ie_len,
    struct wpa_ie_data *data)
{
        if (wpa_ie_len >= 1 && wpa_ie[0] == WLAN_EID_RSN)
                return wpa_parse_wpa_ie_rsn(wpa_ie, wpa_ie_len, data);
        else
                return wpa_parse_wpa_ie_wpa(wpa_ie, wpa_ie_len, data);
}

static char * wpa_supplicant_cipher_txt(char *pos, char *end, int cipher)
{
	int first = 1, ret;
	ret = snprintf(pos, end - pos, "-");
	if (ret < 0 || ret >= end - pos)
		return pos;
	pos += ret;
	if (cipher & WPA_CIPHER_NONE) {
		ret = snprintf(pos, end - pos, "%sNONE", first ? "" : "+");
		if (ret < 0 || ret >= end - pos)
			return pos;
		pos += ret;
		first = 0;
	}
	if (cipher & WPA_CIPHER_WEP40) {
		ret = snprintf(pos, end - pos, "%sWEP40", first ? "" : "+");
		if (ret < 0 || ret >= end - pos)
			return pos;
		pos += ret;
		first = 0;
	}
	if (cipher & WPA_CIPHER_WEP104) {
		ret = snprintf(pos, end - pos, "%sWEP104",
				  first ? "" : "+");
		if (ret < 0 || ret >= end - pos)
			return pos;
		pos += ret;
		first = 0;
	}
	if (cipher & WPA_CIPHER_TKIP) {
		ret = snprintf(pos, end - pos, "%sTKIP", first ? "" : "+");
		if (ret < 0 || ret >= end - pos)
			return pos;
		pos += ret;
		first = 0;
	}
	if (cipher & WPA_CIPHER_CCMP) {
		ret = snprintf(pos, end - pos, "%sCCMP", first ? "" : "+");
		if (ret < 0 || ret >= end - pos)
			return pos;
		pos += ret;
		first = 0;
	}
	return pos;
}

static char * wpa_supplicant_ie_txt(char *pos, char *end, const char *proto,
    const uint8_t *ie, size_t ie_len, int *keymgmt)
{
	struct wpa_ie_data data;
	int first, ret;

	ret = snprintf(pos, end - pos, "[%s-", proto);
	if (ret < 0 || ret >= end - pos)
		return pos;
	pos += ret;

	if (wpa_parse_wpa_ie(ie, ie_len, &data) < 0) {
		ret = snprintf(pos, end - pos, "?]");
		if (ret < 0 || ret >= end - pos)
			return pos;
		pos += ret;
		return pos;
	}

	*keymgmt = data.key_mgmt;

	first = 1;
	if (data.key_mgmt & WPA_KEY_MGMT_IEEE8021X) {
		ret = snprintf(pos, end - pos, "%sEAP", first ? "" : "+");
		if (ret < 0 || ret >= end - pos)
			return pos;
		pos += ret;
		first = 0;
	}
	if (data.key_mgmt & WPA_KEY_MGMT_PSK) {
		ret = snprintf(pos, end - pos, "%sPSK", first ? "" : "+");
		if (ret < 0 || ret >= end - pos)
			return pos;
		pos += ret;
		first = 0;
	}
	if (data.key_mgmt & WPA_KEY_MGMT_WPA_NONE) {
		ret = snprintf(pos, end - pos, "%sNone", first ? "" : "+");
		if (ret < 0 || ret >= end - pos)
			return pos;
		pos += ret;
		first = 0;
	}

	pos = wpa_supplicant_cipher_txt(pos, end, data.pairwise_cipher);

	if (data.capabilities & WPA_CAPABILITY_PREAUTH) {
		ret = snprintf(pos, end - pos, "-preauth");
		if (ret < 0 || ret >= end - pos)
			return pos;
		pos += ret;
	}

	ret = snprintf(pos, end - pos, "]");
	if (ret < 0 || ret >= end - pos)
		return pos;
	pos += ret;

	return pos;
}

static const uint8_t * wpa_bss_get_vendor_ie(const uint8_t *wpaie,
    const uint8_t wpaielen, uint32_t vendor_type)
{
	const uint8_t *end, *pos;

	pos = wpaie;
	end = pos + wpaielen;

	while (pos + 1 < end) {
		if (pos + 2 + pos[1] > end)
			break;
		if (pos[0] == WLAN_EID_VENDOR_SPECIFIC && pos[1] >= 4 &&
		    vendor_type == WPA_GET_BE32(&pos[2]))
			return pos;
		pos += 2 + pos[1];
	}

	return NULL;
}

static const uint8_t * wpa_bss_get_ie(const uint8_t *wpaie,
    const uint8_t wpaielen, uint8_t ie)
{
	const uint8_t *end, *pos;

	pos = wpaie;
	end = pos + wpaielen;

	while (pos + 1 < end) {
		if (pos + 2 + pos[1] > end)
			break;
		if (pos[0] == ie)
			return pos;
		pos += 2 + pos[1];
	}

	return NULL;
}

#define WPA_IE_VENDOR_TYPE 0x0050f201

int wpa_supplicant_capsie2txt(const uint16_t nodecaps, const uint8_t *wpaie,
    const uint8_t wpaielen, char *buf, size_t buflen, int *keymgmt)
{
	char *pos, *end;
	int ret;
	const uint8_t *ie, *ie2;

	pos = buf;
	end = buf + buflen;

	ie = wpa_bss_get_vendor_ie(wpaie, wpaielen, WPA_IE_VENDOR_TYPE);
	if (ie)
		pos = wpa_supplicant_ie_txt(pos, end, "WPA", ie, 2 + ie[1],
		    keymgmt);
	ie2 = wpa_bss_get_ie(wpaie, wpaielen, WLAN_EID_RSN);
	if (ie2)
		pos = wpa_supplicant_ie_txt(pos, end, "WPA2", ie2, 2 + ie2[1],
		    keymgmt);
	if (!ie && !ie2 && nodecaps & IEEE80211_CAP_PRIVACY) {
		ret = snprintf(pos, end - pos, "[WEP]");
		if (ret < 0 || ret >= end - pos)
			return -1;
		pos += ret;
	}
	if (nodecaps & IEEE80211_CAP_IBSS) {
		ret = snprintf(pos, end - pos, "[IBSS]");
		if (ret < 0 || ret >= end - pos)
			return -1;
		pos += ret;
	}
	if (nodecaps & IEEE80211_CAP_ESS) {
		ret = snprintf(pos, end - pos, "[ESS]");
		if (ret < 0 || ret >= end - pos)
			return -1;
		pos += ret;
	}

	return 0;
}
