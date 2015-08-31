/*
 * Please do not edit this file.
 * It was generated using rpcgen.
 */

#include "../../head/rpcsvc/bootparam_prot.h"

#ifndef _KERNEL
#include <stdlib.h>
#endif /* !_KERNEL */

/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */
/* from bootparam_prot.x */

bool_t
xdr_bp_machine_name_t(xdrs, objp)
	XDR *xdrs;
	bp_machine_name_t *objp;
{

	rpc_inline_t *buf;

	if (!xdr_string(xdrs, objp, MAX_MACHINE_NAME))
		return (FALSE);
	return (TRUE);
}

bool_t
xdr_bp_path_t(xdrs, objp)
	XDR *xdrs;
	bp_path_t *objp;
{

	rpc_inline_t *buf;

	if (!xdr_string(xdrs, objp, MAX_PATH_LEN))
		return (FALSE);
	return (TRUE);
}

bool_t
xdr_bp_fileid_t(xdrs, objp)
	XDR *xdrs;
	bp_fileid_t *objp;
{

	rpc_inline_t *buf;

	if (!xdr_string(xdrs, objp, MAX_FILEID))
		return (FALSE);
	return (TRUE);
}

bool_t
xdr_ip_addr_t(xdrs, objp)
	XDR *xdrs;
	ip_addr_t *objp;
{

	rpc_inline_t *buf;

	if (!xdr_char(xdrs, &objp->net))
		return (FALSE);
	if (!xdr_char(xdrs, &objp->host))
		return (FALSE);
	if (!xdr_char(xdrs, &objp->lh))
		return (FALSE);
	if (!xdr_char(xdrs, &objp->impno))
		return (FALSE);
	return (TRUE);
}

bool_t
xdr_bp_address(xdrs, objp)
	XDR *xdrs;
	bp_address *objp;
{

	rpc_inline_t *buf;

	if (!xdr_int(xdrs, &objp->address_type))
		return (FALSE);
	switch (objp->address_type) {
	case IP_ADDR_TYPE:
		if (!xdr_ip_addr_t(xdrs, &objp->bp_address_u.ip_addr))
			return (FALSE);
		break;
	default:
		return (FALSE);
	}
	return (TRUE);
}

bool_t
xdr_bp_whoami_arg(xdrs, objp)
	XDR *xdrs;
	bp_whoami_arg *objp;
{

	rpc_inline_t *buf;

	if (!xdr_bp_address(xdrs, &objp->client_address))
		return (FALSE);
	return (TRUE);
}

bool_t
xdr_bp_whoami_res(xdrs, objp)
	XDR *xdrs;
	bp_whoami_res *objp;
{

	rpc_inline_t *buf;

	if (!xdr_bp_machine_name_t(xdrs, &objp->client_name))
		return (FALSE);
	if (!xdr_bp_machine_name_t(xdrs, &objp->domain_name))
		return (FALSE);
	if (!xdr_bp_address(xdrs, &objp->router_address))
		return (FALSE);
	return (TRUE);
}

bool_t
xdr_bp_getfile_arg(xdrs, objp)
	XDR *xdrs;
	bp_getfile_arg *objp;
{

	rpc_inline_t *buf;

	if (!xdr_bp_machine_name_t(xdrs, &objp->client_name))
		return (FALSE);
	if (!xdr_bp_fileid_t(xdrs, &objp->file_id))
		return (FALSE);
	return (TRUE);
}

bool_t
xdr_bp_getfile_res(xdrs, objp)
	XDR *xdrs;
	bp_getfile_res *objp;
{

	rpc_inline_t *buf;

	if (!xdr_bp_machine_name_t(xdrs, &objp->server_name))
		return (FALSE);
	if (!xdr_bp_address(xdrs, &objp->server_address))
		return (FALSE);
	if (!xdr_bp_path_t(xdrs, &objp->server_path))
		return (FALSE);
	return (TRUE);
}
