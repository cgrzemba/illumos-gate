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
 * Copyright (c) 2010, Oracle and/or its affiliates. All rights reserved.
 * Copyright (c) 2012, Enrico Papi <enricop@computer.org>. All rights reserved.
 */

/*
 * nwamadm is a command interpreter to administer NWAM profiles.  It
 * is all in C (i.e., no lex/yacc), and all the argument passing is
 * argc/argv based.  main() calls the command's handler function,
 * which first calls parse_argv() to parse the input arguments and set
 * approriate variables for each command.  The rest of the program is
 * helper functions for the handler functions.
 */

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <libdlwlan.h>
#include <libinetutil.h>
#include <libnwam.h>
#include <libscf.h>
#include <locale.h>
#include <netinet/in.h>
#include <ofmt.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#if !defined(TEXT_DOMAIN)		/* should be defined by cc -D */
#define	TEXT_DOMAIN	"SYS_TEST"	/* Use this only if it wasn't */
#endif

typedef void (cmd_func_t)(int, char **);

struct cmd {
	uint_t		cmd_num;		/* command number */
	const char	*cmd_name;		/* command name */
	cmd_func_t	*cmd_handler;		/* function to call */
	const char	*cmd_usage;		/* short form help */
	const char	*cmd_desc;		/* command description */
	boolean_t	cmd_needs_nwamd;	/* nwam needs to run */
};

/* constants for commands */
#define	CMD_HELP	0
#define	CMD_ENABLE	1
#define	CMD_DISABLE	2
#define	CMD_LIST	3
#define	CMD_SHOW_EVENTS	4
#define	CMD_SELECT_WIFI	5

#define	CMD_MIN		CMD_HELP
#define	CMD_MAX		CMD_SELECT_WIFI

/* functions to call */
static cmd_func_t help_func, enable_func, disable_func, list_func;
static cmd_func_t show_events_func, select_wifi_func;
static ofmt_cb_t print_list_cb;

/* table of commands and usage */
static struct cmd cmdtab[] = {
	{ CMD_HELP,		"help",		help_func,
	    "help",
	    "Print this usage message.",		B_FALSE		},
	{ CMD_ENABLE,		"enable",	enable_func,
	    "enable [-p <profile-type>] [-c <ncu-class>] <object-name>",
	    "Enable the specified profile.",		B_FALSE		},
	{ CMD_DISABLE,		"disable",	disable_func,
	    "disable [-p <profile-type>] [-c <ncu-class>] <object-name>",
	    "Disable the specified profile.",		B_FALSE		},
	{ CMD_LIST,		"list",		list_func,
	    "list [-x] [-p <profile-type>] [-c <ncu-class>] [<object-name>]",
	    "List profiles and their current states.",	B_TRUE		},
	{ CMD_SHOW_EVENTS,	"show-events",	show_events_func,
	    "show-events",
	    "Display all events.",			B_TRUE		},
	{ CMD_SELECT_WIFI,	"select-wifi",	select_wifi_func,
	    "select-wifi <link-name>",
	    "Make a WLAN selection from WiFi scan results", B_TRUE	}
};

/* Structure for "nwamadm list" output */

typedef struct profile_entry {
	nwam_object_type_t	p_type;
	nwam_ncu_class_t	p_ncu_class;
	char			p_name[NWAM_MAX_NAME_LEN];
	nwam_state_t		p_state;
	nwam_aux_state_t	p_aux_state;
} profile_entry_t;

/* widths of colums for printing */
#define	TYPE_WIDTH		12	/* width of TYPE column */
#define	PROFILE_WIDTH		15	/* width of PROFILE column */
#define	STATE_WIDTH		15	/* width of STATE column */
#define	AUXSTATE_WIDTH		36	/* width of AUXILIARY STATE column */

#define	EVENT_WIDTH		15	/* width of EVENT column */
#define	DESCRIPTION_WIDTH	64	/* width of DESCRIPTION column */

/* id for columns of "nwamadm list" */
typedef enum {
	LIST_TYPE,
	LIST_PROFILE,
	LIST_STATE,
	LIST_AUXSTATE
} list_field_id_t;

static const ofmt_field_t list_fields[] = {
	/* header,		width,		id,		callback */
	{ "TYPE",		TYPE_WIDTH,	LIST_TYPE,	print_list_cb },
	{ "PROFILE",		PROFILE_WIDTH,	LIST_PROFILE,	print_list_cb },
	{ "STATE",		STATE_WIDTH,	LIST_STATE,	print_list_cb },
	{ "AUXILIARY STATE",	AUXSTATE_WIDTH,	LIST_AUXSTATE,	print_list_cb },
	{ NULL,			0,		0,		NULL }
};

/* Global variables */

/* set early in main(), never modified thereafter, used all over the place */
static char *execname;

/* whether the auxilary states are to be printed or not */
static boolean_t extended_list = B_FALSE;

/* Functions */

static const char *
cmd_to_str(int cmd_num)
{
	assert(cmd_num >= CMD_MIN && cmd_num <= CMD_MAX);
	return (cmdtab[cmd_num].cmd_name);
}

/* returns description of given command */
static const char *
long_help(int cmd_num)
{
	assert(cmd_num >= CMD_MIN && cmd_num <= CMD_MAX);
	return (gettext(cmdtab[cmd_num].cmd_desc));
}

/*
 * Called with explicit B_TRUE when help is explicitly required,
 * B_FALSE for errors
 */
static void
usage(boolean_t explicit)
{
	int	i;
	FILE	*fd = explicit ? stdout : stderr;

	(void) fprintf(fd, gettext("usage: <subcommand> <args> ...\n"));
	for (i = CMD_MIN; i <= CMD_MAX; i++) {
		(void) fprintf(fd, "\t%s\n", cmdtab[i].cmd_usage);
		if (explicit)
			(void) fprintf(fd, "\t\t%s\n\n", long_help(i));
	}
}

/* PRINTFLIKE1 */
static void
die(const char *format, ...)
{
	va_list alist;

	format = gettext(format);
	(void) fprintf(stderr, "%s: ", execname);

	va_start(alist, format);
	(void) vfprintf(stderr, format, alist);
	va_end(alist);
	(void) fprintf(stderr, "\n");

	exit(EXIT_FAILURE);
}

/* PRINTFLIKE2 */
static void
die_nwamerr(nwam_error_t err, const char *format, ...)
{
	va_list alist;

	format = gettext(format);
	(void) fprintf(stderr, "%s: ", execname);

	va_start(alist, format);
	(void) vfprintf(stderr, format, alist);
	va_end(alist);
	(void) fprintf(stderr, ": %s\n", nwam_strerror(err));

	exit(EXIT_FAILURE);
}

/* prints the usage for cmd_num and exits */
static void
die_usage(int cmd_num)
{
	assert(cmd_num >= CMD_MIN && cmd_num <= CMD_MAX);

	(void) fprintf(stderr, "%s: %s\n", gettext("usage"),
	    cmdtab[cmd_num].cmd_usage);
	(void) fprintf(stderr, "\t%s\n", long_help(cmd_num));

	exit(EXIT_FAILURE);
}

/*
 * Prints the usage and description of all commands
 */
/* ARGSUSED */
static void
help_func(int argc, char *argv[])
{
	usage(B_TRUE);
}

/* determines if the NCP is active or not.  If so, sets arg and halts walk. */
static int
active_ncp_callback(nwam_ncp_handle_t ncph, void *arg)
{
	char			**namep = arg;
	nwam_state_t		state = NWAM_STATE_UNINITIALIZED;
	nwam_aux_state_t	aux;

	(void) nwam_ncp_get_state(ncph, &state, &aux);
	if (state == NWAM_STATE_ONLINE) {
		if (nwam_ncp_get_name(ncph, namep) != NWAM_SUCCESS)
			*namep = NULL;
		return (1);
	}

	return (0);
}

/* find the currently active NCP and returns its handle */
static nwam_ncp_handle_t
determine_active_ncp()
{
	char *active_ncp;
	nwam_ncp_handle_t ncph;
	nwam_error_t ret;

	if (nwam_walk_ncps(active_ncp_callback, &active_ncp, 0, NULL)
	    == NWAM_WALK_HALTED) {
		if (active_ncp == NULL)
			return (NULL);

		/* retrieve the NCP handle */
		ret = nwam_ncp_read(active_ncp, 0, &ncph);
		free(active_ncp);
		if (ret == NWAM_SUCCESS)
			return (ncph);
	}

	return (NULL);
}

/* check if the given name is a valid loc, test by reading the given loc */
static boolean_t
valid_loc(const char *name)
{
	nwam_loc_handle_t loch;

	if (nwam_loc_read(name, 0, &loch) != NWAM_SUCCESS)
		return (B_FALSE);
	nwam_loc_free(loch);
	return (B_TRUE);
}

static boolean_t
valid_enm(const char *name)
{
	nwam_enm_handle_t enmh;

	if (nwam_enm_read(name, 0, &enmh) != NWAM_SUCCESS)
		return (B_FALSE);
	nwam_enm_free(enmh);
	return (B_TRUE);
}

static boolean_t
valid_ncp(const char *name)
{
	nwam_ncp_handle_t ncph;

	if (nwam_ncp_read(name, 0, &ncph) != NWAM_SUCCESS)
		return (B_FALSE);
	nwam_ncp_free(ncph);
	return (B_TRUE);
}

static boolean_t
valid_ncu(const char *name)
{
	nwam_ncp_handle_t ncph;
	nwam_ncu_handle_t ncuh;
	nwam_error_t	ret;

	if ((ncph = determine_active_ncp()) == NULL)
		return (B_FALSE);

	ret = nwam_ncu_read(ncph, name, NWAM_NCU_TYPE_ANY, 0, &ncuh);
	nwam_ncp_free(ncph);
	if (ret != NWAM_SUCCESS && ret != NWAM_ENTITY_MULTIPLE_VALUES)
		return (B_FALSE);
	nwam_ncu_free(ncuh);
	return (B_TRUE);
}

/*
 * Given a name, returns object type (loc, enm, ncp, or ncu) and how many
 * objects matched that name.
 */
static nwam_object_type_t
determine_object_type(const char *name, int *num)
{
	nwam_object_type_t type;
	int n = 0;

	/* see if a valid loc, enm, ncp and/or ncu exists with given name */
	if (valid_loc(name)) {
		n++;
		type = NWAM_OBJECT_TYPE_LOC;
	}
	if (valid_enm(name)) {
		n++;
		type = NWAM_OBJECT_TYPE_ENM;
	}
	if (valid_ncp(name)) {
		n++;
		type = NWAM_OBJECT_TYPE_NCP;
	}
	if (valid_ncu(name)) {
		n++;
		type = NWAM_OBJECT_TYPE_NCU;
	}

	/* if n > 1, then it means *type was set multiple times, undo it */
	if (n != 1)
		type = NWAM_OBJECT_TYPE_UNKNOWN;

	*num = n;
	return (type);
}

/*
 * Parses argv array and populates object_type and name.
 * Program exits on failure.
 */
static void
parse_argv(int argc, char *argv[], int cmd_num, nwam_object_type_t *object_type,
    nwam_ncu_type_t *ncu_type, nwam_ncu_class_t *ncu_class, const char **name)
{
	int			arg;
	nwam_object_type_t	type = NWAM_OBJECT_TYPE_UNKNOWN;
	uint64_t		ncu = NWAM_NCU_TYPE_ANY;
	uint64_t		class = NWAM_NCU_CLASS_ANY;

	/* check argv for option */
	optind = 0;
	while ((arg = getopt(argc, argv, "?p:c:x")) != EOF) {
		switch (arg) {
		case 'p':
			type = nwam_string_to_object_type(optarg);
			if (type == NWAM_OBJECT_TYPE_UNKNOWN)
				die("Invalid profile-type: %s", optarg);
			break;
		case 'c':
			if (nwam_value_string_get_uint64(NWAM_NCU_PROP_CLASS,
			    optarg, &class) != NWAM_SUCCESS) {
				die("Invalid ncu-class: %s", optarg);
			}
			ncu = nwam_ncu_class_to_type(class);
			if (ncu == NWAM_NCU_TYPE_ANY ||
			    ncu == NWAM_NCU_TYPE_UNKNOWN)
				die("Invalid ncu-class: %s", optarg);
			break;
		case 'x':
			/* -x is only for list */
			if (cmd_num != CMD_LIST)
				die("-x can only be used with 'list'");
			extended_list = B_TRUE;
			break;
		case '?':
		default:
			die_usage(cmd_num);
		}
	}

	if (ncu != NWAM_NCU_TYPE_ANY) {
		/* If -c is given, -p must be NCU. If unspecified, assume NCU */
		if (type != NWAM_OBJECT_TYPE_UNKNOWN &&
		    type != NWAM_OBJECT_TYPE_NCU)
			die("'-c <ncu-class>' can only be used for ncu");

		type = NWAM_OBJECT_TYPE_NCU;
	}

	/* name is mandatory for enable and disable, but not for list */
	if (optind == (argc-1))
		*name = argv[optind];
	else if (argc != optind)
		die("too many profile names given");
	else if (cmd_num != CMD_LIST)
		die("no profile name given");

	/*
	 * No need to determine type for list.
	 * If -p is not given for enable or disable, then determine type.
	 */
	if (cmd_num != CMD_LIST && type == NWAM_OBJECT_TYPE_UNKNOWN) {
		int num = 0;

		type = determine_object_type(*name, &num);
		if (num == 0) {
			die("no profile matched '%s'", *name);
		} else if (num > 1) {
			die("more than one profile matched '%s' - use "
			    "'-p <profile-type>' to specify a profile type.",
			    *name);
		}
	}

	*object_type = type;
	*ncu_type = ncu;
	*ncu_class = class;
}

/* Enables/Disables profiles depending on boolean */
static nwam_error_t
loc_action(const char *name, boolean_t enable, char **realnamep)
{
	nwam_loc_handle_t loch;
	nwam_error_t ret;

	if ((ret = nwam_loc_read(name, 0, &loch)) != NWAM_SUCCESS)
		return (ret);

	if (enable)
		ret = nwam_loc_enable(loch);
	else
		ret = nwam_loc_disable(loch);

	(void) nwam_loc_get_name(loch, realnamep);
	nwam_loc_free(loch);
	return (ret);
}

static nwam_error_t
enm_action(const char *name, boolean_t enable, char **realnamep)
{
	nwam_enm_handle_t enmh;
	nwam_error_t ret;

	if ((ret = nwam_enm_read(name, 0, &enmh)) != NWAM_SUCCESS)
		return (ret);

	if (enable)
		ret = nwam_enm_enable(enmh);
	else
		ret = nwam_enm_disable(enmh);

	(void) nwam_enm_get_name(enmh, realnamep);
	nwam_enm_free(enmh);
	return (ret);
}

static nwam_error_t
ncu_action(const char *name, nwam_ncp_handle_t ncph, nwam_ncu_type_t type,
    boolean_t enable, char **realnamep)
{
	nwam_ncu_handle_t ncuh;
	nwam_error_t ret;
	boolean_t retrieved_ncph = B_FALSE;

	if (ncph == NULL) {
		if ((ncph = determine_active_ncp()) == NULL)
			return (NWAM_ENTITY_NOT_FOUND);
		retrieved_ncph = B_TRUE;
	}

	ret = nwam_ncu_read(ncph, name, type, 0, &ncuh);
	switch (ret) {
	case NWAM_SUCCESS:
		if (enable)
			ret = nwam_ncu_enable(ncuh);
		else
			ret = nwam_ncu_disable(ncuh);
		(void) nwam_ncu_get_name(ncuh, realnamep);
		nwam_ncu_free(ncuh);
		break;
	case NWAM_ENTITY_MULTIPLE_VALUES:
		/* Call ncu_action() for link and interface types */
		ret = ncu_action(name, ncph, NWAM_NCU_TYPE_LINK, enable,
		    realnamep);
		if (ret != NWAM_SUCCESS)
			break;

		ret = ncu_action(name, ncph, NWAM_NCU_TYPE_INTERFACE, enable,
		    realnamep);
		break;
	}
	if (retrieved_ncph)
		nwam_ncp_free(ncph);

	return (ret);
}

/*
 * If more than one type of profile with the same name, return error.
 * In such situations, the -p option must be used.
 * If a location is enabled when a different one is already enabled, then
 * that location is disabled automatically by nwamd.
 */
static void
enable_func(int argc, char *argv[])
{
	nwam_error_t		ret;
	nwam_object_type_t	type = NWAM_OBJECT_TYPE_UNKNOWN;
	nwam_ncu_type_t		ncu_type = NWAM_NCU_TYPE_ANY;
	nwam_ncu_class_t	ncu_class = NWAM_NCU_CLASS_ANY;
	const char		*name;
	char			*realname = NULL;

	/* parse_argv() returns only on success */
	parse_argv(argc, argv, CMD_ENABLE, &type, &ncu_type, &ncu_class, &name);

	/*
	 * NCPs and Locations don't need to disable the currently active
	 * profile - nwamd automatically switches to the new active profile.
	 * and will disable it if necessary.
	 */

	/* activate given profile */
	switch (type) {
	case NWAM_OBJECT_TYPE_LOC:
		ret = loc_action(name, B_TRUE, &realname);
		break;
	case NWAM_OBJECT_TYPE_ENM:
		ret = enm_action(name, B_TRUE, &realname);
		break;
	case NWAM_OBJECT_TYPE_NCP:
	{
		nwam_ncp_handle_t ncph;

		if ((ret = nwam_ncp_read(name, 0, &ncph)) != NWAM_SUCCESS)
			break;

		ret = nwam_ncp_enable(ncph);
		(void) nwam_ncp_get_name(ncph, &realname);
		nwam_ncp_free(ncph);
		break;
	}
	case NWAM_OBJECT_TYPE_NCU:
		ret = ncu_action(name, NULL, ncu_type, B_TRUE, &realname);
		break;
	}

	switch (ret) {
	case NWAM_SUCCESS:
		(void) printf(gettext("Enabling %s '%s'\n"),
		    nwam_object_type_to_string(type),
		    realname != NULL ? realname : name);
		break;
	case NWAM_ENTITY_NOT_MANUAL:
		die("Only profiles with manual activation-mode can be enabled");
		break;
	default:
		die_nwamerr(ret, "Could not enable %s '%s'",
		    nwam_object_type_to_string(type),
		    realname != NULL ? realname : name);
	}
	free(realname);
}

/*
 * Disables a given profile.  Similar to enable, the -p option must be used
 * if more than one type of profile is matched by the given name.
 */
static void
disable_func(int argc, char *argv[])
{
	nwam_error_t		ret;
	nwam_object_type_t	type = NWAM_OBJECT_TYPE_UNKNOWN;
	nwam_ncu_type_t		ncu_type = NWAM_NCU_TYPE_ANY;
	nwam_ncu_class_t	ncu_class = NWAM_NCU_CLASS_ANY;
	const char		*name;
	char			*realname = NULL;

	/* parse_argv() returns only on success */
	parse_argv(argc, argv, CMD_DISABLE, &type, &ncu_type, &ncu_class,
	    &name);

	/* deactivate the given profile */
	switch (type) {
	case NWAM_OBJECT_TYPE_LOC:
		ret = loc_action(name, B_FALSE, &realname);
		break;
	case NWAM_OBJECT_TYPE_ENM:
		ret = enm_action(name, B_FALSE, &realname);
		break;
	case NWAM_OBJECT_TYPE_NCU:
		ret = ncu_action(name, NULL, ncu_type, B_FALSE, &realname);
		break;
	case NWAM_OBJECT_TYPE_NCP:
		die("ncp's cannot be disabled.  Enable a different ncp to "
		    "switch to that ncp");
	}

	switch (ret) {
	case NWAM_SUCCESS:
		(void) printf(gettext("Disabling %s '%s'\n"),
		    nwam_object_type_to_string(type),
		    realname != NULL ? realname : name);
		break;
	case NWAM_ENTITY_NOT_MANUAL:
		die("Only profiles with manual activation-mode can be "
		    "disabled");
		break;
	default:
		die_nwamerr(ret, "Could not disable %s '%s'",
		    nwam_object_type_to_string(type),
		    realname != NULL ? realname : name);
	}
	free(realname);
}

/* prints each column */
static boolean_t
print_list_cb(ofmt_arg_t *ofarg, char *buf, uint_t bufsize)
{
	profile_entry_t *pent = ofarg->ofmt_cbarg;

	switch (ofarg->ofmt_id) {
	case LIST_TYPE:
		/* ncu:ip or ncu:phys for NCUs; ncp, loc, enm for others */
		if (pent->p_type == NWAM_OBJECT_TYPE_NCU) {
			const char *class;
			if (nwam_uint64_get_value_string(NWAM_NCU_PROP_CLASS,
			    pent->p_ncu_class, &class) != NWAM_SUCCESS)
				class = ""; /* empty */
			(void) snprintf(buf, bufsize, "%s:%s",
			    nwam_object_type_to_string(pent->p_type), class);
		} else {
			(void) strlcpy(buf,
			    nwam_object_type_to_string(pent->p_type), bufsize);
		}
		break;
	case LIST_PROFILE:
		(void) strlcpy(buf, pent->p_name, bufsize);
		break;
	case LIST_STATE:
		(void) strlcpy(buf, nwam_state_to_string(pent->p_state),
		    bufsize);
		break;
	case LIST_AUXSTATE:
		(void) strlcpy(buf,
		    nwam_aux_state_to_string(pent->p_aux_state), bufsize);
		break;
	default:
		die("invalid print_list_cb() input: %d", ofarg->ofmt_id);
		break;
	}
	return (B_TRUE);
}

/* returns the state and auxilliary state of the object */
static nwam_state_t
determine_object_state(nwam_object_type_t type, void *handle,
    nwam_aux_state_t *aux_statep)
{
	nwam_state_t state;
	nwam_aux_state_t astate;
	nwam_error_t ret;

	switch (type) {
	case NWAM_OBJECT_TYPE_ENM:
		ret = nwam_enm_get_state(handle, &state, &astate);
		break;
	case NWAM_OBJECT_TYPE_LOC:
		ret = nwam_loc_get_state(handle, &state, &astate);
		break;
	case NWAM_OBJECT_TYPE_NCP:
		ret = nwam_ncp_get_state(handle, &state, &astate);
		break;
	case NWAM_OBJECT_TYPE_NCU:
		ret = nwam_ncu_get_state(handle, &state, &astate);
		break;
	default:
		/* NOTREACHED */
		break;
	}

	if (ret == NWAM_PERMISSION_DENIED) {
		die_nwamerr(ret, "could not get object state");
	} else if (ret != NWAM_SUCCESS) {
		state = NWAM_STATE_UNINITIALIZED;
		astate = NWAM_AUX_STATE_UNINITIALIZED;
	}

	if (aux_statep != NULL)
		*aux_statep = astate;
	return (state);
}

/* populate profile_entry_t with values for object with given handle */
static int
add_to_profile_entry(nwam_object_type_t type, void *handle,
    profile_entry_t *pent)
{
	char		*name;
	nwam_error_t	ret;

	pent->p_type = type;
	if (type == NWAM_OBJECT_TYPE_NCU) {
		nwam_ncu_class_t class;
		if ((ret = nwam_ncu_get_ncu_class(handle, &class))
		    != NWAM_SUCCESS)
			return (ret);
		pent->p_ncu_class = class;
	} else {
		pent->p_ncu_class = -1;
	}

	switch (type) {
	case NWAM_OBJECT_TYPE_ENM:
		ret = nwam_enm_get_name(handle, &name);
		break;
	case NWAM_OBJECT_TYPE_LOC:
		ret = nwam_loc_get_name(handle, &name);
		break;
	case NWAM_OBJECT_TYPE_NCP:
		ret = nwam_ncp_get_name(handle, &name);
		break;
	case NWAM_OBJECT_TYPE_NCU:
		ret = nwam_ncu_get_name(handle, &name);
		break;
	default:
		/* NOTREACHED */
		break;
	}
	if (ret != NWAM_SUCCESS) {
		return (ret);
	}
	(void) strlcpy(pent->p_name, name, sizeof (pent->p_name));
	free(name);

	pent->p_state = determine_object_state(type, handle,
	    &pent->p_aux_state);

	return (NWAM_SUCCESS);
}

/* callback functions used by walk */

static int
list_ncu_cb(nwam_ncu_handle_t ncuh, void *arg)
{
	ofmt_handle_t	ofmt = arg;
	profile_entry_t pent;
	nwam_error_t	ret;

	bzero(&pent, sizeof (profile_entry_t));
	ret = add_to_profile_entry(NWAM_OBJECT_TYPE_NCU, ncuh, &pent);
	if (ret != NWAM_SUCCESS)
		die_nwamerr(ret, "could not add ncu to list");
	ofmt_print(ofmt, &pent);
	return (0);
}

static int
list_ncp_cb(nwam_ncp_handle_t ncph, void *arg)
{
	ofmt_handle_t	ofmt = arg;
	profile_entry_t pent;
	nwam_error_t	ret;
	nwam_state_t	state;

	bzero(&pent, sizeof (profile_entry_t));
	ret = add_to_profile_entry(NWAM_OBJECT_TYPE_NCP, ncph, &pent);
	if (ret != NWAM_SUCCESS)
		die_nwamerr(ret, "could not add ncp to list");
	ofmt_print(ofmt, &pent);

	state = determine_object_state(NWAM_OBJECT_TYPE_NCP, ncph, NULL);
	if (state == NWAM_STATE_ONLINE) {
		(void) nwam_ncp_walk_ncus(ncph, list_ncu_cb, ofmt,
		    NWAM_FLAG_NCU_TYPE_ALL, NULL);
	}
	return (0);
}

static int
list_loc_cb(nwam_loc_handle_t loch, void *arg)
{
	ofmt_handle_t	ofmt = arg;
	profile_entry_t pent;
	nwam_error_t	ret;

	bzero(&pent, sizeof (profile_entry_t));
	ret = add_to_profile_entry(NWAM_OBJECT_TYPE_LOC, loch, &pent);
	if (ret != NWAM_SUCCESS)
		die_nwamerr(ret, "could not add loc to list");
	ofmt_print(ofmt, &pent);
	return (0);
}

static int
list_enm_cb(nwam_enm_handle_t enmh, void *arg)
{
	ofmt_handle_t	ofmt = arg;
	profile_entry_t pent;
	nwam_error_t	ret;

	bzero(&pent, sizeof (profile_entry_t));
	ret = add_to_profile_entry(NWAM_OBJECT_TYPE_ENM, enmh, &pent);
	if (ret != NWAM_SUCCESS)
		die_nwamerr(ret, "could not add enm to list");
	ofmt_print(ofmt, &pent);
	return (0);
}

/*
 * lists all profiles and their state
 */
static void
list_func(int argc, char *argv[])
{
	nwam_error_t		ret = NWAM_SUCCESS;
	nwam_object_type_t	type = NWAM_OBJECT_TYPE_UNKNOWN;
	nwam_ncu_type_t		ncu_type = NWAM_NCU_TYPE_ANY;
	nwam_ncu_class_t	ncu_class = NWAM_NCU_CLASS_ANY;
	char			*name = NULL;

	ofmt_handle_t	ofmt;
	ofmt_status_t	oferr;
	char		*default_fields = "type,profile,state";
	char		*extended_fields = "type,profile,state,auxiliary state";
	char		*fields = NULL;

	/* parse_argv() returns only on success */
	parse_argv(argc, argv, CMD_LIST, &type, &ncu_type, &ncu_class,
	    (const char **)&name);

	if (extended_list)
		fields = extended_fields;
	else
		fields = default_fields;
	oferr = ofmt_open(fields, list_fields, 0, 0, &ofmt);
	if (oferr != OFMT_SUCCESS) {
		char buf[OFMT_BUFSIZE];
		(void) ofmt_strerror(ofmt, oferr, buf, sizeof (buf));
		die("ofmt_open() failed: %s", buf);
	}

	/* object-name given in command-line */
	if (name != NULL) {
		boolean_t found = B_FALSE;

		/*
		 * If objects with different types have the same name
		 * (type = UNKNOWN), then try to open handle for each object
		 * and print if successful.
		 */
		if (type == NWAM_OBJECT_TYPE_NCP ||
		    type == NWAM_OBJECT_TYPE_UNKNOWN) {
			nwam_ncp_handle_t ncph;
			if (nwam_ncp_read(name, 0, &ncph) == NWAM_SUCCESS) {
				found = B_TRUE;
				(void) list_ncp_cb(ncph, ofmt);
				nwam_ncp_free(ncph);
			}
		}
		if (type == NWAM_OBJECT_TYPE_NCU ||
		    type == NWAM_OBJECT_TYPE_UNKNOWN) {
			nwam_ncp_handle_t ncph;
			nwam_ncu_handle_t ncuh;

			if ((ncph = determine_active_ncp()) != NULL) {
				ret = nwam_ncu_read(ncph, name, ncu_type, 0,
				    &ncuh);
				if (ret == NWAM_ENTITY_MULTIPLE_VALUES) {
					found = B_TRUE;
					if (nwam_ncu_read(ncph, name,
					    NWAM_NCU_TYPE_LINK, 0, &ncuh)
					    == NWAM_SUCCESS) {
						(void) list_ncu_cb(ncuh, ofmt);
						nwam_ncu_free(ncuh);
					}
					if (nwam_ncu_read(ncph, name,
					    NWAM_NCU_TYPE_INTERFACE, 0, &ncuh)
					    == NWAM_SUCCESS) {
						(void) list_ncu_cb(ncuh, ofmt);
						nwam_ncu_free(ncuh);
					}
				} else if (ret == NWAM_SUCCESS) {
					found = B_TRUE;
					(void) list_ncu_cb(ncuh, ofmt);
					nwam_ncu_free(ncuh);
				}
				nwam_ncp_free(ncph);
			}
		}
		if (type == NWAM_OBJECT_TYPE_LOC ||
		    type == NWAM_OBJECT_TYPE_UNKNOWN) {
			nwam_loc_handle_t loch;
			if (nwam_loc_read(name, 0, &loch) == NWAM_SUCCESS) {
				found = B_TRUE;
				(void) list_loc_cb(loch, ofmt);
				nwam_loc_free(loch);
			}
		}
		if (type == NWAM_OBJECT_TYPE_ENM ||
		    type == NWAM_OBJECT_TYPE_UNKNOWN) {
			nwam_enm_handle_t enmh;
			if (nwam_enm_read(name, 0, &enmh) == NWAM_SUCCESS) {
				found = B_TRUE;
				(void) list_enm_cb(enmh, ofmt);
				nwam_enm_free(enmh);
			}
		}
		/* If at least object is found, don't return error */
		if (found)
			ret = NWAM_SUCCESS;
		else
			ret = NWAM_ENTITY_NOT_FOUND;
	}

	/* object-name not given in command-line */
	if (name == NULL) {
		/*
		 * If type given (type != UNKNOWN), just walk objects in that
		 * type.  Otherwise, walk all ncp, ncu, loc and enm.
		 */
		if (type == NWAM_OBJECT_TYPE_NCP ||
		    type == NWAM_OBJECT_TYPE_UNKNOWN) {
			ret = nwam_walk_ncps(list_ncp_cb, ofmt, 0, NULL);
			if (ret != NWAM_SUCCESS)
				goto done;
		}
		/* no UNKNOWN for NCUs.  They walked with active NCP above */
		if (type == NWAM_OBJECT_TYPE_NCU) {
			nwam_ncp_handle_t ncph;
			if ((ncph = determine_active_ncp()) != NULL) {
				ret = nwam_ncp_walk_ncus(ncph, list_ncu_cb,
				    ofmt, nwam_ncu_class_to_flag(ncu_class),
				    NULL);
				nwam_ncp_free(ncph);
				if (ret != NWAM_SUCCESS)
					goto done;
			}
		}
		if (type == NWAM_OBJECT_TYPE_LOC ||
		    type == NWAM_OBJECT_TYPE_UNKNOWN) {
			ret = nwam_walk_locs(list_loc_cb, ofmt,
			    NWAM_FLAG_ACTIVATION_MODE_ALL, NULL);
			if (ret != NWAM_SUCCESS)
				goto done;
		}
		if (type == NWAM_OBJECT_TYPE_ENM ||
		    type == NWAM_OBJECT_TYPE_UNKNOWN) {
			ret = nwam_walk_enms(list_enm_cb, ofmt,
			    NWAM_FLAG_ACTIVATION_MODE_ALL, NULL);
			if (ret != NWAM_SUCCESS)
				goto done;
		}
	}

done:
	ofmt_close(ofmt);
	if (ret == NWAM_ENTITY_NOT_FOUND && name != NULL)
		die("no profile matched '%s'", name);
	else if (ret != NWAM_SUCCESS)
		die_nwamerr(ret, "list failed during walk");
}

/*
 * Print NWAM events.
 */
static void
eventhandler(nwam_event_t event)
{
	char description[DESCRIPTION_WIDTH];
	char statestr[DESCRIPTION_WIDTH];
	char objstr[DESCRIPTION_WIDTH];
	char *object = NULL;
	const char *action = NULL;
	char *state = NULL;
	boolean_t display = B_TRUE;

	(void) strlcpy(description, "-", sizeof (description));

	switch (event->nwe_type) {
	case NWAM_EVENT_TYPE_OBJECT_ACTION:
		action = nwam_action_to_string
		    (event->nwe_data.nwe_object_action.nwe_action);
		(void) snprintf(objstr, sizeof (objstr), "%s %s",
		    nwam_object_type_to_string
		    (event->nwe_data.nwe_object_action.nwe_object_type),
		    event->nwe_data.nwe_object_action.nwe_name);
		object = objstr;
		break;

	case NWAM_EVENT_TYPE_OBJECT_STATE:
		(void) snprintf(statestr, sizeof (statestr), "%s, %s",
		    nwam_state_to_string
		    (event->nwe_data.nwe_object_state.nwe_state),
		    nwam_aux_state_to_string
		    (event->nwe_data.nwe_object_state.nwe_aux_state));
		state = statestr;

		(void) snprintf(objstr, sizeof (objstr), "%s %s",
		    nwam_object_type_to_string
		    (event->nwe_data.nwe_object_state.nwe_object_type),
		    event->nwe_data.nwe_object_state.nwe_name);
		object = objstr;
		break;

	case NWAM_EVENT_TYPE_PRIORITY_GROUP:
		(void) snprintf(description, DESCRIPTION_WIDTH,
		    "priority-group: %d",
		    event->nwe_data.nwe_priority_group_info.nwe_priority);
		break;

	case NWAM_EVENT_TYPE_WLAN_SCAN_REPORT:
		(void) printf("%-*s \n", EVENT_WIDTH,
		    nwam_event_type_to_string(event->nwe_type));
		(void) printf("Found %u  Scan Results for link %s",
		    event->nwe_data.nwe_wlan_info.nwe_scanres_num,
		    event->nwe_data.nwe_wlan_info.nwe_name);
		display = B_FALSE;
		break;

	case NWAM_EVENT_TYPE_WLAN_ASSOCIATION_REPORT:
		(void) printf("%-*s \n", EVENT_WIDTH,
		    nwam_event_type_to_string(event->nwe_type));
		display = B_FALSE;
		break;

	case NWAM_EVENT_TYPE_WLAN_WRONG_KEY:
		(void) printf("%-*s \n", EVENT_WIDTH,
		    nwam_event_type_to_string(event->nwe_type));
		display = B_FALSE;
		break;

	case NWAM_EVENT_TYPE_WLAN_CONNECTION_REPORT:
		(void) snprintf(description, DESCRIPTION_WIDTH,
		    gettext("connect to WLAN ESSID %s, BSSID %s %s"),
		    event->nwe_data.nwe_wlan_info.nwe_wlan.nww_essid,
		    event->nwe_data.nwe_wlan_info.nwe_wlan.nww_bssid,
		    "succeeded");
		break;

	case NWAM_EVENT_TYPE_WLAN_DISASSOCIATION_REPORT:
		(void) snprintf(description, DESCRIPTION_WIDTH,
		    gettext("disconnected from WLAN ESSID %s, BSSID %s %s"),
		    event->nwe_data.nwe_wlan_info.nwe_wlan.nww_essid,
		    event->nwe_data.nwe_wlan_info.nwe_wlan.nww_bssid,
		    "failed");
		break;

	case NWAM_EVENT_TYPE_INFO:
		(void) snprintf(description, sizeof (description),
		    "%s", event->nwe_data.nwe_info.nwe_message);
		break;

	case NWAM_EVENT_TYPE_IF_ACTION:
		action = nwam_action_to_string
		    (event->nwe_data.nwe_if_action.nwe_action);
		object = event->nwe_data.nwe_if_action.nwe_name;
		break;

	case NWAM_EVENT_TYPE_IF_STATE:
		object = event->nwe_data.nwe_if_state.nwe_name;
		if (event->nwe_data.nwe_if_state.nwe_addr_valid) {
			struct sockaddr_storage *address =
			    &(event->nwe_data.nwe_if_state.nwe_addr);
			struct sockaddr_storage *netmask =
			    &(event->nwe_data.nwe_if_state.nwe_netmask);
			struct sockaddr_in *v4addr;
			struct sockaddr_in6 *v6addr;
			char addrstr[NWAM_MAX_VALUE_LEN];
			int plen = mask2plen((struct sockaddr *)netmask);

			switch (address->ss_family) {
			case AF_INET:
				v4addr = (struct sockaddr_in *)address;
				(void) inet_ntop(AF_INET, &v4addr->sin_addr,
				    addrstr, sizeof (addrstr));
				break;
			case AF_INET6:
				v6addr = (struct sockaddr_in6 *)address;
				(void) inet_ntop(AF_INET6, &v6addr->sin6_addr,
				    addrstr, sizeof (addrstr));
				break;
			}
			(void) snprintf(statestr, sizeof (statestr),
			    "flags %x addr %s/%d",
			    event->nwe_data.nwe_if_state.nwe_flags,
			    addrstr, plen);
		} else {
			(void) snprintf(statestr, sizeof (statestr),
			    "flags %x", event->nwe_data.nwe_if_state.nwe_flags);
		}
		state = statestr;
		break;

	case NWAM_EVENT_TYPE_LINK_ACTION:
		action = nwam_action_to_string
		    (event->nwe_data.nwe_link_action.nwe_action);
		object = event->nwe_data.nwe_link_action.nwe_name;
		break;

	case NWAM_EVENT_TYPE_LINK_STATE:
		state = event->nwe_data.nwe_link_state.nwe_link_up ?
		    "up" : "down";
		object = event->nwe_data.nwe_link_state.nwe_name;
		break;
	}

	if (object != NULL && action != NULL) {
		(void) snprintf(description, sizeof (description),
		    "%s -> action %s", object, action);
	} else if (object != NULL && state != NULL) {
		(void) snprintf(description, sizeof (description),
		    "%s -> state %s", object, state);
	}

	if (display) {
		(void) printf("%-*s %-*s\n", EVENT_WIDTH,
		    nwam_event_type_to_string(event->nwe_type),
		    DESCRIPTION_WIDTH,
		    description);
	}
}

/*
 * listens for events and displays them via the eventhandler() function above.
 */
/* ARGSUSED */
static void
show_events_func(int argc, char *argv[])
{
	nwam_error_t err;
	nwam_event_t event;

	err = nwam_events_init();

	if (err != NWAM_SUCCESS)
		die_nwamerr(err, "could not bind to receive events");

	/* print header */
	(void) printf("%-*s %-*s\n", EVENT_WIDTH, "EVENT",
	    DESCRIPTION_WIDTH, "DESCRIPTION");

	do {
		/*
		 * Needed for stdout redirection to ensure event output is
		 * regularly flushed to file.
		 */
		(void) fflush(stdout);
		err = nwam_event_wait(&event);
		if (err == NWAM_SUCCESS) {
			eventhandler(event);
			nwam_event_free(event);
		}
	} while (err == NWAM_SUCCESS);
	die_nwamerr(err, "event handling stopped");
}

static void
prompt_sec_policy(dladm_wlan_secmode_t security_mode, dladm_wlan_key_t *key,
    dladm_wlan_eap_t *eap_data)
{
	uint_t k;

	if (security_mode == DLADM_WLAN_SECMODE_EAP) {
		char eap_input[PATH_MAX];
		char mode[3];
		char *user_prompt[] = {
			"EAP Identity [mandatory]",
			"EAP Anonymous Identity [optional]"
		};
		char *filename_prompt[] = {
			"CA Certificate [optional]",
			"EAP-TLS Private Key Filename [mandatory] \n",
			"EAP-TLS Client Certificate [optional if pk12 file was "
			"used for Private Key]"
		};

		/* ask eap mode */
		do {
			char eap_str[8];
			for (k = DLADM_SECOBJ_CLASS_TLS;
			    dladm_valid_secobjclass(k); k++)
				(void) printf("\n %d: %s", k - 2,
				    dladm_secobjclass2str(k, eap_str));
			(void) printf(gettext("\nEnter EAP mode: "));
			(void) fgets(mode, 2, stdin);
			(void) fflush(stdin);
			key->wk_class = atoi(mode) + 2;
		} while (key->wk_class != DLADM_SECOBJ_CLASS_TLS &&
		    key->wk_class != DLADM_SECOBJ_CLASS_TTLS &&
		    key->wk_class != DLADM_SECOBJ_CLASS_PEAP);

		/* user */
		k = 0;
		do {
			(void) memset(eap_input, 0, sizeof (eap_input));
			(void) printf(gettext("\nEnter %s : "), user_prompt[k]);
			(void) fgets(eap_input, sizeof (eap_input), stdin);
			(void) fflush(stdin);
		} while (eap_input[0] == '\n' ||
		    dladm_str2identity(eap_input, eap_data->eap_user));
		eap_data->eap_valid |= DLADM_EAP_ATTR_USER;

		/* anon */
		k++;
		do {
			(void) memset(eap_input, 0, sizeof (eap_input));
			(void) printf(gettext("\nEnter %s : "), user_prompt[k]);
			(void) fgets(eap_input, sizeof (eap_input), stdin);
			(void) fflush(stdin);
		} while (dladm_str2identity(eap_input, eap_data->eap_anon) &&
		    eap_input[0] != '\n');
		if (eap_input[0] != '\n')
			eap_data->eap_valid |= DLADM_EAP_ATTR_ANON;

		/* ca cert */
		k = 0;
		do {
			(void) memset(eap_input, 0, sizeof (eap_input));
			(void) printf(gettext("\nEnter %s : "),
			    filename_prompt[k]);
			(void) fgets(eap_input, sizeof (eap_input), stdin);
			(void) fflush(stdin);
		} while (dladm_str2crtname(eap_input, eap_data->eap_ca_cert) &&
		    eap_input[0] != '\n');
		if (eap_input[0] != '\n')
			eap_data->eap_valid |= DLADM_EAP_ATTR_CACERT;

		if (key->wk_class == DLADM_SECOBJ_CLASS_TLS) {
			/* priv key */
			k++;
			do {
				(void) memset(eap_input, 0, sizeof (eap_input));
				(void) printf(gettext("\nEnter %s: "),
				    filename_prompt[k]);
				(void) fgets(eap_input, sizeof (eap_input),
				    stdin);
				(void) fflush(stdin);
			} while (dladm_str2crtname(eap_input,
			    eap_data->eap_priv));
			eap_data->eap_valid |= DLADM_EAP_ATTR_PRIV;

			(void) fflush(stdin);
			/* cli cert */
			k++;
			do {
				(void) memset(eap_input, 0, sizeof (eap_input));
				(void) printf(gettext("\nEnter %s: "),
				    filename_prompt[k]);
				(void) fgets(eap_input, sizeof (eap_input),
				    stdin);
				(void) fflush(stdin);
			} while (dladm_str2crtname(eap_input,
			    eap_data->eap_cli_cert) && eap_input[0] != '\n');
			if (eap_input[0] != '\n')
				eap_data->eap_valid |= DLADM_EAP_ATTR_CLICERT;
		}
	} else
		key->wk_class = (dladm_secobj_class_t)security_mode;

	while (dladm_secobj_prompt(key, NULL) != DLADM_STATUS_OK) {}

	/* TODO: PKCS11 keystore: if true, import the certificates here */
}

static ofmt_field_t scanres_common_fields[] = {
{ "    WlanID",	11, 0,	NULL},
{ "BSSID",	19, 1,	NULL},
{ "CHAN",	5,  2,	NULL},
{ "STRENGTH",	9,  3,	NULL},
{ "SPEED",	6,  4,	NULL},
{ "ESSID",	32, 5,	NULL},
{ "SECMODE",	1,  6,	NULL},
{ NULL,		0,  0,	NULL}};

static char *scanres_select_fields =
	"    WlanID,bssid,chan,strength,speed,essid,secmode";

static boolean_t
do_print_nww_attr_cb(ofmt_arg_t *ofarg, char *buf, uint_t bufsize)
{
	nwam_wlan_t	*attrp = ofarg->ofmt_cbarg;

	switch (ofarg->ofmt_id) {
	case 0:
		if (attrp->nww_wlanid)
			(void) snprintf(buf, bufsize, "%u)  {%u}",
			    attrp->nww_scanid + 1, attrp->nww_wlanid);
		else
			(void) snprintf(buf, bufsize, "%u)  NONE",
			    attrp->nww_scanid + 1);
		break;
	case 5:
		(void) memcpy(buf, attrp->nww_essid, DLADM_WLAN_MAX_ESSID_LEN);
		break;
	case 1:
		(void) dladm_wlan_bssid2str(attrp->nww_bssid, buf);
		break;
	case 3:
		(void) dladm_wlan_strength2str(attrp->nww_strength, buf);
		break;
	case 6:
		(void) strlcpy(buf, attrp->nww_ietxt,
		    sizeof (attrp->nww_ietxt));
		break;
	case 4:
		(void) dladm_wlan_rate2str(attrp->nww_rate, buf);
		break;
	case 2:
		(void) dladm_wlan_freq2channel(attrp->nww_freq, buf);
		break;
	default:
		break;
	}

	return (B_TRUE);
}

static void
select_wifi_func(int argc, char *argv[])
{
	nwam_error_t err;
	nwam_event_t event;

	uint_t i = 0, choice = 0, num_wlans = 0;

	nwam_wlan_t *wlans = NULL;
	nwam_wlan_t *mywlan = NULL;
	dladm_wlan_eap_t *eap_data = NULL;
	dladm_wlan_key_t *key = NULL;

	char selection[3]; /* max num is "64" */

	ofmt_status_t		oferr;
	ofmt_field_t		*template, *of;
	ofmt_handle_t		ofmt;

	if (argc != 1)
		die_usage(CMD_SELECT_WIFI);

	if ((err = nwam_wlan_scan(argv[0])) != NWAM_SUCCESS)
		die_nwamerr(err, "Scan request failed for %s", argv[0]);
	else
		(void) printf("Scanning on link %s ...\n", argv[0]);

	if ((err = nwam_events_init()) != NWAM_SUCCESS)
		die_nwamerr(err, "Could not bind to receive events");

	for (;;) {
		if ((err = nwam_event_wait(&event)) == NWAM_SUCCESS &&
		    event->nwe_type == NWAM_EVENT_TYPE_WLAN_SCAN_REPORT) {
			nwam_event_free(event);
			break;
		} else if (err == NWAM_SUCCESS) {
			nwam_event_free(event);
		} else {
			die_nwamerr(err, "Event handling stopped");
		}
	}
	nwam_events_fini();

	if ((err = nwam_wlan_get_scan_results(argv[0], &num_wlans, &wlans)) !=
	    NWAM_SUCCESS) {
		die_nwamerr(err, "Could not retrieve scan results for %s",
		    argv[0]);
	} else if (num_wlans == 0) {
		(void) printf("No wireless networks found during last scan(%s)",
		    argv[0]);
	} else {
		template = scanres_common_fields;
		for (of = template; of->of_name != NULL; of++) {
			if (of->of_cb == NULL)
				of->of_cb = do_print_nww_attr_cb;
		}

		oferr = ofmt_open(scanres_select_fields, template, 0, 0, &ofmt);
		if (oferr != OFMT_SUCCESS) {
			char buf[DLADM_STRSIZE];
			free(wlans);
			die("Internal Error %s", ofmt_strerror(ofmt, oferr,
			    buf, sizeof (buf)));
		}
	}
	(void) printf("\n");
	for (i = 0; i < num_wlans; i++)
		ofmt_print(ofmt, &wlans[i]);
	ofmt_close(ofmt);

	(void) printf(gettext("\n%d)  Specify a different AP\n"), i + 1);
	(void) printf(gettext("\nChoose WLAN to connect to [1-%d]: "), i + 1);

	/* Loop until valid selection made */
	for (;;) {
		if (fgets(selection, 3, stdin) != NULL &&
		    (choice = atoi(selection)) >= 1 && choice > 0 &&
		    choice <= (i + 1))
			break;
	}

	if (choice != i + 1) {
		mywlan = &wlans[choice - 1];
	} else {
		mywlan = calloc(1, sizeof (nwam_wlan_t));
		if (mywlan == NULL) {
			free(wlans);
			die("Out of Memory");
		}
	}

	/*
	 * If "Other" or a hidden WLAN is selected, ask for ESSID
	 * In this case we won't search if this ESSID is a known wlan
	 * To perform this search we require bssid
	 */
	if (mywlan->nww_esslen == 0) {
		char tmpess[DLADM_WLAN_MAX_ESSID_LEN+1];
		do {
			(void) memset(tmpess, 0, sizeof (tmpess));
			(void) printf(gettext("\nEnter Wlan SSID (max 32 chars)"
			    ": "));
			while (fgets(tmpess, sizeof (tmpess), stdin) == NULL) {}
		} while (strspn(tmpess, " \n\t") ==
		    strnlen(tmpess, DLADM_WLAN_MAX_ESSID_LEN));
		mywlan->nww_esslen = strnlen(tmpess, DLADM_WLAN_MAX_ESSID_LEN);
		mywlan->nww_esslen--;
		(void) memcpy(mywlan->nww_essid, tmpess, mywlan->nww_esslen);
	}

	/* "Other" was selected - Get Secmode */
	if (choice == i + 1) {
		for (;;) {
			uint_t j;
			char secmode_name[8];
			for (j = DLADM_WLAN_SECMODE_NONE;
			    j <= DLADM_WLAN_SECMODE_EAP; j++)
				(void) printf("\n%d: %s", j,
				    dladm_wlan_secmode2str(j, secmode_name));
			(void) printf(gettext("\nEnter security mode: "));
			if (fgets(selection, 2, stdin) != NULL &&
			    (mywlan->nww_security_mode = atoi(selection)) &&
			    mywlan->nww_security_mode <= DLADM_WLAN_SECMODE_EAP)
				break;
		}
	}

	/* It is not a known wlan */
	if (mywlan->nww_wlanid == 0 &&
	    mywlan->nww_security_mode != DLADM_WLAN_SECMODE_NONE) {
		key = calloc(1, sizeof (dladm_wlan_key_t));

		if (mywlan->nww_security_mode == DLADM_WLAN_SECMODE_EAP)
			eap_data = calloc(1, sizeof (dladm_wlan_eap_t));
		prompt_sec_policy(mywlan->nww_security_mode, key, eap_data);
	}

	(void) printf(gettext("\nConnecting link %s ...\n"), argv[0]);
	err = nwam_wlan_select(argv[0], mywlan, key, eap_data);
	if (err != NWAM_SUCCESS)
		(void) printf(gettext("\nCould not select WLAN %s [%s]\n"),
		    mywlan->nww_essid, nwam_strerror(err));
	if (choice == i + 1)
		free(mywlan);
	if (wlans != NULL)
		free(wlans);
	if (key != NULL)
		free(key);
	if (eap_data != NULL)
		free(eap_data);

	if ((err = nwam_events_init()) != NWAM_SUCCESS)
		die_nwamerr(err, "Could not bind to receive events");

	for (;;) {
		if ((err = nwam_event_wait(&event)) == NWAM_SUCCESS &&
		    (event->nwe_type ==
		    NWAM_EVENT_TYPE_WLAN_CONNECTION_REPORT ||
		    event->nwe_type ==
		    NWAM_EVENT_TYPE_WLAN_DISASSOCIATION_REPORT ||
		    event->nwe_type == NWAM_EVENT_TYPE_WLAN_WRONG_KEY)) {
			nwam_event_free(event);
			break;
		} else if (err == NWAM_SUCCESS) {
			nwam_event_free(event);
		} else {
			die_nwamerr(err, "Event handling stopped");
		}
	}

	nwam_events_fini();
}

int
main(int argc, char *argv[])
{
	int i;
	char *state;

	(void) setlocale(LC_ALL, "");
	(void) textdomain(TEXT_DOMAIN);

	if ((execname = strrchr(argv[0], '/')) == NULL)
		execname = argv[0];
	else
		execname++;

	if (argc < 2) {
		usage(B_FALSE);
		exit(EXIT_FAILURE);
	}

	for (i = CMD_MIN; i <= CMD_MAX; i++) {
		if (strcmp(argv[1], cmd_to_str(i)) == 0) {
			if (cmdtab[i].cmd_needs_nwamd) {
				state = smf_get_state(NWAM_FMRI);
				if (state == NULL || strcmp(state,
				    SCF_STATE_STRING_ONLINE) != 0) {
					free(state);
					die("enable '%s' to use '%s %s'",
					    NWAM_FMRI, execname,
					    cmd_to_str(cmdtab[i].cmd_num));
				}
				free(state);
			}

			cmdtab[i].cmd_handler(argc - 2, &(argv[2]));

			exit(EXIT_SUCCESS);
		}
	}

	(void) fprintf(stderr, gettext("%s: unknown subcommand '%s'\n"),
	    execname, argv[1]);
	usage(B_FALSE);

	return (1);
}
