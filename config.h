#ifndef __CONFIG_H__
#define __CONFIG_H__

#if 1
#define ND (4)
#define DRIVE_IDS "0000:02:00.0_0000:03:00.0_0000:04:00.0_0000:05:00.0"
#else
#define ND (1)
#define DRIVE_IDS "0000:02:00.0"
#endif

#define N_TH (1)

#define URAND (1)
#define THETA (0.1)

#define NLOG2_CACHED (25)

//#define ITEM_SIZE (64ULL)
#define ITEM_SIZE (512ULL)

#define WARMUP_SEC (0)
#define TIME_SEC (180)

#endif // __CONFIG_H__
