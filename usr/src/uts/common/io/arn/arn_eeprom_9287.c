/*
 * Copyright (c) 2008 Sam Leffler, Errno Consulting
 * Copyright (c) 2010 Atheros Communications, Inc.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * $FreeBSD$
 */
#include <sys/types.h>

#include "arn_core.h"
#include "arn_hw.h"
#include "arn_ath9k.h"
#include "ah.h"
#include "arn_reg.h"
#include "arn_phy.h"

#define HALDEBUG(ah, mask, format, ...)    arn_dbg(mask, format, ##__VA_ARGS__)
#define HAL_DEBUG_ANY	ARN_DBG_ANY
#define HAL_DEBUG_ATTACH ARN_DBG_ATTACH
#define HAL_DEBUG_EEPROM ARN_DBG_EEPROM
#define HALASSERT ASSERT
#define   AH_NULL 0
#define __bswap16 ddi_swap16
#define __bswap32 ddi_swap32
#define AH_MIN ARN_MIN
#define ath_hal_eepromRead ath9k_hw_eeprom_read

/* similar with ar5416_get_eep_ver */
static int ar9287_get_eep_ver(struct ath_hal_5416 *ah)
{
        struct ath_hal_5416 *ahp = AH5416(ah);
        return (ahp->ah_eeprom.map9287.ee_base.baseEepHeader.version >> 12) & 0xF;
}

/* similar with ar5416_get_eep_rev */
static int ar9287_get_eep_rev(struct ath_hal_5416 *ah)
{
        struct ath_hal_5416 *ahp = AH5416(ah);
        return (ahp->ah_eeprom.map9287.ee_base.baseEepHeader.version) & 0xFFF;
}

/* get values from the local stored eeprom buffer data 
static HAL_STATUS v9287EepromGet(struct ath_hal *ah, int param, void *val) */
uint32_t ath9k_hw_get_eeprom_kite(struct ath_hal *ah, enum eeprom_param param)
{
#define	CHAN_A_IDX	0
#define	CHAN_B_IDX	1
#define	IS_VERS(op, v)	((pBase->version & AR5416_EEP_VER_MINOR_MASK) op (v))
   	struct ath_hal_5416 *ahp = AH5416(ah);
	HAL_EEPROM_9287 *ee = &ahp->ah_eeprom.map9287;
	const MODAL_EEP_9287_HEADER *pModal = &ee->ee_base.modalHeader;
	const BASE_EEP_9287_HEADER *pBase = &ee->ee_base.baseEepHeader;
        uint16_t ver_minor;

        ver_minor = pBase->version & AR9287_EEP_VER_MINOR_MASK;

	switch (param) {
        case EEP_NFTHRESH_2:
                return pModal->noiseFloorThreshCh[0];
        case EEP_MAC_LSW: //  AR_EEPROM_MAC(0)
        case EEP_MAC_0:
                return pBase->macAddr[0] << 8 | pBase->macAddr[1];
        case EEP_MAC_MID: //  AR_EEPROM_MAC(1)
        case EEP_MAC_1:
                return pBase->macAddr[2] << 8 | pBase->macAddr[3];
        case EEP_MAC_MSW: //  AR_EEPROM_MAC(2)
        case EEP_MAC_2:
                return pBase->macAddr[4] << 8 | pBase->macAddr[5];
        case EEP_REG_0:
                return pBase->regDmn[0];
        case EEP_REG_1:
                return pBase->regDmn[1];
        case EEP_OP_CAP:
                return pBase->deviceCap;
        case EEP_OP_MODE:
                return pBase->opCapFlags;
        case EEP_RF_SILENT:
                return pBase->rfSilent;
        case EEP_MINOR_REV:
                return ver_minor;
        case EEP_TX_MASK:
                return pBase->txMask;
        case EEP_RX_MASK:
                return pBase->rxMask;
        case EEP_DEV_TYPE:
                return pBase->deviceType;
        case EEP_OL_PWRCTRL:
                return pBase->openLoopPwrCntl;
        case EEP_TEMPSENSE_SLOPE:
                if (ver_minor >= AR9287_EEP_MINOR_VER_2)
                        return pBase->tempSensSlope;
                else
                        return 0;
        case EEP_TEMPSENSE_SLOPE_PAL_ON:
                if (ver_minor >= AR9287_EEP_MINOR_VER_3)
                        return pBase->tempSensSlopePalOn;
                else
                        return 0;
        default:
                return 0;
        }
}

#ifdef BSD
/* set values in the local stored eeprom buffer data */
static HAL_BOOL
v9287EepromSet(struct ath_hal *ah, int param, int v)
{
	HAL_EEPROM_9287 *ee = AH_PRIVATE(ah)->ah_eeprom;

	switch (param) {
	case AR_EEP_ANTGAINMAX_2:
		ee->ee_antennaGainMax[1] = (int8_t) v;
		return HAL_OK;
	case AR_EEP_ANTGAINMAX_5:
		ee->ee_antennaGainMax[0] = (int8_t) v;
		return HAL_OK;
	}
	return HAL_EINVAL;
}

static HAL_BOOL
v9287EepromDiag(struct ath_hal *ah, int request,
     const void *args, uint32_t argsize, void **result, uint32_t *resultsize)
{
	HAL_EEPROM_9287 *ee = AH_PRIVATE(ah)->ah_eeprom;

	switch (request) {
	case HAL_DIAG_EEPROM:
		*result = ee;
		*resultsize = sizeof(HAL_EEPROM_9287);
		return AH_TRUE;
	}
	return AH_FALSE;
}

/* Do structure specific swaps if Eeprom format is non native to host */
static void
eepromSwap(HAL_EEPROM_9287 *ee)
{
	uint32_t integer, i;
	uint16_t word;
	MODAL_EEP_9287_HEADER *pModal;

	/* convert Base Eep header */
	word = __bswap16(ee->ee_base.baseEepHeader.length);
	ee->ee_base.baseEepHeader.length = word;

	word = __bswap16(ee->ee_base.baseEepHeader.checksum);
	ee->ee_base.baseEepHeader.checksum = word;

	word = __bswap16(ee->ee_base.baseEepHeader.version);
	ee->ee_base.baseEepHeader.version = word;

	word = __bswap16(ee->ee_base.baseEepHeader.regDmn[0]);
	ee->ee_base.baseEepHeader.regDmn[0] = word;

	word = __bswap16(ee->ee_base.baseEepHeader.regDmn[1]);
	ee->ee_base.baseEepHeader.regDmn[1] = word;

	word = __bswap16(ee->ee_base.baseEepHeader.rfSilent);
	ee->ee_base.baseEepHeader.rfSilent = word;

	word = __bswap16(ee->ee_base.baseEepHeader.blueToothOptions);
	ee->ee_base.baseEepHeader.blueToothOptions = word; 

	word = __bswap16(ee->ee_base.baseEepHeader.deviceCap);
	ee->ee_base.baseEepHeader.deviceCap = word;

	/* convert Modal Eep header */

	/* only 2.4ghz here; so only one modal header entry */
	pModal = &ee->ee_base.modalHeader;

	/* XXX linux/ah_osdep.h only defines __bswap32 for BE */
	integer = __bswap32(pModal->antCtrlCommon);
	pModal->antCtrlCommon = integer;

	for (i = 0; i < AR9287_MAX_CHAINS; i++) {
		integer = __bswap32(pModal->antCtrlChain[i]);
		pModal->antCtrlChain[i] = integer;
	}
	for (i = 0; i < AR5416_EEPROM_MODAL_SPURS; i++) {
		word = __bswap16(pModal->spurChans[i].spurChan);
		pModal->spurChans[i].spurChan = word;
	}
}

static uint16_t 
v9287EepromGetSpurChan(struct ath_hal *ah, int ix, HAL_BOOL is2GHz)
{ 
	HAL_EEPROM_9287 *ee = AH_PRIVATE(ah)->ah_eeprom;
	
	HALASSERT(is2GHz == AH_TRUE);
	if (is2GHz != AH_TRUE)
		return 0;	/* XXX ? */
	
	HALASSERT(0 <= ix && ix <  AR5416_EEPROM_MODAL_SPURS);
	return ee->ee_base.modalHeader.spurChans[ix].spurChan;
}

/**************************************************************************
 * fbin2freq
 *
 * Get channel value from binary representation held in eeprom
 * RETURNS: the frequency in MHz
 */
static uint16_t
fbin2freq(uint8_t fbin, HAL_BOOL is2GHz)
{
	/*
	 * Reserved value 0xFF provides an empty definition both as
	 * an fbin and as a frequency - do not convert
	*/
	if (fbin == AR5416_BCHAN_UNUSED)
		return fbin;
	return (uint16_t)((is2GHz) ? (2300 + fbin) : (4800 + 5 * fbin));
}


/*
 * Copy EEPROM Conformance Testing Limits contents 
 * into the allocated space
 */
/* USE CTLS from chain zero */ 
#define CTL_CHAIN	0 

static void
v9287EepromReadCTLInfo(struct ath_hal *ah, HAL_EEPROM_9287 *ee)
{
	RD_EDGES_POWER *rep = ee->ee_rdEdgesPower;
	int i, j;
	
	HALASSERT(AR9287_NUM_CTLS <= sizeof(ee->ee_rdEdgesPower)/NUM_EDGES);

	for (i = 0; ee->ee_base.ctlIndex[i] != 0 && i < AR9287_NUM_CTLS; i++) {
		for (j = 0; j < NUM_EDGES; j ++) {
			/* XXX Confirm this is the right thing to do when an invalid channel is stored */
			if (ee->ee_base.ctlData[i].ctlEdges[CTL_CHAIN][j].bChannel == AR5416_BCHAN_UNUSED) {
				rep[j].rdEdge = 0;
				rep[j].twice_rdEdgePower = 0;
				rep[j].flag = 0;
			} else {
				rep[j].rdEdge = fbin2freq(
				    ee->ee_base.ctlData[i].ctlEdges[CTL_CHAIN][j].bChannel,
				    (ee->ee_base.ctlIndex[i] & CTL_MODE_M) != CTL_11A);
				rep[j].twice_rdEdgePower = ee->ee_base.ctlData[i].ctlEdges[CTL_CHAIN][j].tPower;
				rep[j].flag = ee->ee_base.ctlData[i].ctlEdges[CTL_CHAIN][j].flag != 0;
			}
		}
		rep += NUM_EDGES;
	}
	ee->ee_numCtls = i;
	HALDEBUG(ah, HAL_DEBUG_ATTACH | HAL_DEBUG_EEPROM,
	    "%s Numctls = %u\n",__func__,i);
}

/*
 * Reclaim any EEPROM-related storage.
 */
static void
v9287EepromDetach(struct ath_hal *ah)
{
	HAL_EEPROM_9287 *ee = AH_PRIVATE(ah)->ah_eeprom;
	ath_hal_free(ee);
	AH_PRIVATE(ah)->ah_eeprom = AH_NULL;
}

#define owl_get_eep_ver(_ee)   \
    (((_ee)->ee_base.baseEepHeader.version >> 12) & 0xF)
#define owl_get_eep_rev(_ee)   \
    (((_ee)->ee_base.baseEepHeader.version) & 0xFFF)

HAL_STATUS
ath_hal_9287EepromAttach(struct ath_hal *ah)
{
/* ath_hal_eepromRead <- REG_READ <- arn_ioread32 <- ddi_get32 */
/* ?? (sizeof(a) / sizeof(uint32_t)) */
#define	NW(a)	(sizeof(a) / sizeof(uint16_t))
	HAL_EEPROM_9287 *ee = AH_PRIVATE(ah)->ah_eeprom;
	uint16_t *eep_data, magic;
	HAL_BOOL need_swap;
	u_int w, off, len;
	uint32_t sum;

	HALASSERT(ee == AH_NULL);
 
	if (!ath_hal_eepromRead(ah, AR5416_EEPROM_MAGIC_OFFSET, &magic)) {
		HALDEBUG(ah, HAL_DEBUG_ANY,
		    "%s Error reading Eeprom MAGIC\n", __func__);
		return HAL_EEREAD;
	}
	HALDEBUG(ah, HAL_DEBUG_ATTACH, "%s Eeprom Magic = 0x%x\n",
	    __func__, magic);
	if (magic != AR5416_EEPROM_MAGIC) {
		HALDEBUG(ah, HAL_DEBUG_ANY, "Bad magic number\n");
		return HAL_EEMAGIC;
	}

	ee = ath_hal_malloc(sizeof(HAL_EEPROM_9287));
	if (ee == AH_NULL) {
		/* XXX message */
		return HAL_ENOMEM;
	}

	eep_data = (uint16_t *) ee;
	for (w = 0; w < NW(HAL_EEPROM_9287); w++) {
		off = AR9287_EEP_START_LOC + w;
		if (!ath_hal_eepromRead(ah, off, &eep_data[w])) {
			HALDEBUG(ah, HAL_DEBUG_ANY,
			    "%s eeprom read error at offset 0x%x\n",
			    __func__, off);
			return HAL_EEREAD;
		}
	}
	/* Convert to eeprom native eeprom endian format */
	if (isBigEndian()) {
		for (w = 0; w < NW(HAL_EEPROM_9287); w++)
			eep_data[w] = __bswap16(eep_data[w]);
	}

	/*
	 * At this point, we're in the native eeprom endian format
	 * Now, determine the eeprom endian by looking at byte 26??
	 */
	need_swap = ((ee->ee_base.baseEepHeader.eepMisc & AR5416_EEPMISC_BIG_ENDIAN) != 0) ^ isBigEndian();
	if (need_swap) {
		HALDEBUG(ah, HAL_DEBUG_ATTACH | HAL_DEBUG_EEPROM,
		    "Byte swap EEPROM contents.\n");
		len = __bswap16(ee->ee_base.baseEepHeader.length);
	} else {
		len = ee->ee_base.baseEepHeader.length;
	}
	len = AH_MIN(len, sizeof(HAL_EEPROM_9287)) / sizeof(uint16_t);
	
	/* Apply the checksum, done in native eeprom format */
	/* XXX - Need to check to make sure checksum calculation is done
	 * in the correct endian format.  Right now, it seems it would
	 * cast the raw data to host format and do the calculation, which may
	 * not be correct as the calculation may need to be done in the native
	 * eeprom format 
	 */
	sum = 0;
	for (w = 0; w < len; w++)
		sum ^= eep_data[w];
	/* Check CRC - Attach should fail on a bad checksum */
	if (sum != 0xffff) {
		HALDEBUG(ah, HAL_DEBUG_ANY,
		    "Bad EEPROM checksum 0x%x (Len=%u)\n", sum, len);
		return HAL_EEBADSUM;
	}

	if (need_swap)
		eepromSwap(ee);	/* byte swap multi-byte data */

	/* swap words 0+2 so version is at the front */
	magic = eep_data[0];
	eep_data[0] = eep_data[2];
	eep_data[2] = magic;

	HALDEBUG(ah, HAL_DEBUG_ATTACH | HAL_DEBUG_EEPROM,
	    "%s Eeprom Version %u.%u\n", __func__,
	    owl_get_eep_ver(ee), owl_get_eep_rev(ee));

	/* NB: must be after all byte swapping */
	if (owl_get_eep_ver(ee) != AR5416_EEP_VER) {
		HALDEBUG(ah, HAL_DEBUG_ANY,
		    "Bad EEPROM version 0x%x\n", owl_get_eep_ver(ee));
		return HAL_EEBADSUM;
	}

	v9287EepromReadCTLInfo(ah, ee);		/* Get CTLs */

	AH_PRIVATE(ah)->ah_eeprom = ee;
	AH_PRIVATE(ah)->ah_eeversion = ee->ee_base.baseEepHeader.version;
	AH_PRIVATE(ah)->ah_eepromDetach = v9287EepromDetach;
	AH_PRIVATE(ah)->ah_eepromGet = v9287EepromGet;
	AH_PRIVATE(ah)->ah_eepromSet = v9287EepromSet;
	AH_PRIVATE(ah)->ah_getSpurChan = v9287EepromGetSpurChan;
	AH_PRIVATE(ah)->ah_eepromDiag = v9287EepromDiag;
	return HAL_OK;
#undef NW
}
#endif 

/* reads EEPROM into memory */
boolean_t
ath9k_hw_fill_kite_eeprom(struct ath_hal *ah)
{
#define SIZE_EEPROM_KITE  (sizeof (HAL_EEPROM_9287) / sizeof (uint16_t))
        struct ath_hal_5416 *ahp = AH5416(ah);
        HAL_EEPROM_9287 *eep = &ahp->ah_eeprom.map9287;
        uint16_t *eep_data;
        int addr, eep_start_loc;

        eep_start_loc = AR9287_EEP_START_LOC;

        if (!ath9k_hw_use_flash(ah)) {
                ARN_DBG((ARN_DBG_EEPROM,
                    "Reading from EEPROM, not flash\n"));
        }

        eep_data = (uint16_t *)eep;

        for (addr = 0; addr < SIZE_EEPROM_KITE; addr++) {
                if (!ath9k_hw_nvram_read(ah, addr + eep_start_loc, eep_data)) {
                        ARN_DBG((ARN_DBG_EEPROM,
                            "Unable to read kite eeprom region \n"));
                        return (B_FALSE);
                }
                eep_data++;
        }
        return (B_TRUE);
#undef SIZE_EEPROM_KITE
}


int
ath9k_hw_check_kite_eeprom(struct ath_hal *ah)
{
        struct ath_hal_5416 *ahp = AH5416(ah);
        struct ar9287_eeprom *eep =
            (struct ar9287_eeprom *)&ahp->ah_eeprom.map9287.ee_base;
        uint16_t *eepdata, temp, magic, magic2;
        uint32_t sum = 0, el;
        boolean_t need_swap = B_FALSE;
        int i, addr;

        if (!ath9k_hw_use_flash(ah)) {

                if (!ath9k_hw_nvram_read(ah, AR5416_EEPROM_MAGIC_OFFSET,
                    &magic)) {
                        ARN_DBG((ARN_DBG_EEPROM,
                            "Reading Magic # failed\n"));
                        return (B_FALSE);
                }

                ARN_DBG((ARN_DBG_EEPROM,
                    "Read Magic = 0x%04X\n", magic));

                if (magic != AR5416_EEPROM_MAGIC) {
                        magic2 = swab16(magic);

                        if (magic2 == AR5416_EEPROM_MAGIC) {
                                need_swap = B_TRUE;
                                eepdata = (uint16_t *)(&ahp->ah_eeprom);

                                for (addr = 0; addr < (sizeof (HAL_EEPROM_9287) / sizeof (uint16_t)); addr++) {
                                        temp = swab16(*eepdata);
                                        *eepdata = temp;
                                        eepdata++;

                                        ARN_DBG((ARN_DBG_EEPROM,
                                            "0x%04X  ", *eepdata));

                                        if (((addr + 1) % 6) == 0)
                                                ARN_DBG((ARN_DBG_EEPROM, "\n"));
                                }
                        } else {
                                ARN_DBG((ARN_DBG_EEPROM,
                                    "Invalid EEPROM Magic. "
                                    "endianness mismatch.\n"));
                                return (EINVAL);
                        }
                }
        }

        ARN_DBG((ARN_DBG_EEPROM, "ath9k_hw_check_kite_eeprom: need_swap = %s.\n",
            need_swap ? "True" : "False"));


       if (need_swap)
                el = swab16(eep->baseEepHeader.length);
        else
                el = ahp->ah_eeprom.map9287.ee_base.baseEepHeader.length;

        ARN_DBG((ARN_DBG_EEPROM,
                    "ath9k_hw_check_kite_eeprom: EEPROM size %04x,%04x,%04x",
                    sizeof (struct ar5416_eeprom_4k), sizeof(HAL_EEPROM_9287), el));

        if (el > sizeof (HAL_EEPROM_9287 ))
                el = sizeof (HAL_EEPROM_9287) / sizeof (uint16_t);
        else
                el = el / sizeof (uint16_t);
        eepdata = (uint16_t *)(&ahp->ah_eeprom);

        for (i = 0; i < el; i++)
                sum ^= *eepdata++;

        if (need_swap) {
                uint32_t integer;
                uint16_t word;

                ARN_DBG((ARN_DBG_EEPROM,
                    "ath9k_hw_check_kite_eeprom: EEPROM Endianness is not native.. Changing \n"));

                word = swab16(eep->baseEepHeader.length);
                eep->baseEepHeader.length = word;

                word = swab16(eep->baseEepHeader.checksum);
                eep->baseEepHeader.checksum = word;

                word = swab16(eep->baseEepHeader.version);
                eep->baseEepHeader.version = word;

                word = swab16(eep->baseEepHeader.regDmn[0]);
                eep->baseEepHeader.regDmn[0] = word;

                word = swab16(eep->baseEepHeader.regDmn[1]);
                eep->baseEepHeader.regDmn[1] = word;

                word = swab16(eep->baseEepHeader.rfSilent);
                eep->baseEepHeader.rfSilent = word;

                word = swab16(eep->baseEepHeader.blueToothOptions);
                eep->baseEepHeader.blueToothOptions = word;

                word = swab16(eep->baseEepHeader.deviceCap);
                eep->baseEepHeader.deviceCap = word;

                integer = swab32(eep->modalHeader.antCtrlCommon);
                eep->modalHeader.antCtrlCommon = integer;

                for (i = 0; i < AR5416_EEP4K_MAX_CHAINS; i++) {
                        integer = swab32(eep->modalHeader.antCtrlChain[i]);
                        eep->modalHeader.antCtrlChain[i] = integer;
                }

                for (i = 0; i < AR5416_EEPROM_MODAL_SPURS; i++) {

                        word = swab16(eep->modalHeader.spurChans[i].spurChan);
                        eep->modalHeader.spurChans[i].spurChan = word;
                }
        }

        if (sum != 0xffff 
                  || ar9287_get_eep_ver(ahp) != AR9287_EEP_VER) {
                ARN_DBG((ARN_DBG_EEPROM,
                    "ath9k_hw_check_kite_eeprom: Bad EEPROM checksum 0x%x or version 0x%04x:%04x\n",
                    sum, ar9287_get_eep_ver(ahp), ar9287_get_eep_rev(ahp)));
                return (EINVAL);
        }
        ARN_DBG((ARN_DBG_EEPROM,
                    "ath9k_hw_check_kite_eeprom: EEPROM checksum 0x%x version 0x%04x\n",
                    sum, ar5416_get_eep_ver(ahp)));

        return (0);
}

static void ar9287_eeprom_get_tx_gain_index(struct ath_hal *ah,
                            struct ath9k_channel *chan,
                            struct cal_data_op_loop_ar9287 *pRawDatasetOpLoop,
                            uint8_t *pCalChans,  uint16_t availPiers, int8_t *pPwr)
{
        uint16_t idxL = 0, idxR = 0, numPiers;
        int match;
        struct chan_centers centers;

        ath9k_hw_get_channel_centers(ah, chan, &centers);

        for (numPiers = 0; numPiers < availPiers; numPiers++) {
                if (pCalChans[numPiers] == AR5416_BCHAN_UNUSED)
                        break;
        }

        match = ath9k_hw_get_lower_upper_index(
                (uint8_t)FREQ2FBIN(centers.synth_center, IS_CHAN_2GHZ(chan)),
                pCalChans, numPiers, &idxL, &idxR);

        if (match) {
                *pPwr = (int8_t) pRawDatasetOpLoop[idxL].pwrPdg[0][0];
        } else {
                *pPwr = ((int8_t) pRawDatasetOpLoop[idxL].pwrPdg[0][0] +
                         (int8_t) pRawDatasetOpLoop[idxR].pwrPdg[0][0])/2;
        }

}

#if 0 /* in BSD not for 9287 */
static void
ath9k_hw_get_kite_gain_boundaries_pdadcs(struct ath_hal *ah,
    struct ath9k_channel *chan,
    struct cal_data_per_freq_ar9287 *pRawDataSet,
    uint8_t *bChans, uint16_t availPiers,
    uint16_t tPdGainOverlap, 
    uint16_t *pPdGainBoundaries, uint8_t *pPDADCValues,
    uint16_t numXpdGains)
{
	int i, j, k;
	int16_t ss;
	uint16_t idxL = 0, idxR = 0, numPiers;
	static uint8_t vpdTableL[AR5416_NUM_PD_GAINS]
	    [AR5416_MAX_PWR_RANGE_IN_HALF_DB];
	static uint8_t vpdTableR[AR5416_NUM_PD_GAINS]
	    [AR5416_MAX_PWR_RANGE_IN_HALF_DB];
	static uint8_t vpdTableI[AR5416_NUM_PD_GAINS]
	    [AR5416_MAX_PWR_RANGE_IN_HALF_DB];

	uint8_t *pVpdL, *pVpdR, *pPwrL, *pPwrR;
	uint8_t minPwrT4[AR5416_NUM_PD_GAINS];
	uint8_t maxPwrT4[AR5416_NUM_PD_GAINS];
	int16_t vpdStep;
	int16_t tmpVal;
	uint16_t sizeCurrVpdTable, maxIndex, tgtIndex;
	boolean_t match;
	int16_t minDelta = 0;
	struct chan_centers centers;
        int pdgain_boundary_default;
        int intercepts;

       if (AR_SREV_9287(ah))
                intercepts = AR9287_PD_GAIN_ICEPTS;
        else
                intercepts = AR5416_PD_GAIN_ICEPTS;

        memset(&minPwrT4, 0, AR5416_NUM_PD_GAINS);

	ath9k_hw_get_channel_centers(ah, chan, &centers);

	for (numPiers = 0; numPiers < availPiers; numPiers++) {
		if (bChans[numPiers] == AR5416_BCHAN_UNUSED)
			break;
	}

	match =
	    ath9k_hw_get_lower_upper_index(
	    (uint8_t)FREQ2FBIN(centers.synth_center, IS_CHAN_2GHZ(chan)),
	    bChans, numPiers, &idxL, &idxR);

	if (match) {
		for (i = 0; i < numXpdGains; i++) {
			minPwrT4[i] = pRawDataSet[idxL].pwrPdg[i][0];
			maxPwrT4[i] = pRawDataSet[idxL].pwrPdg[i][4];
			(void) ath9k_hw_fill_vpd_table(minPwrT4[i], maxPwrT4[i],
			    pRawDataSet[idxL].pwrPdg[i],
			    pRawDataSet[idxL].vpdPdg[i],
			    intercepts,
			    vpdTableI[i]);
		}
	} else {
		for (i = 0; i < numXpdGains; i++) {
			pVpdL = pRawDataSet[idxL].vpdPdg[i];
			pPwrL = pRawDataSet[idxL].pwrPdg[i];
			pVpdR = pRawDataSet[idxR].vpdPdg[i];
			pPwrR = pRawDataSet[idxR].pwrPdg[i];

			minPwrT4[i] = max(pPwrL[0], pPwrR[0]);

			maxPwrT4[i] =
			    min(pPwrL[intercepts - 1],
			    pPwrR[intercepts - 1]);


			(void) ath9k_hw_fill_vpd_table(minPwrT4[i], maxPwrT4[i],
			    pPwrL, pVpdL,
			    intercepts,
			    vpdTableL[i]);
			(void) ath9k_hw_fill_vpd_table(minPwrT4[i], maxPwrT4[i],
			    pPwrR, pVpdR,
			    intercepts,
			    vpdTableR[i]);

			for (j = 0; j <= (maxPwrT4[i] - minPwrT4[i]) / 2; j++) {
				vpdTableI[i][j] =
				    (uint8_t)(ath9k_hw_interpolate((uint16_t)
				    FREQ2FBIN(centers.
				    synth_center,
				    IS_CHAN_2GHZ
				    (chan)),
				    bChans[idxL], bChans[idxR],
				    vpdTableL[i][j], vpdTableR[i][j]));
			}
		}
	}

	k = 0;

	for (i = 0; i < numXpdGains; i++) {
		if (i == (numXpdGains - 1))
			pPdGainBoundaries[i] =
			    (uint16_t)(maxPwrT4[i] / 2);
		else
			pPdGainBoundaries[i] =
			    (uint16_t)((maxPwrT4[i] + minPwrT4[i + 1]) / 4);

		pPdGainBoundaries[i] =
		    min((uint16_t)AR5416_MAX_RATE_POWER, pPdGainBoundaries[i]);

		if ((i == 0) && !AR_SREV_5416_V20_OR_LATER(ah)) {
			minDelta = pPdGainBoundaries[0] - 23;
			pPdGainBoundaries[0] = 23;
		} else {
			minDelta = 0;
		}

		if (i == 0) {
			if (AR_SREV_9280_10_OR_LATER(ah))
				ss = (int16_t)(0 - (minPwrT4[i] / 2));
			else
				ss = 0;
		} else {
			ss = (int16_t)((pPdGainBoundaries[i - 1] -
			    (minPwrT4[i] / 2)) -
			    tPdGainOverlap + 1 + minDelta);
		}
		vpdStep = (int16_t)(vpdTableI[i][1] - vpdTableI[i][0]);
		vpdStep = (int16_t)((vpdStep < 1) ? 1 : vpdStep);

		while ((ss < 0) && (k < (AR5416_NUM_PDADC_VALUES - 1))) {
			tmpVal = (int16_t)(vpdTableI[i][0] + ss * vpdStep);
			pPDADCValues[k++] =
			    (uint8_t)((tmpVal < 0) ? 0 : tmpVal);
			ss++;
		}

		sizeCurrVpdTable =
		    (uint8_t)((maxPwrT4[i] - minPwrT4[i]) / 2 + 1);
		tgtIndex = (uint8_t)(pPdGainBoundaries[i] + tPdGainOverlap -
		    (minPwrT4[i] / 2));
		maxIndex = (tgtIndex < sizeCurrVpdTable) ?
		    tgtIndex : sizeCurrVpdTable;

		while ((ss < maxIndex) && (k < (AR5416_NUM_PDADC_VALUES - 1))) {
			pPDADCValues[k++] = vpdTableI[i][ss++];
		}

		vpdStep = (int16_t)(vpdTableI[i][sizeCurrVpdTable - 1] -
		    vpdTableI[i][sizeCurrVpdTable - 2]);
		vpdStep = (int16_t)((vpdStep < 1) ? 1 : vpdStep);

		if (tgtIndex >= maxIndex) {
			while ((ss <= tgtIndex) &&
			    (k < (AR5416_NUM_PDADC_VALUES - 1))) {
				tmpVal =
				    (int16_t)
				    ((vpdTableI[i][sizeCurrVpdTable - 1] +
				    (ss - maxIndex + 1) * vpdStep));
				pPDADCValues[k++] = (uint8_t)((tmpVal > 255) ?
				    255 : tmpVal);
				ss++;
			}
		}
	}

	while (i < AR5416_PD_GAINS_IN_MASK) {
		pPdGainBoundaries[i] = pPdGainBoundaries[i - 1];
		i++;
	}

	while (k < AR5416_NUM_PDADC_VALUES) {
		pPDADCValues[k] = pPDADCValues[k - 1];
		k++;
	}
}
#endif

static void ar9287_eeprom_olpc_set_pdadcs(struct ath_hal *ah,
                                          int32_t txPower, uint16_t chain)
{
        uint32_t tmpVal;
        uint32_t a;

        /* Enable OLPC for chain 0 */

        tmpVal = REG_READ(ah, 0xa270);
        tmpVal = tmpVal & 0xFCFFFFFF;
        tmpVal = tmpVal | (0x3 << 24);
        REG_WRITE(ah, 0xa270, tmpVal);

        /* Enable OLPC for chain 1 */

        tmpVal = REG_READ(ah, 0xb270);
        tmpVal = tmpVal & 0xFCFFFFFF;
        tmpVal = tmpVal | (0x3 << 24);
        REG_WRITE(ah, 0xb270, tmpVal);

        /* Write the OLPC ref power for chain 0 */

        if (chain == 0) {
                tmpVal = REG_READ(ah, 0xa398);
                tmpVal = tmpVal & 0xff00ffff;
                a = (txPower)&0xff;
                tmpVal = tmpVal | (a << 16);
                REG_WRITE(ah, 0xa398, tmpVal);
        }

        /* Write the OLPC ref power for chain 1 */

        if (chain == 1) {
                tmpVal = REG_READ(ah, 0xb398);
                tmpVal = tmpVal & 0xff00ffff;
                a = (txPower)&0xff;
                tmpVal = tmpVal | (a << 16);
                REG_WRITE(ah, 0xb398, tmpVal);
        }
}

static void ath9k_hw_set_ar9287_power_cal_table(struct ath_hal *ah,
                                                struct ath9k_channel *chan,
                                                int16_t *pTxPowerIndexOffset)
{
        struct cal_data_per_freq_ar9287 *pRawDataset;
        struct cal_data_op_loop_ar9287 *pRawDatasetOpenLoop;
        uint8_t *pCalBChans = NULL;
        uint8_t pdadcValues[AR5416_NUM_PDADC_VALUES];
        uint16_t gainBoundaries[AR5416_PD_GAINS_IN_MASK];
        uint16_t numPiers = 0, i, j;
        uint16_t numXpdGain, xpdMask;
        uint16_t xpdGainValues[AR5416_NUM_PD_GAINS] = {0, 0, 0, 0};
        uint32_t reg32, regOffset, regval;
        int16_t diff = 0;
        struct ath_hal_5416 *ahp = AH5416(ah);
        struct ar9287_eeprom *eep =
            (struct ar9287_eeprom *)&ahp->ah_eeprom.map9287.ee_base;

        xpdMask = eep->modalHeader.xpdGain;

        if (IS_CHAN_2GHZ(chan)) {
                pCalBChans = eep->calFreqPier2G;
                numPiers = AR9287_NUM_2G_CAL_PIERS;
                if (ath9k_hw_get_eeprom_kite(ah, EEP_OL_PWRCTRL)) {
                        pRawDatasetOpenLoop =
                        (struct cal_data_op_loop_ar9287 *)eep->calPierData2G[0];
                        ahp->ah_initPDADC = pRawDatasetOpenLoop->vpdPdg[0][0];
                }
        }

        numXpdGain = 0;

        /* Calculate the value of xpdgains from the xpdGain Mask */
        for (i = 1; i <= AR5416_PD_GAINS_IN_MASK; i++) {
                if ((xpdMask >> (AR5416_PD_GAINS_IN_MASK - i)) & 1) {
                        if (numXpdGain >= AR5416_NUM_PD_GAINS)
                                break;
                        xpdGainValues[numXpdGain] =
                                (uint16_t)(AR5416_PD_GAINS_IN_MASK-i);
                        numXpdGain++;
                }
        }

        REG_RMW_FIELD(ah, AR_PHY_TPCRG1, AR_PHY_TPCRG1_NUM_PD_GAIN,
                      (numXpdGain - 1) & 0x3);
        REG_RMW_FIELD(ah, AR_PHY_TPCRG1, AR_PHY_TPCRG1_PD_GAIN_1,
                      xpdGainValues[0]);
        REG_RMW_FIELD(ah, AR_PHY_TPCRG1, AR_PHY_TPCRG1_PD_GAIN_2,
                      xpdGainValues[1]);
        REG_RMW_FIELD(ah, AR_PHY_TPCRG1, AR_PHY_TPCRG1_PD_GAIN_3,
                      xpdGainValues[2]);

        for (i = 0; i < AR9287_MAX_CHAINS; i++) {
                if (eep->baseEepHeader.txMask & (1 << i)) {
                        pRawDatasetOpenLoop =
                       (struct cal_data_op_loop_ar9287 *)eep->calPierData2G[i];

                                int8_t txPower;
                                ar9287_eeprom_get_tx_gain_index(ah, chan,
                                                        pRawDatasetOpenLoop,
                                                        pCalBChans, numPiers,
                                                        &txPower);
                                ar9287_eeprom_olpc_set_pdadcs(ah, txPower, i);

                }
        }

        *pTxPowerIndexOffset = 0;
}

static void ath9k_hw_set_ar9287_power_per_rate_table(struct ath_hal *ah,
                                                     struct ath9k_channel *chan,
                                                     int16_t *ratesArray,
                                                     uint16_t cfgCtl,
                                                     uint16_t AntennaReduction,
                                                     uint16_t twiceMaxRegulatoryPower,
                                                     uint16_t powerLimit)
{
#define CMP_CTL \
        (((cfgCtl & ~CTL_MODE_M) | (pCtlMode[ctlMode] & CTL_MODE_M)) == \
         eep->ctlIndex[i])

#define CMP_NO_CTL \
        (((cfgCtl & ~CTL_MODE_M) | (pCtlMode[ctlMode] & CTL_MODE_M)) == \
         ((eep->ctlIndex[i] & CTL_MODE_M) | SD_NO_CTL))

#define REDUCE_SCALED_POWER_BY_TWO_CHAIN     6
#define REDUCE_SCALED_POWER_BY_THREE_CHAIN   10

        struct ath_regulatory *regulatory = ath9k_hw_regulatory(ah);
        uint16_t twiceMaxEdgePower = MAX_RATE_POWER;
        static const uint16_t tpScaleReductionTable[5] =
                { 0, 3, 6, 9, MAX_RATE_POWER };
        unsigned int i;
        int16_t twiceLargestAntenna;
        struct cal_ctl_data_ar9287 *rep;
        struct cal_target_power_leg targetPowerOfdm = {0, {0, 0, 0, 0} },
                                    targetPowerCck = {0, {0, 0, 0, 0} };
        struct cal_target_power_leg targetPowerOfdmExt = {0, {0, 0, 0, 0} },
                                    targetPowerCckExt = {0, {0, 0, 0, 0} };
        struct cal_target_power_ht targetPowerHt20,
                                    targetPowerHt40 = {0, {0, 0, 0, 0} };
        uint16_t scaledPower = 0, minCtlPower, maxRegAllowedPower;
        static const uint16_t ctlModesFor11g[] = {
                CTL_11B, CTL_11G, CTL_2GHT20,
                CTL_11B_EXT, CTL_11G_EXT, CTL_2GHT40
        };
        uint16_t numCtlModes = 0;
        const uint16_t *pCtlMode = NULL;
        uint16_t ctlMode, freq;
        struct chan_centers centers;
        int tx_chainmask;
        uint16_t twiceMinEdgePower;
        struct ath_hal_5416 *ahp = AH5416(ah);
        struct ar9287_eeprom *eep =
            (struct ar9287_eeprom *)&ahp->ah_eeprom.map9287.ee_base;
        tx_chainmask = ahp->ah_txchainmask;

        ath9k_hw_get_channel_centers(ah, chan, &centers);

        /* Compute TxPower reduction due to Antenna Gain */
        twiceLargestAntenna = max(eep->modalHeader.antennaGainCh[0],
                                  eep->modalHeader.antennaGainCh[1]);
        twiceLargestAntenna = (int16_t)min((AntennaReduction) -
                                           twiceLargestAntenna, 0);

        /*
         * scaledPower is the minimum of the user input power level
         * and the regulatory allowed power level.
         */
        maxRegAllowedPower = twiceMaxRegulatoryPower + twiceLargestAntenna;

        if (regulatory->tp_scale != ATH9K_TP_SCALE_MAX)
                maxRegAllowedPower -=
                        (tpScaleReductionTable[(regulatory->tp_scale)] * 2);
       scaledPower = min(powerLimit, maxRegAllowedPower);

        /*
         * Reduce scaled Power by number of chains active
         * to get the per chain tx power level.
         */
        switch (ar5416_get_ntxchains(tx_chainmask)) {
        case 1:
                break;
        case 2:
                if (scaledPower > REDUCE_SCALED_POWER_BY_TWO_CHAIN)
                        scaledPower -= REDUCE_SCALED_POWER_BY_TWO_CHAIN;
                else
                        scaledPower = 0;
                break;
        case 3:
                if (scaledPower > REDUCE_SCALED_POWER_BY_THREE_CHAIN)
                        scaledPower -= REDUCE_SCALED_POWER_BY_THREE_CHAIN;
                else
                        scaledPower = 0;
                break;
        }
        scaledPower = max((uint16_t)0, scaledPower);

        /*
         * Get TX power from EEPROM.
         */
        if (IS_CHAN_2GHZ(chan)) {
                /* CTL_11B, CTL_11G, CTL_2GHT20 */
                numCtlModes =
                        ARRAY_SIZE(ctlModesFor11g) - SUB_NUM_CTL_MODES_AT_2G_40;

                pCtlMode = ctlModesFor11g;

                ath9k_hw_get_legacy_target_powers(ah, chan,
                                                  eep->calTargetPowerCck,
                                                  AR9287_NUM_2G_CCK_TARGET_POWERS,
                                                  &targetPowerCck, 4, 0);
                ath9k_hw_get_legacy_target_powers(ah, chan,
                                                  eep->calTargetPower2G,
                                                  AR9287_NUM_2G_20_TARGET_POWERS,
                                                  &targetPowerOfdm, 4, 0);
                ath9k_hw_get_target_powers(ah, chan,
                                           eep->calTargetPower2GHT20,
                                           AR9287_NUM_2G_20_TARGET_POWERS,
                                           &targetPowerHt20, 8, 0);

                if (IS_CHAN_HT40(chan)) {
                        /* All 2G CTLs */
                        numCtlModes = ARRAY_SIZE(ctlModesFor11g);
                        ath9k_hw_get_target_powers(ah, chan,
                                                   eep->calTargetPower2GHT40,
                                                   AR9287_NUM_2G_40_TARGET_POWERS,
                                                   &targetPowerHt40, 8, 1);
                        ath9k_hw_get_legacy_target_powers(ah, chan,
                                                  eep->calTargetPowerCck,
                                                  AR9287_NUM_2G_CCK_TARGET_POWERS,
                                                  &targetPowerCckExt, 4, 1);
                        ath9k_hw_get_legacy_target_powers(ah, chan,
                                                  eep->calTargetPower2G,
                                                  AR9287_NUM_2G_20_TARGET_POWERS,
                                                  &targetPowerOfdmExt, 4, 1);
                }
        }

        for (ctlMode = 0; ctlMode < numCtlModes; ctlMode++) {
                int isHt40CtlMode =
                        (pCtlMode[ctlMode] == CTL_2GHT40) ? 1 : 0;

                if (isHt40CtlMode)
                        freq = centers.synth_center;
                else if (pCtlMode[ctlMode] & EXT_ADDITIVE)
                        freq = centers.ext_center;
                else
                        freq = centers.ctl_center;

                /* Walk through the CTL indices stored in EEPROM */
                for (i = 0; (i < AR9287_NUM_CTLS) && eep->ctlIndex[i]; i++) {
                        struct cal_ctl_edges *pRdEdgesPower;

                        /*
                         * Compare test group from regulatory channel list
                         * with test mode from pCtlMode list
                         */
                        if (CMP_CTL || CMP_NO_CTL) {
                                rep = &(eep->ctlData[i]);
                                pRdEdgesPower =
                                rep->ctlEdges[ar5416_get_ntxchains(tx_chainmask) - 1];

                                twiceMinEdgePower = ath9k_hw_get_max_edge_power(freq,
                                                                pRdEdgesPower,
                                                                IS_CHAN_2GHZ(chan),
                                                                AR5416_NUM_BAND_EDGES);

                                if ((cfgCtl & ~CTL_MODE_M) == SD_NO_CTL) {
                                        twiceMaxEdgePower = min(twiceMaxEdgePower,
                                                                twiceMinEdgePower);
                                } else {
                                        twiceMaxEdgePower = twiceMinEdgePower;
                                        break;
                                }
                        }
                }

                minCtlPower = (uint8_t)min(twiceMaxEdgePower, scaledPower);

                /* Apply ctl mode to correct target power set */
                switch (pCtlMode[ctlMode]) {
                case CTL_11B:
                        for (i = 0; i < ARRAY_SIZE(targetPowerCck.tPow2x); i++) {
                                targetPowerCck.tPow2x[i] =
                                        (uint8_t)min((uint16_t)targetPowerCck.tPow2x[i],
                                                minCtlPower);
                        }
                        break;
                case CTL_11A:
                case CTL_11G:
                        for (i = 0; i < ARRAY_SIZE(targetPowerOfdm.tPow2x); i++) {
                                targetPowerOfdm.tPow2x[i] =
                                        (uint8_t)min((uint16_t)targetPowerOfdm.tPow2x[i],
                                                minCtlPower);
                        }
                        break;
                case CTL_5GHT20:
                case CTL_2GHT20:
                        for (i = 0; i < ARRAY_SIZE(targetPowerHt20.tPow2x); i++) {
                                targetPowerHt20.tPow2x[i] =
                                        (uint8_t)min((uint16_t)targetPowerHt20.tPow2x[i],
                                                minCtlPower);
                        }
                        break;
                case CTL_11B_EXT:
                        targetPowerCckExt.tPow2x[0] =
                                (uint8_t)min((uint16_t)targetPowerCckExt.tPow2x[0],
                                        minCtlPower);
                        break;
                case CTL_11A_EXT:
                case CTL_11G_EXT:
                        targetPowerOfdmExt.tPow2x[0] =
                                (uint8_t)min((uint16_t)targetPowerOfdmExt.tPow2x[0],
                                        minCtlPower);
                        break;
                case CTL_5GHT40:
                case CTL_2GHT40:
                        for (i = 0; i < ARRAY_SIZE(targetPowerHt40.tPow2x); i++) {
                                targetPowerHt40.tPow2x[i] =
                                        (uint8_t)min((uint16_t)targetPowerHt40.tPow2x[i],
                                                minCtlPower);
                        }
                        break;
                default:
                        break;
                }
        }

        /* Now set the rates array */

        ratesArray[rate6mb] =
        ratesArray[rate9mb] =
        ratesArray[rate12mb] =
        ratesArray[rate18mb] =
        ratesArray[rate24mb] = targetPowerOfdm.tPow2x[0];

        ratesArray[rate36mb] = targetPowerOfdm.tPow2x[1];
        ratesArray[rate48mb] = targetPowerOfdm.tPow2x[2];
        ratesArray[rate54mb] = targetPowerOfdm.tPow2x[3];
        ratesArray[rateXr] = targetPowerOfdm.tPow2x[0];

        for (i = 0; i < ARRAY_SIZE(targetPowerHt20.tPow2x); i++)
                ratesArray[rateHt20_0 + i] = targetPowerHt20.tPow2x[i];

        if (IS_CHAN_2GHZ(chan)) {
                ratesArray[rate1l] = targetPowerCck.tPow2x[0];
                ratesArray[rate2s] =
                ratesArray[rate2l] = targetPowerCck.tPow2x[1];
                ratesArray[rate5_5s] =
                ratesArray[rate5_5l] = targetPowerCck.tPow2x[2];
                ratesArray[rate11s] =
                ratesArray[rate11l] = targetPowerCck.tPow2x[3];
        }
        if (IS_CHAN_HT40(chan)) {
                for (i = 0; i < ARRAY_SIZE(targetPowerHt40.tPow2x); i++)
                        ratesArray[rateHt40_0 + i] = targetPowerHt40.tPow2x[i];

                ratesArray[rateDupOfdm] = targetPowerHt40.tPow2x[0];
                ratesArray[rateDupCck]  = targetPowerHt40.tPow2x[0];
                ratesArray[rateExtOfdm] = targetPowerOfdmExt.tPow2x[0];

                if (IS_CHAN_2GHZ(chan))
                        ratesArray[rateExtCck] = targetPowerCckExt.tPow2x[0];
        }
#undef CMP_CTL
#undef CMP_NO_CTL
#undef REDUCE_SCALED_POWER_BY_TWO_CHAIN
#undef REDUCE_SCALED_POWER_BY_THREE_CHAIN
}



int
ath9k_hw_kite_set_txpower(struct ath_hal *ah,
    struct ath9k_channel *chan,
    uint16_t cfgCtl,
    uint8_t twiceAntennaReduction,
    uint8_t twiceMaxRegulatoryPower,
    uint8_t powerLimit)
{
        struct ath_regulatory *regulatory = ath9k_hw_regulatory(ah);
        struct ath_hal_5416 *ahp = AH5416(ah);
        struct ar9287_eeprom *eep =
            (struct ar9287_eeprom *)&ahp->ah_eeprom.map9287.ee_base;
        MODAL_EEP_9287_HEADER *pModal = &eep->modalHeader;
        int16_t ratesArray[Ar5416RateSize];
        int16_t txPowerIndexOffset = 0;
        uint8_t ht40PowerIncForPdadc = 2;
        uint16_t i;
   
	(void) memset(ratesArray, 0, sizeof (ratesArray));
        if ((eep->baseEepHeader.version & AR9287_EEP_VER_MINOR_MASK) >=
            AR9287_EEP_MINOR_VER_2)
                ht40PowerIncForPdadc = pModal->ht40PowerIncForPdadc;

        ath9k_hw_set_ar9287_power_per_rate_table(ah, chan,
                                                 &ratesArray[0], cfgCtl,
                                                 twiceAntennaReduction,
                                                 twiceMaxRegulatoryPower,
                                                 powerLimit);

        ath9k_hw_set_ar9287_power_cal_table(ah, chan, &txPowerIndexOffset);

        regulatory->max_power_level = 0;
        for (i = 0; i < ARRAY_SIZE(ratesArray); i++) {
                ratesArray[i] = (int16_t)(txPowerIndexOffset + ratesArray[i]);
                if (ratesArray[i] > MAX_RATE_POWER)
                        ratesArray[i] = MAX_RATE_POWER;

                if (ratesArray[i] > regulatory->max_power_level)
                        regulatory->max_power_level = ratesArray[i];
        }

        /*
        if (test)
                return;
        */

        if (IS_CHAN_2GHZ(chan))
                i = rate1l;
        else
                i = rate6mb;

        regulatory->max_power_level = ratesArray[i];

        if (AR_SREV_9280_20_OR_LATER(ah)) {
                for (i = 0; i < Ar5416RateSize; i++)
                        ratesArray[i] -= AR9287_PWR_TABLE_OFFSET_DB * 2;
        }
#ifdef linux
        ENABLE_REGWRITE_BUFFER(ah);
#endif

        /* OFDM power per rate */
        REG_WRITE(ah, AR_PHY_POWER_TX_RATE1,
                  ATH9K_POW_SM(ratesArray[rate18mb], 24)
                  | ATH9K_POW_SM(ratesArray[rate12mb], 16)
                  | ATH9K_POW_SM(ratesArray[rate9mb], 8)
                  | ATH9K_POW_SM(ratesArray[rate6mb], 0));

        REG_WRITE(ah, AR_PHY_POWER_TX_RATE2,
                  ATH9K_POW_SM(ratesArray[rate54mb], 24)
                  | ATH9K_POW_SM(ratesArray[rate48mb], 16)
                  | ATH9K_POW_SM(ratesArray[rate36mb], 8)
                  | ATH9K_POW_SM(ratesArray[rate24mb], 0));

        /* CCK power per rate */
        if (IS_CHAN_2GHZ(chan)) {
                REG_WRITE(ah, AR_PHY_POWER_TX_RATE3,
                          ATH9K_POW_SM(ratesArray[rate2s], 24)
                          | ATH9K_POW_SM(ratesArray[rate2l], 16)
                          | ATH9K_POW_SM(ratesArray[rateXr], 8)
                          | ATH9K_POW_SM(ratesArray[rate1l], 0));
                REG_WRITE(ah, AR_PHY_POWER_TX_RATE4,
                          ATH9K_POW_SM(ratesArray[rate11s], 24)
                          | ATH9K_POW_SM(ratesArray[rate11l], 16)
                          | ATH9K_POW_SM(ratesArray[rate5_5s], 8)
                          | ATH9K_POW_SM(ratesArray[rate5_5l], 0));
	}

       /* HT20 power per rate */
        REG_WRITE(ah, AR_PHY_POWER_TX_RATE5,
                  ATH9K_POW_SM(ratesArray[rateHt20_3], 24)
                  | ATH9K_POW_SM(ratesArray[rateHt20_2], 16)
                  | ATH9K_POW_SM(ratesArray[rateHt20_1], 8)
                  | ATH9K_POW_SM(ratesArray[rateHt20_0], 0));

        REG_WRITE(ah, AR_PHY_POWER_TX_RATE6,
                  ATH9K_POW_SM(ratesArray[rateHt20_7], 24)
                  | ATH9K_POW_SM(ratesArray[rateHt20_6], 16)
                  | ATH9K_POW_SM(ratesArray[rateHt20_5], 8)
                  | ATH9K_POW_SM(ratesArray[rateHt20_4], 0));

        /* HT40 power per rate */
        if (IS_CHAN_HT40(chan)) {
                if (ath9k_hw_get_eeprom_kite(ah, EEP_OL_PWRCTRL)) {
                        REG_WRITE(ah, AR_PHY_POWER_TX_RATE7,
                                  ATH9K_POW_SM(ratesArray[rateHt40_3], 24)
                                  | ATH9K_POW_SM(ratesArray[rateHt40_2], 16)
                                  | ATH9K_POW_SM(ratesArray[rateHt40_1], 8)
                                  | ATH9K_POW_SM(ratesArray[rateHt40_0], 0));

                        REG_WRITE(ah, AR_PHY_POWER_TX_RATE8,
                                  ATH9K_POW_SM(ratesArray[rateHt40_7], 24)
                                  | ATH9K_POW_SM(ratesArray[rateHt40_6], 16)
                                  | ATH9K_POW_SM(ratesArray[rateHt40_5], 8)
                                  | ATH9K_POW_SM(ratesArray[rateHt40_4], 0));
                } else {
                        REG_WRITE(ah, AR_PHY_POWER_TX_RATE7,
                                  ATH9K_POW_SM(ratesArray[rateHt40_3] +
                                               ht40PowerIncForPdadc, 24)
                                  | ATH9K_POW_SM(ratesArray[rateHt40_2] +
                                                 ht40PowerIncForPdadc, 16)
                                  | ATH9K_POW_SM(ratesArray[rateHt40_1] +
                                                 ht40PowerIncForPdadc, 8)
                                  | ATH9K_POW_SM(ratesArray[rateHt40_0] +
                                                 ht40PowerIncForPdadc, 0));

                        REG_WRITE(ah, AR_PHY_POWER_TX_RATE8,
                                  ATH9K_POW_SM(ratesArray[rateHt40_7] +
                                               ht40PowerIncForPdadc, 24)
                                  | ATH9K_POW_SM(ratesArray[rateHt40_6] +
                                                 ht40PowerIncForPdadc, 16)
                                  | ATH9K_POW_SM(ratesArray[rateHt40_5] +
                                                 ht40PowerIncForPdadc, 8)
                                  | ATH9K_POW_SM(ratesArray[rateHt40_4] +
                                                 ht40PowerIncForPdadc, 0));
                }

                /* Dup/Ext power per rate */
                REG_WRITE(ah, AR_PHY_POWER_TX_RATE9,
                          ATH9K_POW_SM(ratesArray[rateExtOfdm], 24)
                          | ATH9K_POW_SM(ratesArray[rateExtCck], 16)
                          | ATH9K_POW_SM(ratesArray[rateDupOfdm], 8)
                          | ATH9K_POW_SM(ratesArray[rateDupCck], 0));
        }
#ifdef linux
        REGWRITE_BUFFER_FLUSH(ah);
#endif
        return (0);
}

boolean_t
ath9k_hw_eeprom_set_kite_board_values(struct ath_hal *ah,
    struct ath9k_channel *chan)
{
        struct ath_hal_5416 *ahp = AH5416(ah);
        struct ar9287_eeprom *eep =
            (struct ar9287_eeprom *)&ahp->ah_eeprom.map9287.ee_base;
	MODAL_EEP_9287_HEADER *pModal = &eep->modalHeader;
        int regChainOffset, regval;
        uint8_t txRxAttenLocal;
        int i, j;

        REG_WRITE(ah, AR_PHY_SWITCH_COM, pModal->antCtrlCommon);

        for (i = 0; i < AR9287_MAX_CHAINS; i++) {
                regChainOffset = i * 0x1000;

                REG_WRITE(ah, AR_PHY_SWITCH_CHAIN_0 + regChainOffset,
                          pModal->antCtrlChain[i]);

                REG_WRITE(ah, AR_PHY_TIMING_CTRL4(0) + regChainOffset,
                          (REG_READ(ah, AR_PHY_TIMING_CTRL4(0)
                              + regChainOffset)
                           & ~(AR_PHY_TIMING_CTRL4_IQCORR_Q_Q_COFF |
                               AR_PHY_TIMING_CTRL4_IQCORR_Q_I_COFF)) |
                          SM(pModal->iqCalICh[i],
                             AR_PHY_TIMING_CTRL4_IQCORR_Q_I_COFF) |
                          SM(pModal->iqCalQCh[i],
                             AR_PHY_TIMING_CTRL4_IQCORR_Q_Q_COFF));

                txRxAttenLocal = pModal->txRxAttenCh[i];

                REG_RMW_FIELD(ah, AR_PHY_GAIN_2GHZ + regChainOffset,
                              AR_PHY_GAIN_2GHZ_XATTEN1_MARGIN,
                              pModal->bswMargin[i]);
                REG_RMW_FIELD(ah, AR_PHY_GAIN_2GHZ + regChainOffset,
                              AR_PHY_GAIN_2GHZ_XATTEN1_DB,
                              pModal->bswAtten[i]);
                REG_RMW_FIELD(ah, AR_PHY_RXGAIN + regChainOffset,
                              AR9280_PHY_RXGAIN_TXRX_ATTEN,
                              txRxAttenLocal);
                REG_RMW_FIELD(ah, AR_PHY_RXGAIN + regChainOffset,
                              AR9280_PHY_RXGAIN_TXRX_MARGIN,
                              pModal->rxTxMarginCh[i]);
        }

        if (IS_CHAN_HT40(chan))
                REG_RMW_FIELD(ah, AR_PHY_SETTLING,
                              AR_PHY_SETTLING_SWITCH, pModal->swSettleHt40);
        else
                REG_RMW_FIELD(ah, AR_PHY_SETTLING,
                              AR_PHY_SETTLING_SWITCH, pModal->switchSettling);

        REG_RMW_FIELD(ah, AR_PHY_DESIRED_SZ,
                      AR_PHY_DESIRED_SZ_ADC, pModal->adcDesiredSize);

        REG_WRITE(ah, AR_PHY_RF_CTL4,
                  SM(pModal->txEndToXpaOff, AR_PHY_RF_CTL4_TX_END_XPAA_OFF)
                  | SM(pModal->txEndToXpaOff, AR_PHY_RF_CTL4_TX_END_XPAB_OFF)
                  | SM(pModal->txFrameToXpaOn, AR_PHY_RF_CTL4_FRAME_XPAA_ON)
                  | SM(pModal->txFrameToXpaOn, AR_PHY_RF_CTL4_FRAME_XPAB_ON));

        REG_RMW_FIELD(ah, AR_PHY_RF_CTL3,
                      AR_PHY_TX_END_TO_A2_RX_ON, pModal->txEndToRxOn);

        REG_RMW_FIELD(ah, AR_PHY_CCA,
                      AR9280_PHY_CCA_THRESH62, pModal->thresh62);
        REG_RMW_FIELD(ah, AR_PHY_EXT_CCA0,
                      AR_PHY_EXT_CCA0_THRESH62, pModal->thresh62);

        regval = REG_READ(ah, AR9287_AN_RF2G3_CH0);
        regval &= ~(AR9287_AN_RF2G3_DB1 |
                    AR9287_AN_RF2G3_DB2 |
                    AR9287_AN_RF2G3_OB_CCK |
                    AR9287_AN_RF2G3_OB_PSK |
                    AR9287_AN_RF2G3_OB_QAM |
                    AR9287_AN_RF2G3_OB_PAL_OFF);
        regval |= (SM(pModal->db1, AR9287_AN_RF2G3_DB1) |
                   SM(pModal->db2, AR9287_AN_RF2G3_DB2) |
                   SM(pModal->ob_cck, AR9287_AN_RF2G3_OB_CCK) |
                   SM(pModal->ob_psk, AR9287_AN_RF2G3_OB_PSK) |
                   SM(pModal->ob_qam, AR9287_AN_RF2G3_OB_QAM) |
                   SM(pModal->ob_pal_off, AR9287_AN_RF2G3_OB_PAL_OFF));

        /* Analog write - requires a 100usec delay */
        REG_WRITE(ah, AR9287_AN_RF2G3_CH0, regval);
	drv_usecwait(100000);

        regval = REG_READ(ah, AR9287_AN_RF2G3_CH1);
        regval &= ~(AR9287_AN_RF2G3_DB1 |
                    AR9287_AN_RF2G3_DB2 |
                    AR9287_AN_RF2G3_OB_CCK |
                    AR9287_AN_RF2G3_OB_PSK |
                    AR9287_AN_RF2G3_OB_QAM |
                    AR9287_AN_RF2G3_OB_PAL_OFF);
        regval |= (SM(pModal->db1, AR9287_AN_RF2G3_DB1) |
                   SM(pModal->db2, AR9287_AN_RF2G3_DB2) |
                   SM(pModal->ob_cck, AR9287_AN_RF2G3_OB_CCK) |
                   SM(pModal->ob_psk, AR9287_AN_RF2G3_OB_PSK) |
                   SM(pModal->ob_qam, AR9287_AN_RF2G3_OB_QAM) |
                   SM(pModal->ob_pal_off, AR9287_AN_RF2G3_OB_PAL_OFF));

        REG_WRITE(ah, AR9287_AN_RF2G3_CH1, regval);
	drv_usecwait(100000);

        REG_RMW_FIELD(ah, AR_PHY_RF_CTL2,
            AR_PHY_TX_FRAME_TO_DATA_START, pModal->txFrameToDataStart);
        REG_RMW_FIELD(ah, AR_PHY_RF_CTL2,
            AR_PHY_TX_FRAME_TO_PA_ON, pModal->txFrameToPaOn);

        REG_RMW_FIELD(ah, AR9287_AN_TOP2,
            AR9287_AN_TOP2_XPABIAS_LVL, pModal->xpaBiasLvl);
	drv_usecwait(100000);

        return AH_TRUE;
}

