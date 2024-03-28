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
