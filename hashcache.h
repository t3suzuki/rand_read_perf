#pragma once

#include <libcuckoo/cuckoohash_map.hh>
#include <sys/mman.h>

void *
my_alloc(size_t sz)
{
#if USE_HUGPAGE
  printf("hugepages %f MB\n", sz / 1024.0 / 1024.0);
  void *p = mmap(0, sz, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS|MAP_HUGETLB, -1, 0);
  assert(p != MAP_FAILED);
  return p;
#else
  return malloc(sz);
#endif
}

#define N_RWLOCKS (256)

class HashCache {
  using key_t = uint64_t;
  using val_t = uint64_t;
  using index_t = uint64_t;
  using hash_t = libcuckoo::cuckoohash_map<key_t, index_t>;
  hash_t hash;
  val_t *val_log;
  key_t *key_log;
  index_t cur_index;
  static const key_t invalid_key = -1;
  size_t n_key;
  pthread_rwlock_t rwlocks[N_RWLOCKS];
public:
  HashCache(size_t val_capacity) {
    cur_index = 0;
    n_key = val_capacity / sizeof(val_t);
    printf("HashCache: val_capacity %lu, n_key %lu, N_RWLOCKS %d\n", val_capacity, n_key, N_RWLOCKS);
    val_log = (val_t *)my_alloc(val_capacity);
    key_log = (key_t *)my_alloc(n_key * sizeof(key_t));
    for (uint64_t i=0; i<n_key; i++) {
      key_log[i] = invalid_key;
    }
    for (int i=0; i<N_RWLOCKS; i++) {
      pthread_rwlock_init(&rwlocks[i], NULL);
    }
  }
  void insert(key_t key, val_t val) {
    index_t index = __sync_fetch_and_add(&cur_index, 1) % n_key;
    key_t old_key = key_log[index];
    key_log[index] = invalid_key;
    hash.erase(old_key);
    hash->insert(key, index);

    
    key_log[index] = key;
    val_log[index] = val;
  }
  bool lookup(key_t key, val_t &val) {
  retry:
    index_t index;
    bool found = hash->find(key, index);
    if (found) {
      if (key == key_log[index]) {
	val = val_log[index];
	return true;
      } else {
	goto retry;
      }
    } else {
      return false;
    }
  }
  size_t bucket_count() {
    return hash.bucket_count();
  }
  void rehash() {
    return hash.rehash();
  }
};

