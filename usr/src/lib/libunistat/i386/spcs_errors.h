/* THIS FILE IS AUTOMATICALLY GENERATED: DO NOT EDIT */
#define	DSW_EEMPTY 0x20001 /* Empty string */
#define	DSW_EHDRBMP 0x20002 /* Unable to read or write bitmap header */
#define	DSW_EINVALBMP 0x20003 /* Bitmap magic number is not valid */
#define	DSW_EMISMATCH 0x20004 /* Arguments inconsistent with current bitmap */
#define	DSW_ESHDSIZE 0x20005 /* Shadow too small */
#define	DSW_EBMPSIZE 0x20006 /* Bitmap too small */
#define	DSW_EREGISTER 0x20007 /* Registration failed */
#define	DSW_EDIRTY 0x20008 /* Bitmap is dirty */
#define	DSW_ESHUTDOWN 0x20009 /* Shadow shutting down */
#define	DSW_EOPEN 0x2000a /* nsc_open failed */
#define	DSW_EINUSE 0x2000b /* Volume in use */
#define	DSW_ENOTFOUND 0x2000c /* Volume not enabled */
#define	DSW_ECOPYING 0x2000d /* Volume copy in progress */
#define	DSW_EIO 0x2000e /* I/O error copying data */
#define	DSW_EABORTED 0x2000f /* Copy operation aborted */
#define	DSW_EPARTSIZE 0x20010 /* nsc_partsize failed */
#define	DSW_EDEPENDENCY 0x20011 /* Volumes are not currently independent */
#define	DSW_EMAPMEMORY 0x20012 /* Could not allocate memory for bitmaps in interface library (malloc) */
#define	DSW_ERSRVFAIL 0x20013 /* nsc_reserve failed */
#define	DSW_EOPACKAGE 0x20014 /* Another package would not allow target to be changed at this moment */
#define	DSW_EINCOMPLETE 0x20015 /* Source shadow volume is not complete due to earlier overflow */
#define	DSW_ENOTEXPORTED 0x20016 /* Shadow volume is not exported */
#define	DSW_EALREADY 0x20017 /* Operation already successfully performed */
#define	DSW_EWRONGTYPE 0x20018 /* Wrong type of shadow group */
#define	DSW_EOMAGIC 0x20019 /* Overflow volume magic number or name does not match */
#define	DSW_EODEPENDENCY 0x2001a /* Volumes are currently dependent on overflow volume */
#define	DSW_EOFFLINE 0x2001b /* Volume offline */
#define	DSW_ENOTLOCKED 0x2001c /* Set not pid-locked */
#define	DSW_ECNOTFOUND 0x2001d /* Cluster resource group not found */
#define	DSW_EGNOTFOUND 0x2001e /* No such group defined */
#define	DSW_EDISABLE 0x2001f /* One or more sets failed to be disabled */
#define	DSW_EISEXPORTED 0x20020 /* Update or copy not allowed on an exported shadow volume */
#define	RDC_EEPERM 0x40001 /* Must be super-user to execute */
#define	RDC_EEINVAL 0x1040002 /* Invalid flag %s */
#define	RDC_EALREADY 0x2040003 /* %s ==> %s not already enabled */
#define	RDC_EDISABLEPENDING 0x2040004 /* Disable pending on %s ==> %s, try again later */
#define	RDC_EENABLEPENDING 0x2040005 /* Enable pending on %s ==> %s, try again later */
#define	RDC_EOPNSECSRC 0x2040006 /* Host %s: Source %s, open remote host failed */
#define	RDC_EOPNPRISRC 0x2040007 /* Host %s: Source %s, open failed */
#define	RDC_EOPNSECTGT 0x2040008 /* Host %s: Target %s, open remote host failed */
#define	RDC_EOPNPRITGT 0x2040009 /* Host %s: Target %s, open failed */
#define	RDC_ENOLOCHOST 0x404000a /* Disks %s,%s do not reside on host %s or host %s */
#define	RDC_ENOTREMOTE 0x304000b /* Master %s and Slave %s have same id %s */
#define	RDC_EMASTERNOTLOCAL 0x204000c /* Master %s does not reside on this host: %s */
#define	RDC_EYOUNGER 0x204000d /* %s is failed from  %s..cannot proceed */
#define	RDC_ENODISABLE 0x4000e /* Currently syncing, unable to disable */
#define	RDC_ECONN 0x104000f /* Unable to connect to %s: local disable complete, remote disable aborted */
#define	RDC_EFLUSH 0x2040010 /* Will disable when ATM queue flushes on %s ==> %s */
#define	RDC_ESYNCING 0x1040011 /* Request not serviced, %s is currently being synced. */
#define	RDC_EINITREMOTE 0x2040012 /* Could not initialize remote data structures on %s ==> %s set */
#define	RDC_EINITLOCAL 0x2040013 /* Could not initialize local data structures on %s ==> %s set */
#define	RDC_ENOSLAVE 0x1040014 /* Target %s is failed, cannot set up for sync operation */
#define	RDC_ESIZE 0x6040015 /* Size of Primary %s:%s(%s) must be less than or equal to size of Secondary %s:%s(%s) */
#define	RDC_ESIZCHG 0x40016 /* Device size change in dual copy set */
#define	RDC_ENOBMAP 0x40017 /* Recovery bitmaps not allocated */
#define	RDC_EINITAFTERSYNC 0x2040018 /* Could not initialize data structures on %s ==> %s set after sync */
#define	RDC_EFAIL 0x1040019 /* Dual copy failed, offset:%s */
#define	RDC_EMIRRORDOWN 0x4001a /* Mirror node is down */
#define	RDC_EGETSIZE 0x304001b /* %s:%s has invalid size (%s)..cannot proceed */
#define	RDC_EUPDATE 0x404001c /* Update sync %s:%s ==> %s:%s only allowed for an rdc device set */
#define	RDC_EEQUAL 0x204001d /* Illegal device set %s:%s to itself */
#define	RDC_EMATCH 0x204001e /* Device %s:%s belongs to another RDC device set */
#define	RDC_EMASTER 0x404001f /* Changing the primary SNDR device %s:%s to become secondary and the secondary SNDR device %s:%s to become primary is not allowed in advanced configs */
#define	RDC_ECONNOPEN 0x2040020 /* Could not open file %s:%s on remote node */
#define	RDC_ENOPROC 0x40021 /* Could not create rdc_config process */
#define	RDC_EBITMAP 0x1040022 /* Allocation of bitmap device %s failed */
#define	RDC_EMIRRORUP 0x40023 /* Change request denied, volume mirror is up */
#define	RDC_EVERSION 0x40024 /* Change request denied, don't understand request version */
#define	RDC_EEMPTY 0x40025 /* Empty string */
#define	RDC_EENABLED 0x4040026 /* %s:%s ==> %s:%s is already enabled */
#define	RDC_EOPEN 0x2040027 /* Unable to open %s:%s */
#define	RDC_EADDTOIF 0x2040028 /* Unable to add interface %s to %s */
#define	RDC_EREGISTER 0x1040029 /* Unable to register %s */
#define	RDC_ENOTPRIMARY 0x404002a /* Not primary, cannot sync %s:%s and %s:%s */
#define	RDC_ERSYNCNEEDED 0x404002b /* Reverse sync needed, cannot sync %s:%s ==> %s:%s */
#define	RDC_ENOTHREADS 0x4002c /* Unable to initialize the kernel thread set */
#define	RDC_ENETCONFIG 0x4002d /* NULL struct knetconfig passed down from user program */
#define	RDC_ENETBUF 0x104002e /* NULL struct netbuf passed down from user program for %s */
#define	RDC_ESTATE 0x404002f /* The state of %s:%s ==> %s:%s prevents this operation */
#define	RDC_EMANY2ONE 0x4040030 /* Cannot enable %s:%s ==> %s:%s, secondary in use in another set */
#define	RDC_ERSTATE 0x4040031 /* The remote state of %s:%s ==> %s:%s prevents this operation */
#define	RDC_EBMPINUSE 0x1040032 /* The bitmap %s is already in use */
#define	RDC_EVOLINUSE 0x1040033 /* The volume %s is already in use */
#define	RDC_EMULTI 0x40034 /* Cannot use direct I/O on first leg of multi hop config */
#define	RDC_EGROUP 0x5040035 /* Cannot add %s:%s ==> %s:%s to group %s */
#define	RDC_EGROUPMODE 0x40036 /* Cannot reconfigure sync/async on members of a group */
#define	RDC_ENOTLOGGING 0x4040037 /* Cannot reconfig %s:%s to %s:%s, Must be in logging mode */
#define	RDC_EBMPRECONFIG 0x2040038 /* Bitmap reconfig failed %s:%s */
#define	RDC_EBMAPLOGGING 0x2040039 /* Cannot overwrite bitmap as the set %s:%s is not in logging mode */
#define	RDC_EQDISABLEPEND 0x104003a /* Disable pending on diskq %s, try again later */
#define	RDC_EQNOTLOGGING 0x104003b /* Cannot change disk queue %s, all associated sets must be in logging mode */
#define	RDC_EQALREADY 0x404003c /* %s:%s ==> %s:%s already has a disk queue attached */
#define	RDC_EQNOQUEUE 0x404003d /* Disk queue does not exist for set %s:%s ==> %s:%s */
#define	RDC_EQFLUSHING 0x104003e /* Operation not possible. Disk queue %s is flushing, try again later */
#define	RDC_EQWRONGMODE 0x4003f /* Disk queue operations on synchronous sets not allowed */
#define	RDC_EDISKQINUSE 0x1040040 /* Disk queue %s is already in use */
#define	RDC_EQNOADD 0x1040041 /* Unable to enable disk queue %s */
#define	RDC_EQINITFAIL 0x1040042 /* Initialization of disk queue %s failed */
#define	RDC_EQNOTEMPTY 0x1040043 /* Operation not possible, disk queue %s is not empty. */
#define	RDC_EQUEISREP 0x1040044 /* Disk queue %s operation not possible, set is in replicating mode */
#define	RDC_EQNORSYNC 0x4040045 /* Cannot reverse sync %s:%s <== %s:%s, set is in queuing mode */
#define	RDC_ESETNOTLOGGING 0x2040046 /* can not start sync as set %s:%s is not logging */
#define	RDC_EBITMAP2SMALL 0x1040047 /* Allocation of bitmap device %s failed, volume is too small */
#define	SPCS_EOVERFLOW 0x10001 /* status codes and/or supporting information lost */
#define	SPCS_EBADHANDLE 0x10002 /* The handle presented for access is not valid */
#define	SPCS_EACCESS 0x10003 /* Unable to grant access */
#define	SPCS_EINTERNAL 0x10004 /* An internal error happened */
#define	SPCS_EINUSE 0x10005 /* The handle is already in use */
#define	SPCS_ENODRIVER 0x10006 /* The referenced SPCS driver could not be loaded: */
#define	SPCS_EVERSION 0x10007 /* Expected version not found, got libspcs.jar version: */
#define	SPCS_EUNEXPECTED 0x10008 /* The libspcs.jar version does not match the libspcs.so version */
#define	SPCS_EWRONGMOD 0x10009 /* Handle presented belongs to some other SPCS module */
#define	SPCS_ENEEDROOT 0x1000a /* Root privilege required for libspcs access */
#define	SPCS_EASYNCTEST 0x8101000b /* Test of asynch status output@@ */
#define	SDBC_EDUMMY 0x50001 /* SDBC Place holder until definitions checked in by coresw dev. */
#define	SDBC_EOBSOLETE 0x50002 /* Obsolete sdbc ioctl used */
#define	SDBC_EDISABLE 0x50003 /* Cache deconfig failed.  Not initialized */
#define	SDBC_ECLUSTER_SIZE 0x50004 /* Get cluster size operation failed.  Cache not initialized */
#define	SDBC_ECLUSTER_DATA 0x50005 /* Get cluster data operation failed.  Cache not initialized */
#define	SDBC_EGLMUL_SIZE 0x50006 /* Get global size operation failed.  Cache not initialized */
#define	SDBC_EGLMUL_INFO 0x50007 /* Get global info operation failed.  Cache not initialized */
#define	SDBC_ETOGGLE_FLUSH 0x1050008 /* Cache flushing mode is (mode %s) */
#define	SDBC_EUNSUPPORTED 0x1050009 /* Unknown ioctl: unsupported (cmd %s) */
#define	SDBC_EDISABLEFAIL 0x5000a /* Cache not deconfigured */
#define	SDBC_EPINNED 0x105000b /* Pinned data on %s */
#define	SDBC_EACTIVERDC 0x5000c /* Active RDC pair not closed */
#define	SDBC_EUNREG 0x5000d /* Could not unregister sdbc io module */
#define	SDBC_EALREADY 0x5000e /* Cache enable failed.  Already initialized. */
#define	SDBC_EENABLEFAIL 0x5000f /* Cache enable failed. */
#define	SDBC_ESIZE 0x1050010 /* Cache block size %s not supported. */
#define	SDBC_EMAGIC 0x50011 /* Mismatched versions of scmadm and sdbc module. */
#define	SDBC_ENONETMEM 0x50012 /* Insufficient memory for cache. */
#define	SDBC_ENOIOBMEM 0x50013 /* No memory for iobuf hooks. */
#define	SDBC_ENOIOBCB 0x50014 /* Missing iobuf driver callback. */
#define	SDBC_ENOHANDLEMEM 0x50015 /* No memory for buffer handles. */
#define	SDBC_EMEMCONFIG 0x50016 /* Cache memory initialzation error. */
#define	SDBC_EFLUSHTHRD 0x50017 /* Flush threads create failure. */
#define	SDBC_ENOHASH 0x50018 /* Cannot create hash table */
#define	SDBC_ENOCB 0x50019 /* Cannot allocate cache block structures */
#define	SDBC_ENOCCTL 0x5001a /* Cannot allocate cctl sync structures */
#define	SDBC_ENOCD 0x5001b /* Cannot allocate cache data structures */
#define	SDBC_ENOMIRRORCD 0x5001c /* Cannot allocate cache data structures for mirror areas */
#define	SDBC_ENOSHAREDFILE 0x5001d /* Cannot allocate shared file area */
#define	SDBC_ENOSFNV 0x5001e /* Cannot allocate shared file area in nvmem */
#define	SDBC_ENOREFRESH 0x5001f /* Unable to refresh host memory */
#define	SDBC_EINVHOSTID 0x2050020 /* Hostid %s greater than maximum (%s) */
#define	SDBC_ENOTSAME 0x2050021 /* Self host %s and mirror host %s cannot be the same */
#define	SDBC_ENORMLOCKS 0x50022 /* No RM locks configured */
#define	SDBC_EGLDMAFAIL 0x50023 /* Global information transfer failed */
#define	SDBC_EMODELCONVERT 0x50024 /* 64 bit conversion called on a 32 bit system */
#define	SDBC_EABUFS 0x50025 /* Anonymous buffers currently allocated from sdbc */
#define	SDBC_ENODEVENABLED 0x50026 /* Device not enabled in cache */
#define	SOLARIS_EPERM 0x1 /* Not super-user */
#define	SOLARIS_ENOENT 0x2 /* No such file or directory */
#define	SOLARIS_ESRCH 0x3 /* No such process */
#define	SOLARIS_EINTR 0x4 /* interrupted system call */
#define	SOLARIS_EIO 0x5 /* I/O error */
#define	SOLARIS_ENXIO 0x6 /* No such device or address */
#define	SOLARIS_E2BIG 0x7 /* Arg list too long */
#define	SOLARIS_ENOEXEC 0x8 /* Exec format error */
#define	SOLARIS_EBADF 0x9 /* Bad file number */
#define	SOLARIS_ECHILD 0xa /* No children */
#define	SOLARIS_EAGAIN 0xb /* Resource temporarily unavailable */
#define	SOLARIS_ENOMEM 0xc /* Not enough core */
#define	SOLARIS_EACCES 0xd /* Permission denied */
#define	SOLARIS_EFAULT 0xe /* Bad address */
#define	SOLARIS_ENOTBLK 0xf /* Block device required */
#define	SOLARIS_EBUSY 0x10 /* Mount device busy */
#define	SOLARIS_EEXIST 0x11 /* File exists */
#define	SOLARIS_EXDEV 0x12 /* Cross-device link */
#define	SOLARIS_ENODEV 0x13 /* No such device */
#define	SOLARIS_ENOTDIR 0x14 /* Not a directory */
#define	SOLARIS_EISDIR 0x15 /* Is a directory */
#define	SOLARIS_EINVAL 0x16 /* Invalid argument */
#define	SOLARIS_ENFILE 0x17 /* File table overflow */
#define	SOLARIS_EMFILE 0x18 /* Too many open files */
#define	SOLARIS_ENOTTY 0x19 /* Inappropriate ioctl for device */
#define	SOLARIS_ETXTBSY 0x1a /* Text file busy */
#define	SOLARIS_EFBIG 0x1b /* File too large */
#define	SOLARIS_ENOSPC 0x1c /* No space left on device */
#define	SOLARIS_ESPIPE 0x1d /* Illegal seek */
#define	SOLARIS_EROFS 0x1e /* Read only file system */
#define	SOLARIS_EMLINK 0x1f /* Too many links */
#define	SOLARIS_EPIPE 0x20 /* Broken pipe */
#define	SOLARIS_EDOM 0x21 /* Math arg out of domain of func */
#define	SOLARIS_ERANGE 0x22 /* Math result not representable */
#define	SOLARIS_ENOMSG 0x23 /* No message of desired type */
#define	SOLARIS_EIDRM 0x24 /* Identifier removed */
#define	SOLARIS_ECHRNG 0x25 /* Channel number out of range */
#define	SOLARIS_EL2NSYNC 0x26 /* Level 2 not synchronized */
#define	SOLARIS_EL3HLT 0x27 /* Level 3 halted */
#define	SOLARIS_EL3RST 0x28 /* Level 3 reset */
#define	SOLARIS_ELNRNG 0x29 /* Link number out of range */
#define	SOLARIS_EUNATCH 0x2a /* Protocol driver not attached */
#define	SOLARIS_ENOCSI 0x2b /* No CSI structure available */
#define	SOLARIS_EL2HLT 0x2c /* Level 2 halted */
#define	SOLARIS_EDEADLK 0x2d /* Deadlock condition. */
#define	SOLARIS_ENOLCK 0x2e /* No record locks available. */
#define	SOLARIS_ECANCELED 0x2f /* Operation canceled */
#define	SOLARIS_ENOTSUP 0x30 /* Operation not supported */
#define	SOLARIS_EDQUOT 0x31 /* Disc quota exceeded */
#define	SOLARIS_EBADE 0x32 /* invalid exchange */
#define	SOLARIS_EBADR 0x33 /* invalid request descriptor */
#define	SOLARIS_EXFULL 0x34 /* exchange full */
#define	SOLARIS_ENOANO 0x35 /* no anode */
#define	SOLARIS_EBADRQC 0x36 /* invalid request code */
#define	SOLARIS_EBADSLT 0x37 /* invalid slot */
#define	SOLARIS_EDEADLOCK 0x38 /* file locking deadlock error */
#define	SOLARIS_EBFONT 0x39 /* bad font file fmt */
#define	SOLARIS_EUNUSED58 0x3a /* not defined */
#define	SOLARIS_EUNUSED59 0x3b /* not defined */
#define	SOLARIS_ENOSTR 0x3c /* Device not a stream */
#define	SOLARIS_ENODATA 0x3d /* no data (for no delay io) */
#define	SOLARIS_ETIME 0x3e /* timer expired */
#define	SOLARIS_ENOSR 0x3f /* out of streams resources */
#define	SOLARIS_ENONET 0x40 /* Machine is not on the network */
#define	SOLARIS_ENOPKG 0x41 /* Package not installed */
#define	SOLARIS_EREMOTE 0x42 /* The object is remote */
#define	SOLARIS_ENOLINK 0x43 /* the link has been severed */
#define	SOLARIS_EADV 0x44 /* advertise error */
#define	SOLARIS_ESRMNT 0x45 /* srmount error */
#define	SOLARIS_ECOMM 0x46 /* Communication error on send */
#define	SOLARIS_EPROTO 0x47 /* Protocol error */
#define	SOLARIS_EUNISED72 0x48 /* undefined */
#define	SOLARIS_EUNISED73 0x49 /* undefined */
#define	SOLARIS_EMULTIHOP 0x4a /* multihop attempted */
#define	SOLARIS_EUNISED75 0x4b /* undefined */
#define	SOLARIS_EUNISED76 0x4c /* undefined */
#define	SOLARIS_EBADMSG 0x4d /* trying to read unreadable message */
#define	SOLARIS_ENAMETOOLONG 0x4e /* path name is too long */
#define	SOLARIS_EOVERFLOW 0x4f /* value too large to be stored in data type */
#define	SOLARIS_ENOTUNIQ 0x50 /* given log. name not unique */
#define	SOLARIS_EBADFD 0x51 /* f.d. invalid for this operation */
#define	SOLARIS_EREMCHG 0x52 /* Remote address changed */
#define	SOLARIS_ELIBACC 0x53 /* Can't access a needed shared lib. */
#define	SOLARIS_ELIBBAD 0x54 /* Accessing a corrupted shared lib. */
#define	SOLARIS_ELIBSCN 0x55 /* .lib section in a.out corrupted. */
#define	SOLARIS_ELIBMAX 0x56 /* Attempting to link in too many libs. */
#define	SOLARIS_ELIBEXEC 0x57 /* Attempting to exec a shared library. */
#define	SOLARIS_EILSEQ 0x58 /* Illegal byte sequence. */
#define	SOLARIS_ENOSYS 0x59 /* Unsupported file system operation */
#define	SOLARIS_ELOOP 0x5a /* Symbolic link loop */
#define	SOLARIS_ERESTART 0x5b /* Restartable system call */
#define	SOLARIS_ESTRPIPE 0x5c /* if pipe/FIFO, don't sleep in stream head */
#define	SOLARIS_ENOTEMPTY 0x5d /* directory not empty */
#define	SOLARIS_EUSERS 0x5e /* Too many users (for UFS) */
#define	SOLARIS_ENOTSOCK 0x5f /* Socket operation on non-socket */
#define	SOLARIS_EDESTADDRREQ 0x60 /* Destination address required */
#define	SOLARIS_EMSGSIZE 0x61 /* Message too long */
#define	SOLARIS_EPROTOTYPE 0x62 /* Protocol wrong type for socket */
#define	SOLARIS_ENOPROTOOPT 0x63 /* Protocol not available */
#define	SOLARIS_EUNUSED100 0x64 /* undefined */
#define	SOLARIS_EUNUSED101 0x65 /* undefined */
#define	SOLARIS_EUNUSED102 0x66 /* undefined */
#define	SOLARIS_EUNUSED103 0x67 /* undefined */
#define	SOLARIS_EUNUSED104 0x68 /* undefined */
#define	SOLARIS_EUNUSED105 0x69 /* undefined */
#define	SOLARIS_EUNUSED106 0x6a /* undefined */
#define	SOLARIS_EUNUSED107 0x6b /* undefined */
#define	SOLARIS_EUNUSED108 0x6c /* undefined */
#define	SOLARIS_EUNUSED109 0x6d /* undefined */
#define	SOLARIS_EUNUSED110 0x6e /* undefined */
#define	SOLARIS_EUNUSED111 0x6f /* undefined */
#define	SOLARIS_EUNUSED112 0x70 /* undefined */
#define	SOLARIS_EUNUSED113 0x71 /* undefined */
#define	SOLARIS_EUNUSED114 0x72 /* undefined */
#define	SOLARIS_EUNUSED115 0x73 /* undefined */
#define	SOLARIS_EUNUSED116 0x74 /* undefined */
#define	SOLARIS_EUNUSED117 0x75 /* undefined */
#define	SOLARIS_EUNUSED118 0x76 /* undefined */
#define	SOLARIS_EUNUSED119 0x77 /* undefined */
#define	SOLARIS_EPROTONOSUPPORT 0x78 /* Protocol not supported */
#define	SOLARIS_ESOCKTNOSUPPORT 0x79 /* Socket type not supported */
#define	SOLARIS_EOPNOTSUPP 0x7a /* Operation not supported on socket */
#define	SOLARIS_EPFNOSUPPORT 0x7b /* Protocol family not supported */
#define	SOLARIS_EAFNOSUPPORT 0x7c /* Address family not supported by protocol family */
#define	SOLARIS_EADDRINUSE 0x7d /* Address already in use */
#define	SOLARIS_EADDRNOTAVAIL 0x7e /* Can't assign requested address operational errors */
#define	SOLARIS_ENETDOWN 0x7f /* Network is down */
#define	SOLARIS_ENETUNREACH 0x80 /* Network is unreachable */
#define	SOLARIS_ENETRESET 0x81 /* Network dropped connection because of reset */
#define	SOLARIS_ECONNABORTED 0x82 /* Software caused connection abort */
#define	SOLARIS_ECONNRESET 0x83 /* Connection reset by peer */
#define	SOLARIS_ENOBUFS 0x84 /* No buffer space available */
#define	SOLARIS_EISCONN 0x85 /* Socket is already connected */
#define	SOLARIS_ENOTCONN 0x86 /* Socket is not connected */
#define	SOLARIS_EXENIX135 0x87 /* XENIX 135 */
#define	SOLARIS_EXENIX136 0x88 /* XENIX 136 */
#define	SOLARIS_EXENIX137 0x89 /* XENIX 137 */
#define	SOLARIS_EXENIX138 0x8a /* XENIX 138 */
#define	SOLARIS_EXENIX139 0x8b /* XENIX 139 */
#define	SOLARIS_EXENIX140 0x8c /* XENIX 140 */
#define	SOLARIS_EXENIX141 0x8d /* XENIX 141 */
#define	SOLARIS_EXENIX142 0x8e /* XENIX 142 */
#define	SOLARIS_ESHUTDOWN 0x8f /* Can't send after socket shutdown */
#define	SOLARIS_ETOOMANYREFS 0x90 /* Too many references: can't splice */
#define	SOLARIS_ETIMEDOUT 0x91 /* Connection timed out */
#define	SOLARIS_ECONNREFUSED 0x92 /* Connection refused */
#define	SOLARIS_EHOSTDOWN 0x93 /* Host is down */
#define	SOLARIS_EHOSTUNREACH 0x94 /* No route to host */
#define	SOLARIS_EALREADY 0x95 /* operation already in progress */
#define	SOLARIS_EINPROGRESS 0x96 /* operation now in progress */
#define	SOLARIS_ESTALE 0x97 /* Stale NFS file handle */
#define	SV_ENOSLOTS 0x30001 /* No more SVs available */
#define	SV_EARRBOUNDS 0x3030002 /* Array bounds check (min %s, max %s, got %s) */
#define	SV_EDEVEXIST 0x30003 /* Device already present in kernel */
#define	SV_ENOSSTATE 0x30004 /* Soft state alloc failed */
#define	SV_EBADSSTATE 0x30005 /* Soft state corrupted */
#define	SV_EMKNOD 0x30006 /* failed */
#define	SV_ENODEV 0x30007 /* Device not present in kernel configuration */
#define	SV_EENABLED 0x30008 /* Device already enabled */
#define	SV_ELOAD 0x30009 /* Unable to load/hold underlying disk driver */
#define	SV_ESDOPEN 0x3000a /* Error from nsc_open() */
#define	SV_EDISABLED 0x3000b /* Device not enabled */
#define	SV_EBUSY 0x3000c /* Unable to disable device - device in use */
#define	SV_EBADDEV 0x3000d /* Bad dev_t in config structure */
#define	SV_ESTRATEGY 0x3000e /* Recursive strategy functions */
#define	SV_EGENERIC 0x3000f /* Libspcs detected a nonspecific SV error */
#define	SV_ESTAT 0x30010 /* Error from stat() */
#define	SV_EKERNEL 0x30011 /* User access to this device is not allowed by StorageTek kernel software */
#define	SV_EAMODE 0x30012 /* Illegal access mode */
#define	SV_EGUARDVER 0x30013 /* Incompatible guard magic or version number */
#define	SV_ELOOKUP 0x30014 /* Unable to translate pathname to major/minor number */
#define	SV_ENOGCLIENT 0x30015 /* Unable to match guard client module to internal module id */
#define	SV_EGUSER 0x30016 /* Unable to create kernel only guard, device already user enabled */
#define	SV_ELYROPEN 0x30017 /* Error from layered driver open */
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
 * Copyright 2008 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
*/
/*
 * The SPCS subsystem numbers
 * TODO: derive the static finals in Spcs.java from these or visa versa!
 */

#define SPCS_SOLARIS 0
#define SPCS_SPCS 1
#define SPCS_DSW 2
#define SPCS_SV 3
#define SPCS_RDC 4
#define SPCS_SDBC 5
#define SPCS_MAX_SUBSYSTEM 5
