#include <cstdint>
#include <cstdio>
#include <unistd.h>
#include <stdlib.h>
#include <iostream>
#include <chrono>
#include <vector>
#include <thread>
#include <string.h>
#include <immintrin.h>
#include <libcuckoo/cuckoohash_map.hh>

typedef uint64_t index_t;
#include "stackless.h"
#include "zipf.h"
#include "nvme.h"

#define N_TH (1)
#define N_CORO (512)
//#define N_ITEM (1024ULL*1024*1024*16)
#define N_ITEM (1024ULL*1024*1024*16)
//#define N_ITEM (1024ULL)
//#define ALIGN_SIZE (64)
#define ALIGN_SIZE (64)
#define TIME_SEC (20)

#define THETA (0.7)
//#define CHASE (1)

using hash_t = libcuckoo::cuckoohash_map<index_t, index_t>;

static hash_t hashcache;
static char rbuf[N_CORO][512];
static char *mmap_base_addr;

class MyNVMeCached {
public:
  static void open() {
    printf("MyNVMeCached init\n");
    nvme_init();
  }
  static inline void prefetch(co_t *co, index_t index) {
    index_t value;
    if (hashcache.find(index, value)) {
      *(index_t *)rbuf[co->i_coro] = value;
      co->rid = -1;
    } else {
      uint64_t lba = index * ALIGN_SIZE / 512;
      co->rid = nvme_read_req(lba, 1, co->i_th, ALIGN_SIZE, rbuf[co->i_coro]);
    }
  }
  static inline bool prefetch_done(co_t *co, index_t index) {
    if (co->rid == -1)
      return true;
    else
      return nvme_check(co->rid);
  }
  static inline index_t read(co_t *co, index_t index) {
    if (co->rid == -1) {
      return *(index_t *)(&rbuf[co->i_coro][0]);
    } else {
      index_t v = *(index_t *)(&rbuf[co->i_coro][index * ALIGN_SIZE % 512]);
      hashcache.insert(index, v);
      return v;
    }
  }
  static void close() {
  }
};


#include "cachelib/allocator/CacheAllocator.h"

using namespace facebook::cachelib;
using Cache = S3FIFOAllocator;

static Cache *cache;
static PoolId pool;
class MyNVMeS3fifo {
  using key_t = index_t;
  using value_t = index_t;
  
  static void mycache_init(int64_t cache_size_in_mb, unsigned int hashpower,
		    Cache **cache_p, PoolId *pool_p) {
    Cache::Config config;
    
    config.setCacheSize(cache_size_in_mb * 1024 * 1024)
      .setCacheName("My cache")
      .setAccessConfig({hashpower, hashpower})
      .validate();
    *cache_p = new Cache(config);
    *pool_p = (*cache_p)->addPool("default",
				  (*cache_p)->getCacheMemoryStats().ramCacheSize);
    util::setCurrentTimeSec(1);
    assert(util::getCurrentTimeSec() == 1);
    printf("Enable S3fifo\n");
  }
  static inline std::string gen_strkey(uint64_t key) {
    auto strkey = fmt::format_int(key).str();
    return strkey;
  }
  static inline int cache_lookup(key_t key, value_t &value) {
    Cache::ReadHandle item_handle = cache->find(gen_strkey(key));
    if (item_handle) {
      assert(item_handle->getSize() == ALIGN_SIZE);
      const char *data = reinterpret_cast<const char *>(item_handle->getMemory());
      return 1; // hit
    } else {
      return 0; // miss
    }
  }
  static inline bool cache_set(key_t key, value_t value) {
    Cache::WriteHandle item_handle = cache->allocate(pool, gen_strkey(key), ALIGN_SIZE);
    if (item_handle == nullptr || item_handle->getMemory() == nullptr) {
      return 1;
    }
    std::memcpy(item_handle->getMemory(), &value, sizeof(value_t));
    cache->insertOrReplace(item_handle);
    return 0;
  }
public:
  
  static void open() {
    const int hashpower = 21; // >= 20
    const int size_mb = ALIGN_SIZE << (hashpower - 20);
    printf("MyNVMeS3fifo init\n");
    nvme_init();
    mycache_init(size_mb, hashpower, &cache, &pool);
  }
  static inline void prefetch(co_t *co, index_t index) {
    index_t value;
    if (cache_lookup(index, value)) {
      *(index_t *)rbuf[co->i_coro] = value;
      co->rid = -1;
    } else {
      uint64_t lba = index * ALIGN_SIZE / 512;
      co->rid = nvme_read_req(lba, 1, co->i_th, ALIGN_SIZE, rbuf[co->i_coro]);
    }
  }
  static inline bool prefetch_done(co_t *co, index_t index) {
    if (co->rid == -1)
      return true;
    else
      return nvme_check(co->rid);
  }
  static inline index_t read(co_t *co, index_t index) {
    if (co->rid == -1) {
      return *(index_t *)(&rbuf[co->i_coro][0]);
    } else {
      index_t v = *(index_t *)(&rbuf[co->i_coro][index * ALIGN_SIZE % 512]);
      cache_set(index, v);
      return v;
    }
  }
  static void close() {
  }
};


class MyNVMe {
public:
  static void open() {
    printf("MyNVMe init\n");
    nvme_init();
  }
  static inline void prefetch(co_t *co, index_t index) {
    uint64_t lba = index * ALIGN_SIZE / 512;
    //memset(rbuf[co->i_coro], 0, 512);
    co->rid = nvme_read_req(lba, 1, co->i_th, ALIGN_SIZE, rbuf[co->i_coro]);
  }
  static inline bool prefetch_done(co_t *co, index_t index) {
    return nvme_check(co->rid);
  }
  static inline index_t read(co_t *co, index_t index) {
    return *(index_t *)(&rbuf[co->i_coro][index * ALIGN_SIZE % 512]);
  }
  static void close() {
  }
};

class Mmap {
public:
  static void open() {
    printf("MMap init\n");
    mmap_base_addr = (char *)malloc(ALIGN_SIZE * N_ITEM);
  }
  static inline void prefetch(co_t *co, index_t index) {
  }
  static inline bool prefetch_done(co_t *co, index_t index) {
    return true;
  }
  static inline index_t read(co_t *co, index_t index) {
    return *(index_t *)(mmap_base_addr + index * ALIGN_SIZE);
  }
  static void close() {
    free(mmap_base_addr);
  }
};

class Nop {
public:
  static void open() {
    printf("Nop init\n");
  }
  static inline bool prefetch(co_t *co, index_t index) {
    return false;
  }
  static inline bool prefetch_done(co_t *co, index_t index) {
    return true;
  }
  static inline index_t read(co_t *co, index_t index) {
    return 0;
  }
  static void close() {
  }
};

class Genr {
private:
  struct zipf * zipf;
public:
  Genr(int seed) {
    zipf = zipf_create(N_ITEM, THETA, seed);
  }
  inline uint64_t gen() {
    return zipf_generate(zipf);
  }
};

template<class T>
inline coret_t co_work(co_t *co, uint64_t &iter, volatile bool *quit, Genr &genr, uint64_t &tmp, uint64_t &hit) {
  coBegin(co_work);
  while (*quit == false) {
    //if (iter < 10000){
    if (1){
      iter++;
#if CHASE
      T::prefetch(co, co->index);
      whlie (!prefetch_done(co, co->index)) {
	coSuspend(co_work);
      }
      co->index = T::read(co, co->index);
#else
      co->index = genr.gen();
      T::prefetch(co, co->index);
      if (co->rid == -1)
	hit++;
      else {
	while (T::prefetch_done(co, co->index) == 0)
	  coSuspend(co_work);
      }
      tmp += T::read(co, co->index);
#endif
    }
  }
  coEnd(co_work);
  co->done = true;
  return 1;
}

#define _GNU_SOURCE
#include <sched.h>

void setThreadAffinity(int core)
{
#if 0
  printf("[Note] use core %d\n", core);
  cpu_set_t cpu_set;
  CPU_ZERO(&cpu_set);
  CPU_SET(core, &cpu_set);
  sched_setaffinity(0, sizeof(cpu_set_t), &cpu_set);
#else
  printf("[Note] skipping %s\n", __func__);
#endif
}

uint64_t g_cnt[N_TH];
uint64_t g_tmp[N_TH];
uint64_t g_hit[N_TH];

template<class T>
void worker(int i_th, volatile bool *begin, volatile bool *quit)
{
  setThreadAffinity(i_th);

  Genr genr(i_th);
  int n_done = 0;
  uint64_t iter = 0;
  uint64_t tmp = 0;
  uint64_t hit = 0;
  co_t co[N_CORO];

  for (int i_coro=0; i_coro<N_CORO; i_coro++) {
    co[i_coro].done = false;
    co[i_coro].i_th = i_th;
    co[i_coro].i_coro = i_coro;
    co[i_coro].index = i_coro * N_ITEM / N_CORO;
  }
  while (1) {
    if (*begin)
      break;
    _mm_pause();
  }

  do {
    for (int i_coro=0; i_coro<N_CORO; i_coro++) {
      if (!co[i_coro].done) {
	coret_t coret = co_work<T>(&co[i_coro], iter, quit, genr, tmp, hit);
	if (coret > 0) {
	  n_done++;
	}
      }
    }
  } while (n_done != N_CORO);

  g_cnt[i_th] = iter;
  g_tmp[i_th] = tmp;
  g_hit[i_th] = hit;
}

template<class T>
void run_test() {
  T::open();

  auto start = std::chrono::steady_clock::now();
  std::vector<std::thread> thv;
  bool begin = false;
  bool quit = false;
  for (auto i_th=0; i_th<N_TH; i_th++)
    thv.emplace_back(worker<T>, i_th, &begin, &quit);
    
  begin = true;
  for (auto i=1; i<=TIME_SEC; i++) {
    sleep(1);
    printf("Elapsed %d/%d\n", i, TIME_SEC);
  }
  quit = true;
  for (auto &th: thv)
    th.join();
  auto end = std::chrono::steady_clock::now();
    
  uint64_t sum = 0;
  uint64_t tmp = 0;
  uint64_t hit = 0;
  for (auto i_th=0; i_th<N_TH; i_th++) {
    sum += g_cnt[i_th];
    tmp += g_tmp[i_th];
    hit += g_hit[i_th];
  }
  
  auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end-start);
  std::cout << "num ops: " << sum << "\n";
  std::cout << "num hits: " << hit << "\n";
  std::cout << "hit rate: " << hit / (double)sum << "\n";
  std::cout << "elapsed time: " << elapsed.count() << "ms\n";
  double miops = sum / 1000.0 / elapsed.count();
  std::cout << miops << " M IOPS" << std::endl;
  std::cout << "tmp = " << tmp << std::endl;
  //std::cout << 1/miops * 1000 << " ns/req" << std::end;

  T::close();
}


int
main(int argc, char **argv)
{
  std::cout << "Using " << N_TH << " threads. " << N_CORO << " contexts/thread. " << std::endl;
  
  std::cout << "Running..." << std::endl;

  int mode = 0;
  if (argc > 1) {
    mode = atoi(argv[1]);
  }
  switch (mode) {
  case 0:
    run_test<Nop>();
    break;
  case 1:
    run_test<Mmap>();
    break;
  case 2:
    run_test<MyNVMe>();
    break;
  case 3:
    run_test<MyNVMeCached>();
    break;
  case 4:
    run_test<MyNVMeS3fifo>();
    break;
  }
  std::cout << "Done!" << std::endl;
  exit(0);
}
