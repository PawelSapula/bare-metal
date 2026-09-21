#ifndef UTILS_H_
#define UTILS_H_
#include <stdlib.h>

#ifdef NDEBUG
#define VALIDATION_LAYERS 0
#else
#define VALIDATIONS_LAYERS 1
#endif

#define GET_ARRAY(type, count) malloc(sizeof(type)*count)

#define OPTIONAL_REGISTER(T) typedef struct {T value; uint8_t is_changed;} T ## _opt; // Inspiration https://github.com/m-kru/c-option/tree/master
#define OPTIONAL_MODIFY(ref) ref.is_changed = 1; ref.value  // Use = to assign
#define OPTIONAL_CHANGED(ref) (ref.is_changed == 1 ? 1 : 0)

OPTIONAL_REGISTER(uint32_t);

#define SUCCESS 1 // For personal function returns
#define FAILURE 0

#endif /* UTILS_H_ */
