/*
 * Please do not edit this file.
 * It was generated using rpcgen.
 */

#ifndef _BOOTPARAM_PROT_H_RPCGEN
#define	_BOOTPARAM_PROT_H_RPCGEN

#include <rpc/rpc.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */
/* from bootparam_prot.x */

#pragma ident	"%Z%%M%	%I%	%E% SMI"

#define	MAX_MACHINE_NAME 255
#define	MAX_PATH_LEN 1024
#define	MAX_FILEID 32
#define	IP_ADDR_TYPE 1

typedef char *bp_machine_name_t;

typedef char *bp_path_t;

typedef char *bp_fileid_t;

struct ip_addr_t {
	char net;
	char host;
	char lh;
	char impno;
};
typedef struct ip_addr_t ip_addr_t;

struct bp_address {
	int address_type;
	union {
		ip_addr_t ip_addr;
	} bp_address_u;
};
typedef struct bp_address bp_address;

struct bp_whoami_arg {
	bp_address client_address;
};
typedef struct bp_whoami_arg bp_whoami_arg;

struct bp_whoami_res {
	bp_machine_name_t client_name;
	bp_machine_name_t domain_name;
	bp_address router_address;
};
typedef struct bp_whoami_res bp_whoami_res;

struct bp_getfile_arg {
	bp_machine_name_t client_name;
	bp_fileid_t file_id;
};
typedef struct bp_getfile_arg bp_getfile_arg;

struct bp_getfile_res {
	bp_machine_name_t server_name;
	bp_address server_address;
	bp_path_t server_path;
};
typedef struct bp_getfile_res bp_getfile_res;

#define	BOOTPARAMPROG	100026
#define	BOOTPARAMVERS	1

#if defined(__STDC__) || defined(__cplusplus)
#define	BOOTPARAMPROC_WHOAMI	1
extern  bp_whoami_res * bootparamproc_whoami_1(bp_whoami_arg *, CLIENT *);
extern  bp_whoami_res * bootparamproc_whoami_1_svc(bp_whoami_arg *, struct svc_req *);
#define	BOOTPARAMPROC_GETFILE	2
extern  bp_getfile_res * bootparamproc_getfile_1(bp_getfile_arg *, CLIENT *);
extern  bp_getfile_res * bootparamproc_getfile_1_svc(bp_getfile_arg *, struct svc_req *);
extern int bootparamprog_1_freeresult(SVCXPRT *, xdrproc_t, caddr_t);

#else /* K&R C */
#define	BOOTPARAMPROC_WHOAMI	1
extern  bp_whoami_res * bootparamproc_whoami_1();
extern  bp_whoami_res * bootparamproc_whoami_1_svc();
#define	BOOTPARAMPROC_GETFILE	2
extern  bp_getfile_res * bootparamproc_getfile_1();
extern  bp_getfile_res * bootparamproc_getfile_1_svc();
extern int bootparamprog_1_freeresult();
#endif /* K&R C */

/* the xdr functions */

#if defined(__STDC__) || defined(__cplusplus)
extern  bool_t xdr_bp_machine_name_t(XDR *, bp_machine_name_t*);
extern  bool_t xdr_bp_path_t(XDR *, bp_path_t*);
extern  bool_t xdr_bp_fileid_t(XDR *, bp_fileid_t*);
extern  bool_t xdr_ip_addr_t(XDR *, ip_addr_t*);
extern  bool_t xdr_bp_address(XDR *, bp_address*);
extern  bool_t xdr_bp_whoami_arg(XDR *, bp_whoami_arg*);
extern  bool_t xdr_bp_whoami_res(XDR *, bp_whoami_res*);
extern  bool_t xdr_bp_getfile_arg(XDR *, bp_getfile_arg*);
extern  bool_t xdr_bp_getfile_res(XDR *, bp_getfile_res*);

#else /* K&R C */
extern bool_t xdr_bp_machine_name_t();
extern bool_t xdr_bp_path_t();
extern bool_t xdr_bp_fileid_t();
extern bool_t xdr_ip_addr_t();
extern bool_t xdr_bp_address();
extern bool_t xdr_bp_whoami_arg();
extern bool_t xdr_bp_whoami_res();
extern bool_t xdr_bp_getfile_arg();
extern bool_t xdr_bp_getfile_res();

#endif /* K&R C */

#ifdef __cplusplus
}
#endif

#endif /* !_BOOTPARAM_PROT_H_RPCGEN */
