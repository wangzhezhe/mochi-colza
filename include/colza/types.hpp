/* Types to label different implementations of the communication primitives */
#ifndef __COLZA_TYPES_HPP
#define __COLZA_TYPES_HPP

enum class bcast_algorithm { sequential, binomial };

enum class barrier_algorithm { bcast, dissemination };

#endif
