#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include "dyn_arr.h"

#if MYSTD_DEBUG  == 0
struct dyn_arr_s {
  size_t offset;
  size_t data_type_size;
  void* ptr;
};

static struct dyn_arr_s* array_table = NULL;
static size_t table_pointer = 0; // Top of the table.
static size_t table_offset = 0;
#else
struct dyn_arr_s* array_table = NULL; //TODO: Remember to null & free when empty.
size_t table_pointer = 0; // Top of the table.
size_t table_offset = 0;
#endif


static void init_array_table() {
  if (array_table != NULL) return;
  table_offset = sizeof(struct dyn_arr_s);
  array_table = malloc(table_offset);
}

static void update_array_table() {
  if(table_pointer == 0) {
    free(array_table);
    array_table = NULL;
    return;
  }
  if (table_offset < (sizeof(struct dyn_arr_s)*(table_pointer+1))) {
    table_offset = (table_pointer+1)*sizeof(struct dyn_arr_s);
    array_table = realloc(array_table, table_offset);
  }
}

static void remove_array_table(size_t index){
  struct dyn_arr_s arrays[table_pointer - (index+1)] = {}; // Not include start (the one that is removed)
 
  int array_iterator = 0;
  for (size_t i = index+1; i < table_pointer; i++) { // Iterate from and to end
    arrays[array_iterator++] = array_table[i];
  }
  memcpy(&array_table[index], arrays, sizeof(arrays));

  table_pointer--;
  table_offset = (table_pointer)*sizeof(struct dyn_arr_s);
  array_table = realloc(array_table, table_offset);
  update_array_table();
}

static struct dyn_arr_s* get_ref_array(void* ptr) {
  for (size_t i = 0; i < table_pointer; i++) {
    struct dyn_arr_s curr_array = array_table[i];
    if(curr_array.ptr == ptr){
      return &array_table[i];
    }
  }
  return NULL;
}

static int get_ref_array_index(void* ptr) {
  for (size_t i = 0; i < table_pointer; i++) {
    struct dyn_arr_s curr_array = array_table[i];
    if(curr_array.ptr == ptr){
      return i;
    }
  }
  return 0;
}

static void* impl_init_array(size_t data_type_size, size_t count) {
  init_array_table();
  //if (info.INITIALIZATION_TYPE == MYSTD_DYNARR_SPECIFIED) {
     const size_t offset = data_type_size * count;
     void* arr_ptr = malloc(offset);
     memset(arr_ptr, 0, offset);
     struct dyn_arr_s array = {.ptr = arr_ptr,
       .data_type_size = data_type_size,
       .offset = offset};
     array_table[table_pointer++] = array;
     update_array_table(); 
     return arr_ptr;
  //}
  /**
  if(info.INITIALIZATION_TYPE == MYSTD_DYNARR_UNSPECIFIED) {
    void* arr_ptr = malloc(info.size);
    struct dyn_arr_s array = {.ptr = arr_ptr,
    .data_type_size = 0,
    .offset = info.size};
    array_table[table_pointer++] = array;
    update_array_table();
    return arr_ptr;
  }
  */
  return NULL;
}

  void* impl_adjust (void* self, size_t new_size) {
    struct dyn_arr_s* parray_table_ref = get_ref_array(self);
    parray_table_ref->offset = new_size;
    parray_table_ref->ptr = realloc((void*)parray_table_ref->ptr, parray_table_ref->offset*parray_table_ref->data_type_size);
    return parray_table_ref->ptr;
  }

  size_t impl_size (void* self) {
  struct dyn_arr_s* parray_table_ref = get_ref_array(self);
  return parray_table_ref->offset;
  }

  size_t impl_length (void* self) {
  struct dyn_arr_s* parray_table_ref = get_ref_array(self);
  return (parray_table_ref->offset/parray_table_ref->data_type_size);
  }

static void impl_finish(void** self) {
 int index_array_table = get_ref_array_index(*self);
 remove_array_table(index_array_table);
 free(*self);
 *self = NULL;
}

static void* impl_add(void* self, void* value) {
  struct dyn_arr_s* parray_table_ref = get_ref_array(self);

  size_t length = impl_length(self);
  int* p_check_initialized = (int*)parray_table_ref->ptr;       // TODO might be dumb to check like this
  int index = -1;
  for(int i = 0; i < length; i++) {   
    if(p_check_initialized[i] == 0) {
      index = i;
      break;
    }
  }

  //printf("Length of array: %lu, index %d\n", length, index);
  //printf("Calculated now: %p\n", parray_table_ref->ptr + index*parray_table_ref->data_type_size);
  //printf("Calculated using index: %p\n", &parray_table_ref->ptr[index]);

  if(index != -1) {
   memcpy(parray_table_ref->ptr + index*parray_table_ref->data_type_size, value, parray_table_ref->data_type_size); 
   return parray_table_ref->ptr;
  }

  parray_table_ref->offset += parray_table_ref->data_type_size;
  //printf("New offset: %lu\n", parray_table_ref->offset);
  //printf("Copy dest: %p\n", parray_table_ref->ptr + (length)*parray_table_ref->data_type_size);
  parray_table_ref->ptr = realloc(parray_table_ref->ptr, parray_table_ref->offset);
  memcpy(parray_table_ref->ptr + (length)*parray_table_ref->data_type_size, value, parray_table_ref->data_type_size);
  return parray_table_ref->ptr;
}

static void impl_remove(void* self, size_t index) {
  struct dyn_arr_s* parray_table_ref = get_ref_array(self);

  size_t index_offset = (index)*parray_table_ref->data_type_size;
  size_t post_offset = (index+1)*parray_table_ref->data_type_size;
  size_t delta_offset = parray_table_ref->offset - post_offset;
  //printf("Index offset: %lu, Post offset: %lu, Delta: %lu", index_offset, post_offset, delta_offset);
  if (delta_offset == 0){
    goto move;
  }

  {
    char buffer[delta_offset];
    memcpy(buffer, parray_table_ref->ptr + post_offset, delta_offset);
    memmove(parray_table_ref->ptr + index_offset, buffer, delta_offset);
  }
  
  move:
  parray_table_ref->offset -= parray_table_ref->data_type_size;
  parray_table_ref->ptr = realloc(parray_table_ref->ptr, parray_table_ref->offset);

}


const struct mystd_dyn_api mystd = {
  .init_array = impl_init_array,
  .adjust = impl_adjust,
  .finish = impl_finish,
  .size = impl_size,
  .length = impl_length,
  .add = impl_add,
  .remove = impl_remove
};
