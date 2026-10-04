#ifndef MYSTD_H
#define MYSTD_H

#include "dyn_arr.h"

struct mystd_api {
  struct mystd_dyn_api dyn_arr; 
} mystd;

#if MYSTD_DEBUG == 1
#include "tests/dyn_arr.c"
void mystd_tests() {
  dyn_arr_test();
}
#else
void mystd_tests() {}
#endif


#endif /* MYSTD_H */
