/*
 * Copyright 2010 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 * Copyright (c) 2012, Enrico Papi <enricop@computer.org>. All rights reserved.
 */

/*
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#include <sys/param.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/strsun.h>
#include <sys/policy.h>
#include <inet/common.h>
#include <inet/nd.h>
#include <inet/mi.h>
#include <sys/door.h>
#include <sys/note.h>
#include <sys/mac_provider.h>
#include "net80211_impl.h"
#include <inet/wifi_ioctl.h>

static size_t
wifi_strnlen(const char *s, size_t n)
{
	size_t i;

	for (i = 0; i < n && s[i] != '\0'; i++)
		/* noop */;
	return (i);
}

/*
 * Initialize an output message block by copying from an
 * input message block. The message is of type wldp_t.
 *    mp     input message block
 *    buflen length of wldp_buf
 */
static void
wifi_setupoutmsg(mblk_t *mp, int buflen)
{
	wldp_t *wp;

	wp = (wldp_t *)mp->b_rptr;
	wp->wldp_length = WIFI_BUF_OFFSET + buflen;
	wp->wldp_result = WL_SUCCESS;
	mp->b_wptr = mp->b_rptr + wp->wldp_length;
}

#define	WIFI_HAVE_CAP(in, flag)	(((in)->in_capinfo & (flag)) ? 1 : 0)
#define	WIFI_HAVE_HTCAP(in)	(((in)->in_htcap != 0) ? 1 : 0)

/*
 * Callback function used by ieee80211_iterate_nodes() in
 * wifi_cfg_esslist() to get info of each node in a node table
 *    arg  output buffer, pointer to wl_ess_list_t
 *    in   each node in the node table
 */
static void
wifi_read_ap(void *arg, struct ieee80211_node *in)
{
	wl_ess_list_t *aps = arg;
	ieee80211com_t *ic = in->in_ic;
	struct ieee80211_channel *chan = in->in_chan;
	struct ieee80211_rateset *rates = &(in->in_rates);
	wl_ess_conf_t *conf;
	uint8_t *end;
	uint_t i;

	end = (uint8_t *)aps + aps->wl_ess_list_size - sizeof (wl_ess_list_t);
	conf = &aps->wl_ess_list_ess[aps->wl_ess_list_num];
	if ((uint8_t *)conf > end) {
		aps->wl_ess_list_size = USHRT_MAX;
		return;
	}

	/* skip newly allocated NULL bss node */
	if (IEEE80211_ADDR_EQ(in->in_macaddr, ic->ic_macaddr))
		return;

	if (in->in_esslen != 0) {
		conf->wl_ess_conf_essid.wl_essid_length = in->in_esslen;
		bcopy(in->in_essid, conf->wl_ess_conf_essid.wl_essid_essid,
		    in->in_esslen);
	} else {
		conf->wl_ess_conf_essid.wl_essid_length = 0;
	}

	bcopy(in->in_bssid, conf->wl_ess_conf_bssid.wl_bssid_bssid,
	    IEEE80211_ADDR_LEN);

	conf->wl_ess_conf_beacon_period = in->in_intval;
	conf->wl_ess_conf_beacon_tsf = in->in_tstamp.tsf;
	conf->wl_ess_conf_beacon_age = in->in_rstamp;

	conf->wl_ess_conf_sl = ic->ic_node_getrssi(in);

	conf->wl_ess_conf_caps = in->in_capinfo;
	/* authmode is set to IEEE80211_AUTH_OPEN = 1 by default */

	if (in->in_wpa_ie == NULL) {
		conf->wl_ess_conf_wpa_ie.wpa_ie_len = 0;
	} else {
		conf->wl_ess_conf_wpa_ie.wpa_ie_len = in->in_wpa_ie[1] + 2;
		bcopy(in->in_wpa_ie, conf->wl_ess_conf_wpa_ie.wpa_ie,
		    conf->wl_ess_conf_wpa_ie.wpa_ie_len);
	}

	/* physical (FH, DS, ERP) parameters */
	if (IEEE80211_IS_CHAN_A(chan) || IEEE80211_IS_CHAN_T(chan)) {
		wl_ofdm_t *ofdm = &conf->wl_ess_conf_phys.wl_phy_ofdm_conf;
		ofdm->wl_ofdm_subtype = WL_OFDM;
		ofdm->wl_ofdm_frequency = chan->ich_freq;
		ofdm->wl_ofdm_ht_enabled = WIFI_HAVE_HTCAP(in);
	} else {
		switch (in->in_phytype) {
		case IEEE80211_T_FH: {
			wl_fhss_t *fhss =
			    &conf->wl_ess_conf_phys.wl_phy_fhss_conf;

			fhss->wl_fhss_subtype = WL_FHSS;
			fhss->wl_fhss_frequency	= chan->ich_freq;
			fhss->wl_fhss_dwelltime = in->in_fhdwell;
			break;
		}
		case IEEE80211_T_DS: {
			wl_dsss_t *dsss =
			    &conf->wl_ess_conf_phys.wl_phy_dsss_conf;

			dsss->wl_dsss_subtype = WL_DSSS;
			dsss->wl_dsss_frequency = chan->ich_freq;
			dsss->wl_dsss_have_short_preamble = WIFI_HAVE_CAP(in,
			    IEEE80211_CAPINFO_SHORT_PREAMBLE);
			dsss->wl_dsss_agility_enabled = WIFI_HAVE_CAP(in,
			    IEEE80211_CAPINFO_CHNL_AGILITY);
			dsss->wl_dsss_have_pbcc = dsss->wl_dsss_pbcc_enable =
			    WIFI_HAVE_CAP(in, IEEE80211_CAPINFO_PBCC);
			break;
		}
		case IEEE80211_T_OFDM: {
			wl_erp_t *erp = &conf->wl_ess_conf_phys.wl_phy_erp_conf;

			erp->wl_erp_subtype = WL_ERP;
			erp->wl_erp_frequency = chan->ich_freq;
			erp->wl_erp_have_short_preamble = WIFI_HAVE_CAP(in,
			    IEEE80211_CAPINFO_SHORT_PREAMBLE);
			erp->wl_erp_have_agility = erp->wl_erp_agility_enabled =
			    WIFI_HAVE_CAP(in, IEEE80211_CAPINFO_CHNL_AGILITY);
			erp->wl_erp_have_pbcc = erp->wl_erp_pbcc_enabled =
			    WIFI_HAVE_CAP(in, IEEE80211_CAPINFO_PBCC);
			erp->wl_erp_dsss_ofdm_enabled =
			    WIFI_HAVE_CAP(in, IEEE80211_CAPINFO_DSSSOFDM);
			erp->wl_erp_sst_enabled = WIFI_HAVE_CAP(in,
			    IEEE80211_CAPINFO_SHORT_SLOTTIME);
			erp->wl_erp_ht_enabled = WIFI_HAVE_HTCAP(in);
			break;
		} /* case IEEE80211_T_OFDM */
		} /* switch in->in_phytype */
	}

	for (i = 0; i < rates->ir_nrates; i++)
		conf->wl_ess_conf_rates.wl_rates_rates[i] =
		    rates->ir_rates[rates->ir_nrates - i - 1];
	conf->wl_ess_conf_rates.wl_rates_num = rates->ir_nrates;

	aps->wl_ess_list_num++;
}

static int
wifi_cmd_scan(struct ieee80211com *ic, mblk_t *mp)
{
	ieee80211_dbg(IEEE80211_MSG_CONFIG, "wifi_cmd_scan");

	IEEE80211_UNLOCK(ic);

	ieee80211_new_state(ic, IEEE80211_S_SCAN, 0);

	IEEE80211_LOCK(ic);

	wifi_setupoutmsg(mp, 0);
	return (0);
}

static void
wifi_loaddefdata(struct ieee80211com *ic)
{
	struct ieee80211_node *in = ic->ic_bss;
	int i;

	ic->ic_des_esslen = 0;
	bzero(ic->ic_des_essid, IEEE80211_NWID_LEN);
	ic->ic_flags &= ~IEEE80211_F_DESBSSID;
	bzero(ic->ic_des_bssid, IEEE80211_ADDR_LEN);

	bzero(ic->ic_bss->in_bssid, IEEE80211_ADDR_LEN);

	ic->ic_des_chan = IEEE80211_CHAN_ANYC;
	ic->ic_fixed_rate = IEEE80211_FIXED_RATE_NONE;
	in->in_authmode = IEEE80211_AUTH_OPEN;

	ic->ic_flags &= ~IEEE80211_F_PRIVACY;
	ic->ic_flags &= ~IEEE80211_F_WPA;	/* mask WPA mode */

	for (i = 0; i < IEEE80211_KEY_MAX; i++) {
		ic->ic_nw_keys[i].wk_keylen = 0;
		bzero(ic->ic_nw_keys[i].wk_key, IEEE80211_KEYBUF_SIZE);
	}
	ic->ic_curmode = IEEE80211_MODE_AUTO;
	ic->ic_flags &= ~IEEE80211_F_IBSSON;
	ic->ic_opmode = IEEE80211_M_STA;
}

static int
wifi_cmd_disassoc(struct ieee80211com *ic, mblk_t *mp)
{
	ieee80211_dbg(IEEE80211_MSG_CONFIG, "wifi_cmd_disassoc");

	if (ic->ic_state != IEEE80211_S_INIT) {
		IEEE80211_UNLOCK(ic);
		(void) ieee80211_new_state(ic, IEEE80211_S_INIT, -1);
		IEEE80211_LOCK(ic);
	}
	wifi_loaddefdata(ic);
	wifi_setupoutmsg(mp, 0);
	return (0);
}

static int
wifi_cfg_getset(struct ieee80211com *ic, mblk_t **mp)
{
	mblk_t *mp1 = *mp;
	wldp_t *wp = (wldp_t *)mp1->b_rptr;
	int err = 0;

	ASSERT(ic != NULL && mp1 != NULL);
	IEEE80211_LOCK_ASSERT(ic);
	if (MBLKL(mp1) < WIFI_BUF_OFFSET) {
		ieee80211_err("wifi_cfg_getset: "
		    "invalid input buffer, size=%d\n", MBLKL(mp1));
		return (EINVAL);
	}

	switch (wp->wldp_id) {
	/* Commands */
	case WL_SCAN:
		err = wifi_cmd_scan(ic, mp1);
		break;
	case WL_DISASSOCIATE:
		err = wifi_cmd_disassoc(ic, mp1);
		break;
	default:
		wifi_setupoutmsg(mp1, 0);
		wp->wldp_result = WL_LACK_FEATURE;
		err = ENOTSUP;
		break;
	}

	return (err);
}

/*
 * Typically invoked by drivers in response to requests for
 * information or to change settings from the userland.
 *
 * Return value should be checked by WiFi drivers. Return 0
 * on success. Otherwise, return non-zero value to indicate
 * the error. Driver should operate as below when the return
 * error is:
 * ENETRESET	Reset wireless network and re-start to join a
 *		WLAN. ENETRESET is returned when a configuration
 *		parameter has been changed.
 *		When acknowledge a M_IOCTL message, thie error
 *		is ignored.
 */
int
ieee80211_ioctl(struct ieee80211com *ic, queue_t *wq, mblk_t *mp)
{
	struct iocblk *iocp;
	int32_t cmd, err, len;
	boolean_t need_privilege;
	mblk_t *mp1;

	if (MBLKL(mp) < sizeof (struct iocblk)) {
		ieee80211_err("ieee80211_ioctl: ioctl buffer too short, %u\n",
		    MBLKL(mp));
		miocnak(wq, mp, 0, EINVAL);
		return (EINVAL);
	}

	/*
	 * Validate the command
	 */
	iocp = (struct iocblk *)mp->b_rptr;
	iocp->ioc_error = 0;
	cmd = iocp->ioc_cmd;
	need_privilege = B_TRUE;
	switch (cmd) {
	case WLAN_COMMAND:
		break;
	default:
		ieee80211_dbg(IEEE80211_MSG_ANY, "ieee80211_ioctl(): "
		    "unknown cmd 0x%x (%d) \n", cmd, cmd);
		miocnak(wq, mp, 0, EINVAL);
		return (EINVAL);
	}

	if (need_privilege && (err = secpolicy_dl_config(iocp->ioc_cr)) != 0) {
		miocnak(wq, mp, 0, err);
		return (err);
	}

	IEEE80211_LOCK(ic);

	/* sanity check */
	mp1 = mp->b_cont;
	if (iocp->ioc_count == 0 || iocp->ioc_count < sizeof (wldp_t) ||
	    mp1 == NULL) {
		miocnak(wq, mp, 0, EINVAL);
		IEEE80211_UNLOCK(ic);
		return (EINVAL);
	}

	/* assuming single data block */
	if (mp1->b_cont != NULL) {
		freemsg(mp1->b_cont);
		mp1->b_cont = NULL;
	}

	err = wifi_cfg_getset(ic, &mp1);
	mp->b_cont = mp1;
	IEEE80211_UNLOCK(ic);

	len = msgdsize(mp1);
	/* ignore ENETRESET when acknowledge the M_IOCTL message */
	if (err == 0 || err == ENETRESET)
		miocack(wq, mp, len, 0);
	else
		miocack(wq, mp, len, err);

	return (err);
}

/*
 * The following routines are for brussels support
 */

/*
 * MAC_PROP_WL_ESSID
 */
static int
wl_set_essid(struct ieee80211com *ic, const void *wldp_buf)
{
	wl_essid_t *iw_essid = (wl_essid_t *)wldp_buf;

	if (iw_essid->wl_essid_length > IEEE80211_NWID_LEN ||
	    iw_essid->wl_essid_length == 0) {
		ieee80211_err("wl_set_essid: "
		    "essid too long, %u, max %u\n",
		    iw_essid->wl_essid_length, IEEE80211_NWID_LEN);

		return (EINVAL);
	}

	ieee80211_dbg(IEEE80211_MSG_CONFIG, "wl_set_essid: "
	    "set essid=%s length=%d\n",
	    iw_essid->wl_essid_essid, iw_essid->wl_essid_length);

	ic->ic_des_esslen = iw_essid->wl_essid_length;
	bzero(ic->ic_des_essid, sizeof (ic->ic_des_essid));
	bcopy(iw_essid->wl_essid_essid, ic->ic_des_essid,
	    ic->ic_des_esslen);

	return (ENETRESET);
}

static void
wl_get_essid(struct ieee80211com *ic, void *wldp_buf)
{
	uint8_t *essid;
	wl_essid_t *ow_essid = wldp_buf;

	essid = ic->ic_des_essid;
	if (essid[0] == 0)
		essid = ic->ic_bss->in_essid;

	bzero(ow_essid, sizeof (wl_essid_t));
	ow_essid->wl_essid_length = wifi_strnlen((const char *)essid,
	    IEEE80211_NWID_LEN);
	bcopy(essid, &ow_essid->wl_essid_essid, ow_essid->wl_essid_length);
}

/*
 * MAC_PROP_WL_BSSID
 */
static int
wl_set_bssid(struct ieee80211com *ic, const void* wldp_buf)
{
	const wl_bssid_t *sta_bssid = wldp_buf;

	ieee80211_dbg(IEEE80211_MSG_CONFIG, "wl_set_bssid: "
	    "set bssid=%s\n",
	    ieee80211_macaddr_sprintf(wldp_buf));

	bcopy(sta_bssid->wl_bssid_bssid, ic->ic_des_bssid, IEEE80211_ADDR_LEN);
	ic->ic_flags |= IEEE80211_F_DESBSSID;

	return (0);
}

static void
wl_get_bssid(struct ieee80211com *ic, void *wldp_buf)
{
	uint8_t *bssid;
	wl_bssid_t *ow_bssid = wldp_buf;

	if (ic->ic_flags & IEEE80211_F_DESBSSID)
		bssid = ic->ic_des_bssid;
	else
		bssid = ic->ic_bss->in_bssid;

	bzero(ow_bssid, sizeof (wl_bssid_t));
	bcopy(bssid, ow_bssid->wl_bssid_bssid, IEEE80211_ADDR_LEN);
}

/*
 * MAC_PROP_WL_BSSTYP
 */
static int
wl_set_bsstype(struct ieee80211com *ic, const void *wldp_buf)
{
	wl_bss_type_t *iw_opmode = (wl_bss_type_t *)wldp_buf;

	ieee80211_dbg(IEEE80211_MSG_CONFIG, "wl_set_bsstype: "
	    "set bsstype=%u\n", *iw_opmode);

	switch (*iw_opmode) {
	case IEEE80211_MODE_INFRA:
		ic->ic_flags &= ~IEEE80211_F_IBSSON;
		ic->ic_opmode = IEEE80211_M_STA;
		break;
	case IEEE80211_MODE_IBSS:
		if ((ic->ic_caps & IEEE80211_C_IBSS) == 0)
			return (ENOTSUP);

		ic->ic_opmode = IEEE80211_M_IBSS;
		break;
	default:
		ieee80211_err("wl_set_bsstype: "
		    "unknown opmode\n");
		return (EINVAL);
	}
	return (0);
}

static void
wl_get_bsstype(struct ieee80211com *ic, void *wldp_buf)
{
	wl_bss_type_t ow_opmode;

	switch (ic->ic_opmode) {
	case IEEE80211_M_STA:
		ow_opmode = IEEE80211_MODE_INFRA;
		break;
	case IEEE80211_M_IBSS:
		ow_opmode = IEEE80211_MODE_IBSS;
		break;
	default:
		ow_opmode = IEEE80211_MODE_INFRA;
		break;
	}

	bcopy(&ow_opmode, wldp_buf, sizeof (wl_bss_type_t));
}

/*
 * MAC_PROP_WL_LINKSTATUS
 */
static void
wl_get_linkstatus(struct ieee80211com *ic, void *wldp_buf)
{
	wl_linkstatus_t ow_linkstat = (ic->ic_state == IEEE80211_S_RUN);

	if ((ic->ic_flags & IEEE80211_F_WPA) &&
	    (ieee80211_crypto_getciphertype(ic) != WIFI_SEC_WPA))
		ow_linkstat = B_FALSE;

	bcopy(&ow_linkstat, wldp_buf, sizeof (wl_linkstatus_t));
}

/*
 * MAC_PROP_WL_DESIRED_RATES
 */
static int
wl_set_desrates(struct ieee80211com *ic, const void *wldp_buf)
{
	int i, j;
	uint8_t drate;
	boolean_t isfound;
	wl_rates_t *iw_rates = (wl_rates_t *)wldp_buf;
	struct ieee80211_node *in = ic->ic_bss;
	struct ieee80211_rateset *rs = &in->in_rates;

	drate = iw_rates->wl_rates_rates[0];
	if (ic->ic_fixed_rate == drate)
		return (0);

	ieee80211_dbg(IEEE80211_MSG_CONFIG, "wl_set_desrates: "
	    "set desired rate=%u\n", drate);

	if (drate == 0) {
		ic->ic_fixed_rate = IEEE80211_FIXED_RATE_NONE;
		if (ic->ic_state == IEEE80211_S_RUN) {
			IEEE80211_UNLOCK(ic);
			ieee80211_new_state(ic, IEEE80211_S_ASSOC, 0);
			IEEE80211_LOCK(ic);
		}
		return (0);
	}

	/*
	 * Set desired rate. The desired rate is for data transfer
	 * and usally is checked and used when driver changes to
	 * RUN state.
	 * If the driver is in AUTH | ASSOC | RUN state, desired
	 * rate is checked anainst rates supported by current ESS.
	 * If it's supported and current state is AUTH|ASSOC, nothing
	 * needs to be done by driver since the desired rate will
	 * be enabled when the device changes to RUN state. And
	 * when current state is RUN, Re-associate with the ESS to
	 * enable the desired rate.
	 */

	if (ic->ic_state != IEEE80211_S_INIT &&
	    ic->ic_state != IEEE80211_S_SCAN) {
		for (i = 0; i < rs->ir_nrates; i++) {
			if (drate == IEEE80211_RV(rs->ir_rates[i]))
				break;
		}
		/* supported */
		if (i < rs->ir_nrates) {
			ic->ic_fixed_rate = drate;
			if (ic->ic_state == IEEE80211_S_RUN) {
				IEEE80211_UNLOCK(ic);
				ieee80211_new_state(ic,
				    IEEE80211_S_ASSOC, 0);
				IEEE80211_LOCK(ic);
			}
			return (0);
		}
	}

	/*
	 * In INIT or SCAN state
	 * check if the desired rate is supported by device
	 */
	isfound = B_FALSE;
	for (i = 0; i < IEEE80211_MODE_MAX; i++) {
		rs = &ic->ic_sup_rates[i];
		for (j = 0; j < rs->ir_nrates; j++) {
			if (drate ==  IEEE80211_RV(rs->ir_rates[j])) {
				isfound = B_TRUE;
				break;
			}
		}
		if (isfound)
			break;
	}
	if (!isfound) {
		ieee80211_err("wl_set_desrates: "
		    "invald rate %d\n", drate);
		return (EINVAL);
	}
	ic->ic_fixed_rate = drate;
	if (ic->ic_state != IEEE80211_S_SCAN)
		return (ENETRESET);

	return (0);
}

static void
wl_get_desrates(struct ieee80211com *ic, void *wldp_buf)
{
	uint8_t srate;
	wl_rates_t ow_rates;
	struct ieee80211_node *in = ic->ic_bss;
	struct ieee80211_rateset *rs = &in->in_rates;

	srate = rs->ir_rates[in->in_txrate] & IEEE80211_RATE_VAL;
	ow_rates.wl_rates_num = 1;
	ow_rates.wl_rates_rates[0] =
	    (ic->ic_fixed_rate == IEEE80211_FIXED_RATE_NONE) ?
	    srate : ic->ic_fixed_rate;
	bcopy(&ow_rates, wldp_buf, sizeof (wl_rates_t));

}

/*
 * MAC_PROP_AUTH_MODE
 */
static int
wl_set_authmode(struct ieee80211com *ic, const void *wldp_buf)
{
	wl_authmode_t *iw_auth = (wl_authmode_t *)wldp_buf;

	ieee80211_dbg(IEEE80211_MSG_CONFIG, "wl_set_authmode: "
	    "set authmode=%u\n", *iw_auth);

	if (*iw_auth == ic->ic_bss->in_authmode)
		return (0);

	switch (*iw_auth) {
	case IEEE80211_AUTH_OPEN:
	case IEEE80211_AUTH_SHARED:
		ic->ic_bss->in_authmode = *iw_auth;
		break;
	default:
		ic->ic_bss->in_authmode = IEEE80211_AUTH_AUTO;
		ieee80211_err("wl_set_authmode: "
		    "unknown authmode %u\n, setting to AUTO", *iw_auth);
		break;
	}

	return (0);
}

static void
wl_get_authmode(struct ieee80211com *ic, void *wldp_buf)
{
	wl_authmode_t ow_auth;

	ow_auth = ic->ic_bss->in_authmode;
	bcopy(&ow_auth, wldp_buf, sizeof (wl_authmode_t));

}

/*
 * MAC_PROP_WL_ENCRYPTION
 */
static int
wl_set_encrypt(struct ieee80211com *ic, const void *wldp_buf)
{
	uint32_t flags;
	wl_encryption_t *iw_encryp = (wl_encryption_t *)wldp_buf;

	ieee80211_dbg(IEEE80211_MSG_CONFIG, "wl_set_encrypt: "
	    "set encryption=%u\n", *iw_encryp);

	flags = ic->ic_flags;
	if (*iw_encryp == WL_NOENCRYPTION)
		flags &= ~IEEE80211_F_PRIVACY;
	else
		flags |= IEEE80211_F_PRIVACY;

	if (ic->ic_flags != flags)
		ic->ic_flags = flags;

	return (0);
}

static void
wl_get_encrypt(struct ieee80211com *ic, void *wldp_buf)
{
	wl_encryption_t *ow_encryp;

	ow_encryp = (wl_encryption_t *)wldp_buf;
	*ow_encryp = (ic->ic_flags & IEEE80211_F_PRIVACY) ? 1 : 0;
	if (ic->ic_flags & IEEE80211_F_WPA)
		*ow_encryp = WL_ENC_WPA;

}

/*
 * MAC_PROP_WL_RSSI
 */
static void
wl_get_rssi(struct ieee80211com *ic, void *wldp_buf)
{
	wl_rssi_t *ow_rssi;
	struct ieee80211_node *in = ic->ic_bss;

	ow_rssi = (wl_rssi_t *)wldp_buf;
	*ow_rssi = ic->ic_node_getrssi(in);
}

/*
 * MAC_PROP_WL_PHY_CONFIG
 * Only used when setting IBSS mode channel
 */

static int
wl_set_phy(struct ieee80211com *ic, const void* wldp_buf)
{
	int16_t ch;
	wl_dsss_t *dsss;
	wl_phy_conf_t *iw_phy = (wl_phy_conf_t *)wldp_buf;

	dsss = (wl_dsss_t *)iw_phy;
	/* this is actually the channel number */
	ch = dsss->wl_dsss_frequency;

	ieee80211_dbg(IEEE80211_MSG_CONFIG, "wl_set_phy: "
	    "set channel=%d\n", ch);

	if (ch == 0 || ch == (int16_t)IEEE80211_CHAN_ANY) {
		ic->ic_des_chan = IEEE80211_CHAN_ANYC;
	} else if ((uint_t)ch > IEEE80211_CHAN_MAX ||
	    ieee80211_isclr(ic->ic_chan_active, ch)) {
		return (EINVAL);
	} else {
		ic->ic_des_chan = ic->ic_ibss_chan =
		    &ic->ic_sup_channels[ch];
	}

	switch (ic->ic_state) {
	case IEEE80211_S_INIT:
	case IEEE80211_S_SCAN:
		return (ENETRESET);
	default:
		/*
		 * If hte desired channel has changed (to something
		 * other than any) and we're not already scanning,
		 * then kick the state machine.
		 */
		if (ic->ic_des_chan != IEEE80211_CHAN_ANYC &&
		    ic->ic_bss->in_chan != ic->ic_des_chan &&
		    (ic->ic_flags & IEEE80211_F_SCAN) == 0)
			return (ENETRESET);
		break;
	}

	return (0);
}

#define	WIFI_HT_MODE(in)	(((in)->in_flags & IEEE80211_NODE_HT) ? 1 : 0)

static int
wl_get_phy(struct ieee80211com *ic, void *wldp_buf)
{
	int err = 0;
	wl_phy_conf_t *ow_phy;
	struct ieee80211_channel *ch = ic->ic_curchan;
	struct ieee80211_node *in = ic->ic_bss;

	ow_phy = (wl_phy_conf_t *)wldp_buf;
	bzero(wldp_buf, sizeof (wl_phy_conf_t));

	/* get current phy parameters: FH|DS|ERP */
	if (IEEE80211_IS_CHAN_A(ch) || IEEE80211_IS_CHAN_T(ch)) {
		wl_ofdm_t *ofdm = (wl_ofdm_t *)ow_phy;
		ofdm->wl_ofdm_subtype = WL_OFDM;
		ofdm->wl_ofdm_frequency = ch->ich_freq;
		ofdm->wl_ofdm_ht_enabled = WIFI_HT_MODE(in);
	} else {
		switch (ic->ic_phytype) {
		case IEEE80211_T_FH: {
			wl_fhss_t *fhss = (wl_fhss_t *)ow_phy;
			fhss->wl_fhss_subtype = WL_FHSS;
			fhss->wl_fhss_frequency = ch->ich_freq;
			break;
		}
		case IEEE80211_T_DS: {
			wl_dsss_t *dsss = (wl_dsss_t *)ow_phy;
			dsss->wl_dsss_subtype = WL_DSSS;
			dsss->wl_dsss_frequency = ch->ich_freq;
			break;
		}
		case IEEE80211_T_OFDM: {
			wl_erp_t *erp = (wl_erp_t *)ow_phy;
			erp->wl_erp_subtype = WL_ERP;
			erp->wl_erp_frequency = ch->ich_freq;
			erp->wl_erp_ht_enabled = WIFI_HT_MODE(in);
			break;
		}
		default:
			ieee80211_err("wl_get_phy: "
			    "unknown phy type, %x\n", ic->ic_phytype);
			err = EIO;
			break;
		}
	}

	return (err);
}

/*
 * MAC_PROP_WL_CAPABILITY
 */
static void
wl_get_capability(struct ieee80211com *ic, void *wldp_buf)
{
	wl_capability_t ow_caps;

	ow_caps = ic->ic_caps;
	bcopy(&ow_caps, wldp_buf, sizeof (wl_capability_t));

}

/*
 * MAC_PROP_WL_WPA
 */
static int
wl_set_wpa(struct ieee80211com *ic, const void *wldp_buf)
{
	wl_wpa_t *wpa = (wl_wpa_t *)wldp_buf;

	ieee80211_dbg(IEEE80211_MSG_BRUSSELS, "wl_set_wpa: "
	    "set wpa=%u\n", *wpa);

	if (*wpa == B_TRUE) {
		/* enable wpa mode */
		ic->ic_flags |= IEEE80211_F_PRIVACY;
		ic->ic_flags |= IEEE80211_F_WPA;
	} else if (*wpa == B_FALSE) {
		ic->ic_flags &= ~IEEE80211_F_PRIVACY;
		ic->ic_flags &= ~IEEE80211_F_WPA;
	} else {
		return (EINVAL);
	}

	return (0);
}

static void
wl_get_wpa(struct ieee80211com *ic, void *wldp_buf)
{
	wl_wpa_t *wpa;

	wpa = (wl_wpa_t *)wldp_buf;
	*wpa = ((ic->ic_flags & IEEE80211_F_WPA) ? B_TRUE : B_FALSE);

	ieee80211_dbg(IEEE80211_MSG_BRUSSELS, "wl_get_wpa: "
	    "get wpa=%u\n", *wpa);

}

/*
 * MAC_PROP_WL_COUNTERM
 */
static int
wl_set_counterm(struct ieee80211com *ic, const void *wldp_buf)
{
	boolean_t *counterm = (boolean_t *)wldp_buf;

	ieee80211_dbg(IEEE80211_MSG_BRUSSELS, "wl_set_counterm: "
	    "set tkip countermeasures = %u\n", *counterm);

	if (*counterm == B_FALSE) {
		ic->ic_flags &= ~IEEE80211_F_COUNTERM;
	} else {
		if ((ic->ic_flags & IEEE80211_F_WPA) == 0)
			return (EOPNOTSUPP);
		ic->ic_flags |= IEEE80211_F_COUNTERM;
	}

	return (0);
}

/*
 * MAC_PROP_WL_ESS_LIST
 */
static int
wl_get_esslist(struct ieee80211com *ic, void *wldp_buf, uint_t wldp_length)
{
	wl_ess_list_t *ess_list;

	ess_list = (wl_ess_list_t *)wldp_buf;

	ess_list->wl_ess_list_num = 0;
	ess_list->wl_ess_list_size = (uint16_t)wldp_length;

	ieee80211_iterate_nodes(&ic->ic_scan, wifi_read_ap, ess_list);

	if (ess_list->wl_ess_list_size == USHRT_MAX)
		return (ENOSPC);

	ieee80211_dbg(IEEE80211_MSG_CONFIG, "wl_get_esslist: "
	    "nodes num=%u\n", ess_list->wl_ess_list_num);

	return (0);
}

/*
 * MAC_PROP_WL_WEP_KEY
 */
static int
wl_set_wepkey(struct ieee80211com *ic, const void *wldp_buf)
{
	uint16_t i;
	uint32_t klen;
	struct ieee80211_key *key;
	wl_wep_key_t *wepkey = (wl_wep_key_t *)wldp_buf;

	/* set all valid keys */
	for (i = 0; i < IEEE80211_KEY_MAX; i++) {
		if (wepkey[i].wl_wep_operation != WL_ADD)
			continue;
		klen = wepkey[i].wl_wep_length;
		if (klen > IEEE80211_KEYBUF_SIZE) {
			ieee80211_err("wl_set_wepkey: "
			    "invalid wepkey length, %u\n", klen);
			return (EINVAL);
		}
		if (klen == 0)
			continue;

		/*
		 *  Set key contents. Only WEP is supported
		 */
		ieee80211_dbg(IEEE80211_MSG_CONFIG, "wl_set_wepkey: "
		    "set key %u, len=%u\n", i, klen);
		key = &ic->ic_nw_keys[i];
		if (ieee80211_crypto_newkey(ic, IEEE80211_CIPHER_WEP,
		    IEEE80211_KEY_XMIT | IEEE80211_KEY_RECV, key) == 0) {
			ieee80211_err("wl_set_wepkey: "
			    "abort, create key failed. id=%u\n", i);
			return (EIO);
		}

		key->wk_keyix = i;
		key->wk_keylen = (uint8_t)klen;
		key->wk_flags |= IEEE80211_KEY_XMIT | IEEE80211_KEY_RECV;
		bzero(key->wk_key, IEEE80211_KEYBUF_SIZE);
		bcopy(wepkey[i].wl_wep_keyval, key->wk_key, klen);
		if (ieee80211_crypto_setkey(ic, key, ic->ic_macaddr)
		    == 0) {
			ieee80211_err("wl_set_wepkey: "
			    "set key failed len=%u\n", klen);
			return (EIO);
		}
	}

	return (0);
}

/*
 * MAC_PROP_WL_OPTIE
 */

static int
wl_get_optie(struct ieee80211com *ic, const void *wldp_buf)
{
	wl_wpa_ie_t *ie_in = (wl_wpa_ie_t *)wldp_buf;

	if (ic->ic_opmode != IEEE80211_M_STA) {
		ieee80211_err("wl_get_optie: opmode err\n");
		return (EINVAL);
	}
	if (ic->ic_opt_ie_len == 0 ||
	    ic->ic_opt_ie_len > IEEE80211_MAX_OPT_IE) {
		ieee80211_err("wl_get_optie: optie is not set\n");
		return (EINVAL);
	}
	if (ic->ic_opt_ie == NULL) {
		ieee80211_dbg(IEEE80211_MSG_BRUSSELS,
		    "wl_get_optie: ic_opt_ie == NULL\n");
		return (EINVAL);
	}
	(void) memcpy(ie_in->wpa_ie, ic->ic_opt_ie, ic->ic_opt_ie_len);
	ie_in->wpa_ie_len = ic->ic_opt_ie_len;

	return (0);
}

static int
wl_set_optie(struct ieee80211com *ic, const void *wldp_buf)
{
	uint8_t *ie;
	wl_wpa_ie_t *ie_in = (wl_wpa_ie_t *)wldp_buf;

	if (ic->ic_opmode != IEEE80211_M_STA) {
		ieee80211_err("wl_set_optie: opmode err\n");
		return (EINVAL);
	}

	ieee80211_dbg(IEEE80211_MSG_BRUSSELS,
	    "wl_set_optie: opt_ie len %u\n", ie_in->wpa_ie_len);

	if (ic->ic_opt_ie != NULL)
		ieee80211_free(ic->ic_opt_ie);

	ie = ieee80211_malloc(ie_in->wpa_ie_len);
	(void) memcpy(ie, ie_in->wpa_ie, ie_in->wpa_ie_len);

	ic->ic_opt_ie = ie;
	ic->ic_opt_ie_len = ie_in->wpa_ie_len;

	return (0);
}

/*
 * MAC_PROP_WL_DELKEY
 */
static int
wl_set_delkey(struct ieee80211com *ic, const void *wldp_buf)
{
	int kid;
	wl_del_key_t *dk = (wl_del_key_t *)wldp_buf;

	ieee80211_dbg(IEEE80211_MSG_BRUSSELS, "wl_set_delkey(): "
	    "keyix=%d\n", dk->idk_keyix);

	kid = dk->idk_keyix;

	if (kid == IEEE80211_KEYIX_NONE ||
	    kid >= IEEE80211_WEP_NKID) {
		ieee80211_err("wl_set_delkey: incorrect keyix\n");
		return (EINVAL);
	} else {
		(void) ieee80211_crypto_delkey(ic,
		    &ic->ic_nw_keys[kid]);
		ieee80211_mac_update(ic);
	}

	return (0);
}

/*
 * MAC_PROP_WL_MLME
 */

static int
wl_set_mlme(struct ieee80211com *ic, const void *wldp_buf)
{
	ieee80211_node_t *in;
	wl_mlme_t *mlme = (wl_mlme_t *)wldp_buf;

	switch (mlme->im_op) {
	case IEEE80211_MLME_DISASSOC:
	case IEEE80211_MLME_DEAUTH:
		if (ic->ic_opmode == IEEE80211_M_STA) {
			ieee80211_dbg(IEEE80211_MSG_WPA,
			    "wl_set_mlme: DISASSOC <%s> \n",
			    ieee80211_macaddr_sprintf(mlme->im_macaddr));
			IEEE80211_UNLOCK(ic);
			ieee80211_new_state(ic, IEEE80211_S_INIT,
			    mlme->im_reason);
			IEEE80211_LOCK(ic);
		}
		break;
	case IEEE80211_MLME_ASSOC:
		if (ic->ic_opmode != IEEE80211_M_STA) {
			ieee80211_err("wifi_cfg_setmlme: opmode err\n");
			return (EINVAL);
		}
		ieee80211_dbg(IEEE80211_MSG_WPA,
		    "wl_set_mlme: ASSOC <%s> \n",
		    ieee80211_macaddr_sprintf(mlme->im_macaddr));
		if (ic->ic_flags & IEEE80211_F_DESBSSID) {
			in = ieee80211_find_node(&ic->ic_scan,
			    ic->ic_des_bssid);
		} else if (ic->ic_des_esslen != 0) {
		/*
		 * Desired ssid specified; must match both bssid and
		 * ssid to distinguish ap advertising multiple ssid's.
		 */
			in = ieee80211_find_node_with_ssid(&ic->ic_scan,
			    mlme->im_macaddr,
			    ic->ic_des_esslen,
			    ic->ic_des_essid);
		} else {
		/*
		 * Normal case; just match bssid.
		 */
			in = ieee80211_find_node(&ic->ic_scan,
			    mlme->im_macaddr);
		}
		if (in == NULL) {
			ieee80211_err("wifi_cfg_setmlme: no matched node\n");
			return (EINVAL);
		}
		IEEE80211_UNLOCK(ic);
		ieee80211_sta_join(ic, in);
		IEEE80211_LOCK(ic);
		break;
	default:
		return (EINVAL);
	}

	return (0);
}

/*
 * MAC_PROP_WL_WPA_KEY
 */
static int
wl_set_wpakey(struct ieee80211com *ic, const void *wldp_buf)
{
	int err = 0;
	struct ieee80211_node *in;
	struct ieee80211_key *wk;
	uint16_t kid;
	wl_key_t ik;

	bcopy(wldp_buf, &ik, sizeof (wl_key_t));

	/* Note: cipher support is verified by ieee80211_crypt_newkey */
	/* Note: this also checks ik->ik_keylen > sizeof (wk->wk_key) */
	if (ik.ik_keylen > sizeof (ik.ik_keydata))
		return (E2BIG);

	ieee80211_dbg(IEEE80211_MSG_BRUSSELS, "wl_set_wpakey: "
	    "idx=%u flags=0x%x\n", ik.ik_keyix, ik.ik_flags);

	kid = ik.ik_keyix;
	if (kid == IEEE80211_KEYIX_NONE) {
		/* XXX unicast keys currently must be tx/rx */
		if (ik.ik_flags != (IEEE80211_KEY_XMIT | IEEE80211_KEY_RECV |
		    IEEE80211_KEY_DEFAULT))
			return (EINVAL);
		if (ic->ic_opmode == IEEE80211_M_STA) {
			in = ieee80211_ref_node(ic->ic_bss);
			if (!IEEE80211_ADDR_EQ(ik.ik_macaddr, in->in_bssid)) {
				ieee80211_free_node(in);
				return (EADDRNOTAVAIL);
			}
		} else {
			in = ieee80211_find_node(&ic->ic_sta, ik.ik_macaddr);
			if (in == NULL)
				return (ENOENT);
		}
		wk = &ic->ic_nw_keys[ic->ic_def_txkey];
	} else {
		if (kid >= IEEE80211_WEP_NKID)
			return (EINVAL);
		wk = &ic->ic_nw_keys[kid];
		/*
		 * Global slots start off w/o any assigned key index.
		 * Force one here for consistency with IEEE80211_IOC_WEPKEY.
		 */
		if (wk->wk_keyix == IEEE80211_KEYIX_NONE)
			wk->wk_keyix = kid;
		in = NULL;
	}

	KEY_UPDATE_BEGIN(ic);
	if (ieee80211_crypto_newkey(ic, ik.ik_type, ik.ik_flags, wk)) {
		wk->wk_keylen = ik.ik_keylen;
		/* Note: MIC presence is implied by cipher type */
		if (wk->wk_keylen > IEEE80211_KEYBUF_SIZE)
			wk->wk_keylen = IEEE80211_KEYBUF_SIZE;
		wk->wk_keyrsc = ik.ik_keyrsc;
		wk->wk_keytsc = 0;			/* new key, reset */
		bzero(wk->wk_key, sizeof (wk->wk_key));
		bcopy(ik.ik_keydata, wk->wk_key, ik.ik_keylen);
		if (!ieee80211_crypto_setkey(ic, wk,
		    in != NULL ? in->in_macaddr : ik.ik_macaddr)) {
			err = EIO;
		} else if ((ik.ik_flags & IEEE80211_KEY_DEFAULT)) {
			ieee80211_mac_update(ic);
		}
	} else
		err = ENXIO;
	KEY_UPDATE_END(ic);
	if (in != NULL)
		ieee80211_free_node(in);

	return (err);
}

/*
 * MAC_PROP_WL_SUP_RATE
 */
static void
wl_get_suprates(struct ieee80211com *ic, void *wldp_buf)
{
	int i, j, k, l;
	uint8_t srates;
	uint8_t *drates;
	wl_rates_t *wl_rates;
	const struct ieee80211_rateset *srs;

	wl_rates = (wl_rates_t *)wldp_buf;

	wl_rates->wl_rates_num = 0;
	drates = (uint8_t *)wl_rates->wl_rates_rates;
	for (i = 0; i < IEEE80211_MODE_MAX; i++) {
		srs = &ic->ic_sup_rates[i];
		if (srs->ir_nrates == 0)
			continue;
		for (j = 0; j < srs->ir_nrates; j++) {
			srates = IEEE80211_RV(srs->ir_rates[j]);
			/* sort & skip duplicated rates */
			for (k = 0; k < wl_rates->wl_rates_num; k++) {
				if (srates <= drates[k])
					break;
			}
			if (srates == drates[k])
				/* skip duplicated rates */
				continue;
			/* sort */
			for (l = wl_rates->wl_rates_num; l > k; l--)
				drates[l] = drates[l-1];
			drates[k] = srates;
			wl_rates->wl_rates_num++;
		}
	}

}

/*
 * MAC_PROP_WL_CREATE_IBSS
 */
static int
wl_set_createibss(struct ieee80211com *ic, const void *wldp_buf)
{
	wl_create_ibss_t *iw_ibss = (wl_create_ibss_t *)wldp_buf;

	ieee80211_dbg(IEEE80211_MSG_CONFIG, "wl_set_ibss: "
	    "set createibss=%u\n", *iw_ibss);

	if ((ic->ic_caps & IEEE80211_C_IBSS) == 0)
		return (ENOTSUP);

	if (*iw_ibss) {
		if ((ic->ic_flags & IEEE80211_F_IBSSON) == 0) {
			ic->ic_flags |= IEEE80211_F_IBSSON;
			ic->ic_opmode = IEEE80211_M_IBSS;
			/*
			 * Yech, slot time may change depending on the
			 * operating mode so reset it to be sure
			 * everything is setup appropriately.
			 */
			ieee80211_reset_erp(ic);
			ieee80211_create_ibss(ic, ic->ic_ibss_chan);
			return (0);
		}
	} else {
		if (ic->ic_flags & IEEE80211_F_IBSSON)
			ic->ic_flags &= ~IEEE80211_F_IBSSON;
	}

	return (ENETRESET);
}

static void
wl_get_createibss(struct ieee80211com *ic, void *wldp_buf)
{
	wl_create_ibss_t *ow_ibss = (wl_create_ibss_t *)wldp_buf;

	*ow_ibss = (ic->ic_flags & IEEE80211_F_IBSSON)? 1 : 0;
}

/*
 * Typically invoked by drivers in response to request for
 * information or to change settings from the userland.
 *
 * Return value should be checked by WiFi drivers. Return 0
 * on success. Otherwise, return non-zero value to indicate
 * the error. Driver should operate as below when the return
 * error is:
 * ENETRESET	Reset wireless network and re-start to join a
 * 		WLAN, ENETRESET is returned when a configuration
 * 		parameter has been changed.
 * 		When acknowledge a M_IOCTL message, this error
 * 		is ignored
 *
 * Not yet implemented:
 * MAC_PROP_WL_POWER_MODE
 * MAC_PROP_WL_RADIO,
 */

/* ARGSUSED */
int
ieee80211_setprop(void *ic_arg, const char *pr_name, mac_prop_id_t wldp_pr_num,
    uint_t wldp_length, const void *wldp_buf)
{
	int err = 0;
	struct ieee80211com *ic = ic_arg;

	ASSERT(ic != NULL);
	IEEE80211_LOCK(ic);

	switch (wldp_pr_num) {
	/* mac_prop_id */
	case MAC_PROP_WL_ESSID:
		err = wl_set_essid(ic, wldp_buf);
		break;
	case MAC_PROP_WL_BSSID:
		err = wl_set_bssid(ic, wldp_buf);
		break;
	case MAC_PROP_WL_PHY_CONFIG:
		err = wl_set_phy(ic, wldp_buf);
		break;
	case MAC_PROP_WL_KEY_TAB:
		err = wl_set_wepkey(ic, wldp_buf);
		break;
	case MAC_PROP_WL_AUTH_MODE:
		err = wl_set_authmode(ic, wldp_buf);
		break;
	case MAC_PROP_WL_ENCRYPTION:
		err = wl_set_encrypt(ic, wldp_buf);
		break;
	case MAC_PROP_WL_BSSTYPE:
		err = wl_set_bsstype(ic, wldp_buf);
		break;
	case MAC_PROP_WL_DESIRED_RATES:
		err = wl_set_desrates(ic, wldp_buf);
		break;
	case MAC_PROP_WL_WPA:
		err = wl_set_wpa(ic, wldp_buf);
		break;
	case MAC_PROP_WL_COUNTERM:
		err = wl_set_counterm(ic, wldp_buf);
		break;
	case MAC_PROP_WL_KEY:
		err = wl_set_wpakey(ic, wldp_buf);
		break;
	case MAC_PROP_WL_DELKEY:
		err = wl_set_delkey(ic, wldp_buf);
		break;
	case MAC_PROP_WL_OPTIE:
		err = wl_set_optie(ic, wldp_buf);
		break;
	case MAC_PROP_WL_MLME:
		err = wl_set_mlme(ic, wldp_buf);
		break;
	case MAC_PROP_WL_CREATE_IBSS:
		err = wl_set_createibss(ic, wldp_buf);
		break;
	case MAC_PROP_WL_LINKSTATUS:
	case MAC_PROP_WL_ESS_LIST:
	case MAC_PROP_WL_SUPPORTED_RATES:
	case MAC_PROP_WL_RSSI:
	case MAC_PROP_WL_CAPABILITY:
	default:
		ieee80211_err("ieee80211_setprop: opmode not support\n");
		err = ENOTSUP;
		break;
	}

	IEEE80211_UNLOCK(ic);

	return (err);
}

/* ARGSUSED */
int
ieee80211_getprop(void *ic_arg, const char *pr_name, mac_prop_id_t wldp_pr_num,
    uint_t wldp_length, void *wldp_buf)
{
	int err = 0;
	struct ieee80211com *ic = ic_arg;

	ASSERT(ic != NULL);
	IEEE80211_LOCK(ic);

	switch (wldp_pr_num) {
	/* mac_prop_id */
	case MAC_PROP_WL_ESSID:
		wl_get_essid(ic, wldp_buf);
		break;
	case MAC_PROP_WL_BSSID:
		wl_get_bssid(ic, wldp_buf);
		break;
	case MAC_PROP_WL_PHY_CONFIG:
		err = wl_get_phy(ic, wldp_buf);
		break;
	case MAC_PROP_WL_AUTH_MODE:
		wl_get_authmode(ic, wldp_buf);
		break;
	case MAC_PROP_WL_ENCRYPTION:
		wl_get_encrypt(ic, wldp_buf);
		break;
	case MAC_PROP_WL_BSSTYPE:
		wl_get_bsstype(ic, wldp_buf);
		break;
	case MAC_PROP_WL_DESIRED_RATES:
		wl_get_desrates(ic, wldp_buf);
		break;
	case MAC_PROP_WL_LINKSTATUS:
		wl_get_linkstatus(ic, wldp_buf);
		break;
	case MAC_PROP_WL_ESS_LIST:
		err = wl_get_esslist(ic, wldp_buf, wldp_length);
		break;
	case MAC_PROP_WL_SUPPORTED_RATES:
		wl_get_suprates(ic, wldp_buf);
		break;
	case MAC_PROP_WL_RSSI:
		wl_get_rssi(ic, wldp_buf);
		break;
	case MAC_PROP_WL_CAPABILITY:
		wl_get_capability(ic, wldp_buf);
		break;
	case MAC_PROP_WL_WPA:
		wl_get_wpa(ic, wldp_buf);
		break;
	case MAC_PROP_WL_CREATE_IBSS:
		wl_get_createibss(ic, wldp_buf);
		break;
	case MAC_PROP_WL_OPTIE:
		err = wl_get_optie(ic, wldp_buf);
		break;
	case MAC_PROP_WL_KEY_TAB:
	case MAC_PROP_WL_KEY:
	case MAC_PROP_WL_DELKEY:
	case MAC_PROP_WL_COUNTERM:
	case MAC_PROP_WL_MLME:
		ieee80211_err("ieee80211_setprop: opmode err\n");
		err = EINVAL;
		break;
	default:
		ieee80211_err("ieee80211_setprop: opmode not support\n");
		err = ENOTSUP;
		break;
	}

	IEEE80211_UNLOCK(ic);

	return (err);
}

void ieee80211_propinfo(void *ic_arg, const char *pr_name,
    mac_prop_id_t wldp_pr_num, mac_prop_info_handle_t prh)
{
        _NOTE(ARGUNUSED(pr_name, ic_arg));

	/*
	 * By default permissions are read/write unless specified
	 * otherwise by the driver.
	 */

	switch (wldp_pr_num) {
	case MAC_PROP_WL_LINKSTATUS:
	case MAC_PROP_WL_ESS_LIST:
	case MAC_PROP_WL_SUPPORTED_RATES:
	case MAC_PROP_WL_RSSI:
	case MAC_PROP_WL_CAPABILITY:
	case MAC_PROP_WL_CREATE_IBSS:
		mac_prop_info_set_perm(prh, MAC_PROP_PERM_READ);
	}
}

/*
 * Register WPA door
 */
void
ieee80211_register_door(ieee80211com_t *ic, const char *drvname, int inst)
{
	(void) snprintf(ic->ic_wpadoor, MAX_IEEE80211STR, "%s_%s%d",
	    WPA_DOOR, drvname, inst);
}

/*
 * ieee80211_event_thread
 * open wpa door, send event to wpa_supplicant
 */
static void
ieee80211_event_thread(void *arg)
{
	ieee80211com_t *ic = arg;
	door_handle_t event_door = NULL;	/* Door for upcalls */
	door_info_t dinfo;
	door_arg_t darg;
	wl_event_t ev;

	if (arg == NULL)
		return;

	mutex_enter(&ic->ic_doorlock);

	ieee80211_dbg(IEEE80211_MSG_DEBUG, "ieee80211_event_thread(%d)\n",
	    ic->ic_ev_type);

	ev.wpa_ev_type = ic->ic_ev_type;
	ev.wpa_ev_reason = ic->ic_ev_reason;
	bcopy(ic->ic_ev_beacon, ev.wpa_ev_beacon, IEEE80211_ADDR_LEN);

	/*
	 * Locate the door used for upcalls
	 */
	if (door_ki_open(ic->ic_wpadoor, &event_door) != 0) {
		if (ic->ic_ev_type != NET80211_EVENT_INTERFACE_STATUS)
			ieee80211_err("ieee80211_event_thread: door_ki_open(%s)"
			    " err\n", ic->ic_wpadoor);
		goto out;
	}

	if (door_ki_info(event_door, &dinfo) != 0) {
		ieee80211_err("ieee80211_event_thread: door_ki_info(%s) err\n",
		    ic->ic_wpadoor);
		goto out;
	}

	if ((dinfo.di_attributes & DOOR_REVOKED) ||
	    (dinfo.di_attributes & DOOR_IS_UNREF)) {
		ieee80211_err("ieee80211_event_thread: door revoked (%s) err\n",
		    ic->ic_wpadoor);
		goto out;
	}

	darg.data_ptr = (char *)&ev;
	darg.data_size = sizeof (wl_event_t);
	darg.desc_ptr = NULL;
	darg.desc_num = 0;
	darg.rbuf = NULL;
	darg.rsize = 0;

	if (door_ki_upcall_limited(event_door, &darg, NULL, 0, 0) != 0)
		ieee80211_err("ieee80211_event_thread: door_ki_upcall() err\n");

out:
	if (event_door)	/* release our hold (if any) */
		door_ki_rele(event_door);

	mutex_exit(&ic->ic_doorlock);
}

/*
 * Send system messages to notify the device has joined a WLAN.
 * This is an OS specific function. illumos marks link status as up.
 */
void
ieee80211_notify_node_join(ieee80211com_t *ic, ieee80211_node_t *in)
{
	ieee80211_dbg(IEEE80211_MSG_WPA, "ieee80211_notify_node_join()");
	ic->ic_ev_type = NET80211_EVENT_ASSOC;
	if (in == ic->ic_bss)
		mac_link_update(ic->ic_mach, LINK_STATE_UP);

	/* notify after state change is over */
	(void) timeout(ieee80211_event_thread, (void *)ic, 0);
}

void
ieee80211_notify_node_leave(ieee80211com_t *ic, ieee80211_node_t *in)
{
	ieee80211_dbg(IEEE80211_MSG_WPA, "ieee80211_notify_node_leave()");
	ic->ic_ev_type = NET80211_EVENT_DISASSOC;
	if (in == ic->ic_bss)
		mac_link_update(ic->ic_mach, LINK_STATE_DOWN);
	(void) timeout(ieee80211_event_thread, (void *)ic, 0);
}

void
ieee80211_notify_scan_res(ieee80211com_t *ic)
{
	ieee80211_dbg(IEEE80211_MSG_WPA, "ieee80211_notify_scan_res()");
	ic->ic_ev_type = NET80211_EVENT_SCAN_RESULTS;

	/* notify after state change is over */
	(void) timeout(ieee80211_event_thread, (void *)ic, 0);
}

void
ieee80211_notify_rejected(ieee80211com_t *ic, ieee80211_node_t *in,
    uint16_t code)
{
	ieee80211_dbg(IEEE80211_MSG_WPA, "ieee80211_notify_rejected()");
	ic->ic_ev_type = NET80211_EVENT_ASSOC_REJECT;
	ic->ic_ev_reason = code;
	bcopy(in->in_bssid, ic->ic_ev_beacon, IEEE80211_ADDR_LEN);
	(void) timeout(ieee80211_event_thread, (void *)ic, 0);
}

void
ieee80211_notify_timedout(ieee80211com_t *ic, ieee80211_node_t *in)
{
	ieee80211_dbg(IEEE80211_MSG_WPA, "ieee80211_notify_timedout()");
	ic->ic_ev_type = NET80211_EVENT_ASSOC_TIMED_OUT;
	bcopy(in->in_bssid, ic->ic_ev_beacon, IEEE80211_ADDR_LEN);
	(void) timeout(ieee80211_event_thread, (void *)ic, 0);
}

void
ieee80211_notify_detach(ieee80211com_t *ic)
{
	size_t len;
	ieee80211_dbg(IEEE80211_MSG_WPA, "ieee80211_notify_detach()");
	ic->ic_ev_type = NET80211_EVENT_INTERFACE_STATUS;
	len = wifi_strnlen(WPA_DOOR, 100);
	bcopy(ic->ic_wpadoor + len, ic->ic_ev_beacon, IEEE80211_ADDR_LEN);
	ieee80211_event_thread(ic);
}
