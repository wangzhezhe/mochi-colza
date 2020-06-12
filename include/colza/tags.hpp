/* Tags for point to point operations which implement collective operations */
#ifndef __COLZA_TAGS_HPP
#define __COLZA_TAGS_HPP

#define COLZA_BARRIER_TAG               1
#define COLZA_BCAST_TAG                 2
#define COLZA_GATHER_TAG                3
#define COLZA_GATHERV_TAG               4
#define COLZA_SCATTER_TAG               5
#define COLZA_SCATTERV_TAG              6
#define COLZA_ALLGATHER_TAG             7
#define COLZA_ALLGATHERV_TAG            8
#define COLZA_ALLTOALL_TAG              9
#define COLZA_ALLTOALLV_TAG            10
#define COLZA_REDUCE_TAG               11
#define COLZA_USER_REDUCE_TAG          12
#define COLZA_USER_REDUCEA_TAG         13
#define COLZA_ALLREDUCE_TAG            14
#define COLZA_USER_ALLREDUCE_TAG       15
#define COLZA_USER_ALLREDUCEA_TAG      16
#define COLZA_REDUCE_SCATTER_TAG       17
#define COLZA_USER_REDUCE_SCATTER_TAG  18
#define COLZA_USER_REDUCE_SCATTERA_TAG 19
#define COLZA_SCAN_TAG                 20
#define COLZA_USER_SCAN_TAG            21
#define COLZA_USER_SCANA_TAG           22
#define COLZA_LOCALCOPY_TAG            23
#define COLZA_EXSCAN_TAG               24
#define COLZA_ALLTOALLW_TAG            25
#define COLZA_TOPO_A_TAG               26
#define COLZA_TOPO_B_TAG               27
#define COLZA_REDUCE_SCATTER_BLOCK_TAG 28
#define COLZA_SHRINK_TAG               29
#define COLZA_AGREE_TAG                30
#define COLZA_FIRST_HCOLL_TAG          31
#define COLZA_LAST_HCOLL_TAG           (COLZA_FIRST_HCOLL_TAG + 255)
#define COLZA_FIRST_NBC_TAG            (COLZA_LAST_HCOLL_TAG + 1)

#define COLZA_TAG_BITS_DEFAULT (31)

#endif
