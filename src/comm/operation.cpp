
#include <assert.h>
#include <iostream>
#include "colza/types.hpp"
namespace colza {

//TODO support same operation for different types
int COLZA_OP_SUM_INT64(const void* inputBuffer, void* cumulatedBuffer,
                        size_t elemSize, size_t count, void* context) {
  assert(elemSize == sizeof(int64_t));
  const int64_t* x = static_cast<const int64_t*>(inputBuffer);
  int64_t* y = static_cast<int64_t*>(cumulatedBuffer);
  for (auto i = 0; i < count; i++) {
    y[i] = x[i] + y[i];
  }
  return 0;
}

int COLZA_OP_SUM_DOUBLE(const void* inputBuffer, void* cumulatedBuffer,
                        size_t elemSize, size_t count, void* context) {
  assert(elemSize == sizeof(double));
  const double* x = static_cast<const double*>(inputBuffer);
  double* y = static_cast<double*>(cumulatedBuffer);
  for (auto i = 0; i < count; i++) {
    y[i] = x[i] + y[i];
  }
  return 0;
}

}  // namespace colza
