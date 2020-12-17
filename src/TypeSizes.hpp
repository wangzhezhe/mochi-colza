/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#ifndef __COLZA_TYPE_SIZES_H
#define __COLZA_TYPE_SIZES_H

#include "colza/Types.hpp"

namespace colza {

inline size_t ComputeDataSize(const std::vector<size_t>& dimensions, const Type& type) {
    size_t s = 1;
    for(auto d : dimensions)
        s *= d;
    switch(type) {
        case Type::INT8:
        case Type::UINT8:
            return s;
        case Type::INT16:
        case Type::UINT16:
            return 2*s;
        case Type::FLOAT32:
        case Type::INT32:
        case Type::UINT32:
            return 4*s;
        case Type::INT64:
        case Type::UINT64:
        case Type::FLOAT64:
            return 8*s;
    }
    return 0;
}

}

#endif
