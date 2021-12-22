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

#ifdef NS_PORT
#include "ns-2/aodv-uu.h"
#else
#include <netinet/in.h>
#include "aodv_rrep.h"
#include "aodv_rreq.h"
#include "aodv_neighbor.h"
#include "aodv_hello.h"
#include "routing_table.h"
#include "aodv_timeout.h"
#include "timer_queue.h"
#include "aodv_socket.h"
#include "defs.h"
#include "debug.h"
#include "params.h"

extern int unidir_hack, optimized_hellos, llfeedback;

// fxj
#ifdef USE_FXJ
extern list_t seekhead; 
#endif 
// fxj_end

#endif

RREP *NS_CLASS rrep_create(u_int8_t flags,
			   u_int8_t prefix,
			   u_int8_t hcnt,
			   struct in_addr dest_addr,
			   u_int32_t dest_seqno,
			   struct in_addr orig_addr, u_int32_t life)
{
    RREP *rrep;

    rrep = (RREP *) aodv_socket_new_msg();
    rrep->type = AODV_RREP;
    rrep->res1 = 0;
    rrep->res2 = 0;
    rrep->prefix = prefix;
    rrep->hcnt = hcnt;
    rrep->dest_addr = dest_addr.s_addr;
    rrep->dest_seqno = htonl(dest_seqno);
    rrep->orig_addr = orig_addr.s_addr;
    rrep->lifetime = htonl(life);

    if (flags & RREP_REPAIR)
	rrep->r = 1;
    if (flags & RREP_ACK)
	rrep->a = 1;

    /* Don't print information about hello messages... */
#ifdef DEBUG_OUTPUT
    if (rrep->dest_addr != rrep->orig_addr) {
	DEBUG(LOG_DEBUG, 0, "Assembled RREP:");
	log_pkt_fields((AODV_msg *) rrep);
    }
#endif

    return rrep;
}
//by cyo

RREP *NS_CLASS rrep_create(u_int8_t flags,
                           u_int8_t prefix,
                           u_int8_t hcnt,
                           struct in_addr dest_addr,
                           u_int32_t dest_seqno,
                           struct in_addr orig_addr,
						   u_int32_t life,
						   struct hello_info hello_infos[][3],
						   u_int8_t sta_nb) //by cyo
{
    RREP *rrep;

    rrep = (RREP *) aodv_socket_new_msg();
    rrep->type = AODV_RREP;
    rrep->res1 = 0;
    rrep->res2 = 0;
    rrep->prefix = prefix;
    rrep->hcnt = hcnt;
    rrep->dest_addr = dest_addr.s_addr;
    rrep->dest_seqno = htonl(dest_seqno);
    rrep->orig_addr = orig_addr.s_addr;
    rrep->lifetime = htonl(life);
    //by cyo
    for(int i = 0;i<NUM_NODE;i++){
        for(int j = 0;j<3;j++){
            rrep->union_data.hello_infos[i][j] = hello_infos[i][j];
        }
    }
	rrep->sta_nb = sta_nb;
    //cyo_end
    if (flags & RREP_REPAIR)
        rrep->r = 1;
    if (flags & RREP_ACK)
        rrep->a = 1;

    /* Don't print information about hello messages...*/
#ifdef DEBUG_OUTPUT
    if (rrep->dest_addr != rrep->orig_addr) {
    DEBUG(LOG_DEBUG, 0, "Assembled RREP:");
    log_pkt_fields((AODV_msg *) rrep);
    }
#endif

    return rrep;
}

//cyo_end
RREP_ack *NS_CLASS rrep_ack_create()
{
    RREP_ack *rrep_ack;

    rrep_ack = (RREP_ack *) aodv_socket_new_msg();
    rrep_ack->type = AODV_RREP_ACK;

    DEBUG(LOG_DEBUG, 0, "Assembled RREP_ack");

    return rrep_ack;
}

void NS_CLASS rrep_ack_process(RREP_ack * rrep_ack, int rrep_acklen,
			       struct in_addr ip_src, struct in_addr ip_dst)
{
    rt_table_t *rt;

    rt = rt_table_find(ip_src);

    if (rt == NULL) {
	DEBUG(LOG_WARNING, 0, "No RREP_ACK expected for %s", ip_to_str(ip_src));

	return;
    }
    DEBUG(LOG_DEBUG, 0, "Received RREP_ACK from %s", ip_to_str(ip_src));

    /* Remove unexpired timer for this RREP_ACK */
    timer_remove(&rt->ack_timer);
}

AODV_ext *NS_CLASS rrep_add_ext(RREP * rrep, int type, unsigned int offset,
				int len, char *data)
{
    AODV_ext *ext = NULL;

    if (offset < RREP_SIZE)
	return NULL;

    ext = (AODV_ext *) ((char *) rrep + offset);

    ext->type = type;
    ext->length = len;

    memcpy(AODV_EXT_DATA(ext), data, len);

    return ext;
}

void NS_CLASS rrep_send(RREP * rrep, rt_table_t * rev_rt,
			rt_table_t * fwd_rt, int size)
{
    u_int8_t rrep_flags = 0;
    struct in_addr dest;

    if (!rev_rt) {
	DEBUG(LOG_WARNING, 0, "Can't send RREP, rev_rt = NULL!");
	return;
    }

    dest.s_addr = rrep->dest_addr;

    /* Check if we should request a RREP-ACK */
    if ((rev_rt->state == VALID && rev_rt->flags & RT_UNIDIR) ||
	(rev_rt->hcnt == 1 && unidir_hack)) {
	rt_table_t *neighbor = rt_table_find(rev_rt->next_hop);

	if (neighbor && neighbor->state == VALID && !neighbor->ack_timer.used) {
	    /* If the node we received a RREQ for is a neighbor we are
	       probably facing a unidirectional link... Better request a
	       RREP-ack */
	    rrep_flags |= RREP_ACK;
	    neighbor->flags |= RT_UNIDIR;

	    /* Must remove any pending hello timeouts when we set the
	       RT_UNIDIR flag, else the route may expire after we begin to
	       ignore hellos... */
	    timer_remove(&neighbor->hello_timer);
	    neighbor_link_break(neighbor);

	    DEBUG(LOG_DEBUG, 0, "Link to %s is unidirectional!",
		  ip_to_str(neighbor->dest_addr));

	    timer_set_timeout(&neighbor->ack_timer, NEXT_HOP_WAIT);
	}
    }

    DEBUG(LOG_DEBUG, 0, "Sending RREP to next hop %s about %s->%s",
	  ip_to_str(rev_rt->next_hop), ip_to_str(rev_rt->dest_addr),
	  ip_to_str(dest));

    aodv_socket_send((AODV_msg *) rrep, rev_rt->next_hop, size, MAXTTL,
		     &DEV_IFINDEX(rev_rt->ifindex));

    /* Update precursor lists */
    if (fwd_rt) {
	precursor_add(fwd_rt, rev_rt->next_hop);
	precursor_add(rev_rt, fwd_rt->next_hop);
    }

    if (!llfeedback && optimized_hellos)
	hello_start();
}

void NS_CLASS rrep_forward(RREP * rrep, int size, rt_table_t * rev_rt,
			   rt_table_t * fwd_rt, int ttl)
{
    /* Sanity checks... */
    if (!fwd_rt || !rev_rt) {
	DEBUG(LOG_WARNING, 0, "Could not forward RREP because of NULL route!");
	return;
    }

    if (!rrep) {
	DEBUG(LOG_WARNING, 0, "No RREP to forward!");
	return;
    }

    DEBUG(LOG_DEBUG, 0, "Forwarding RREP to %s", ip_to_str(rev_rt->next_hop));

    /* Here we should do a check if we should request a RREP_ACK,
       i.e we suspect a unidirectional link.. But how? */
    if (0) {
	rt_table_t *neighbor;

	/* If the source of the RREP is not a neighbor we must find the
	   neighbor (link) entry which is the next hop towards the RREP
	   source... */
	if (rev_rt->dest_addr.s_addr != rev_rt->next_hop.s_addr)
	    neighbor = rt_table_find(rev_rt->next_hop);
	else
	    neighbor = rev_rt;

	if (neighbor && !neighbor->ack_timer.used) {
	    /* If the node we received a RREQ for is a neighbor we are
	       probably facing a unidirectional link... Better request a
	       RREP-ack */
	    rrep->a = 1;
	    neighbor->flags |= RT_UNIDIR;

	    timer_set_timeout(&neighbor->ack_timer, NEXT_HOP_WAIT);
	}
    }

    rrep = (RREP *) aodv_socket_queue_msg((AODV_msg *) rrep, size);
    rrep->hcnt = fwd_rt->hcnt;	/* Update the hopcount */

	//rrep->cost = rev_rt->cost; //by yrb
	rrep->channel = rev_rt->channel; //by yrb

    aodv_socket_send((AODV_msg *) rrep, rev_rt->next_hop, size, ttl,
		     &DEV_IFINDEX(rev_rt->ifindex));

    precursor_add(fwd_rt, rev_rt->next_hop);
    precursor_add(rev_rt, fwd_rt->next_hop);

    rt_table_update_timeout(rev_rt, ACTIVE_ROUTE_TIMEOUT);
}


void NS_CLASS rrep_process(RREP * rrep, int rreplen, struct in_addr ip_src,
			   struct in_addr ip_dst, int ip_ttl,
			   unsigned int ifindex)
{
    u_int32_t rrep_lifetime, rrep_seqno, rrep_new_hcnt;
    u_int8_t pre_repair_hcnt = 0, pre_repair_flags = 0;
    rt_table_t *fwd_rt, *rev_rt;
    AODV_ext *ext;
    unsigned int extlen = 0;
    int rt_flags = 0;
    struct in_addr rrep_dest, rrep_orig;
#ifdef CONFIG_GATEWAY
    struct in_addr inet_dest_addr;
    int inet_rrep = 0;
#endif


	/* added by yrb */
	// to ori_node的上一跳link_cost
	float last_cost = 0;
	float all_cost = rrep->all_cost;
	if (USE_YRB) {
		int i = 0;
		for (i = 0; i < NUM_NODE; i++) {
			if (hash_cmp(&(this_host.hello_infos[i][rrep->channel].ipaddr), &ip_src)) {
				break;
			}
		}
		if (i < NUM_NODE) {
			last_cost = this_host.nb_tbl[i][rrep->channel].cost;
			last_cost = cost_normalize(last_cost);
		}
		if (YRB_OUT) {
			printf("[%d->%d] node(%d) rrep.cost=%f, r.last.cost=%f, last.channel(%d)\n", rrep_orig.s_addr,  rrep_dest.s_addr, DEV_NR(0).ipaddr.s_addr, rrep->all_cost, last_cost, rrep->channel);
		}
		all_cost *= last_cost;
		//if (cost < COST_MIN) volat = 1;
	} else {
		all_cost = 1;
		//volat = 0;
	}
	/* end yrb */



	// fxj
	#ifdef USE_FXJ

	int fxj_fix = 0;

	if (rrep->t) {
		#ifdef FXJ_OUT
		printf("fxj_: %d recvd nb_tbl from %d, begin searching...\n", ip_dst.s_addr, ip_src.s_addr);
		#endif
		recvd_nb_tbl(ip_src, ip_dst, rrep);
		return;
	}
	if (rrep->A) {
		#ifdef FXJ_OUT
		printf("fxj_: %d comfirmed %d \'s alive...\n", ip_dst.s_addr, ip_src.s_addr);
		#endif
		in_addr src, dst;
		src.s_addr = rrep->orig_addr;
		dst.s_addr = rrep->dest_addr;
		int dest_seqno = rrep->dest_seqno;
		send_RRepC(src, ip_dst, ip_src, dst, dest_seqno);
		return;
	}
	if (rrep->c) {
		#ifdef FXJ_OUT
		printf("fxj_: %d recved %d\'s conform -> %d finally to %d.. ! !\n", 
			ip_dst.s_addr, ip_src.s_addr, rrep->orig_addr, rrep->dest_addr);
		#endif
		in_addr nbr, dst;
		nbr.s_addr = rrep->orig_addr;
		dst.s_addr = rrep->dest_addr;
		confirm_repair(ip_src, ip_dst, nbr, dst, ifindex, rrep);
		// return;
	}
	if (rrep->f) {
		fxj_fix = 1;
		#ifdef FXJ_OUT
		printf("fxj_: %d will create a forward rt  %d -> %d ! !\n", 
			ip_dst.s_addr, rrep->orig_addr, rrep->dest_addr);
		#endif
		create_forward_route(rrep, ifindex);
		return;
	}
	#endif
	// fxj_end

    /* Convert to correct byte order on affeected fields: */
    rrep_dest.s_addr = rrep->dest_addr;
    rrep_orig.s_addr = rrep->orig_addr;
    rrep_seqno = ntohl(rrep->dest_seqno);
    rrep_lifetime = ntohl(rrep->lifetime);
    /* Increment RREP hop count to account for intermediate node... */
    rrep_new_hcnt = rrep->hcnt + 1;

    if (rreplen < (int) RREP_SIZE) {
	alog(LOG_WARNING, 0, __FUNCTION__,
	     "IP data field too short (%u bytes)"
	     " from %s to %s", rreplen, ip_to_str(ip_src), ip_to_str(ip_dst));
	return;
    }

    /* Ignore messages which aim to a create a route to one self */
    if (rrep_dest.s_addr == DEV_IFINDEX(ifindex).ipaddr.s_addr)
	return;

    DEBUG(LOG_DEBUG, 0, "from %s about %s->%s",
	  ip_to_str(ip_src), ip_to_str(rrep_orig), ip_to_str(rrep_dest));
#ifdef DEBUG_OUTPUT
    log_pkt_fields((AODV_msg *) rrep);
#endif

    /* Determine whether there are any extensions */
    ext = (AODV_ext *) ((char *) rrep + RREP_SIZE);

    while ((rreplen - extlen) > RREP_SIZE) {
	switch (ext->type) {
	case RREP_EXT:
	    DEBUG(LOG_INFO, 0, "RREP include EXTENSION");
	    /* Do something here */
	    break;
#ifdef CONFIG_GATEWAY
	case RREP_INET_DEST_EXT:
	    if (ext->length == sizeof(u_int32_t)) {

		/* Destination address in RREP is the gateway address, while the
		 * extension holds the real destination */
		memcpy(&inet_dest_addr, AODV_EXT_DATA(ext), ext->length);

		DEBUG(LOG_DEBUG, 0, "RREP_INET_DEST_EXT: <%s>",
		      ip_to_str(inet_dest_addr));
		/* This was a RREP from a gateway */
		rt_flags |= RT_GATEWAY;
		inet_rrep = 1;
		break;
	    }
#endif
	default:
	    alog(LOG_WARNING, 0, __FUNCTION__, "Unknown or bad extension %d",
		 ext->type);
	    break;
	}
	extlen += AODV_EXT_SIZE(ext);
	ext = AODV_EXT_NEXT(ext);
    }

    /* ---------- CHECK IF WE SHOULD MAKE A FORWARD ROUTE ------------ */

    fwd_rt = rt_table_find(rrep_dest);
    rev_rt = rt_table_find(rrep_orig);

    if (!fwd_rt) {
	/* We didn't have an existing entry, so we insert a new one. */
	fwd_rt = rt_table_insert(rrep_dest, ip_src, rrep_new_hcnt, rrep_seqno,
				 rrep_lifetime, VALID, rt_flags, ifindex,
				 1.0, all_cost, rrep->channel); //added by yrb
		if (rev_rt) {
			rev_rt->last_all_cost = all_cost; //added by yrb
			fwd_rt->last_all_cost = rev_rt->next_all_cost; //added by yrb
		}
		if (YRB_OUT) {
			printf("[%d->%d] node(%d) rrep insert: next(%d) r.last.cost(%f), r.lastall.cost(%f), channel(%d)\n", rrep_orig.s_addr, rrep_dest.s_addr, DEV_NR(0).ipaddr.s_addr, ip_src.s_addr, last_cost, all_cost, rrep->channel);
		}
		// fxj
		#ifdef USE_FXJ
		if (fxj_fix) {
				
			} else {
				// by fxj: add nexts to rt_tbl
				for (unsigned int i = 0; i < rrep_new_hcnt; i++) {
					fwd_rt->all_nexts[i] = rrep->union_data.nexts[i];
				}
			}
		#endif
		// fxj_end
    } 
    /* 更新正向路由 */
	else if (fwd_rt->dest_seqno == 0 ||
	       (int32_t) rrep_seqno > (int32_t) fwd_rt->dest_seqno ||
		   (rrep_seqno == fwd_rt->dest_seqno &&
		   (fwd_rt->state == INVALID || fwd_rt->flags & RT_UNIDIR || rrep_new_hcnt < fwd_rt->hcnt)))  
		   // yrb note: 不用管cost，因为序列号rrep_create会更新
	{
		
		if (YRB_OUT) {
			printf("[%d->%d] node(%d) rrep update: next(%d) r.last.cost(%f), r.lastall.cost(%f), channel(%d)\n", rrep_orig.s_addr, rrep_dest.s_addr, DEV_NR(0).ipaddr.s_addr, ip_src.s_addr, last_cost, all_cost, rrep->channel);
		}
		
		pre_repair_hcnt = fwd_rt->hcnt;
		pre_repair_flags = fwd_rt->flags;

    	/* 传递cost值 */
		fwd_rt = rt_table_update(fwd_rt, ip_src, rrep_new_hcnt, rrep_seqno,
					rrep_lifetime, VALID,
					rt_flags | fwd_rt->flags,
					1.0, all_cost, rrep->channel); //added by yrb
		if (rev_rt) {
			rev_rt->last_all_cost = all_cost; //added by yrb
			fwd_rt->last_all_cost = rev_rt->next_all_cost; //added by yrb
		}
		#ifdef USE_FXJ
		if (fxj_fix) {
				
			} else {
				// by fxj: add nexts to rt_tbl
				for (unsigned int i = 0; i < rrep_new_hcnt; i++) {
					fwd_rt->all_nexts[i] = rrep->union_data.nexts[i];
				}
			}
		#endif
    } 
	else {
		if (fwd_rt->hcnt > 1) {
			DEBUG(LOG_DEBUG, 0,
			"Dropping RREP, fwd_rt->hcnt=%d fwd_rt->seqno=%ld",
			fwd_rt->hcnt, fwd_rt->dest_seqno);
		}
	return;
    }


    /* If the RREP_ACK flag is set we must send a RREP
       acknowledgement to the destination that replied... */
    if (rrep->a) {
	RREP_ack *rrep_ack;

	rrep_ack = rrep_ack_create();
	aodv_socket_send((AODV_msg *) rrep_ack, fwd_rt->next_hop,
			 NEXT_HOP_WAIT, MAXTTL, &DEV_IFINDEX(fwd_rt->ifindex));
	/* Remove RREP_ACK flag... */
	rrep->a = 0;
    }

    /* Check if this RREP was for us (i.e. we previously made a RREQ
       for this host). */
    if (rrep_orig.s_addr == DEV_IFINDEX(ifindex).ipaddr.s_addr) {
#ifdef CONFIG_GATEWAY
	if (inet_rrep) {
	    rt_table_t *inet_rt;
	    inet_rt = rt_table_find(inet_dest_addr);

	    /* Add a "fake" route indicating that this is an Internet
	     * destination, thus should be encapsulated and routed through a
	     * gateway... */
	    if (!inet_rt) {
			rt_table_insert(inet_dest_addr, rrep_dest, rrep_new_hcnt, 0,
				rrep_lifetime, VALID, RT_INET_DEST, ifindex,
				rev_rt->next_all_cost, 1, rev_rt->channel);
			// fxj
			#ifdef USE_FXJ
			if (fxj_fix) {
				
			} else {
				// by fxj: add nexts to rt_tbl
				for (unsigned int i = 0; i < rrep_new_hcnt; i++) {
					fwd_rt->all_nexts[i] = rrep->union_data.nexts[i];
				}
			}
			#endif
			// fxj_end
		} else if (inet_rt->state == INVALID || rrep_new_hcnt < inet_rt->hcnt) {
			rt_table_update(inet_rt, rrep_dest, rrep_new_hcnt, 0,
							rrep_lifetime, VALID, RT_INET_DEST |
							inet_rt->flags,
				 			rev_rt->next_all_cost, 1, rev_rt->channel);
			// fxj
			#ifdef USE_FXJ
			if (fxj_fix) {
				
			} else {
				// by fxj: add nexts to rt_tbl
				for (unsigned int i = 0; i < rrep_new_hcnt; i++) {
					fwd_rt->all_nexts[i] = rrep->union_data.nexts[i];
				}
			}
			#endif
			// fxj_end
	    } else {
		DEBUG(LOG_DEBUG, 0, "INET Response, but no update %s",
		      ip_to_str(inet_dest_addr));
	    }
	}
#endif				/* CONFIG_GATEWAY */

	/* If the route was previously in repair, a NO DELETE RERR should be
	   sent to the source of the route, so that it may choose to reinitiate
	   route discovery for the destination. Fixed a bug here that caused the
	   repair flag to be unset and the RERR never being sent. Thanks to
	   McWood <hjw_5@hotmail.com> for discovering this. */
	if (pre_repair_flags & RT_REPAIR) {
	    if (fwd_rt->hcnt > pre_repair_hcnt) {
		RERR *rerr;
		u_int8_t rerr_flags = 0;
		struct in_addr dest;

		dest.s_addr = AODV_BROADCAST;

		rerr_flags |= RERR_NODELETE;
		rerr = rerr_create(rerr_flags, fwd_rt->dest_addr,
				   fwd_rt->dest_seqno);

		if (fwd_rt->nprec)
		    aodv_socket_send((AODV_msg *) rerr, dest,
				     RERR_CALC_SIZE(rerr), 1,
				     &DEV_IFINDEX(fwd_rt->ifindex));
	    }
	}
    } else {
	/* --- Here we FORWARD the RREP on the REVERSE route --- */
	if (rev_rt && rev_rt->state == VALID) {
		// fxj_: add himself into the chain
		#ifdef USE_FXJ
		if (!fxj_fix) {
			rrep->union_data.nexts[rrep_new_hcnt] = ip_dst;
		}
		#endif
		// fxj_end
	    rrep_forward(rrep, rreplen, rev_rt, fwd_rt, --ip_ttl);
	} else {
	    DEBUG(LOG_DEBUG, 0, "Could not forward RREP - NO ROUTE!!!");
	}
    }

    if (!llfeedback && optimized_hellos)
	hello_start();
}

/************************************************************************/

/* Include a Hello Interval Extension on the RREP and return new offset */

int rrep_add_hello_ext(RREP * rrep, int offset, u_int32_t interval)
{
    AODV_ext *ext;

    ext = (AODV_ext *) ((char *) rrep + RREP_SIZE + offset);
    ext->type = RREP_HELLO_INTERVAL_EXT;
    ext->length = sizeof(interval);
    memcpy(AODV_EXT_DATA(ext), &interval, sizeof(interval));

    return (offset + AODV_EXT_SIZE(ext));
}

// fxj
#ifdef USE_FXJ
void NS_CLASS send_RRepA(in_addr mid, in_addr nbr, in_addr src, in_addr dst, int ifindex) {
	rt_table_t *rt_entry = rt_table_find(dst);
	if (rt_entry) {
		RREP *rrep = rrep_create(0, 0, 1, dst, rt_entry->dest_seqno, src, 1);
		rrep->A = 1;
		aodv_socket_send((AODV_msg*)rrep, mid, sizeof(RREP), 1, &DEV_NR(ifindex));
	}
}

void NS_CLASS send_RRepC(in_addr src, in_addr mid, in_addr nbr, in_addr dst, int dest_seqno) {
	RREP *rrep = rrep_create(0, 0, 1, dst, dest_seqno, nbr, 1);
	rrep->c = 1;
	int ifindex = 0;
	for (ifindex = 0; ifindex < MAX_NR_INTERFACES; ifindex++) {
        if (!DEV_NR(ifindex).enabled)
            continue;
        else
            break;
    }
	aodv_socket_send((AODV_msg*)rrep, src, sizeof(RREP), 1, &DEV_NR(ifindex));
}

void NS_CLASS send_RRepF(in_addr mid, in_addr nbr, rt_table_t *rt_entry, int ifindex) {
	RREP *rrep = rrep_create(0, 0, rt_entry->hcnt - 1, rt_entry->dest_addr, 0, nbr, 1);
	rrep->f = 1;
	for (int i = 0; i < rrep->hcnt; i++) {
		rrep->union_data.nexts[i].s_addr = rt_entry->all_nexts[i + 1].s_addr;
	}
	aodv_socket_send((AODV_msg*)rrep, mid, sizeof(RREP), 1, &DEV_NR(ifindex));
}

void NS_CLASS confirm_repair(in_addr mid, in_addr src, in_addr nbr, in_addr dst, int ifindex, RREP* rrep) {
	rt_table_t *rt_entry = rt_table_find(dst);
	if (!rt_entry) return;
	// send route table
	// remove sk list and send cached pack
	list_t *pos, *temp__;
	list_foreach_safe(pos, temp__, &seekhead) {
		seek_list_t *sk_entry = (seek_list_t *) pos;
		if (sk_entry->dest_addr.s_addr != dst.s_addr) continue;
		list_detach(pos);
		sk_entry->seek_timer.handler = 0;
		rt_entry->flags &= 0xfffffffd;
		rt_entry->next_hop.s_addr = mid.s_addr;
		int i;
		for (i = 0; i < rt_entry->hcnt; i++) {
			if (nbr.s_addr = rt_entry->all_nexts[i].s_addr) 
				break;
		}
		if (i == 0) {
			for (int j = rt_entry->hcnt; j > 0; j--)
				rt_entry->all_nexts[j].s_addr = rt_entry->all_nexts[j - 1].s_addr;
			rt_entry->hcnt++;
		} else if (i >= 2) {
			for (int j = 1; j < rt_entry->hcnt + 1 - i; j++)
				rt_entry->all_nexts[j].s_addr = rt_entry->all_nexts[j + i - 1].s_addr;
			rt_entry->hcnt -= i - 1;
		}
		rrep->hcnt = rt_entry->hcnt;
		rt_entry->all_nexts[0].s_addr = mid.s_addr;
		send_RRepF(mid, nbr, rt_entry, ifindex);
		#ifdef FXJ_OUT
		printf("fxj_: %d will forwar to %d -> %d finally to %d...\n", 
			src.s_addr, mid.s_addr, nbr.s_addr, dst.s_addr);
		printf("fxj_: %d is sending all cached packets to dst %d via %d...A SUCCESSFUL REPAIR ! !\n", 
			src.s_addr, dst.s_addr, mid.s_addr);
		#endif		
		packet_queue_set_verdict(sk_entry->dest_addr, PQ_SEND);
		break;
    }
}

void NS_CLASS create_forward_route(RREP *rrep, int ifindex) {
	in_addr dest, next; 
	next.s_addr = rrep->orig_addr;
	dest.s_addr = rrep->dest_addr;
	rt_table_t *re = rt_table_find(dest);
	if (re) return;
	int channel = nb_best_channel(next);
	re = rt_table_insert(dest, next, rrep->hcnt - 1, 0,
				 MY_ROUTE_TIMEOUT, VALID, 0, ifindex,
				 1, 1, channel); //added by yrb
	// by fxj_: add nexts to rt_tbl
	for (int i = 1; i < rrep->hcnt - 1; i++) {
		re->all_nexts[i - 1] = rrep->union_data.nexts[i];
	}
}

void NS_CLASS recvd_nb_tbl(in_addr mid, in_addr src, RREP* rrep) {
	#ifdef FXJ_OUT
	printf("       valid neighbors: %d. ", rrep->hcnt);
	if (rrep->hcnt > 0) {
        printf("(");
        for (int i = 0; i < rrep->hcnt; i++)
            printf("%d, ", rrep->union_data.nexts[i]);
        printf(")\n");
    } else {
        printf("\n");
    }
	#endif
	list_t *pos, *temp__;
    list_foreach_safe(pos, temp__, &seekhead) {
		seek_list_t *sk_entry = (seek_list_t *) pos;
		// sk_entry->dest_addr.s_addr
		rt_table_t *rt_entry = rt_table_find(sk_entry->dest_addr);
		#ifdef FXJ_OUT_1
		printf("fxj_: searching dest %d.\n", sk_entry->dest_addr.s_addr);
		#endif
		if (rt_entry) {
			for (int i = 0; i < rt_entry->hcnt; i++) {
				#ifdef FXJ_OUT_2
				printf("       searching next %d.\n", rt_entry->all_nexts[i].s_addr);
				#endif
				if (rt_entry->all_nexts[i].s_addr == mid.s_addr) {
					#ifdef FXJ_OUT
					printf("fxj_: %d found a next %d in range, GREAT ! ! \n", src.s_addr, mid.s_addr);
					#endif
					list_detach(pos);
					sk_entry->seek_timer.handler = 0;
					rt_entry->next_hop.s_addr = mid.s_addr;
					rt_entry->flags &= 0xfffffffd;
					packet_queue_set_verdict(sk_entry->dest_addr, PQ_SEND);
					#ifdef FXJ_OUT
					printf("fxj_: %d is sending all cached packets to dst %d via %d...A SUCCESSFUL REPAIR ! !\n", 
						src.s_addr, sk_entry->dest_addr, mid.s_addr);
					#endif
					break;
				}
				for (int j = 0; j < rrep->hcnt; j++) {
					if (rt_entry->all_nexts[i].s_addr == rrep->union_data.nexts[j].s_addr) {
						send_RReqT(src, mid, rrep->union_data.nexts[j], sk_entry->dest_addr, ifindex);
					}
				}
			}
		}
    }
}

#endif
//fxj_end
