#pragma once

#define CORET_SUSPEND (-1)

#define coBegin(func)     switch(co->func.state) { case 0:;
#define coEnd(func)     } co->func.state = 0;

#define coSuspend(func)		  \
  do {                            \
    co->func.state =__LINE__;            \
    return (CORET_SUSPEND); case __LINE__:;	  \
  } while (0)


typedef struct {
  int state;
} co_work_t;

typedef struct {
  int i_coro;
  int i_th;
  bool done;
  int rid;
  co_work_t co_work;
  index_t index;
} co_t;

using coret_t = int;
