#ifndef MINER_H
#define MINER_H

#include "config.h"

#ifdef __MINGW32__
#include <winsock2.h>
#endif

#if defined(USE_GIT_VERSION) && defined(GIT_VERSION)
#undef VERSION
#define VERSION GIT_VERSION
#endif

#ifdef BUILD_NUMBER
#define CGMINER_VERSION VERSION "-" BUILD_NUMBER
#else
#define CGMINER_VERSION VERSION
#endif

#include "algorithm.h"

#include <stdbool.h>
#include <stdint.h>
#include <sys/time.h>
#include <pthread.h>
#include <jansson.h>
#ifdef HAVE_LIBCURL
#include <curl/curl.h>
#else
typedef char CURL;
extern char *curly;
#define curl_easy_init(curl) (curly)
#define curl_easy_cleanup(curl) {}
#define curl_global_cleanup() {}
#define CURL_GLOBAL_ALL 0
#define curl_global_init(X) (0)
#endif
#include <sched.h>

#include "elist.h"
#include "uthash.h"
#include "logging.h"
#include "util.h"
#include <sys/types.h>
#ifndef WIN32
# include <sys/socket.h>
# include <netdb.h>
#endif

#ifdef __APPLE_CC__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

#ifdef STDC_HEADERS
# include <stdlib.h>
# include <stddef.h>
#else
# ifdef HAVE_STDLIB_H
#  include <stdlib.h>
# endif
#endif
#ifdef HAVE_ALLOCA_H
# include <alloca.h>
#elif defined __GNUC__
# ifndef WIN32
#  define alloca __builtin_alloca
# else
#  include <malloc.h>
# endif
#elif defined _AIX
# define alloca __alloca
#elif defined _MSC_VER
# include <malloc.h>
# define alloca _alloca
#else
# ifndef HAVE_ALLOCA
#  ifdef  __cplusplus
extern "C"
#  endif
void *alloca (size_t);
# endif
#endif

#ifdef __MINGW32__
#include <windows.h>
#include <io.h>
static inline int fsync (int fd)
{
  return (FlushFileBuffers ((HANDLE) _get_osfhandle (fd))) ? 0 : -1;
}

#ifndef EWOULDBLOCK
# define EWOULDBLOCK EAGAIN
#endif

#ifndef MSG_DONTWAIT
# define MSG_DONTWAIT 0x1000000
#endif
#endif /* __MINGW32__ */

#if defined (__unix__) && !defined(UNIX)
# define UNIX
#endif
#if defined (__linux__) && !defined(LINUX)
# define LINUX
#endif

#ifdef WIN32
  #ifndef timersub
    #define timersub(a, b, result)                     \
    do {                                               \
      (result)->tv_sec = (a)->tv_sec - (b)->tv_sec;    \
      (result)->tv_usec = (a)->tv_usec - (b)->tv_usec; \
      if ((result)->tv_usec < 0) {                     \
        --(result)->tv_sec;                            \
        (result)->tv_usec += 1000000;                  \
      }                                                \
    } while (0)
  #endif
 #ifndef timeradd
 # define timeradd(a, b, result)            \
   do {                   \
    (result)->tv_sec = (a)->tv_sec + (b)->tv_sec;       \
    (result)->tv_usec = (a)->tv_usec + (b)->tv_usec;        \
    if ((result)->tv_usec >= 1000000)           \
      {                   \
  ++(result)->tv_sec;             \
  (result)->tv_usec -= 1000000;           \
      }                   \
   } while (0)
 #endif
#endif


#ifdef HAVE_ADL
 #include "ADL_SDK/adl_sdk.h"
#endif

#include <ccan/opt/opt.h>

#if (!defined(WIN32) && ((__GNUC__ > 4) || (__GNUC__ == 4 && __GNUC_MINOR__ >= 3))) \
    || (defined(WIN32) && ((__GNUC__ > 4) || (__GNUC__ == 4 && __GNUC_MINOR__ >= 7)))
#ifndef bswap_16
 #define bswap_16 __builtin_bswap16
 #define bswap_32 __builtin_bswap32
 #define bswap_64 __builtin_bswap64
#endif
#else
#if HAVE_BYTESWAP_H
#include <byteswap.h>
#elif defined(USE_SYS_ENDIAN_H)
#include <sys/endian.h>
#elif defined(__APPLE__)
#include <libkern/OSByteOrder.h>
#define bswap_16 OSSwapInt16
#define bswap_32 OSSwapInt32
#define bswap_64 OSSwapInt64
#else
#define bswap_16(value)  \
  ((((value) & 0xff) << 8) | ((value) >> 8))

#define bswap_32(value) \
  (((uint32_t)bswap_16((uint16_t)((value) & 0xffff)) << 16) | \
  (uint32_t)bswap_16((uint16_t)((value) >> 16)))

#define bswap_64(value) \
  (((uint64_t)bswap_32((uint32_t)((value) & 0xffffffff)) \
      << 32) | \
  (uint64_t)bswap_32((uint32_t)((value) >> 32)))
#endif
#endif /* !defined(__GLXBYTEORDER_H__) */

/* This assumes htobe32 is a macro in endian.h, and if it doesn't exist, then
 * htobe64 also won't exist */
#ifndef htobe32
# if __BYTE_ORDER == __LITTLE_ENDIAN
#  define htole16(x) (x)
#  define htole32(x) (x)
#  define htole64(x) (x)
#  define le32toh(x) (x)
#  define le64toh(x) (x)
#  define be32toh(x) bswap_32(x)
#  define be64toh(x) bswap_64(x)
#  define htobe32(x) bswap_32(x)
#  define htobe64(x) bswap_64(x)
# elif __BYTE_ORDER == __BIG_ENDIAN
#  define htole16(x) bswap_16(x)
#  define htole32(x) bswap_32(x)
#  define le32toh(x) bswap_32(x)
#  define le64toh(x) bswap_64(x)
#  define htole64(x) bswap_64(x)
#  define be32toh(x) (x)
#  define be64toh(x) (x)
#  define htobe32(x) (x)
#  define htobe64(x) (x)
#else
#error UNKNOWN BYTE ORDER
#endif
#endif

#undef unlikely
#undef likely
#if defined(__GNUC__) && (__GNUC__ > 2) && defined(__OPTIMIZE__)
#define unlikely(expr) (__builtin_expect(!!(expr), 0))
#define likely(expr) (__builtin_expect(!!(expr), 1))
#else
#define unlikely(expr) (expr)
#define likely(expr) (expr)
#endif
#define __maybe_unused    __attribute__((unused))

#define uninitialised_var(x) x = x

#if defined(__i386__)
#define WANT_CRYPTOPP_ASM32
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#endif

#ifdef MIPSEB
#ifndef roundl
#define roundl(x)   (long double)((long long)((x==0)?0.0:((x)+((x)>0)?0.5:-0.5)))
#endif
#endif

/* No semtimedop on apple so ignore timeout till we implement one */
#ifdef __APPLE__
#define semtimedop(SEM, SOPS, VAL, TIMEOUT) semop(SEM, SOPS, VAL)
#endif

#ifndef MIN
#define MIN(x, y) ((x) > (y) ? (y) : (x))
#endif
#ifndef MAX
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#endif

/* Adding a device here will update all macros in the code that use
 * the *_PARSE_COMMANDS macros for each listed driver.
 */
#define DRIVER_PARSE_COMMANDS(DRIVER_ADD_COMMAND) \
  DRIVER_ADD_COMMAND(opencl)

#define DRIVER_ENUM(X) DRIVER_##X,
#define DRIVER_PROTOTYPE(X) struct device_drv X##_drv;

/* Create drv_driver enum from DRIVER_PARSE_COMMANDS macro */
enum drv_driver {
  DRIVER_PARSE_COMMANDS(DRIVER_ENUM)
  DRIVER_MAX
};

/* Use DRIVER_PARSE_COMMANDS to generate extern device_drv prototypes */
#ifndef _MSC_VER
DRIVER_PARSE_COMMANDS(DRIVER_PROTOTYPE)
#endif

// helper to check for empty or NULL strings
#ifndef empty_string
  #define empty_string(str) ((str && str[0] != '\0')?0:1)
#endif
// helper to safely compare strings: 0 = equal, 1 = not equal
#ifndef safe_cmp
  #define safe_cmp(val1, val2) ((!empty_string(val1) && !empty_string(val2))?((strcasecmp(val1, val2) == 0)?0:1):((empty_string(val1) && empty_string(val2))?0:1))
#endif
// helper to pick pool settings or default profile settings
#ifndef pool_cmp
  #define pool_cmp(val1, val2) (((val1 && val2 && strcasecmp(val1, val2) == 0) || empty_string(val1))?1:0)
#endif
// helper safely output a string
#ifndef isnull
  #define isnull(str, default_str) ((str == NULL)?default_str:str)
#endif
// helper to get boolean value out of string
#ifndef strtobool
  #define strtobool(str) ((str && (!strcasecmp(str, "true") || !strcasecmp(str, "yes") || !strcasecmp(str, "1")))?true:false)
#endif

extern int opt_remoteconf_retry;
extern int opt_remoteconf_wait;
extern bool opt_remoteconf_usecache;

extern bool opt_benchmark;
extern uint8_t opt_benchmark_seq[17];


enum alive {
  LIFE_WELL,
  LIFE_SICK,
  LIFE_DEAD,
  LIFE_NOSTART,
  LIFE_INIT,
};

enum pool_strategy {
  POOL_FAILOVER,
  POOL_ROUNDROBIN,
  POOL_ROTATE,
  POOL_LOADBALANCE,
  POOL_BALANCE,
};

typedef void (*_Voidfp)(void*);

#define TOP_STRATEGY (POOL_BALANCE)

struct strategies {
  const char *s;
};

extern enum pool_strategy pool_strategy;
extern struct strategies strategies[];

struct cgpu_info;

struct gpu_adl {
  int iAdapterIndex;
  int lpAdapterID;
  int iBusNumber;
  char strAdapterName[256];

#ifdef HAVE_ADL
  ADLTemperature lpTemperature;
  ADLPMActivity lpActivity;
  ADLODParameters lpOdParameters;
  ADLODPerformanceLevels *DefPerfLev;
  ADLFanSpeedInfo lpFanSpeedInfo;
  ADLFanSpeedValue lpFanSpeedValue;
  ADLFanSpeedValue DefFanSpeedValue;
#endif

  bool def_fan_valid;

  int iEngineClock;
  int iMemoryClock;
  int iVddc;
  int iPercentage;

  bool autofan;
  bool autoengine;
  bool managed; /* Were the values ever changed on this card */

  int lastengine;
  int lasttemp;
  int targetfan;
  int targettemp;
  int overtemp;
  int minspeed;
  int maxspeed;

  int gpu;
  bool has_fanspeed;
  struct gpu_adl *twin;
};

/***********************************
 * Switcher stuff
 ****************************************/
#ifndef opt_isset
  #define opt_isset(opt, val) (((opt & val) == val)?1:0)
#endif
#ifndef get_pool_setting
  #define get_pool_setting(val, default_val) ((!empty_string(val))?val:((!empty_string(default_val))?default_val:""))
#endif

enum switcher_mode {
  SWITCH_OFF,
  SWITCH_ALGO,
  SWITCH_POOL
};

extern int opt_switchmode;

enum switcher_options {
  SWITCHER_APPLY_NONE = 0x00,
  SWITCHER_APPLY_ALGO = 0x01,
  SWITCHER_APPLY_DEVICE = 0x02,
  SWITCHER_APPLY_GT = 0x04,
  SWITCHER_APPLY_LG = 0x08,
  SWITCHER_APPLY_RAWINT = 0x10,
  SWITCHER_APPLY_XINT = 0x20,
  SWITCHER_APPLY_INT = 0x40,
  SWITCHER_APPLY_INT8 = 0x80,
  SWITCHER_APPLY_SHADER = 0x100,
  SWITCHER_APPLY_TC = 0x200,
  SWITCHER_APPLY_WORKSIZE = 0x400,
  SWITCHER_APPLY_GPU_ENGINE = 0x800,
  SWITCHER_APPLY_GPU_MEMCLOCK = 0x1000,
  SWITCHER_APPLY_GPU_FAN = 0x2000,
  SWITCHER_APPLY_GPU_POWERTUNE = 0x4000,
  SWITCHER_APPLY_GPU_VDDC = 0x8000,
  SWITCHER_SOFT_RESET = 0x4000000,
  SWITCHER_HARD_RESET = 0x8000000
};

enum gpu_adl_options {
  APPLY_ENGINE = 1,
  APPLY_MEMCLOCK = 2,
  APPLY_FANSPEED = 4,
  APPLY_POWERTUNE = 8,
  APPLY_VDDC = 16
};

extern void blank_get_statline_before(char *buf, size_t bufsiz, struct cgpu_info __maybe_unused *cgpu);

struct api_data;
struct thr_info;
struct work;

struct device_drv {
  enum drv_driver drv_id;

  char *dname;
  char *name;

  // DRV-global functions
  void (*drv_detect)(void);

  // Device-specific functions
  void (*reinit_device)(struct cgpu_info *);
  void (*get_statline_before)(char *, size_t, struct cgpu_info *);
  void (*get_statline)(char *, size_t, struct cgpu_info *);
  struct api_data *(*get_api_stats)(struct cgpu_info *);
  bool (*get_stats)(struct cgpu_info *);
  void (*identify_device)(struct cgpu_info *); // e.g. to flash a led
  char *(*set_device)(struct cgpu_info *, char *option, char *setting, char *replybuf);

  // Thread-specific functions
  bool (*thread_prepare)(struct thr_info *);
  uint64_t (*can_limit_work)(struct thr_info *);
  bool (*thread_init)(struct thr_info *);
  bool (*prepare_work)(struct thr_info *, struct work *);

  /* Which hash work loop this driver uses. */
  void (*hash_work)(struct thr_info *);
  /* Two variants depending on whether the device divides work up into
   * small pieces or works with whole work items and may or may not have
   * a queue of its own. */
  int64_t (*scanhash)(struct thr_info *, struct work *, int64_t);
  int64_t (*scanwork)(struct thr_info *);

  /* Used to extract work from the hash table of queued work and tell
   * the main loop that it should not add any further work to the table.
   */
  bool (*queue_full)(struct cgpu_info *);
  /* Tell the driver of a block change */
  void (*flush_work)(struct cgpu_info *);
  /* Tell the driver of an updated work template for eg. stratum */
  void (*update_work)(struct cgpu_info *);

  void (*hw_error)(struct thr_info *);
  void (*thread_shutdown)(struct thr_info *);
  void (*thread_enable)(struct thr_info *);

  /* What should be zeroed in this device when global zero stats is sent */
  void (*zero_stats)(struct cgpu_info *);

  // Does it need to be free()d?
  bool copy;

  /* Highest target diff the device supports */
  double max_diff;
  double working_diff;
};

enum dev_enable {
  DEV_ENABLED,
  DEV_DISABLED,
  DEV_RECOVER,
};

enum dev_reason {
  REASON_THREAD_FAIL_INIT,
  REASON_THREAD_ZERO_HASH,
  REASON_THREAD_FAIL_QUEUE,
  REASON_DEV_SICK_IDLE_60,
  REASON_DEV_DEAD_IDLE_600,
  REASON_DEV_NOSTART,
  REASON_DEV_OVER_HEAT,
  REASON_DEV_THERMAL_CUTOFF,
  REASON_DEV_COMMS_ERROR,
  REASON_DEV_THROTTLE,
};

#define REASON_NONE     "None"
#define REASON_THREAD_FAIL_INIT_STR "Thread failed to init"
#define REASON_THREAD_ZERO_HASH_STR "Thread got zero hashes"
#define REASON_THREAD_FAIL_QUEUE_STR  "Thread failed to queue work"
#define REASON_DEV_SICK_IDLE_60_STR "Device idle for 60s"
#define REASON_DEV_DEAD_IDLE_600_STR  "Device dead - idle for 600s"
#define REASON_DEV_NOSTART_STR    "Device failed to start"
#define REASON_DEV_OVER_HEAT_STR  "Device over heated"
#define REASON_DEV_THERMAL_CUTOFF_STR "Device reached thermal cutoff"
#define REASON_DEV_COMMS_ERROR_STR  "Device comms error"
#define REASON_DEV_THROTTLE_STR   "Device throttle"
#define REASON_UNKNOWN_STR    "Unknown reason - code bug"

#define MIN_SEC_UNSET 99999999

struct sgminer_stats {
  uint32_t getwork_calls;
  struct timeval getwork_wait;
  struct timeval getwork_wait_max;
  struct timeval getwork_wait_min;
};

// Just the actual network getworks to the pool
struct sgminer_pool_stats {
  uint32_t getwork_calls;
  uint32_t getwork_attempts;
  struct timeval getwork_wait;
  struct timeval getwork_wait_max;
  struct timeval getwork_wait_min;
  double getwork_wait_rolling;
  bool hadrolltime;
  bool canroll;
  bool hadexpire;
  uint32_t rolltime;
  double min_diff;
  double max_diff;
  double last_diff;
  uint32_t min_diff_count;
  uint32_t max_diff_count;
  uint64_t times_sent;
  uint64_t bytes_sent;
  uint64_t net_bytes_sent;
  uint64_t times_received;
  uint64_t bytes_received;
  uint64_t net_bytes_received;
};

typedef struct _gpu_sysfs_info {
	char *HWMonPath;
	uint8_t *pptable;	  uint32_t MinFanSpeed;
	uint8_t *default_pptable;	  uint32_t MaxFanSpeed;
	size_t pptable_size;	  uint32_t OverHeatTemp;
	uint32_t min_fanspeed;	  uint32_t TargetTemp;
	uint32_t max_fanspeed;	  float TgtFanSpeed;
	uint32_t overheat_temp;	  float LastFanSpeed;
	uint32_t target_temp;	  float LastTemp;
} gpu_sysfs_info;

struct _eth_dag_t;
typedef struct _eth_cache_t {
  uint8_t seed_hash[32];
  uint8_t *dag_cache;
  struct _eth_dag_t **dags;
  uint32_t current_epoch;
  uint32_t nDevs;
  bool disabled;
} eth_cache_t;

typedef struct _eth_dag_t {
  cglock_t lock;
  cl_mem dag_buffer;
  struct pool *pool;
  uint32_t current_epoch;
  uint32_t max_epoch;
} eth_dag_t;

struct cgpu_info {
  int sgminer_id;
  struct device_drv *drv;
  int device_id;
  char *name;  /* GPU family codename. */
  char *device_path;
  void *device_data;

  enum dev_enable deven;
  int accepted;
  int rejected;
  int hw_errors;
  double rolling;
  double total_mhashes;
  double utility;
  enum alive status;
  char init[40];
  struct timeval last_message_tv;

  int threads;
  struct thr_info **thr;

  int64_t max_hashes;

  bool mapped;
  int virtual_gpu;
  int virtual_adl;

  int intensity;
  int xintensity;
  int rawintensity;
  bool dynamic;

  cl_uint vwidth;
  size_t work_size;
  cl_ulong max_alloc;
  algorithm_t algorithm;

  int opt_lg, lookup_gap;
  size_t opt_tc, thread_concurrency;
  size_t shaders;
  struct timeval tv_gpustart;
  int intervals;

  bool new_work;

  float temp;
  int cutofftemp;

  bool has_sysfs_hwcontrols;
  bool has_adl;
  struct gpu_adl adl;
  gpu_sysfs_info sysfs_info;

  int gpu_engine;
  int min_engine;
  int gpu_fan;
  int min_fan;
  int gpu_memclock;
  int gpu_memdiff;
  int gpu_powertune;
  float gpu_vddc;

  double diff1;
  double diff_accepted;
  double diff_rejected;
  int last_share_pool;
  time_t last_share_pool_time;
  double last_share_diff;
  time_t last_device_valid_work;

  time_t device_last_well;
  time_t device_last_not_well;
  enum dev_reason device_not_well_reason;
  int thread_fail_init_count;
  int thread_zero_hash_count;
  int thread_fail_queue_count;
  int dev_sick_idle_60_count;
  int dev_dead_idle_600_count;
  int dev_nostart_count;
  int dev_over_heat_count;  // It's a warning but worth knowing
  int dev_thermal_cutoff_count;
  int dev_comms_error_count;
  int dev_throttle_count;

  struct sgminer_stats sgminer_stats;
  eth_dag_t eth_dag;

  bool shutdown;

  struct timeval dev_start_tv;
};

extern bool add_cgpu(struct cgpu_info*);

struct thread_q {
  struct list_head  q;

  bool frozen;

  pthread_mutex_t   mutex;
  pthread_cond_t    cond;
};

struct thr_info {
  int   id;
  int   device_thread;

  pthread_t pth;
  cgsem_t   sem;
  struct thread_q *q;
  struct cgpu_info *cgpu;
  void *cgpu_data;
  int pool_no;
  struct timeval last;
  struct timeval sick;

  bool  pause;
  bool  paused;
  bool  getwork;
  double  rolling;

  bool  work_restart;
  bool  work_update;
};

struct string_elist {
  char *string;
  bool free_me;

  struct list_head list;
};

static inline uint32_t swab32(uint32_t v)
{
  return bswap_32(v);
}

static inline void swap256(void *dest_p, const void *src_p)
{
  uint32_t *dest = (uint32_t *)dest_p;
  const uint32_t *src = (uint32_t *)src_p;

  dest[0] = src[7];
  dest[1] = src[6];
  dest[2] = src[5];
  dest[3] = src[4];
  dest[4] = src[3];
  dest[5] = src[2];
  dest[6] = src[1];
  dest[7] = src[0];
}

static inline void swab256(void *dest_p, const void *src_p)
{
  uint32_t *dest = (uint32_t *)dest_p;
  const uint32_t *src = (uint32_t *)src_p;

  dest[0] = swab32(src[7]);
  dest[1] = swab32(src[6]);
  dest[2] = swab32(src[5]);
  dest[3] = swab32(src[4]);
  dest[4] = swab32(src[3]);
  dest[5] = swab32(src[2]);
  dest[6] = swab32(src[1]);
  dest[7] = swab32(src[0]);
}

static inline void flip32(void *dest_p, const void *src_p)
{
  uint32_t *dest = (uint32_t *)dest_p;
  const uint32_t *src = (uint32_t *)src_p;
  int i;

  for (i = 0; i < 8; i++)
    dest[i] = swab32(src[i]);
}

static inline void flip64(void *dest_p, const void *src_p)
{
  uint32_t *dest = (uint32_t *)dest_p;
  const uint32_t *src = (uint32_t *)src_p;
  int i;

  for (i = 0; i < 16; i++)
    dest[i] = swab32(src[i]);
}

static inline void flip80(void *dest_p, const void *src_p)
{
  uint32_t *dest = (uint32_t *)dest_p;
  const uint32_t *src = (uint32_t *)src_p;
  int i;

  for (i = 0; i < 20; i++)
    dest[i] = swab32(src[i]);
}

static inline void flip128(void *dest_p, const void *src_p)
{
  uint32_t *dest = (uint32_t *)dest_p;
  const uint32_t *src = (uint32_t *)src_p;
  int i;

  for (i = 0; i < 32; i++)
    dest[i] = swab32(src[i]);
}

static inline void flip168(void *dest_p, const void *src_p)
{
  uint32_t *dest = (uint32_t *)dest_p;
  const uint32_t *src = (uint32_t *)src_p;
  int i;

  for (i = 0; i < 42; i++)
    dest[i] = swab32(src[i]);
}


/* For flipping to the correct endianness if necessary */
#if defined(__BIG_ENDIAN__) || defined(MIPSEB)
static inline void endian_flip32(void *dest_p, const void *src_p)
{
  flip32(dest_p, src_p);
}

static inline void endian_flip128(void *dest_p, const void *src_p)
{
  flip128(dest_p, src_p);
}
static inline void endian_flip168(void *dest_p, const void *src_p)
{
  flip168(dest_p, src_p);
}

#else
static inline void
endian_flip32(void __maybe_unused *dest_p, const void __maybe_unused *src_p)
{
}

static inline void
endian_flip128(void __maybe_unused *dest_p, const void __maybe_unused *src_p)
{
}
static inline void
endian_flip168(void __maybe_unused *dest_p, const void __maybe_unused *src_p)
{
}
#endif


extern double cgpu_runtime(struct cgpu_info *cgpu);
extern void _quit(int status);

/*
 * Set this to non-zero to enable lock tracking
 * Use the API lockstats command to see the locking status on stderr
 *  i.e. in your log file if you 2> log.log - but not on the screen
 * API lockstats is privilidged but will always exist and will return
 *  success if LOCK_TRACKING is enabled and warning if disabled
 * In production code, this should never be enabled since it will slow down all locking
 * So, e.g. use it to track down a deadlock - after a reproducable deadlock occurs
 * ... Of course if the API code itself deadlocks, it wont help :)
 */
#define LOCK_TRACKING 0

#if LOCK_TRACKING
enum cglock_typ {
  CGLOCK_MUTEX,
  CGLOCK_RW,
  CGLOCK_UNKNOWN
};

extern uint64_t api_getlock(void *lock, const char *file, const char *func, const int line);
extern void api_gotlock(uint64_t id, void *lock, const char *file, const char *func, const int line);
extern uint64_t api_trylock(void *lock, const char *file, const char *func, const int line);
extern void api_didlock(uint64_t id, int ret, void *lock, const char *file, const char *func, const int line);
extern void api_gunlock(void *lock, const char *file, const char *func, const int line);
extern void api_initlock(void *lock, enum cglock_typ typ, const char *file, const char *func, const int line);

#define GETLOCK(_lock, _file, _func, _line) uint64_t _id1 = api_getlock((void *)(_lock), _file, _func, _line)
#define GOTLOCK(_lock, _file, _func, _line) api_gotlock(_id1, (void *)(_lock), _file, _func, _line)
#define TRYLOCK(_lock, _file, _func, _line) uint64_t _id2 = api_trylock((void *)(_lock), _file, _func, _line)
#define DIDLOCK(_ret, _lock, _file, _func, _line) api_didlock(_id2, _ret, (void *)(_lock), _file, _func, _line)
#define GUNLOCK(_lock, _file, _func, _line) api_gunlock((void *)(_lock), _file, _func, _line)
#define INITLOCK(_lock, _typ, _file, _func, _line) api_initlock((void *)(_lock), _typ, _file, _func, _line)
#else
#define GETLOCK(_lock, _file, _func, _line)
#define GOTLOCK(_lock, _file, _func, _line)
#define TRYLOCK(_lock, _file, _func, _line)
#define DIDLOCK(_ret, _lock, _file, _func, _line)
#define GUNLOCK(_lock, _file, _func, _line)
#define INITLOCK(_typ, _lock, _file, _func, _line)
#endif

#define mutex_lock(_lock) _mutex_lock(_lock, __FILE__, __func__, __LINE__)
#define mutex_unlock_noyield(_lock) _mutex_unlock_noyield(_lock, __FILE__, __func__, __LINE__)
#define mutex_unlock(_lock) _mutex_unlock(_lock, __FILE__, __func__, __LINE__)
#define mutex_trylock(_lock) _mutex_trylock(_lock, __FILE__, __func__, __LINE__)
#define wr_lock(_lock) _wr_lock(_lock, __FILE__, __func__, __LINE__)
#define wr_trylock(_lock) _wr_trylock(_lock, __FILE__, __func__, __LINE__)
#define rd_lock(_lock) _rd_lock(_lock, __FILE__, __func__, __LINE__)
#define rw_unlock(_lock) _rw_unlock(_lock, __FILE__, __func__, __LINE__)
#define rd_unlock_noyield(_lock) _rd_unlock_noyield(_lock, __FILE__, __func__, __LINE__)
#define wr_unlock_noyield(_lock) _wr_unlock_noyield(_lock, __FILE__, __func__, __LINE__)
#define rd_unlock(_lock) _rd_unlock(_lock, __FILE__, __func__, __LINE__)
#define wr_unlock(_lock) _wr_unlock(_lock, __FILE__, __func__, __LINE__)
#define mutex_init(_lock) _mutex_init(_lock, __FILE__, __func__, __LINE__)
#define rwlock_init(_lock) _rwlock_init(_lock, __FILE__, __func__, __LINE__)
#define cglock_init(_lock) _cglock_init(_lock, __FILE__, __func__, __LINE__)
#define cg_rlock(_lock) _cg_rlock(_lock, __FILE__, __func__, __LINE__)
#define cg_ilock(_lock) _cg_ilock(_lock, __FILE__, __func__, __LINE__)
#define cg_iunlock(_lock) _cg_iunlock(_lock, __FILE__, __func__, __LINE__)
#define cg_ulock(_lock) _cg_ulock(_lock, __FILE__, __func__, __LINE__)
#define cg_wlock(_lock) _cg_wlock(_lock, __FILE__, __func__, __LINE__)
#define cg_dwlock(_lock) _cg_dwlock(_lock, __FILE__, __func__, __LINE__)
#define cg_dwilock(_lock) _cg_dwilock(_lock, __FILE__, __func__, __LINE__)
#define cg_dlock(_lock) _cg_dlock(_lock, __FILE__, __func__, __LINE__)
#define cg_runlock(_lock) _cg_runlock(_lock, __FILE__, __func__, __LINE__)
#define cg_ruwlock(_lock) _cg_ruwlock(_lock, __FILE__, __func__, __LINE__)
#define cg_wunlock(_lock) _cg_wunlock(_lock, __FILE__, __func__, __LINE__)

static inline void _mutex_lock(pthread_mutex_t *lock, const char *file, const char *func, const int line)
{
  GETLOCK(lock, file, func, line);
  if (unlikely(pthread_mutex_lock(lock)))
    quitfrom(1, file, func, line, "WTF MUTEX ERROR ON LOCK! errno=%d", errno);
  GOTLOCK(lock, file, func, line);
}

static inline void _mutex_unlock_noyield(pthread_mutex_t *lock, const char *file, const char *func, const int line)
{
  if (unlikely(pthread_mutex_unlock(lock)))
    quitfrom(1, file, func, line, "WTF MUTEX ERROR ON UNLOCK! errno=%d", errno);
  GUNLOCK(lock, file, func, line);
}

static inline void _mutex_unlock(pthread_mutex_t *lock, const char *file, const char *func, const int line)
{
  _mutex_unlock_noyield(lock, file, func, line);
  sched_yield();
}

static inline int _mutex_trylock(pthread_mutex_t *lock, __maybe_unused const char *file, __maybe_unused const char *func, __maybe_unused const int line)
{
  TRYLOCK(lock, file, func, line);
  int ret = pthread_mutex_trylock(lock);
  DIDLOCK(ret, lock, file, func, line);
  return ret;
}

static inline void _wr_lock(pthread_rwlock_t *lock, const char *file, const char *func, const int line)
{
  GETLOCK(lock, file, func, line);
  if (unlikely(pthread_rwlock_wrlock(lock)))
    quitfrom(1, file, func, line, "WTF WRLOCK ERROR ON LOCK! errno=%d", errno);
  GOTLOCK(lock, file, func, line);
}

static inline int _wr_trylock(pthread_rwlock_t *lock, __maybe_unused const char *file, __maybe_unused const char *func, __maybe_unused const int line)
{
  TRYLOCK(lock, file, func, line);
  int ret = pthread_rwlock_trywrlock(lock);
  DIDLOCK(ret, lock, file, func, line);
  return ret;
}

static inline void _rd_lock(pthread_rwlock_t *lock, const char *file, const char *func, const int line)
{
  GETLOCK(lock, file, func, line);
  if (unlikely(pthread_rwlock_rdlock(lock)))
    quitfrom(1, file, func, line, "WTF RDLOCK ERROR ON LOCK! errno=%d", errno);
  GOTLOCK(lock, file, func, line);
}

static inline void _rw_unlock(pthread_rwlock_t *lock, const char *file, const char *func, const int line)
{
  if (unlikely(pthread_rwlock_unlock(lock)))
    quitfrom(1, file, func, line, "WTF RWLOCK ERROR ON UNLOCK! errno=%d", errno);
  GUNLOCK(lock, file, func, line);
}

static inline void _rd_unlock_noyield(pthread_rwlock_t *lock, const char *file, const char *func, const int line)
{
  _rw_unlock(lock, file, func, line);
}

static inline void _wr_unlock_noyield(pthread_rwlock_t *lock, const char *file, const char *func, const int line)
{
  _rw_unlock(lock, file, func, line);
}

static inline void _rd_unlock(pthread_rwlock_t *lock, const char *file, const char *func, const int line)
{
  _rw_unlock(lock, file, func, line);
  sched_yield();
}

static inline void _wr_unlock(pthread_rwlock_t *lock, const char *file, const char *func, const int line)
{
  _rw_unlock(lock, file, func, line);
  sched_yield();
}

static inline void _mutex_init(pthread_mutex_t *lock, const char *file, const char *func, const int line)
{
  if (unlikely(pthread_mutex_init(lock, NULL)))
    quitfrom(1, file, func, line, "Failed to pthread_mutex_init errno=%d", errno);
  INITLOCK(lock, CGLOCK_MUTEX, file, func, line);
}

static inline void mutex_destroy(pthread_mutex_t *lock)
{
  /* Ignore return code. This only invalidates the mutex on linux but
   * releases resources on windows. */
  pthread_mutex_destroy(lock);
}

static inline void _rwlock_init(pthread_rwlock_t *lock, const char *file, const char *func, const int line)
{
  if (unlikely(pthread_rwlock_init(lock, NULL)))
    quitfrom(1, file, func, line, "Failed to pthread_rwlock_init errno=%d", errno);
  INITLOCK(lock, CGLOCK_RW, file, func, line);
}

static inline void rwlock_destroy(pthread_rwlock_t *lock)
{
  pthread_rwlock_destroy(lock);
}

static inline void _cglock_init(cglock_t *lock, const char *file, const char *func, const int line)
{
  _mutex_init(&lock->mutex, file, func, line);
  _rwlock_init(&lock->rwlock, file, func, line);
}

static inline void cglock_destroy(cglock_t *lock)
{
  rwlock_destroy(&lock->rwlock);
  mutex_destroy(&lock->mutex);
}

/* Read lock variant of cglock. Cannot be promoted. */
static inline void _cg_rlock(cglock_t *lock, const char *file, const char *func, const int line)
{
  _mutex_lock(&lock->mutex, file, func, line);
  _rd_lock(&lock->rwlock, file, func, line);
  _mutex_unlock_noyield(&lock->mutex, file, func, line);
}

/* Intermediate variant of cglock - behaves as a read lock but can be promoted
 * to a write lock or demoted to read lock. */
static inline void _cg_ilock(cglock_t *lock, const char *file, const char *func, const int line)
{
  _mutex_lock(&lock->mutex, file, func, line);
}

/* Unlock intermediate lock - behaves like a mutex. */
static inline void _cg_iunlock(cglock_t *lock, const char *file, const char *func, const int line)
{
  _mutex_unlock_noyield(&lock->mutex, file, func, line);
}

/* Upgrade intermediate variant to a write lock */
static inline void _cg_ulock(cglock_t *lock, const char *file, const char *func, const int line)
{
  _wr_lock(&lock->rwlock, file, func, line);
}

/* Write lock variant of cglock */
static inline void _cg_wlock(cglock_t *lock, const char *file, const char *func, const int line)
{
  _mutex_lock(&lock->mutex, file, func, line);
  _wr_lock(&lock->rwlock, file, func, line);
}

/* Downgrade write variant to a read lock */
static inline void _cg_dwlock(cglock_t *lock, const char *file, const char *func, const int line)
{
  _wr_unlock_noyield(&lock->rwlock, file, func, line);
  _rd_lock(&lock->rwlock, file, func, line);
  _mutex_unlock_noyield(&lock->mutex, file, func, line);
}

/* Demote a write variant to an intermediate variant */
static inline void _cg_dwilock(cglock_t *lock, const char *file, const char *func, const int line)
{
  _wr_unlock(&lock->rwlock, file, func, line);
}

/* Downgrade intermediate variant to a read lock */
static inline void _cg_dlock(cglock_t *lock, const char *file, const char *func, const int line)
{
  _rd_lock(&lock->rwlock, file, func, line);
  _mutex_unlock_noyield(&lock->mutex, file, func, line);
}

static inline void _cg_runlock(cglock_t *lock, const char *file, const char *func, const int line)
{
  _rd_unlock(&lock->rwlock, file, func, line);
}

/* This drops the read lock and grabs a write lock. It does NOT protect data
 * between the two locks! */
static inline void _cg_ruwlock(cglock_t *lock, const char *file, const char *func, const int line)
{
  _rd_unlock_noyield(&lock->rwlock, file, func, line);
  _cg_wlock(lock, file, func, line);
}

static inline void _cg_wunlock(cglock_t *lock, const char *file, const char *func, const int line)
{
  _wr_unlock_noyield(&lock->rwlock, file, func, line);
  _mutex_unlock(&lock->mutex, file, func, line);
}

/*
* Encode a length len/4 vector of (uint32_t) into a length len vector of
* (unsigned char) in big-endian form.  Assumes len is a multiple of 4.
*/
static inline void be32enc_vect(uint32_t *dst, const uint32_t *src, uint32_t len)
{
	uint32_t i;

	for (i = 0; i < len; i++)
		dst[i] = htobe32(src[i]);
}

struct pool;

#define API_MCAST_CODE "FTW"
#define API_MCAST_ADDR "224.0.0.75"

extern bool opt_work_update;
extern bool opt_protocol;
extern bool have_longpoll;
extern char *opt_kernel_path;
extern char *opt_socks_proxy;

#if defined(unix) || defined(__APPLE__)
    extern char *opt_stderr_cmd;
#endif // defined(unix)

struct schedtime {
  bool enable;
  struct tm tm;
};

extern struct schedtime schedstart;
extern struct schedtime schedstop;

extern char *sgminer_path;
extern int opt_shares;
extern bool opt_fail_only;
extern int opt_fail_switch_delay;
extern int opt_watchpool_refresh;
extern bool opt_autofan;
extern bool opt_autoengine;
extern bool use_curses;
extern char *opt_api_allow;
extern bool opt_api_mcast;
extern char *opt_api_mcast_addr;
extern char *opt_api_mcast_code;
extern char *opt_api_mcast_des;
extern int opt_api_mcast_port;
extern char *opt_api_groups;
extern char *opt_api_description;
extern int opt_api_port;
extern bool opt_api_listen;
extern bool opt_api_network;
extern bool opt_delaynet;
extern time_t last_getwork;
extern bool opt_disable_client_reconnect;
extern bool opt_restart;
extern bool opt_worktime;
extern int swork_id;
extern int opt_tcp_keepalive;
extern bool opt_incognito;

extern float (*gpu_temp)(int);
extern int (*gpu_engineclock)(int);
extern int (*gpu_memclock)(int);
extern float (*gpu_vddc)(int);
extern int (*gpu_activity)(int);
extern int (*gpu_fanspeed)(int);
extern float (*gpu_fanpercent)(int);
extern int (*set_powertune)(int, int);
extern int (*set_fanspeed)(int, float);
extern int (*set_vddc)(int, float);
extern int (*set_engineclock)(int, int);
extern int (*set_memoryclock)(int, int);
extern bool (*gpu_stats)(int, float *, int *, int *, float *, int *, int *, int *, int *);
extern void (*gpu_autotune) (int, enum dev_enable *);

// Xn Algorithm options
extern int opt_keccak_unroll;
extern bool opt_blake_compact;
extern bool opt_luffa_parallel;
extern int opt_hamsi_expand_big;
extern bool opt_hamsi_short;

#if LOCK_TRACKING
extern pthread_mutex_t lockstat_lock;
#endif

extern pthread_rwlock_t netacc_lock;

extern const uint32_t sha256_init_state[];
#ifdef HAVE_LIBCURL
extern json_t *json_rpc_call(CURL *curl, char *curl_err_str, const char *url, const char *userpass,
           const char *rpc_req, bool, bool, int *,
           struct pool *pool, bool);
#endif
extern const char *proxytype(proxytypes_t proxytype);
extern char *get_proxy(char *url, struct pool *pool);
extern void __bin2hex(char *s, const unsigned char *p, size_t len);
extern char *bin2hex(const unsigned char *p, size_t len);
extern bool hex2bin(unsigned char *p, const char *hexstr, size_t len);
extern bool eth_hex2bin(unsigned char *p, const char *hexstr, size_t len);

typedef bool (*sha256_func)(struct thr_info*, const unsigned char *pmidstate,
  unsigned char *pdata,
  unsigned char *phash1, unsigned char *phash,
  const unsigned char *ptarget,
  uint32_t max_nonce,
  uint32_t *last_nonce,
  uint32_t nonce);

extern bool fulltest(const unsigned char *hash, const unsigned char *target);

extern int opt_queue;
extern int opt_scantime;
extern int opt_expiry;

extern cglock_t control_lock;
extern pthread_mutex_t hash_lock;
extern pthread_mutex_t console_lock;
extern cglock_t ch_lock;
extern pthread_rwlock_t mining_thr_lock;
extern pthread_rwlock_t devices_lock;

extern pthread_mutex_t restart_lock;
extern pthread_cond_t restart_cond;

extern void clear_stratum_shares(struct pool *pool);
extern void clear_pool_work(struct pool *pool);
extern void set_target(unsigned char *dest_target, double diff, double diff_multiplier2, const int thr_id);
extern void set_target_neoscrypt(unsigned char *target, double diff, const int thr_id);
extern double le256todiff(const void* le256, double diff_multiplier);

extern void kill_work(void);

extern void reinit_device(struct cgpu_info *cgpu);

extern void api(int thr_id);

extern struct pool *current_pool(void);
extern int enabled_pools;
extern void get_intrange(char *arg, int *val1, int *val2);
extern char *set_devices(char *arg);
extern bool detect_stratum(struct pool *pool, char *url);
extern void print_summary(void);
extern void adjust_quota_gcd(void);
extern struct pool *add_pool(void);
extern bool add_pool_details(struct pool *pool, bool live, char *url, char *user, char *pass, char *name, char *desc, char *profile, char *algo);

#define MAX_GPUDEVICES 42
#define MAX_DEVICES 4096

#define MIN_INTENSITY 4
#define MIN_INTENSITY_STR "4"
#define MAX_INTENSITY 31
#define MAX_INTENSITY_STR "31"
#define MIN_XINTENSITY 1
#define MIN_XINTENSITY_STR "1"
#define MAX_XINTENSITY 9999
#define MAX_XINTENSITY_STR "9999"
#define MIN_RAWINTENSITY 1
#define MIN_RAWINTENSITY_STR "1"
#define MAX_RAWINTENSITY 2147483647
#define MAX_RAWINTENSITY_STR "2147483647"

extern struct list_head scan_devices;
extern int nDevs;
extern int hw_errors;
extern bool use_syslog;
extern bool opt_quiet;
extern struct thr_info *control_thr;
extern struct thr_info **mining_thr;
extern struct cgpu_info gpus[MAX_GPUDEVICES];
extern double total_secs;
extern int mining_threads;
extern int total_devices;
extern bool devices_enabled[MAX_DEVICES];
extern int opt_devs_enabled;
extern bool opt_removedisabled;
extern struct cgpu_info **devices;
extern int total_pools;
extern struct pool **pools;
extern struct strategies strategies[];
extern enum pool_strategy pool_strategy;
extern int opt_rotate_period;
extern double total_rolling;
extern double total_mhashes_done;
extern unsigned int new_blocks;
extern unsigned int found_blocks;
extern int total_accepted, total_rejected;
extern double total_diff1;
extern int total_getworks, total_stale, total_discarded;
extern double total_diff_accepted, total_diff_rejected, total_diff_stale;
extern unsigned int local_work;
extern unsigned int total_go, total_ro;
extern int opt_cutofftemp;
extern int opt_log_interval;
extern unsigned long long global_hashrate;
extern char current_hash[68];
extern double current_diff;
extern double best_diff;
extern struct timeval block_timeval;
extern char *workpadding;

//config options table
extern struct opt_table opt_config_table[];

typedef struct _dev_blk_ctx {
  cl_uint ctx_a; cl_uint ctx_b; cl_uint ctx_c; cl_uint ctx_d;
  cl_uint ctx_e; cl_uint ctx_f; cl_uint ctx_g; cl_uint ctx_h;
  cl_uint cty_a; cl_uint cty_b; cl_uint cty_c; cl_uint cty_d;
  cl_uint cty_e; cl_uint cty_f; cl_uint cty_g; cl_uint cty_h;
  cl_uint merkle; cl_uint ntime; cl_uint nbits; cl_uint nonce;
  cl_uint fW0; cl_uint fW1; cl_uint fW2; cl_uint fW3; cl_uint fW15;
  cl_uint fW01r; cl_uint fcty_e; cl_uint fcty_e2;
  cl_uint W16; cl_uint W17; cl_uint W2;
  cl_uint PreVal4; cl_uint T1;
  cl_uint C1addK5; cl_uint D1A; cl_uint W2A; cl_uint W17_2;
  cl_uint PreVal4addT1; cl_uint T1substate0;
  cl_uint PreVal4_2;
  cl_uint PreVal0;
  cl_uint PreW18;
  cl_uint PreW19;
  cl_uint PreW31;
  cl_uint PreW32;

  /* FIXME: remove (For diakgcn) */
  cl_uint B1addK6, PreVal0addK7, W16addK16, W17addK17;
  cl_uint zeroA, zeroB;
  cl_uint oneA, twoA, threeA, fourA, fiveA, sixA, sevenA;

  struct work *work;
} dev_blk_ctx;

struct curl_ent {
  CURL *curl;
  char curl_err_str[CURL_ERROR_SIZE];
  struct list_head node;
  struct timeval tv;
};

/* The lowest enum of a freshly calloced value is the default */
enum pool_state {
  POOL_ENABLED,
  POOL_DISABLED,
  POOL_REJECTING,
  POOL_HIDDEN,
};

struct stratum_work {
  char *job_id;
  char *prev_hash;
  unsigned char **merkle_bin;
  char *bbversion;
  char *nbit;
  char *ntime;
  bool clean;

  size_t cb_len;
  size_t header_len;
  int merkles;
  double diff;
};

#define RBUFSIZE 8192
#define RECVSIZE (RBUFSIZE - 4)

struct pool {
  int pool_no;
  char *name;
  char *description;
  int prio;
  bool extranonce_subscribe;
  int accepted, rejected;
  int seq_rejects;
  int seq_getfails;
  int solved;
  double diff1;
  char diff[8];
  int quota;
  int quota_gcd;
  int quota_used;
  int works;
  eth_cache_t eth_cache;
  uint8_t Target[32];
  uint8_t EthWork[32];
  uint8_t NetDiff[32];

  //XMR stuff
  char XMRAuthID[64];
  uint32_t XMRBlobLen;
  uint8_t XMRBlob[128];
  pthread_mutex_t XMRGlobalNonceLock;
  uint32_t XMRGlobalNonce;
  bool is_monero;

  double diff_accepted;
  double diff_rejected;
  double diff_stale;

  bool submit_fail;
  bool idle;
  bool lagging;
  bool probed;
  enum pool_state state;
  bool no_keepalive;
  bool submit_old;
  bool remove_at_start;
  bool removed;
  bool lp_started;
  bool backup;

  char *hdr_path;
  char *lp_url;

  unsigned int getwork_requested;
  unsigned int stale_shares;
  unsigned int discarded_work;
  unsigned int getfail_occasions;
  unsigned int remotefail_occasions;
  struct timeval tv_idle;

  double utility;
  int last_shares, shares;

  char *rpc_req;
  char *rpc_url;
  char *rpc_userpass;
  char *rpc_user, *rpc_pass;
  proxytypes_t rpc_proxytype;
  char *rpc_proxy;

  char *profile;
  algorithm_t algorithm;
  const char *devices;
  char *intensity;
  char *xintensity;
  char *rawintensity;
  const char *lookup_gap;
  const char *gpu_engine;
  const char *gpu_memclock;
  const char *gpu_threads;
  const char *gpu_fan;
  const char *gpu_powertune;
  const char *gpu_vddc;
  const char *shaders;
  const char *thread_concurrency;
  const char *worksize;

  pthread_mutex_t pool_lock;
  cglock_t data_lock;

  struct thread_q *submit_q;
  struct thread_q *getwork_q;

  pthread_t longpoll_thread;
  pthread_t test_thread;
  bool testing;

  int curls;
  pthread_cond_t cr_cond;
  struct list_head curlring;

  time_t last_share_time;
  double last_share_diff;
  double best_diff;

  struct sgminer_stats sgminer_stats;
  struct sgminer_pool_stats sgminer_pool_stats;

  /* The last block this particular pool knows about */
  char prev_block[32];

  /* Stratum variables */
  bool has_stratum;
  char *stratum_url;
  char *stratum_port;
  struct addrinfo stratum_hints;
  SOCKETTYPE sock;
  char *sockbuf;
  size_t sockbuf_size;
  char *sockaddr_url; /* stripped url used for sockaddr */
  char *sockaddr_proxy_url;
  char *sockaddr_proxy_port;

  char *nonce1;
  unsigned char *nonce1bin;
  size_t n1_len;
  uint64_t nonce2;
  int n2size;
  char *sessionid;
  bool stratum_active;
  bool stratum_init;
  bool stratum_notify;
  struct stratum_work swork;
  pthread_t stratum_sthread;
  pthread_t stratum_rthread;
  pthread_mutex_t stratum_lock;
  struct thread_q *stratum_q;
  int sshares; /* stratum shares submitted waiting on response */

  /* GBT variables */
  bool has_gbt;
  cglock_t gbt_lock;
  unsigned char previousblockhash[32];
  unsigned char gbt_target[32];
  char *coinbasetxn;
  char *longpollid;
  char *gbt_workid;
  int gbt_expires;
  uint32_t gbt_version;
  uint32_t curtime;
  uint32_t gbt_bits;
  unsigned char *txn_hashes;
  size_t gbt_txns;
  size_t coinbase_len;

  /* equihash GBT */
  unsigned char reserved[32];

  /* Shared by both stratum & GBT */
  unsigned char *coinbase;
  size_t nonce2_offset;
  unsigned char header_bin[128];
  double next_diff;
  int merkle_offset;

  bool is_dev_pool;

  struct timeval tv_lastwork;
};

#define GETWORK_MODE_TESTPOOL 'T'
#define GETWORK_MODE_POOL 'P'
#define GETWORK_MODE_LP 'L'
#define GETWORK_MODE_BENCHMARK 'B'
#define GETWORK_MODE_STRATUM 'S'
#define GETWORK_MODE_GBT 'G'

struct work {
  unsigned char data[168];
  unsigned char midstate[128];
  unsigned char target[32];
  unsigned char hash[32];
  unsigned char mixhash[32];

  unsigned char device_target[32];
  double    device_diff;
  double    share_diff;
  double    network_diff;

  uint32_t eth_epoch;
  uint64_t Nonce;

  /* cryptonight stuff */
  uint32_t XMRBlobLen;
  bool is_monero;

  unsigned char equihash_data[1487];

  int   rolls;
  int   drv_rolllimit; /* How much the driver can roll ntime */

  dev_blk_ctx blk;

  struct thr_info *thr;
  int   thr_id;
  struct pool *pool;
  struct timeval  tv_staged;

  bool    mined;
  bool    clone;
  bool    cloned;
  int   rolltime;
  bool    longpoll;
  bool    stale;
  bool    mandatory;
  bool    block;

  bool    stratum;
  char    *job_id;
  uint64_t  nonce2;
  size_t    nonce2_len;
  char    *ntime;
  double    sdiff;
  char    *nonce1;

  bool    gbt;
  char    *coinbase;
  int   gbt_txns;

  unsigned int  work_block;
  int   id;
  UT_hash_handle  hh;

  double    work_difficulty;

  // Allow devices to identify work if multiple sub-devices
  int   subid;
  // Allow devices to flag work for their own purposes
  bool    devflag;
  // Allow devices to timestamp work for their own purposes
  struct timeval  tv_stamp;

  struct timeval  tv_getwork;
  struct timeval  tv_getwork_reply;
  struct timeval  tv_cloned;
  struct timeval  tv_work_start;
  struct timeval  tv_work_found;
  char    getwork_mode;
};

#define TAILBUFSIZ 64

#define tailsprintf(buf, bufsiz, fmt, ...) do { \
  char tmp13[TAILBUFSIZ]; \
  size_t len13, buflen = strlen(buf); \
  snprintf(tmp13, sizeof(tmp13), fmt, ##__VA_ARGS__); \
  len13 = strlen(tmp13); \
  if ((buflen + len13) >= bufsiz) \
    quit(1, "tailsprintf buffer overflow in %s %s line %d", __FILE__, __func__, __LINE__); \
  strcat(buf, tmp13); \
} while (0)

extern void get_datestamp(char *, size_t, struct timeval *);
extern void inc_hw_errors(struct thr_info *thr);
extern bool test_nonce(struct work *work, uint32_t nonce);
extern bool submit_tested_work(struct thr_info *thr, struct work *work);
extern bool submit_nonce(struct thr_info *thr, struct work *work, uint32_t nonce);
extern struct work *get_work(struct thr_info *thr, const int thr_id);
extern void _wlog(const char *str);
extern void _wlogprint(const char *str);
extern int curses_int(const char *query);
extern char *curses_input(const char *query);
extern void kill_work(void);

//helper macro to preserve existing code
#ifndef switch_pools
  #define switch_pools(p) __switch_pools(p, true)
#endif
extern void __switch_pools(struct pool *selected, bool saveprio);

extern void discard_work(struct work *work);
extern void remove_pool(struct pool *pool);
//extern void write_config(FILE *fcfg);
extern void zero_bestshare(void);
extern void zero_stats(void);
extern void default_save_file(char *filename);
extern bool _log_curses_only(int prio, const char *datetime, const char *str);
extern void clear_logwin(void);
extern void logwin_update(void);
extern bool pool_tclear(struct pool *pool, bool *var);
extern void pool_failed(struct pool *pool);
extern struct thread_q *tq_new(void);
extern void tq_free(struct thread_q *tq);
extern bool tq_push(struct thread_q *tq, void *data);
extern void *tq_pop(struct thread_q *tq, const struct timespec *abstime);
extern void tq_freeze(struct thread_q *tq);
extern void tq_thaw(struct thread_q *tq);
extern bool successful_connect;
extern void adl(void);
extern void app_restart(void);
extern void clean_work(struct work *work);
extern void free_work(struct work *work);
extern struct work *copy_work_noffset(struct work *base_work, int noffset);
#define copy_work(work_in) copy_work_noffset(work_in, 0)
extern struct cgpu_info *get_devices(int id);

extern char *set_int_0_to_9999(const char *arg, int *i);
extern char *set_int_1_to_65535(const char *arg, int *i);
extern char *set_int_0_to_10(const char *arg, int *i);
extern char *set_int_1_to_10(const char *arg, int *i);

enum api_data_type {
  API_ESCAPE,
  API_STRING,
  API_CONST,
  API_UINT8,
  API_UINT16,
  API_INT,
  API_UINT,
  API_UINT32,
  API_HEX32,
  API_UINT64,
  API_DOUBLE,
  API_ELAPSED,
  API_BOOL,
  API_TIMEVAL,
  API_TIME,
  API_MHS,
  API_KHS,
  API_MHTOTAL,
  API_TEMP,
  API_UTILITY,
  API_FREQ,
  API_VOLTS,
  API_HS,
  API_DIFF,
  API_PERCENT,
  API_AVG
};

struct api_data {
  enum api_data_type type;
  char *name;
  void *data;
  bool data_was_malloc;
  struct api_data *prev;
  struct api_data *next;
};

extern struct api_data *api_add_escape(struct api_data *root, char *name, char *data, bool copy_data);
extern struct api_data *api_add_string(struct api_data *root, char *name, char *data, bool copy_data);
extern struct api_data *api_add_const(struct api_data *root, char *name, const char *data, bool copy_data);
extern struct api_data *api_add_uint8(struct api_data *root, char *name, uint8_t *data, bool copy_data);
extern struct api_data *api_add_uint16(struct api_data *root, char *name, uint16_t *data, bool copy_data);
extern struct api_data *api_add_int(struct api_data *root, char *name, int *data, bool copy_data);
extern struct api_data *api_add_uint(struct api_data *root, char *name, unsigned int *data, bool copy_data);
extern struct api_data *api_add_uint32(struct api_data *root, char *name, uint32_t *data, bool copy_data);
extern struct api_data *api_add_hex32(struct api_data *root, char *name, uint32_t *data, bool copy_data);
extern struct api_data *api_add_uint64(struct api_data *root, char *name, uint64_t *data, bool copy_data);
extern struct api_data *api_add_double(struct api_data *root, char *name, double *data, bool copy_data);
extern struct api_data *api_add_elapsed(struct api_data *root, char *name, double *data, bool copy_data);
extern struct api_data *api_add_bool(struct api_data *root, char *name, bool *data, bool copy_data);
extern struct api_data *api_add_timeval(struct api_data *root, char *name, struct timeval *data, bool copy_data);
extern struct api_data *api_add_time(struct api_data *root, char *name, time_t *data, bool copy_data);
extern struct api_data *api_add_mhs(struct api_data *root, char *name, double *data, bool copy_data);
extern struct api_data *api_add_khs(struct api_data *root, char *name, double *data, bool copy_data);
extern struct api_data *api_add_mhstotal(struct api_data *root, char *name, double *data, bool copy_data);
extern struct api_data *api_add_temp(struct api_data *root, char *name, float *data, bool copy_data);
extern struct api_data *api_add_utility(struct api_data *root, char *name, double *data, bool copy_data);
extern struct api_data *api_add_freq(struct api_data *root, char *name, double *data, bool copy_data);
extern struct api_data *api_add_volts(struct api_data *root, char *name, float *data, bool copy_data);
extern struct api_data *api_add_hs(struct api_data *root, char *name, double *data, bool copy_data);
extern struct api_data *api_add_diff(struct api_data *root, char *name, double *data, bool copy_data);
extern struct api_data *api_add_percent(struct api_data *root, char *name, double *data, bool copy_data);
extern struct api_data *api_add_avg(struct api_data *root, char *name, float *data, bool copy_data);

#endif /* MINER_H */
