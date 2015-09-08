
typedef union
#ifdef __cplusplus
	YYSTYPE
#endif
 {
	int ival;
	char *strval;
	cmd_t *cmd;
} YYSTYPE;
extern YYSTYPE yylval;
# define CANCEL 257
# define CLEAR 258
# define COMMIT 259
# define CREATE 260
# define DESTROY 261
# define END 262
# define EXIT 263
# define EXPORT 264
# define GET 265
# define HELP 266
# define LIST 267
# define REVERT 268
# define SELECT 269
# define SET 270
# define VERIFY 271
# define WALKPROP 272
# define LOC 273
# define NCP 274
# define NCU 275
# define ENM 276
# define WLAN 277
# define PHYS 278
# define IP 279
# define TOKEN 280
# define EQUAL 281
# define OPTION 282
# define UNKNOWN 283
# define ACTIVATION_MODE 284
# define CONDITIONS 285
# define ENABLED 286
# define TYPE 287
# define CLASS 288
# define PARENT 289
# define PRIORITY_GROUP 290
# define PRIORITY_MODE 291
# define LINK_MACADDR 292
# define LINK_AUTOPUSH 293
# define LINK_MTU 294
# define IP_VERSION 295
# define IPV4_ADDRSRC 296
# define IPV4_ADDR 297
# define IPV4_DEFAULT_ROUTE 298
# define IPV6_ADDRSRC 299
# define IPV6_ADDR 300
# define IPV6_DEFAULT_ROUTE 301
# define ENM_STATE 302
# define ENM_FMRI 303
# define ENM_START 304
# define ENM_STOP 305
# define LOC_NAMESERVICES 306
# define LOC_NAMESERVICES_CONFIG 307
# define LOC_DNS_CONFIGSRC 308
# define LOC_DNS_DOMAIN 309
# define LOC_DNS_SERVERS 310
# define LOC_DNS_SEARCH 311
# define LOC_NIS_CONFIGSRC 312
# define LOC_NIS_SERVERS 313
# define LOC_LDAP_CONFIGSRC 314
# define LOC_LDAP_SERVERS 315
# define LOC_DEFAULT_DOMAIN 316
# define LOC_NFSV4_DOMAIN 317
# define LOC_IPF_CONFIG 318
# define LOC_IPF_V6_CONFIG 319
# define LOC_IPNAT_CONFIG 320
# define LOC_IPPOOL_CONFIG 321
# define LOC_IKE_CONFIG 322
# define LOC_IPSECPOL_CONFIG 323
# define WLAN_SSID 324
# define WLAN_BSSID 325
# define WLAN_KEYNAME 326
# define WLAN_PRIORITY 327
# define WLAN_DISABLED 328
# define WLAN_EAP_USER 329
# define WLAN_EAP_ANON 330
# define WLAN_CA_CERT 331
# define WLAN_PRIV 332
# define WLAN_CLI_CERT 333
