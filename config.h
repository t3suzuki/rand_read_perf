#ifndef __CONFIG_H__
#define __CONFIG_H__

#if 1
#define ND (4)
#define DRIVE_IDS "0000:02:00.0_0000:03:00.0_0000:04:00.0_0000:05:00.0"
#else
#define ND (1)
#define DRIVE_IDS "0000:02:00.0"
#endif

//#define N_TH (2)

//#define URAND (1)
//#define THETA (0.1)

#define NLOG2_CACHED (24)

//#define ITEM_SIZE (64ULL)
#define ITEM_SIZE (512ULL) // should be <= 512
#define N_ITEM (1024ULL*1024*32)

#if 0 // long exec
#define WARMUP_SEC (60)
#define TIME_SEC (180)
#else // short exec
#define WARMUP_SEC (3)
#define TIME_SEC (7)
#endif


#endif // __CONFIG_H__
