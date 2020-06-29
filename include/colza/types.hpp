/* Types to label different implementations of the communication primitives */
#ifndef __COLZA_TYPES_HPP
#define __COLZA_TYPES_HPP

#include <cstddef>
#include <cstdint>

namespace colza {

enum class COLZA_Bcast { sequential, binomial };

enum class COLZA_Barrier { bcast, dissemination };

// define the function type of the operations
// the parameters are sendbuffer, accumulated recvbuffer, element size(for data
// checking), number of element, and the context (for flexibility)
typedef int (*COLZA_Operation_Func)(const void*, void*, size_t, size_t, void*);

int COLZA_OP_SUM_INT64(const void* inputBuffer, void* cumulatedBuffer,
                       size_t elemSize, size_t count, void* context);

int COLZA_OP_SUM_DOUBLE(const void* inputBuffer, void* cumulatedBuffer,
                        size_t elemSize, size_t count, void* context);

}  // namespace colza
#endif
