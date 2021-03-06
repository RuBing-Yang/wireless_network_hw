/*****************************************************************************
 *
 * Copyright (C) 2001 Uppsala University & Ericsson AB.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Authors: Erik Nordstr�m, <erik.nordstrom@it.uu.se>
 *          
 *
 *****************************************************************************/
#ifndef _AODV_RREP_H
#define _AODV_RREP_H

#ifndef NS_NO_GLOBALS
#include <endian.h>

#include "defs.h"
#include "routing_table.h"

/* RREP Flags: */

#define RREP_ACK       0x1
#define RREP_REPAIR    0x2

//by fxj
union rrep_union{
    struct hello_info hello_infos[NUM_NODE][3];
    struct in_addr nexts[NUM_NODE];
} ;
//fxj_end
typedef struct {
    u_int8_t type;
// by fxj
#if defined(__LITTLE_ENDIAN)
    u_int16_t res1:1;
    u_int16_t f:1;
    u_int16_t c:1;
    u_int16_t A:1;
    u_int16_t t:1;
    u_int16_t n:1;
    u_int16_t a:1;
    u_int16_t r:1;
    u_int16_t prefix:5;
    u_int16_t res2:3;
#elif defined(__BIG_ENDIAN)
    u_int16_t r:1;
    u_int16_t a:1;
    u_int16_t n:1;
    u_int16_t t:1;
    u_int16_t A:1;
    u_int16_t c:1;
    u_int16_t f:1;
    u_int16_t res1:1;
    u_int16_t res2:3;
    u_int16_t prefix:5;
#else
#error "Adjust your <bits/endian.h> defines"
#endif
// fxj_end

    u_int8_t hcnt;
    u_int32_t dest_addr;
    u_int32_t dest_seqno;
    u_int32_t orig_addr;
    u_int32_t lifetime;

    //by cyo & fxj
    union rrep_union union_data;
     u_int8_t sta_nb;//todo add history_sta
    //cyo_end  fxj_end
    
    /* by gcy */
    u_int8_t channel;
    /* end */

    float cost; // by yrb
    
} RREP;

#define RREP_SIZE sizeof(RREP)

typedef struct {
    u_int8_t type;
    u_int8_t reserved;
} RREP_ack;

#define RREP_ACK_SIZE sizeof(RREP_ack)
#endif				/* NS_NO_GLOBALS */

#ifndef NS_NO_DECLARATIONS
RREP *rrep_create(u_int8_t flags,
		  u_int8_t prefix,
		  u_int8_t hcnt,
		  struct in_addr dest_addr,
		  u_int32_t dest_seqno,
		  struct in_addr orig_addr, u_int32_t life);
//by cyo

RREP *rrep_create(u_int8_t flags,
                  u_int8_t prefix,
                  u_int8_t hcnt,
                  struct in_addr dest_addr,
                  u_int32_t dest_seqno,
                  struct in_addr orig_addr, 
                  u_int32_t life,
                  struct hello_info hello_infos[][3],
                  u_int8_t sta_nb);

//cyo_end


RREP_ack *rrep_ack_create();
AODV_ext *rrep_add_ext(RREP * rrep, int type, unsigned int offset,
		       int len, char *data);
void rrep_send(RREP * rrep, rt_table_t * rev_rt, rt_table_t * fwd_rt, int size);
void rrep_forward(RREP * rrep, int size, rt_table_t * rev_rt,
		  rt_table_t * fwd_rt, int ttl);
void rrep_process(RREP * rrep, int rreplen, struct in_addr ip_src,
		  struct in_addr ip_dst, int ip_ttl, unsigned int ifindex);
void rrep_ack_process(RREP_ack * rrep_ack, int rreplen, struct in_addr ip_src,
		      struct in_addr ip_dst);
void recvd_nb_tbl(in_addr mid, in_addr src, RREP* rrep);
void send_RRepA(in_addr mid, in_addr nbr, in_addr src, in_addr dst, int ifindex); // fxj
void send_RRepC(in_addr src, in_addr mid, in_addr nbr, in_addr dst); // fxj
void confirm_repair(in_addr mid, in_addr src, in_addr nbr, in_addr dst, int ifindex);  // fxj
void send_RRepF(in_addr mid, in_addr nbr, rt_table_t *rt_entry, int ifindex); // fxj
void create_forward_route(RREP *rrep, int ifindex);   //fxj
#endif				/* NS_NO_DECLARATIONS */

#endif				/* AODV_RREP_H */
