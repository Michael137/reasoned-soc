#ifndef DRIVER_PROF_SHARED_H_IN
#define DRIVER_PROF_SHARED_H_IN

struct fastrpc_ioctl_perf
{
	/* kernel performance data */
	uintptr_t data;
	uint32_t numkeys;
	uintptr_t keys;
};

struct hlist_node {
	struct hlist_node *next, **pprev;
};

struct fastrpc_perf {
        int64_t count;
        int64_t flush;
        int64_t map;
        int64_t copy;
        int64_t link;
        int64_t getargs;
        int64_t putargs;
        int64_t invargs;
        int64_t invoke;
        int64_t tid;
        struct hlist_node hn;
};

//struct fastrpc_file
//{
//	struct hlist_node hn;
//	spinlock_t hlock;
//	struct hlist_head maps;
//	struct hlist_head cached_bufs;
//	struct hlist_head remote_bufs;
//	struct fastrpc_ctx_lst clst;
//	struct fastrpc_session_ctx* sctx;
//	struct fastrpc_buf* init_mem;
//	struct fastrpc_session_ctx* secsctx;
//	uint32_t mode;
//	uint32_t profile;
//	int sessionid;
//	int tgid;
//	int cid;
//	int ssrcount;
//	int pd;
//	char* spdname;
//	int file_close;
//	int sharedcb;
//	struct fastrpc_apps* apps;
//	struct hlist_head perf;
//	struct dentry* debugfs_file;
//	struct mutex perf_mutex;
//	struct pm_qos_request pm_qos_req;
//	int qos_request;
//	struct mutex map_mutex;
//	struct mutex fl_map_mutex;
//	int refcount;
//	/* Identifies the device (MINOR_NUM_DEV / MINOR_NUM_SECURE_DEV) */
//	int dev_minor;
//	char* debug_buf;
//};

#define FASTRPC_IOCTL_GETPERF _IOWR( 'R', 9, struct fastrpc_ioctl_perf )

struct kgsl_perfcounter_query
{
	unsigned int groupid;
	unsigned int* countables;
	unsigned int count;
	unsigned int max_counters;
	unsigned int __pad[2];
};

#define KGSL_IOC_TYPE 0x09
#define IOCTL_KGSL_PERFCOUNTER_QUERY                                           \
	_IOWR( KGSL_IOC_TYPE, 0x3A, struct kgsl_perfcounter_query )

#endif // DRIVER_PROF_SHARED_H_IN
