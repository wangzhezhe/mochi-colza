/* Types to label different implementations of the communication primitives */
#ifndef __COLZA_TYPES_HPP
#define __COLZA_TYPES_HPP

#include <cstddef>
#include <cstdint>

#define COLZA_IN_PLACE  (void *) -1

// define the datatype and the operation type to index the function
// refer to the vtk data type
#define COLZA_VOID 0
#define COLZA_BIT 1
#define COLZA_CHAR 2
#define COLZA_SIGNED_CHAR 15
#define COLZA_UNSIGNED_CHAR 3
#define COLZA_SHORT 4
#define COLZA_UNSIGNED_SHORT 5
#define COLZA_INT 6
#define COLZA_UNSIGNED_INT 7
#define COLZA_LONG 8
#define COLZA_UNSIGNED_LONG 9
#define COLZA_FLOAT 10
#define COLZA_DOUBLE 11
#define COLZA_STRING 13
#define COLZA_OPAQUE 14
#define COLZA_LONG_LONG 16
#define COLZA_UNSIGNED_LONG_LONG 17

namespace colza {

enum class COLZA_Bcast { sequential, binomial };

enum class COLZA_Barrier { bcast, dissemination };

// define the operation for data reduction
enum COLZA_Reduction_Op : uint32_t{
  MAX,
  MIN,
  SUM,
  PRODUCT,
  LOGICAL_AND,
  BITWISE_AND,
  LOGICAL_OR,
  BITWISE_OR,
  LOGICAL_XOR,
  BITWISE_XOR
};

// define the function type of the operations
// the parameters are sendbuffer, accumulated recvbuffer, element size(for data
// checking), number of element, and the context (for flexibility)
typedef int (*COLZA_Operation_Func)(const void*, void*, size_t, size_t, void*);

COLZA_Operation_Func get_ops_func(uint32_t opID,uint32_t dataID);

void init_ops_map();

//implemented operations
int COLZA_OP_SUM_INT64(const void* inputBuffer, void* cumulatedBuffer,
                       size_t elemSize, size_t count, void* context);

int COLZA_OP_SUM_DOUBLE(const void* inputBuffer, void* cumulatedBuffer,
                        size_t elemSize, size_t count, void* context);

int COLZA_OP_SUM_UNSIGNED_LONG_LONG(const void* inputBuffer, void* cumulatedBuffer,
                        size_t elemSize, size_t count, void* context);

int COLZA_OP_MAX_UNSIGNED_CHAR(const void* inputBuffer, void* cumulatedBuffer,
                        size_t elemSize, size_t count, void* context);

int COLZA_OP_MAX_INT(const void* inputBuffer, void* cumulatedBuffer,
                        size_t elemSize, size_t count, void* context);

int COLZA_OP_MAX_DOUBLE(const void* inputBuffer, void* cumulatedBuffer,
                        size_t elemSize, size_t count, void* context);

int COLZA_OP_MIN_DOUBLE(const void* inputBuffer, void* cumulatedBuffer,
                        size_t elemSize, size_t count, void* context);



}  // namespace colza


#endif
