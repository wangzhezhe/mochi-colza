
#include <assert.h>

#include <iostream>

#include "colza/types.hpp"
#include <unordered_map>
namespace colza {

// TODO support same operation for different types
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

int COLZA_OP_SUM_UNSIGNED_LONG_LONG(const void* inputBuffer,
                                    void* cumulatedBuffer, size_t elemSize,
                                    size_t count, void* context) {
  assert(elemSize == sizeof(unsigned long long));
  const unsigned long long* x =
      static_cast<const unsigned long long*>(inputBuffer);
  unsigned long long* y = static_cast<unsigned long long*>(cumulatedBuffer);
  for (auto i = 0; i < count; i++) {
    y[i] = x[i] + y[i];
  }
  return 0;
}

int COLZA_OP_MAX_UNSIGNED_CHAR(const void* inputBuffer, void* cumulatedBuffer,
                               size_t elemSize, size_t count, void* context) {
  assert(elemSize == sizeof(unsigned char));
  const unsigned char* x = static_cast<const unsigned char*>(inputBuffer);
  unsigned char* y = static_cast<unsigned char*>(cumulatedBuffer);
  for (auto i = 0; i < count; i++) {
    y[i] = std::max(x[i], y[i]);
  }
  return 0;
}

int COLZA_OP_MAX_INT(const void* inputBuffer, void* cumulatedBuffer,
                     size_t elemSize, size_t count, void* context) {
  assert(elemSize == sizeof(int));
  const int* x = static_cast<const int*>(inputBuffer);
  int* y = static_cast<int*>(cumulatedBuffer);
  for (auto i = 0; i < count; i++) {
    y[i] = std::max(x[i], y[i]);
  }
  return 0;
}

int COLZA_OP_MAX_DOUBLE(const void* inputBuffer, void* cumulatedBuffer,
                        size_t elemSize, size_t count, void* context) {
  assert(elemSize == sizeof(double));
  const double* x = static_cast<const double*>(inputBuffer);
  double* y = static_cast<double*>(cumulatedBuffer);
  for (auto i = 0; i < count; i++) {
    y[i] = std::max(x[i], y[i]);
  }
  return 0;
}

int COLZA_OP_MIN_DOUBLE(const void* inputBuffer, void* cumulatedBuffer,
                        size_t elemSize, size_t count, void* context) {
  assert(elemSize == sizeof(double));
  const double* x = static_cast<const double*>(inputBuffer);
  double* y = static_cast<double*>(cumulatedBuffer);
  for (auto i = 0; i < count; i++) {
    y[i] = std::min(x[i], y[i]);
  }
  return 0;
}

// register the function into the map for indexing
std::unordered_map<uint64_t, COLZA_Operation_Func> OpsFuncMap;

void addOpFunc(uint32_t opID, uint32_t dataID, COLZA_Operation_Func funcptr) {
  uint64_t funcID = ((uint64_t)opID << 32) | dataID;
  OpsFuncMap[funcID] = funcptr;
  return;
}

void init_ops_map() {
  // add the function used by the iceT render.py
  /*
  vtk type: 6 operation: 0
  vtk type: 3 operation: 0
  vtk type: 11 operation: 0
  vtk type: 17 vtk operation: 2
  vtk type: 11 vtk operation: 1
  vtk type: 11 vtk operation: 0
  */
  addOpFunc(COLZA_Reduction_Op::MAX, COLZA_UNSIGNED_CHAR,
            &COLZA_OP_MAX_UNSIGNED_CHAR);
  addOpFunc(COLZA_Reduction_Op::MAX, COLZA_INT, &COLZA_OP_MAX_INT);
  addOpFunc(COLZA_Reduction_Op::MAX, COLZA_DOUBLE, &COLZA_OP_MAX_DOUBLE);

  addOpFunc(COLZA_Reduction_Op::MIN, COLZA_DOUBLE, &COLZA_OP_MIN_DOUBLE);
  addOpFunc(COLZA_Reduction_Op::SUM, COLZA_UNSIGNED_LONG_LONG,
            &COLZA_OP_SUM_UNSIGNED_LONG_LONG);
  return;
}

COLZA_Operation_Func get_ops_func(uint32_t opID, uint32_t dataID) {
  uint64_t funcID = ((uint64_t)opID << 32) | dataID;
  if (OpsFuncMap.count(funcID) == 0) {
    return nullptr;
  }
  return OpsFuncMap[funcID];
}

}  // namespace colza
