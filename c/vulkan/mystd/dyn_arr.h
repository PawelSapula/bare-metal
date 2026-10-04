#ifndef MYSTD_DYN_ARR_H
#define MYSTD_DYN_ARR_H
#include <stdlib.h>
#include "mystd_conf.h"

//#define MYSTD_DYNARR_SPECIFIED 1
struct dyn_arr_info {
  //unsigned int INITIALIZATION_TYPE : 1;
  //size_t size;

  size_t element_count;
  size_t data_type_size;
};


struct mystd_dyn_api {
  void* (*init_array)(size_t data_type_size, size_t count);
  void* (*adjust) (void* self, size_t new_size);
  size_t (*size) (void* self);
  size_t (*length) (void* self);
  void (*finish) (void** pself);
  void* (*add)(void* self, void* value);
  void (*remove)(void* self, size_t index);
};

const struct mystd_dyn_api dyn_arr;

#if MYSTD_DEBUG == 1
struct dyn_arr_s {
  size_t offset;
  size_t data_type_size;
  void* ptr;
};


struct dyn_arr_s* array_table; //TODO: Remember to null & free when empty.
size_t table_pointer;
size_t table_offset;
#endif


#endif /* MYSTD_DYN_ARR_H */
