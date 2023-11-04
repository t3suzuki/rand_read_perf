#include <cstdint>
#include <cstdio>
#include <unistd.h>
#include <stdlib.h>
#include <iostream>
#include <chrono>
#include <vector>
#include <thread>
#include <immintrin.h>

typedef uint64_t index_t;
#include "stackless.h"

#define N_TH (1)
#define N_CORO (16)
#define N_ITEM (1024*1024)
#define ALIGN_SIZE (64)
#define TIME_SEC (5)


static char *mmap_base_addr;
class Mmap {
public:
  static void open() {
    mmap_base_addr = (char *)malloc(ALIGN_SIZE * N_ITEM);
  }
  static inline bool prefetch(index_t index) {
    return false;
  }
  static inline index_t read(index_t index) {
    return *(index_t *)(mmap_base_addr + index * ALIGN_SIZE);
  }
  static void close() {
    free(mmap_base_addr);
  }
};

template<class T>
inline coret_t co_work(co_t *co, uint64_t &iter, volatile bool *quit) {
  coBegin(co_work);
  while (*quit == false) {
    iter++;
    if (T::prefetch(co->index))
      coSuspend(co_work);
    co->index = T::read(co->index);
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
uint64_t g_last[N_TH];

template<class T>
void worker(int i_th, volatile bool *begin, volatile bool *quit)
{
  setThreadAffinity(i_th);
  
  int n_done = 0;
  uint64_t iter = 0;
  co_t co[N_CORO];

  for (int i_coro=0; i_coro<N_CORO; i_coro++) {
    co[i_coro].done = false;
    co[i_coro].id = i_coro;
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
	coret_t coret = co_work<T>(&co[i_coro], iter, quit);
	if (coret > 0) {
	  n_done++;
	}
      }
    }
  } while (n_done != N_CORO);

  g_cnt[i_th] = iter;
  for (int i_coro=0; i_coro<N_CORO; i_coro++) {
    g_last[i_th] += co[i_coro].index;
  }
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
  for (auto i_th=0; i_th<N_TH; i_th++)
    sum += g_cnt[i_th];
    
  auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end-start);
  std::cout << "elapsed time: " << elapsed.count() << "ms\n";
  double miops = sum / 1000.0 / elapsed.count();
  //std::cout << miops << " M IOPS" << std::endl;
  //std::cout << 1/miops * 1000 << " ns/req" << std::end;

  T::close();
}


int
main()
{
  std::cout << "Using " << N_TH << " threads. " << N_CORO << " contexts/thread. " << std::endl;
  
  std::cout << "Running..." << std::endl;
  run_test<Mmap>();
  std::cout << "Done!" << std::endl;
  exit(0);
}
