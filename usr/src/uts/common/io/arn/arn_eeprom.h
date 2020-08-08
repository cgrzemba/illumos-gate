/*
 * Copyright (c) 2008 Atheros Communications Inc.
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
 */

#ifndef _ARN_EEPROM_H
#define _ARN_EEPROM_H

#ifdef __cplusplus
extern "C" {
#endif

/* #include "ah.h" */

#define AR_EEPROM_VER1          0x1000  /* Version 1.0; 5210 only */
/*
 * Version 3 EEPROMs are all 16K.
 * 3.1 adds turbo limit, antenna gain, 16 CTL's, 11g info,
 *      and 2.4Ghz ob/db for B & G
 * 3.2 has more accurate pcdac intercepts and analog chip
 *      calibration.
 * 3.3 adds ctl in-band limit, 32 ctl's, and frequency
 *      expansion
 * 3.4 adds xr power, gainI, and 2.4 turbo params
 */
#define AR_EEPROM_VER3          0x3000  /* Version 3.0; start of 16k EEPROM */
#define AR_EEPROM_VER3_1        0x3001  /* Version 3.1 */
#define AR_EEPROM_VER3_2        0x3002  /* Version 3.2 */
#define AR_EEPROM_VER3_3        0x3003  /* Version 3.3 */
#define AR_EEPROM_VER3_4        0x3004  /* Version 3.4 */
#define AR_EEPROM_VER4          0x4000  /* Version 4.x */
#define AR_EEPROM_VER4_0        0x4000  /* Version 4.0 */
#define AR_EEPROM_VER4_1        0x4001  /* Version 4.0 */
#define AR_EEPROM_VER4_2        0x4002  /* Version 4.0 */
#define AR_EEPROM_VER4_3        0x4003  /* Version 4.0 */
#define AR_EEPROM_VER4_6        0x4006  /* Version 4.0 */
#define AR_EEPROM_VER4_7        0x3007  /* Version 4.7 */
#define AR_EEPROM_VER4_9        0x4009  /* EEPROM EAR futureproofing */
#define AR_EEPROM_VER5          0x5000  /* Version 5.x */
#define AR_EEPROM_VER5_0        0x5000  /* Adds new 2413 cal powers and added params */
#define AR_EEPROM_VER5_1        0x5001  /* Adds capability values */
#define AR_EEPROM_VER5_3        0x5003  /* Adds spur mitigation table */
#define AR_EEPROM_VER5_4        0x5004
/*
 * Version 14 EEPROMs came in with AR5416.
 * 14.2 adds txFrameToPaOn, txFrameToDataStart, ht40PowerInc
 * 14.3 adds bswAtten, bswMargin, swSettle, and base OpFlags for HT20/40
 */
#define AR_EEPROM_VER14         0xE000  /* Version 14.x */
#define AR_EEPROM_VER14_1       0xE001  /* Adds 11n support */
#define AR_EEPROM_VER14_2       0xE002
#define AR_EEPROM_VER14_3       0xE003
#define AR_EEPROM_VER14_7       0xE007
#define AR_EEPROM_VER14_9       0xE009
#define AR_EEPROM_VER14_16      0xE010
#define AR_EEPROM_VER14_17      0xE011
#define AR_EEPROM_VER14_19      0xE013

enum {
        AR_EEP_RFKILL,          /* use ath_hal_eepromGetFlag */
        AR_EEP_AMODE,           /* use ath_hal_eepromGetFlag */
        AR_EEP_BMODE,           /* use ath_hal_eepromGetFlag */
        AR_EEP_GMODE,           /* use ath_hal_eepromGetFlag */
        AR_EEP_TURBO5DISABLE,   /* use ath_hal_eepromGetFlag */
        AR_EEP_TURBO2DISABLE,   /* use ath_hal_eepromGetFlag */
        AR_EEP_ISTALON,         /* use ath_hal_eepromGetFlag */
        AR_EEP_32KHZCRYSTAL,    /* use ath_hal_eepromGetFlag */
        AR_EEP_MACADDR,         /* uint8_t* */
        AR_EEP_COMPRESS,        /* use ath_hal_eepromGetFlag */
        AR_EEP_FASTFRAME,       /* use ath_hal_eepromGetFlag */
        AR_EEP_AES,             /* use ath_hal_eepromGetFlag */
        AR_EEP_BURST,           /* use ath_hal_eepromGetFlag */
        AR_EEP_MAXQCU,          /* uint16_t* */
        AR_EEP_KCENTRIES,       /* uint16_t* */
        AR_EEP_NFTHRESH_5,      /* int16_t* */
        AR_EEP_NFTHRESH_2,      /* int16_t* */
        AR_EEP_REGDMN_0,        /* uint16_t* */
        AR_EEP_REGDMN_1,        /* uint16_t* */
        AR_EEP_OPCAP,           /* uint16_t* */
        AR_EEP_OPMODE,          /* uint16_t* */
        AR_EEP_RFSILENT,        /* uint16_t* */
        AR_EEP_OB_5,            /* uint8_t* */
        AR_EEP_DB_5,            /* uint8_t* */
        AR_EEP_OB_2,            /* uint8_t* */
        AR_EEP_DB_2,            /* uint8_t* */
        AR_EEP_TXMASK,          /* uint8_t* */
        AR_EEP_RXMASK,          /* uint8_t* */
        AR_EEP_RXGAIN_TYPE,     /* uint8_t* */
        AR_EEP_TXGAIN_TYPE,     /* uint8_t* */
        AR_EEP_DAC_HPWR_5G,     /* uint8_t* */
        AR_EEP_OL_PWRCTRL,      /* use ath_hal_eepromGetFlag */
        AR_EEP_FSTCLK_5G,       /* use ath_hal_eepromGetFlag */
        AR_EEP_ANTGAINMAX_5,    /* int8_t* */
        AR_EEP_ANTGAINMAX_2,    /* int8_t* */
        AR_EEP_WRITEPROTECT,    /* use ath_hal_eepromGetFlag */
        AR_EEP_PWR_TABLE_OFFSET,/* int8_t* */
        AR_EEP_PWDCLKIND        /* uint8_t* */
};

#define AR_EEPROM_MODAL_SPURS   5

/* historic should not used anymore */
#define AR_EEPROM_MAC(i)        (0x1d+(i))

#define AR5416_EEPROM_MAGIC_OFFSET      0x0
#ifdef __BIG_ENDIAN
#define AR5416_EEPROM_MAGIC     0x5aa5
#else
#define AR5416_EEPROM_MAGIC     0xa55a
#endif
#define AR5416_EEPMISC_BIG_ENDIAN            0x01

#define AR5416_MAX_CHAINS               3
#define AR5416_EEP4K_MAX_CHAINS               1
#define AR5416_NUM_5G_CAL_PIERS         8
#define AR5416_NUM_2G_CAL_PIERS         4
#define AR5416_NUM_5G_20_TARGET_POWERS  8
#define AR5416_NUM_5G_40_TARGET_POWERS  8
#define AR5416_NUM_2G_CCK_TARGET_POWERS 3
#define AR5416_NUM_2G_20_TARGET_POWERS  4
#define AR5416_NUM_2G_40_TARGET_POWERS  4
#define AR5416_NUM_CTLS                 24
#define AR5416_NUM_BAND_EDGES           8
#define AR5416_NUM_PD_GAINS             4
#define AR5416_PD_GAINS_IN_MASK         4
#define AR5416_PD_GAIN_ICEPTS           5
#define AR5416_NUM_PDADC_VALUES         128
#define AR5416_BCHAN_UNUSED             0xFF
#define AR5416_MAX_PWR_RANGE_IN_HALF_DB 64
#define AR5416_MAX_CHAINS               3
#define AR9300_MAX_CHAINS               3
#define AR5416_PWR_TABLE_OFFSET_DB     -5

/* Rx gain type values */
#define AR5416_EEP_RXGAIN_23DB_BACKOFF     0
#define AR5416_EEP_RXGAIN_13DB_BACKOFF     1
#define AR5416_EEP_RXGAIN_ORIG             2

/* Tx gain type values */
#define AR5416_EEP_TXGAIN_ORIGINAL         0
#define AR5416_EEP_TXGAIN_HIGH_POWER       1

#define AR5416_EEP4K_START_LOC                64
#define AR5416_EEP4K_NUM_2G_CAL_PIERS         3
#define AR5416_EEP4K_NUM_2G_CCK_TARGET_POWERS 3
#define AR5416_EEP4K_NUM_2G_20_TARGET_POWERS  3
#define AR5416_EEP4K_NUM_2G_40_TARGET_POWERS  3
#define AR5416_EEP4K_NUM_CTLS                 12
#define AR5416_EEP4K_NUM_BAND_EDGES           4
#define AR5416_EEP4K_NUM_PD_GAINS             2
#define AR5416_EEP4K_MAX_CHAINS               1

#define AR5416_OPFLAGS_11G              0x02

#define AR9287_EEP_VER               0xE
#define AR9287_EEP_VER_MINOR_MASK    0xFFF
#define AR9287_EEP_MINOR_VER_1       0x1
#define AR9287_EEP_MINOR_VER_2       0x2
#define AR9287_EEP_MINOR_VER_3       0x3
#define AR9287_EEP_MINOR_VER         AR9287_EEP_MINOR_VER_3
#define AR9287_EEP_MINOR_VER_b       AR9287_EEP_MINOR_VER
#define AR9287_EEP_NO_BACK_VER       AR9287_EEP_MINOR_VER_1

#define AR9287_EEP_START_LOC            128
#define AR9287_HTC_EEP_START_LOC        256
#define AR9287_NUM_2G_CAL_PIERS         3
#define AR9287_NUM_2G_CCK_TARGET_POWERS 3
#define AR9287_NUM_2G_20_TARGET_POWERS  3
#define AR9287_NUM_2G_40_TARGET_POWERS  3
#define AR9287_NUM_CTLS                 12
#define AR9287_NUM_BAND_EDGES           4
#define AR9287_PD_GAIN_ICEPTS           1
#define AR9287_EEPMISC_BIG_ENDIAN       0x01
#define AR9287_EEPMISC_WOW              0x02
#define AR9287_MAX_CHAINS               2
#define AR9287_ANT_16S                  32

#define AR9287_DATA_SZ                  32

#define AR9287_PWR_TABLE_OFFSET_DB  -5

#define AR9287_CHECKSUM_LOCATION (AR9287_EEP_START_LOC + 1)

#define AR9287_AN_RF2G3_CH0                     0x7808
#define AR9287_AN_RF2G3_CH1                     0x785c
#define AR9287_AN_RF2G3_DB1                     0xE0000000
#define AR9287_AN_RF2G3_DB1_S                   29
#define AR9287_AN_RF2G3_DB2                     0x1C000000
#define AR9287_AN_RF2G3_DB2_S                   26
#define AR9287_AN_RF2G3_OB_CCK                  0x03800000
#define AR9287_AN_RF2G3_OB_CCK_S                23
#define AR9287_AN_RF2G3_OB_PSK                  0x00700000
#define AR9287_AN_RF2G3_OB_PSK_S                20
#define AR9287_AN_RF2G3_OB_QAM                  0x000E0000
#define AR9287_AN_RF2G3_OB_QAM_S                17
#define AR9287_AN_RF2G3_OB_PAL_OFF              0x0001C000
#define AR9287_AN_RF2G3_OB_PAL_OFF_S            14

#define AR9287_AN_TXPC0                         0x7898
#define AR9287_AN_TXPC0_TXPCMODE                0x0000C000
#define AR9287_AN_TXPC0_TXPCMODE_S              14
#define AR9287_AN_TXPC0_TXPCMODE_NORMAL         0
#define AR9287_AN_TXPC0_TXPCMODE_TEST           1
#define AR9287_AN_TXPC0_TXPCMODE_TEMPSENSE      2
#define AR9287_AN_TXPC0_TXPCMODE_ATBTEST        3

#define AR9287_AN_TOP2                          0x78b4
#define AR9287_AN_TOP2_XPABIAS_LVL              0xC0000000
#define AR9287_AN_TOP2_XPABIAS_LVL_S            30


enum eeprom_param {
        EEP_NFTHRESH_5 = 0,
        EEP_NFTHRESH_2,
        EEP_MAC_MSW,
        EEP_MAC_MID,
        EEP_MAC_LSW,
        EEP_REG_0,
        EEP_REG_1,
        EEP_OP_CAP,
        EEP_OP_MODE,
        EEP_RF_SILENT,
        EEP_OB_5,
        EEP_DB_5,
        EEP_OB_2,
        EEP_DB_2,
        EEP_MINOR_REV,
        EEP_TX_MASK,
        EEP_RX_MASK,
        EEP_RXGAIN_TYPE,
        EEP_TXGAIN_TYPE,
        EEP_OL_PWRCTRL,
        EEP_RC_CHAIN_MASK,
        EEP_DAC_HPWR_5G,
        EEP_FRAC_N_5G,
        EEP_DEV_TYPE,
        EEP_TEMPSENSE_SLOPE,
        EEP_TEMPSENSE_SLOPE_PAL_ON,
        EEP_PWR_TABLE_OFFSET,
        EEP_DRIVE_STRENGTH,
        EEP_INTERNAL_REGULATOR,
        EEP_SWREG,
        EEP_PAPRD,
        EEP_MODAL_VER,
        EEP_ANT_DIV_CTL1,
        EEP_CHAIN_MASK_REDUCE,
        EEP_MAC_0 = AR_EEPROM_MAC(0), /* 1d+i; so should be at the end */
        EEP_MAC_1 = AR_EEPROM_MAC(1),
        EEP_MAC_2 = AR_EEPROM_MAC(2)
};

enum ar5416_rates {
        rate6mb, rate9mb, rate12mb, rate18mb,
        rate24mb, rate36mb, rate48mb, rate54mb,
        rate1l, rate2l, rate2s, rate5_5l,
        rate5_5s, rate11l, rate11s, rateXr,
        rateHt20_0, rateHt20_1, rateHt20_2, rateHt20_3,
        rateHt20_4, rateHt20_5, rateHt20_6, rateHt20_7,
        rateHt40_0, rateHt40_1, rateHt40_2, rateHt40_3,
        rateHt40_4, rateHt40_5, rateHt40_6, rateHt40_7,
        rateDupCck, rateDupOfdm, rateExtCck, rateExtOfdm,
        Ar5416RateSize
};

enum ath9k_hal_freq_band {
        ATH9K_HAL_FREQ_BAND_5GHZ = 0,
        ATH9K_HAL_FREQ_BAND_2GHZ = 1
};

#pragma pack(1)
/* 2.6.30 */
struct base_eep_header {
        uint16_t length;
        uint16_t checksum;
        uint16_t version;
        uint8_t opCapFlags;
        uint8_t eepMisc;
        uint16_t regDmn[2];
        uint8_t macAddr[6];
        uint8_t rxMask;
        uint8_t txMask;
        uint16_t rfSilent;
        uint16_t blueToothOptions;
        uint16_t deviceCap;
        uint32_t binBuildNumber;
        uint8_t deviceType;
        uint8_t pwdclkind;
        uint8_t futureBase_1[2];
        uint8_t rxGainType;
        uint8_t dacHiPwrMode_5G;
        uint8_t openLoopPwrCntl;
        uint8_t dacLpMode;
        uint8_t txGainType;
        uint8_t rcChainMask;
        uint8_t desiredScaleCCK;
        uint8_t power_table_offset;
        uint8_t frac_n_5g;
        uint8_t futureBase_3[21];
};

struct base_eep_header_4k {
        uint16_t length;
        uint16_t checksum;
        uint16_t version;
        uint8_t opCapFlags;
        uint8_t eepMisc;
        uint16_t regDmn[2];
        uint8_t macAddr[6];
        uint8_t rxMask;
        uint8_t txMask;
        uint16_t rfSilent;
        uint16_t blueToothOptions;
        uint16_t deviceCap;
        uint32_t binBuildNumber;
        uint8_t deviceType;
        uint8_t futureBase[1];
};

struct spur_chan {
        uint16_t spurChan;
        uint8_t spurRangeLow;
        uint8_t spurRangeHigh;
};

struct modal_eep_header {
        uint32_t antCtrlChain[AR5416_MAX_CHAINS];
        uint32_t antCtrlCommon;
        uint8_t antennaGainCh[AR5416_MAX_CHAINS];
        uint8_t switchSettling;
        uint8_t txRxAttenCh[AR5416_MAX_CHAINS];
        uint8_t rxTxMarginCh[AR5416_MAX_CHAINS];
        uint8_t adcDesiredSize;
        uint8_t pgaDesiredSize;
        uint8_t xlnaGainCh[AR5416_MAX_CHAINS];
        uint8_t txEndToXpaOff;
        uint8_t txEndToRxOn;
        uint8_t txFrameToXpaOn;
        uint8_t thresh62;
        uint8_t noiseFloorThreshCh[AR5416_MAX_CHAINS];
        uint8_t xpdGain;
        uint8_t xpd;
        uint8_t iqCalICh[AR5416_MAX_CHAINS];
        uint8_t iqCalQCh[AR5416_MAX_CHAINS];
        uint8_t pdGainOverlap;
        uint8_t ob;
        uint8_t db;
        uint8_t xpaBiasLvl;
        uint8_t pwrDecreaseFor2Chain;
        uint8_t pwrDecreaseFor3Chain;
        uint8_t txFrameToDataStart;
        uint8_t txFrameToPaOn;
        uint8_t ht40PowerIncForPdadc;
        uint8_t bswAtten[AR5416_MAX_CHAINS];
        uint8_t bswMargin[AR5416_MAX_CHAINS];
        uint8_t swSettleHt40;
        uint8_t xatten2Db[AR5416_MAX_CHAINS];
        uint8_t xatten2Margin[AR5416_MAX_CHAINS];
        uint8_t ob_ch1;
        uint8_t db_ch1;
        uint8_t useAnt1:1,
            force_xpaon:1,
            local_bias:1,
            femBandSelectUsed:1, xlnabufin:1, xlnaisel:2, xlnabufmode:1;
        uint8_t futureModalar9280;
        uint16_t xpaBiasLvlFreq[3];
        uint8_t futureModal[6];

        struct spur_chan spurChans[AR_EEPROM_MODAL_SPURS];
};

struct modal_eep_4k_header {
    uint32_t  antCtrlChain[AR5416_EEP4K_MAX_CHAINS];
    uint32_t  antCtrlCommon;
    uint8_t   antennaGainCh[AR5416_EEP4K_MAX_CHAINS];
    uint8_t   switchSettling;
    uint8_t   txRxAttenCh[AR5416_EEP4K_MAX_CHAINS];
    uint8_t   rxTxMarginCh[AR5416_EEP4K_MAX_CHAINS];
    uint8_t   adcDesiredSize;
    uint8_t   pgaDesiredSize;
    uint8_t   xlnaGainCh[AR5416_EEP4K_MAX_CHAINS];
    uint8_t   txEndToXpaOff;
    uint8_t   txEndToRxOn;
    uint8_t   txFrameToXpaOn;
    uint8_t   thresh62;
    uint8_t   noiseFloorThreshCh[AR5416_EEP4K_MAX_CHAINS];
    uint8_t   xpdGain;
    uint8_t   xpd;
    uint8_t   iqCalICh[AR5416_EEP4K_MAX_CHAINS];
    uint8_t   iqCalQCh[AR5416_EEP4K_MAX_CHAINS];
    uint8_t   pdGainOverlap;
    uint8_t   ob_01;
    uint8_t   db1_01;
    uint8_t   xpaBiasLvl;
    uint8_t   txFrameToDataStart;
    uint8_t   txFrameToPaOn;
    uint8_t   ht40PowerIncForPdadc;
    uint8_t   bswAtten[AR5416_EEP4K_MAX_CHAINS];
    uint8_t   bswMargin[AR5416_EEP4K_MAX_CHAINS];
    uint8_t   swSettleHt40;
    uint8_t   xatten2Db[AR5416_EEP4K_MAX_CHAINS];
    uint8_t   xatten2Margin[AR5416_EEP4K_MAX_CHAINS];
    uint8_t   db2_01;
    uint8_t   version;
    uint16_t  ob_234;
    uint16_t  db1_234;
    uint16_t  db2_234;
    uint8_t   futureModal[4];

    struct spur_chan spurChans[AR_EEPROM_MODAL_SPURS];
};

struct base_eep_ar9287_header {
        uint16_t length;
        uint16_t checksum;
        uint16_t version;
        uint8_t opCapFlags;
        uint8_t eepMisc;
        uint16_t regDmn[2];
        uint8_t macAddr[6];
        uint8_t rxMask;
        uint8_t txMask;
        uint16_t rfSilent;
        uint16_t blueToothOptions;
        uint16_t deviceCap;
        uint32_t binBuildNumber;
        uint8_t deviceType;
        uint8_t openLoopPwrCntl;
        int8_t pwrTableOffset;
        int8_t tempSensSlope;
        int8_t tempSensSlopePalOn;
        uint8_t futureBase[29];
};

struct modal_eep_ar9287_header {
        uint32_t antCtrlChain[AR9287_MAX_CHAINS];
        uint32_t antCtrlCommon;
        int8_t antennaGainCh[AR9287_MAX_CHAINS];
        uint8_t switchSettling;
        uint8_t txRxAttenCh[AR9287_MAX_CHAINS];
        uint8_t rxTxMarginCh[AR9287_MAX_CHAINS];
        int8_t adcDesiredSize;
        uint8_t txEndToXpaOff;
        uint8_t txEndToRxOn;
        uint8_t txFrameToXpaOn;
        uint8_t thresh62;
        int8_t noiseFloorThreshCh[AR9287_MAX_CHAINS];
        uint8_t xpdGain;
        uint8_t xpd;
        int8_t iqCalICh[AR9287_MAX_CHAINS];
        int8_t iqCalQCh[AR9287_MAX_CHAINS];
        uint8_t pdGainOverlap;
        uint8_t xpaBiasLvl;
        uint8_t txFrameToDataStart;
        uint8_t txFrameToPaOn;
        uint8_t ht40PowerIncForPdadc;
        uint8_t bswAtten[AR9287_MAX_CHAINS];
        uint8_t bswMargin[AR9287_MAX_CHAINS];
        uint8_t swSettleHt40;
        uint8_t version;
        uint8_t db1;
        uint8_t db2;
        uint8_t ob_cck;
        uint8_t ob_psk;
        uint8_t ob_qam;
        uint8_t ob_pal_off;
        uint8_t futureModal[30];
        struct spur_chan spurChans[AR_EEPROM_MODAL_SPURS];
};

struct cal_data_per_freq {
        uint8_t pwrPdg[AR5416_NUM_PD_GAINS][AR5416_PD_GAIN_ICEPTS];
        uint8_t vpdPdg[AR5416_NUM_PD_GAINS][AR5416_PD_GAIN_ICEPTS];
} __attribute__((packed));

struct cal_data_per_freq_4k {
        uint8_t pwrPdg[AR5416_EEP4K_NUM_PD_GAINS][AR5416_PD_GAIN_ICEPTS];
        uint8_t vpdPdg[AR5416_EEP4K_NUM_PD_GAINS][AR5416_PD_GAIN_ICEPTS];
} __attribute__((packed));

struct cal_target_power_leg {
        uint8_t bChannel;
        uint8_t tPow2x[4];
} __attribute__((packed));

struct cal_target_power_ht {
        uint8_t bChannel;
        uint8_t tPow2x[8];
} __attribute__((packed));

#ifdef __BIG_ENDIAN_BITFIELD
struct cal_ctl_edges {
        uint8_t bChannel;
        uint8_t flag:2, tPower:6;
};
#else
struct cal_ctl_edges {
        uint8_t bChannel;
        uint8_t tPower:6, flag:2;
#ifdef BSD
#define CAL_CTL_EDGES_POWER     0x3f
#define CAL_CTL_EDGES_POWER_S   0
#define CAL_CTL_EDGES_FLAG      0xc0
#define CAL_CTL_EDGES_FLAG_S    6
#endif
};
#endif

struct cal_data_op_loop_ar9287 {
        uint8_t pwrPdg[2][5];
        uint8_t vpdPdg[2][5];
        uint8_t pcdac[2][5];
        uint8_t empty[2][5];
} __attribute__((packed));

struct cal_data_per_freq_ar9287 {
        uint8_t pwrPdg[AR5416_NUM_PD_GAINS][AR9287_PD_GAIN_ICEPTS];
        uint8_t vpdPdg[AR5416_NUM_PD_GAINS][AR9287_PD_GAIN_ICEPTS];
} __attribute__((packed));

union cal_data_per_freq_ar9287_u {
        struct cal_data_op_loop_ar9287 calDataOpen;
        struct cal_data_per_freq_ar9287 calDataClose;
} __attribute__((packed));


struct cal_ctl_data_ar9287 {
        struct cal_ctl_edges
        ctlEdges[AR9287_MAX_CHAINS][AR9287_NUM_BAND_EDGES];
} __attribute__((packed));

struct cal_ctl_data {
        struct cal_ctl_edges
        ctlEdges[AR5416_MAX_CHAINS][AR5416_NUM_BAND_EDGES];
} __attribute__((packed));

struct cal_ctl_data_4k {
        struct cal_ctl_edges
        ctlEdges[AR5416_EEP4K_MAX_CHAINS][AR5416_EEP4K_NUM_BAND_EDGES];
} __attribute__((packed));

struct ar5416_eeprom_def {
        struct base_eep_header baseEepHeader;
        uint8_t custData[64];
        struct modal_eep_header modalHeader[2];
        uint8_t calFreqPier5G[AR5416_NUM_5G_CAL_PIERS];
        uint8_t calFreqPier2G[AR5416_NUM_2G_CAL_PIERS];
        struct cal_data_per_freq
            calPierData5G[AR5416_MAX_CHAINS][AR5416_NUM_5G_CAL_PIERS];
        struct cal_data_per_freq
            calPierData2G[AR5416_MAX_CHAINS][AR5416_NUM_2G_CAL_PIERS];
        struct cal_target_power_leg
            calTargetPower5G[AR5416_NUM_5G_20_TARGET_POWERS];
        struct cal_target_power_ht
            calTargetPower5GHT20[AR5416_NUM_5G_20_TARGET_POWERS];
        struct cal_target_power_ht
            calTargetPower5GHT40[AR5416_NUM_5G_40_TARGET_POWERS];
        struct cal_target_power_leg
            calTargetPowerCck[AR5416_NUM_2G_CCK_TARGET_POWERS];
        struct cal_target_power_leg
            calTargetPower2G[AR5416_NUM_2G_20_TARGET_POWERS];
        struct cal_target_power_ht
            calTargetPower2GHT20[AR5416_NUM_2G_20_TARGET_POWERS];
        struct cal_target_power_ht
            calTargetPower2GHT40[AR5416_NUM_2G_40_TARGET_POWERS];
        uint8_t ctlIndex[AR5416_NUM_CTLS];
        struct cal_ctl_data ctlData[AR5416_NUM_CTLS];
        uint8_t padding;
};

struct ar5416_eeprom_4k {
        struct base_eep_header_4k baseEepHeader;
        uint8_t custData[20];
        struct modal_eep_4k_header modalHeader;
        uint8_t calFreqPier2G[AR5416_EEP4K_NUM_2G_CAL_PIERS];
        struct cal_data_per_freq_4k
        calPierData2G[AR5416_EEP4K_MAX_CHAINS][AR5416_EEP4K_NUM_2G_CAL_PIERS];
        struct cal_target_power_leg
        calTargetPowerCck[AR5416_EEP4K_NUM_2G_CCK_TARGET_POWERS];
        struct cal_target_power_leg
        calTargetPower2G[AR5416_EEP4K_NUM_2G_20_TARGET_POWERS];
        struct cal_target_power_ht
        calTargetPower2GHT20[AR5416_EEP4K_NUM_2G_20_TARGET_POWERS];
        struct cal_target_power_ht
        calTargetPower2GHT40[AR5416_EEP4K_NUM_2G_40_TARGET_POWERS];
        uint8_t ctlIndex[AR5416_EEP4K_NUM_CTLS];
        struct cal_ctl_data_4k ctlData[AR5416_EEP4K_NUM_CTLS];
        uint8_t padding;
};

struct ar9287_eeprom {
        struct base_eep_ar9287_header baseEepHeader;
        uint8_t custData[AR9287_DATA_SZ];
        struct modal_eep_ar9287_header modalHeader;
        uint8_t calFreqPier2G[AR9287_NUM_2G_CAL_PIERS];
        union cal_data_per_freq_ar9287_u
          calPierData2G[AR9287_MAX_CHAINS][AR9287_NUM_2G_CAL_PIERS];
        struct cal_target_power_leg
          calTargetPowerCck[AR9287_NUM_2G_CCK_TARGET_POWERS];
        struct cal_target_power_leg
          calTargetPower2G[AR9287_NUM_2G_20_TARGET_POWERS];
        struct cal_target_power_ht
          calTargetPower2GHT20[AR9287_NUM_2G_20_TARGET_POWERS];
        struct cal_target_power_ht
          calTargetPower2GHT40[AR9287_NUM_2G_40_TARGET_POWERS];
        uint8_t ctlIndex[AR9287_NUM_CTLS];
        struct cal_ctl_data_ar9287 ctlData[AR9287_NUM_CTLS];
        uint8_t padding;
};

#pragma pack()

typedef struct {
        uint16_t        rdEdge;
        uint16_t        twice_rdEdgePower;
        boolean_t        flag;
} RD_EDGES_POWER;

typedef struct {
        struct ar9287_eeprom ee_base;
#define NUM_EDGES        8
        uint16_t        ee_numCtls;
        RD_EDGES_POWER  ee_rdEdgesPower[NUM_EDGES*AR9287_NUM_CTLS];
        /* XXX these are dynamically calculated for use by shared code */
        int8_t          ee_antennaGainMax[2];
} HAL_EEPROM_9287;

typedef struct modal_eep_ar9287_header MODAL_EEP_9287_HEADER;
typedef struct base_eep_ar9287_header BASE_EEP_9287_HEADER;

#ifdef __cplusplus
}
#endif

#endif /* _ARN_EEPROM_H */

