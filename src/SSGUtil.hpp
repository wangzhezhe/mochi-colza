/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#ifndef __COLZA_SSG_UTIL_HPP
#define __COLZA_SSG_UTIL_HPP

#include <ssg.h>

namespace colza {

inline static uint64_t ComputeGroupHash(ssg_group_id_t gid) {
    uint64_t hash = 0;
    int size = ssg_get_group_size(gid);
    for(int i = 0; i < size; i++) {
        auto id = ssg_get_group_member_id_from_rank(gid, i);
        hash ^= id;
    }
    return hash;
}

inline static uint64_t UpdateGroupHash(uint64_t hash, ssg_member_id_t id) {
   return hash ^ id;
}

}

#endif
