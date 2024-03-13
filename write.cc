#include <stdint.h>
#include <stdio.h>

#include "config.h"
#include "nvme.h"

#define WSIZE (4096)
static char wbuf[WSIZE];

int
main()
{
  uint64_t capa = N_ITEM * ITEM_SIZE;
  uint64_t i;
  uint64_t n = capa/WSIZE;
  
  nvme_init();
  for (i=0; i<WSIZE; i++) {
    wbuf[i] = i;
  }
  
  for (i=0; i<n; i++) {
    int rid = nvme_write_req(i * WSIZE/512, WSIZE/512, 0, WSIZE, wbuf);
    while (1) {
      int ret = nvme_check(rid);
      if (ret)
	break;
    }
    if (i % 1024 == 0) {
      printf("%ld/%ld\n", i, n);
    }
  }
}
