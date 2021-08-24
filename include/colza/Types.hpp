/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#ifndef __COLZA_TYPES_HPP
#define __COLZA_TYPES_HPP

namespace colza {

#define COLZA_TYPES \
    X(INT8)         \
    X(UINT8)        \
    X(INT16)        \
    X(UINT16)       \
    X(INT32)        \
    X(UINT32)       \
    X(INT64)        \
    X(UINT64)       \
    X(FLOAT32)      \
    X(FLOAT64)      \

enum class Type : uint32_t {
#define X(t) t,
    COLZA_TYPES
#undef X
};

}

#endif
