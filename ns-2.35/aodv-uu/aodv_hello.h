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
#ifndef _AODV_HELLO_H
#define _AODV_HELLO_H

#ifndef NS_NO_GLOBALS
#include "defs.h"
#include "aodv_rrep.h"
#include "routing_table.h"
#endif				/* NS_NO_GLOBALS */

#ifndef NS_NO_DECLARATIONS

#define ROUTE_TIMEOUT_SLACK 100
#define JITTER_INTERVAL 100

void hello_start();
void hello_stop();
void hello_send(void *arg);
void hello_process(RREP * hello, int rreplen, unsigned int ifindex);
void hello_process_non_hello(AODV_msg * aodv_msg, struct in_addr source,
			     unsigned int ifindex);
NS_INLINE void hello_update_timeout(rt_table_t * rt, struct timeval *now,
				    long time);

void update_stability();
void  nb_add(in_addr ip_temp);
void  nb_update_cost(in_addr ip_temp,int channel,float cost_value);

void  nb_setIsValid(in_addr ip_temp,int channel,int isValid);
void  hello_received_add(in_addr ip_temp, int channel, int num);
void  hello_ip_add(in_addr ip_temp, int channel);
void  hello_send_add();
void  hello_received_add_nb(in_addr ip_temp, int channel, int num);
void  hello_send_add_nb(in_addr ip_temp, int channel, int num);
void  hello_infos_clear();
void  add_f_value(float f, in_addr ip_temp, int channel);
float getE(u_int8_t A_send,  u_int8_t B_send, u_int8_t A_received, u_int8_t B_received);
float getF(in_addr ip_temp, int channel);
float getG(const struct node_info historyStab[], int neighbor_sum, int neighbor_change);
void  updateCost(in_addr ip_temp,int channel);
void  hello_infos_timer_add();
int hash_cmp(struct in_addr *addr1, struct in_addr *addr2);
#ifdef NS_PORT
long hello_jitter();
#endif
#endif				/* NS_NO_DECLARATIONS */

#endif				/* AODV_HELLO_H */
