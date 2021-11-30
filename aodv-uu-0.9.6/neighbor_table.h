//
// Created by Alex on 2021/11/30.
//

#ifndef _NEIGHBOR_TABLE_H
#define _NEIGHBOR_TABLE_H

#ifndef NS_NO_GLOBALS
#include "defs.h"
#include "list.h"

typedef struct nb_table nb_table_t;

typedef u_int32_t hash_value;	/* A hash value */

// by fxj
#define NB_TABLESIZE 64		/* Must be a power of 2 */
#define NB_TABLEMASK (NB_TABLESIZE - 1)
struct dest_time {
    struct in_addr dest;
    struct time last_time;
};
// fxj_end

/* neighbor table entries */
struct nb_table {
    // public seg
    list_t l;
    // by fxj
    struct in_addr neighbor_addr;
    float link_stability;

    // todo: Set some notes on time and timeval here
    /*
    struct timer rt_timer;
    struct timer ack_timer;
    struct timer hello_timer;
    struct timeval last_hello_time;
    */
    // fxj_end

    // private seg
    // gcy:

    // end gcy
    // -----------------
    // cyo:

    // end cyo
    // -----------------
    // yrb:

    // end yrb
    // -----------------
    // by fxj
    u_int32_t backup_valid;
    struct in_addr backup_addr;
    float backup_stability;
    struct time backup_last_refresh_time;
    u_int32_t backup_dest_count;
    list_t backup_dest_tbl[NB_TABLESIZE];   // for dest_time
    // fxj_end
};


/*
#define RT_UNIDIR        0x1
#define RT_REPAIR        0x2
#define RT_INV_SEQNO     0x4
#define RT_INET_DEST     0x8
#define RT_GATEWAY       0x10
*/


/* neighbor entry states */
#define INVALID   0
#define VALID     1

// by fxj
struct neighbor_table {
    unsigned int num_entries;
    list_t tbl[NB_TABLESIZE];
};
// fxj_end


#endif				/* NS_NO_GLOBALS */

#ifndef NS_NO_DECLARATIONS

struct neighbor_table nb_tbl;
/*
void rt_table_init();
void rt_table_destroy();
rt_table_t *rt_table_insert(struct in_addr dest, struct in_addr next,
        u_int8_t hops, u_int32_t seqno, u_int32_t life,
        u_int8_t state, u_int16_t flags,
        unsigned int ifindex);
rt_table_t *rt_table_update(rt_table_t * rt, struct in_addr next, u_int8_t hops,
        u_int32_t seqno, u_int32_t lifetime, u_int8_t state,
        u_int16_t flags);
NS_INLINE rt_table_t *rt_table_update_timeout(rt_table_t * rt,
                                              u_int32_t lifetime);
void rt_table_update_route_timeouts(rt_table_t * fwd_rt, rt_table_t * rev_rt);
rt_table_t *rt_table_find(struct in_addr dest);
rt_table_t *rt_table_find_gateway();
int rt_table_update_inet_rt(rt_table_t * gw, u_int32_t life);
int rt_table_invalidate(rt_table_t * rt);
void rt_table_delete(rt_table_t * rt);
void precursor_add(rt_table_t * rt, struct in_addr addr);
void precursor_remove(rt_table_t * rt, struct in_addr addr);
*/
#endif				/* NS_NO_DECLARATIONS */



#endif //_NEIGHBOR_TABLE_H
