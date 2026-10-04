#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include "../mystd.h"

// Requires debug flag to be set in mystd.h!!!

static void print_array(int* arr, size_t size){
  for(size_t i = 0; i < size; i++){
    printf("[%lu] - %p - %d\n", i, &arr[i], arr[i]);
  }
  printf("\n");
}

static void print_table() {
  printf("Table address: %p\n", array_table);
  for(size_t i = 0; i < table_pointer; i++){
    printf("ARRAY TABLE [%lu] - %p\n - %p\n", i, &array_table[i], array_table[i].ptr);
  }

  printf("TP: %lu\n", table_pointer);
  printf("Offset: %lu\n\n", table_offset);
}

static void dyn_arr_test() {

  size_t type = sizeof(int);
  size_t count =  10;

  print_table();
  printf("Struct size: %lu\n", sizeof(struct dyn_arr_s));

  // Initialization of the array table and array instances.

  int* my_arr_1 = mystd.dyn_arr.init_array(type, count);
  int* my_arr_2 = mystd.dyn_arr.init_array(type, count);
  int* my_arr_3 = mystd.dyn_arr.init_array(type, count);
  int* my_arr_4 = mystd.dyn_arr.init_array(type, count);
  print_table();
  assert(table_pointer == 4);
  mystd.dyn_arr.finish((void**)&my_arr_2);
  print_table();
  assert(array_table[1].ptr == my_arr_3);
  mystd.dyn_arr.finish((void**)&my_arr_1);
  mystd.dyn_arr.finish((void**)&my_arr_3);
  mystd.dyn_arr.finish((void**)&my_arr_4);
  assert(array_table == NULL);


  int* my_arr = mystd.dyn_arr.init_array(type, count);
  print_array(my_arr, count);
  print_table();
  mystd.dyn_arr.finish((void**)&my_arr);
  print_table();
  assert(my_arr == NULL);
  assert(array_table == NULL);

  // Adjusting, size and length

  my_arr = mystd.dyn_arr.init_array(type, count);
  print_table();
  assert(mystd.dyn_arr.size(my_arr) == type*count);
  assert(mystd.dyn_arr.length(my_arr) == count);
  my_arr = mystd.dyn_arr.adjust(my_arr, 11);
  assert(mystd.dyn_arr.size(my_arr) == 11);
  mystd.dyn_arr.finish((void**)&my_arr);
  print_table();
  assert(my_arr == NULL);
  assert(array_table == NULL);

  // Addition and removal by index

  my_arr = mystd.dyn_arr.init_array(sizeof(int), 2);
  print_array(my_arr, mystd.dyn_arr.length(my_arr));
  int a = 23;
  my_arr = mystd.dyn_arr.add(my_arr, &a);
  print_array(my_arr, mystd.dyn_arr.length(my_arr));
  my_arr = mystd.dyn_arr.add(my_arr, &a);
  print_array(my_arr, mystd.dyn_arr.length(my_arr));
  my_arr = mystd.dyn_arr.add(my_arr, &a);
  print_array(my_arr, mystd.dyn_arr.length(my_arr));
  mystd.dyn_arr.remove(my_arr, 0);
  print_array(my_arr, mystd.dyn_arr.length(my_arr));
  my_arr = mystd.dyn_arr.add(my_arr, &a);
  print_array(my_arr, mystd.dyn_arr.length(my_arr));
  my_arr = mystd.dyn_arr.add(my_arr, &a);
  print_array(my_arr, mystd.dyn_arr.length(my_arr));
  mystd.dyn_arr.remove(my_arr, 2);
  print_array(my_arr, mystd.dyn_arr.length(my_arr));
  mystd.dyn_arr.remove(my_arr, 2);
  print_array(my_arr, mystd.dyn_arr.length(my_arr));


  my_arr = mystd.dyn_arr.add(my_arr, &a);
  my_arr = mystd.dyn_arr.add(my_arr, &a);
  my_arr = mystd.dyn_arr.add(my_arr, &a);
  my_arr = mystd.dyn_arr.add(my_arr, &a);
  print_array(my_arr, mystd.dyn_arr.length(my_arr));
  mystd.dyn_arr.remove(my_arr, mystd.dyn_arr.length(my_arr)-1);
  print_array(my_arr, mystd.dyn_arr.length(my_arr));
  mystd.dyn_arr.remove(my_arr, 0);
  print_array(my_arr, mystd.dyn_arr.length(my_arr));
  mystd.dyn_arr.finish((void**)&my_arr);
  assert(array_table == NULL);

  return;
}
